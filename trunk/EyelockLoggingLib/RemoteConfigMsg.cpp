/*
 * RemoteConfigMsg.cpp
 *
 *  Created on: Mar 1, 2020
 *      Author: root
 */

#include "RemoteConfigMsg.h"



const char *LogConfigMsg::remoteIPMsg=0;
const char *LogConfigMsg::imgloglevelMsg=0;
const char *LogConfigMsg::loggerloglevelMsg=0;
const char *LogConfigMsg::rebootMsg=0;
const char *LogConfigMsg::revertConfigMsg=0;


int LogConfigMsg::remoteIPMsgLen=0;
int LogConfigMsg::imglogllevelMsgLen=0;
int LogConfigMsg::loggerloglevelMsgLen=0;
int LogConfigMsg::rebootMsgLen=0;
int LogConfigMsg::revertConfigMsgLen=0;

class LogConfigMsgInit{
public:
	LogConfigMsgInit(){ LogConfigMsg::init("REMOTEIP", "IMGLOGLEVEL", "LOGGERLOGLEVEL", "REBOOT", "REVERTCONFIG");}
};

LogConfigMsgInit h;

// used to initialize the strings initially
void LogConfigMsg::init(
		const char *remoteIPStr,
		const char *imgloglevelStr,
		const char *loggerloglevelStr,
		const char *rebootStr,
		const char *revertConfigStr)
{
	remoteIPMsg=remoteIPStr;
	remoteIPMsgLen=strlen(remoteIPMsg);

	imgloglevelMsg=imgloglevelStr;
	imglogllevelMsgLen=strlen(imgloglevelMsg);

	loggerloglevelMsg=loggerloglevelStr;
	loggerloglevelMsgLen=strlen(loggerloglevelMsg);

	rebootMsg=rebootStr;
	rebootMsgLen=strlen(rebootMsg);

	revertConfigMsg=revertConfigStr;
	revertConfigMsgLen=strlen(revertConfigMsg);
}



