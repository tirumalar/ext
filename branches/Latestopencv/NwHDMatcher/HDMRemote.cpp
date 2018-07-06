/*
 * HDMRemote.cpp
 *
 *  Created on: 04-Mar-2010
 *      Author: akhil
 */
#include <unistd.h>
#include <utility>
#include <iostream>
#include "HDMRemote.h"
#include "SocketFactory.h"
#include "MessageExt.h"
#include "MatchManager.h"
#include "HDMatcherFactory.h"
#include "RWFactory.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "FileRW.h"
#include "MemRW.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include "DBAdapter.h"
#include "logging.h"
#include "UtilityFunctions.h"

using std::pair;
using std::min;
using namespace std;
const char logger[30] = "HDMRemote";

HDMRemote::HDMRemote(int size, int id,const char* addr):
	HDMatcher(size, id,false),m_socketFactory(0){
	EyelockLog(logger, DEBUG, "Address %s ",addr);
	m_pAddress = new HostAddress(addr);
	m_genMsg = new TextMessage(64);
	m_dbMsg = new BinMessage(128*1024);
	m_matchMsg = new BinMessage(48*1024);
	//May be we can Check if the the other end responds to PING
	m_sock = 0;
	m_outDataMsg = new FixedMsg();
	m_numIris = m_size;

	m_Status=REGISTERED;
}

HDMRemote::~HDMRemote() {
	delete m_pAddress;
	delete m_genMsg;
	delete m_dbMsg;
	if(m_sock)
		delete m_sock;
	m_sock=0;
	if(m_socketFactory)
		delete m_socketFactory;
}

void HDMRemote::InitSSL(){
	Configuration& conf = (Configuration&) *getConf();
    m_socketFactory = new SocketFactory(conf);
}

//int  HDMRemote::GetNumEyesPossibleInBuffer(){
//	int val = 0;
//	if(m_IrisDBHeader)
//		val= 2*(GetSize()/m_IrisDBHeader->GetOneRecSizeinDBFile());
//	return val;
//}
bool HDMRemote::StartMatch(unsigned char *iriscode, int taskid) {
	int featurelen = 2560;
	int val = NwMatcherSerialzer::getBitStreamSize((char*)iriscode);
	int len = sprintf(m_matchMsg->GetBuffer(), "MATCH;%d;%d;%d;", GetID(),
			taskid,featurelen);
	memcpy(m_matchMsg->GetBuffer()+len,iriscode,val);
	m_matchMsg->SetSize(len+val);

	try {
		if(m_sock) delete m_sock;
		m_sock=0;
		m_sock= m_socketFactory->createSocketClientP("Eyelock.SlaveMasterCommSecure");
		m_sock->SetTimeouts(m_timeOut);
		m_sock->ConnectByHostname(*m_pAddress); // DJH: retry=true
//		EyelockLog(logger, DEBUG, "Sending MATCHMSG");
//		m_sock->SendAll(*m_matchMsg);

		m_outDataMsg->SetData((char*)m_matchMsg->GetBuffer(),len+val);
		m_sock->SendAll(*m_outDataMsg);
		SetAssigned(1);
		m_Status=BUSY;
	} catch (Exception& ex) {
		DeclareBad();
		if(m_sock)delete m_sock;
		m_sock=0;
		cout <<"StartMatch::";
		ex.PrintException();
		return false;
	}
	return true;
}

//char *format = "MATCHRESULT;%d;%d;%d;%0.4f;";

bool HDMRemote::GetResult() {
	// assumes that the result is received
	char* Buffer = m_dbMsg->GetBuffer();
	std::pair<int, float> res;
	char *temp = Buffer;
	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	int id = atoi(temp); //ID

	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	int taskid = atoi(temp); // TaskId

	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	res.first = atoi(temp); // res.first

	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	res.second = atof(temp); // res.second

	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	res.first += GetStartIndx();

	std::string keyret;
	keyret.resize(2+GUID_SIZE);
	memset((void*)keyret.c_str(),0,2);
	memcpy((void*)(keyret.c_str()+2),temp,GUID_SIZE);

	m_pMatchManagerInterface->RegisterResult(id, taskid,res,(unsigned char *)keyret.c_str());
	return true;
}

