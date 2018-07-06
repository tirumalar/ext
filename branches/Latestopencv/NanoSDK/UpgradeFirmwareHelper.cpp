/*
 * UpgradeFirmwareHelper.cpp
 *
 *  Created on: Jan 19, 2015
 *      Author: developer
 */

#include "UpgradeFirmwareHelper.h"
#include "stdio.h"
#include "string.h"
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include "sstream"
#include <istream>
#include <vector>
#include <ctime>
#include "UtilityFunctions.h"
#include "FileConfiguration.h"

using namespace std;

static std::string removeSpaces(std::string in){
	std::string out = in;

	std::string::iterator it = in.begin();
	while(*it == ' ' && it != in.end()){
		out.erase(out.begin());
		it++;
	};

	it = in.end();
	while(*it == ' ' && it != in.begin()){
			out.erase(out.end());
			it--;
		};
	return out;
}

static std::vector<std::string> split(std::string in,char delimiter){
	std::vector<std::string> outVec;
	size_t pos = std::string::npos;
	while((pos = in.find(delimiter)) != std::string::npos){
		std::string token = in.substr(0,pos);
		in = in.substr(pos+1);
		outVec.push_back(token);

	};
	outVec.push_back(in);
	return outVec;
}

std::string replacechar(std::string in,char current,char replace){
	for(int itr = 0;itr < in.size();itr++){
		if(in[itr] == current)
			in[itr] = replace;
	}
	return in;
}

int UpgradeFirmwareHelper::CompareVersion(std::string newVersion,std::string currentVersion){

	std::string scriptsPath;
#ifdef __ARM__
	scriptsPath = "/home/root/scripts/";
#else
#ifdef HBOX_PG
	scriptsPath = "data/scripts/";
#else
	scriptsPath = "./data/scripts/";
#endif	
#endif


	newVersion = removeSpaces(newVersion);
	currentVersion = removeSpaces(currentVersion);
	newVersion = replacechar(newVersion,'x','0');
	newVersion = replacechar(newVersion,'X','0');
	currentVersion = replacechar(currentVersion,'x','0');
	currentVersion = replacechar(currentVersion,'X','0');

	std::vector<std::string> currentVersionVecTemp = split(currentVersion,' ');
	std::vector<std::string> currentVersionVec;
	if (currentVersionVec.size() > 1)
		currentVersionVec = split(currentVersionVecTemp[0],'.');
	else
		currentVersionVec = split(currentVersion,'.');

	std::vector<std::string> newVersionVec = split(newVersion,'.');

	int nMajorCurrent = atoi(currentVersionVec[0].c_str());
	int nMajorUpgrade = atoi(newVersionVec[0].c_str());
	int nMinorCurrent = atoi(currentVersionVec[1].c_str());
	int nMinorUpgrade = atoi(newVersionVec[1].c_str());
	int nBuildCurrent = atoi(currentVersionVec[2].c_str());
	int nBuildUpgrade = atoi(newVersionVec[2].c_str());

	 if (nMajorUpgrade > nMajorCurrent) {
		        return 1;
	}else if (nMajorUpgrade == nMajorCurrent) {
		if (nMinorUpgrade > nMinorCurrent) {
			return 1;
		}else if (nMinorUpgrade == nMinorCurrent) {
			if (nBuildUpgrade > nBuildCurrent) {
				return 1;
			}
			else if (nBuildUpgrade == nBuildCurrent) {
				return 0;
			}
		}
	}
	return -1;
}


int UpgradeFirmwareHelper::ValidateUploadedFiles(std::string strFirmwareDir,std::string strFilename){
	 // Make sure that checkmd5 script is ok...
	string filename = strFirmwareDir+"/"+strFilename;
	string md5 = getmd5(filename);

	string md5path = filename + ".md5";
	FILE* fp = fopen(md5path.c_str(),"r");
	if(fp == NULL) return 0;
	char output[32];
	fread(output,32,1,fp);
	fclose(fp);
	return strncmp(output,md5.c_str(),32);
}

