/*
 * InitializeOpenSSLLibrary.h
 *
 *  Created on: Apr 21, 2012
 *      Author: mamigo
 */

#ifndef INITIALIZEOPENSSLLIBRARY_H_
#define INITIALIZEOPENSSLLIBRARY_H_

#include <string>
#include <map>
#include <vector>
#include <Singleton.h>
#include <pthread.h>
#include <openssl/ssl.h> //openssl/ssl.h
#include <openssl/err.h> //openssl/err.h
#include <openssl/rsa.h>
using namespace std;
const char* get_validation_errstr(long e);

class OpenSSLSupport : public Singleton<OpenSSLSupport>{
public:
	OpenSSLSupport();
	static void ReadCert(X509 **x,const char *fname);
	virtual ~OpenSSLSupport();
	virtual void init();
	virtual void term();
	SSL_CTX* InitCTX(bool bClient);
	void cacheSession(const std::string key, SSL_SESSION *sess);
	SSL_SESSION *getCachedSession(const std::string key);
	static X509 *m_rca;
	static X509_STORE *m_cert_ctx;
	static bool m_shadebug;
	static int VerifyCallback(int preverify_ok, X509_STORE_CTX *ctx);
	static int CheckIfSignedbyRootCA(X509 *e);
	static int GetCommonNameAndExpiry(X509 *e,string& name,time_t& validity);
	int privateDecrypt(unsigned char * encrypted, unsigned char *decrypted, char *filename, char *password);
	bool verifySign(const unsigned char* msg, int mlen, unsigned char* sig, int slen, char* rsaPubKeyFile);
	bool verifySign(const char* file, const char* sigfile, char* rsaPubKeyFile);

private:
	void LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile, const char* CAFile);
	void TermCTX(SSL_CTX* ctx);
	std::map<std::string,SSL_SESSION*> m_sessionCache;	// a cache of sessions to reuse
	SSL_CTX* m_ctxSrv;
	SSL_CTX* m_ctxClnt;

    static pthread_mutex_t **__mutex;
	static void  OpenSSLLockingCallback(int mode, int type, const char* file, int line);
	friend class Singleton<OpenSSLSupport>;
	RSA * createRSAWithFilename(char *filename, char *password, int publicCrypto);

};

#endif /* INITIALIZEOPENSSLLIBRARY_H_ */
