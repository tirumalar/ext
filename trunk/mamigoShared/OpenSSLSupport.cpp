/*
 * InitializeOpenSSLLibrary.cpp
 *
 *  Created on: Apr 21, 2012
 *      Author: mamigo
 */

#include "OpenSSLSupport.h"
#include "FileConfiguration.h"
#ifdef HAVE_OPENSSL
#include "ssl_private.h"
#endif
#include "DBAdapter_Keys.h"
#include <string.h>
#include "asn1_locl.h"
#include "logging.h"

#include <sys/types.h>
#include <sys/stat.h>
extern "C"{
#include "file_manip.h"
}
#include <vector>

bool           m_debug = 0;
const char logger[30] = "OpenSSLSupport";

void OpenSSLSupport::OpenSSLLockingCallback(int mode, int type,
		const char* /*file*/, int /*line*/) {
	if (!__mutex)
		return;

	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(__mutex[type]);
	else
		pthread_mutex_unlock(__mutex[type]);
}

pthread_mutex_t **OpenSSLSupport::__mutex = 0;
X509* OpenSSLSupport::m_rca=NULL;
X509_STORE* OpenSSLSupport::m_cert_ctx= NULL;
bool OpenSSLSupport::m_shadebug=false;
static vector<pair<string,int64_t> > m_KeyVec;
OpenSSLSupport::OpenSSLSupport():m_ctxSrv(0),m_ctxClnt(0) {
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	init();
}

OpenSSLSupport::~OpenSSLSupport() {

	for(std::map<std::string,SSL_SESSION*>::iterator it=m_sessionCache.begin();it!=m_sessionCache.end();it++)
	{
		SSL_SESSION_free(it->second);
	}

	m_sessionCache.clear();

	if(m_rca){
		X509_free(m_rca);
		m_rca=NULL;
	}
	if(m_cert_ctx){
		X509_STORE_free(m_cert_ctx);
		m_cert_ctx=NULL;
	}

	TermCTX(m_ctxSrv);
	TermCTX(m_ctxClnt);

	ERR_free_strings();
	ERR_remove_state(0);
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	term();
}

void OpenSSLSupport::init() {
	FileConfiguration conf ("Eyelock.ini");
	m_shadebug = conf.getValue("Eyelock.SSLDebug", false);
	DBAdapter_Keys db;
#ifndef HBOX_PG	
	if(db.OpenFile("keys.db")){
		if(0!= db.ReadDB(m_KeyVec)){
			m_KeyVec.clear();
		}
	}
#else
	if(db.OpenFile("keys.db3")){
		if(0!= db.ReadDB(m_KeyVec)){
			m_KeyVec.clear();
		}
	}
#endif
	if(m_shadebug){
		printf("Keys in DB \n");
		for(int i=0;i<m_KeyVec.size();i++){
			printf("%d -> Common Name:[ %s ]  Validity [ %ld ] \n",i,m_KeyVec[i].first.c_str(),m_KeyVec[i].second);
		}
	}

	
	m_debug =  conf.getValue("Eyelock.OpenSSL.Debug",false);

	const char *rootcapath =  conf.getValue("Eyelock.CAPath","./rootCert/rootCA.cert");
	ReadCert(&m_rca,rootcapath);

	m_cert_ctx=X509_STORE_new();
	if(m_rca){
		int ret = X509_STORE_add_cert(m_cert_ctx,m_rca);
		X509_STORE_set_flags(m_cert_ctx, 0);
	}

	__mutex = (pthread_mutex_t **) calloc(CRYPTO_num_locks(),sizeof(pthread_mutex_t *));

	if (__mutex == NULL)
		throw "Failed to create SSL critical sections required for OpenSSL";

	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		__mutex[i] = (pthread_mutex_t *)calloc(1,sizeof(pthread_mutex_t));

		int rv = pthread_mutex_init(__mutex[i], NULL);
		if (rv != 0 || __mutex[i] == NULL)
			throw "Failed to create SSL critical sections i required for OpenSSL";
	}

	CRYPTO_set_locking_callback(OpenSSLLockingCallback);
}

