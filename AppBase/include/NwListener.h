/*
 * NwListener.h
 * Accepts connection over TCP/IP and saves data to the queue
 * Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef NWLISTENER_H_
#define NWLISTENER_H_

#include "HThread.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include <map>
#include <string>
#include <vector>
#include "ProcessorChain.h"
#include "Synchronization.h"

// #define HBOX_PG

using namespace std;
//fwd decl
class HostAddress;
class Configuration;
class MatchProcessor;
class DBRecieveMsg;
class NwMatchManager;
class SocketFactory;
class EyeDispatcher;
class MT9P001FrameGrabber;
class ImageProcessor;
class HTTPPostMessageHandler
{
public:
	HTTPPostMessageHandler() {}
	virtual bool Handle(HTTPPOSTMsg &message, Socket *client) = 0;
};

class NwListener: public HThread, public ProcessorChain{
public:
	NwListener(Configuration& pConf);
	virtual ~NwListener();
	virtual unsigned int MainLoop();
	virtual int End();
	const char *getName(){
		return "NwListener";
	}
	ProcessorChain *m_ledConsolidator;
	ProcessorChain *m_DBDispatcher;
	ProcessorChain *m_F2FDispatcher;
	ProcessorChain *m_imageProcessor;
	MatchProcessor *m_matchProcessor;
	NwMatchManager *m_nwMatchManager;
	char			m_version[32];
	EyeDispatcher *m_pEyeDispatcher;
	MT9P001FrameGrabber *m_frameGrabber;
	ImageProcessor *m_pIp;
	bool m_debug;

	virtual bool IsHealthy(uint64_t curTime=0);
	uint64_t GetDBtimeStamp(){ return  m_DbTimeStamp;}

    void LogHeartBeat(HTTPPOSTMsg & msg); /* DJH: Make public for master process HB w/o socket */
	void AddMessageHandler(HTTPPostMessageHandler *pHandler);
	bool do_serv_task(Socket & client);
	void dispatchToLED(HTTPPOSTMsg & msg);
	void SetCalibration(HTTPPOSTMsg& msg);
#ifndef UNITTEST
private:
#endif

	void CloseServer();
    void HandleMessage(HTTPPOSTMsg & msg);


    void UpdateF2F(HTTPPOSTMsg & msg);
    static void onConnect(Socket & client, void *arg);
    bool HandleReceiveDB(Socket & client);
    uint64_t GetFuteristicTime(int numsec=0);
	HTTPPOSTMsg *m_HTTPMsg;
	DBRecieveMsg *m_DBRxMsg;
	MatchResult m_ledMR,m_f2fResult;
	int m_port;
	int m_safetimeFor2000eyes;
	uint64_t m_DbTimeStamp;
	map<string,uint64_t> m_healthStatus;
	unsigned long m_healthTimeOutMS;
	std::vector<HTTPPostMessageHandler *> m_MessageHandlers;
#if 0
	Safe<SocketServer *> m_pSockSrv;
#else
	SocketServer *m_pSockSrv;
#endif
	Mutex m_HBLock; // DJH: Add protection for m_healthStatus
	bool m_logging;
	SocketFactory *m_socketFactory;
	bool m_secure;
	pthread_t phpThread;
	bool m_phpServerEnable;
	int m_sleepAfterDipatchingEye;
	bool m_transTOC;
};

#endif /* NWLISTENER_H_ */
