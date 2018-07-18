/*
 * DBReceive.cpp
 *
 *  Created on: 13-Feb-2010
 *      Author: akhil
 */
#include <unistd.h>
#include "DBReceive.h"
#include "LEDDispatcher.h"
#include "F2FDispatcher.h"
#include "MessageExt.h"
#include "SocketFactory.h"
#include "IrisDBHeader.h"
#include "logging.h"
#include "SocketFactory.h"
#include "socket.h"
#include "UtilityFunctions.h"
#include "DBUpdates.h"
#include <iostream>
#include "CommonDefs.h"
extern "C" {
#include "file_manip.h"
#include "sqlite3.h"
}
#include "logging.h"

const char logger[30] = "DBReceive";

DBReceive::DBReceive(Configuration& conf ):GenericProcessor(conf),m_DBFileMsg(0),m_ReloadMsg(0),m_ledDispatcher(0)
,m_trigDispatcher(0),m_f2fDispatcher(0),m_tempDbFileName(0),m_F2FDbRxMsg(256),m_F2FDbDoneMsg(256),m_Debug(false),m_rxfilenumber(0)
,m_socketFactory(NULL),m_resultDestAddr(NULL),m_tempBuffer(NULL){
#ifndef HBOX_PG
	m_dbFileName = conf.getValue("GRI.irisCodeDatabaseFile", "data/sqlite.db3");
#else
	m_dbFileName = conf.getValue("GRI.irisCodeDatabaseFile", "data/sqlite.db3");
#endif
	int usleeptime = conf.getValue("GRI.DBSleeptimeInMilliSec", 1)*1000;
	m_Debug = conf.getValue("GRI.DBDebug",false);
	m_tempDbFileName= (char *)malloc(strlen(m_dbFileName)+128);
	sprintf(m_tempDbFileName,"%s_%d.tmp",m_dbFileName,m_rxfilenumber);
    m_DBFileMsg = new FileMsg(m_tempDbFileName,usleeptime,m_Debug?1:0);

    int timeOutms = conf.getValue("GRI.DBReceiveTimeoutMilliSec", 500);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;

	m_ReloadMsg = new HTTPPOSTMsg(32);
	m_ReloadMsg->makeReloadMsg();

//	char* ack = "RECEIVEDB;DONE";
//	m_DBack = new BinMessage(ack,strlen(ack));
//
//	char* nack = "RECEIVEDB;NACK";
//	m_DBNack = new BinMessage(nack,strlen(nack));

	const char *str =  conf.getValue("Eyelock.F2FCardDataDbReceive","0x0000");
	MakeF2FMsg(str ,DBRELOAD,m_F2FDbRxMsg);

	const char *str1 =  conf.getValue("Eyelock.F2FCardDataDbDone","0x0000");
	MakeF2FMsg(str1 ,DBRELOAD,m_F2FDbDoneMsg);

	m_f2fResult.init();
	m_f2fResult.setState(DBRELOAD);

	const char *svrAddr = conf.getValue("Eyelock.F2FDestAddr", "NONE");
	if(strcmp(svrAddr,"NONE")!= 0){
		m_resultDestAddr = HostAddress::MakeHost(svrAddr, eIPv4, false);
		timeOutms = conf.getValue("Eyelock.F2FSocketTimeOutMillis", 500);
		m_timeOutSend.tv_sec = timeOutms / 1000;
		m_timeOutSend.tv_usec = (timeOutms % 1000) * 1000;

	}
	m_socketFactory = new SocketFactory(conf);
	m_DeleteUpdateMsg = new HDMatcherMsg();
	m_tempBuffer = new char[65536];
	m_ReceiveMsg = new HTTPPOSTMsg(65536);

}

DBReceive::~DBReceive() {

	if (m_DBFileMsg)
		delete m_DBFileMsg;
	if (m_ReloadMsg)
		delete m_ReloadMsg;
	if(m_tempDbFileName)
		free(m_tempDbFileName);
	if(m_resultDestAddr)
		delete m_resultDestAddr;
	if(m_socketFactory)
		delete m_socketFactory;
	if(m_tempBuffer)
		delete m_tempBuffer;
	if(m_DeleteUpdateMsg)
		delete m_DeleteUpdateMsg;
}

Copyable *DBReceive::createNewQueueItem(){
	return new DBRecieveMsg(false);
}