void OpenSSLSupport::term() {
	//Clean up the SSL critical sections
	if (__mutex) {
		for (int i = 0; i < CRYPTO_num_locks(); i++)
			pthread_mutex_destroy(__mutex[i]);

		free(__mutex);
	}

	__mutex = 0;
}

SSL_CTX* OpenSSLSupport::InitCTX(bool bClient) {

   if(m_debug)
		EyelockLog(logger, DEBUG, "OpenSSLSupport::InitCTX"); fflush(stdout);

	SSL_CTX *ctx=0;

	FileConfiguration conf ("Eyelock.ini");
	std::string certPath, certKeyPath, caFilePath;
#ifndef HBOX_PG
	certPath = conf.getValue("Eyelock.SecureCertificate", "./rootCert/certs/nanoNXTDefault.crt");
	certKeyPath = conf.getValue("Eyelock.SecureCertificateKey", "./rootCert/certs/nanoNXTDefault.key");
	caFilePath = conf.getValue("Eyelock.CAPath", "./rootCert/rootCA.cert");
#else
	certPath.assign("rootCert/certs/nanoNXTDefault.crt");
	certKeyPath.assign("rootCert/certs/nanoNXTDefault.key");
	caFilePath.assign("rootCert/rootCA.cert");
#endif
	bool enableTLS = conf.getValue("Eyelock.TLSEnable",false);

	if (bClient){
		if(m_ctxClnt==0){
			if (enableTLS)
				m_ctxClnt= SSL_CTX_new(TLSv1_2_client_method());
			else
				m_ctxClnt= SSL_CTX_new(SSLv23_client_method());
			LoadCertificates(m_ctxClnt,certPath.c_str(), certKeyPath.c_str(), caFilePath.c_str());
		}
		ctx =m_ctxClnt;
	}
	else
	{
		if(m_ctxSrv==0){
			if (enableTLS)
				m_ctxSrv= SSL_CTX_new(TLSv1_2_server_method());
			else
				m_ctxSrv=SSL_CTX_new(SSLv23_server_method());
			LoadCertificates(m_ctxSrv,certPath.c_str(), certKeyPath.c_str(), caFilePath.c_str());
		}
		ctx = m_ctxSrv;
	}
	if (ctx == NULL) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}
void OpenSSLSupport::TermCTX(SSL_CTX *ctx)
{
	if(ctx)
		SSL_CTX_free(ctx);
}


void OpenSSLSupport::ReadCert(X509 **x,const char *fname){
	// Parse the certificatea
	X509 *x5_deviceCert=NULL;
	std:string deviceCert;
	int certLength = FileSize(fname);
	deviceCert.resize(certLength);
	FILE *fp = fopen(fname,"rb");
	if(fp){
		int len = fread((void*)deviceCert.c_str(),1,certLength,fp);
		fclose(fp);
		if(len != certLength )
			return;
	}else{
		return;
	}
	BIO *deviceCertbio = BIO_new_mem_buf((void*)deviceCert.c_str(), certLength);
	x5_deviceCert = PEM_read_bio_X509(deviceCertbio, NULL, NULL, NULL);
	BIO_free_all(deviceCertbio);
	*x = x5_deviceCert;
}

