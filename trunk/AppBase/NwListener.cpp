/*
 * NwListener.cpp
 *
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include "NwListener.h"
#include "Configuration.h"
#include "MatchProcessor.h"
#include "socket.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "MessageExt.h"
#include "DBReceive.h"
#include "LEDConsolidator.h"
#include "F2FDispatcher.h"
#include "NwMatchManager.h"
#include "Synchronization.h"
#include "SocketFactory.h"
#include "logging.h"
#include "MT9P001FrameGrabber.h"
#include "ImageProcessor.h"
#include "MatchManagerInterface.h"

using namespace std;
extern "C" {
#include "file_manip.h"
}
#include "NetworkUtilities.h"
#include "NwMatcherSerialzer.h"
#include "EyeDispatcher.h"

const char logger[30] = "NwListener";

#define HBOX_PG
uint64_t  NwListener::GetFuteristicTime(int futSec)
{
    struct timeval curTime;
    gettimeofday(&curTime, 0);
    uint64_t temp = (curTime.tv_sec + futSec);
    temp = temp * 1000000u;
    temp += curTime.tv_usec;
    temp = temp/1000;
    return temp;
}

NwListener::NwListener(Configuration& conf)
:m_ledConsolidator(0)
,m_DBDispatcher(0)
,m_F2FDispatcher(0)
,m_DbTimeStamp(0)
,m_safetimeFor2000eyes(0)
,m_nwMatchManager(0)
,m_debug(0)
,m_logging(false)
,m_socketFactory(0)
,m_matchProcessor(0)
,m_sleepAfterDipatchingEye(0)
,m_frameGrabber(NULL)
,m_pIp(NULL)
{
	m_port = conf.getValue("server.port", 8081);
	m_debug = conf.getValue("NwListener.Debug",false);
	CvSize cropSize;
	cropSize.width = conf.getValue("GRI.cropWidth", 640);
	cropSize.height = conf.getValue("GRI.cropHeight", 480);

	int HTTPMsgSize = HTTPPOSTMsg::calcMessageSize(cropSize.width,cropSize.height);
	m_HTTPMsg = new HTTPPOSTMsg(HTTPMsgSize);

	m_ledMR.init(0,1.0f,0,0);

	m_f2fResult.init(0,1.0f,0,0);
	m_healthTimeOutMS=1000*conf.getValue("GRI.SecsBeforeSickSymptoms",10);
	m_safetimeFor2000eyes = conf.getValue("GRI.safetimeFor2000eyes",40);
	m_DBRxMsg = new DBRecieveMsg(true);
	// Initial time assuming with in 20 sec it will get HB
  	m_DbTimeStamp = GetFuteristicTime(20);
  	m_pSockSrv = 0;
  	m_logging = conf.getValue("Eyelock.Logging", false);
  	m_socketFactory = new SocketFactory(conf);

  	m_phpServerEnable = conf.getValue("Eyelock.PhpServerEnable", false);
  	m_sleepAfterDipatchingEye = conf.getValue("Eyelock.SleepAfterDispatchingEye", 50000);
  	m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);
}

NwListener::~NwListener() {
	if (m_HTTPMsg)
		delete m_HTTPMsg;
	if (m_DBRxMsg)
		delete m_DBRxMsg;
	if (m_socketFactory)
		delete m_socketFactory;

}

// an HeartBeat can be logged from any message recieved if it identifies the camera
void NwListener::LogHeartBeat(HTTPPOSTMsg& msg)
{
	ScopeLock lock(m_HBLock);

	char camID[32]={0};
	if (msg.getCameraID(camID)) {
		string sCamID(camID);
		if(m_debug)
			EyelockLog(logger, DEBUG, "NwListener::LogHeartBeat(): %s", sCamID.c_str()); fflush(stdout);
		m_healthStatus[sCamID] = GetFuteristicTime();
	}

	/*
	if(m_debug)
	{
		for(std::map<std::string, uint64_t>::iterator iter = m_healthStatus.begin(); iter != m_healthStatus.end(); iter++)
		{
			EyelockLog(logger, DEBUG, "heartbeat[%s] = %lu", (*iter).first.c_str(), (*iter).second);
		}
	}*/
}

