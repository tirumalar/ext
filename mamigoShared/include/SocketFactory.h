/*
 * SocketFactory.h
 *
 *  Controls creation of sockets... main concern it addresses is
 *  the ability to create sockets in secure or unsecure fashion
 *  additionally adds the ability to have fine grain control on which sockets are secured
 *  Created on: May 22, 2012
 *      Author: mamigo
 */

#ifndef SOCKETFACTORY_H_
#define SOCKETFACTORY_H_

#include "socket.h"
#include "Configuration.h"

enum SocketSecurityType{
	SOCK_UNSECURE=0,
	SOCK_SECURE,
};
class SocketFactory {
public:
	SocketFactory(Configuration& conf);
	virtual ~SocketFactory();
	SocketClient createSocketClient(SocketSecurityType secType);
	SocketServer createSocketServer(SocketSecurityType secType, int port, ENetwork Network=eIPv4, int QLen=15);
	SocketServer createSocketServer(SocketSecurityType secType, HostAddress& Addr, int QLen=15);
	static Socket wrapSocket(int sd, SecureTrait *st);
	SocketClient createSocketClient(const char* iniKey);
	SocketClient* createSocketClientP(const char* iniKey);
	SocketServer createSocketServer(const char* iniKey, int port, ENetwork Network=eIPv4, int QLen=15);
	SocketServer createSocketServer(const char* iniKey, HostAddress& Addr, int QLen=15);
	SocketSecurityType getSecurityType(const char* iniKey);
private:

	Configuration& m_conf;
	std::string m_serverCipher;
};

#endif /* SOCKETFACTORY_H_ */