int OpenSSLSupport::CheckIfSignedbyRootCA(X509 *e){
	int ret = -1;
	if((!OpenSSLSupport::m_cert_ctx) ||(!e)){
		return 0;
	}
	if(m_shadebug){
		printf("Rx Sha ::\n");
		for(int i=0;i<20;i++){
			printf("%02x ",e->sha1_hash[i]);
		}
		printf("\n");
	}

	static vector<string> m_sha1;
	bool sha1match=false;
	if(m_sha1.size()>0){
		for(int i=0;i<m_sha1.size();i++){
			if(memcmp(m_sha1[i].c_str(),e->sha1_hash,20) == 0){
				sha1match = true;
				ret = 1;
			}
		}
	}
	if(!sha1match){
		X509_STORE_CTX *csc;
		csc = X509_STORE_CTX_new();
		if (csc == NULL){
			EyelockLog(logger, ERROR, "CheckIfSignedbyRootCA() - Unable to create store \n");
			return -1;
		}
		if(!X509_STORE_CTX_init(csc,OpenSSLSupport::m_cert_ctx,e,0)){
			EyelockLog(logger, ERROR, "CheckIfSignedbyRootCA() - Unable to create store init \n");
			return -2;
		}
		ret = X509_verify_cert(csc);
		X509_STORE_CTX_free(csc);
		if(ret == 1){
			string test;
			vector<string>::iterator it;
			it = m_sha1.begin();
			it = m_sha1.insert ( it ,test );
			m_sha1[0].resize(20);
			memcpy((void*)m_sha1[0].c_str(),e->sha1_hash,20);
		}
	}
	if( ret == 1){
		string commonname;
		time_t expiry=0;
		GetCommonNameAndExpiry(e,commonname,expiry);
		bool match = false;
	
		if (m_KeyVec.size()>1 && 0 == strcmp("eyelock-pc",commonname.c_str()))
			match = false;
		else {
			for(int i=0;i<m_KeyVec.size();i++){
				if(m_KeyVec[i].first.length() == commonname.length()){
					if(0 == memcmp(m_KeyVec[i].first.c_str(),commonname.c_str(),m_KeyVec[i].first.length())){
						if(m_KeyVec[i].second == expiry) match = true;
					}
				}
			}
		}
		if(!match){
			EyelockLog(logger, ERROR, "Certificate Not There in DB Sorry\n");
			ret = 0;
		}
	}

	return ret;
}

int OpenSSLSupport::GetCommonNameAndExpiry(X509 *e,string& name,time_t& validity){
	bool ret = false;
	string commname;
	int lastpos = -1;
	lastpos = X509_NAME_get_index_by_NID(X509_get_subject_name(e), NID_commonName, lastpos);
	if (lastpos == -1){
		return ret;
	}
	X509_NAME_ENTRY *entry = X509_NAME_get_entry(X509_get_subject_name(e), lastpos);
	unsigned char *commonName = (unsigned char*)ASN1_STRING_data(entry->value);
	commname.insert(0, (const char*)commonName);
	if (commname.length()){
		time_t certtime = -1, local = -1;
		char *str = (char*)ASN1_STRING_data(X509_get_notAfter(e));
		if(m_shadebug)printf("Validity Date %s\n",str);
		tm certTM;
		char dig1[3]={0};
		memset(&certTM, 0, sizeof(struct tm));
		dig1[0]= str[0], dig1[1] = str[1];
		certTM.tm_year = 100 + atoi (dig1);
		dig1[0]= str[2], dig1[1] = str[3];
		certTM.tm_mon = atoi (dig1)-1;
		dig1[0]= str[4], dig1[1] = str[5];
		certTM.tm_mday = atoi (dig1);
		dig1[0]= str[6], dig1[1] = str[7];
		certTM.tm_hour = atoi (dig1);
		dig1[0]= str[8], dig1[1] = str[9];
		certTM.tm_min = atoi (dig1);
		dig1[0]= str[10], dig1[1] = str[11];
		certTM.tm_sec = atoi (dig1);
		time_t expiry = timegm (&certTM);
		ret = true;
		validity = expiry;
		name = commname;
		if(m_shadebug)printf("Common Name:[ %s ]  Validity [ %ld ] \n",name.c_str(),validity);
	}else{
		ret = false;
	}
	return ret;
}

