/*
 * UtilityFunctions.cpp
 *
 *  Created on: Jan 19, 2013
 *      Author: mamigo
 */

#include "UtilityFunctions.h"
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "I2CBus.h"
#include "Synchronization.h"


unsigned char xtodLoitering(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return c = 0; // not Hex digit
}


void MakeF2FMsg(const char* str ,char val, HTTPPOSTMsg& f2fMsg) {
	std::string line(str);
	unsigned char buff[256] = { };
	unsigned int b = 0;
	for (; b < line.size(); b++) {
		if (line[b] == 'x' || line[b] == 'X') {
			b++;
			break;
		}
	}
	int i = 0;
	printf("Buffer Decoded :: ");
	for (i = 0; b < line.size(); b = b + 2, i++) {
		buff[i] = xtodLoitering(line[b]);
		buff[i] = buff[i] << 4;
		//buerrorff[i] = buff[i] | xtodLoitering(line[b + 1]);
		printf("%#02x ", buff[i]);
	}
	printf("\n");
	i = 0;
	unsigned char* ptr = (unsigned char*) (f2fMsg.GetBuffer());
	ptr[0] = 'F';
	ptr[1] = '2';
	ptr[2] = 'F';
	ptr[3] = ';';
	ptr[4] = val;
	ptr[5] = ';';
	ptr[6] = buff[1];
	ptr[7] = (buff[0]);
	int bitcnt = buff[0];
	bitcnt = bitcnt * 256 + buff[1];
	if (bitcnt > 0) {
		for (i = 0; i < (bitcnt + 7) >> 3; i++) {
			ptr[8 + i] = buff[i + 2];
		}
		ptr[8 + i] = ';';
	} else {
		ptr[8] = ';';
	}
	int sz = 4+2+2+((bitcnt + 7) >> 3)+1;
	f2fMsg.SetSize(sz);
}

void SetKeyInFile(const char * fname,const char *key ,const char *val){
	char buffer[256]={0};
	int len ;

	 len = snprintf(buffer,255,"sed -r \"/.*%s.\\*$/d\" -i %s && sed -r \"$ a\\%s=%s\" -i %s",key,fname,key,val,fname);
#ifndef HBOX_PG
	std::string s("./data/Scripts/ReplaceKey.sh ");
#else
	std::string s("data/Scripts/ReplaceKey.sh ");
#endif
	s.append(buffer);
	std::string ret = RunCmd((char*)s.c_str());
	//factory.push_back(" sed -r \"/.*Eyelock.TestImageLevel.\*$/d\" -i /mnt/mmc/Eyelock.ini && sed -r \"$ a\Eyelock.TestImageLevel=0\" -i /mnt/mmc/Eyelock.ini\n");
}

std::string RunCmd(const char *ptr){
	//printf("RunCmd: %s\n",ptr);
	EnableIRLEDMonitor(false);
	FILE *fp;
	char path[1024*16]={0};
	std::string str;
	fp = popen(ptr, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		EnableIRLEDMonitor(true);
		return str;
	}
	int chars_read = fread(path,sizeof(char),1024*16,fp);
	pclose(fp);
	str.assign(path,chars_read);
	//printf("RunCmd Return:  %s\n",str.c_str());
	EnableIRLEDMonitor(true);
	return str;
}

int32_t runScriptAndCheckReturn(std::string scriptFile, std::string args) {
	std::stringstream ss;
	ss << "chmod 777 " << scriptFile;
	RunCmd(ss.str().c_str()); ss.str("");

	ss << scriptFile << " " << args;
	std::string shell_ret = RunCmd(ss.str().c_str());

	if(shell_ret == "success\n") {
		return 0;
	} else {
		return 1;
	}
}

bool FileExists(const char* fname){
	FILE *fptr = fopen(fname,"r");
	bool ret = false;
	if(fptr){
		ret = true;
		fclose(fptr);
	}
	return ret;
}

std::string getmd5(std::string path){
	char output[1024]={0};
	std::string ret;
	if(FileExists(path.c_str())){
		std::string command = "openssl md5 "+ path + " | cut -f 2 -d '=' | cut -f 2 -d ' '";
		FILE* fp = popen(command.c_str(), "r");
		fgets(output, sizeof(output)-1, fp);
		pclose(fp);
		std::string outstr = output;
		ret = outstr.substr(0,32);
	}
	return ret;
}



