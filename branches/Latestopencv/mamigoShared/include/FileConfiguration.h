/*
 * FileConfiguration.h
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 *
 *   Simple File based configuration which reads a given file for key value pairs
 *   key=value
 */

#ifndef FILECONFIGURATION_H_
#define FILECONFIGURATION_H_

#include "Configuration.h"

class FileConfiguration: public Configuration {
public:
	FileConfiguration(const char* fName);
	virtual ~FileConfiguration();
	bool writeIni(const char* fName);
protected:
	bool shouldIgnore(std::string& line);
};

#endif /* FILECONFIGURATION_H_ */