bool HDMRemote::GetAck(Socket* sock,NWMSGTYPE ackRx) {
	char *expect = "";
	int bytes = 0;
	try{
		bytes = sock->Receive(*m_dbMsg);
	}
	catch(Exception& ncex){
		cout <<"GetAcK::"<<ackRx;
		ncex.PrintException();
	}
	catch(const char* msg){
		cout <<"GetAcK::"<<ackRx<<msg <<endl;
	}

	if(bytes<1) return false;

	if (ackRx == ASSIGNDB) {
		expect = "ASSIGNDB;DONE";
	} else if (ackRx == PING) {
		expect = "PONG";
	} else if (ackRx == MATCH) {
		expect = "MATCHRESULT";
	} else if (ackRx == EXITIT) {
		expect = "DONE";
	} else if (ackRx == UPDATEIRIS) {
		expect = "UPDATEIRIS;DONE";
	}else if (ackRx == DELETEIRIS) {
		expect = "DELETEIRIS;DONE";
	}
	int expectSz = strlen(expect);
	bool ret = memcmp(expect, m_dbMsg->GetBuffer(), expectSz) ? false : true;
	if (!ret) {
		EyelockLog(logger, ERROR, "expected %s not found", expect);
		throw Exception(SimpleString("Unable to Get Acknowledgement"));
	}
	return ret;
}

void HDMRemote::AssignDB(DBAdapter *dbAdapter){
	if(dbAdapter){
		dbAdapter->CloseConnection();
		EyelockLog(logger, DEBUG, "Start Indx %d Num Eyes %d ",GetStartIndx(),GetNumEyes());
		int sz = FileSize(dbAdapter->GetDBFileName().c_str());
		int len = sprintf(m_dbMsg->GetBuffer(), "ASSIGNDB;%d;%d;%d;",GetStartIndx(),GetNumEyes(),sz);
		FILE *fp = NULL;
		fp = fopen((char*)dbAdapter->GetDBFileName().c_str(),"rb");
		SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
		try {
			sock.ConnectByHostname(*m_pAddress); // DJH: retry=true
			int position = 0;
			int maxBytes = sz;
			int bytesToRead = maxBytes;
			//EyelockLog(logger, DEBUG, "Bytes To Read %d %d", bytesToRead,m_dbMsg->GetAvailable());
			int initialfill = len;
			while (bytesToRead > 0) {
				int tryRead = min(m_dbMsg->GetAvailable(), bytesToRead);
				tryRead -=initialfill;
				//int bytes = fRW->Read((unsigned char *)m_dbMsg->GetBuffer()+initialfill,tryRead,position);
				int bytes = fread((unsigned char *)m_dbMsg->GetBuffer()+initialfill,1,tryRead,fp);
				if(bytes != tryRead){
					DeclareBad();
					EyelockLog(logger, ERROR, "HDMRemote:: Unable to Read Data base from File %s", dbAdapter->GetDBFileName().c_str());
//					if(fRW){
//						delete fRW;
//						fRW = NULL;
//					}
					fclose(fp);
					fp = NULL;
					return;
				}
				position+=bytes;
				m_dbMsg->SetSize(bytes+initialfill);
				bytesToRead -= bytes;
				sock.SendAll(*m_dbMsg);
				initialfill = 0;
				//EyelockLog(logger, DEBUG, "%d %d Bytes To Read %d %d", bytes,position,bytesToRead,m_dbMsg->GetAvailable());
			}
			EyelockLog(logger, DEBUG, "\n ******** All bytes sent !!! *************** \n"); fflush(stdout);

			if(GetAck(&sock,ASSIGNDB))// blocks till i get a ack
				m_Status=AVAILABLE;

			EyelockLog(logger, DEBUG, "\n ******** ACK received !!! *************** \n"); fflush(stdout);
		}catch (NetIOException nex) {
			EyelockLog(logger, DEBUG, "HDMRemote::AssignDB::NetIOException");
			cout <<"HDMRemote::AssignDB";
			nex.PrintException();
		}
		catch (const char *msg) {
			EyelockLog(logger, DEBUG, "HDMRemote::AssignDB::CHAR *");
			cout <<"HDMRemote::AssignDB";
			cerr <<msg <<endl;
		}
		catch (Exception& ex) {
			EyelockLog(logger, DEBUG, "HDMRemote::AssignDB");
			cout <<"HDMRemote::AssignDB";
			ex.PrintException();
			DeclareBad();
		}
		if(fp)
			fclose(fp);
//		if(fRW)
//			delete fRW;
	}
}

