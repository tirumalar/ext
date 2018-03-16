/*
 * EyelockNanoSdkThread.h
 *
 *  Created on: Nov 3, 2014
 *      Author: developer
 */

#ifndef EYELOCKNANOSDKTHREAD_H_
#define EYELOCKNANOSDKTHREAD_H_


#include "GenericProcessor.h"
#include <boost/shared_ptr.hpp>
#include <EyelockNanoDevice_server.h>
#include <AudioDispatcher.h>
#include <NwMatchManager.h>

namespace apache {
namespace thrift
	{
		namespace server {
		class TThreadPoolServer;
		class TSimpleServer;
		}
		namespace transport {
		class TServerTransport;
		}
	}
}

class MatchIrisInDB;
class LedColor;
class EyeLockThread;
class WebSocketServer;
//class SSlWrap;
class EyelockNanoDeviceHandler;
class SDKDispatcher;

class EyelockNanoSdkThread: public GenericProcessor {
public:

		virtual void init(){};
		virtual int End();
		virtual unsigned int MainLoop();
	    const virtual char *getName(){return "EyelockNanoSdkThread";}
	    void SetEyelockThread(EyeLockThread *ptr);
	    void SetEyelockFirmwareVersion(std::string firmwareRevision);
	    void SetRGBcontrollerNano(RGBController *ptr);
	    void SetConsolidator(LEDConsolidator *ptr);
	    EyelockNanoSdkThread(Configuration &conf);
	    void Setup();
	    void SetAudioDispatcher(AudioDispatcher *ptr);
	    void SetNwMatchManager(NwMatchManager *ptr);
	    int32_t SetAudiolevel(float Volume);
	    void GetAudiolevel(void);
	    int32_t IsDeviceTampered(void);
	    void SetWebSocketServer(WebSocketServer *ws){
	    	m_webSocketServer = ws;
	    }
	    void SetSDKDispatcher(SDKDispatcher *ptr);
	    virtual ~EyelockNanoSdkThread();

protected:
	    virtual void process(Copyable *msg){}
	    int getQueueSize(Configuration *conf){return 0;}
	    Copyable *createNewQueueItem(){return NULL;}
	    virtual Copyable *getNextMsgToProcess(){return NULL;}
	    void Cleanup();
	    boost::shared_ptr<apache::thrift::transport::TServerTransport> m_transPort;
	    boost::shared_ptr<apache::thrift::server::TThreadPoolServer> m_server;
	    boost::shared_ptr<EyelockNanoDeviceHandler> m_handler;
	    int m_port,m_numberOfThread,m_timeoutSec, m_keepAliveTime, m_keepAliveIntvl, m_keepAliveProbes;
	    bool m_serverInitialized, m_securelistener, m_bMaster, m_forceTLS12, m_enableKeepAlive;
	    std::string m_certPath, m_certKeyPath,m_rootCAPath;
	    WebSocketServer* m_webSocketServer;
};

#endif /* EYELOCKNANOSDKTHREAD_H_ */
