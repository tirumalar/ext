/*
 * MasterSlaveNwListner.cpp
 *
 *  Created on: Mar 5, 2015
 *      Author: developer
 */

#include "MasterSlaveNwListner.h"
#include "Configuration.h"
#include "socket.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "MessageExt.h"
#include "LEDConsolidator.h"
#include "NwMatchManager.h"
#include "Synchronization.h"
#include "SocketFactory.h"
#include "logging.h"
#include "ImageProcessor.h"

using namespace std;
extern "C" {
#include "file_manip.h"
}
#include "NetworkUtilities.h"
#include "NwMatcherSerialzer.h"
#include "EyeDispatcher.h"

const char logger[30] = "MasterSlaveNwListner";
#define HBOX_PG

MasterSlaveNwListner::MasterSlaveNwListner(Configuration& conf):m_sleepAfterDispatchingEye(0),m_ledConsolidator(NULL)
,m_imageProcessor(NULL),m_nwMatchManager(NULL),m_pEyeDispatcher(NULL){
	m_port = conf.getValue("MasterSlaveNwListner.port",8083);
	m_debug = conf.getValue("MasterSlaveNwListner.Debug",false);
	CvSize cropSize;
	cropSize.width = conf.getValue("GRI.cropWidth", 640);
	cropSize.height = conf.getValue("GRI.cropHeight", 480);

	int HTTPMsgSize = HTTPPOSTMsg::calcMessageSize(cropSize.width,cropSize.height);
	m_HTTPMsg = new HTTPPOSTMsg(HTTPMsgSize);
  	m_pSockSrv = 0;
  	m_logging = conf.getValue("Eyelock.Logging", false);
  	m_socketFactory = new SocketFactory(conf);
  	m_sleepAfterDispatchingEye = conf.getValue("Eyelock.SleepAfterDispatchingEye",200000);
}

MasterSlaveNwListner::~MasterSlaveNwListner() {
	if (m_HTTPMsg)
		delete m_HTTPMsg;
	if (m_socketFactory)
		delete m_socketFactory;
}

void MasterSlaveNwListner::dispatchToLED(HTTPPOSTMsg& msg){
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

void MasterSlaveNwListner::HandleMessage(HTTPPOSTMsg& msg) {

	NWMESSAGETYPE msgType=msg.getMsgType();
	switch(msgType){
	case IMG_MSG:
		// This is to receive eyes from the slave in the single ip env ---Rajesh
		if (m_pEyeDispatcher){
			m_pEyeDispatcher->enqueMsg(msg);
			usleep(m_sleepAfterDispatchingEye);
		}
		break;
	case LED_MSG:
			dispatchToLED(msg);
		break;
	case RESET_EYELOCK_MSG:
			if (m_imageProcessor){
				m_imageProcessor->enqueMsg(msg);
			}
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

void MasterSlaveNwListner::onConnect(Socket& client, void *arg) {
	MasterSlaveNwListner *me = (MasterSlaveNwListner *) arg;
	if(me->do_serv_task(client)){
		client.CloseInput();
	}else{
		// since another thread is using this socket lets avoid closing it
		client.SetshouldClose(false);
	}
}

bool MasterSlaveNwListner::do_serv_task(Socket& client) {
	try {
		if(ShouldIQuit()) return true;
		client.Receive(*m_HTTPMsg);
		if(m_HTTPMsg->getMsgType() == MATCH_MSG){
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
		HandleMessage(*m_HTTPMsg);
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

extern void DeleteSocketStream(SocketServer *&s);

int MasterSlaveNwListner::End()
{
	EyelockLog(logger, DEBUG, "MasterSlaveNwListner::End()"); fflush(stdout);
	m_QuitStatus.lock(); m_QuitStatus.set(true); m_QuitStatus.unlock();
    {
    	SafeLock<SocketServer *> lock(m_pSockSrv);
    	if(m_pSockSrv.get()){
    		m_pSockSrv.get()->CloseInput();
    		m_pSockSrv.get()->CloseOutput();
    	}
    }
	EyelockLog(logger, DEBUG, "MasterSlaveNwListner::End() => HThread::End()"); fflush(stdout);
	HThread::End();
	EyelockLog(logger, DEBUG, "MasterSlaveNwListner::End() joined!!!"); fflush(stdout);
}

unsigned int MasterSlaveNwListner::MainLoop() {
#ifdef HBOX_PG
	return;
#endif
	std::string name = "MasterSlaveNwListner::";
	bool reset = false;
	while (!ShouldIQuit()) {
		if(m_debug)
			EyelockLog(logger, DEBUG, "%s Looping again!", name.c_str()); fflush(stdout);
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
					m_pSockSrv.get()= new SocketServer(m_socketFactory->createSocketServer("GRI.MasterSlaveNwListenerSecure",m_port));
				}
			}
			m_pSockSrv.get()->ShareAddress(true);
			m_pSockSrv.get()->Accept(onConnect, this);

			if(m_debug)
				EyelockLog(logger, DEBUG, "%s: processed connection", name.c_str()); fflush(stdout);
		}
		catch(Exception& ncex){
			EyelockLog(logger, ERROR, "MainLoop() exception: %d", ncex.GetError());
			cout <<name; cout.flush();
			ncex.PrintException();
			reset = true;
			sleep(1);
		}
		catch(const char* msg){
			EyelockLog(logger, ERROR, "MainLoop() exception: %s", msg);
			cout <<name<<msg <<endl;
			reset = true;
			sleep(1);
		}
	}
	EyelockLog(logger, DEBUG, "NwListener::MainLoop; CloserServer()"); fflush(stdout);
//	CloseServer();
	return 0;
}


