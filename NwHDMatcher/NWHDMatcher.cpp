/*
 * NWHDMatcher.cpp
 *
 *  Created on: 03-Mar-2010
 *      Author: akhil
 */
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Configuration.h"
#include "NWHDMatcher.h"
#include "MessageExt.h"
#include "HDMatcher.h"
#include "socket.h"
#include "SocketFactory.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "FileRW.h"
#include "PermRW.h"
#include "MemRW.h"
#include "HDMatcherFactory.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include "DBAdapter.h"

extern "C"{
#include "file_manip.h"
}
using namespace std;

NWHDMatcher::NWHDMatcher(Configuration& conf):m_DB(0),m_ShouldIQuit(false),m_KeyFilename(0),
m_ID(0),m_BufferSize(0),m_dbRdr(0),m_MatchMsgRxBuffer(0),m_IrisData(0),m_DBFileMsg(NULL),m_portBinded(false)
{
	// TODO Auto-generated constructor stub
	m_port = conf.getValue("GRI.HDMatcher.port", 8082);
	m_BufferSize = conf.getValue("GRI.HDMatcherBuffSize",1024*1024);

	int irissz = conf.getValue("GRI.IrisSize",1280);

	m_IrisDB = (unsigned char*) malloc(irissz*2);

	m_DB = (unsigned char*) malloc(m_BufferSize);
	if(m_DB==0) {
		printf("Could not allocate a db buffer of size %d",m_BufferSize);
		exit(1);
	}


	m_ID = conf.getValue("GRI.HDMatcherID",-1);
	m_Debug = conf.getValue("GRI.HDMatcherDebug",false);
	if(m_ID == -1){
		printf("Unable to Get Id \n");
		throw("Unable to Get ID...\n");
	}

	HDMatcherFactory HDMFactory(conf);
	m_HDMatcher = HDMFactory.Create(NWMATCHER,irissz,m_BufferSize,m_ID);
	if(conf.getValue("GRI.useCoarseFineMatch",false)){
		int sz = m_BufferSize>>2;
		m_CoarseDB =(unsigned char*) malloc(IRIS_SIZE_INCLUDING_MASK>sz?IRIS_SIZE_INCLUDING_MASK:sz);
	}
	else m_CoarseDB =0;


	m_KeyFilename = (char*)conf.getValue("GRI.KeyFile","/mnt/mmc/Keys.bin");

//	m_dbRdr = new PermRW(new AesRW(new MemRW((char*)m_DB +m_IrisDBHeader->GetHeaderSize())));
	m_BinMsg = new BinMessage(65536);
	m_HDMsg = new HDMatcherMsg();
	char* ack = "ASSIGNDB;DONE";
	m_DBack = new BinMessage(ack,strlen(ack));

	m_Pongack = new BinMessage(256);
	GeneratePongMsg();
	char* done = "DONE";
	m_Exitack = new BinMessage(done,strlen(done));
	m_Matchack= new BinMessage(512);
	m_MatchMsgRxBuffer = (char *)malloc(1024*10);
	m_IrisData = new IrisData;
	m_socketFactory = new SocketFactory(conf);
#ifndef HBOX_PG
	m_dbFileName = (char *)conf.getValue("GRI.irisCodeDatabaseFile", "data/sqlite.db3");
#else
	m_dbFileName = (char *)conf.getValue("GRI.irisCodeDatabaseFile", "data/sqlite.db3");
#endif	
	int usleeptime = conf.getValue("GRI.DBSleeptimeInMilliSec", 1)*1000;
//	m_Debug = conf.getValue("GRI.DBDebug",false);
	m_tempDbFileName= (char *)malloc(strlen(m_dbFileName)+10);
	sprintf(m_tempDbFileName,"%s.tmp",m_dbFileName);
    m_DBFileMsg = new FileMsg(m_tempDbFileName,usleeptime,m_Debug?1:0);
    m_dbAdapter = new DBAdapter();

}

void NWHDMatcher::Init(char *ptr){
	m_HDMatcher->Init(1280);
	printf("ID %d Buff %d \n",m_ID,m_BufferSize);
}

