/*
 * RemoteConfigMsg.h
 *
 *  Created on: Feb 26, 2020
 *      Author: root
 */

#pragma once

#ifndef INCLUDE_REMOTECONFIGMSG_H_
#define INCLUDE_REMOTECONFIGMSG_H_

#include "MessageExt.h"
#include "base64.h"
#include <string.h>
#include <stdlib.h>

#if (defined(__BFIN__)||defined(__linux__))
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

enum LOGCONFIGMESSAGETYPE
{
	UNKNOWNLOG_MSG = 0,
	REMOTEIP_MSG,
	IMGLOGLEVEL_MSG,
	LOGGERLOGLEVEL_MSG,
	REBOOTDEVICE_MSG,
	REVERTCONFIG_MSG
};


class LogConfigMsg : public BinMessage
{
public:
	LogConfigMsg(unsigned int Bytes):BinMessage(Bytes){}
	LogConfigMsg(const char *Msg, unsigned int Len):BinMessage(Msg,Len){}
#if 0
	virtual void CopyFrom(const Copyable& _other)	{

		const LogConfigMsg& other=(const LogConfigMsg&)_other;
		Timestamp=other.Timestamp;
		char *image=other.getImage();
		long long int alignement= ((long long int)image) & 0x3;
		if(alignement == 0){
			SetData(other.Buffer,other.Size);
			return;
		}
		// we are here means the image is not 4 bytes aligned
		// first copy a few bytes from begining
		int size=(image-other.Buffer-28);
		SetData(other.Buffer,size);
//commit comment
		char *filler="    ";
		Append(filler,4-alignement);

		size=other.Size-size;
		Append(image-28,size);
	}
#endif

	virtual bool  IsDone()
	{
		if (isRemoteIP())
			return isRemoteIPDone();
		else if (isImgLogLevel())
			return isImgLogLevelDone();
		else if (isLoggerLogLevel())
			return isLoggerLogLevelDone();
		else if (isRebootDevice())
			return true;
		else if (isRevertConfigFile())
			return true;

		return false;
	}


	LOGCONFIGMESSAGETYPE getMsgType()
	{
		if(isRemoteIP()) return REMOTEIP_MSG;
		if(isImgLogLevel()) return IMGLOGLEVEL_MSG;
		if(isLoggerLogLevel()) return LOGGERLOGLEVEL_MSG;
		if(isRebootDevice()) return REBOOTDEVICE_MSG;
		if(isRevertConfigFile()) return REVERTCONFIG_MSG;

		return UNKNOWNLOG_MSG;
	}


bool isRemoteIP()
{
	return (0==strncmp(Buffer, "REMOTEIP", 8));
}

bool isImgLogLevel()
{
	return (0==strncmp(Buffer, "IMGLOGLEVEL", 11));
}

bool isLoggerLogLevel()
{
	return (0 == strncmp(Buffer, "LOGGERLOGLEVEL", 14));
}

bool isRebootDevice()
{
	return (0 == strncmp(Buffer, "REBOOT", 6));
}

bool isRevertConfigFile()
{
	return (0 == strncmp(Buffer, "REVERTCONFIG", 12));
}


bool isRemoteIPDone()
{
	char szRemoteIP[64];
	memset(szRemoteIP, '\0', 64);

	return getRemoteIPData(szRemoteIP);
}

// Format of Msg
bool getRemoteIPData(char *pRemoteIP)
{
	char *temp=Buffer;

	// Skip MSGID Text
	temp=strstr(temp,";");

	if(temp==0)
		return false;

	//Move to data
	temp+=1;

	// Parse IP
	char *t1=NULL;
	t1 = strstr(temp,";");
	if(t1)
	{
		memcpy(pRemoteIP,temp,t1-temp);
		return true;
	}
	else
		return false;
}


bool isImgLogLevelDone()
{
	char szRequestedLogLevel[64];
	bool bSaveToDisk;

	memset(szRequestedLogLevel, '\0', 64);

	return getImgLogLevelData(szRequestedLogLevel, bSaveToDisk);
}

// IMGLOGLEVEL;logLevel
bool getImgLogLevelData(char *pRequestedLogLevel, bool& bSaveToDisk)
{
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(temp==0)
		return false;

	temp+=1;

	char *t1=NULL;
	t1=strstr(temp,";");

	if(t1)
	{
		memcpy(pRequestedLogLevel,temp,t1-temp);
		t1 += 1;
	}
	else
		return false;

	temp=strstr(t1,";");


	if(temp==0)
		return false;


	int nSaveToDisk = atoi(t1);
	bSaveToDisk = (nSaveToDisk == 1);

	return true;
}


bool isLoggerLogLevelDone()
{
	char szLogger[64];
	char szRequestedLogLevel[64];

	memset(szLogger, '\0', 64);
	memset(szRequestedLogLevel, '\0', 64);

	return getLoggerLogLevelData(szLogger, szRequestedLogLevel);
}


// LOGGERLOGLEVEL;<logger>;logLevel
bool getLoggerLogLevelData(char *pLogger, char *pRequestedLogLevel)
{
	char *temp=Buffer;

	// Skip MSGID Text
	temp=strstr(temp,";");

	if(temp==0)
		return false;

	//Move to data
	temp+=1;

	// Parse logger
	char *t1=NULL;
	t1 = strstr(temp,";");
	if(t1)
	{
		memcpy(pLogger,temp,t1-temp);
		t1+=1;
	}
	else
		return false;

	temp=strstr(t1,";");

	if(temp)
	{
		memcpy(pRequestedLogLevel,t1,temp-t1);
		return true;
	}
	else
		return false;

	return true;
}

bool isRebootDeviceDone()
{
	return getRebootDevice();
}

bool getRebootDevice()
{
	char *temp=Buffer;

	// Skip MSGID Text
	temp=strstr(temp,";");

	if(temp==0)
		return false;

	return true;
}


bool isRevertConfigFileDone()
{
	return getRevertConfigFile();
}

bool getRevertConfigFile()
{
	char *temp=Buffer;

	// Skip MSGID Text
	temp=strstr(temp,";");

	if(temp==0)
		return false;

	return true;
}



static int calcMessageSize() {return 2048;}

protected:

std::string tempStr;

static const char *remoteIPMsg;
static int remoteIPMsgLen;

static const char *imgloglevelMsg;
static int imglogllevelMsgLen;

static const char *loggerloglevelMsg;
static int loggerloglevelMsgLen;

static const char *rebootMsg;
static int rebootMsgLen;

static const char *revertConfigMsg;
static int revertConfigMsgLen;

public:
static void init(const char *remoteIPStr, const char *imgloglevelStr, const char *loggerloglevelStr, const char *rebootStr, const char *revertConfigStr);
};


#endif /* INCLUDE_REMOTECONFIGMSG_H_ */