bool NwListener::IsHealthy(uint64_t curTime)
{
	ScopeLock lock(m_HBLock);

	if(m_healthStatus.size()==0) return false;
	std::map<string,uint64_t>::iterator it = m_healthStatus.begin();
	uint64_t minval = it->second;
	for(;it != m_healthStatus.end(); it++)
	{	if(it->second < minval)
			minval = it->second;
	}
	if(m_debug)
		EyelockLog(logger, DEBUG, "%s :: %lu %lu %ld ",getName(),curTime,minval,curTime-minval);
	if((curTime-minval) >m_healthTimeOutMS) return false;
	return true;
} //commitcomment

void NwListener::AddMessageHandler(HTTPPostMessageHandler *pHandler)
{
	m_MessageHandlers.push_back(pHandler);
}

void NwListener::SetCalibration(HTTPPOSTMsg& msg){
	int nwset,flashtime,triggertime,led;
	bool ret = msg.getCalibrationData(nwset,flashtime,triggertime,led);
	if(m_frameGrabber && ret){
		printf("Setting Calibration NwSet %d FlashTime %d TriggerTime %d illuminator %d \n", nwset,flashtime,triggertime,led);
		m_frameGrabber->SetExposureandIlluminator(nwset?true:false,flashtime,triggertime,led);
		//if(m_pIp)m_pIp->setShouldDetectEyes(nwset?false:true);
		if(m_pEyeDispatcher) m_pEyeDispatcher->SetSecureComm(false);
	}
}

void NwListener::HandleMessage(HTTPPOSTMsg& msg) {

	NWMESSAGETYPE msgType=msg.getMsgType();
	switch(msgType){
//	case IMG_MSG:
//		// This is to receive eyes from the slave in the single ip env ---Rajesh
//		if (m_pEyeDispatcher){
//			m_pEyeDispatcher->enqueMsg(msg);
//			usleep(m_sleepAfterDipatchingEye);
//		}
//		break;
	case RELOADDB_MSG:
		callNext(msg);
		break;
	case F2F_MSG:
		UpdateF2F(msg);
		break;
	case LED_MSG:
		dispatchToLED(msg);
		break;
		// do not break;
	case HB_MSG:
		// do nothing
		if(m_debug)
			EyelockLog(logger, DEBUG, "InCmg Msg %.*s",30,msg.GetBuffer());
		break;
	case RESET_EYELOCK_MSG:
			if (m_imageProcessor){
				m_imageProcessor->enqueMsg(msg);
			}
			break;

	case ENABLETHREADPROPERTIES:
			SetFreq(msg.getThreadMessageType());
			break;
	case CALIBRATION_MSG:
		SetCalibration(msg);
		break;
	default:
		cerr<<"Unknown MessageType "<<endl;
		if(m_logging){
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer,a);
			FILE *fp = fopen("dump.txt","a");
			int le =0;
			fprintf(fp,"%.*s;%llu;\n",50,msg.GetBuffer(),a);
			fclose(fp);
		}

		break;
	}
}

void NwListener::UpdateF2F(HTTPPOSTMsg& msg){
	int type= -2;
	char *ptr = msg.getF2F(type);
	EyelockLog(logger, DEBUG, "Got F2F %d ",type);
	if(type == PASSED){
		LEDResult l;
		l.setState(LED_PASSED);
		l.setGeneratedState(eREMOTEGEN);
		if(m_ledConsolidator)m_ledConsolidator->enqueMsg(l);
	}

	if(ptr){
		m_f2fResult.setF2F(ptr);
	}
	else
	{
		if(m_debug)
			EyelockLog(logger, DEBUG, "NwListener::UpdateF2F::could not get F2F data from msg");
	}
	if (m_F2FDispatcher){
		m_f2fResult.setState((MatchResultState)type);
		m_F2FDispatcher->enqueMsg(m_f2fResult);
	}

}


void NwListener::dispatchToLED(HTTPPOSTMsg& msg){
	LedState state=(LedState)msg.getLEDMessageType();
	LEDResult l;
	l.setState(state);
	l.setGeneratedState(eREMOTEGEN);
	if(state == LED_NWSET){
		int value=0,sleeptime=0;
		msg.getLEDMessageData(value,sleeptime);
		l.setNwValandSleep(value,sleeptime);
	}
	if(m_ledConsolidator)
		m_ledConsolidator->enqueMsg(l);
}

