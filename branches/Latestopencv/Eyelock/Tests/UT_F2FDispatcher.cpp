/*
 * UT_SafePtr.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "NwListener.h"
#include "F2FDispatcher.h"
#include "CommonDefs.h"
#include "socket.h"
#include <unistd.h>

namespace tut {
struct F2FTestData {
	TestConfiguration cfg;//empty configuration
	F2FTestData() {
		cfg.setValue("GRITrigger.F2FEnable","true");
		cfg.setValue("Eyelock.SystemReadyFrequencySec","5");
		cfg.setValue("Eyelock.TimeDelayBetweenMsgMilliSec","200");
		cfg.setValue("Eyelock.SystemReadyDebug","1");
		cfg.setValue("GRITrigger.F2FDebug","1");
		cfg.setValue("Eyelock.SystemReadyCardData","0x0023123456789abcdef");
	}
	~F2FTestData() {
	}
};
typedef test_group<F2FTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("F2F Processor tests");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		set_test_name("F2F queue tests");
		F2FDispatcher proc(cfg);
		proc.init();/// proc is now running on a different thread
		proc.Begin();
		unsigned int ts = proc.getPreviousTS();
		sleep(6);
		unsigned int ts1 = proc.getPreviousTS();
		//ensure("TimeStamp is not same ",!(ts1 == ts));
		while(1){
			sleep(1000);
		}
		proc.End();
	}

	template<>
	template<>
	void testobject::test<2>() {
		set_test_name("F2F queue tests1");
		F2FDispatcher proc(cfg);
		proc.init();
		proc.Begin();
		unsigned int ts = proc.getPreviousTS();
		sleep(1);
		unsigned int ts1 = proc.getPreviousTS();
		sleep(1);
		ts = proc.getPreviousTS();
		ensure("TimeStamp is not same 1",(ts1 == ts));
		sleep(1);
		ts1 = proc.getPreviousTS();
		ensure("TimeStamp is not same 2",(ts1 == ts));
		proc.End();
	}

	template<>
	template<>
	void testobject::test<3>() {
		set_test_name("F2F test");
		F2FDispatcher proc(cfg);
		proc.init();/// proc is now running on a different thread
		proc.SetSendingEveryNSec(false);
		proc.Begin();
		int i=0;
		while (i++ < 10){
			unsigned int ts = proc.getPreviousTS();
			sleep(1);
			unsigned int ts1 = proc.getPreviousTS();
			ensure("TimeStamp is ZERO ",ts1 == 0);
			ensure("TimeStamp is ZERO ",ts == 0);
		}
		proc.End();
	}

}
