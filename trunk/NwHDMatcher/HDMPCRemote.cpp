/*
 * HDMPCRemote.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: developer1
 */

#include <utility>
#include <iostream>
#include "HDMPCRemote.h"

#include "SocketFactory.h"
#include "MessageExt.h"
#include "MatchManager.h"
#include "HDMatcherFactory.h"
#include "RWFactory.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "IrisDBHeader.h"
#include "FileRW.h"
#include "MemRW.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"

using std::pair;
using std::min;
using namespace std;

HDMPCRemote::HDMPCRemote(int size, int id,const char* addr):HDMRemote(size,id,addr) {
	m_taskId = -1;
	m_Status=AVAILABLE;
}

HDMPCRemote::~HDMPCRemote() {
}

int HDMPCRemote::GetNumEyesPossibleInBuffer(){
	return 0;
}
bool HDMPCRemote::UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris){
	return false;
}
bool HDMPCRemote::DeleteSingleUserOnly(unsigned char *guid){
	return false;
}
void HDMPCRemote::AssignDB(DBAdapter *dbAdapter){
}


bool HDMPCRemote::StartMatch(unsigned char *iriscode, int taskid) {

	int featurelen = 2560;

	NwMatcherSerialzer nw;
	bool ret = nw.ExtractNwMsg(m_irisData,(char*)iriscode);
	if(!ret){
		printf("HDMPCRemote::ERROR in IRIS CODE\n");
	}

	// Append the device ID - format "deviceType:deviceID = 3:234" for nanonxt234
	char *deviceID = m_pMatchManagerInterface->GetDeviceID();
	m_taskId = taskid;
	//printf("MATCH:1;3:%s;%d;%d;\n", myID, 2, featurelen);
	int len = sprintf(m_matchMsg->GetBuffer(), "MATCH:1;3:%s;%d;%d;", deviceID, 2, featurelen);
	memcpy(m_matchMsg->GetBuffer()+len,m_irisData->getIris(),featurelen);
	m_matchMsg->SetSize(len+featurelen);

	try {
		if(m_sock) delete m_sock;
		m_sock=0;
		m_sock= m_socketFactory->createSocketClientP("Eyelock.NwMatcherCommSecure");
		m_sock->SetTimeouts(m_timeOut);
		m_sock->Connect((*m_pAddress));
		m_sock->SendAll(*m_matchMsg);
		m_Status=BUSY;
		SetAssigned(1);
	} catch (Exception& ex) {
		printf("Unable to send to PC MATCHER\n");
		if(m_sock)delete m_sock;
		m_sock=0;
		ex.PrintException();
		return false;
	}
	return true;
}

bool HDMPCRemote::SendPCMatchMsg(char *msg, int msglen) {

	int len = 0;
	char *deviceID = m_pMatchManagerInterface->GetDeviceID();
	len = sprintf(m_matchMsg->GetBuffer(), "MATCH:1;3:%s;%d;%d;", deviceID, 2, msglen);	// 2 is ACS in network matcher
	m_Status=BUSY;
	//printf( "MATCH:1;3:%s;%d;%d;", deviceID, 2, msglen);
    //Verb:  VALIDATE:X;DEV_TYPE{:DEV_ID};CARDDATA
    //ID:  x:y   x = deviceType  y = deviceid
    //CARDDATA:  c = carddata (encoded)
	if (msg) {
		len = sprintf(m_matchMsg->GetBuffer(), "VALIDATECARD:1;3:%s;", deviceID);
		printf("VALIDATECARD:1;3:%s;\n", deviceID);
		memcpy(m_matchMsg->GetBuffer()+len,msg,msglen);
		m_Status=CARDMATCH;
	}
	m_matchMsg->SetSize(len+msglen);

	try {
		if(m_sock) delete m_sock;
		m_sock=0;
		m_sock= m_socketFactory->createSocketClientP("Eyelock.NwMatcherCommSecure");
		m_sock->SetTimeouts(m_timeOut);
		m_sock->Connect((*m_pAddress));
		m_sock->SendAll(*m_matchMsg);
		SetAssigned(1);
	} catch (Exception& ex) {
		printf("SendPCMatchMsg() - Unable to send to PC MATCHER\n");
		if(m_sock)delete m_sock;
		m_sock=0;
		m_Status=AVAILABLE;
		ex.PrintException();
		return false;
	}
	return true;
}
//char *format = "MATCHRESULT;%d;%d;%d;%0.4f;";

