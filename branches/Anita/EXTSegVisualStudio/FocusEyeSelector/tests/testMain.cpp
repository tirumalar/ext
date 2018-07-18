/*
 * testMain.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <tut/tut_result.hpp>
#include <iostream>

namespace tut {
test_runner_singleton runner;
}
using namespace tut;
using namespace std;

int main(int count, char *strings[]) {

	reporter reporter;
	runner.get().set_callback(&reporter);
	test_result tr;

	groupnames allTests= runner.get().list_groups();

	cout << "Usage: "<<strings[0]<<" [[group_index] [test_index]] " <<endl;
	cout << strings[0]<<" runs all the tests" <<endl;
	cout << "here are the groups" <<endl;
	for(int i=0;i<allTests.size();i++){
		cout <<"["<<i<<"]:"<<allTests[i]<<endl;
	}

	if(count<=1) {
		runner.get().run_tests();
	}
	else if(count==2){
		int gindex=atoi(strings[1]);
		runner.get().run_tests(allTests[gindex]);
	}
	else{
		int gindex=atoi(strings[1]);
		int tindex=atoi(strings[2]);
		runner.get().run_test(allTests[gindex],tindex);
	}

	return !reporter.all_ok();
}

