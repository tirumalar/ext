/*
 * NwDispatcher.cpp
 *
 *  Created on: 17-Sep-2009
 *      Author: mamigo
 */
#include "SocketFactory.h"
#include "NwDispatcher.h"
#include "Configuration.h"
#include <iostream>
#include <stdio.h>
extern "C"{
#include "file_manip.h"
}
using namespace std;


NwDispatcher::NwDispatcher(Configuration& conf):ResultDispatcher(conf),m_outMsg(NW_MATCHMSG_MAXLEN) {
	// derived class
	const char *svrAddr = conf.getValue("GRI.MatchResultDestAddr", "192.168.10.103:8081");
	// don't try to resolve on first attempt
	m_resultDestAddr= HostAddress::MakeHost(svrAddr, eIPv4, false);
	int timeOutms = conf.getValue("GRI.socketTimeOutMillis", 5000);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
	m_fileName = conf.getValue("GRI.NwMsgFile", (const char *)NULL);
	m_msgFormat=conf.getValue("GRI.MatchResultNwMsgFormat","Matched:%d;Score:%0.4f;%llu;");
	m_nwFormat=conf.getValue("GRI.FeatureVarianceNwMsgFormat","UnAuthorised Access:%f;%llu;");
    m_type = (NwMsgType) conf.getValueIndex("GRI.MatchResultNwMsgType", UID,RAW,UID,"UID","F2F","RAW");
	printf("Value index : %d\n", m_type);
	m_debug = conf.getValue("GRI.NwDebug",false);
	if(m_debug){
		printf("Debug enabled for NW Dispatcher\n");
		printf("Type::%d\n",m_type);
	}

	m_DLFileMsg = m_fileName ? new FileMsg(m_fileName,1000) : 0;
	m_timeCount = 5;//sec
	m_socketFactory = new SocketFactory(conf);

}

int NwDispatcher::getQueueSize(Configuration* conf){
	return conf->getValue("GRI.NwDispatcherQSize",15);
}

NwDispatcher::~NwDispatcher() {
	if(m_resultDestAddr)
		delete m_resultDestAddr;
	if(m_DLFileMsg)
		delete m_DLFileMsg;
	if(m_socketFactory)
		delete m_socketFactory;
}


