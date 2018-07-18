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
#include <tut.h>
#include "NwListener.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "HTTPPOSTMsg.h"
#include "MatchProcessor.h"
#include "LEDDispatcher.h"
#include "TestDispatcher.h"
#include "ImageMsgWriter.h"
#include "BiOmega.h"
#include "F2FDispatcher.h"

extern "C" {
#include "file_manip.h"
}

namespace tut {
struct TestDataNW {
	TestConfiguration cfg;//empty configuration
	HTTPPOSTMsg *msg;
	F2FDispatcher *pF2FDispatcher;
	TestDataNW() {
		cfg.setValue("GRI.cropWidth", "64");
		cfg.setValue("GRI.cropHeight", "48");
		cfg.setValue("GRITrigger.F2FNumBytes","12");
		cfg.setValue("GRITrigger.F2FEnable","true");
		msg = new HTTPPOSTMsg(HTTPPOSTMsg::calcMessageSize(640, 480));
	}
	~TestDataNW() {
		delete msg;
	}
};
typedef test_group<TestDataNW, 20> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("N/W tests");
}

namespace tut {

template<>
template<>
void testobject::test<1>() {
	set_test_name("HTTPPostMsg F2F test");
	const char* rcvdbmsg = "F2F;1;ABCDEFGHIJKLMNOPQ";
	pF2FDispatcher =new F2FDispatcher(cfg);
	pF2FDispatcher->init();


	msg->SetData(rcvdbmsg, strlen(rcvdbmsg));
	unsigned char *ptr = (unsigned char *)msg->GetBuffer();
	ptr[4]= 96;
	ptr[5]= 0;
	ensure("msg->IsDone", msg->IsDone());
	NwListener* proc = new NwListener(cfg);
	proc->m_F2FDispatcher = pF2FDispatcher;


	proc->HandleMessage(*msg);
	//proc.HandleMessage()
//	printf("result -> %.*s\n",proc.m_f2fResult.m_key+2);
//	for(int i=0;i<8;i++)
//	{	printf("%c ",proc.m_f2fResult.m_key[i+2]);
//		ensure_equals("Key val",proc.m_f2fResult.m_key[i+2],rcvdbmsg[i+6]);
//	}
	delete proc;
	pF2FDispatcher->End();
	delete pF2FDispatcher;
}


template<>
template<>
void testobject::test<2>() {
	set_test_name("HTTPPostMsg F2F test");
	const char* rcvdbmsg = "F2F;ABCDEFGHIJKLMNOPQ";

	msg->SetData(rcvdbmsg, strlen(rcvdbmsg));
	ensure("msg->IsDone", msg->IsDone());
	ensure_equals("msg->getMsgType() ", msg->getMsgType(), F2F_MSG);
	ensure_equals("msg->isHB ", msg->isHeartBeat(), false);
	ensure_equals("msg->isLED ", msg->isLED(), false);
	ensure_equals("msg->isRELOAD ", msg->isReloadDB(), false);
	ensure_equals("msg->isRECVDB ", msg->isRecvDB(), false);
	ensure_equals("msg->isFaceGrab ", msg->isFaceGrab(), false);
	ensure_equals("msg->isDOENROLL ", msg->isDoEnroll(), false);
	ensure_equals("msg->isSETPIN ", msg->isSetPin(), false);
	ensure_equals("msg->isF2F ", msg->isF2F(), true);
}

template<>
template<>
void testobject::test<3>() {
	set_test_name("HTTPPostMsg SETPIN test");
	const char* rcvdbmsg = "SETPIN;J5;0;0";

	msg->SetData(rcvdbmsg, strlen(rcvdbmsg));
	ensure("msg->IsDone", msg->IsDone());
	ensure_equals("msg->getMsgType() ", msg->getMsgType(), SETPIN_MSG);
	ensure_equals("msg->isHB ", msg->isHeartBeat(), false);
	ensure_equals("msg->isLED ", msg->isLED(), false);
	ensure_equals("msg->isRELOAD ", msg->isReloadDB(), false);
	ensure_equals("msg->isRECVDB ", msg->isRecvDB(), false);
	ensure_equals("msg->isFaceGrab ", msg->isFaceGrab(), false);
	ensure_equals("msg->isDOENROLL ", msg->isDoEnroll(), false);
	ensure_equals("msg->isF2F ", msg->isF2F(), false);
	ensure_equals("msg->isSETPIN ", msg->isSetPin(), true);

}

template<>
template<>
void testobject::test<4>() {
	set_test_name("HTTPPostMsg LEDNwtest");
	const char* ledmsg = "LEDSET;4;240;100";

	msg->SetData(ledmsg, strlen(ledmsg));
	ensure("msg->IsDone", msg->IsDone());
	ensure_equals("msg->getMsgType() ", msg->getMsgType(), LED_MSG);
	ensure_equals("msg->isHB ", msg->isHeartBeat(), false);
	ensure_equals("msg->isLED ", msg->isLED(), true);
	ensure_equals("msg->isRELOAD ", msg->isReloadDB(), false);
	ensure_equals("msg->isRECVDB ", msg->isRecvDB(), false);
	ensure_equals("msg->isFaceGrab ", msg->isFaceGrab(), false);
	ensure_equals("msg->isDOENROLL ", msg->isDoEnroll(), false);
	ensure_equals("msg->isF2F ", msg->isF2F(), false);
	ensure_equals("msg->isSETPIN ", msg->isSetPin(), false);
	int val,sleep;
	int ret = msg->getLEDMessageData(val,sleep);
	ensure_equals("msg->LED value ", val, 240);
	ensure_equals("msg->LED value ", sleep, 100000);
	ensure_equals("msg->LED ret ", ret, 0);
}
template<>
template<>
void testobject::test<5>() {
	set_test_name("HTTPPostMsg Calibration Msg");
	const char* calmsg = "CALIBRATION;0;2500;500;0;";

	msg->SetData(calmsg, strlen(calmsg));
	ensure("msg->IsDone", msg->IsDone());
	ensure_equals("msg->getMsgType() ", msg->getMsgType(), CALIBRATION_MSG);
	ensure_equals("msg->isHB ", msg->isHeartBeat(), false);
	ensure_equals("msg->isLED ", msg->isLED(), false);
	ensure_equals("msg->isRELOAD ", msg->isReloadDB(), false);
	ensure_equals("msg->isRECVDB ", msg->isRecvDB(), false);
	ensure_equals("msg->isFaceGrab ", msg->isFaceGrab(), false);
	ensure_equals("msg->isDOENROLL ", msg->isDoEnroll(), false);
	ensure_equals("msg->isF2F ", msg->isF2F(), false);
	ensure_equals("msg->isSETPIN ", msg->isSetPin(), false);
	ensure_equals("msg->isCalibration ", msg->isCalibration(),true);

	int enable,flashtime,triggertime,led;
	bool ret = msg->getCalibrationData(enable,flashtime,triggertime,led);
	ensure_equals("msg->enable ", enable, 0);
	ensure_equals("msg->flashtime ", flashtime, 2500);
	ensure_equals("msg->triggertime ", triggertime, 500);
	ensure_equals("msg->ledval ", led, 0);
	ensure("msg->enable",enable==0);
}

}
