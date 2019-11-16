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
#include "MatchProcessor.h"
#include "BiOmega.h"

extern "C" {
#include "file_manip.h"
}

namespace tut {
struct MatchProcCfgTestData {
	TestConfiguration cfg;//empty configuration
	MatchProcCfgTestData() {
		cfg.setValue("server.inQSize", "1");
		cfg.setValue("GRI.cropWidth", "640");
		cfg.setValue("GRI.cropHeight", "480");
		cfg.setValue("GRI.cameraID", "cam_12");
		cfg.setValue("GRI.MatchMgrDebug","1");
		char *fname = "./data/sqlite.db3";
		cfg.setValue("GRI.irisCodeDatabaseFile",fname);
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","500000");


	}
	~MatchProcCfgTestData() {
	}
};
typedef test_group<MatchProcCfgTestData, 4> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("Match Processor config tests");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("Setting Search ROI Tests");
	CvRect SearchArea;
	MatchProcessor proc(cfg);
	proc.GetbioInstance()->GetEyeLocationSearchArea(SearchArea.x,SearchArea.y,SearchArea.width,SearchArea.height);
	ensure_equals("x",240, SearchArea.x);
	ensure_equals("y",180, SearchArea.y);
	ensure_equals("width",160, SearchArea.width);
	ensure_equals("height",120, SearchArea.height);

}
template<>
template<>
void testobject::test<2>() {
	set_test_name("Setting Search ROI Tests1");
	CvRect SearchArea;
	cfg.setValue("GRI.EyeLocationSearchArea.x", "0");
	cfg.setValue("GRI.EyeLocationSearchArea.y", "600");
	cfg.setValue("GRI.EyeLocationSearchArea.width", "20");
	cfg.setValue("GRI.EyeLocationSearchArea.height", "400");
	cfg.setValue("GRI.irisCodeAllocSize", "16000");
	MatchProcessor proc(cfg);

	proc.GetbioInstance()->GetEyeLocationSearchArea(SearchArea.x,SearchArea.y,SearchArea.width,SearchArea.height);
	ensure_equals("x",240, SearchArea.x);
	ensure_equals("y",180, SearchArea.y);
	ensure_equals("width",160, SearchArea.width);
	ensure_equals("height",120, SearchArea.height);
}
template<>
template<>
void testobject::test<3>() {
	set_test_name("Setting Search ROI Tests2");
	CvRect SearchArea;
	cfg.setValue("GRI.EyeLocationSearchArea.x", "0");
	cfg.setValue("GRI.EyeLocationSearchArea.y", "0");
	cfg.setValue("GRI.EyeLocationSearchArea.width", "300");
	cfg.setValue("GRI.EyeLocationSearchArea.height", "400");
	cfg.setValue("GRI.irisCodeAllocSize", "16000");
	MatchProcessor proc(cfg);

	proc.GetbioInstance()->GetEyeLocationSearchArea(SearchArea.x,SearchArea.y,SearchArea.width,SearchArea.height);

	ensure_equals("x",0, SearchArea.x);
	ensure_equals("y",0, SearchArea.y);
	ensure_equals("width",300, SearchArea.width);
	ensure_equals("height",400, SearchArea.height);

}

template<>
template<>
void testobject::test<4>() {
	set_test_name("Setting Corrupt Bits percentage");

	{
		MatchProcessor proc(cfg);
		ensure_equals("GetMaxCorruptBitsPercAllowed defaults",proc.GetbioInstance()->GetMaxCorruptBitsPercAllowed(),70);
	}

	{
		cfg.setValue("GRI.MaxCorruptBitPercentage", "50");
		MatchProcessor proc(cfg);
		ensure_equals("set MaxCorruptBitsPercAllowed",proc.GetbioInstance()->GetMaxCorruptBitsPercAllowed(),50);
	}
}
}
