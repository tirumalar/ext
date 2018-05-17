#include <tut/tut.hpp>
#include "mamigo_test_reporter.hpp"
#include <stdlib.h>
#include <unistd.h>

namespace tut
{
    test_runner_singleton runner;
}
int seqindx=0;

int argc;
char ** argv;
int main(int count, char *strings[]){
	argc = count;
	argv = strings;
    mamigo_test_reporter reporter;
    tut::runner.get().set_callback(&reporter);

	groupnames allTests= runner.get().list_groups();

	cout << "Usage: "<<strings[0]<<" [[group_index] [test_index]] " <<endl;
	cout << strings[0]<<" runs all the tests" <<endl;
	cout << "here are the groups" <<endl;
	for(unsigned int i=0;i<allTests.size();i++){
		cout <<"["<<i<<"]:"<<allTests[i]<<endl;
	}

	if(count<=1) {

		if(false){
			run_test();
		}else if(false){
			run_test("USB Mode change Test",1);
		}else{
			ignoredTests ignored;
			ignored.add("TestDataNWHD","Madhav","29092014","TestDataNWHD Test Commented");
			run_tests_ignoring(ignored,reporter);
		}
	}
	else if(count==2){
		int gindex=atoi(strings[1]);
		run_test(allTests[gindex]);
	}
	else{
		int gindex=atoi(strings[1]);
		int tindex=atoi(strings[2]);
		seqindx = 0;
		if(count!=3) seqindx = atoi(strings[3]);
		run_test(allTests[gindex],tindex);
	}
	//sleep(10);
	reporter.print_report();

	return !reporter.all_ok();

}