void NwListener::onConnect(Socket& client, void *arg) {
	NwListener *me = (NwListener *) arg;
	if(me->do_serv_task(client))
	{
		client.CloseInput();
	}
	else
	{
		// since another thread is using this socket lets avoid closing it
		client.SetshouldClose(false);
	}
}


bool NwListener::HandleReceiveDB(Socket& client)
{
	// initialize a dbRcvMsg using the code below
	// enqueue it to the dbRcv processor
	// if enque is successful return false=>
	//		meaning we should not close this socket
	// else return true;
	if(!m_DBDispatcher) return true; //Close the socket
	int preludeSize=0,dbFileSize=0;
	char ip[1024]={0};

	if(m_HTTPMsg->isSqlite()){
		m_DBRxMsg->m_msgType = eSQLITEDB;
		m_DBRxMsg->m_prelude = m_HTTPMsg->getSqliteServerIPAndDataAndPreludeSize(ip,preludeSize,dbFileSize);
		m_DBRxMsg->m_IpPort.assign(ip,strlen(ip));
	}else if(m_HTTPMsg->isUpdateUsr() || m_HTTPMsg->isEncUpdateUsr()){
		m_DBRxMsg->m_msgType = eUPDATEDB;
		m_DBRxMsg->m_prelude=m_HTTPMsg->getDBPrelude(preludeSize,dbFileSize);
	}else if(m_HTTPMsg->isDeleteUsr()){
		m_DBRxMsg->m_msgType = eDELETEDB;
		m_DBRxMsg->m_prelude = m_HTTPMsg->getServerIPAndDataAndPreludeSize(ip,preludeSize,dbFileSize);
		m_DBRxMsg->m_IpPort.assign(ip,strlen(ip));
	}else{
		m_DBRxMsg->m_msgType = eREPLACEDB;
		m_DBRxMsg->m_prelude=m_HTTPMsg->getDBPrelude(preludeSize,dbFileSize);
	}
	m_DBRxMsg->m_preludeSize = preludeSize;
	m_DBRxMsg->m_FileSize = dbFileSize;
	m_DBRxMsg->m_SD = client.GetSD();
	m_DBRxMsg->m_SecureTrait = client.GetST();
	if (m_HTTPMsg->isEncUpdateUsr() || m_HTTPMsg->isRecvEncDB())
		m_DBRxMsg->m_isEncrypt = 1;
	else
		m_DBRxMsg->m_isEncrypt = 0;
	EyelockLog(logger, DEBUG, "HandleReceiveDB(): msgType %d. Length %d, m_isEncrypt %d",m_DBRxMsg->m_msgType, dbFileSize, m_DBRxMsg->m_isEncrypt);
	EyelockLog(logger, DEBUG, "Prelude %d",preludeSize);

	struct timeval curTime;
	gettimeofday(&curTime, 0);
	int numsec = (int)((dbFileSize/5000.0)*(m_safetimeFor2000eyes/1000.0)+0.5);
  	m_DbTimeStamp = GetFuteristicTime(numsec);
  	EyelockLog(logger, DEBUG,"Added Time %d %llu",numsec,m_DbTimeStamp);

	if(m_DBDispatcher->enqueMsg(*m_DBRxMsg))
		return false; //Do not close socket
	else
		return true; //Close the socket
}
// called when there is a connection
bool NwListener::do_serv_task(Socket& client) {
	try {

		if(ShouldIQuit()) return true;

		if(m_debug)EyelockLog(logger, DEBUG, "NwListener::do_serv_task - msgType %d", m_HTTPMsg->getMsgType()); fflush(stdout);
		client.Receive(*m_HTTPMsg);
		//Handle RECEIVE DB SEPERATELY
	   if ((m_HTTPMsg->getMsgType() == RECVDB_MSG)||(m_HTTPMsg->getMsgType() == UPDATEUSR)||(m_HTTPMsg->getMsgType() == RECEIVESQLITE)){
			EyelockLog(logger, DEBUG, "DB msg %d", m_HTTPMsg->getMsgType());
			if (m_transTOC)
				return true;
			if(m_matchProcessor){
				if(m_matchProcessor->GetPCMatcher())
					return true;
			}
			if (m_pIp){
				if(m_pIp->getShouldDetectEyes() == false){
					EyelockLog(logger, DEBUG, "No license installed");
					return true;
				}
			}
			bool test = HandleReceiveDB(client);
			return test;
		}
		else if(m_HTTPMsg->getMsgType() == MATCH_MSG){
			int id=0,taskid=0;
			if(m_nwMatchManager){
				char *ptr = NULL;
				int size=0;
				bool test=m_HTTPMsg->getMatchIris(&ptr,id,taskid,size);
				if(m_debug)EyelockLog(logger, DEBUG, "ID %d TaskID %d Size %d ",id,taskid,size);

				BinMessage Matchack(512);
				const char *format = "MATCHRESULT;%d;%d;%d;%0.6f;";
				int len = sprintf(Matchack.GetBuffer(), format, id, taskid,-1, 1.0);
				Matchack.SetSize(len + 2);
				client.Send(Matchack);
				int val = NwMatcherSerialzer::getBitStreamSize(ptr);
				if((test)&&(size > 0)&&(val >0)){
					BinMessage msg(ptr,val);
					m_nwMatchManager->enqueMsg(msg);
				}else
					 return true;
				return true;
			}
		}
		else if(m_HTTPMsg->getMsgType() == PING_MSG){
			int id=0,startindx=0,numeyes=0;
			BinMessage Matchack(32);
	    	const char *format = "PONG;%d;%d;%d;";
	        int len = sprintf(Matchack.GetBuffer(),format, id,startindx,numeyes);
	        Matchack.SetSize(len);
	        client.Send(Matchack);
	        return true;
		}
		else if(m_HTTPMsg->getMsgType() == GET_VERSION_MSG){
			EyelockLog(logger, DEBUG, "GET_VERSION_MSG msg: %s",m_version);
			BinMessage Matchack(32);
	    	const char *format = "%s;";
	    	int len = sprintf(Matchack.GetBuffer(),format,  m_version);
	        Matchack.SetSize(len);
	        client.Send(Matchack);
	        EyelockLog(logger, DEBUG, "Sending version number for GET_VERSION_MSG %d, %s",len,Matchack.GetBuffer());
	        return true;
		}
		else if(m_HTTPMsg->getMsgType() == GETUSERCOUNT_MSG){
			//EyelockLog(logger, DEBUG, "GETUSERCOUNT_MSG msg: %s",m_version);
			//SQLITE QUERY GO!
			//GET_USER_COUNT

			//EyelockLog(logger, DEBUG, "Requested records count");
			int count = 0;
			if (m_nwMatchManager != NULL)
			{
				count = m_nwMatchManager->GetUserCount(true);
				if (count == -1)
				{
					EyelockLog(logger, ERROR, "Cannot retrieve record count from NwMatchManager");
					count = 0;
				}
			}
			else
			{
				EyelockLog(logger, ERROR, "Cannot access NwMatchManager");
			}
			BinMessage Matchack(32);
			const char *format = "%d;";
			int len = sprintf(Matchack.GetBuffer(),format,  count);
			Matchack.SetSize(len);
			client.Send(Matchack);
			return true;
		}
		else if(m_HTTPMsg->getMsgType() == GETACS){
			EyelockLog(logger, DEBUG, "GETACS msg");
			char *acsString = getACSDisplayTestData();
			BinMessage Matchack(32);
			const char *format = "%s;";
			int len = sprintf(Matchack.GetBuffer(),format, acsString);
			Matchack.SetSize(len);
			client.Send(Matchack);
			EyelockLog(logger, DEBUG, "Sending ACS string for GETACS %d, %s\n",len,Matchack.GetBuffer());
			if(acsString){
				delete [] acsString;
			}
			return true;
		}
		else if (m_HTTPMsg->getMsgType() == TESTACS)
		{
			int bytes, bitlen;
			char * testData = getACSTestData(bytes, bitlen);
			//transmit the data
			if(m_nwMatchManager){
				//TODO: send the message
				F2FDispatcher * mf2f = m_nwMatchManager->GetF2FDispatcher();
				mf2f->m_testCode = true;
				//m_nwMatchManager->
				MatchResult res;
				res.setF2F(testData);
				res.setState(PASSED);
				mf2f->enqueMsg(res);
			}
			if(testData) delete [] testData;

		}
		else if (m_HTTPMsg->getMsgType() == TIMESYNC_MSG)
		{
			if(m_nwMatchManager){
				F2FDispatcher * mf2f = m_nwMatchManager->GetF2FDispatcher();
				mf2f->settime();
			}
		}
		else if (m_HTTPMsg->getMsgType() == LOCATEDEVICE_MSG)
		{
			if(m_nwMatchManager){
				m_nwMatchManager->GetF2FDispatcher()->LocateDevice();
			}
		}
		else if (m_HTTPMsg->getMsgType() == TESTMATCH)
		{
			if(m_nwMatchManager){
				bool ret;
				char *address =m_HTTPMsg->getNwMatchIpaddr();
				if(m_debug)EyelockLog(logger, DEBUG, "TESTMATCH address %s ",address);
				MatchManagerInterface * mmi = m_nwMatchManager->GetMM();
				ret = mmi->TestPCMatcher(address);

				BinMessage Matchack(32);
				const char *format = "%d";
				int len = sprintf(Matchack.GetBuffer(),format, ret);
				Matchack.SetSize(len);
				client.Send(Matchack);
				EyelockLog(logger, DEBUG, "Sending test PC match result for TESTMATCH length %d, value %s\n",len,Matchack.GetBuffer());
				return true;
			}
		}
		else if(m_HTTPMsg->getMsgType() == UNKNOWN_MSG){
			EyelockLog(logger, DEBUG, "UNKNOWN_MSG");
			BinMessage Matchack(32);
	    	const char *format = "%s;";
	        int len = sprintf(Matchack.GetBuffer(), format, "error: unknown message");
	        Matchack.SetSize(len);
	        client.Send(Matchack);
	        return true;
		}
		LogHeartBeat(*m_HTTPMsg);
		HandleMessage(*m_HTTPMsg);

		// Generic message handlers:
		for(std::vector<HTTPPostMessageHandler *>::iterator iter = m_MessageHandlers.begin(); iter != m_MessageHandlers.end(); iter++)
		{
			(*iter)->Handle(*m_HTTPMsg, &client);
		}
		return true;

	} catch (NetIOException nex) {
		EyelockLog(logger, ERROR, "do_serv_task() exception: %d", nex.GetError());
		nex.PrintException();
	}
	catch (const char *msg) {
		EyelockLog(logger, ERROR, "do_serv_task() exception: %s", msg);
		cerr <<msg <<endl;
	}
	return true;
}

