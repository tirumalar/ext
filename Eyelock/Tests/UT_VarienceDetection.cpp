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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "VarianceBasedDetection.h"
namespace tut {
struct VBDTestData {
	TestConfiguration conf;//empty configuration
	VBDTestData() {
	}
	~VBDTestData() {
	}
};
typedef test_group<VBDTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("Variance Based Detection");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		set_test_name("Simple Flow VBD");
		float var = 30;

		VarianceBasedDetection vbd(10.0,1,0,8,0);
		bool test = vbd.Process(0,true,&var);
		ensure("Matched State ",!test);
	}

	template<>
	template<>
	void testobject::test<2>() {
		set_test_name("VBD Complex FLOW");
		float var = 30;

		VarianceBasedDetection vbd(30.0,1,0,8,0);
		bool test = vbd.Process(0,true,&var);
		ensure("Matched State ",!test);

		test = vbd.Process(0,false,&var);
		ensure("Initial State ",!test);

		test = vbd.Process(1,false,&var);
		ensure("Matching State ",test);
	}
	template<>
	template<>
	void testobject::test<3>() {
		set_test_name("VBD General FLOW1");
		float var = 50;

		VarianceBasedDetection vbd(20.0,5,0,8,0);
		//No Match
		bool test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Initial",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);


		test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(1,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(2,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(3,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(4,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(5,false,&var);
		ensure("Intruder ACCESS ",test);
		ensure("INPROCESS-Initial",vbd.GetState() == VS_INITIAL);

	}

	template<>
	template<>
	void testobject::test<4>() {
		set_test_name("Before time out if any variance is less than threshold it should not matter");
		float var = 50;

		VarianceBasedDetection vbd(20.0,5,0,8,0);
		//No Match
		bool test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Initial",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(1,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(2,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(3,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(4,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		var = 19;
		test = vbd.Process(5,false,&var);
		ensure("Intruder ACCESS will happen",test);
		ensure("INPROCESS-Initial",vbd.GetState() == VS_INITIAL);
	}

	template<>
	template<>
	void testobject::test<5>() {
		set_test_name("Just before time out the intruder goes and after much delay other guy comes");
		float var = 50;

		VarianceBasedDetection vbd(20.0,5,0,8,0);
		//No Match
		bool test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Initial",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(0,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(1,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(2,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(3,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(4,false,&var);
		ensure("INPROCESS-Searching",!test);
		ensure("INPROCESS-Searching",vbd.GetState() == VS_SEARCHING);

		test = vbd.Process(13,false,&var);
		ensure("Intruder ACCESS won't happen",!test);
		ensure("INPROCESS-Initial",vbd.GetState() == VS_INITIAL);
	}

}
