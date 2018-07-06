/*
 * NwLEDDispatcher.cpp
 *
 *  Created on: Jan 28, 2013
 *      Author: mamigo
 */
#include "NwLEDDispatcher.h"
#include "SocketFactory.h"
#include <iostream>
#include <stdio.h>

NwLEDDispatcher::NwLEDDispatcher(Configuration& conf):GenericProcessor(conf),m_outMsg(64) {
	m_ledstr = (char*)conf.getValue("GRI.LEDSetStr","LEDSET");
	m_resultDestAddr[0] = NULL;
	m_resultDestAddr[1] = NULL;
	// don't try to resolve on first attempt
	const char *svrAddr = conf.getValue("Eyelock.TSMasterDestAddr", "NONE");
	if(strcmp(svrAddr,"NONE") == 0){
		m_tsDestAddrpresent = false;
		svrAddr = conf.getValue("Eyelock.TSSlaveAddr0", "NONE");
		if(strcmp(svrAddr,"NONE") != 0)
			m_resultDestAddr[0] = HostAddress::MakeHost(svrAddr, eIPv4, false);
		svrAddr = conf.getValue("Eyelock.TSSlaveAddr1", "NONE");
		if(strcmp(svrAddr,"NONE") != 0)
			m_resultDestAddr[1] = HostAddress::MakeHost(svrAddr, eIPv4, false);

		int timeOutms = conf.getValue("Eyelock.TSSlaveSocketTimeOutMillis", 200);
		m_timeOut.tv_sec = timeOutms / 1000;
		m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
	}else{
		m_tsDestAddrpresent = true;
		m_resultDestAddr[0] = HostAddress::MakeHost(svrAddr, eIPv4, false);
		int timeOutms = conf.getValue("Eyelock.TSMasterSocketTimeOutMillis", 200);
		m_timeOut.tv_sec = timeOutms / 1000;
		m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
	}
	m_socketFactory = new SocketFactory(conf);
	m_debug =  conf.getValue("Eyelock.NwLEDDispatcherDebug", false);
}

NwLEDDispatcher::~NwLEDDispatcher() {

	for(int i=0;i<2;i++){
		if(m_resultDestAddr[i])
			delete m_resultDestAddr[i];
	}

	if(m_socketFactory)
		delete m_socketFactory;
}

int NwLEDDispatcher::getQueueSize(Configuration* conf){
	return conf->getValue("GRI.NwLEDDispatcherQSize",15);
}

Copyable *NwLEDDispatcher::createNewQueueItem(){
	return new LEDResult;
}


void NwLEDDispatcher::process(Copyable* msg) {
	try{
		LEDResult *l = (LEDResult *)msg;
		char *msgBuff=m_outMsg.GetBuffer();
		int val=-1,time= -1;
		l->getNwValandSleep(val,time);
		int len=snprintf(msgBuff,m_outMsg.GetAvailable(),"%s;%d;%d;%d;",m_ledstr,l->getState(),val,time);
		m_outMsg.SetSize(len);
		if(m_debug) {
			printf("NwLEDDispatcher->%d::%.*s\n",m_outMsg.GetSize(),m_outMsg.GetSize(),m_outMsg.GetBuffer());
		}
		for(int i=0;i<2;i++){
			if(m_resultDestAddr[i]){
				if(m_debug)
					printf("%d -> %s \n",i,m_resultDestAddr[i]->GetOrigHostName());
				SocketClient client=m_socketFactory->createSocketClient("Eyelock.NwLEDDispatcherSecure");
				client.SetTimeouts(m_timeOut);
				client.ConnectByHostname(*(m_resultDestAddr[i]));
				client.SetTimeouts(m_timeOut);
				client.Send(m_outMsg);
			}
		}
	}
	catch(const char *msg){
		cerr <<"NwLEDDispatcher::"<<msg <<endl;
		printf("Nw Error in NwLEDDispatcher\n");
	}
	catch(exception ex){
		cerr <<"NwLEDDispatcher::"<<ex.what()<<endl;
		printf("exception::Nw Error in NwLEDDispatcher\n");
	}
	catch(Exception& ex){
		cerr <<"NwLEDDispatcher::";
		ex.PrintException();
		printf("Exception::Nw Error in NwLEDDispatcher\n");
	}
}
