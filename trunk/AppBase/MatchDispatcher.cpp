/*
 * MatchDispatcher.cpp
 *
 *  Created on: Nov 29, 2012
 *      Author: mamigo
 */

#include <iostream>
#include <unistd.h>
#include "socket.h"
#include "SocketFactory.h"
#include "HTTPPOSTMsg.h"
#include "MatchDispatcher.h"
#include "NwMatchManager.h"
#include "F2FDispatcher.h"
#include "LoiteringDetector.h"
#include "NwLEDDispatcher.h"
#include "LEDConsolidator.h"
#include "SDKDispatcher.h"
#include "DBMap.h"
#include "logging.h"
extern "C" {
#include "BobListener.h"
}

#include <iostream>
const char logger[30] = "MatchDispatcher";

MatchDispatcher::MatchDispatcher(Configuration& conf)
: ResultDispatcher(conf)
,m_f2fDispatcher(NULL)
,m_statusDestAddr(NULL)
,m_weigandDestAddr(NULL)
,m_clientAddr(NULL)
,m_ResetMsg(0)
,m_logging(false)
,m_nwMatchMgr(NULL)
,m_dualMatcherPolicy(false)
,m_dualEyeMatchThresholdTime(2000000)
,m_matchDispSleepusec(500000)
,m_nwLedDispatcher(NULL)
,m_resultDestAddr(NULL)
,m_imageProcessor(NULL)
{
	// TODO Auto-generated constructor stub
	m_logging = conf.getValue("Eyelock.Logging", false);
	m_LastAuthorizationID = -1;

	m_checkUID = conf.getValue("Eyelock.CheckUID",true);

	m_TimeofLastAuthorization = 0;
	m_RepeatAuthorizationPeriod = 1000 * conf.getValue("GRI.RepeatAuthorizationPeriod", 2000);//2sec


	const char *resetStr=conf.getValue("GRI.ResetStr","RESETEYELOCK");
	m_ResetMsg = new HTTPPOSTMsg(resetStr,strlen(resetStr));


	m_clientAddr = (char*)conf.getValue("GRI.SlaveAddressList","NONE");
	if(strcmp(m_clientAddr, "NONE") == 0){
		m_clientAddr = 0;
    }
	if(m_clientAddr)
		m_statusDestAddr = HostAddress::MakeHost(m_clientAddr);


    m_clientAddr = (char*)conf.getValue("Eyelock.WeigandDestAddr","NONE");
	if(strcmp(m_clientAddr, "NONE") == 0){
		m_clientAddr = 0;
	}
	if(m_clientAddr)
		m_weigandDestAddr = HostAddress::MakeHost(m_clientAddr);

//new items
	m_Debug = conf.getValue("Eyelock.MatchDispDebug",false);
#if 0
	m_matchDispSleepusec = 1000 * conf.getValue("Eyelock.MatchDispSleepusec", 200);//200 milli sec
#else
	m_matchDispSleepusec = 1000 * conf.getValue("Eyelock.MatchDispSleepusec", 10);// 10 milli sec
#endif
	m_dualEyeMatchThresholdTime = 1000 * conf.getValue("Eyelock.DualEyeMatchThresholdTime", 2000);//2sec
	m_dualMatcherPolicy = conf.getValue("Eyelock.DualMatcherPolicy",false);


	const char *svrAddr = conf.getValue("Eyelock.TSMasterDestAddr", "NONE");
	if(strcmp(svrAddr, "NONE") == 0){
    	m_tsDestAddrpresent = false;
	}else
		m_tsDestAddrpresent = true;

	m_resultDestAddr= HostAddress::MakeHost(svrAddr, eIPv4, false);
	int timeOutms = conf.getValue("Eyelock.TSMasterSocketTimeOutMillis", 200);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
	m_socketFactory = new SocketFactory(conf);
}

MatchDispatcher::~MatchDispatcher() {
	// TODO Auto-generated destructor stub

	if(m_statusDestAddr)
		delete m_statusDestAddr;
	if(m_weigandDestAddr)
		delete m_weigandDestAddr;
	if(m_resultDestAddr)
		delete m_resultDestAddr;
	if(m_socketFactory)
		delete m_socketFactory;
	ClearMatchedItems();
}