void HDMRemote::AssignDB(char *fname, int memio) {
#ifdef MADHAV
	// first send the message header
	//ASSIGNDB;<IRISDBHEADER><Data>
	int numrectoread = (GetNumEyes()>>1);
	int recstartindx = (GetStartIndx()>>1);
	int len = sprintf(m_dbMsg->GetBuffer(), "ASSIGNDB;%d;", GetIrisDBHeader()->GetHeaderSize()+numrectoread*GetIrisDBHeader()->GetOneRecSizeinDBFile());

	EyelockLog(logger, DEBUG, "%d MSG: %.*s",len,len,m_dbMsg->GetBuffer());
// Create Reader Writer Factory
	ReaderWriter *fRW;
    if(memio){
    	fRW = new MemRW(fname+GetIrisDBHeader()->GetHeaderSize());
    }else{
    	fRW = new FileRW((char*)fname,GetIrisDBHeader()->GetHeaderSize());
    }

	//FileRW *fRW = new FileRW((char*)fname,GetIrisDBHeader()->GetHeaderSize());
	SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
	try {
		sock.ConnectByHostname(*m_pAddress); // DJH: retry=true
		int position = recstartindx * GetIrisDBHeader()->GetOneRecSizeinDBFile();
		int maxBytes = numrectoread* GetIrisDBHeader()->GetOneRecSizeinDBFile();
		int bytesToRead = maxBytes;
		EyelockLog(logger, DEBUG, "Bytes To Read %d ", bytesToRead);

//send db header first
		IrisDBHeader temp;
		GetIrisDBHeader()->CopyHeader(&temp);
		temp.SetNumRecord(numrectoread);
		temp.SetNumEyes(GetNumEyes());

		MemRW mem(m_dbMsg->GetBuffer()+len);
		temp.WriteHeader(&mem);

		int initialfill = temp.GetHeaderSize()+len;

//		for(int i=len;i<initialfill;i++)
//			EyelockLog(logger, DEBUG, "%#0x ",*(m_dbMsg->GetBuffer()+i));
//		EyelockLog(logger, DEBUG, "");

		while (bytesToRead > 0) {
			int tryRead = min(m_dbMsg->GetAvailable(), bytesToRead);
			tryRead -=initialfill;
			int bytes = fRW->Read((unsigned char *)m_dbMsg->GetBuffer()+initialfill,tryRead,position);
			if(bytes != tryRead){
				DeclareBad();
				EyelockLog(logger, ERROR, "HDMRemote:: Unable to Read Data base from File %s", fname);
				if(fRW){
					delete fRW;
					fRW = NULL;
				}
				return;
			}
			position+=bytes;
			m_dbMsg->SetSize(bytes+initialfill);
			bytesToRead -= bytes;
			sock.SendAll(*m_dbMsg);
			initialfill = 0;
			EyelockLog(logger, DEBUG, "Bytes To Read %d ", bytesToRead);
		}

		if(GetAck(&sock,ASSIGNDB))// blocks till i get a ack
			m_Status=AVAILABLE;
	} catch (Exception& ex) {
		cout <<"HDMRemote::AssignDB";
		ex.PrintException();
		DeclareBad();
	}
	if(fRW)
		delete fRW;
#endif
}