NWHDMatcher::~NWHDMatcher() {
	delete m_HDMatcher;
	delete m_BinMsg;
	delete m_HDMsg;
	delete m_DBack;
	delete m_Pongack;
	delete m_Exitack;
	delete m_Matchack;
	delete m_dbRdr;
	if(m_DB) free(m_DB);
	if(m_CoarseDB) free(m_CoarseDB);
	if(m_IrisDB) free(m_IrisDB);
	if(m_IrisData){
		delete m_IrisData;
	}
	if(m_MatchMsgRxBuffer)
		free (m_MatchMsgRxBuffer);
	if(m_socketFactory)
		free (m_socketFactory);
	if (m_DBFileMsg)
		delete m_DBFileMsg;
    if(m_dbAdapter)
    	delete m_dbAdapter;

}
void NWHDMatcher::run()
{
	// start listening on network
	std::string name = "NWHDMatcher::";
	SocketServer *sockSrv=0;

	printf("Listening on Port %d \n",m_port);
	while (!m_ShouldIQuit) {
		try{
			m_portBinded=false;
#ifndef HBOX_PG
	//		if(!sockSrv) sockSrv= new SocketServer(m_socketFactory->createSocketServer("Eyelock.NWHDMatcherSecure",m_port));
#else
			if(!sockSrv) sockSrv= new SocketServer(m_port);
#endif
			if(!sockSrv) sockSrv= new SocketServer(m_port);
			m_portBinded=true;
			sockSrv->Accept(OnConnect, this);
		}
		catch(Exception& ncex){
			if(sockSrv)delete sockSrv;
			sockSrv=0;
			cout <<name;
			ncex.PrintException();
			// sleep(1);
		}
		catch(const char* msg){
			if(sockSrv)delete sockSrv;
			sockSrv=0;
			cout <<name<<msg <<endl;
			// sleep(1);
		}
	}
	if(sockSrv){
		printf("Closing Socket Srv\n");
		sockSrv->CloseInput();
		sockSrv->CloseOutput();
		delete sockSrv;
	}
	sockSrv=0;

	// on a network connection call OnConnect
}
int NWHDMatcher::CheckMsg(char* inp){

	if(m_Debug){
		printf("Check MSG::%.*s\n",30,inp);
	}
	const char *assigndb= "ASSIGNDB";
	if (0==strncmp(inp,assigndb,(int)strlen(assigndb)))
		return 0;
	const char *ping= "PING";
	if (0==strncmp(inp,ping,(int)strlen(ping)))
		return 1;
	const char *match= "MATCH";
	if (0==strncmp(inp,match,(int)strlen(match)))
		return 2;
	const char *exit= "EXIT";
	if (0==strncmp(inp,exit,(int)strlen(exit)))
		return 3;
	const char *updateIris= "UPDATEIRIS";
	if (0==strncmp(inp,updateIris,(int)strlen(updateIris)))
		return 4;
	const char *deleteIris= "DELETEIRIS";
	if (0==strncmp(inp,deleteIris,(int)strlen(deleteIris)))
		return 5;
	char msg[10]={0}; strncpy(msg,inp,9);
	cout <<"Unknown message recvd by NWHDMatcher: "<<msg<<endl;

	return -1;
}

char* NWHDMatcher::getUpdateIris(char* Buffer,int size,int& msgsize,int& preludeSize){
   	char *temp=Buffer;
   	temp=strstr(temp,";");
    if(0==temp) return 0;
	temp+=1;
	msgsize=atoi(temp); //msgsize
	temp=strstr(temp,";");
	if(0==temp) return 0;
	temp+=1;
	preludeSize  = size-(temp-Buffer);
	return temp;
}


char* NWHDMatcher::getIris(char* Buffer,int size,int& id,int& taskId,int& irissize,int& preludeSize)
{
   	char *temp=Buffer;
   	temp=strstr(temp,";");
    if(0==temp) return 0;
    temp+=1;
    id=atoi(temp); //ID
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;
   	taskId=atoi(temp); //taskid
	temp=strstr(temp,";");
	if(0==temp) return 0;
	temp+=1;
   	irissize=atoi(temp); //irisSize
	temp=strstr(temp,";");
	if(0==temp) return 0;
	temp+=1;
	preludeSize  = size-(temp-Buffer);
	return temp;
}



