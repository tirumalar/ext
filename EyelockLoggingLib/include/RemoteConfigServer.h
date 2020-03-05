/*
 * RemoteConfigServer.h
 * Accepts connection over TCP/IP and saves data to the queue
 * Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef REMOTECONFIGSERVER_H
#define REMOTECONFIGSERVER_H

#include "HThread.h"
#include "RemoteConfigMsg.h"
#include "socket.h"
#include <map>
#include <string>
#include <vector>
#include "ProcessorChain.h"
#include "Synchronization.h"


using namespace std;
//fwd decl
//class HostAddress;
class Configuration;
class SocketFactory;

class LogConfigMessageHandler
{
public:
	LogConfigMessageHandler() {}
	virtual bool Handle(LogConfigMsg &message, Socket *client) = 0;
};

class RemoteConfigServer: public HThread
{
public:
	RemoteConfigServer(Configuration& pConf);
	virtual ~RemoteConfigServer();
	virtual unsigned int MainLoop();
	virtual int End();
	const char *getName(){
		return "RemoteConfigServer";
	}
	char			m_version[32];
	bool m_debug;

	virtual bool IsHealthy(uint64_t curTime=0);

    void LogHeartBeat(LogConfigMsg & msg); /* DJH: Make public for master process HB w/o socket */
	bool do_serv_task(Socket & client);
#ifndef UNITTEST
private:
#endif

	void CloseServer();
	void DeleteSocketStream(SocketServer *&s);
    void HandleMessage(LogConfigMsg & msg);

    static void onConnect(Socket & client, void *arg);
    uint64_t GetFuteristicTime(int numsec=0);
	LogConfigMsg *m_LogConfigMsg;
	int m_port;
	map<string,uint64_t> m_healthStatus;
	unsigned long m_healthTimeOutMS;
#if defined(HBOX_PG) || defined(CMX_C1)
	SocketServer *m_pSockSrv;
#else
	Safe<SocketServer *> m_pSockSrv;
#endif

	Mutex m_HBLock; // DJH: Add protection for m_healthStatus
	bool m_logging;
	SocketFactory *m_socketFactory;
	bool m_secure;
};

#endif /* REMOTECONFIGSERVER_H */
