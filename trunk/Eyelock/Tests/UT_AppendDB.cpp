/*
 * UT_AppendDB.cpp
 *
 *  Created on: Apr 9, 2013
 *      Author: mamigo
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "Configuration.h"
#include "SocketFactory.h"
#include "NwListener.h"
#include "NwMatchManager.h"
#include "DBReceive.h"
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
#include "DBReceive.h"
#include "FileRW.h"
#include "UtilityFunctions.h"
#include <unistd.h>

#include <string.h>
extern "C"{
#include "file_manip.h"
}

char* GUIDTOREMOVE[]={
	"972E6E6C-D9DA-335E-1601-C3424E98362C",
	"93AA9CD4-AEBA-4860-DF7E-6FA59B5A647F",
	"BE542BF7-FB2B-24A3-EC22-100568F471A2",
	"C28226C4-3ADD-D5BD-4F16-122D6461CD28",
	"9DE7BF9C-D4D2-54E1-39A3-830B63FC438D",
	"D134C0AA-FCE2-D71A-9FD3-EF73F7FBFA1C",
	"F076D775-1D7B-E773-4EBE-0F9BEDA07E96",
};

namespace tut {
extern void *hdDispatcher(void *args);
extern pthread_t startHDListener(NWHDMatcher *n);

	struct DBReceiveTestData {
		TestConfiguration cfg;
		TestConfiguration cfg1;//empty configuration
		DBReceiveTestData() {
			cfg1.setValue("GRI.HDMatcher.port", "8082");
			cfg1.setValue("GRI.HDMatcherBuffSize","500000");
			cfg1.setValue("GRI.HDMatcherID","0");
			cfg1.setValue("GRI.HDMatcherDebug","0");
			cfg1.setValue("GRI.irisCodeDatabaseFile","./abc.db3");
			cfg1.setValue("GRI.DBDebug","0");
			cfg1.setValue("GRI.MatcherFeatureMask","15");
			cfg1.setValue("GRI.HDDebug","0");
		}
		~DBReceiveTestData() {
			remove("./data/testing.db3");
			remove("./abc.db3");
		}
		void SendDB(char *fname="./data/sqlite.db3") {
			HostAddress host("localhost:8081");
			char rcvdbmsg[100];
			//char* fname = "./data/sqlite.db3";
			int sz = FileSize(fname);
			int dbSize = sz + 2000;
			unsigned char* db = (unsigned char*) (malloc(dbSize * sizeof(char)));
			FileRW file(fname, 0);
			file.Read(db, sz, 0);
			if (db == NULL) {
				printf("Unable to Allocate the Buffer0\n");
			} else {
				sprintf(rcvdbmsg, "RECEIVEDB;%d;",sz);
				BinMessage msg(rcvdbmsg, strlen(rcvdbmsg));
				int len = strlen(rcvdbmsg);
				msg.Append((char*) db, sz);
				SocketClient client;
				client.Connect(host);
				client.KeepAlive(true);
				client.SendAll(host, msg);
				BinMessage *rcvdMsg = new BinMessage(256);
				client.Receive(*rcvdMsg);
//				printf("Rx %s \n",rcvdMsg->GetBuffer());

				ensure_equals("ASSIGNDB DONE RXED", 0,memcmp("RECEIVEDB;DONE;", rcvdMsg->GetBuffer(), 15));
				delete rcvdMsg;
				client.CloseInput();
				client.CloseOutput();

			}
			free(db);
		}
		void SendUpdate(char *ptr ="localhost:8081") {
			HostAddress host(ptr);
			char rcvdbmsg[100];
			char* fname = "./data/update.db3";
			int sz = FileSize(fname);
			int dbSize = sz + 2000;
			unsigned char* db = (unsigned char*) (malloc(dbSize * sizeof(char)));
			FileRW file(fname, 0);
			file.Read(db, sz, 0);
			if (db == NULL) {
				printf("Unable to Allocate the Buffer0\n");
			} else {
				sprintf(rcvdbmsg, "UPDATEUSR;%d;",sz);
				BinMessage msg(rcvdbmsg, strlen(rcvdbmsg));
				int len = strlen(rcvdbmsg);
				msg.Append((char*) db, sz);
				SocketClient client;
				client.Connect(host);
				client.KeepAlive(true);
				client.SendAll(host, msg);

				BinMessage rcvdMsg(256);
				try{
					client.Receive(rcvdMsg);
				}
				catch(Exception& ncex){
					//cout <<"GetAcK::"<<;
					ncex.PrintException();
				}
				catch(const char* msg){
					cout <<"GetAcK::"<<msg <<endl;
				}
//				printf("Rx %s \n",rcvdMsg.GetBuffer());
				ensure_equals("ASSIGNDB DONE RXED", 0,memcmp("UPDATEUSR;DONE;", rcvdMsg.GetBuffer(), 15));
				client.CloseInput();
				client.CloseOutput();

			}
			free(db);
		}		
		void RunAppenDB1(bool compress,bool coarsetofine){
			cfg.setValue("GRI.MatchMgrDebug","0");
			cfg.setValue("GRI.HDDebug","0");
			cfg.setValue("GRI.HDMatcherCount","1");
			cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
			cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
			cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
			cfg.setValue("GRI.useCoarseFineMatch","1");
			if(coarsetofine)cfg.setValue("GRI.MatcherFeatureMask","15");
			if(compress)cfg.setValue("GRI.CompressedMatching","1");
			cfg.setValue("GRI.DBDebug","1");

			remove("./data/testing.db3");
			{
				system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3");
				system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/update.db3");
				DBAdapter* db = new DBAdapter();
				db->OpenFile("./data/testing.db3");
				int cnt = db->GetUserCount();
				string username,leftiris,rightiris,acd, acdnop;
				int acdlen;
				DBAdapter inc;
				inc.OpenFile("./data/update.db3");
				for(int i=0;i<7;i++){
					string gui(GUIDTOREMOVE[i]);
					int ret = db->GetUserData(gui,username,leftiris,rightiris,acd,acdlen, acdnop);
					ensure("Read data ",0 == ret);
					ret = inc.UpdateUser(gui,username,leftiris,rightiris,acd,acdlen,acdnop);
					ensure("written data ",0 == ret);
					ret = inc.SetUpdateUser(gui,DELETEPERSON);
					ensure("Updated Type ",0 == ret);
				}
				vector<pair<int,string> > retd = inc.GetIncrementalData();
				ensure("The Count should remain same",retd.size() == 7);
				for(int i=0;i<7;i++){
					ensure("Update type should match",retd[i].first == 1);
				}

				db->CloseConnection();
				delete db;
			}
			NwMatchManager nmm(cfg);
			nmm.init();
			MatchManagerInterface *mm = nmm.GetMM();
			int numeyesbef = mm->GetNumEyes();
			bool ret = nmm.UploadDB("./data/update.db3",eUPDATEDB);
			ensure("Deleted 7 USers from DB",ret);
			int numeyesafter = mm->GetNumEyes();
			ensure("Delete all 7 Users",(numeyesbef-numeyesafter)== 7*2);
			remove("./data/testing.db3");
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

		void Run3A(bool comp,bool coarsefine){
			cfg.setValue("GRI.MatchMgrDebug","0");
			cfg.setValue("GRI.HDMatcherCount","1");
			cfg.setValue("GRI.HDMatcher.0.Type","REMOTE");
			cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
			cfg.setValue("GRI.HDMatcher.0.Address","localhost:8082");
			cfg.setValue("GRI.HDMatcher.1.Type","LOCAL");
			cfg.setValue("GRI.HDMatcher.1.BuffSize","300000");

			cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
			cfg.setValue("GRI.MatcherFeatureMask","15");
			cfg.setValue("GRI.HDDebug","0");
			cfg.setValue("GRI.DBDebug","0");

			system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3");

			cfg1.setValue("GRI.HDMatcherDebug","0");
			if(coarsefine){
				cfg1.setValue("GRI.useCoarseFineMatch","1");
				cfg.setValue("GRI.useCoarseFineMatch","1");
			}else{
				cfg1.setValue("GRI.useCoarseFineMatch","0");
				cfg.setValue("GRI.useCoarseFineMatch","0");
			
			}
			if(comp){
				cfg1.setValue("GRI.CompressedMatching","1");
				cfg.setValue("GRI.CompressedMatching","1");
			}else{
				cfg1.setValue("GRI.CompressedMatching","0");
				cfg.setValue("GRI.CompressedMatching","0");
			}
			string matchBuffer1,matchBuffer2;
			ReadDB("./data/sqlite.db3",matchBuffer1,matchBuffer2,comp);

			NWHDMatcher *nw = new NWHDMatcher(cfg1);
			pthread_t pt = startHDListener(nw);
			sleep(2);
			while(!nw->GetBindedStatus()){
				usleep(10000);
				printf(".");
			}
//			printf("Binded to port 8082 \n");
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
			//Delete and Match
			for(int i=0;i<11;i++){
				bool del = mm->DeleteSingleUser(string(guid[i]));
				ensure("USer deleted",del);
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
				int ret = ns.GetSizeOfNwMsg(m_irisData);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
				ensure("match result even should match",0 != memcmp(result->getGUID().c_str(),guid[i],36));
//				printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
				{
					FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
					int ret = ns.GetSizeOfNwMsg(m_irisData);
					BinMessage msg1(ret);
					ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
					MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//					printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
					ensure("match result odd should match",0 != memcmp(result->getGUID().c_str(),guid[i],36));
				}
			}
			//Add and Match
			system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3");
			//proc.recoverFromBadState();
			mm->RecoverFromBadState();
			mm->AssignDB();
			for(int i=0;i<11;i++){
				//
				string left,right,gui;
				left.resize(IRIS_SIZE_INCLUDING_MASK);memset((void*)left.c_str(),0,IRIS_SIZE_INCLUDING_MASK);
				right.resize(IRIS_SIZE_INCLUDING_MASK);memset((void*)right.c_str(),0,IRIS_SIZE_INCLUDING_MASK);
				gui.resize(GUID_SIZE);memset((void*)right.c_str(),0,GUID_SIZE);
//				memcpy((void*)left.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON,IRIS_SIZE_INCLUDING_MASK);
//				memcpy((void*)right.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON+IRIS_SIZE_INCLUDING_MASK,IRIS_SIZE_INCLUDING_MASK);
//				memcpy((void*)gui.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON+IRIS_SIZE_INCLUDING_MASK_PER_PERSON,GUID_SIZE);
				bool add = mm->UpdateSingleUser(gui,left,right);
				ensure("User Updated",!add);
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
				int ret = ns.GetSizeOfNwMsg(m_irisData);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
				ensure("match result even should match",0 == memcmp(result->getGUID().c_str(),guid[i],36));
//				printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
				{
					FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
					int ret = ns.GetSizeOfNwMsg(m_irisData);
					BinMessage msg1(ret);
					ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
					MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//					printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
					ensure("match result odd should match",0 == memcmp(result->getGUID().c_str(),guid[i],36));
				}
			}
			for(int i=0;i<11;i++){
				//
				string left,right,gui;
				left.resize(IRIS_SIZE_INCLUDING_MASK);memset((void*)left.c_str(),0,IRIS_SIZE_INCLUDING_MASK);
				right.resize(IRIS_SIZE_INCLUDING_MASK);memset((void*)right.c_str(),0,IRIS_SIZE_INCLUDING_MASK);
				gui.resize(GUID_SIZE);memset((void*)right.c_str(),0,GUID_SIZE);
//				memcpy((void*)left.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON,IRIS_SIZE_INCLUDING_MASK);
//				memcpy((void*)right.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON+IRIS_SIZE_INCLUDING_MASK,IRIS_SIZE_INCLUDING_MASK);
				memcpy((void*)gui.c_str(),matchBuffer2.c_str() + i*IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON+IRIS_SIZE_INCLUDING_MASK_PER_PERSON,GUID_SIZE);
				bool add = mm->UpdateSingleUser(gui,left,right);
				ensure("User Updated",add);
				FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,false);
				int ret = ns.GetSizeOfNwMsg(m_irisData);
				BinMessage msg1(ret);
				ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
				MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//				printf("Username %s GUID %s Score %f \n",result->getName().c_str(),result->getGUID().c_str(),result->getScore());
				ensure("match result even should match",0 != memcmp(result->getGUID().c_str(),guid[i],36));
				{
					FillIrisData(comp,coarsefine,matchBuffer1,matchBuffer2,m_irisData,i,true);
					int ret = ns.GetSizeOfNwMsg(m_irisData);
					BinMessage msg1(ret);
					ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
					MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//					printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
					ensure("match result odd should match",0 != memcmp(result->getGUID().c_str(),guid[i],36));
				}
			}
			delete m_irisData;
			SendMsg();
			sleep(1);
			delete nw;
			pthread_cancel(pt);
			remove("./data/testing.db3");
		}
		void Run5A(bool comp,bool coarsefine){
			//printf("Run5A\n");

			cfg.setValue("GRI.MatchMgrDebug","0");
			cfg.setValue("GRI.HDMatcherCount","2");
			cfg.setValue("GRI.HDMatcher.0.Type","REMOTE");
			cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
			cfg.setValue("GRI.HDMatcher.0.Address","localhost:8082");
			cfg.setValue("GRI.HDMatcher.1.Type","LOCAL");
			cfg.setValue("GRI.HDMatcher.1.BuffSize","300000");

			cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
			cfg.setValue("GRI.MatcherFeatureMask","15");
			cfg.setValue("GRI.HDDebug","0");
			cfg.setValue("GRI.DBDebug","0");

			system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/testing.db3");

			cfg1.setValue("GRI.HDMatcherDebug","0");
			if(coarsefine){
				cfg1.setValue("GRI.useCoarseFineMatch","1");
				cfg.setValue("GRI.useCoarseFineMatch","1");
			}else{
				cfg1.setValue("GRI.useCoarseFineMatch","0");
				cfg.setValue("GRI.useCoarseFineMatch","0");

			}
			if(comp){
				cfg1.setValue("GRI.CompressedMatching","1");
				cfg.setValue("GRI.CompressedMatching","1");
			}else{
				cfg1.setValue("GRI.CompressedMatching","0");
				cfg.setValue("GRI.CompressedMatching","0");
			}

			NWHDMatcher *nw = new NWHDMatcher(cfg1);
			pthread_t pt= startHDListener(nw);
			sleep(2);
			while(!nw->GetBindedStatus()){
				usleep(10000);
				printf(".");
			}
			printf("Binded to port 8082 \n");
			string left,right,usrname("Madhav Shanbhag"),acd("THIS IS ACD DATA"),guid("AAAAAAAA-BBBB-BBBB-BBBB-AAAAAAAAAAAA"),acdnop;
			int acdlen=25;
			left.resize(2560);
			memset((void*)left.c_str(),0xEE,2560);
			right.resize(2560);
			memset((void*)right.c_str(),0xEE,2560);
			{
				DBAdapter db;
				db.OpenFile("./data/testing.db3");
				int ret = db.GetUserCount();
				ensure("User count should be 0",ret == 0);
				ret = db.UpdateUser(guid,usrname,left,right,acd,acdlen,acdnop);
				ensure("Inserted in db successfully",ret == 0);
			}


			NwMatchManager proc(cfg);
			proc.init();
			sleep(2);
			MatchManagerInterface *mm = proc.GetMM();
			IrisData *m_irisData =  new IrisData();
			NwMatcherSerialzer ns;
			bool del = mm->DeleteSingleUser(guid);
			ensure("User deleted",del);
			{
				DBAdapter db;
				db.OpenFile("./data/testing.db3");
				int ret = db.GetUserCount();
				ensure("User count should not be 0",ret == 1);
				ret = db.DeleteUser(guid);
				ensure("Deleted in db successfully",ret == 0);
			}
			memcpy((void*)m_irisData->getIris(),(void*)left.c_str(),IRIS_SIZE_INCLUDING_MASK);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			proc.process(&msg1);
			//proc.MainLoop();
			proc.RunHDMDiagnostics();
			ensure("HDM should be in bad state",!proc.getHDMStatus());
			proc.RunR();
			ensure("HDM should be in Good state",proc.getHDMStatus());
//			MatchResult *result=mm->DoMatch((unsigned char *)msg1.GetBuffer());
//			ensure("match result even should match",0 != memcmp(result->getGUID().c_str(),guid,36));
//			printf("Username %s GUID %s \n",result->getName().c_str(),result->getGUID().c_str());
			sleep(1);

			proc.process(&msg1);
			ensure("HDM should be in Good state",proc.getHDMStatus());
			delete m_irisData;
			SendMsg();
			sleep(1);
			delete nw;
			pthread_cancel(pt);
			remove("./data/testing.db3");
		}
		void Run6A(bool comp,bool coarsefine){
			//printf("Run6A\n");

			cfg.setValue("GRI.MatchMgrDebug","0");
			cfg.setValue("GRI.HDMatcherCount","1");
			cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
			cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");

			cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
			cfg.setValue("GRI.MatcherFeatureMask","15");
			cfg.setValue("GRI.HDDebug","0");
			cfg.setValue("GRI.DBDebug","0");

			system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/testing.db3");

			cfg1.setValue("GRI.HDMatcherDebug","0");
			if(coarsefine){
				cfg1.setValue("GRI.useCoarseFineMatch","1");
				cfg.setValue("GRI.useCoarseFineMatch","1");
			}else{
				cfg1.setValue("GRI.useCoarseFineMatch","0");
				cfg.setValue("GRI.useCoarseFineMatch","0");

			}
			if(comp){
				cfg1.setValue("GRI.CompressedMatching","1");
				cfg.setValue("GRI.CompressedMatching","1");
			}else{
				cfg1.setValue("GRI.CompressedMatching","0");
				cfg.setValue("GRI.CompressedMatching","0");
			}

			NwMatchManager proc(cfg);
			proc.init();
			MatchManagerInterface *mm = proc.GetMM();
			IrisData *m_irisData =  new IrisData();
			NwMatcherSerialzer ns;

			string left,right,usrname("Madhav Shanbhag"),acd("THIS IS ACD DATA"),guid("AAAAAAAA-BBBB-BBBB-BBBB-AAAAAAAAAAAA"),acdnop;
			int acdlen=25;
			left.resize(2560);
			memset((void*)left.c_str(),0xEE,2560);
			right.resize(2560);
			memset((void*)right.c_str(),0xEE,2560);
			memcpy((void*)m_irisData->getIris(),(void*)left.c_str(),IRIS_SIZE_INCLUDING_MASK);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);

			{
				system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/testing1.db3");
				DBAdapter db;
				db.OpenFile("./data/testing1.db3");
				int ret = db.GetUserCount();
				ensure("User count should be 0",ret == 0);
				ret = db.UpdateUser(guid,usrname,left,right,acd,acdlen,acdnop);
				ensure("Inserted in db successfully",ret == 0);
				int r = db.SetUpdateUser(guid,UPDATEPERSON);
				ensure("Updated Type ",0 == r);
			}
			BinMessage dbmsg(128);
			int sz = sprintf(dbmsg.GetBuffer(),"RECEIVEUSR;%d;%s;",eUPDATEDB,"./data/testing1.db3");
			dbmsg.SetSize(sz);
			proc.process(&dbmsg);

			proc.process(&msg1);
			MatchResult *result = proc.getMatchResult();
			string name = result->getName();
			string g  = result->getGUID();
//			printf("%s -> %s \n",result->getGUID().c_str(),result->getName().c_str());
			ensure("guid should match",0 == memcmp(result->getGUID().c_str(),guid.c_str(),36));
			ensure("name should match",0 == memcmp(result->getName().c_str(),usrname.c_str(),usrname.length()));

			delete m_irisData;

			remove("./data/testing.db3");
		}
		void Run7A(bool comp,bool coarsefine){
			cfg.setValue("GRI.MatchMgrDebug","0");
			cfg.setValue("GRI.HDMatcherCount","1");
			cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
			cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");

			cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
			cfg.setValue("GRI.MatcherFeatureMask","15");
			cfg.setValue("GRI.HDDebug","0");
			cfg.setValue("GRI.DBDebug","0");

			system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/testing.db3");

			cfg1.setValue("GRI.HDMatcherDebug","0");
			if(coarsefine){
				cfg1.setValue("GRI.useCoarseFineMatch","1");
				cfg.setValue("GRI.useCoarseFineMatch","1");
			}else{
				cfg1.setValue("GRI.useCoarseFineMatch","0");
				cfg.setValue("GRI.useCoarseFineMatch","0");

			}
			if(comp){
				cfg1.setValue("GRI.CompressedMatching","1");
				cfg.setValue("GRI.CompressedMatching","1");
			}else{
				cfg1.setValue("GRI.CompressedMatching","0");
				cfg.setValue("GRI.CompressedMatching","0");
			}

			IrisData *m_irisData =  new IrisData();
			NwMatcherSerialzer ns;

			string left,right,usrname("Madhav Shanbhag"),acd("THIS IS ACD DATA"),guid("AAAAAAAA-BBBB-BBBB-BBBB-AAAAAAAAAAAA"),acdnop;
			int acdlen=25;
			left.resize(2560);
			memset((void*)left.c_str(),0xEE,2560);
			right.resize(2560);
			memset((void*)right.c_str(),0xEE,2560);
			memcpy((void*)m_irisData->getIris(),(void*)left.c_str(),IRIS_SIZE_INCLUDING_MASK);
			int ret = ns.GetSizeOfNwMsg(m_irisData);
			BinMessage msg1(ret);
			ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);
			{
				system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/testing1.db3");
				DBAdapter db;
				db.OpenFile("./data/testing1.db3");
				int ret = db.GetUserCount();
				ensure("User count should be 0",ret == 0);
				ret = db.UpdateUser(guid,usrname,left,right,acd,acdlen,acdnop);
				ensure("Inserted in db successfully",ret == 0);
				int r = db.SetUpdateUser(guid,UPDATEPERSON);
				ensure("Updated Type ",0 == r);
			}

		NwListener nl(cfg);
		DBReceive dbrx(cfg);
		dbrx.init();
		NwMatchManager nmm(cfg);
		nmm.init();

		nl.m_DBDispatcher = &dbrx;
		dbrx.addProcessor(&nmm);

		dbrx.Begin();
		nmm.Begin();
		nl.Begin();
		sleep(1);
		SendDB("./data/testing1.db3");
		sleep(2);

		MatchManagerInterface *mm = nmm.GetMM();
		nmm.process(&msg1);
		MatchResult *result = nmm.getMatchResult();
		string name = result->getName();
		string g  = result->getGUID();
		printf("%s -> %s \n",result->getGUID().c_str(),result->getName().c_str());
		ensure("guid should match",0 == memcmp(result->getGUID().c_str(),guid.c_str(),36));
		ensure("name should match",0 == memcmp(result->getName().c_str(),usrname.c_str(),usrname.length()));

		nl.End();
		nmm.End();
		dbrx.End();
		remove("./data/testing.db3");
		remove("./data/testing1.db3");
		delete m_irisData;

		}
	};
	typedef test_group<DBReceiveTestData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group1("DB Updating TESTS");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		HTTPPOSTMsg msg(1024);
		int len = -1;
		len = sprintf(msg.GetBuffer(),"RELOADDB;%d;%d;%d;%d;%d;",eREPLACEDB,2000,50000,(void*)0xffffffff,0);
		msg.SetSize(len);
		int msgtype;
		int filenumber = -1;
		int sd = NULL;
		int st = NULL;
		int isEnc = 0;

		bool ret= msg.getReloadDBParsedMsg(msgtype, filenumber, sd, st, isEnc);
		ensure("Return should be true",ret);

		ensure("msgtype  should match",msgtype == eREPLACEDB);
		ensure("filenumber should match",filenumber == 2000);
		ensure("sd should match",sd == 50000);
		ensure("st should match",st == 0xffffffff);
		len = sprintf(msg.GetBuffer(),"RELOADDB;%d;%d;%d;%d;%d;",eUPDATEDB,-1,-5,(void*)-6,0);
		ret= msg.getReloadDBParsedMsg(msgtype, filenumber, sd, st, isEnc);
		ensure("Return should be true1",ret);

		ensure("msgtype  should match1",msgtype == eUPDATEDB);
		ensure("filenumber should match1",filenumber == -1);
		ensure("sd should match1",sd == -5);
		ensure("st should match1",st == -6);

//		len = sprintf(passnext.GetBuffer(),"RELOADDB;%d;%d;%d;%d;",eDELETEDB,m_rxfilenumber-1,msg->m_SD,(void*)msg->m_SecureTrait);

	}
	template<>
	template<>
	void testobject::test<2>() {
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDDebug","0");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
		cfg.setValue("GRI.useCoarseFineMatch","1");
		cfg.setValue("GRI.MatcherFeatureMask","15");
		cfg.setValue("GRI.DBDebug","1");

		remove("./data/testing.db3");
#ifndef RUN_STANDALONE
		NwListener nl(cfg);
		DBReceive dbrx(cfg);
		dbrx.init();
		NwMatchManager nmm(cfg);
		nmm.init();

		nl.m_DBDispatcher = &dbrx;
		dbrx.addProcessor(&nmm);

		dbrx.Begin();
		nmm.Begin();
		nl.Begin();
		sleep(1);
		SendDB();
		sleep(1);
		nl.End();
		nmm.End();
		dbrx.End();
#endif
	}
	template<>
	template<>
	void testobject::test<3>() {
		cfg.setValue("GRI.MatchMgrDebug","0");
		cfg.setValue("GRI.HDDebug","0");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/testing.db3");
		cfg.setValue("GRI.useCoarseFineMatch","1");
		cfg.setValue("GRI.MatcherFeatureMask","15");
		cfg.setValue("GRI.DBDebug","1");
		cfg.setValue("NwListener.Debug","1");
		remove("./data/testing.db3");
		int cnt;
		{
			system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3");
			system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/update.db3");
			DBAdapter* db = new DBAdapter();
			db->OpenFile("./data/testing.db3");
			cnt = db->GetUserCount();

			string username,leftiris,rightiris,acd,acdnop;
			int acdlen;
			DBAdapter inc;
			inc.OpenFile("./data/update.db3");
			for(int i=0;i<7;i++){
				string gui(GUIDTOREMOVE[i]);
				int ret = db->GetUserData(gui,username,leftiris,rightiris,acd,acdlen,acdnop);
				ensure("Read data ",0 == ret);
				ret = inc.UpdateUser(gui,username,leftiris,rightiris,acd,acdlen,acdnop);
				ensure("written data ",0 == ret);
				ret = inc.SetUpdateUser(gui,DELETEPERSON);
				ensure("Updated Type ",0 == ret);
			}
			vector<pair<int,string> > retd = inc.GetIncrementalData();
			ensure("The Count should remain same",retd.size() == 7);
			for(int i=0;i<7;i++){
				ensure("Update type should match",retd[i].first == 1);
			}

			db->CloseConnection();
			delete db;
		}
#ifndef RUN_STANDALONE
		NwListener nl(cfg);
		DBReceive dbrx(cfg);
		dbrx.init();
		NwMatchManager nmm(cfg);
		nmm.init();

		nl.m_DBDispatcher = &dbrx;
		dbrx.addProcessor(&nmm);

		dbrx.Begin();
		nmm.Begin();
		nl.Begin();
		sleep(1);
		SendUpdate();
		sleep(2);

		{
			DBAdapter d;
			d.OpenFile("./data/testing.db3");
			int newcnt = d.GetUserCount();
			ensure_equals("There Should be 7 people deleted from db",(newcnt+7),cnt);
		}
		{
			DBAdapter inc;
			inc.OpenFile("./data/update.db3");
			for(int i=0;i<7;i++){
				string gui(GUIDTOREMOVE[i]);
				int ret = inc.SetUpdateUser(gui,UPDATEPERSON);
				ensure("Updated Type ",0 == ret);
			}
			inc.CloseConnection();
		}
		SendUpdate();
		sleep(2);
		{
			DBAdapter d;
			d.OpenFile("./data/testing.db3");
			int newcnt = d.GetUserCount();
			ensure_equals("There Should be 7 people deleted from db",(newcnt),cnt);
		}

		nl.End();
		nmm.End();
		dbrx.End();
		remove("./data/testing.db3");
		remove("./data/update.db3");
#endif
	}
	template<>
	template<>
	void testobject::test<4>() {
		RunAppenDB1(false,false);
		RunAppenDB1(false,true);
		RunAppenDB1(true,false);
		RunAppenDB1(true,true);

	}

	template<>
	template<>
	void testobject::test<5>() {
		Run3A(false,false);
	}
	template<>
	template<>
	void testobject::test<6>() {
		Run3A(false,true);
	}
	template<>
	template<>
	void testobject::test<7>() {
		Run3A(true,false);
	}
	template<>
	template<>
	void testobject::test<8>() {
		Run3A(true,true);
	}

	template<>
	template<>
	void testobject::test<9>() {
		Run5A(false,false);
	}
	template<>
	template<>
	void testobject::test<10>() {
		Run5A(false,true);
	}
	template<>
	template<>
	void testobject::test<11>() {
		Run5A(true,false);
	}
	template<>
	template<>
	void testobject::test<12>() {
		Run5A(true,true);
	}
	template<>
	template<>
	void testobject::test<13>() {
		Run6A(false,false);
	}
	template<>
	template<>
	void testobject::test<14>() {
		Run6A(false,true);
	}
	template<>
	template<>
	void testobject::test<15>() {
		Run6A(true,false);
	}
	template<>
	template<>
	void testobject::test<16>() {
		Run6A(true,true);
	}
	template<>
	template<>
	void testobject::test<17>() {
		Run7A(false,false);
	}
	template<>
	template<>
	void testobject::test<18>() {
		Run7A(false,true);
	}
	template<>
	template<>
	void testobject::test<19>() {
		Run7A(true,false);
	}
	template<>
	template<>
	void testobject::test<20>() {
		Run7A(true,true);
	}

}
