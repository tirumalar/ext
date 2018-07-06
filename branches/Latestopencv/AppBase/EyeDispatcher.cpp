/*
 * EyeDispatcher.cpp
 *
 *  Created on: Jan 17, 2011
 *      Author: developer1
 */
#include <iostream>
#include "SocketFactory.h"
#include "EyeDispatcher.h"
extern "C"{
#include "file_manip.h"
}

#define TERMINATE_WITHOUT_DESTINATION 0

EyeDispatcher::EyeDispatcher(Configuration& conf, const char *address):GenericProcessor(conf), m_queueSz(0),m_prevcam(0),m_debug(false),m_eyeDispatcherSleepUsec(100000)
{
	Init(conf, address);
}

EyeDispatcher::EyeDispatcher(Configuration& conf):GenericProcessor(conf),m_queueSz(0),m_socketFactory(NULL),m_prevcam(0),m_debug(false),m_eyeDispatcherSleepUsec(100000)
{
    const char *address =conf.getValue("GRI.EyeDestAddr",(const char*)0);
    Init(conf, address);
}

void EyeDispatcher::Init(Configuration& conf, const char *address)
{
	m_queueFullBehaviour = DROP;
    m_cw = conf.getValue("GRI.cropWidth", 384);
    m_ch = conf.getValue("GRI.cropHeight", 288);
    m_block = conf.getValue("Eyelock.SendBlock", 1);
    int numbits = conf.getValue("Eyelock.NumBits",8);
    m_numbits = numbits > 8?16:numbits;

    int timeOutConnms = conf.getValue("GRI.EyeConnecTimeoutmsec", 2000);
	int timeOutSendms = conf.getValue("GRI.EyeSendTimeoutmsec", 2000);

    m_timeOutConn.tv_sec = timeOutConnms / 1000;
	m_timeOutConn.tv_usec = (timeOutConnms % 1000) * 1000;
	m_timeOutSend.tv_sec = timeOutSendms / 1000;
	m_timeOutSend.tv_usec = (timeOutSendms % 1000) * 1000;

	// Allow for possibility of NULL address, to support dynamic enrollment requests
	m_haddr = 0;
	m_destAddr.clear();
	SetAddress(address);
	m_socketFactory = new SocketFactory(conf);
	m_bSecure = (int) (m_socketFactory->getSecurityType("GRI.EyeDispatcherSecure"));
	m_bOrigSecure = m_bSecure;


	int qsz = conf.getValue("GRI.EyeDispatcherQueueSize",30);
	qsz = qsz/2;

	int msgSize = HTTPPOSTMsg::calcMessageSize(m_cw, m_ch);
	msgSize = msgSize *(m_numbits>>3);
	m_tempMsg = new HTTPPOSTMsg(msgSize);

	for(int i=0;i<qsz;i++){
		std::pair<HTTPPOSTMsg*,bool> test;
		test.second = false;
		test.first = new HTTPPOSTMsg(msgSize);
	    m_CList[0].push_back(test);
	    test.first = new HTTPPOSTMsg(msgSize);
	    m_CList[1].push_back(test);
	}
	m_debug = conf.getValue("GRI.EyeDispatcherDebug",false);
	m_eyeDispatcherSleepUsec = conf.getValue("GRI.EyeDispatcherSleepMsec",100);
	m_eyeDispatcherSleepUsec = m_eyeDispatcherSleepUsec*1000; // microsec
}

bool EyeDispatcher::IsActive()
{
	return ((m_haddr != 0) || (m_haddrList.size() > 0));
}

void EyeDispatcher::SetAddress(const char *address)
{
	if(address != 0)
	{
		// m_destAddr will change after following call !!
		HostAddress *pAddress = 0;
		try
		{
			pAddress = HostAddress::MakeHost(address);
			m_destAddr = address; // only set these if no exception is thrown
			m_haddr = pAddress;
		}
		catch(...)
		{
			printf("EyeDispatcher can't resolve target hostname %s, skipping...\n", address); fflush(stdout);
		}
	}
}

