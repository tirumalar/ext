/*
 * SocketFactory.cpp
 *
 *  Created on: May 22, 2012
 *      Author: mamigo
 */

#include "SocketFactory.h"

SocketFactory::SocketFactory(Configuration& conf):m_conf(conf) {
	 const char *str = m_conf.getValue("Eyelock.Cipher","TLSv1.2");
	 m_serverCipher.assign(str);
}

SocketFactory::~SocketFactory() {
	// TODO Auto-generated destructor stub
}

SocketClient SocketFactory::createSocketClient(SocketSecurityType secType)
{
	SocketClient sc;

	if(secType==SOCK_SECURE)
		sc.SecureIt();

	return sc;
}

SocketServer SocketFactory::createSocketServer(SocketSecurityType secType, int port, ENetwork Network, int QLen)
{
	SocketServer sc(port,Network,QLen);

	if(secType==SOCK_SECURE){
		sc.SecureIt();
		sc.AddCipher(m_serverCipher);
	}
	return sc;
}

SocketServer* SocketFactory::createSocketServer2(SocketSecurityType secType, int port, ENetwork Network, int QLen)
{
	SocketServer* pSc = new SocketServer(port,Network,QLen);

	if(secType==SOCK_SECURE){
		pSc->SecureIt();
		pSc->AddCipher(m_serverCipher);
	}
	return pSc;
}

SocketServer SocketFactory::createSocketServer(SocketSecurityType secType, HostAddress& Addr, int QLen)
{
	SocketServer sc(Addr,QLen);

	if(secType==SOCK_SECURE){
		sc.SecureIt();
		sc.AddCipher(m_serverCipher);
	}
	return sc;
}
SocketSecurityType SocketFactory::getSecurityType(const char* iniKey)
{
	return (SocketSecurityType)m_conf.getValueIndex(iniKey,SOCK_UNSECURE,SOCK_SECURE,SOCK_UNSECURE,"unsecure","secure");
}

SocketClient* SocketFactory::createSocketClientP(const char* iniKey)
{
	SocketSecurityType secType= getSecurityType(iniKey);
	SocketClient* sc = new SocketClient;
	if(secType==SOCK_SECURE)
		sc->SecureIt();
	return sc;
}

SocketClient SocketFactory::createSocketClient(const char* iniKey)
{
	return createSocketClient(getSecurityType(iniKey));
}
SocketServer SocketFactory::createSocketServer(const char* iniKey, int port, ENetwork Network, int QLen)
{
	return createSocketServer(getSecurityType(iniKey), port, Network, QLen);
}
SocketServer* SocketFactory::createSocketServer2(const char* iniKey, int port, ENetwork Network, int QLen)
{
	return createSocketServer2(getSecurityType(iniKey), port, Network, QLen);
}

SocketServer SocketFactory::createSocketServer(const char* iniKey, HostAddress& Addr, int QLen)
{
	return createSocketServer(getSecurityType(iniKey), Addr, QLen);
}
Socket SocketFactory::wrapSocket(int sd, SecureTrait *st){
	Socket client(sd);
	client.SecureIt(st);
	return client;
}
