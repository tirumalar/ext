/*
 * UT_SafePtr.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include <iostream>
#include <socket.h>
#include "SocketFactory.h"
#include "HTTPPOSTMsg.h"
#include "BiOmega.h"
#include "NwMatchManager.h"
#include "MatchManagerInterface.h"
#include "MatchManagerFactory.h"
#include "IrisDBHeader.h"
#include "LEDDispatcher.h"
#include "logging.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include "LoiteringDetector.h"
#include "F2FDispatcher.h"
#include "MatchDispatcher.h"
#include "DBAdapter.h"
#include "NWHDMatcher.h"
#include "MatchManagerSimple.h"
#include "FileRW.h"
#include "UtilityFunctions.h"

extern "C" {
#include "file_manip.h"
}

namespace tut {

void *hdDispatcher(void *args) {
	NWHDMatcher *n  = (NWHDMatcher *) (args);
	n->run();
}
pthread_t startHDListener(NWHDMatcher *n) {
	pthread_t hdThread;
	pthread_create(&hdThread, NULL,hdDispatcher, (void *)n);
	return hdThread;
}

struct TestDataMat {
	TestConfiguration cfg1;
	TestConfiguration cfg;//empty configuration
	TestDataMat() {
		//For Nw HDMatcher
		//empty configuration
		cfg1.setValue("GRI.HDMatcher.port", "8082");
		cfg1.setValue("GRI.HDMatcherBuffSize","500000");
		cfg1.setValue("GRI.HDMatcherID","0");
		cfg1.setValue("GRI.HDMatcherDebug","0");
		cfg1.setValue("GRI.irisCodeDatabaseFile","./abc.db3");
		cfg1.setValue("GRI.DBDebug","0");
		cfg1.setValue("GRI.MatcherFeatureMask","15");
		cfg1.setValue("GRI.HDDebug","0");
	}
	~TestDataMat() {
		SendMsg();
		remove("./abc.db3");
	}
	void SendDB(int start,int numeyes) {
		//SocketServer sockSrv(8082);
		// launch the main program first on the below ip first test no 1
		HostAddress host("localhost:8082");
		char rcvdbmsg[100];
		char* fname = "./data/sqlite.db3";
		FileRW file(fname, 0);
		int sz = FileSize(fname);
		int dbSize = sz + 2000;
		unsigned char* db = (unsigned char*) (malloc(dbSize * sizeof(char)));
		file.Read(db, sz, 0);
		if (db == NULL) {
			printf("Unable to Allocate the Buffer0\n");
		} else {
			sprintf(rcvdbmsg, "ASSIGNDB;%d;%d;%d;",start,numeyes,sz);
			BinMessage msg(rcvdbmsg, strlen(rcvdbmsg));
			int len = strlen(rcvdbmsg);
			msg.Append((char*) db, sz);
			SocketClient client;
			client.Connect(host);
			client.SendAll(host, msg);

			BinMessage *rcvdMsg = new BinMessage(256);
			client.Receive(*rcvdMsg);
			ensure_equals("ASSIGNDB DONE RXED", 0,memcmp("ASSIGNDB;DONE", rcvdMsg->GetBuffer(), 13));
			delete rcvdMsg;
			client.CloseInput();
			client.CloseOutput();
		}
		free(db);
	}
	void SendMsg(){
		HostAddress h("localhost:8082");
		struct timeval timeOut;
		timeOut.tv_sec = 1;
		timeOut.tv_usec = 0;
		try{
			SocketFactory s(cfg);
			SocketClient client=s.createSocketClient((SocketSecurityType)0);
			client.SetTimeouts(timeOut);
			client.ConnectByHostname(h);
			BinMessage msg("EXIT",4);
			client.Send(msg,MSG_DONTWAIT);
		}
		catch(Exception& nex){
			printf("SendMsg Exception -=--------------------------------\n"); fflush(stdout);
			nex.PrintException();
		}catch(const char *msg){
			printf("SendMsg Exception -=--------------------------------\n"); fflush(stdout);
			std::cout<< msg <<endl;
		}catch(...){
			std::cout<< "Unknown exception during SendMsg for eyelock" <<endl;
		}
	}

void ReadDB(char *fname,string& matchBuffer1,string& matchBuffer2,bool comp){
	int req = 50*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE);
	matchBuffer1.resize(req);
	memset((void*)matchBuffer1.c_str(),0,req);
	matchBuffer2.resize(req);
	memset((void*)matchBuffer2.c_str(),0,req);

	DBAdapter *db = new DBAdapter();
	db->OpenFile(fname);
	int person = db->GetUserCount();
	int ret = db->MakeMatchBuffer((char*)matchBuffer1.c_str(),matchBuffer1.length(),person,0,comp);
	ensure("ret should be 0",ret == 0);
	ret = db->MakeMatchBuffer((char*)matchBuffer2.c_str(),matchBuffer2.length(),person,0,false);
	ensure("ret should be 0",ret == 0);
	delete db;
}

	void FillIrisData(bool comp,bool coarsefine,string& matchBuffer1,string& matchBuffer2,IrisData* m_irisData,int i,bool even){
		char* ptr=NULL;
		if(comp){
			if(!coarsefine){
				//even?0:COMPRESS_IRIS_SIZE_INCLUDING_MASK
				ptr = (char*)(matchBuffer1.c_str()+i*(COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON));
				if(even) ptr+=COMPRESS_IRIS_SIZE_INCLUDING_MASK;
				UnCompressIris((unsigned char*)ptr,m_irisData->getIris(),COMPRESS_IRIS_SIZE_INCLUDING_MASK);
			}else{
				//+ even?0:IRIS_SIZE_INCLUDING_MASK
				ptr = (char*)(matchBuffer2.c_str()+i*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE));
				if(even) ptr+=IRIS_SIZE_INCLUDING_MASK;
				memcpy((void*)m_irisData->getIris(),ptr,IRIS_SIZE_INCLUDING_MASK);
			}
		}else{
			//+ even?0:IRIS_SIZE_INCLUDING_MASK
			ptr = (char*)(matchBuffer2.c_str()+i*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE));
			if(even) ptr+=IRIS_SIZE_INCLUDING_MASK;
			memcpy((void*)m_irisData->getIris(),ptr,IRIS_SIZE_INCLUDING_MASK);
		}
	}
	void Run1(bool comp,bool coarsefine){
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDDebug","0");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
		cfg.setValue("GRI.MatcherFeatureMask","15");
		if(coarsefine)cfg.setValue("GRI.useCoarseFineMatch","1");
		if(comp)cfg.setValue("GRI.CompressedMatching","1");
		string matchBuffer1,matchBuffer2;
		ReadDB("./data/sqlite.db3",matchBuffer1,matchBuffer2,comp);

		NwMatchManager proc(cfg);

		MatchManagerInterface *mm = proc.GetMM();
		IrisData *m_irisData =  new IrisData();
		NwMatcherSerialzer ns;

		for(int i=0;i<11;i++){
//			char *ptr = NULL;
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//			printf("Score %f \n",result->getScore());
			ensure("match result even should match",result->getEyeIndex()==2*i);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			{
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
				int ret = ns.GetSizeOfNwMsg(m_irisData);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//				printf("Score %f \n",result->getScore());
				ensure("match result odd should match",result->getEyeIndex()==2*i+1);
//				printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			}
		}
		delete m_irisData;
	}

	void Run2(bool comp,bool coarsefine){
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","REMOTE");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.HDMatcher.0.Address","localhost:8082");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
		cfg.setValue("GRI.MatcherFeatureMask","15");

		if(coarsefine)cfg1.setValue("GRI.useCoarseFineMatch","1");
		if(comp)cfg1.setValue("GRI.CompressedMatching","1");
		string matchBuffer1,matchBuffer2;
		ReadDB("./data/sqlite.db3",matchBuffer1,matchBuffer2,comp);

		NWHDMatcher *nw = new NWHDMatcher(cfg1);
		startHDListener(nw);

		NwMatchManager proc(cfg);
		MatchManagerInterface *mm = proc.GetMM();
		IrisData *m_irisData =  new IrisData();
		NwMatcherSerialzer ns;

		for(int i=0;i<11;i++){
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
			ensure("match result even should match",result->getEyeIndex()==2*i);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			{
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
				ensure("match result odd should match",result->getEyeIndex()==2*i+1);
//				printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			}
		}
		delete m_irisData;
		SendMsg();
		usleep(500);
		delete nw;
	}

	void Run3(bool comp,bool coarsefine){
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDMatcherCount","2");
		cfg.setValue("GRI.HDMatcher.0.Type","REMOTE");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.HDMatcher.0.Address","localhost:8082");
		cfg.setValue("GRI.HDMatcher.1.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.1.BuffSize","300000");

		cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
		cfg.setValue("GRI.MatcherFeatureMask","15");
		cfg.setValue("GRI.HDDebug","0");
		cfg.setValue("GRI.DBDebug","0");

		cfg1.setValue("GRI.HDMatcherDebug","0");
		if(coarsefine){
			cfg1.setValue("GRI.useCoarseFineMatch","1");
			cfg.setValue("GRI.useCoarseFineMatch","1");
		}
		if(comp){
			cfg1.setValue("GRI.CompressedMatching","1");
			cfg.setValue("GRI.CompressedMatching","1");
		}
		string matchBuffer1,matchBuffer2;
		ReadDB("./data/sqlite.db3",matchBuffer1,matchBuffer2,comp);

		NWHDMatcher *nw = new NWHDMatcher(cfg1);
		startHDListener(nw);

		NwMatchManager proc(cfg);
		MatchManagerInterface *mm = proc.GetMM();
		IrisData *m_irisData =  new IrisData();
		NwMatcherSerialzer ns;
		char * guid[]={
				"972E6E6C-D9DA-335E-1601-C3424E98362C",
				"93AA9CD4-AEBA-4860-DF7E-6FA59B5A647F",
				"BE542BF7-FB2B-24A3-EC22-100568F471A2",
				"C28226C4-3ADD-D5BD-4F16-122D6461CD28",
				"9DE7BF9C-D4D2-54E1-39A3-830B63FC438D",
				"D134C0AA-FCE2-D71A-9FD3-EF73F7FBFA1C",
				"F076D775-1D7B-E773-4EBE-0F9BEDA07E96",
				"0005CDE8-577E-BD4A-E596-4EADE6F79349",
				"3767F516-38EE-71A5-79DE-BA62F88A36D7",
				"34775E16-AFFC-EF02-0CEC-8B8416697FAB",
				"5FD9CE75-73EA-9F33-B76D-F6FC26543008"
		};

		for(int i=0;i<11;i++){
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
			ensure("match result even should match",0 == memcmp(result->getGUID().c_str(),guid[i],36));
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			{
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
				int ret = ns.GetSizeOfNwMsg(m_irisData);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
	//			ensure("match result odd should match",result->getEyeIndex()==2*i+1);
//				printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
				ensure("match result odd should match",0 == memcmp(result->getGUID().c_str(),guid[i],36));
			}
		}
		delete m_irisData;
		SendMsg();
		usleep(500);
		delete nw;
	}
	void Run4(bool comp,bool coarsefine){
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDDebug","0");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
		cfg.setValue("GRI.MatcherFeatureMask","15");
		if(coarsefine){
			cfg.setValue("GRI.useCoarseFineMatch","1");
		}
		if(comp){
			cfg.setValue("GRI.CompressedMatching","1");
		}
		remove("./data/testing.db3");
		string matchBuffer1,matchBuffer2;
		ReadDB("./data/sqlite.db3",matchBuffer1,matchBuffer2,comp);

		NwMatchManager proc(cfg);
		MatchManagerInterface *mm = proc.GetMM();
		IrisData *m_irisData =  new IrisData();
		NwMatcherSerialzer ns;

		for(int i=0;i<11;i++){
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
			ensure("match result even should match",result->getEyeIndex()==-1);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
			int ret1 = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg2(ret1);
			ns.MakeNwMsg(msg2.GetBuffer(),m_irisData);
			result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
			ensure("match result odd should match",result->getEyeIndex()==-1);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
		}

		BinMessage passnext(1024);
		int len = sprintf(passnext.GetBuffer(),"RELOADDB;%d;%d;%d;%d;",eREPLACEDB,0,0,0);
		passnext.SetSize(len);

		system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3_0.tmp");

		proc.process(&passnext);
		usleep(2000);
		for(int i=0;i<11;i++){
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
			ensure("match result even should match",result->getEyeIndex()==2*i);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
			int ret1 = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg2(ret1);
			ns.MakeNwMsg(msg2.GetBuffer(),m_irisData);
			result=mm->DoMatch((unsigned char *)msg2.GetBuffer());
			ensure_equals("match result odd should match",result->getEyeIndex(),2*i+1);
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
		}
		remove("./data/testing.db3");
		delete m_irisData;
	}
};
typedef test_group<TestDataMat> tg;
typedef tg::object testobject;
}

extern int seqindx;
namespace {
tut::tg test_group("Match Manager tests");
}

namespace tut {


template<>
template<>
	void testobject::test<1>() {
		//compression,coarsefine
		Run1(false,false);
	}
template<>
template<>
	void testobject::test<2>() {
		Run1(false,true);
	}
template<>
template<>
	void testobject::test<3>() {
		Run1(true,false);
	}
template<>
template<>
	void testobject::test<4>() {
		Run1(true,true);
	}
template<>
template<>
	void testobject::test<5>() {
	//compression,coarsefine
		Run2(false,false);
	}
template<>
template<>
	void testobject::test<6>() {
		Run2(false,true);
	}
template<>
template<>
	void testobject::test<7>() {
		Run2(true,false);
	}
template<>
template<>
	void testobject::test<8>() {
		Run2(true,true);
	}

// One local One Remote Matcher

template<>
template<>
	void testobject::test<9>() {
	//compression,coarsefine
		Run3(false,false);
	}
template<>
template<>
	void testobject::test<10>() {
		Run3(false,true);
	}
template<>
template<>
	void testobject::test<11>() {
		Run3(true,false);
	}
template<>
template<>
	void testobject::test<12>() {
		Run3(true,true);
	}
template<>
template<>
	void testobject::test<13>() {
	//compression,coarsefine
		Run4(false,false);
	}
template<>
template<>
	void testobject::test<14>() {
		Run4(false,true);
	}
template<>
template<>
	void testobject::test<15>() {
		Run4(true,false);
	}
template<>
template<>
	void testobject::test<16>() {
		Run4(true,true);
	}

#if 0
	template<>
	template<>
	void testobject::test<5>() {
		set_test_name("Check the ASSIGNDB to NW HD TEST");
		NWHDMatcher *nw = new NWHDMatcher(cfg1);
		startHDListener(nw);
		SendDB(0,72);
		SendMsg();
		usleep(500);
		delete nw;
	}
	template<>
	template<>
	void testobject::test<6>() {
		NWHDMatcher *nw = new NWHDMatcher(cfg1);
		startHDListener(nw);

		SendDB(10,62);

		HostAddress host("localhost:8082");
		char *ping = "PING";
		BinMessage msg(ping, strlen(ping));
		SocketClient client;
		client.Connect(host);
		client.SendAll(host, msg);
		BinMessage *rcvdMsg = new BinMessage(256);
		client.Receive(*rcvdMsg);
		printf("Got Message %3s",rcvdMsg->GetBuffer());
		ensure_equals("PONG RXED",0,memcmp("PONG",rcvdMsg->GetBuffer(),4));
		client.CloseInput();
		client.CloseOutput();
		SendMsg();
		usleep(500);
		delete rcvdMsg;
		delete nw;
	}
	char* ParseMatchresult(char* Buffer,int size,int& id,int& taskId,int& irisno,float& score)
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
		irisno=atoi(temp); //irisno
		temp=strstr(temp,";");
		if(0==temp) return 0;
		temp+=1;
		score=atof(temp); //Score
		temp=strstr(temp,";");
		if(0==temp) return 0;
		temp+=1;
		return temp;
	}

	template<>
	template<>
	void testobject::test<7>() {
		set_test_name("Check the MATCH MSG for NW HD TEST");

		NWHDMatcher *nw = new NWHDMatcher(cfg1);
		startHDListener(nw);
		int stoff= 10;
		SendDB(stoff,84);
		//seqindx

		// Now the testcase begins
		string matchBuffer1;
		int req = 50*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE);
		matchBuffer1.resize(req);

		DBAdapter *db = new DBAdapter();
		db->OpenFile("./data/sqlite.db3");
		int person = db->GetUserCount();
		int ret = db->MakeMatchBuffer((char*)matchBuffer1.c_str(),matchBuffer1.length(),person,0);
		ensure("ret should be 0",ret == 0);
		delete db;

		HostAddress host("localhost:8082");
		char rcvdbmsg[100];
		if(((seqindx) > 84)||((seqindx) <= 0))
			seqindx = stoff;
		IrisData *m_irisData =  new IrisData();
		NwMatcherSerialzer ns;
		char *ptr = ((char*)(matchBuffer1.c_str()) + (seqindx>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON) +(seqindx&0x1)*IRIS_SIZE_INCLUDING_MASK);
		memcpy((void*)m_irisData->getIris(),ptr,IRIS_SIZE_INCLUDING_MASK);
		ret = ns.GetSizeOfNwMsg(m_irisData);
		BinMessage msg1(ret);
		ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);

		sprintf(rcvdbmsg,"MATCH;%d;%d;%d;",0,1268,2560);
		BinMessage msg(rcvdbmsg, strlen(rcvdbmsg));
		msg.Append(msg1.GetBuffer(),ret);

		SocketClient client;
		client.Connect(host);
		client.SendAll(host, msg);
		sleep(2);
		BinMessage *rcvdMsg = new BinMessage(256);
		client.Receive(*rcvdMsg);
		printf("%d::Message %.*s",rcvdMsg->GetSize(),rcvdMsg->GetSize(),rcvdMsg->GetBuffer());
		ensure_equals("REGISTER RXED",0,memcmp("MATCHRESULT",rcvdMsg->GetBuffer(),6));
		int id,taskid,res;
		float score;
		char *keygot = ParseMatchresult(rcvdMsg->GetBuffer(),rcvdMsg->GetSize(),id,taskid,res,score);
		//	char *key = (char*)(Buff + 4 +rptr.GetFeatureLength()*2 + (seqindx>>1)*rptr.GetOneRecSizeinDB());
		char *key = (char*)(matchBuffer1.c_str() + (seqindx>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON) + IRIS_SIZE_INCLUDING_MASK_PER_PERSON);
		ensure_equals("Key got",0,memcmp(keygot,key,16));
		ensure_equals("ID",0,id);
		ensure_equals("TaskId",1268,taskid);
		ensure_equals("ID",seqindx,res+stoff);
		delete rcvdMsg;
		delete m_irisData;
		SendMsg();
		usleep(500);
		delete nw;
	}
#endif
}
