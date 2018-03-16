/*
 * UT_Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include <string.h>
extern "C"{
#include "file_manip.h"
}

namespace tut {

struct ConfigTestData {
	TestConfiguration *cfg;//empty configuration
		ConfigTestData() {
		cfg = new TestConfiguration();
	}
	~ConfigTestData() {
		delete cfg;
	}
};

typedef test_group<ConfigTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("Configuration TESTS");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("Configuration test 1");
	// right a text file and check if we can read it
	std::ofstream myfile("frServ.ini");
	if (myfile.is_open()) {
		myfile << "key1=string1\n";
		myfile << "key2=true\n";
		myfile << "key3=y\n";
		myfile << "key4=0\n";
		myfile << "key5=10\n";
		myfile << ";key6=comment\n";
		myfile << "key7=10.99\n";
		myfile << "key9=bangalore\n";
		myfile << "key10=cameraId=\"%s\"\n";
		myfile.close();
	}

	FileConfiguration conf("frServ.ini");
	ensure_equals("key1", strcmp(conf.getValue("key1", "fail"), "string1"), 0);
	ensure_equals("key2", conf.getValue("key2", false), true);
	ensure_equals("key3", conf.getValue("key3", false), true);
	ensure_equals("key4", conf.getValue("key4", -1), 0);
	ensure_equals("key5", conf.getValue("key5", 5), 10);
	ensure_equals("key7", conf.getValue("key7", 5.0f), 10.99f);
	ensure_equals("non-existent", conf.getValue("nokey", 15), 15);
	ensure_equals("commented 1", conf.getValue("key6", 20), 20);
	ensure_equals("commented 2", conf.getValue(";key6", 30), 30);
	ensure_equals("case", conf.getValue("Key5", 5), 10);

	ensure_equals("list", conf.getValueIndex("key8", 0, 4, -2, "delhi",
			"Bangalore", "mumbai", "new york"), -2);
	ensure_equals("list", conf.getValueIndex("key9", 0, 4, -2, "delhi",
			"Bangalore", "mumbai", "new york"), 1);

	ensure_equals("char keys return ascii character not number", conf.getValue("key3", 'F'), 'y');
	ensure_equals("char keys return ascii character not number", conf.getValue("key4", 'F'), '0');
	ensure_equals("Allow = in value", strcmp(conf.getValue("key10", "fail"), "cameraId=\"%s\""), 0);

}
}