char* NWHDMatcher::getDBPrelude(char* Buffer, int size,int& preludeSize, int& dbFileSize,int& startindex,int& numeyes){
   	char *temp=Buffer;
   	temp=strstr(temp,";");
    if(0==temp) return 0;
    temp+=1;
    startindex=atoi(temp); //startindex
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;

   	numeyes=atoi(temp); //numeyes
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;

    dbFileSize=atoi(temp); //DBSIZE
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;
	preludeSize  = size-(temp-Buffer);
	return temp;
}


void NWHDMatcher::GetCompleteData(char *DBptr ,char *srcptr, int preludeSize, int dbsize, Socket & client)
{
    memcpy(DBptr, srcptr, preludeSize);
    if(dbsize > preludeSize){
        m_HDMsg->SetBuffer(DBptr + preludeSize);
        m_HDMsg->SetWrittenSize(preludeSize);
        m_HDMsg->SetFileSize(dbsize);
    	try{
    		client.ReceiveOn(*m_HDMsg);
    	}catch(Exception& ncex){
    		cout <<"NWHDMatcher::GetCompleteData";
    		ncex.PrintException();
    	}
    	catch(const char* msg){
    		cout <<"NWHDMatcher::GetCompleteData"<<msg <<endl;
    	}
    }
}



void NWHDMatcher::GetCompleteDBData(char *srcptr, int preludeSize, int dbFileSize, Socket & client){

	m_DBFileMsg->SetFileSize(dbFileSize);
	m_DBFileMsg->Save(srcptr, preludeSize);
	if(dbFileSize == preludeSize){
		m_DBFileMsg->Cleaup();
	}

	if(dbFileSize > preludeSize){
		client.Receive(*m_DBFileMsg);
	}
	m_DBFileMsg->Cleaup();
}

void NWHDMatcher::SaveFile(char *DBptr ,int size)
{
   FILE *fp = fopen("/mnt/mmc/Dump.bin","wb");
   fwrite(DBptr,1,size,fp);
   fclose(fp);
}
void NWHDMatcher::GeneratePongMsg(){
	char *format = "PONG;%d;%d;%d;";
	int len = sprintf(m_Pongack->GetBuffer(),format,m_HDMatcher->GetID(),m_HDMatcher->GetStartIndx(),m_HDMatcher->GetNumEyes());
	m_Pongack->SetSize(len);
}


void NWHDMatcher::GetDB(char *& ptr, int & preludeSize, Socket & client)
{
    int dbFileSize =0;
    int startindex=0;
    int numeyes=0;

    if(m_Debug){
		printf("GetDB %d::%.*s\n",m_BinMsg->GetSize(),min(20,m_BinMsg->GetSize()),m_BinMsg->GetBuffer());
	}
    ptr = getDBPrelude(m_BinMsg->GetBuffer(), m_BinMsg->GetSize(), preludeSize, dbFileSize,startindex,numeyes);
    if(m_Debug){
		printf("Prelude %d,DbSize %d, Start %d,Eyes %d\n",preludeSize,dbFileSize,startindex,numeyes);
	}
    Init(ptr);
    bool success = false;
    if(ptr){

        GetCompleteDBData(ptr, preludeSize, dbFileSize, client);
        m_dbAdapter->CloseConnection();
        m_dbAdapter->OpenFile(m_tempDbFileName);
        if(m_dbAdapter->GetUserCount() >= (numeyes>>1)){
        	m_dbAdapter->CloseConnection();
        	if(0 == rename(m_tempDbFileName,m_dbFileName)){
        		m_dbAdapter->OpenFile(m_dbFileName);
        		m_HDMatcher->UpdateHD(startindex,numeyes);
        		if(m_HDMatcher->InitializeDb(m_dbAdapter,m_DB,m_CoarseDB)){
        			success = true;
        		}
        	}
        }
        if(m_Debug){
        	printf("Received DB in %#0x\n",m_DB);
        }
        try{
			client.SendAll(*m_DBack);
		}catch(Exception& ncex){
			cout <<"NWHDMatcher::GetDB";
			ncex.PrintException();
		}
		catch(const char* msg){
			cout <<"NWHDMatcher::GetDB"<<msg <<endl;
		}
    }
    if(!success){
    	m_HDMatcher->UpdateHD(0,0);
    }
}

