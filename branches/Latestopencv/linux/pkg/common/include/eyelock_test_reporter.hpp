#ifndef EYELOCK_TEST_REPORTER
#define EYELOCK_TEST_REPORTER

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include "apr_time.h"

#define QUOTE(X) #X


using namespace std;
using namespace tut;
class ignoredTest{
public:
	string name;
	string by;
	string date;
	string reason;
	string getReason(){
		string s("Ignored by ");
		s+=by;
		s+=" since ";
		s+=date;
		s+=" for ";
		s+=reason;
		return s;
	}
};
class ignoredTests : private map<string,ignoredTest*>{
public:
    ~ignoredTests(){
        iterator it=begin();
        while(it!=end()){
            delete it->second;
            it->second=0;
            it++;
        }
    }
    void add(const string& name, const string& by, const string& since, const string& reason){
        ignoredTest *iTest = new ignoredTest();
        iTest->name=name;
        iTest->by=by;
        iTest->date=since;
        iTest->reason=reason;
        this->operator [](iTest->name)=iTest;
    }
    ignoredTest* isIgnored(string& name){
        iterator it=find(name);
        if(it==end()) return 0;
        return it->second;
    }
    
};

// Since we run each test seaparately we want a result summary to be reported hence this
class eyelock_test_reporter: public tut::reporter{
	ofstream xRes;
	apr_time_t lastTime;
	int ignored_count;
    
	void run_started(){
		//NOP
	}
	void run_completed(){
		//NOP
	}
public:
	eyelock_test_reporter():ignored_count(0),xRes ("testResults.xml",ios_base::out|ios_base::trunc)
	{
		xRes<<"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"<<endl;
		xRes<<"<tests>"<<endl;
	}
	~eyelock_test_reporter(){
		xRes<<"</tests>"<<endl;
		xRes.close();
	}
	void group_started(const std::string& name)
    {
		string fName(name);
        
		xRes<<"<testsuite name=\""<<xml_encode(fName)<<"\" >"<<endl;
        
		lastTime=apr_time_now();
    }
	void group_completed(const std::string& name)
    {
		xRes<<"</testsuite>"<<endl;
    }
	void test_completed(const test_result& tr)
	{
		reporter::test_completed(tr);
		
		apr_time_t curTime=apr_time_now();
		apr_time_t timeDiff=apr_time_as_msec(curTime-lastTime);
		
		char buf[16];
		sprintf(buf,"test<%d>:",tr.test);
		string name=buf;
		name+=tr.name;
		xml_encode(name);
		
		xRes<<"<testcase name=\""<<name<< "\" " <<"time=\""<<timeDiff<<"\" >"<<endl;
		string result;
		switch(tr.result)
		{
            case test_result::ok:
                result= "pass";
                break;
            case test_result::fail:
                result= "failure";
                break;
            case test_result::ex_ctor:
            case test_result::warn:
            case test_result::term:
            case test_result::ex:
            default:
                result= "error";
                break;
		}
		
		if(tr.result!=test_result::ok)
		{
			xRes<<"<"<<result<<">"<<endl;
			xRes<< xml_cdata(tr.message);
			xRes<<"</"<<result<<">"<<endl;
		}
		xRes<<"</testcase>"<<endl;
		lastTime=apr_time_now();
	}
	void print_report(){
        // for the console
		reporter::run_completed();
		cout<<"ignored:"<<ignored_count<<endl;
	}
	void report_ignored(ignoredTest *ignoreData){
		string reason=ignoreData->getReason();
		test_completed(test_result(ignoreData->name,0,reason,test_result::ok));
		cout<<reason<<endl;
		ignored_count++;
	}
	static string& xml_encode(string& input){
		char buf[16];
		size_t pos=input.find_first_of("@&<>\"\'");
		while(pos!=input.npos){
			int nRep=sprintf(buf,"&#%d;",input.at(pos));
			input.replace(pos,1,buf);
			pos+=nRep;
			pos=input.find_first_of("@&<>\"\'",pos);
		}
		return input;
	}
	static string xml_cdata(const string& input){
		return string("<![CDATA["+input+"]]>");
	}
};
// conv functions below

static void run_test()
{
	tut::runner.get().run_tests();
}
static void run_test(const std::string& group_name)
{
	tut::runner.get().run_tests(group_name);
}

static void run_test(const std::string& group_name, int n)
{
	tut::test_result tr;
	tut::runner.get().run_test(group_name,n,tr);
	if(tr.result == tut::test_result::dummy){
		std::cout << "No such Test "<< group_name << "<"<<n<<">"<<std::endl;
	}
}


	static void run_tests_ignoring(ignoredTests& ignored, eyelock_test_reporter& reporter){
        groupnames allTests=runner.get().list_groups();
        groupnames::iterator git=allTests.begin();
        while(git !=allTests.end()){
            
            ignoredTest* ignoreData=ignored.isIgnored(*git);
            if(!ignoreData) {
                // run as usual if it is not ignored
                runner.get().run_tests(*git);
            }
            else{
                reporter.group_started(ignoreData->name);
                reporter.report_ignored(ignoreData);
                reporter.group_completed(ignoreData->name);
            }
            ++git;
        }
}

#endif