void HDMRemote::PrintGUIDs(){
	printf("Remote:: %d\n",m_numIris);
}

bool HDMRemote::DeleteSingleUserOnly(unsigned char *guid){
	bool ret = false;
	int len = sprintf(m_dbMsg->GetBuffer(), "DELETEIRIS;%d;",GUID_SIZE);
	SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
	try {
		sock.ConnectByHostname(*m_pAddress); // DJH: retry=true
		memcpy(m_dbMsg->GetBuffer()+len,guid,GUID_SIZE);len+=GUID_SIZE;
		m_dbMsg->SetSize(len);
		sock.Send(*m_dbMsg);
		ret = GetAck(&sock,DELETEIRIS);// blocks till i get a ack
		int retval=-1;
		char* Buffer = m_dbMsg->GetBuffer();
		char *temp = Buffer;
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		retval = atoi(temp); //retval
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		if(retval == 1){
			ret = true;
		}else{
			ret = false;
		}

	}
	catch(Exception& ncex){
		EyelockLog(logger, DEBUG, "Exceptin in DeleteSingleUserOnly()");
		cout <<"UpdateSingleUserOnly::";
		ncex.PrintException();
	}
	catch(const char* msg){
		EyelockLog(logger, DEBUG, "Exceptin in DeleteSingleUserOnly() %s", msg);
		cout <<"UpdateSingleUserOnly::"<<msg <<endl;
	}
	return ret;
}

