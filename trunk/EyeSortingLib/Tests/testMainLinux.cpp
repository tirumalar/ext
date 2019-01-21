/*
 * testMain.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

#include <iostream>
#include <stdlib.h>
namespace tut {
test_runner_singleton runner;
}
using namespace tut;
using namespace std;

int seqindx =0;

int argc=0;
char **argv= NULL;

int main(int count, char *strings[]) {
argc = count;
argv = strings;
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
		seqindx = 0;
		if(count!=3)seqindx = atoi(strings[3]);
		runner.get().run_test(allTests[gindex],tindex,tr);
	}

	return !reporter.all_ok();
}

