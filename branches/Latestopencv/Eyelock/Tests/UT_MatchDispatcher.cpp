#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include <iostream>
#include <socket.h>
#include "HTTPPOSTMsg.h"
#include "MatchDispatcher.h"
#include "LEDDispatcher.h"
#include "F2FDispatcher.h"
#include "NwDispatcher.h"
#include "string.h"
#include <vector>
#include <unistd.h>
using namespace std;


extern "C" {
#include "file_manip.h"
};

#include "HTTPPOSTMsg.h"
#include "socket.h"

extern "C" {
#include "file_manip.h"
}

namespace tut {
struct TestMatchDisp{

	void init()
	{

		m_ledDisp = new LEDDispatcher(cfg);
		m_f2fDisp = new F2FDispatcher(cfg);
		m_nwDisp = new NwDispatcher(cfg);
		m_matchDisp = new MatchDispatcher(cfg);

		m_ledDisp->init();
		m_f2fDisp->init();
		m_nwDisp->init();
		m_matchDisp->init();

		m_matchDisp->addProcessor(m_ledDisp);
		m_matchDisp->addProcessor(m_f2fDisp);
		m_matchDisp->addProcessor(m_nwDisp);

		m_ledDisp->Begin();
		m_f2fDisp->Begin();
		m_nwDisp->Begin();

	}

	void term()
	{

		if(m_ledDisp)
		{
			m_ledDisp->End();
			delete m_ledDisp;
		}

		if(m_f2fDisp)
		{
			m_f2fDisp->End();
			delete m_f2fDisp;
		}

		if(m_nwDisp)
		{
			m_nwDisp->End();
			delete m_nwDisp;
		}
		if(m_matchDisp)
		{
			m_matchDisp->End();
			delete m_matchDisp;
		}

	}

	TestMatchDisp()
	: m_f2fserver(8090,eIPv4)
	, m_nwserver(8091,eIPv4)
	, m_slaveserver(8092,eIPv4)
	{
		cfg.setValue("GRI.irisCodeDatabaseFile","../Eyelock/data/PDB22.bin");
		cfg.setValue("GRITrigger.F2FEnable","1");
		cfg.setValue("Eyelock.WeigandDestAddr","0.0.0.0:8090");// a dummy address
		cfg.setValue("GRI.MatchResultDestAddr", "0.0.0.0:8091");
		cfg.setValue("GRI.SlaveAddressList","0.0.0.0:8092");// a dummy address for slave
		cfg.setValue("GRI.NwDebug","1");
		//cfg.setValue("GRI.LEDDebug","1");

		init();
	}
	~TestMatchDisp() {

		term();
		m_f2fserver.CloseInput();
		m_f2fserver.CloseOutput();
		m_nwserver.CloseInput();
		m_nwserver.CloseOutput();
		m_slaveserver.CloseInput();
		m_slaveserver.CloseOutput();
	}

	static void OnAcceptClient(Socket& client)
		{
			try
			{
				int sz = 256;

				//lets see what the client wants
				BinMessage *rcvdMsg = new BinMessage(sz);
				client.Receive(*rcvdMsg);

				std::string clientResp(rcvdMsg->GetBuffer());
				if(clientResp.compare(0,3,"F2F") != 0)
				{
					if(clientResp.compare(0,7,"Matched") == 0)
					{
						delete rcvdMsg;
						client.CloseInput();
						client.CloseOutput();
						return;
					}
					else if(clientResp.compare(0,12,"RESETEYELOCK") == 0)
					{
						;
					}
					else
						ensure("Unknown message sent by the client",0);
				}

				printf("A request from the client ---> %s\n",rcvdMsg->GetBuffer());
				delete rcvdMsg;
				client.CloseInput();
				client.CloseOutput();
			}
			catch(NetIOException ex)
			{
				ex.PrintException();
			}
			catch(...)
			{
				printf("Client socket could not be closed properly\n");
			}
		}
		static void *StartF2FListening(void* param)
		{
			try
			{
				TestMatchDisp* ptr = (TestMatchDisp*)param;
				if(ptr)
					ptr->m_f2fserver.Accept(OnAcceptClient);
			}
			catch(...)
			{
				cout << "\nStartF2FListening Generic exception\n";
			}
		}
		void StartF2FServer()
		{
			m_f2fServerThread = pthread_create(&m_f2fthread, NULL, TestMatchDisp::StartF2FListening, this);
		}

		static void *StartNWListening(void* param)
		{
			try
			{
				TestMatchDisp* ptr = (TestMatchDisp*)param;
				if(ptr)
					ptr->m_nwserver.Accept(OnAcceptClient);
			}
			catch(...)
			{
				cout << "\nStartNWListening Generic Exception\n";
			}
		}

		void StartNWServer()
		{
			m_nwServerThread = pthread_create(&m_nwthread, NULL, TestMatchDisp::StartNWListening, this);
		}

		static void *StartSlaveListening(void* param)
		{
			try
			{
				TestMatchDisp* ptr = (TestMatchDisp*)param;
				if(ptr)
					ptr->m_slaveserver.Accept(OnAcceptClient);
			}
			catch(...)
			{
				cout << "StartSlaveListening Generic Exception\n";
			}
		}

