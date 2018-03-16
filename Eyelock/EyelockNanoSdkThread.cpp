/*
 * EyelockNanoSdkThread.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: developer
 */



#include <string>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TSSLServerSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "EyeLockThread.h"
#include "EyelockNanoSdkThread.h"
#include "socket.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "Configuration.h"
#include "UtilityFunctions.h"
#include "logging.h"
#include "AudioDispatcher.h"
#include "SDKDispatcher.h"
#include "OpenSSLSupport.h"
//#include "SSLWrap.h"
//#include "DBSync.h"
//#include "eyelockUtil.h"

const char logger[30] = "EyelockNanoSdk";
#ifdef __BFIN__
	#define SDCARD_PATH "/mnt/mmc/cert/"
#else
	#define SDCARD_PATH "./"
#endif


using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::EyelockNano;
using boost::shared_ptr;

EyelockNanoSdkThread::EyelockNanoSdkThread(Configuration &conf):GenericProcessor(conf),m_serverInitialized(NULL),
		m_webSocketServer(NULL), m_securelistener(false), m_enableKeepAlive(false), m_keepAliveTime(0), m_keepAliveIntvl(0), m_keepAliveProbes(0) {
	EyelockLog(logger, DEBUG, "EyelockNanoSdkThread ConstructoSSLWrapr");
	m_port = conf.getValue("Eyelock.NanoSDKPort",8090);
	m_handler.reset(new EyelockNanoDeviceHandler(&conf));
	m_bMaster = (strcmp(conf.getValue("GRI.masterMode","2"),"1") == 0) ? true : false;
	if (m_bMaster)
	{
		m_securelistener = conf.getValue("Eyelock.SecureThrift", false);
		m_enableKeepAlive = conf.getValue("Eyelock.SdkEnableKeepAlive", true);
		m_keepAliveTime = conf.getValue("Eyelock.SdkKeepAliveTime", 120);
		m_keepAliveIntvl = conf.getValue("Eyelock.SdkKeepAliveIntvl", 10);
		m_keepAliveProbes = conf.getValue("Eyelock.SdkKeepAliveProbes", 5);
	}
	m_certPath = conf.getValue("Eyelock.SecureCertificate", "./rootCert/certs/nanoNXTDefault.crt");
	m_certKeyPath = conf.getValue("Eyelock.SecureCertificateKey", "./rootCert/certs/nanoNXTDefault.key");
	m_rootCAPath = conf.getValue("Eyelock.CAPath", "./rootCert/rootCA.cert");
	m_forceTLS12 = conf.getValue("Eyelock.TLSEnable",false);
}


EyelockNanoSdkThread::~EyelockNanoSdkThread() {

}

void EyelockNanoSdkThread::Setup()
{
	if(m_serverInitialized)
		return;

	boost::shared_ptr<TProcessor> processor(new EyelockNanoDeviceProcessor(m_handler));
	TServerSocket *pServerSocket = NULL;
	if (m_securelistener)
	{
		EyelockLog(logger, DEBUG, "Enabling Secure on port-- %d ", m_port);
		try
		{
			boost::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(m_forceTLS12));
		    sslSocketFactory->setEyelockSocket(true);

		    sslSocketFactory->loadCertificate(m_certPath.c_str());
		    sslSocketFactory->loadPrivateKey(m_certKeyPath.c_str());
		    sslSocketFactory->loadTrustedCertificates(m_rootCAPath.c_str());
		    int  mode  =  SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_CLIENT_ONCE;
		    sslSocketFactory->setCertAuthenticationCallback((void*)OpenSSLSupport::VerifyCallback,mode);
		    pServerSocket = new TSSLServerSocket(m_port, sslSocketFactory);
		}
		catch (TTransportException ex)
		{
			EyelockLog(logger, ERROR, "EyelockNanoSdkThread Secure Certificate Exception - %s", ex.what());
		}
	}
	else
	{
		pServerSocket = new TServerSocket(m_port);
	}

	if (m_bMaster && m_enableKeepAlive)
	{
		pServerSocket->setKeepAlive(m_enableKeepAlive);
		pServerSocket->setKeepAliveTime(m_keepAliveTime);
		pServerSocket->setKeepAliveIntvl(m_keepAliveIntvl);
		pServerSocket->setKeepAliveProbes(m_keepAliveProbes);
	}

	m_transPort.reset(pServerSocket);

	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	boost::shared_ptr<ThreadManager> threadManager(ThreadManager::newSimpleThreadManager(5, 1) );
	boost::shared_ptr<ThreadFactory> threadFactory(new PosixThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();
	m_server.reset(new TThreadPoolServer(processor,m_transPort,transportFactory,protocolFactory,threadManager));
	m_serverInitialized = true;

	Cleanup();
}

int EyelockNanoSdkThread::End(){
	return 0;
}

void EyelockNanoSdkThread::SetEyelockThread(EyeLockThread *ptr)
{
	if(m_handler.get())
		m_handler->SetEyelockThread(ptr);
}
void EyelockNanoSdkThread::SetConsolidator(LEDConsolidator *ptr)
{
	if(m_handler.get()){
		m_handler->SetConsolidator(ptr);
	}
}
void EyelockNanoSdkThread::SetSDKDispatcher(SDKDispatcher *ptr){
	if(m_handler.get()){
		m_handler->SetSDKDispatcher(ptr);
	}
}


void EyelockNanoSdkThread::SetEyelockFirmwareVersion(std::string firmwareRevision){
	if(m_handler.get())
	m_handler->SetEyelockFirmwareVersion(firmwareRevision);
}

unsigned int EyelockNanoSdkThread::MainLoop() {
	while (!ShouldIQuit()) {
		try{
			if(m_serverInitialized){
				//EyelockLog(logger, DEBUG, "%f::EyelockNanoSdkThread server START\n",GetUpTime());
				//WriteKlog(test.c_str(),test.length());
				m_server.get()->serve();
			}
		}
		catch(Exception& ncex){
			EyelockLog(logger, ERROR, "EyelockNanoSdkThread Exception");
			ncex.PrintException();
			sleep(1);
		}
		catch(const char* msg){
			EyelockLog(logger, ERROR, "EyelockNanoSdkThread Exception %s",msg);
			sleep(1);
		}
		catch(apache::thrift::TException ex){
			EyelockLog(logger, ERROR, "Thrift Exception in EyelockNanoSdkThread %s",ex.what());
			EyelockLog(logger, ERROR, "Error = %d",errno);
		}
		sleep(1);
	}
	return 0;
}

void EyelockNanoSdkThread::SetAudioDispatcher(AudioDispatcher *ptr)
{
	if(m_handler.get())
	m_handler->SetAudioDispatcher(ptr);
}

int32_t EyelockNanoSdkThread::SetAudiolevel(float volumelevel)
{
	int32_t ret = 1;
	if(m_handler.get())
	ret = m_handler->SetAudiolevel(volumelevel);
	return ret;
}

void EyelockNanoSdkThread::SetNwMatchManager(NwMatchManager* ptr){
	if(m_handler.get())
		m_handler->setNwMatchManager(ptr);
}

int32_t EyelockNanoSdkThread::IsDeviceTampered(void){
	int32_t ret = 1;
	if(m_handler.get())
	ret = m_handler->IsDeviceTampered();
	return ret;
}

void EyelockNanoSdkThread::Cleanup()
{
	if(m_handler.get())
		m_handler->RemoveTempDbFiles();
}