void DeleteSocketStream(SocketServer *&s)
{
	if(s != NULL)
	{
		s->CloseInput();
		s->CloseOutput();
		delete s;
		s = 0;
	}
}

void NwListener::CloseServer()
{
	// NOOP
}

int NwListener::End()
{
	EyelockLog(logger, DEBUG, "NwListener::End()"); fflush(stdout);
	m_QuitStatus.lock(); m_QuitStatus.set(true); m_QuitStatus.unlock();
    {
#ifndef HBOX_PG
    	SafeLock<SocketServer *> lock(m_pSockSrv);
    	if(m_pSockSrv.get())
    	{
    		m_pSockSrv.get()->CloseInput();
    		m_pSockSrv.get()->CloseOutput();
    	}
#else
    	if(m_pSockSrv)
    	{
    		m_pSockSrv->CloseInput();
    	    m_pSockSrv->CloseOutput();
    	}
#endif
    }

   // if (phpThread)
    	//pthread_join (phpThread, NULL);

	EyelockLog(logger, DEBUG, "NwListener::End() => HThread::End()"); fflush(stdout);
	HThread::End();
	EyelockLog(logger, DEBUG, "NwListener::End() joined!!!"); fflush(stdout);
}

void onConnectPhp(Socket& client, void *arg) {
	NwListener *me = (NwListener *) arg;
	if(me->do_serv_task(client))
	{
		client.CloseInput();
	}
	else
	{
		// since another thread is using this socket lets avoid closing it
		client.SetshouldClose(false);
	}
}
void * phpServer(void *arg)
{
	long int tid = syscall(SYS_gettid);
	printf("***************%s thread %u ********************* \n",phpServer,tid);
	NwListener *me = (NwListener *) arg;
	SocketServer *phpSockSrv = 0;
	EyelockLog(logger, DEBUG, "phpServer(): start!"); fflush(stdout);
	while (!me->ShouldIQuit()) {
		if(me->m_debug)
			EyelockLog(logger, DEBUG, "phpServer(): Looping again!"); fflush(stdout);

		try	{
			if(!phpSockSrv)
			{
				EyelockLog(logger, DEBUG, "phpServer(): SocketServer Create!"); fflush(stdout);
				phpSockSrv= new SocketServer(8085);
			}

			phpSockSrv->Accept(onConnectPhp, arg);

			//EyelockLog(logger, DEBUG, "phpServer(): processed connection"); fflush(stdout);
		}
		catch(Exception& ncex){
			EyelockLog(logger, ERROR, "phpServer() exception: %d", ncex.GetError());
			cout <<"phpServer()"; cout.flush();
			ncex.PrintException();
			break;

		}
		catch(const char* msg){
			EyelockLog(logger, ERROR, "phpServer() exception: %s", msg);
			cout <<"phpServer()"<<msg <<endl;
			break;
		}
	}
	sleep(1);
	DeleteSocketStream(phpSockSrv);
	EyelockLog(logger, DEBUG, "NwListener::phpServer(): exit"); fflush(stdout);

	return NULL;

}

