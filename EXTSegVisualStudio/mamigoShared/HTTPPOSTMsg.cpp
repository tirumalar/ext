/*
 * HTTPPOSTMsg.cpp
 *
 *  Created on: 19-Sep-2009
 *      Author: mamigo
 */

#include "HTTPPOSTMsg.h"

const char *HTTPPOSTMsg::hb=0;
const char *HTTPPOSTMsg::ledMsg=0;
const char *HTTPPOSTMsg::reloadMsg=0;
const char *HTTPPOSTMsg::recvdbMsg=0;
const char *HTTPPOSTMsg::doenrollMsg=0;
const char *HTTPPOSTMsg::faceGrabMsg=0;
const char *HTTPPOSTMsg::motionMsg=0;
const char *HTTPPOSTMsg::f2fMsg=0;
const char *HTTPPOSTMsg::setpinMsg=0;
const char *HTTPPOSTMsg::matchMsg=0;
const char *HTTPPOSTMsg::pingMsg=0;
const char *HTTPPOSTMsg::resetMsg=0;
const char *HTTPPOSTMsg::threadPropMsg=0;
const char *HTTPPOSTMsg::downloadDBMsg=0;
const char *HTTPPOSTMsg::sqliteMsg=0;


int HTTPPOSTMsg::hblen=0;
int HTTPPOSTMsg::ledMsglen=0;
int HTTPPOSTMsg::reloadMsglen=0;
int HTTPPOSTMsg::recvdbMsglen=0;
int HTTPPOSTMsg::doenrollMsglen=0;
int HTTPPOSTMsg::faceGrabMsglen=0;
int HTTPPOSTMsg::motionMsglen=0;
int HTTPPOSTMsg::f2fMsglen=0;
int HTTPPOSTMsg::setpinMsglen=0;
int HTTPPOSTMsg::matchMsglen=0;
int HTTPPOSTMsg::pingMsglen=0;
int HTTPPOSTMsg::resetMsglen=0;
int HTTPPOSTMsg::threadPropMsglen=0;
int HTTPPOSTMsg::downloadDBMsglen=0;
int HTTPPOSTMsg::sqliteMsglen=0;

class HTTPPOSTMsgInit{
public:
	HTTPPOSTMsgInit(){ HTTPPOSTMsg::init("HEARTBEAT", "LEDSET", "RELOADDB", "RECEIVEDB", "DOENROLL","FACEGRAB","MOTION","F2F","SETPIN","MATCH","PING","RESETEYELOCK","ENABLETHREADPROPERTIES","RECEIVESQLITE","DOWNLOADDB");}
};

HTTPPOSTMsgInit h;

// used to initialize the strings initially
void HTTPPOSTMsg::init(
		const char *hbStr,
		const char *ledStr,
		const char *reloadStr,
		const char *recvdbStr,
		const char * doenrollStr,
		const char * faceGrabStr,
		const char * motionStr,
		const char * f2fStr,
		const char * setpinStr,
		const char * matchStr,
		const char *pingStr,
		const char *resetStr,
		const char *threadPropStr,
		const char *sqliteStr,
		const char *downloadDBStr){
	hb=hbStr;
	hblen=strlen(hb);

	ledMsg=ledStr;
	ledMsglen=strlen(ledMsg);

	reloadMsg=reloadStr;
	reloadMsglen=strlen(reloadMsg);

	recvdbMsg=recvdbStr;
	recvdbMsglen=strlen(recvdbMsg);

	sqliteMsg=sqliteStr;
	sqliteMsglen=strlen(sqliteMsg);

	doenrollMsg=doenrollStr;
	doenrollMsglen=strlen(doenrollMsg);

	faceGrabMsg=faceGrabStr;
	faceGrabMsglen=strlen(faceGrabStr);

	motionMsg=motionStr;
	motionMsglen=strlen(motionStr);

	f2fMsg=f2fStr;
	f2fMsglen=strlen(f2fStr);

	setpinMsg=setpinStr;
	setpinMsglen=strlen(setpinStr);

	matchMsg=matchStr;
	matchMsglen=strlen(matchStr);

	pingMsg=pingStr;
	pingMsglen=strlen(pingStr);

	resetMsg=resetStr;
	resetMsglen=strlen(resetStr);

	threadPropMsg = threadPropStr;
	threadPropMsglen = strlen(threadPropStr);

	downloadDBMsg = downloadDBStr;
	downloadDBMsglen = strlen(downloadDBStr);
}