int UpgradeFirmwareHelper::installScriptCalling(std::string mainUpgradePath){
	std::string installscriptPath = "./data/Scripts/Install.sh";

	int lastindex = mainUpgradePath.find_last_of(".");
	string tarFilePath = mainUpgradePath.substr(0, lastindex);

	stringstream ss;
	ss<<"chmod 777 "<<installscriptPath;
	RunSystemCmd(ss.str().c_str());

	ss.str("");
	ss<<installscriptPath<<" "<<mainUpgradePath<<" "<<tarFilePath;
	int32_t ret2 = RunSystemCmd(ss.str().c_str());
	if(!ret2)
	{
		//cout<<"Installed Successfully"<<endl;
		return 0;
	}
	else
	{
		//cout<<"Not installed properly, Failed in install.sh"<<endl;
		return 1;
	}
}

int UpgradeFirmwareHelper::SetExecPrivileges(std::string Folder, std::string Filename)
{
	stringstream ss;
	ss<<"cd "<<Folder<<";chmod 777 "<<Folder<<"/"<<Filename;
	//cout<<ss.str()<<endl;
	RunSystemCmd(ss.str().c_str());
    return 0;
}

int UpgradeFirmwareHelper::UpgradeBobFirmware(std::string upgradeFileDir, std::string upgradeFile) {
	SetExecPrivileges("//home//root", "icm_communicator");
	stringstream ss;
	ss << "//home//root//icm_communicator -p " << upgradeFileDir << "//" << upgradeFile;
	string IcmComminucatorOutput = RunCmd(ss.str().c_str()); ss.str("");
	if (IcmComminucatorOutput.find("Successfully") != string::npos) {
		return 0;
	}
	else return 1;
}

