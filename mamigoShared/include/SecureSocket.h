/*
 * SecureSocketClient.h
 *
 *  Created on: Apr 23, 2012
 *      Author: mamigo
 */

#ifndef SECURESOCKETCLIENT_H_
#define SECURESOCKETCLIENT_H_

#include "socket.h"
#include <openssl/ssl.h>
#include <string>
using namespace std;
// The secured trait of a socket
class SecureTrait{
public:
	SecureTrait(bool bClient,SSL *ssl):m_ssl(ssl),m_ctx(0),m_bClient(bClient),m_sessionCaching(false){}
	virtual ~SecureTrait();
	int mySend(char *buf, int bytes) const;
	int myRecv(char *buf, int bytes) const;
	void initCtx();
	void initSSL(int sd);
	void OnClientAccept(int clientSD, void (*Server)(Socket& Client));
	void OnClientAccept(int clientSD, void (*Server)(Socket& Client, void *arg), void *arg);
	void Connect(HostAddress& Addr);
	void SetCipher(std::string cipher);
protected:
	SSL * createSSL(int sd);
	void setSession(SSL_SESSION *sess);
	SSL_SESSION *getSession();

	SSL *m_ssl;
	SSL_CTX* m_ctx;
	bool m_bClient;
	bool m_sessionCaching;
	std::string m_serverCipher;
};
#endif /* SECURESOCKETCLIENT_H_ */
