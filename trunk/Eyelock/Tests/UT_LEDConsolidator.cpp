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
#include "Configuration.h"
#include "SocketFactory.h"
#include "LEDConsolidator.h"
#include <unistd.h>
extern "C" {
#include "file_manip.h"
}

namespace tut {
struct LEDConsolidatorTestData {
		TestConfiguration cfg;//empty configuration
		LEDConsolidatorTestData() {
			cfg.setValue("Eyelock.LoiteringQSize", "2");
			cfg.setValue("GRI.cameraID","CAMERA1");
			cfg.setValue("Eyelock.LoiteringCardData","0x00123456789ABCDEF");
			cfg.setValue("Eyelock.LoiteringDebug","1");
		}
		~LEDConsolidatorTestData() {
		}
	};
	typedef test_group<LEDConsolidatorTestData>tg;
	typedef tg::object testobject;
}
namespace {
	tut::tg test_group("LED Consolidator Tests");
}

namespace tut {

	template<>
	template<>
	void testobject::test<1>() {
		//skip();
		set_test_name("SimpleConSolidation Test");
		cfg.setValue("Eyelock.Type", "NTSGLM");
		LEDConsolidator proc(cfg);
		LEDResult mr;
		mr.setState(LED_FAILED);
		proc.enqueMsg(mr);

		proc.ConsolidateState();
		ensure_equals("State should be LED_DETECT",LED_DETECT,proc.GetState());
		sleep(3);
		ensure_equals("State should be LED_DETECT",LED_FAILED,proc.GetPrevState());
		proc.DispatchToAll();
		ensure_equals("State should be LED_DETECT",LED_FAILED,proc.GetPrevState());
		mr.setState(LED_DETECT);
		proc.enqueMsg(mr);
		mr.setState(LED_LOITERING);
		proc.enqueMsg(mr);
		proc.ConsolidateState();
		ensure_equals("State should be LED_DETECT",LED_FAILED,proc.GetPrevState());
		ensure_equals("State should be LED_LOITERING",LED_LOITERING,proc.GetState());
		proc.DispatchToAll();
		ensure_equals("State should be LED_DETECT",LED_FAILED,proc.GetPrevState());

		mr.setState(LED_DETECT);
		proc.enqueMsg(mr);
		mr.setState(LED_CONFUSION);
		proc.enqueMsg(mr);
		ensure_equals("State should be LED_DETECT",LED_FAILED,proc.GetPrevState());
		proc.ConsolidateState();
		ensure_equals("State should be LED_CONFUSION",LED_CONFUSION,proc.GetState());

		mr.setState(LED_PASSED);
		proc.enqueMsg(mr);
		mr.setState(LED_CONFUSION);
		proc.enqueMsg(mr);
		proc.ConsolidateState();
		ensure_equals("State should be MATCHED",LED_PASSED,proc.GetState());
		proc.DispatchToAll();
		ensure_equals("State should be MATCHED",LED_FAILED,proc.GetPrevState());
	}
}