bool HDMRemote::UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris){
	bool ret = false;
	int len = sprintf(m_dbMsg->GetBuffer(), "UPDATEIRIS;%d;",IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
	try {
		sock.ConnectByHostname(*m_pAddress); // DJH: retry=true
		memcpy(m_dbMsg->GetBuffer()+len,leftiris,IRIS_SIZE_INCLUDING_MASK);len+=IRIS_SIZE_INCLUDING_MASK;
		memcpy(m_dbMsg->GetBuffer()+len,rightiris,IRIS_SIZE_INCLUDING_MASK);len+=IRIS_SIZE_INCLUDING_MASK;
		memcpy(m_dbMsg->GetBuffer()+len,perid,GUID_SIZE);len+=GUID_SIZE;
		m_dbMsg->SetSize(len);
		sock.Send(*m_dbMsg);
		ret = GetAck(&sock,UPDATEIRIS);// blocks till i get a ack
		int retval=-1;
		char* Buffer = m_dbMsg->GetBuffer();

		char *temp = Buffer;
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		retval = atoi(temp); //retval
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		if(retval == 1){
			ret = true;
		}else{
			ret = false;
		}

	}
	catch(Exception& ncex){
		EyelockLog(logger, DEBUG, "Exceptin in UpdateSingleUserOnly()");
		cout <<"UpdateSingleUserOnly::";
		ncex.PrintException();
	}
	catch(const char* msg){
		EyelockLog(logger, DEBUG, "Exceptin in UpdateSingleUserOnly() %s", msg);
		cout <<"UpdateSingleUserOnly::"<<msg <<endl;
	}
	return ret;
}

bool HDMRemote::CheckIfDone(bool check)
{
	if(m_Status!=BUSY) return true;
	if(!check){
		//EyelockLog(logger, DEBUG, "HDMRemote::CheckIfDone close socket");
		if(m_sock)
			delete m_sock;
		m_sock=0;
		m_Status=AVAILABLE;
		return true;
	}

	assert(m_sock!=0);
	if(GetAck(m_sock,MATCH)){

		if(GetResult())
			m_Status=AVAILABLE;
		else
			DeclareBad();

		delete m_sock;
		m_sock=0;
		return true;
	}
	return false;
}

bool HDMRemote::SendPing(){

	strcpy(m_genMsg->GetBuffer(),"PING");
	m_genMsg->SetSize(strlen(m_genMsg->GetBuffer()));
	SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
	//SocketClient sock = m_socketFactory->createSocketClient(SOCK_UNSECURE);
	bool ret = false;
	try{
		//EyelockLog(logger, DEBUG, "SendPing() - timeout sec=%d, usec=%d", m_timeOut.tv_sec, m_timeOut.tv_usec);
		sock.SetTimeouts(m_timeOut);
		sock.ConnectByHostname(*m_pAddress); // DJH: rety=true

//		sock.DontDelay(true);
		sock.SendAll(*m_genMsg);
		ret = GetAck(&sock,PING);// blocks till i get a ack
	}
	catch(Exception& ncex){
		EyelockLog(logger, DEBUG, "Exceptin in SendPing()");
		cout <<"PING::";
		ncex.PrintException();
	}
	catch(const char* msg){
		EyelockLog(logger, DEBUG, "Exceptin in SendPing() %s", msg);
		cout <<"PING::"<<msg <<endl;
	}
	return ret;
}

bool HDMRemote::isReboot(bool reset){
	if (!reset) {
		if (access("/home/root/slaveFailRebootCnt.txt", F_OK ) != -1)
			remove("/home/root/slaveFailRebootCnt.txt");
		return false;
	}

	bool ret = true;
	FILE *fp;
	
	if (strcmp(m_pAddress->GetOrigHostName(), "192.168.40.2:8082"))
		return false;	// not slave
	EyelockLog(logger, DEBUG, "isReboot()\n");
	if (access("/home/root/slaveFailRebootCnt.txt", F_OK ) == -1)
		fp = fopen("/home/root/slaveFailRebootCnt.txt","w+");
	else
		fp = fopen("/home/root/slaveFailRebootCnt.txt","r+");
	if(fp) {
		char failCnt = getc(fp);
		failCnt++;
		if (failCnt > 3) {
			ret = false;
		}
		
		fseek(fp, 0, SEEK_SET);
		fputc (failCnt, fp);
		fclose(fp);
		RunSystemCmd("sync");
		EyelockLog(logger, DEBUG, "isReboot() fail count = %d", failCnt);
	}
	if (ret){
		EyelockLog(logger, DEBUG, "Reboot system due to no response from Slave");
	}
	return ret;
}
bool HDMRemote::GetPong(int& id,int& start,int& numeyes){

	strcpy(m_genMsg->GetBuffer(),"PING");
	m_genMsg->SetSize(strlen(m_genMsg->GetBuffer()));
	SocketClient sock = m_socketFactory->createSocketClient("Eyelock.SlaveMasterCommSecure");
	//SocketClient sock = m_socketFactory->createSocketClient(SOCK_UNSECURE);
	bool ret = false;
	try{
		sock.SetTimeouts(m_timeOut);
		sock.ConnectByHostname(*m_pAddress); // DJH: retry=true

		sock.SendAll(*m_genMsg);
		ret = GetAck(&sock,PING);// blocks till i get a ack
		id=start=numeyes=0;
		char* Buffer = m_dbMsg->GetBuffer();

		char *temp = Buffer;
		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		id = atoi(temp); //ID

		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		start = atoi(temp); // start

		temp = strstr(temp, ";");
		if (0 == temp)
			return false;
		temp += 1;
		numeyes = atoi(temp); // numeyes
		temp = strstr(temp, ";");
		if (0 == temp)
			ret = false;
	}
	catch(Exception& ncex){
		cout <<"PONG::";
		ncex.PrintException();
	}
	catch(const char* msg){
		cout <<"PONG::"<<msg <<endl;
	}
	return ret;
}