unsigned int NwListener::MainLoop() {

	std::string name = "NwListener::";

	bool reset = false;

	if(m_phpServerEnable){
		// create a new non-secure server for Webconfig php
		if (pthread_create (&phpThread, NULL, phpServer, this)) {
			EyelockLog(logger, ERROR, "MainLoop(): Error creating thread phpServer");
			fprintf(stderr, "Error creating thread phpServer\n");
		}
	}

	while (!ShouldIQuit()) {
		if(m_debug)
			EyelockLog(logger, DEBUG, "%s Looping again!", name.c_str()); fflush(stdout);
#ifndef HBOX_PG
		try{
			{
				SafeLock<SocketServer *> lock(m_pSockSrv);
				if(reset)
				{
					EyelockLog(logger, INFO, "%s SocketServer Delete!", name.c_str()); fflush(stdout);
					DeleteSocketStream(m_pSockSrv.get());
					reset = false;
				}
				if(!m_pSockSrv.get())
				{
					EyelockLog(logger, INFO, "%s SocketServer Create!", name.c_str()); fflush(stdout);
					m_pSockSrv.get()= new SocketServer(m_socketFactory->createSocketServer("GRI.NwListenerSecure",m_port));
				}

			}
			m_pSockSrv.get()->ShareAddress(true);
			m_pSockSrv.get()->Accept(onConnect, this);

			if(m_debug)
				EyelockLog(logger, DEBUG, "%s: processed connection", name.c_str()); fflush(stdout);
		}
#else
		try{
			// printf("Inside try of NwListener\n");
					{
						// SafeLock<SocketServer *> lock(m_pSockSrv);
						if(reset)
						{
							EyelockLog(logger, INFO, "%s SocketServer Delete!", name.c_str()); fflush(stdout);
							DeleteSocketStream(m_pSockSrv);
							reset = false;
						}
						if(!m_pSockSrv)
						{
							// printf("m_port...%d\n", m_port);
							// printf("Inside m_pSockSrv of NwListener\n");
							EyelockLog(logger, INFO, "%s SocketServer Create!", name.c_str()); fflush(stdout);
							m_pSockSrv = m_socketFactory->createSocketServer2("GRI.NwListenerSecure",m_port);
							// perror("Error in Socket creation in NwListener\n");
						}


					}
					m_pSockSrv->ShareAddress(true);
					m_pSockSrv->Accept(onConnect, this);

					if(m_debug)
						EyelockLog(logger, DEBUG, "%s: processed connection", name.c_str()); fflush(stdout);
				}

#endif
		catch(Exception& ncex){
			EyelockLog(logger, ERROR, "MainLoop() exception: %d", ncex.GetError());
			cout <<name; cout.flush();
			ncex.PrintException();
			reset = true;
			//sleep(1);
		}
		catch(const char* msg){
			EyelockLog(logger, ERROR, "MainLoop() exception: %s", msg);
			cout <<name<<msg <<endl;
			reset = true;
			//sleep(1);
		}
	}

	EyelockLog(logger, DEBUG, "NwListener::MainLoop; CloserServer()"); fflush(stdout);

	CloseServer();

	return 0;
}