int OpenSSLSupport::VerifyCallback(int preverify_ok, X509_STORE_CTX *ctx){
	int ret1 = -1;
	ret1 = CheckIfSignedbyRootCA( ctx->cert);
	if(ret1 == 1){
		return 1;
	}else{
		return 0;
	}

#if 0
	static X509   *last_cert = NULL;
	char    buf[256];
	X509   *err_cert, *cert;
	int     err, depth;
	SSL    *ssl;
   if(m_debug)
		EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, preverify_ok : %d", preverify_ok); fflush(stdout);

   if ((!preverify_ok) && (ctx != NULL))
   {
	   if(m_debug)
			EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, preverify_ok : %d", preverify_ok); fflush(stdout);
	   return 0;
   }
   if(m_debug)
		EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, Before X509_STORE_CTX_get_error"); fflush(stdout);
   err = X509_STORE_CTX_get_error(ctx);

   if (err != X509_V_OK)
   {
	   if(m_debug)
			EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, Error in the certificate store : %s", get_validation_errstr(err)); fflush(stdout);

	   fprintf(stderr, "\nError in the certificate store : %s\n", get_validation_errstr(err));
	   return 0;
   }
   if(m_debug)
		EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, Before accessing cert"); fflush(stdout);

   cert = ctx->cert;

   if(m_debug)
		EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, Before last_cert check"); fflush(stdout);

   if ((NULL != last_cert) && (NULL != cert))
   {
	   if(m_debug)
	   {
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, X509_get_subject_name for last_cert"); fflush(stdout);
		    X509_NAME_oneline(X509_get_subject_name(last_cert), buf, 256);
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, X509_get_subject_name for last_cert : %s", buf); fflush(stdout);
		    X509_NAME_oneline(X509_get_issuer_name(last_cert), buf, 256);
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, X509_get_issuer_name for last_cert : %s", buf); fflush(stdout);

		    X509_NAME_oneline(X509_get_subject_name(last_cert), buf, 256);
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, X509_get_subject_name for cert : %s", buf); fflush(stdout);
		    X509_NAME_oneline(X509_get_issuer_name(last_cert), buf, 256);
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, X509_get_issuer_name for cert : %s", buf); fflush(stdout);
	   }
	   int check = X509_cmp (cert, last_cert);
	   // Cert was matched already ??
	   if (!check)
	   {
		   if(m_debug)
			    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, last_cert matched"); fflush(stdout);
		   if (NULL != last_cert)
		   {
			   X509_free(last_cert);
			   last_cert = NULL;
		   }
		   last_cert = X509_dup (cert);
		   if(m_debug)
			    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, last_cert matched returning"); fflush(stdout);

		   return preverify_ok;
	   }
   }

   if(m_debug)
	    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, Check if the certificate exists in the database"); fflush(stdout);

   // Check if the certificate exists in the database
   std::string pcName, pcCert, pcKey, pcValidity, certValidity;
   bool isDevice = false;
   int64_t validity = -1;
   int indexOfDevice = -1, lastpos =-1, loc = -1, retVal = -1, matchtime =-1;
   bool ret = false;
   X509_NAME_ENTRY *e;

   loc = -1;

   // Loop through the common names
   for (;;)
   {
	   lastpos = X509_NAME_get_index_by_NID(X509_get_subject_name(ctx->cert), NID_commonName, lastpos);
	   if (lastpos == -1)
			  break;
	   e = X509_NAME_get_entry(X509_get_subject_name(ctx->cert), lastpos);
	   unsigned char *commonName = (unsigned char*)ASN1_STRING_data(e->value);
	   pcName.insert(0, (const char*)commonName);
	   if(m_debug)
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, CommonName in store is : %s", pcName.c_str()); fflush(stdout);

	   retVal = db.GetData(pcName, indexOfDevice, validity, isDevice, pcCert, pcKey);
	   // Found a record in the db, match the validity
	   if (retVal > -1)
	   {
		    time_t ptime = validity;
		    time_t certtime = -1, local = -1;
			char buffer [80], buffer2 [80];
		    strftime (buffer,80,"365 days it's %F:%I:%M%p.",gmtime(&ptime));
		    strftime (buffer2,80,"365 days it's %F:%I:%M%p.",localtime(&ptime));
		    char *str = (char*)ASN1_STRING_data(X509_get_notAfter(cert));
		    string certDate = str;
		   if(m_debug)
				EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback,  gmTime in database : %s , localTime in database : %s , Time in cert : %s \n", buffer, buffer2, str); fflush(stdout);

		    tm certTM, dbTM;
    		char dig1[2];
			memset(&certTM, 0, sizeof(struct tm));
			memset(&dbTM, 0, sizeof(struct tm));

    		dig1[0]= str[0], dig1[1] = str[1];
    		certTM.tm_year = 100 + atoi (dig1);
    		dig1[0]= str[2], dig1[1] = str[3];
    		certTM.tm_mon = atoi (dig1)-1;
    		dig1[0]= str[4], dig1[1] = str[5];
    		certTM.tm_mday = atoi (dig1);
    		dig1[0]= str[6], dig1[1] = str[7];
    		certTM.tm_hour = atoi (dig1);
    		dig1[0]= str[8], dig1[1] = str[9];
    		certTM.tm_min = atoi (dig1);
    		dig1[0]= str[10], dig1[1] = str[11];
    		certTM.tm_sec = atoi (dig1);
    		certtime = timegm (&certTM);

    		if (ptime == certtime)
    		{
    			matchtime = 0;
    		}
	   }
	   if (matchtime == 0)
	   {
		  if(m_debug)
			EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback,  Certificate matched with db"); fflush(stdout);
		  break;
	   }
	   else
	   {
		  if(m_debug)
			EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback,  Certificate wasn't found in db. Looping."); fflush(stdout);
	   }
   }
   if (matchtime == 0)
   {
	   if(m_debug)
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, certificate was in db"); fflush(stdout);

	   if (NULL != last_cert)
	   {
		   X509_free(last_cert);
		   last_cert = NULL;
	   }
	   last_cert = X509_dup (cert);
	   return preverify_ok;
   }
   else
   {
	   if(m_debug)
		    EyelockLog(logger, DEBUG, "OpenSSLSupport::verify_callback, certificate wasn't in db"); fflush(stdout);
	   return 0;
   }
#endif

}

