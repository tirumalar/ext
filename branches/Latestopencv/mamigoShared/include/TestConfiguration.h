/*
 * TestConfiguration.h
 *
 *  Created on: 23 Dec, 2008
 *      Author: akhil
 */

#ifndef TESTCONFIGURATION_H_
#define TESTCONFIGURATION_H_

#include "Configuration.h"

class TestConfiguration: public Configuration {
public:
	TestConfiguration() {
	}
	virtual ~TestConfiguration() {
	}
	void setValue(const char* key, const char *value){
		std::string sKey(key);
		std::string sValue(value);
		_setValue(sKey,sValue);}
	bool removeKey(const char* key){std::string sKey(key);return _removeKey(sKey);	}
};

#endif /* TESTCONFIGURATION_H_ */