void DBReceive::SendMessage(HTTPPOSTMsg& outMsg)
{
	int type = -1;
	char *ptr = outMsg.getF2F(type);
	if(m_Debug){
		printf("DBReceive::Update F2F %.*s\n",30,ptr);
		for(int i=0;i<outMsg.GetSize();i++ ){
				printf("%02x ",ptr[i]);
		}
		printf("\n");
	}
	if(ptr){
		m_f2fResult.setF2F(ptr);
	}
	int len ,bits;
	char *test = m_f2fResult.getF2F(len,bits);
	if(bits){
		if(m_f2fDispatcher){
			m_f2fDispatcher->enqueMsg(m_f2fResult);
		}
		else{
			try
			{
				if(m_socketFactory&&m_resultDestAddr){
					SocketClient client = m_socketFactory->createSocketClient("Eyelock.F2FDispatcherSecure");
					client.SetTimeouts(m_timeOutSend);
					client.ConnectByHostname(*m_resultDestAddr);
					client.SendAll(outMsg,MSG_DONTWAIT);
				}
			}
			catch(Exception& nex)
			{
				EyelockLog(logger, ERROR, "DBReceive::SendMessage failed\n"); fflush(stdout);
				nex.PrintException();
			}
		}
	}
}
void DBReceive::process(Copyable *_msg) {
	DBRecieveMsg *msg = (DBRecieveMsg *) _msg;

	//xxlog("process DB msg type is %d\n", msg->m_msgType);
	EyelockLog(logger, DEBUG, "process DB msg type is %d", msg->m_msgType);

	// save prelude
	bool savedSucc = false;
	bool dbReloadSuccess = true;
	EyelockLog(logger, DEBUG, "Length %d\n", msg->m_FileSize);
	Socket client =	SocketFactory::wrapSocket(msg->m_SD,(SecureTrait*)msg->m_SecureTrait);
	client.SetTimeouts(m_timeOut);
	try {
		if(m_ledDispatcher)m_ledDispatcher->SetDBUploadState();
		SendMessage(m_F2FDbRxMsg);
		if(m_f2fDispatcher)m_f2fDispatcher->SetSendingEveryNSec(false);
		sprintf(m_tempDbFileName,"%s_%d.tmp",m_dbFileName,m_rxfilenumber++);
		m_DBFileMsg->SetFileName(m_tempDbFileName);
		m_DBFileMsg->SetFileSize(msg->m_FileSize);
		m_DBFileMsg->Save(msg->m_prelude, msg->m_preludeSize);
		if(m_Debug)
			EyelockLog(logger, DEBUG, "File %s, Filesize %d, Prelude %d\n", m_tempDbFileName, msg->m_FileSize, msg->m_preludeSize);
		if(msg->m_FileSize == msg->m_preludeSize){
			m_DBFileMsg->Cleaup();
		}
		// if we have to get more than prelude lets read from socket
		if (msg->m_FileSize >= msg->m_preludeSize) {
			if(msg->m_FileSize > msg->m_preludeSize)
				client.Receive(*m_DBFileMsg);

			bool test=true;

			HTTPPOSTMsg passnext(1024);
			int len = -1;

#if defined(HBOX_PG) || defined(CMX_C1) // for x64
			EyelockLog(logger, DEBUG, "Formatting message for x64");
			const char* formatStr = "RELOADDB;%d;%d;%d;%lu;%d;";
#else
			const char* formatStr = "RELOADDB;%d;%d;%d;%lu;%d;";
			//const char* formatStr = "RELOADDB;%d;%d;%d;%d;%d;";
#endif
			if(msg->m_msgType == eREPLACEDB){
				len = sprintf(passnext.GetBuffer(),formatStr,eREPLACEDB,m_rxfilenumber-1,msg->m_SD,(unsigned long)msg->m_SecureTrait,msg->m_isEncrypt);
			}else if(msg->m_msgType == eUPDATEDB){
				len = sprintf(passnext.GetBuffer(),formatStr,eUPDATEDB,m_rxfilenumber-1,msg->m_SD,(unsigned long)msg->m_SecureTrait,msg->m_isEncrypt);
			}
			if(len>0){
				passnext.SetSize(len);
				client.SetshouldClose(false);
				callNext(passnext);
			}else{
				dbReloadSuccess = false;
				EnableDispatchers(dbReloadSuccess);
				EyelockLog(logger, DEBUG, "Sending Nack for RECEIVEDB\n");
			}
		}
		m_DBFileMsg->Cleaup();
	} catch (Exception& ex) {
		dbReloadSuccess = false;
		m_DBFileMsg->Cleaup();
		ex.PrintException();
		EnableDispatchers(dbReloadSuccess);
		EyelockLog(logger, ERROR, "Unable to recieve db over network,ignoring\n");
	} catch (...) {
		dbReloadSuccess = false;
		m_DBFileMsg->Cleaup();
		EnableDispatchers(dbReloadSuccess);
		EyelockLog(logger, ERROR, "Unable to recieve db generic error\n");
	}

	try
	{
		if(!dbReloadSuccess)
		{
			char nack[32]= {0};
			memcpy(nack,"RECEIVEDB;NACK;",15);
			BinMessage Nack(nack,strlen(nack));
			EyelockLog(logger, INFO, "Sending Nack for RECEIVEDB %.*s\n",strlen(nack),Nack.GetBuffer());
			client.Send(Nack);
		}
		client.CloseInput();
	}
	catch(...)
	{
		EyelockLog(logger, ERROR, "Sorry could not send the acknowledgement\n");
	}
}
void DBReceive::EnableDispatchers(bool success){

	if(m_f2fDispatcher)m_f2fDispatcher->SetSendingEveryNSec(true);
	if(!success){
		SendMessage(m_F2FDbDoneMsg);
	}
	usleep(1000*1000);
	if(m_ledDispatcher)m_ledDispatcher->SetInitialState();

//	if(m_trigDispatcher)m_trigDispatcher->RestartDispatcher();
//	if(m_f2fDispatcher)m_f2fDispatcher->RestartDispatcher();
}