int UpgradeFirmwareHelper::UpgradePostProcess(std::string extractedFirmwareDir, bool bobUpdate)
{
	//cout << "Merging settings..." << endl;
	FileConfiguration iniSrc("/home/root/Eyelock.ini.upgrade");
	FileConfiguration iniDest("/home/root/Eyelock.ini");
	map<string, pair<string, string> > &confMapSrc = iniSrc.getRefMap();

	// Settings merging:
	// Values from Src ini-file are copied to Dest ini-file if the corresponding key exists in Dest ini-file
	//map<string, pair<string, string> >::iterator iterConf;
	//for (iterConf=confMapSrc.begin(); iterConf!=confMapSrc.end(); ++iterConf) {
	//	iniDest.setValueIfKeyExists(iterConf->first.c_str(), iniSrc.getValue(iterConf->first.c_str(), string("").c_str()));
	//
	//}

	// Values of pre-defined settings are copied from Src ini-file to Dest ini-file
	string settingsToBePreserved[] =
		{"Eyelock.AllowSiteAdminUpgrade",
		"Eyelock.DualMatcherPolicy",
		"Eyelock.EnableNegativeMatchTimeout",
		"Eyelock.NegativeMatchResetTimer",
		"Eyelock.NegativeMatchTimeout",
		"Eyelock.NwMatcherCommSecure",
		"Eyelock.OSDPAddress",
		"Eyelock.OSDPBaudRate",
		"Eyelock.SoftwareUpdateDateCheck",
		"Eyelock.TamperNotifyAddr",
		"Eyelock.TamperNotifyMessage",
		"Eyelock.TamperOutSignalHighToLow",
		"Eyelock.TamperSignalHighToLow",
		"GRI.AuthorizationToneVolume",
		"GRI.HDMatcherCount",
		"GRI.HDMatcher.2.Address",
		"GRI.HDMatcher.2.Type",
		"GRI.HDMatcher.2.BuffSize",
		"GRI.InternetTimeAddr",
		"GRI.InternetTimeSync",
		"GRI.LEDBrightness",
		"GRI.MatchResultDestAddr",
		"GRI.MatchResultNwMsgFormat",
		"GRI.NwDispatcherSecure",
		"GRI.RepeatAuthorizationPeriod",
		"GRI.TamperToneVolume",
		"GRITrigger.DenyRelayTimeInMS",
		"GRITrigger.DualAuthNCardMatchWaitIrisTime",
		"GRITrigger.DualAuthNLedControlledByACS",
		"GRITrigger.DualAuthenticationMode",
		"GRITrigger.DualAuthenticationParity",
		"GRITrigger.EnableRelayWithSignal",
		"GRITrigger.F2FEnable",
		"GRITrigger.OSDPEnable",
		"GRITrigger.OSDPInputEnable",
		"GRITrigger.PACEnable",
		"GRITrigger.RelayTimeInMS",
		"GRITrigger.WeigandEnable",
		"GRITrigger.WeigandHidEnable"};

	for (int i = 0; i <= sizeof(settingsToBePreserved)/sizeof(settingsToBePreserved[0])-1; ++i) {
		if (iniSrc.hasKey(settingsToBePreserved[i].c_str())) {
			iniDest.setValue(settingsToBePreserved[i].c_str(), iniSrc.getValue(settingsToBePreserved[i].c_str(), string("").c_str()));
		}
	}

	//cout << "Setting upgrade time... " << endl;
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%a %dth %b %Y %T %p  (UTC)", timeinfo);

	iniDest.setValue("Eyelock.SoftwareUpdateDateNano", buffer);
		if (bobUpdate) iniDest.setValue("Eyelock.SoftwareUpdateDateBob", buffer);
	//cout << "Upgrade time is: " << buffer << endl;

	//cout << "Writing ini-file..." << endl;
	iniDest.writeIni("//home//root//Eyelock.ini");

	//cout << "Removing settings backup and update flag..." << endl;
	RunCmd("rm //home//root//Eyelock.ini.upgrade");

	RunCmd("rm /home/untarpackage.txt");
	RunCmd("rm /home/slaveUpdated.txt");
	RunCmd("rm /home/firmwareUpdate.txt");
	RunCmd("rm /home/updateInProgress.txt");

	return 0;

}

int UpgradeFirmwareHelper::CopySlaveFileToSlaveDevice(string FirmwareDir, string Filename){

	 stringstream ss;
	 ss<<"ssh root@192.168.140.2 'mkdir "<<FirmwareDir<<"'";

	 //cout<<ss.str()<<endl;
	 RunSystemCmd(ss.str().c_str());

	 ss.str("");
	 //sprintf(buffer,"cat %s/%s | ssh root@192.168.40.2 'cat > %s/%s'",FirmwareDir,Filename,FirmwareDir,Filename);
	 ss<<"cat "<<FirmwareDir<<"/"<<Filename<<" | ssh root@192.140.9.2 'cat > "<<FirmwareDir<<"/"<<Filename<<"'";
	 //cout<<ss.str()<<endl;
	 RunSystemCmd(ss.str().c_str());
	 return 0;
}

void UpgradeFirmwareHelper::CleanupFiles(string dir) {
	stringstream ss;
	string removeTempDataCommand("cd " + dir + "; rm -r *.*");
	ss << removeTempDataCommand << " && " << "ssh root@192.168.40.2 '" << removeTempDataCommand << "' ";
	RunSystemCmd(ss.str().c_str());
}

string UpgradeFirmwareHelper::getBobSoftwareVersion(string filePath){
	string bobversion = "";
	FILE* fp = fopen(filePath.c_str(),"r");
	if(fp == NULL) return bobversion;

	char buffer[512]={0};
	while(fgets(buffer,512,fp) != NULL){
		if(strncmp(buffer,"ICM software version:",21) == 0){
			bobversion = string(buffer).substr(22);
		}
	}

	return bobversion;
}

