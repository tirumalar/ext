/*
 * KeyMgr.h
 *
 *  Created on: Mar 9, 2015
 *      Author: developer
 */

#ifndef KEY_MGR_H_
#define KEY_MGR_H_

#include <iostream>
#include "SSLWrap.h"
#include <vector>
#include <string>


class KeyMgr {
public:
	KeyMgr();
	virtual ~KeyMgr();
	int DecryptFile(char* inpfilename,char* outfilename);
	int EncryptFile(char* inpfilename,char* outfilename,char *keyver);
	bool ConvertValidity (const X509 *cert,  int64_t& certvalidity);
	void LoadCertKey(const std::string& certFilename, const std::string& pkeyFilename, const bool isDevice);
	void LoadCertKey(const char *pCert, int certLength, const char *pKey, int keyLength, const bool isDevice);
	void WriteBinFile(const std::string& outputFile);
	void GenerateCertKey(const std::string& hostname, const int serialNo, const std::string& numDays, const bool isDevice);
	bool GenerateBinFile(const int& indexOfDevice, const std::string& pcCertFile, const std::string& pcKeyFile, const std::string& outputFile);
	void ReplaceCertKeyOnDevice(const std::string& certFilename, const std::string& pkeyFilename);
	void AddToDatabase(const int& index, const std::string& hostname, bool isDevice);
	void GenerateAndAddCertKey (const int& index, const std::string& hostname, const int serialNo,
								const std::string& numDays, const bool isDevice);
	bool GenerateBinFile (const int& indexOfDevice, const int& indexOfPC, const std::string& outputFile);
	int64_t GetEpochTime(int days);
	void GetSerialNumber(std::string& _return);
	void RemoveSpecial(std::string& str);
	bool SetSerialNumber(const std::string& serialNum);
	bool DeleteAllKeys();
	int GetKeyNumber();
private:
	SSlWrap *m_sslWrap;
	std::vector<std::pair<std::string, std::string> > m_pcCert;
};

#endif /* KEY_MGR_H_ */
