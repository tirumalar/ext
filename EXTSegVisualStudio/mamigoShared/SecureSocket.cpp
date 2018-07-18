/*
 * SecureSocket.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: mamigo
 */

#include <errno.h>
#include "SecureSocket.h"
#include "OpenSSLSupport.h"
#include "logging.h"
#include <unistd.h>
//#include "socket.h"

const char logger[30] = "SecureSocket";

void SecureTrait::initCtx() {
	m_ctx=OpenSSLSupport::instance().InitCTX(m_bClient);
	//OpenSSLSupport::instance().LoadCertificates(m_ctx,"eyelock.cert", "eyelock.key");
}
void SecureTrait::initSSL(int sd)
{
	m_ssl= createSSL(sd);
}
SSL *SecureTrait::createSSL(int sd)
{
	SSL * ssl = SSL_new(m_ctx);   /* get new SSL state with context */
	SSL_set_fd(ssl, sd);
	return ssl;
}

SecureTrait::~SecureTrait()
{
	if(m_ssl)
	{
		SSL_shutdown(m_ssl);
		//close(SSL_get_fd(m_ssl));
		SSL_free(m_ssl);
	}
//	if(m_ctx)
//		OpenSSLSupport::instance().TermCTX(m_ctx);
}
int SecureTrait::mySend(char *buf, int bytes) const
{
	int rc=SSL_write(m_ssl,buf,bytes);
	if(rc<=0)
	{
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		ERR_print_errors_fp(stderr);
	}
	return rc;
}

int SecureTrait::myRecv(char *buf, int bytes) const
{
	int rc=SSL_read(m_ssl,buf,bytes);
	if(rc<=0)
	{
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		ERR_print_errors_fp(stderr);
	}
	return rc;
}
void SecureTrait::OnClientAccept(int clientSD, void (*Server)(Socket& Client))
{
	SSL * ssl = createSSL(clientSD);
	SSL_set_cipher_list(ssl,m_serverCipher.c_str());
    if ( SSL_accept(ssl) <= 0 ) /* do SSL-protocol accept */
    {
    	EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        ERR_print_errors_fp(stderr);
    	if(ssl)
    	{
    		int r = SSL_shutdown(ssl);
    		if (r == 0){
    			SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
    			r = SSL_shutdown(ssl);
    		}
            if (r >= 0)
            {
                close(SSL_get_fd(ssl));
            }
    		SSL_free(ssl);
    	}
        return;
    }
	// now create a new client socket and secure it
	Socket c(clientSD);
	c.SecureIt(new SecureTrait(false,ssl));
	Server(c);
}
void SecureTrait::OnClientAccept(int clientSD, void (*Server)(Socket& Client, void *arg), void *arg)
{
	SSL * ssl = createSSL(clientSD);
	SSL_set_cipher_list(ssl,m_serverCipher.c_str());
    if ( SSL_accept(ssl) <= 0) /* do SSL-protocol accept */
    {
    	EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        ERR_print_errors_fp(stderr);
    	if(ssl)
    	{
    		int r = SSL_shutdown(ssl);
    		if (r == 0){
    			SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
    			r = SSL_shutdown(ssl);
    		}
            if (r >= 0)
            {
                close(SSL_get_fd(ssl));
            }
    		SSL_free(ssl);
    	}
        return;
    }

	// now create a new client socket and secure it
	Socket c(clientSD);
	c.SecureIt(new SecureTrait(false,ssl));
	Server(c,arg);
}
void SecureTrait::Connect(HostAddress& Addr)
{
	// do we have this session cached ?
	SSL_SESSION *sess= OpenSSLSupport::instance().getCachedSession(Addr.GetOrigHostName());
	if(sess)
	{
		setSession(sess);
	}
	if(SSL_connect(m_ssl)<=0)
	{
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		 ERR_print_errors_fp(stderr);
	}
	else
	{
		if(m_sessionCaching)
			OpenSSLSupport::instance().cacheSession(Addr.GetOrigHostName(),getSession());
	}
}

SSL_SESSION *SecureTrait::getSession()
{
	SSL_get1_session(m_ssl);
}


void SecureTrait::setSession(SSL_SESSION *sess)
{
	SSL_set_session(m_ssl,sess);
}

void SecureTrait::SetCipher(std::string cipher){
	m_serverCipher = cipher;
}