		void StartSlaveServer()
		{
			m_slaveServerThread = pthread_create(&m_slavethread, NULL, TestMatchDisp::StartSlaveListening, this);
		}

		void StartServers()
		{
			this->StartF2FServer();
			this->StartNWServer();
			this->StartSlaveServer();
			sleep(2);
		}
		void WaitForServers()
		{
			pthread_join (m_f2fthread, NULL);
			pthread_join (m_nwthread, NULL);
			pthread_join (m_slavethread, NULL);
		}

		void FillMatchResults()
		{
			for(int i = 0; i<10;i++)
				m_resultItems.push_back(new MatchResult());
		}

		void ClearMatchResults()
		{
			std::vector<MatchResult*>::iterator iter = m_resultItems.begin();
			for(;iter != m_resultItems.end();iter++)
			{
				delete (*iter);
			}

		}

	std::vector<MatchResult*> m_resultItems;
	TestConfiguration cfg;//empty configuration

	LEDDispatcher* m_ledDisp;
	F2FDispatcher* m_f2fDisp;
	NwDispatcher* m_nwDisp;
	MatchDispatcher *m_matchDisp;


	SocketServer m_f2fserver;
	int m_f2fServerThread;
	pthread_t m_f2fthread;

	SocketServer m_nwserver;
	int m_nwServerThread;
	pthread_t m_nwthread;

	SocketServer m_slaveserver;
	int m_slaveServerThread;
	pthread_t m_slavethread;

};

typedef test_group<TestMatchDisp> tg;
typedef tg::object testobject;
}

namespace {
tut::tg test_group("Match dispatcher test");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>()
	{
		StartServers();
		set_test_name("Test F2F and Network disp");

		MatchResult mr;
		mr.setState(DETECT);
		mr.init();
		CURR_TV_AS_USEC(curr);
		mr.setTimeStamp(curr);

		m_matchDisp->enqueMsg(mr);

		m_matchDisp->ProcessMatchedItem();

		WaitForServers();
	}

	template<>
	template<>
	void testobject::test<2>()
	{
		StartServers();

		set_test_name("Enforce dual eye matching , alternate eyes queued");

		m_matchDisp->SetDualEyeMatch(true);

		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();
		int count = 0;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));

			count = (count == 1) ? 0:1;
		}


		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
		WaitForServers();
	}

	template<>
	template<>
	void testobject::test<3>()
	{
		StartServers();

		set_test_name("Enforce dual eye matching , 1odd+9even eyes");
		m_matchDisp->SetDualEyeMatch(true);

		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();

		int count = 1;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));

			count = 0;
		}

		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
		WaitForServers();
	}

	template<>
	template<>
	void testobject::test<4>()
	{
		StartServers();

		set_test_name("Enforce dual eye matching , 9odd+1even eyes");
		m_matchDisp->SetDualEyeMatch(true);

		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();

		int count = 0;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));

			count = 1;
		}

		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
		WaitForServers();
	}

	template<>
	template<>
	void testobject::test<5>()
	{
		set_test_name("Enforce dual eye matching , all odd eyes");
		m_matchDisp->SetDualEyeMatch(true);

		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();

		int count = 5;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));
		}

		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
	}

	template<>
	template<>
	void testobject::test<6>()
	{
		set_test_name("Enforce dual eye matching , all even eyes");
		m_matchDisp->SetDualEyeMatch(true);
		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();

		int count = 8;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));
		}

		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
	}

	template<>
	template<>
	void testobject::test<7>()
	{
		set_test_name("Enforce dual eye matching , valid eyes with match result exceeding the threshold time stamp");
		m_matchDisp->SetDualEyeMatch(true);

		std::vector<MatchResult*> resultItems;

		FillMatchResults();

		std::vector<MatchResult*>::iterator iter = m_resultItems.begin();

		int count = 0;
		for(;iter != m_resultItems.end();iter++)
		{
			(*iter)->setState(DETECT);
			(*iter)->init(count);
			CURR_TV_AS_USEC(curr);
			(*iter)->setTimeStamp(curr);
			m_matchDisp->enqueMsg(*(*iter));
			count = (count == 1)?0:1;
			usleep(500000);
		}

		m_matchDisp->ProcessMatchedItem();

		ClearMatchResults();
	}
	template<>
	template<>
	void testobject::test<8>()
	{
		set_test_name("Check F2F Message");
		MatchResult mr;
		char *req = mr.getKey();
		req[0] = 35;
		req[1] = 0x0;
		req[2]= 0x12;
		req[3]= 0x34;
		req[4]= 0x56;
		req[5]= 0x78;
		req[6]= 0x9A;

		BinMessage msg(256);
		m_matchDisp->MakeNwF2FMsg(&mr,&msg);

		char *ptr1 = msg.GetBuffer();
		char ptr[12];
		ptr[0] = 'F';
		ptr[1] = '2';
		ptr[2] = 'F';
		ptr[3] = ';';
		ptr[4]= req[0];
		ptr[5]= req[1];
		ptr[6]= req[2];
		ptr[7]= req[3];
		ptr[8]= req[4];
		ptr[9]= req[5];
		ptr[10]= req[6];
		ptr[11]= ';';

		int ret = memcmp(ptr,ptr1,12);
		ensure("F2F Generated String ",0 == ret);

	}

}