int MatchDispatcher::getQueueSize(Configuration* conf)
{
	return 1;
}

bool MatchDispatcher::enqueMsg(Copyable& msg)
{
	ScopeLock lock(m_matchLock);
	MatchResult& res = (MatchResult&)msg;
	CURR_TV_AS_USEC(ts)
	res.setTimeStamp(ts);

	if(m_matchedItems.size()>0){
		if(ts - m_matchedItems[0].getTimeStamp() >= m_dualEyeMatchThresholdTime)
		{
			ClearMatchedItems();
		}
	}

	m_matchedItems.push_back(res);
	return true;
}

Copyable *MatchDispatcher::createNewQueueItem(){
	return new BinMessage(10);
}



void MatchDispatcher::SendResetMsg(){
	struct timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 50000;
	try{
		if(m_statusDestAddr){
			SocketClient client=m_socketFactory->createSocketClient("Eyelock.MasterSlaveCommSecure");
			client.SetTimeouts(timeOut);
			client.ConnectByHostname(*m_statusDestAddr); // DJH: retry=true
			client.Send(*m_ResetMsg,MSG_DONTWAIT);
		}
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
		std::cout<< "Unknown exception during SendResetMsg" <<endl;
	}
}
void MatchDispatcher::MakeNwF2FMsg(MatchResult* mr, BinMessage* msg) {
	char* ptr = msg->GetBuffer();
	memset(ptr, 0, 256);
	ptr[0] = 'F';
	ptr[1] = '2';
	ptr[2] = 'F';
	ptr[3] = ';';
	ptr[4] = PASSED;
	ptr[5] = ';';
	int len = -1, bits = -1;
	char* key = mr->getF2F(len, bits);
	len = MAX(len,0);
	len = len + 2;
	memcpy(ptr + 6, mr->getKey(), len);
	ptr[6 +len+1] = ';';
	msg->SetSize(6 + len + 1);
}

void MatchDispatcher::SendF2FMsg(MatchResult *mr){
	struct timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 500000;
	BinMessage msg(256);
	MakeNwF2FMsg(mr,&msg);

	try{
		if(m_weigandDestAddr){
			SocketClient client;
			//EyelockLog(logger, DEBUG, "Sending Weigand Msg %d::  %.*s to %s",4+len+1,msg.GetSize(),ptr,m_weigandDestAddr->GetOrigHostName());
			client.SetTimeouts(timeOut);
			client.ConnectByHostname(*m_weigandDestAddr); // DJH: retry=true
			client.Send(*m_weigandDestAddr,msg,MSG_DONTWAIT);
		}
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
		std::cout<< "Unknown exception during SendF2FMsg" <<endl;
	}

}