void EyeDispatcher::SetSecureComm(bool secure)
{
	ScopeLock lock(m_SecureCommLock);
	if (m_bSecure != secure)
	{
		m_bSecure = secure;
	}
}

void EyeDispatcher::ResetSecureComm()
{
	ScopeLock lock(m_SecureCommLock);
	m_bSecure = m_bOrigSecure;
}

#define DEBUG_STARVATION 0

class SemLock
{
public:
	SemLock(BinarySemaphore &sem) : m_Sem(sem)
	{
		m_Sem.Lock();
	}
	~SemLock()
	{
		m_Sem.Unlock();
	}
protected:
	BinarySemaphore &m_Sem;
};

#define USE_SEMAPHORE 1

void EyeDispatcher::AddAddress(const char *address)
{
	//printf("EyeDispatcher::Enter AddAddress => %s\n", address); fflush(stdout);
	try
	{
		if(address != 0)
		{
			//printf("HostAddress::HostAddress(%s) =>", address); fflush(stdout);
			HostAddress *pAddress = new HostAddress(address);
			//printf("HostAddress::HostAddress(%s) <=", address); fflush(stdout);

#if USE_SEMAPHORE
			SemLock lock(m_Sem);
#else
			ScopeLock lock(m_Lock);
#endif
			m_haddrList.push_back(pAddress);
		}
	}
	catch(Exception& nex)
	{
		printf("EyeDispatcher::AddAddress exception\n"); fflush(stdout);
		nex.PrintException();
	}
	//printf("EyeDispatcher::Exit AddAddress <=\n"); fflush(stdout);
}

void EyeDispatcher::ClearAddressList()
{
	//printf("EyeDispatcher::Enter ClearAddressList >>>\n"); fflush(stdout);
#if USE_SEMAPHORE
	SemLock lock(m_Sem);
#else
	ScopeLock lock(m_Lock);
#endif
	for(int i = 0; i < m_haddrList.size(); i++)
	{
		if(m_haddrList[i])
		{
			delete m_haddrList[i];
		}
	}
	m_haddrList.clear();
	//printf("EyeDispatcher::Exit ClearAddressList <<<\n"); fflush(stdout);
}

bool EyeDispatcher::GetFreeMsg(int cam,int& index){
	index = -1;
	bool ret = false;
	for(int i=0;i<m_CList[cam].size();i++){
		if(m_CList[cam][i].second == false){
			index = i;
			ret = true;
			break;
		}
	}
	return ret;
}

HTTPPOSTMsg* EyeDispatcher::GetMsgToSend(){
	ScopeLock lock(m_vectorLock);
	HTTPPOSTMsg *msg=NULL;
	int cam = m_prevcam;
	int index = -1;
	bool ret= false;
	for(int i=0;i<m_CList[cam].size();i++){
		if(m_CList[cam][i].second == true){
			index = i;
			ret = true;
			break;
		}
	}
	if(ret){
		m_tempMsg->CopyFrom(*(m_CList[cam][index].first));
		m_CList[cam][index].second = false;
	}
	if(ret)msg = m_tempMsg;
	m_prevcam = 1-m_prevcam;
	return msg;
}



void EyeDispatcher::PushMsg(int cam,HTTPPOSTMsg& msg){
	ScopeLock lock(m_vectorLock);
	int indx = -1;
	if((cam == 0)||(cam == 1)){
		if(GetFreeMsg(cam,indx)){
			m_CList[cam][indx].first->CopyFrom(msg);
			m_CList[cam][indx].second = true;
		}
	}
	if(indx == -1) printf("ED Queue Full Dropping for %d \n",cam);
}

bool EyeDispatcher::enqueMsg(Copyable& msg)
{
	HTTPPOSTMsg & hMsg = (HTTPPOSTMsg &) msg;
	int cam = -1;
	bool test = hMsg.getCameraNo(cam);
	if(m_debug)printf("Enque Msg %d %d %d\n",cam,hMsg.getFrameIndex(),hMsg.getEyeIndex());
	// Don't bother queueing images if we don't have an active destination address
	if(!IsActive()) return false;
	PushMsg(cam,hMsg);
}