void CompressIris(unsigned char *inp,unsigned char *out,int len){
	for(int i=0;i<len;i=i+2)	{
		unsigned char a= *(inp++);
		unsigned char b= *(inp++);
		*out++ = ((a&0x0F)<<4) | ((b&0x0F));
	}
}
void UnCompressIris(unsigned char *inp,unsigned char *out,int len){
	for(int i=0;i<len;i++)	{
		unsigned char a= *(inp++);
		*out++ = (a&0xF0)>>4;
		*out++ = ((a&0x0F));
	}
}

void CreateCoarse(unsigned char *inp,unsigned char *out,int len){
	len = len >> 2;
	unsigned char *ic = inp;
	for (int i = 0; i < len; i++, ic += 4) {
		out[i] = (ic[0] & 0xC0) | ((ic[1] & 0xC0) >> 2) | ((ic[2] & 0xC0) >> 4) | ((ic[3] & 0xC0) >> 6);
	}
}


int64_t GetEpochMilliseconds()
{
	struct timeval tv;
	struct timezone tz;
	if (gettimeofday(&tv, &tz) != 0)
		return 0;
	else
		return (int64_t)tv.tv_sec * 1000 + (int64_t)(tv.tv_usec / 1000);
}

void RunCmdSafe(const char* executable, char* const args[], bool needWait)
{
	pid_t child_pid;
	int status = 0;

	if ((child_pid = fork()) < 0 )
	{
		//std::cerr << "RunCmdSafe: fork failure" << std::endl;
		return;
	}

	if (child_pid == 0)
	{
		execv(executable, args);
		_exit(1);
	}
	else
	{
		if (needWait)
		{
			wait(&status);
		}
	}
}

void DecryptFile(const std::string filename, const std::string parentDir)
{
	std::string outFile = filename + ".dec";

	// /home/root/KeyMgr -d -i file.tar.gz -o ${firmwareDir}/out.tar.gz;
	char* const KeyMgr_args[] = {"KeyMgr", "-d", "-i", (char*) filename.c_str(), "-o", (char*) outFile.c_str(), NULL};
	RunCmdSafe("/home/root/KeyMgr", KeyMgr_args, true);

	char* const mv_args[] = {"mv", (char*) outFile.c_str(), (char*) filename.c_str(), NULL};
	RunCmdSafe("/bin/mv", mv_args, true);

	//tar -xvzf file.tar -C parentDir
	char* const tar_args[] = {"tar", "-xvzf", (char*) filename.c_str(), "-C", (char*) parentDir.c_str(), NULL};
	RunCmdSafe("/bin/tar", tar_args, true);
}

void EnableIRLEDMonitor(bool enable) // internal API
{
	int status;

	ScopeLock lock(I2CBusNanoAPI::instance().GetLock());

	if(0 > I2CBusNanoAPI::instance().Assign(NANO_LED_I2C_ADDR0)){
		printf("EnableIRLEDMonitor() ERROR: Failed to assign LED address on I2C bus ");
		return;
	}

	if (enable){
		for (int i=0; i<3; i++) {
			status = I2CBusNanoAPI::instance().Write( 4, 7 );	// COMMAND=4, Enable the IR LED trigger monitor = value 7
			usleep(1000);
		}
	}
	else {
		for (int i=0; i<3; i++) {
			status = I2CBusNanoAPI::instance().Write( 4, 6 );	// COMMAND=4, Disable the IR LED trigger monitor = value 6
			usleep(1000);
		}
	}
}
bool runSystemCommand = false;

int RunSystemCmd(const char *ptr){
	//printf("RunSystemCmd: %s\n",ptr);
	runSystemCommand = true;
	EnableIRLEDMonitor(false);
	int status = system(ptr);
	EnableIRLEDMonitor(true);
	runSystemCommand = false;
	return status;
}

int RunSystemCmd_Audio(const char *ptr){
	//printf("RunSystemCmd_Audio: %s\n",ptr);
	int status = system(ptr);
	return status;
}

bool isRunSystemCmd()
{
	return runSystemCommand;
}
