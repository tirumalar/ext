/*
 * UpgradeFirmwareHelper.h
 *
 *  Created on: Jan 19, 2015
 *      Author: developer
 */

#ifndef UPGRADEFIRMWAREHELPER_H_
#define UPGRADEFIRMWAREHELPER_H_
#include <iostream>
#include <string.h>
using namespace std;

class UpgradeFirmwareHelper {
public:
	int CompareVersion(std::string newVersion,std::string currentVersion);
	int ValidateUploadedFiles(std::string strFirmwareDir,std::string strFilename);
	int installScriptCalling(std::string filePath);
	int UpgradeBobFirmware(string UpgradeFileDirectory, string UpgradeFilename);
	int SetExecPrivileges(std::string Folder, std::string Filename);
	int CopySlaveFileToSlaveDevice(string FirmwareDir, string Filename);
	int UpgradePostProcess(string extractedFirmwareDir, bool bobUpdate);
	void CleanupFiles(string firmwareDir);
	string getBobSoftwareVersion(string filePath);
};

#endif /* UPGRADEFIRMWAREHELPER_H_ */
