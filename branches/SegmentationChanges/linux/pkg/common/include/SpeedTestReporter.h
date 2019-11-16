#pragma once

#include "Singleton.h"
#include "mamigo_test_reporter.hpp"
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#define REPORTSPEED( M, D) SpeedTestReporter::instance().addResult(get_test_name(), M, D)
using namespace std;
class SpeedTestReporter: public Singleton<SpeedTestReporter>{
public:
	~SpeedTestReporter(){
		xRes<<"</speedtests>"<<endl;
		xRes.close();
	}
	void addResult(const string& groupName, const string& testName, double millis){
		printf("%s takes %0.3f ms\n",testName.c_str(),millis);
		if(res[groupName].find(testName)!=res[groupName].end()){
			cerr <<"WARN: "<< groupName << ':'<<testName<<" nonunique, result will be overwritten"<<endl;
		}
		res[groupName][testName]=millis;
	}
	void print_report(){
		Results::iterator git=res.begin();
		for(;git!=res.end();git++){
			string name=git->first;
			xRes<<"<speedtestsuite name=\""<<xml_encode(name)<<"\" >"<<endl;
			SpeedResult::iterator sit=git->second.begin();
			for(;sit!=git->second.end();sit++){
				name=sit->first;
				xRes<<"<speedtestcase name=\""<<xml_encode(name)<< "\" " <<"time=\""<<sit->second<<"\" />"<<endl;
			}
			xRes<<"</speedtestsuite>"<<endl;
		}
	}
private:
	SpeedTestReporter():xRes ("speedTestResults.xml",ios_base::out|ios_base::trunc){
		xRes<<"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"<<endl;
		xRes<<"<speedtests>"<<endl;
	}
	static string& xml_encode(string& input){return mamigo_test_reporter::xml_encode(input);}
	friend class Singleton<SpeedTestReporter>;
	typedef std::map<std::string,double> SpeedResult;
	typedef std::map<std::string, SpeedResult> Results;
	Results res;
	std::ofstream xRes;
};