void MatchDispatcher::MatchDetected(MatchResult *result,int frindx,int eyeindx)
{
	uint64_t timestamp = result->getTimeStamp();
	int len = -1;
	string uid = result->getGUID();
    int personId = result->getEyeIndex() >> 1;
    uint64_t currtimestamp = timestamp;
    int64_t elapsedTime = currtimestamp - m_TimeofLastAuthorization;

    bool ismatch= false;
    if(m_checkUID){//Try to check for UID
    	if(uid.length()>0){
    		//EyelockLog(logger, DEBUG, "Curr UID %s Prev UID %s ",uid,m_prevUID.c_str());
    		int mem = memcmp(uid.c_str(),m_prevUID.c_str(),strlen(m_prevUID.c_str()));
			if(mem != 0){
				ismatch = true;
			}
    	}
    }
	else{
		if(personId != m_LastAuthorizationID){
			ismatch = true;
		}
	}

	if ((elapsedTime > m_RepeatAuthorizationPeriod)	|| ismatch)
	{
	    cout << "Best matched with record " << result->getEyeIndex() << " with score " << result->getScore()<< " @ time " << timestamp << endl;
	    int fr,ey;
	    string cam;
	    result->getFrameInfo(fr,ey,cam);
	    CURR_TV_AS_MSEC(t);
	    int nwval,mt;
	    result->getNwValandSleep(nwval,mt);
	    EyelockLog(logger, INFO, "%llu::MATCHED %d %d %s Matched timing %d msec\n",t,fr,ey,cam.c_str(),mt);
	   // printf("%llu::MATCHED %d %d %s Matched timing %d msec\n",t,fr,ey,cam.c_str(),mt);

		if(m_logging){
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer,a);
			FILE *fp = fopen("dump.txt","a");
			int le =0;
			fprintf(fp,"MATCH;%llu;%d;%f;%s;%d;%d;\n",a,result->getEyeIndex(),result->getScore(),result->getUID(le),frindx,eyeindx);
			fclose(fp);
		}

		EyelockLog(logger, DEBUG, "%llu > %llu || %d !=%d  [%d %f %d %d] ",elapsedTime,m_RepeatAuthorizationPeriod,personId,m_LastAuthorizationID,result->getEyeIndex(),result->getScore(),frindx,eyeindx);
  		m_LastAuthorizationID = personId;
		m_TimeofLastAuthorization = currtimestamp;
		m_mr.CopyFrom(*result);
		m_mr.setState(PASSED);

		EyelockLog(logger, DEBUG, "xMatchDIspatcher: M_mr eyeindex %d, result eyeindex %d", m_mr.getEyeIndex(), result->getEyeIndex());

		if(uid.length()>0){
			m_prevUID.assign(uid);
		}
		m_mr.setTimeStamp(timestamp);

		if(m_nwMatchMgr)
			m_nwMatchMgr->FlushLoitering();

		//if(m_weigandDestAddr){
			//SendF2FMsg(&m_mr);
		//}

		if(m_statusDestAddr)
			SendResetMsg();

		if(m_nwMatchMgr)
			m_nwMatchMgr->FlushQueue(false);

		if(m_imageProcessor)
			m_imageProcessor->enqueMsg(*m_ResetMsg);

		if(m_tsDestAddrpresent){// It is Nano TS Slave Master
			SendToMaster(&m_mr);
		}

		/*if(m_sdkDispatcher && (result->getState()==PASSED)){
			m_sdkDispatcher->enqueMsg(*result);
		}*/

		if(result->getEyeIndex() == -1){
			m_mr.setState(CONFUSION);
			//Call for Dispatcher.

		}
		callNext(m_mr);

	}
	else{
		if(m_Debug) EyelockLog(logger, DEBUG, "Repeat authorization period");
		if(m_nwMatchMgr)m_nwMatchMgr->FlushLoitering();
	}

}


void MatchDispatcher::SendToMaster(MatchResult *mr) {
	try{
		BinMessage msg(256);
		MakeNwF2FMsg(mr,&msg);
		if(m_Debug) {
			EyelockLog(logger, DEBUG, "MatchDispatcher->%d::%.*s",msg.GetSize(),msg.GetSize(),msg.GetBuffer());
		}
		if(m_resultDestAddr){
			SocketClient client=m_socketFactory->createSocketClient("Eyelock.NwLEDDispatcherSecure");
			client.SetTimeouts(m_timeOut);
			client.ConnectByHostname(*m_resultDestAddr);
			client.SetTimeouts(m_timeOut);
			client.Send(msg);
		}
	}
	catch(const char *msg){
		cerr <<"MatchDispatcher::"<<msg <<endl;
		EyelockLog(logger, ERROR, "Nw Error in MatchDispatcher");
	}
	catch(exception ex){
		cerr <<"MatchDispatcher::"<<ex.what()<<endl;
		EyelockLog(logger, ERROR, "exception::Nw Error in MatchDispatcher");
	}
	catch(Exception& ex){
		cerr <<"MatchDispatcher::";
		ex.PrintException();
		EyelockLog(logger, ERROR, "Exception::Nw Error in MatchDispatcher");
	}
}