static int session_id_context = 1;

void OpenSSLSupport::LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile,  const char* CAFile)
{
	SSL    *ssl;

	/* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
    	EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
    	EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        ERR_print_errors_fp(stderr);
        abort();
    }

    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
 	   if(m_debug)
 		    EyelockLog(logger, DEBUG, "OpenSSLSupport::LoadCertificates, Private key does not match the public certificate"); fflush(stdout);

        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }

   if(m_debug)
      EyelockLog(logger, DEBUG, "LoadCertificates, Before SSL_CTX_set_verify"); fflush(stdout);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, OpenSSLSupport::VerifyCallback);
    SSL_CTX_set_session_id_context(ctx, (const unsigned char *) &session_id_context, sizeof session_id_context);

    // Let the verify_callback catch the verify_depth error so that we get
    //an appropriate error in the logfile.
    SSL_CTX_set_verify_depth(ctx, 1);

    if(m_debug)
       EyelockLog(logger, DEBUG, "OpenSSLSupport::LoadCertificates, Before SSL_CTX_load_verify_locations"); fflush(stdout);

	// load the trusted client CA certificate into context
	if (SSL_CTX_load_verify_locations(ctx, CAFile, NULL) != 1)
	{
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		ERR_print_errors_fp(stderr);
		return;
	}

	// allow this CA to be sent to the client during handshake
	STACK_OF(X509_NAME) * list = SSL_load_client_CA_file(CAFile);
	if (NULL == list)
	{
	    EyelockLog(logger, ERROR, "Failed to load SSL client CA file.");
		return ;
	}
	SSL_CTX_set_client_CA_list(ctx, list);

    if(m_debug)
       EyelockLog(logger, DEBUG, "LoadCertificates - Load completed"); fflush(stdout);

}


void OpenSSLSupport::cacheSession(const std::string key, SSL_SESSION *sess)
{
	std::map<std::string,SSL_SESSION*>::iterator it=m_sessionCache.find(key);
	if (it != m_sessionCache.end()) {

		if (sess == it->second) {
			// already cached
			return;
		} else { //free the old one.
			SSL_SESSION_free(it->second);
		}
	}
	//cache the new one
	m_sessionCache[key]=sess;
}
SSL_SESSION *OpenSSLSupport::getCachedSession(const std::string key)
{
	std::map<std::string,SSL_SESSION*>::iterator it=m_sessionCache.find(key);
	if(it!=m_sessionCache.end())
	{
		return it->second;
	}
	else
		return NULL;
}

const char* get_validation_errstr(long e) {
		switch ((int) e) {
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
				return "ERR_UNABLE_TO_GET_ISSUER_CERT";
			case X509_V_ERR_UNABLE_TO_GET_CRL:
				return "ERR_UNABLE_TO_GET_CRL";
			case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
				return "ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE";
			case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
				return "ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE";
			case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
				return "ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY";
			case X509_V_ERR_CERT_SIGNATURE_FAILURE:
				return "ERR_CERT_SIGNATURE_FAILURE";
			case X509_V_ERR_CRL_SIGNATURE_FAILURE:
				return "ERR_CRL_SIGNATURE_FAILURE";
			case X509_V_ERR_CERT_NOT_YET_VALID:
				return "ERR_CERT_NOT_YET_VALID";
			case X509_V_ERR_CERT_HAS_EXPIRED:
				return "ERR_CERT_HAS_EXPIRED";
			case X509_V_ERR_CRL_NOT_YET_VALID:
				return "ERR_CRL_NOT_YET_VALID";
			case X509_V_ERR_CRL_HAS_EXPIRED:
				return "ERR_CRL_HAS_EXPIRED";
			case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
				return "ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD";
			case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
				return "ERR_ERROR_IN_CERT_NOT_AFTER_FIELD";
			case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
				return "ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD";
			case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
				return "ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD";
			case X509_V_ERR_OUT_OF_MEM:
				return "ERR_OUT_OF_MEM";
			case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
				return "ERR_DEPTH_ZERO_SELF_SIGNED_CERT";
			case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
				return "ERR_SELF_SIGNED_CERT_IN_CHAIN";
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
				return "ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY";
			case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
				return "ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE";
			case X509_V_ERR_CERT_CHAIN_TOO_LONG:
				return "ERR_CERT_CHAIN_TOO_LONG";
			case X509_V_ERR_CERT_REVOKED:
				return "ERR_CERT_REVOKED";
			case X509_V_ERR_INVALID_CA:
				return "ERR_INVALID_CA";
			case X509_V_ERR_PATH_LENGTH_EXCEEDED:
				return "ERR_PATH_LENGTH_EXCEEDED";
			case X509_V_ERR_INVALID_PURPOSE:
				return "ERR_INVALID_PURPOSE";
			case X509_V_ERR_CERT_UNTRUSTED:
				return "ERR_CERT_UNTRUSTED";
			case X509_V_ERR_CERT_REJECTED:
				return "ERR_CERT_REJECTED";
			case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
				return "ERR_SUBJECT_ISSUER_MISMATCH";
			case X509_V_ERR_AKID_SKID_MISMATCH:
				return "ERR_AKID_SKID_MISMATCH";
			case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
				return "ERR_AKID_ISSUER_SERIAL_MISMATCH";
			case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
				return "ERR_KEYUSAGE_NO_CERTSIGN";
			case X509_V_ERR_INVALID_EXTENSION:
				return "ERR_INVALID_EXTENSION";
			case X509_V_ERR_INVALID_POLICY_EXTENSION:
				return "ERR_INVALID_POLICY_EXTENSION";
			case X509_V_ERR_NO_EXPLICIT_POLICY:
				return "ERR_NO_EXPLICIT_POLICY";
			case X509_V_ERR_APPLICATION_VERIFICATION:
				return "ERR_APPLICATION_VERIFICATION";
			default:
				return "ERR_UNKNOWN";
		}
	}

RSA * OpenSSLSupport::createRSAWithFilename(char *filename, char *password, int publicCrypto)
{
	// printf("createRSAWithFilename pw %s file %s\n", password, filename);
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	ERR_load_crypto_strings();

	FILE * fp = fopen(filename,"rb");
	if(fp == NULL)
	{
		EyelockLog(logger, ERROR, "Unable to open file %s \n",filename);
		return NULL;
	}

    RSA *rsa= RSA_new() ;
    if(publicCrypto)
	    rsa = PEM_read_RSA_PUBKEY(fp, &rsa,NULL, NULL);
    else
	    rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, password);

    if(rsa==NULL)
    	EyelockLog(logger, ERROR, "Could NOT read RSA private key file\n");

    fclose(fp);
	return rsa;
}

int OpenSSLSupport::privateDecrypt(unsigned char * encrypted, unsigned char *decrypted, char *filename, char *password)
{
	ERR_load_crypto_strings();
	RSA * rsa = createRSAWithFilename(filename, password, 0);
	if (!rsa) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return -1;
	}
	int datalen = RSA_size(rsa);
	int  result = RSA_private_decrypt(datalen, encrypted, decrypted, rsa, RSA_PKCS1_PADDING);  //RSA_PKCS1_OAEP_PADDING
	if ( result == -1 ) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
	}

	return result;


}

int readFile(const char* filename, unsigned char **buffer, int& length)
{
	ifstream is(filename, ifstream::binary);
	if (is)
	{
		is.seekg(0, is.end);
		length = is.tellg();
		is.seekg(0, is.beg);
		*buffer = new unsigned char[length];
		memset(*buffer, 0, length);
		is.read((char *)*buffer, length);
		is.close();
		return 0;
	}
	else
	{
		EyelockLog(logger, ERROR, "Error opening %s\n", filename);
	}
	return 1;
}

bool OpenSSLSupport::verifySign(const unsigned char* msg, int mlen, unsigned char* sig, int slen, char* rsaPubKeyFile)
{
	ERR_load_crypto_strings();
	RSA *pRsaKey = createRSAWithFilename(rsaPubKeyFile, NULL, 1);
	if (!pRsaKey) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}
	EVP_MD_CTX *mdctx = NULL;
	if ( !(mdctx = EVP_MD_CTX_create())) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	EVP_PKEY *pKey = EVP_PKEY_new();
	if (!EVP_PKEY_set1_RSA(pKey, pRsaKey)) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	if (1 != EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pKey)) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	if (1 != EVP_DigestVerifyUpdate(mdctx, msg, mlen)) {
		EyelockLog(logger, ERROR, "%s\n", ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	int result = EVP_DigestVerifyFinal(mdctx, sig, slen);

	RSA_free(pRsaKey);
	EVP_PKEY_free(pKey);
	EVP_MD_CTX_destroy(mdctx);

	if (1 == result) {
	    printf("Signature verification succeeded\n");
	    return true;
	} else {
		EyelockLog(logger, ERROR, "Signature verification failed\n");
		return false;
	}
}

bool OpenSSLSupport::verifySign(const char* file, const char* sigFile, char* rsaPubKeyFile)
{
	unsigned char *fileBuf = NULL;
	int fileLen = 0;
	if (readFile(file, &fileBuf, fileLen)) {
		return 1;
	}

	unsigned char *sigFileBuf = NULL;
	int sigFileLen = 0;
	if (readFile(sigFile, &sigFileBuf, sigFileLen)) {
		return 1;
	}

	/*cout << "TestFile: " <<  testFileBuf << endl;
	cout << "SigFile: " <<  sigFileBuf << endl;
	cout << "TestFile len: " <<  testFileLen << endl;
	cout << "SigFile len: " <<  sigFileLen << endl;*/

	bool ret = false;
	if (fileBuf != NULL && sigFileBuf != NULL) {
		ret = verifySign(fileBuf, fileLen, sigFileBuf, sigFileLen, rsaPubKeyFile);
		delete[] fileBuf;
		delete[] sigFileBuf;
 	}

	return ret;
}