void NWHDMatcher::DeleteIris(char *ptr, int preludeSize, Socket & client){
    int msgsz;
    ptr = getUpdateIris(m_BinMsg->GetBuffer(),m_BinMsg->GetSize(),msgsz, preludeSize);
    if(m_Debug){
    	printf("DeleteIris %d Prelude %d\n", msgsz, preludeSize);
    }
    if((msgsz == GUID_SIZE) && (ptr)){
        GetCompleteData((char*)m_MatchMsgRxBuffer, ptr, preludeSize, msgsz, client);

        bool ret = m_HDMatcher->DeleteSingleUser((unsigned char*)&m_MatchMsgRxBuffer[0],m_DB,m_CoarseDB);
    	char *format = "DELETEIRIS;DONE;%d;";
        int len = sprintf(m_Matchack->GetBuffer(), format,ret?1:0);
        m_Matchack->SetSize(len);
        client.SendAll(*m_Matchack);
    }else{
    	char buff[20];
		strncpy(buff,m_BinMsg->GetBuffer(),19);
    	if(msgsz != IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON)
			printf("IrisMsgSize not equal to %d bytes %s ",IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON,buff);
    	if(!ptr){
         	printf("Unable to Parse Message: %s\n",buff);
		}
    }
}

void NWHDMatcher::UpdateIris(char *ptr, int preludeSize, Socket & client){
    int msgsz;
    ptr = getUpdateIris(m_BinMsg->GetBuffer(),m_BinMsg->GetSize(),msgsz, preludeSize);
    if(m_Debug){
    	printf("UpdateIris %d Prelude %d\n", msgsz, preludeSize);
    }
    if((msgsz == IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON) && (ptr)){
        GetCompleteData((char*)m_MatchMsgRxBuffer, ptr, preludeSize, msgsz, client);
        bool ret = m_HDMatcher->UpdateSingleUser((unsigned char*)&m_MatchMsgRxBuffer[IRIS_SIZE_INCLUDING_MASK_PER_PERSON],(unsigned char*)&m_MatchMsgRxBuffer[0],(unsigned char*)&m_MatchMsgRxBuffer[IRIS_SIZE_INCLUDING_MASK],m_DB,m_CoarseDB);
    	char *format = "UPDATEIRIS;DONE;%d;";
        int len = sprintf(m_Matchack->GetBuffer(), format,ret?1:0);
        m_Matchack->SetSize(len);
        client.SendAll(*m_Matchack);
    }else{
    	char buff[20];
		strncpy(buff,m_BinMsg->GetBuffer(),19);
    	if(msgsz != IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON)
			printf("IrisMsgSize not equal to %d bytes %s ",IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON,buff);
    	if(!ptr){
         	printf("Unable to Parse Message: %s\n",buff);
		}
    }
}