void NwDispatcher::process(MatchResult *mr) {
	static int idReady = 0;
	static char myID[10];
#ifndef HBOX_PG
	if(!(mr->getState()==PASSED || mr->getState()==FAILED || mr->getState()==CONFUSION)) {
#else	
	if(!(mr->getState()==PASSED ||mr->getState()==FAILED)) {
#endif	
		cerr<<"NwDispatcher::Warning: Only PASSED results should reach NWResultDispatcher"<<endl;
		return;
	}
	try{
		char *msgBuff=m_outMsg.GetBuffer();
		if(mr->getState() == PASSED){

			int idlen=0,bitlen=0;
			if(m_debug) {
				printf("NW::process m_outMsg available %d\n", m_outMsg.GetAvailable());
			}
			int len=snprintf(msgBuff,m_outMsg.GetAvailable(),m_msgFormat,mr->getEyeIndex(),mr->getScore(),mr->getTimeStamp());
			m_outMsg.SetSize(len);
			if(m_debug){
				printf("NW::process wrote %d bytes to message, %d available, msg is %s\n", len,m_outMsg.GetAvailable(), msgBuff);
			}

			if(m_type == UID){
				//char * uid=mr->getUID(idlen);
				string guid = mr->getGUID();
				if(m_debug)printf("UID::(%d) %s\n",guid.length(),guid.c_str());
				if(guid.length()>0){
					m_outMsg.Append("ID:",3);
					m_outMsg.Append(guid.c_str(),guid.length());
					len = len+3;
				}
			}
			else if(m_type == F2F){
				char * uid=mr->getF2F(idlen,bitlen);
//				if(m_debug)printf("F2F::(%d) %s\n",idlen,uid);
				if(idlen>0){
					m_outMsg.Append("F2F:",4);
					m_outMsg.Append(uid,idlen);
					len = len+4;
				}
			}
			else if(m_type == RAW){
				//char * uid=mr->getUID(idlen);
				std::string uidStr = mr->getName();
				const char *uid = uidStr.c_str();
				idlen = strlen(uid); //getUID seems to truncate IDs to 128 length.  Need to find out why.
				//in the mean time this will do the job.  The whole ID gets returned so there shouldn't be a problem.
				if(m_debug)printf("RAW::(%d) %s\n",idlen,uid);
				if(idlen>0){
					m_outMsg.Append(uid,idlen);
				}
			}

			// Append the device ID
			// Matched:0;Score:0.1544;Time:1434554054471557;ID:Rajesh  Tirumala|37eb87c5-1e52-4726-80b4-ebc6a0d50ea3;DeviceID:234|ATH-100;
			if (!idReady) {
				strcpy(myID, "0000");
#ifndef HBOX_PG
				FILE *fp = fopen("/home/root/id.txt","r");
#else								
				FILE *fp = fopen("id.txt","r");
#endif				
				if(fp == NULL){
					printf("Unable to open device ID file\n");
				} else {
				      char *p = fgets(myID, 10, fp);
				      if (p != NULL) {
				           myID[strlen(myID)-1] = '\0';
				           idReady = 1;
				      }
				}
				if(fp)
					fclose(fp);
			}

			m_outMsg.Append(";DeviceID:", 10);
			m_outMsg.Append(myID, strlen(myID));
			m_outMsg.Append("|ATH-100", 8);

			if(0){
				printf("NWMsg:Len:%d:: %.*s\n",m_outMsg.GetSize(),len,msgBuff);
				msgBuff+=len;
				for(int i=0;i<idlen;i++){
					char* fmt = "%02x ";
					if(m_debug == 2){
						fmt = "%c";
					}
					printf(fmt,(unsigned char)(*msgBuff++) );
				}
				printf("\n");
			}
		}else{
			int len=snprintf(msgBuff,m_outMsg.GetAvailable(),m_nwFormat,mr->getVar(),mr->getTimeStamp());
			m_outMsg.SetSize(len);
		}

		char temp[3]={0};
		temp[0]=';';
		temp[1]='\n';
		temp[2]='\0';
		m_outMsg.Append(temp,2);
		if(m_debug) {
			printf("NW->%d::%.*s\n",m_outMsg.GetSize(),m_outMsg.GetSize(),m_outMsg.GetBuffer());
		}

		int fsz = m_fileName ? FileSize(m_fileName) : 0; // Don't bother checking if filename is not specified
		if(m_debug) printf("File Sz %d \n",fsz);

		//printf("m_resultDestAddr = %d\n", m_resultDestAddr); fflush(stdout);
		if(m_resultDestAddr){
			SocketClient client=m_socketFactory->createSocketClient("GRI.NwDispatcherSecure");
			client.SetTimeouts(m_timeOut);
			client.ConnectByHostname(*m_resultDestAddr); // DJH: retry=true
			if(fsz){
				client.SendChunk(*m_DLFileMsg);
				if(0 == remove(m_fileName))
					printf("Sent and Deleted file\n");
			}
			client.Send(m_outMsg);
			if(m_debug){
				printf("Sent Nw MSG\n");
			}
		}
	}
	catch(const char *msg){
		cerr <<"NwDispatcher::"<<msg <<endl;
		printf("Nw Error in dispatcher Appending to file\n");
		AppendFile(&m_outMsg);
	}
	catch(exception ex){
		cerr <<"NwDispatcher::"<<ex.what()<<endl;
		printf("exception::Nw Error in dispatcher Appending to file\n");
		AppendFile(&m_outMsg);
	}
	catch(Exception& ex){
		cerr <<"NwDispatcher::";
		ex.PrintException();
		printf("Exception::Nw Error in dispatcher Appending to file\n");
		AppendFile(&m_outMsg);
	}
}

void NwDispatcher::ProcessOnEmptyQueue(){

	timeval timeuSec;
	gettimeofday(&timeuSec,0);
	if(m_timeCount > timeuSec.tv_sec)
		return;
	m_timeCount = timeuSec.tv_sec + 5;

	try{
		int fsz = FileSize(m_fileName);
		if(m_debug) printf("ProcessOnEmptyQueue::File Sz %d \n",fsz);

		if(fsz&&m_resultDestAddr){
			SocketClient client=m_socketFactory->createSocketClient("GRI.NwDispatcherSecure");
			client.SetTimeouts(m_timeOut);
			client.ConnectByHostname(*m_resultDestAddr); // DJH: retry=true
			client.SendChunk(*m_DLFileMsg);
			if(0 == remove(m_fileName))
				printf("Sent and Deleted file\n");
		}
	}
	catch(const char *msg){
		cerr <<"NwDispatcher::"<<msg <<endl;
		printf("Nw Error in dispatcher Appending to file\n");
	}
	catch(exception ex){
		cerr <<"NwDispatcher::"<<ex.what()<<endl;
		printf("exception::Nw Error in dispatcher Appending to file\n");
	}
	catch(Exception& ex){
		cerr <<"NwDispatcher::";
		ex.PrintException();
		printf("Exception::Nw Error in dispatcher Appending to file\n");
	}
}


void NwDispatcher::AppendFile(BinMessage* out){
	FILE *fp = fopen(m_fileName,"a");
	if(fp == NULL){
		printf("Unable To create Nw Dispatcher file\n");
		return;
	}
	int ret = fwrite(out->GetBuffer(),1,out->GetSize(),fp);
	printf("ret != out->GetSize() %d != %d\n",ret,out->GetSize());
	if(ret != out->GetSize())
		printf("Error in writing to Nw Dispatcher file\n");
	if(fp)
		fclose(fp);
	return;
}