bool HDMPCRemote::GetResult() {
	// assumes that the result is received
	char* Buffer = m_dbMsg->GetBuffer();
	std::pair<int, float> res;

	//printf("MSG:: %.*s\n",m_dbMsg->GetSize(),m_dbMsg->GetBuffer());

	// PCMATCH:1;x;x;ACS:43:allthedata;WINAUTH:23:winauthinfo;etc…
	char *temp = Buffer;
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

	// ACS:43:[acdlen][acd][pid] ==> ACS:43:[total length in 2 bytes][acdlen][acd][pid]
	temp = strstr(temp, ":");
	if (0 == temp)
		return false;
	temp += 1;
	int f2fsz = atoi(temp); // total f2f sz
	if (0 == f2fsz)
		return false;

	temp = strstr(temp, ":");
	if (0 == temp)
		return false;
	temp += 1;

	// 1st byte is card data length, not used for now
	// 2fsz -= 1;
	// temp += 1;

	res.first += GetStartIndx();
	m_pMatchManagerInterface->RegisterResult( GetID(), m_taskId, res, (unsigned char *)temp,f2fsz); // 1st byte is card data length

	return true;
}

void HDMPCRemote::AssignDB(char *fname, int memio) {
	m_Status=AVAILABLE;
}

bool HDMPCRemote::CheckIfDone(bool check){
	if(!(m_Status==BUSY || m_Status==CARDMATCH)) 
	    return true;
	if(!check){
		printf("HDMPCRemote::CheckIfDone close socket\n");
		if(m_sock)delete m_sock;
		m_sock=0;
		m_Status=AVAILABLE;
		return true;
	}
	assert(m_sock!=0);

	if (m_Status==CARDMATCH) {
		if(GetAck(m_sock,VALIDATECARD)){
			bool result = GetCardResult();
			m_Status=AVAILABLE;
			delete m_sock;
			m_sock=0;
			return result;
		}
	}
	else {
		if(GetAck(m_sock,MATCH)){
			GetResult();
			m_Status=AVAILABLE;
			delete m_sock;
			m_sock=0;
			return true;
		}
	}

	return false;
}

bool HDMPCRemote::GetAck(Socket* sock,NWMSGTYPE ackRx) {
	char *expect = "";
	int bytes = 0;
	try{
		//printf("Rx Timeout %d -> %d %d \n",m_ID,sock->GetReceiveTimeout().tv_sec,sock->GetReceiveTimeout().tv_usec);
		bytes = sock->Receive(*m_dbMsg);
	}
	catch(Exception& ncex){
		cout <<"GetAcK::";
		ncex.PrintException();
	}
	catch(const char* msg){
		cout <<"GetAcK::"<<msg <<endl;
	}

	if(bytes<1) return false;

	if (ackRx == ASSIGNDB) {
		expect = "ASSIGNDB;DONE";
	} else if (ackRx == PING) {
		expect = "PONG";
	} else if (ackRx == MATCH) {
		expect = "PCMATCH";
	} else if (ackRx == EXITIT) {
		expect = "DONE";
	} else if (ackRx == VALIDATECARD) {
		expect = "VALIDATECARD";
	}
	int expectSz = strlen(expect);
	bool ret = memcmp(expect, m_dbMsg->GetBuffer(), expectSz) ? false : true;
	if (!ret) {
		printf("expected %s not found\n", expect);
		throw Exception(SimpleString("Unable to Get Acknowledgement"));
	}
	return ret;
}

bool HDMPCRemote::GetPong(int& id,int& start,int& numeyes){
	id = m_ID; //ID
	start = m_startIndx; // start
	numeyes = m_numIris; // numeyes
	return true;
}

bool HDMPCRemote::GetCardResult() {
	// assumes that the result is received
	char* Buffer = m_dbMsg->GetBuffer();

	// VALIDATECARD;False
	char *temp = Buffer;
	temp = strstr(temp, ";");
	if (0 == temp)
		return false;
	temp += 1;
	//printf("card match: %s\n", temp);
	if (!strncmp(temp, "True", 4)){
		temp = strstr(temp, ";");
		if (0 != temp)
			strcpy(m_cardMatchName, temp+1);
		return true;
	}


	return false;

}
