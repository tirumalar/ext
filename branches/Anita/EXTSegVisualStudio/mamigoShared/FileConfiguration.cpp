/*
 * FileConfiguration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */

#include "FileConfiguration.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/*
 * If the line starts with a comment ignore it
 */
bool FileConfiguration::shouldIgnore(std::string& line){
	line=trimStr(line);
	if(line.size()==0) return true;
	if(line.at(0)==';') return true;
	return false;
}

FileConfiguration::FileConfiguration(const char* fName) {
	std::string line,key,value;

	std::ifstream myfile(fName);
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			std::getline(myfile, line);
			if(shouldIgnore(line)) continue;
			std::istringstream iss(line);
			std::getline(iss, key, '=');
			std::getline(iss, value);
			_setValue(key,value);
		}
		myfile.close();
	}
}

bool FileConfiguration::writeIni(const char* fName) {
	FILE *pFile;
	pFile = fopen(fName, "w");
		if (pFile != NULL) {
		  std::map<std::string, std::pair<std::string, std::string> > confMap = this->getMap();
	      std::map<std::string, std::pair<std::string, std::string> >::const_iterator iterConf;
		  for (iterConf = confMap.begin(); iterConf != confMap.end(); ++iterConf) {
			TStrStrPair pair = iterConf->second;
			std::string data = pair.first + "=" + pair.second + "\n";
			fwrite(data.c_str(), 1, data.size(), pFile);
		}
		fclose(pFile);
		return 0;
	} else return 1;
}

FileConfiguration::~FileConfiguration() {
	// TODO Auto-generated destructor stub
}