void NWHDMatcher::GetMatch(char *ptr, int preludeSize, Socket & client)
{
    int taskId;
    int irissize;
    int id;
    int featurelen = 2560;

    ptr = getIris(m_BinMsg->GetBuffer(), m_BinMsg->GetSize(), id, taskId, irissize, preludeSize);
    if(m_Debug){
    	printf("ID %d Task %d IrisSz %d Prelude %d\n", id, taskId, irissize, preludeSize);
    }
	int irisdatareqd =0;
    if(preludeSize > 20){
    	irisdatareqd = NwMatcherSerialzer::getBitStreamSize(ptr);
  //  	printf("Reqd Sz %d \n",irisdatareqd);
    }else{
    	printf("ERROR NWHDMatcher::GetMatch\n");
    }

    if((irissize == featurelen) && (ptr) && (m_HDMatcher->GetID() == id)){
        GetCompleteData((char*)m_MatchMsgRxBuffer, ptr, preludeSize, irisdatareqd, client);

        NwMatcherSerialzer nws;
        nws.ExtractNwMsg(m_IrisData,m_MatchMsgRxBuffer);

    	std::pair<int, float> res= std::make_pair(-1, 2.0);
    	 char *key = NULL;
    	 int f2flength = GUID_SIZE;
    	 char test[] = "00000000-0000-0000-0000-000000000000";

       //Check if the numeyes is 0 means there is no DB to it and Eyes have come.
        // Inform the MatchSvr abt the bad state..
		if (m_HDMatcher->GetNumEyes() != 0) {
			XTIME_OP("NWHDMatcher",res = m_HDMatcher->MatchIrisCode(m_IrisData->getIris(),m_DB, m_CoarseDB));
			int indx = res.first >= 0 ? res.first : 0;
			//key = (char*) m_HDMatcher->GetF2FAndIDKey(m_DB, indx);
			std::string guid = m_HDMatcher->GetMatchGUID(m_DB,indx);

			if(res.first >= 0){
				key = (char*)guid.c_str();
			}else{
				key = test;
			}

    	}else{
    		key = test;
    		memset(m_Matchack->GetBuffer(),0,512);
    	}
    	char *format = "MATCHRESULT;%d;%d;%d;%0.6f;";
        int len = sprintf(m_Matchack->GetBuffer(), format, id, taskId, res.first, res.second);
        m_Matchack->SetSize(len + f2flength);
        memcpy(m_Matchack->GetBuffer() + len, key, f2flength);
        client.SendAll(*m_Matchack);

        if(m_Debug){
        	printf("F2F:");
        	for(int i=0;i<f2flength;i++){
        		printf(" %d",*(m_Matchack->GetBuffer()+len+i));
        	}
        	printf("\n");
        	printf("MSG: %d:%.*s\n",len + f2flength,len + f2flength,m_Matchack->GetBuffer());
        }
    }else{
    	char buff[20];
		strncpy(buff,m_BinMsg->GetBuffer(),19);

    	if(irissize != 2560){
			printf("Iris not equal to %d bytes %s ",2560,buff);
    	}
    	if(!ptr){
         	printf("Unable to Parse Message: %s\n",buff);
		}
		if(m_HDMatcher->GetID() != id){
        	printf("Message is not for the Current ID: %s\n",buff);
		}
    }
}

bool NWHDMatcher::do_serv_task(Socket& client) {
	try {
		client.Receive(*m_BinMsg);
		int preludeSize=0;
		char *ptr;
		int ret = CheckMsg(m_BinMsg->GetBuffer());
		switch(ret){
		case 0:
			GetDB(ptr, preludeSize, client);
			break;
		case 1:
			GeneratePongMsg();
		    if(m_Debug){
				printf("PONG %d::%.*s\n",m_Pongack->GetSize(),min(20,m_Pongack->GetSize()),m_Pongack->GetBuffer());
			}
			client.SendAll(*m_Pongack);
			break;
		case 2:
			GetMatch(ptr, preludeSize, client);
			break;
		case 3:
			client.SendAll(*m_Exitack);
			m_ShouldIQuit = true;
			break;
		case 4:
			UpdateIris(ptr, preludeSize, client);
			break;
		case 5:
			DeleteIris(ptr, preludeSize, client);
			break;
		default:
			char buff[20]={0};
			strncpy(buff,m_BinMsg->GetBuffer(),19);
			printf("Unknown Message %s ",buff);
			break;
		}

	} catch (Exception& nex) {
		nex.PrintException();
		return false;
	}
	catch (std::exception ex) {
		cout<<ex.what()<<std::endl;
		return false;
	}
	catch (...)
	{
		cout<<"Unknown exception during NWHDMATCHER"<<std::endl;
		return false;
	}
	return true;
}

void NWHDMatcher::OnConnect(Socket& client,void* arg)
{
	NWHDMatcher *me = (NWHDMatcher *)arg;
	client.KeepAlive(true);
	//block till we know the response
	//parse message
	//act on it
	//create response
	me->do_serv_task(client);
	//client.CloseInput();
}