EyeDispatcher::~EyeDispatcher()
{
	if(m_haddr)
	{
		delete m_haddr;
	}
	ClearAddressList();
	if(m_tempMsg)
		delete m_tempMsg;
	ScopeLock lock(m_vectorLock);

	for(int i=0;i<m_CList[0].size();i++){
		delete m_CList[0][i].first;
	}
	for(int i=0;i<m_CList[1].size();i++){
		delete m_CList[1][i].first;
	}
}


void EyeDispatcher::FlushAll()
{
	printf("-");
	for(int i=0;i<m_queueSz;i++){
		Safe<Copyable *> & currMsg = m_sendIter.curr();
		currMsg.lock();
		if(currMsg.isUpdated()){
			m_inQueue.decrCounter();
		}
//		printf("%d %d \n",i,currMsg.isUpdated()?1:0);
		currMsg.setUpdated(false);
		currMsg.unlock();
		m_sendIter.next();
	}
	for(int i=0;i<m_CList[0].size();i++){
		m_CList[0][i].second = false;
	}
	for(int i=0;i<m_CList[1].size();i++){
		m_CList[1][i].second = false;
	}

}

void EyeDispatcher::SendMessage(HTTPPOSTMsg* out_msg,bool bblock)
{
	// Always send to the default address if enabled
	if(m_haddr)
	{
		SendMessage(out_msg, *m_haddr, bblock, true);
	}

#if USE_SEMAPHORE
	SemLock lock(m_Sem); // lock protected m_haddrList
#else
	ScopeLock lock(m_Lock);
#endif
	if(m_haddrList.size()) // Be sure to lock the address list in case of dynamic changes
	{
		for(int i = 0; i < m_haddrList.size(); i++)
		{
			char CamID[256]={0};
			out_msg->getCameraID(CamID);
			if(m_debug)printf("Sending to %s %d %d %s \n",CamID,out_msg->getFrameIndex(),out_msg->getEyeIndex(),m_haddrList[i]->GetOrigHostName());
			SendMessage(out_msg, *m_haddrList[i], bblock, false);
		}
	}
}

void EyeDispatcher::SendMessage(HTTPPOSTMsg* out_msg, HostAddress &address, bool bblock, bool retry)
{
	try
	{
		Configuration* conf = getConf();
		bool supportNanoSDK = conf->getValue("GRI.SupportNanoSDK",0);
		SocketSecurityType security = SOCK_UNSECURE;
		if(supportNanoSDK)
		{
			ScopeLock lock(m_SecureCommLock);
			security = m_bSecure ? SOCK_SECURE : SOCK_UNSECURE;
		}
		SocketClient client= (supportNanoSDK) ? m_socketFactory->createSocketClient(security): m_socketFactory->createSocketClient ("GRI.EyeDispatcherSecure");
		client.SetTimeouts(m_timeOutConn);

		if(retry) {
			client.ConnectByHostname(address);
		} else {
			client.Connect(address);
		}

		client.SetTimeouts(m_timeOutSend);

		if(bblock){
			client.Send(*out_msg);
		}
		else{
			client.Send(*out_msg,MSG_DONTWAIT);
		}
	}
	catch(Exception& nex)
	{
		printf("EyeDispatcher::SendMessage failed\n"); fflush(stdout);
		nex.PrintException();
	}
}

void EyeDispatcher::GrabSendMsg()
{
	HTTPPOSTMsg* eyeMsg = GetMsgToSend();
	if(!eyeMsg) return;
	try{
		XTIME_OP("EDSENDMSG",SendMessage(eyeMsg,m_block?true:false);
		);
	}
	catch(Exception& nex)
	{
		nex.PrintException();
	}
	catch(const char *msg)
	{
		std::cout<< msg <<endl;
	}
	catch(...)
	{
		std::cout<< "Unknown exception during Eye Send" <<endl;
	}
}


unsigned int EyeDispatcher::MainLoop() {

	std::string name = "EyeDispatcher::";
	try {
		while (!ShouldIQuit()) {
			GrabSendMsg();
			Frequency();
			usleep(m_eyeDispatcherSleepUsec);
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}
	return 0;
}



