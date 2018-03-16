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
#include "LEDDispatcher.h"
#include "CommonDefs.h"
#include "socket.h"
#include <unistd.h>


namespace tut {
struct LEDTestData {
	TestConfiguration cfg;//empty configuration
	MatchResult mr;
	LEDTestData() {
	}
	~LEDTestData() {
	}
};
typedef test_group<LEDTestData,5> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("LED Processor tests");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("simple queue tests");
	LEDDispatcher proc(cfg);
	proc.init();

	// proc is now running on a different thread
	proc.Begin();

	printf("white\n");
	sleep(2);

	mr.setState(PASSED);
	proc.enqueMsg(mr);
	printf("Green To white \n");
	sleep(2);

	mr.setState(DETECT);
	proc.enqueMsg(mr);
	printf("Blue to white \n");
	sleep(2);

	mr.setState(PASSED);
	proc.enqueMsg(mr);
	printf("Green to White\n");
	sleep(2);

	printf("Now testing priorities\n");
	mr.setState(DETECT);
	proc.enqueMsg(mr);
	mr.setState(DETECT);
	proc.enqueMsg(mr);
	printf("Should have dropped DETECT\n");

	mr.setState(PASSED);
	proc.enqueMsg(mr);
	printf("Should have overwritten by PASSED\n");
	sleep(2);
	proc.End();

}
#ifdef __ARM__
template<>
template<>
void testobject::test<2>() {

	set_test_name("timeout and peek queue tests");

	cfg.setValue("GRI.LEDDebug", "1");
	LEDDispatcher proc(cfg);
	proc.init();

	proc.Begin();

	printf("Now testing timeout\nBlue state should persist for 5 seconds\n");
	mr.setState(DETECT);
	proc.enqueMsg(mr);
	sleep(2);
	ensure_equals(proc.m_testState,DETECT);
	sleep(5);
	ensure_equals(proc.m_testState,INITIAL);


	printf("Now testing peek behaviour\n");
	mr.setState(DETECT);
	proc.enqueMsg(mr);
	sleep(2);
	ensure_equals(proc.m_testState,DETECT);

	mr.setState(PASSED);
	proc.enqueMsg(mr);
	printf("should flicker Green and return to white\n");
	sleep(2);
	ensure_equals(proc.m_testState,INITIAL);

	printf("Now testing flickering of blue\n");
	mr.setState(DETECT);
	proc.enqueMsg(mr);
	sleep(2);
	proc.enqueMsg(mr);
	sleep(2);
	proc.enqueMsg(mr);
	sleep(2);
	ensure_equals(proc.m_testState,DETECT);
	printf("blue should not have flickered\n");

	proc.End();
}
#endif

template<>
template<>
void testobject::test<3>() {
	set_test_name("NwSetTest");
	cfg.setValue("GRI.LEDDebug","1");

	LEDDispatcher proc(cfg);
	proc.init();

	// proc is now running on a different thread
	proc.Begin();
	sleep(2);
	mr.setState(CONFUSION);
	mr.setNwValandSleep(255,2000);
	proc.enqueMsg(mr);
	sleep(8);
	proc.End();

}
template<>
template<>
void testobject::test<4>() {
	set_test_name("Lotering SetTest");
	cfg.setValue("GRI.LEDDebug","1");

	LEDDispatcher proc(cfg);
	proc.init();

	// proc is now running on a different thread
	proc.Begin();
	sleep(2);
	mr.setState(LOITERING);
	mr.setNwValandSleep(24,2000000);
	proc.enqueMsg(mr);
	sleep(8);
	proc.End();

}



}
