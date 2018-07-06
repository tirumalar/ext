/*
 * UtilityFunctions.h
 *
 *  Created on: Jan 19, 2013
 *      Author: mamigo
 */

#ifndef UTILITYFUNCTIONS_H_
#define UTILITYFUNCTIONS_H_
#include <string>
#include "HTTPPOSTMsg.h"
extern "C"{
	#include "file_manip.h"
}


unsigned char xtodLoitering(char c);
void MakeF2FMsg(const char* str ,char val, HTTPPOSTMsg& f2fMsg);
std::string RunCmd(const char *ptr);
int32_t runScriptAndCheckReturn(std::string scriptFile, std::string args);
bool FileExists(const char* fname);
void CompressIris(unsigned char *inp,unsigned char *out,int len);
void UnCompressIris(unsigned char *inp,unsigned char *out,int len);
void CreateCoarse(unsigned char *inp,unsigned char *out,int len);
void SetKeyInFile(const char * fname,const char *key ,const char *val);
std::string getmd5(std::string path);
int64_t GetEpochMilliseconds(void);
void RunCmdSafe(const char* executable, char* const args[], bool needWait);
void DecryptFile(const std::string filename, const std::string parentDir);
void EnableIRLEDMonitor(bool enable);
int RunSystemCmd(const char *ptr);
int RunSystemCmd_Audio(const char *ptr);
bool isRunSystemCmd();

#endif /* UTILITYFUNCTIONS_H_ */