bool MatchDispatcher::EnforceDualMatchPolicy()
{
	if(m_matchedItems.size() == 0 )
		return false;

	if(m_matchedItems.size() == 1 )
	{
		CURR_TV_AS_USEC(curr);
		if(curr - m_matchedItems.back().getTimeStamp() >= m_dualEyeMatchThresholdTime)
		{
			ClearMatchedItems();
			return false;
		}
	}

	//check if the matched eyes exceeds the threshold
	if(m_matchedItems.back().getTimeStamp() - m_matchedItems.front().getTimeStamp() >= m_dualEyeMatchThresholdTime)
	{
		ClearMatchedItems();
		return false;
	}


	char* userID1 = NULL;
	char* userID2 = NULL;
	int len = -1;
	int firstindx = -1;
	int secindx = -1;

	int matchSuccess = 0;
	bool foundEvenIdx = false;
	std::vector<MatchResult>::iterator iter = m_matchedItems.begin();
	for(;iter != m_matchedItems.end();iter++)
	{
		if((iter->getEyeIndex() & 0x1) == 0)
		{
			if((1 == matchSuccess) && (false == foundEvenIdx))
			{
				userID2 = iter->getUID(len);
				if(memcmp((void*)userID2,(void*)userID1,len) == 0)
				{
					matchSuccess = 2;
				}
				secindx = iter->getEyeIndex();
				break;
			}
			else if(0 == matchSuccess)
			{
				userID1 = iter->getUID(len);
				matchSuccess = 1;
				firstindx = iter->getEyeIndex();
			}
			foundEvenIdx = true;
		}
		else
		{
			if((1 == matchSuccess) && (true == foundEvenIdx))
			{
				userID2 = iter->getUID(len);
				if(memcmp((void*)userID2,(void*)userID1,len) == 0)
				{
					matchSuccess = 2;
				}
				secindx = iter->getEyeIndex();
				break;
			}
			else if(0 == matchSuccess)
			{
				userID1 = iter->getUID(len);
				matchSuccess = 1;
				firstindx = iter->getEyeIndex();
			}
		}
	}

	if((secindx != -1)&&(firstindx != -1)){
		EyelockLog(logger, DEBUG, "Got the Match for index %d %d ",firstindx,secindx);
	}

	return (2 == matchSuccess) ? true:false;
}
void MatchDispatcher::ClearMatchedItems()
{
	if(m_Debug){
		for(int i=0;i< (int)m_matchedItems.size();i++){
			EyelockLog(logger, DEBUG, "%d %llu %d ",i,m_matchedItems[i].getTimeStamp(),m_matchedItems[i].getEyeIndex());
		}
	}
	m_matchedItems.clear();

}

bool MatchDispatcher::GetOneResult(MatchResult& res)
{
	if(m_matchedItems.size() == 0)
		return false;
	res=m_matchedItems.front();
	return true;
}
void MatchDispatcher::ProcessMatchedItem()
{
	ScopeLock lock(m_matchLock);

	if(true == m_dualMatcherPolicy){
		bool tocheck= true;
		for(int i=0;i<m_matchedItems.size();i++){
			if(m_matchedItems[i].getState() == FAILED){
				tocheck = false;
				m_matchedItems.front() = m_matchedItems[i];
			}
			if((m_matchedItems[i].getState() == PASSED) && (m_matchedItems[i].GetPersonIrisInfo() == SINGLE)){
				m_matchedItems.front() = m_matchedItems[i];
				tocheck = false;
				break;
			}
		}
		if(tocheck){
			if(false == EnforceDualMatchPolicy())
				return;
		}
	}
	MatchResult result;
	if(false == GetOneResult(result)){
		return;
	}
	if(m_Debug){
		if(m_dualMatcherPolicy)
			EyelockLog(logger, DEBUG, "Dual eye match succeeded.");
		else
			EyelockLog(logger, DEBUG, "Single eye match succeeded.");
	}
	MatchDetected(&result);
	ClearMatchedItems();
}

unsigned int MatchDispatcher::MainLoop()
{
	std::string name = "Match Dispatcher";
	try{
		while (!ShouldIQuit()){
			ProcessMatchedItem();
			usleep(m_matchDispSleepusec);
			Frequency();
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
