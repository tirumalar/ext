/*
 * MasterSlaveNwListner.h
 *
 *  Created on: Mar 5, 2015
 *      Author: developer
 */

#ifndef MASTERSLAVENWLISTNER_H_
#define MASTERSLAVENWLISTNER_H_

#include "HThread.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include <map>
#include <string>
#include <vector>
#include "ProcessorChain.h"
#include "Synchronization.h"
using namespace std;


class HostAddress;
class Configuration;
class NwMatchManager;
class SocketFactory;
class EyeDispatcher;

class ImageProcessor;

class MasterSlaveNwListner: public HThread, public ProcessorChain {
public:
	MasterSlaveNwListner(Configuration& pConf);
	virtual ~MasterSlaveNwListner();
	virtual unsigned int MainLoop();
	virtual int End();
	const char *getName(){
		return "MasterSlaveNwListner";
	}
	ProcessorChain *m_imageProcessor;
	NwMatchManager *m_nwMatchManager;
	EyeDispatcher *m_pEyeDispatcher;
	ProcessorChain *m_ledConsolidator;
	bool m_debug;
	bool do_serv_task(Socket & client);
#ifndef UNITTEST
private:
#endif
	void dispatchToLED(HTTPPOSTMsg& msg);
    void HandleMessage(HTTPPOSTMsg & msg);
    static void onConnect(Socket & client, void *arg);
	HTTPPOSTMsg *m_HTTPMsg;
	int m_port;
	Safe<SocketServer *> m_pSockSrv;
	bool m_logging;
	SocketFactory *m_socketFactory;
	int m_sleepAfterDispatchingEye;
};

#endif /* MASTERSLAVENWLISTNER_H_ */
