/*
 * SSLWrap.h
 *
 *  Created on: Dec 7, 2013
 *      Author: developer
 */
#ifndef SSLWRAP_H_
#define SSLWRAP_H_
#if defined _WIN32 || defined _WIN64
#include <Windows.h>
#endif
#include <string>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/aes.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#define CERTKEYFILESIZE 10240;
#define ROOT_CERT "./rootCert/rootCA.cert"
#define ROOT_KEY "./rootCert/rootCA.key"

typedef struct __KeyItem
{
	char randData[157];
	int size ;
	unsigned char data[2048];
} CertKey;
typedef struct __DevKey
{
	CertKey ELCERT;
	CertKey LPCERT;
	CertKey LPKEY;
}DevKey;

class SSlWrap
{
private :
	// replace with std::string
	unsigned char m_organizationIssuerName[256] ;
	unsigned char m_organizationalIssuerUnit[256] ;
    unsigned char m_countryIssuerName[256];
	unsigned char m_CommonName[256] ;
	unsigned char m_organizationLocation[256] ;
	unsigned char m_emailAddress[256] ;

	int add_ext(X509 *cert, int nid, char *value);
	void fillCertStruct(CertKey *certKey,X509 *x);
	void fillKeyStruct (CertKey *certKey, EVP_PKEY *k );
	void permuteBuffer(unsigned char * ,int);
	void recoverBuffer(unsigned char * ,int);
	void saveBinfile(CertKey * p_ELCERT,CertKey * p_LPCERT,CertKey * p_LPKEY,unsigned char * outPutbuffer);
public :
	EVP_PKEY *m_ELpkey,*m_LPpkey, *m_CApkey;
	X509 *m_ELx509, *m_LPx509;
    int mkcert(int bits, int serial, int days,bool isdevice);
    int mkcert(int bits, int serial, int days,bool isdevice, std::string& hostname);

    char *get_EL_PKEY();
    char* get_EL_X509Cert();
    char *get_LP_PKEY();
    char* get_LP_X509Cert();

    void get_LP_PKEY(std::string &key);
    void get_LP_X509Cert(std::string &cert);
    void get_EL_PKEY(std::string &key);
    void get_EL_X509Cert(std::string &cert);
    //std::string& get_EL_PKEY();
    //std::string& get_EL_X509Cert();
    //std::string& get_LP_PKEY();
    //std::string& get_LP_X509Cert();

    void get_CertKeyPair(bool isdevice, char * output );
    void  set_CertKeyPair(char * data, bool isDevice);

    void  CreateBinFile(unsigned char * outPutbuffer);
	void CreateBinFile(char* elcertData, char * lpCerData, char *lpKeyData,unsigned char * outPutbuffer);
	void CreateBinFile(char * lpCerData, int lpcertLen, char *lpKeyData,
			int lpKeyLen, char* elcertData, int elcertLen, unsigned char * buffer);
	void fillCertKeyStruct (X509 *x, EVP_PKEY *k, bool isDevice );

  //  void ReadBinFile(char* key, Devkey *d);
    void ReadBinFile(const char * filename);
    void ReadBinFile(unsigned char *bindata ,int binCertKeySize);
    void setOrganizationIssuerName( std::string  orgName = "Eyelock Inc"  );
    void setOrganizationalIssuerUnit( std::string  orgUnit = "Nano"  );
    void setCountryIssuerName(std::string countryName ="US");
	void setCommonName(std::string  commonName="www.eyelock.com" );
	void setOrganizationLocation(std::string  location="New York" );
	void setOrganizationEmailAddress(std::string  email="support@eyelock.com" );
    SSlWrap();
    ~SSlWrap();

};

#endif /* SSLWRAP_H_ */
