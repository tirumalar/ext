/*
 * UT_Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <string.h>
extern "C"{
#include "file_manip.h"
#include "BobListener.h"
}

namespace tut {

struct BOBData {
	BOBData() {

	}
	~BOBData() {

	}
};

typedef test_group<BOBData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("BOB TESTS");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	int val = 0x08;
	setinputAcs(val);
	int ret = BoBGetACSTamperIn(0);
	ensure("Tamper is not present",ret==0);

	int ret1 = BoBGetACSTamperIn(1);
	ensure("Tamper is present",ret1==1);

	}
template<>
template<>
void testobject::test<2>() {

	}
}