bool DBReceive::instateNewFile()
{
	if(0 ==rename(m_tempDbFileName,m_dbFileName)){
		sync();
		EyelockLog(logger, DEBUG, "Received File Now run sync()  Cmd \n");
		return true;
	}
	else
		return false;
}

DBRecieveMsg::DBRecieveMsg(bool shallow):m_shallow(shallow),m_prelude(0),m_preludeSize(0),m_FileSize(0),m_SecureTrait(0),m_isDownloadUsr(false),m_msgType(eREPLACEDB)
{
	if(!m_shallow)
		m_prelude=(char *)malloc(SOCKET_RECV_BUFF_SIZE);
}

DBRecieveMsg::~DBRecieveMsg()
{
	if(!m_shallow) free(m_prelude);
}

void DBRecieveMsg::CopyFrom(const Copyable& _other)
{
	assert(!m_shallow);
	DBRecieveMsg& other= (DBRecieveMsg&)_other;
	m_SD=other.m_SD;
	m_SecureTrait = other.m_SecureTrait;
	assert(other.m_preludeSize<=SOCKET_RECV_BUFF_SIZE);
	m_preludeSize=other.m_preludeSize;
	m_FileSize = other.m_FileSize;
	m_isDownloadUsr = other.m_isDownloadUsr;
	m_msgType = other.m_msgType;
	memcpy(m_prelude,other.m_prelude,m_preludeSize);
	m_IpPort = other.m_IpPort;
	m_isEncrypt = other.m_isEncrypt;
}


bool DBRecieveMsg::CheckDB(char *fname){
	bool ret = true;
	IrisDBHeader *pIrisDBHeader = new IrisDBHeader;
	pIrisDBHeader->SetDefaults();
	EyelockLog(logger, DEBUG, "Reading received DB file %s\n", fname);
	pIrisDBHeader->ReadHeader(fname);
	pIrisDBHeader->PrintAll();
	int fsz = FileSize(fname);

	EyelockLog(logger, DEBUG, "(%d*%d + %d)!= %d \n",pIrisDBHeader->GetOneRecSizeinDBFile(),pIrisDBHeader->GetNumRecord(),pIrisDBHeader->GetHeaderSize(),fsz);
	if(pIrisDBHeader->GetOneRecSizeinDBFile()*pIrisDBHeader->GetNumRecord()+pIrisDBHeader->GetHeaderSize() != fsz){
		EyelockLog(logger, INFO, "File size does not match to the header\n");
		ret=false;
	}

	if(pIrisDBHeader->GetMagic() != 0x6431){
		EyelockLog(logger, INFO, "Magic in header does not match\n");
		ret=false;
	}

	if(fsz != m_FileSize){
		EyelockLog(logger, INFO, "File size does not match %d != %d\n",fsz,m_FileSize);
		ret = false;
	}

	delete pIrisDBHeader;

	if(false == ret)
	{
		char szNewCorruptDB[256] = {0};
		strcpy(szNewCorruptDB,fname);
		strcat(szNewCorruptDB,".corrupt");
		rename(fname,szNewCorruptDB);
	}

	return ret;
}

void DBReceive::GetCompleteData(char * DBptr ,char *srcptr, int preludeSize, int dbsize, Socket & client){
	EyelockLog(logger, DEBUG, "DBReceive::GetCompleteData %d of %d \n",preludeSize,dbsize);
    memcpy(DBptr, srcptr, preludeSize);
    m_DeleteUpdateMsg->SetBuffer(DBptr + preludeSize);
    m_DeleteUpdateMsg->SetWrittenSize(preludeSize);
    m_DeleteUpdateMsg->SetFileSize(dbsize);
    EyelockLog(logger, DEBUG, "DBReceive::GetCompleteData2 %d of %d \n",preludeSize,dbsize);
    if(dbsize > preludeSize){
    	try{
    		client.ReceiveOn(*m_DeleteUpdateMsg);
    	}catch(Exception& ncex){
    		cout <<"DBReceive::GetCompleteData";
    		ncex.PrintException();
      	}
    	catch(const char* msg){
    		cout <<"DBReceive::GetCompleteData"<<msg <<endl;
    	}
    }

}


int DBReceive::callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;

  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");

	//calculate the length of this query
	std::string * ret = (std::string *)NotUsed;
	for(i = 0; i < argc; i++)
	{
		ret->append(argv[i]);
		//if(i < argc-1)
			ret->append(";");
	}
    return 0;
  }
