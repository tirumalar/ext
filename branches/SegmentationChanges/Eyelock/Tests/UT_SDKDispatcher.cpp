/*
 * UT_VarienceDetection.cpp
 *
 *  Created on: Jun 1, 2011
 *      Author: developer1
 */

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
#include "SDKDispatcher.h"
#include "NwListener.h"
#include "IrisDBHeader.h"

namespace tut {


struct SDKDispatcherTestData {
	int __dbPort;
	int __selDB;

	TestConfiguration conf;//empty configuration
	SocketServer m_dbserver;
	int m_dbServerThread;
	pthread_t m_thread;

	SDKDispatcherTestData():__dbPort(9000),__selDB(0), m_dbserver(__dbPort,eIPv4)
	{	}

	~SDKDispatcherTestData(){
		m_dbserver.CloseInput();
		m_dbserver.CloseOutput();
	}

	static void OnAcceptClient(Socket& client){
		try	{
			char rcvdbmsg[100];
			char Buff[256];
			//lets see what the client wants
			BinMessage *rcvdMsg = new BinMessage(1024);
			client.Receive(*rcvdMsg);
			printf("A request from the client ---> %s\n",rcvdMsg->GetBuffer());
			ensure("There is a request for updated database",strcmp("MATCHED;GUID:",rcvdMsg->GetBuffer()) == 0);
			delete rcvdMsg;
			client.CloseInput();
			client.CloseOutput();
		}
		catch(NetIOException ex){
			ex.PrintException();
		}
		catch(...){
			printf("Client socket could not be closed properly\n");
		}
	}
	static void *StartListening(void* param){
		SDKDispatcherTestData* ptr = (SDKDispatcherTestData*)param;
		if(ptr)
			ptr->m_dbserver.Accept(OnAcceptClient);
	}
	void StartDBServer(){
		m_dbServerThread = pthread_create(&m_thread, NULL, SDKDispatcherTestData::StartListening, this);
		//m_dbserver.Accept(OnAcceptClient);
	}
};
typedef test_group<SDKDispatcherTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("SDK Dispatcher");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		conf.setValue("Eyelock.SDKDispatcherDebug","1");
		SDKDispatcher sdk(&conf);
		sdk.init();
		sdk.Begin();
		sleep(1);
		MatchResult mr;
		string name("Mamigo");
		string guid("2F1E4FC0-81FD-11DA-9156-00036A0F876A");
		mr.setName(name);
		mr.setGUID(guid);
		//mr.printF2FData();
		mr.setState(PASSED);
		sleep(1);
		bool ret = sdk.enqueMsg(mr);
		ensure("Enque msg for Match Event is proper\n",ret == true);
		sdk.End();


	}
	template<>
	template<>
	void testobject::test<2>() {

		conf.setValue("Eyelock.SDKDispatcherDebug","1");
		StartDBServer();
		SDKDispatcher sdk(&conf);
		sdk.init();
		sdk.Begin();
		sleep(1);
		sdk.AppendMatchServer("localhost:9000");
		MatchResult mr;
		string name("Mamigo");
		string guid("2F1E4FC0-81FD-11DA-9156-00036A0F876A");
		mr.setName(name);
		mr.setGUID(guid);
		int type = -1;
		int bytelen, bitlen;
		char * testData = getACSTestData(bytelen, bitlen);
		mr.setF2F(testData);
		mr.setState(PASSED);
		if(testData) delete [] testData;
		bool ret = sdk.enqueMsg(mr);
		sleep(1);
		ensure("Enque msg for Match Event is proper with network\n",ret == true);
		sdk.End();
	}

	template<>
	template<>
	void testobject::test<3 >() {
		printf ("SDK Dispatcher test 3\n");
			conf.setValue("Eyelock.SDKDispatcherDebug","1");
			StartDBServer();
			conf.setValue("Eyelock.SDKRegisterIPs","./SDKRegisterIPs.txt");
			std::string path=conf.getValue("Eyelock.SDKRegisterIPs", "./SDKRegisterIPs.txt");
			SDKDispatcher sdk(&conf);
			sdk.init();
			sdk.Begin();
			sleep(1);
			sdk.AppendMatchServer("localhost:9000");
			MatchResult mr;
			string name("Mamigo");
			string guid("2F1E4FC0-81FD-11DA-9156-00036A0F876A");
			mr.setName(name);
			mr.setGUID(guid);
			//mr.setKey("123585868645",12);
			int type = -1;
			int bytelen, bitlen;
			char *testData = getACSTestData(bytelen, bitlen);
			mr.setF2F(testData);
			mr.setState(PASSED);
			if(testData) delete [] testData;
			bool ret = sdk.enqueMsg(mr);
			ensure("Enque msg for Match Event and cleared everything from match server is proper\n",ret == true);
			sleep(2);
			sdk.ClearMatchServer();
			sdk.End();
			remove(path.c_str());
		}

	template<>
	template<>
	void testobject::test<4>() {
		printf ("SDK Dispatcher test 4\n");
			conf.setValue("Eyelock.SDKDispatcherDebug","1");
			SDKDispatcher sdk(&conf);
			sdk.init();
			sdk.Begin();
			sleep(1);
			std::set<std::string>TamperEventAddr;
			TamperEventAddr = sdk.GetTampreAddressList();
			int32_t Ts = TamperEventAddr.size();
			printf("size of dataset %d \n",Ts);
			ensure("size of dataset \n",Ts == 0);
			sdk.ClearTamperServer();
			TamperEventAddr = sdk.GetTampreAddressList();
			sleep(1);
			bool ret = sdk.DeleteTamperServer("192.168.9.54:8881");
			ensure("Bad Test...Delete Tamper Server without Appending \n",ret == false);
			sdk.End();
		}

	template<>
	template<>
	void testobject::test<5>() {
		printf ("SDK Dispatcher test 5\n");
		conf.setValue("Eyelock.SDKRegisterIPs","./data/SDKRegisterIPs.txt");
		std::string path=conf.getValue("Eyelock.SDKRegisterIPs", "./SDKRegisterIPs.txt");
		conf.setValue("Eyelock.SDKDispatcherDebug","1");
		SDKDispatcher sdk(&conf);
		sdk.init();
		sdk.Begin();
		sleep(1);
		FILE *fPath = fopen("./tamper","w");
		fclose(fPath);
		std::set<std::string>TamperEventAddr;
		TamperEventAddr = sdk.GetTampreAddressList();
		int32_t Ts = TamperEventAddr.size();
		printf("size of dataset %d \n",Ts);
		sdk.AppendTamperServer("localhost:9000");
		sleep(1);
		TamperEventAddr = sdk.GetTampreAddressList();
		sleep(1);
	    Ts = TamperEventAddr.size();
		printf("size of dataset after appending %d \n",Ts);
		ensure("size of dataset \n",Ts == 1);
		bool del = sdk.DeleteTamperServer("localhost:9000");
		TamperEventAddr = sdk.GetTampreAddressList();
		sleep(1);
	    Ts = TamperEventAddr.size();
		printf("size of dataset %d \n",Ts);
		remove("fpath");
		remove(path.c_str());
		ensure("Good Test...Delete Tamper Server at obtained index is proper\n",del == true);
		sdk.End();
	}

	template<>
	template<>
	void testobject::test<6>() {
		printf ("SDK Dispatcher test 6\n");
		conf.setValue("Eyelock.SDKDispatcherDebug","1");
		conf.setValue("Eyelock.SDKRegisterIPs","./data/SDKRegisterIPs.txt");
		std::string path=conf.getValue("Eyelock.SDKRegisterIPs", "./data/SDKRegisterIPs.txt");
		SDKDispatcher sdk(&conf);
		sdk.init();
		sdk.Begin();
		sleep(1);
		MatchResult mr;
		string name("Mamigo");
		string guid("2F1E4FC0-81FD-11DA-9156-00036A0F876A");
		mr.setName(name);
		mr.setGUID(guid);
		int type = -1;
		int bytelen, bitlen;
		char * testData = getACSTestData(bytelen, bitlen);
		mr.setF2F(testData);
		mr.setState(PASSED);
		if(testData) delete [] testData;
		sdk.enqueMsg(mr);
		sleep(1);
		sdk.ClearMatchServer();
		std::set<std::string>MatchEventAddr;
		MatchEventAddr = sdk.GeMatchAddressList();
		int32_t ms = MatchEventAddr.size();
		ensure("size of dataset \n",ms == 0);
		printf("size of dataset %d \n",ms);
		sdk.AppendMatchServer("localhost:9000");
		sdk.AppendMatchServer("localhost:8088");
		sdk.AppendMatchServer("192.168.4.5:4348");
		MatchEventAddr = sdk.GeMatchAddressList();
		ms = MatchEventAddr.size();
		printf("size of dataset %d \n",ms);
		ensure("size of dataset \n",ms == 3);
		sleep(1);
		bool del = sdk.DeleteMatchServer("localhost:8088");
		MatchEventAddr = sdk.GeMatchAddressList();
		ms = MatchEventAddr.size();
		ensure("size of dataset \n",ms == 2);
		ensure("Good Test... Delete Match Server at obtained index is proper\n",del == true);
		remove(path.c_str());
		sdk.End();
	}
}
