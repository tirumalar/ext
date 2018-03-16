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
#include <fcntl.h>
#include <unistd.h>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/propertyconfigurator.h"
using namespace log4cxx;
using namespace log4cxx::helpers;


namespace tut {
struct Log4cxxData {
	Log4cxxData() {
		//system("pwd");
		system("ls -al *.log*");
		system("rm Eyelock*.log*");
		system("rm mylog*.log*");
	}
	~Log4cxxData() {
		system("rm Eyelock*.log*");
		system("rm mylog*.log*");
	}
};
typedef test_group<Log4cxxData,2> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("Log4cxx tests");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		LoggerPtr testlog(Logger::getLogger("Eyelock"));
		LoggerPtr testlog1(Logger::getLogger("Eyelock"));
		PropertyConfigurator::configure("data/Log4cxx.cfg");
		for(int i=0;i<2;i++){
				LOG4CXX_INFO(testlog, "LOG4CXX_INFO");
				LOG4CXX_TRACE(testlog, "LOG4CXX_TRACE");
				LOG4CXX_ERROR(testlog, "LOG4CXX_ERROR");
				LOG4CXX_FATAL(testlog, "LOG4CXX_FATAL");
				LOG4CXX_DEBUG(testlog, "LOG4CXX_DEBUG");

				LOG4CXX_INFO(testlog1, "LOG4CXX_INFO1");
				LOG4CXX_TRACE(testlog1, "LOG4CXX_TRACE1");
				LOG4CXX_ERROR(testlog1, "LOG4CXX_ERROR1");
				LOG4CXX_FATAL(testlog1, "LOG4CXX_FATAL1");
				LOG4CXX_DEBUG(testlog1, "LOG4CXX_DEBUG1");
		}
		int file_desc=0;
		file_desc = open("Eyelock.log",O_RDONLY);
		ensure("Log4cxx ",file_desc!=0);
		if (file_desc) {
			close(file_desc);
		}
	}
	template<>
	template<>
	void testobject::test<2>() {
		LoggerPtr testlog(Logger::getLogger("my.logger1"));
		LoggerPtr testlog1(Logger::getLogger("my.logger2"));
		PropertyConfigurator::configure("data/Log4cxx.cfg");
		for(int i=0;i<2;i++){
			LOG4CXX_TRACE(testlog,"LOG4CXX_TRACE");
			LOG4CXX_DEBUG(testlog,"LOG4CXX_DEBUG");
			LOG4CXX_INFO(testlog,"LOG4CXX_INFO");
			LOG4CXX_WARN(testlog,"LOG4CXX_WARN");
			LOG4CXX_ERROR(testlog,"LOG4CXX_ERROR");
			LOG4CXX_FATAL(testlog,"LOG4CXX_FATAL");

			LOG4CXX_TRACE(testlog1,"LOG4CXX_TRACE1");
			LOG4CXX_DEBUG(testlog1,"LOG4CXX_DEBUG1");
			LOG4CXX_INFO(testlog1,"LOG4CXX_INFO1");
			LOG4CXX_WARN(testlog1,"LOG4CXX_WARN1");
			LOG4CXX_ERROR(testlog1,"LOG4CXX_ERROR1");
			LOG4CXX_FATAL(testlog1,"LOG4CXX_FATAL1");
		}
		int file_desc=0;
		file_desc = open("mylog1.log",O_RDONLY);
		ensure("mylog1 is not created ",file_desc!=0);
		if (file_desc) {
			close(file_desc);
		}

		file_desc = open("mylog2.log",O_RDONLY);
		ensure("mylog2 is not created ",file_desc!=0);
		if (file_desc) {
			close(file_desc);
		}
	}
}
