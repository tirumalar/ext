#include "SSLWrap.h"
#include "PermuteServer.h"
#include <cstring>
#include <iostream>
#include <unistd.h>

//#define CERTKEYFILESIZE 20480;
#define FEATURELENGTH 1280;
static void callback(int p, int n, void *arg);

SSlWrap::SSlWrap() {
	m_ELpkey = NULL;
	m_ELx509 = NULL;
	m_LPpkey = NULL;
	m_LPx509 = NULL;
	m_CApkey = NULL;
	setOrganizationIssuerName();
	setOrganizationalIssuerUnit();
	setCountryIssuerName();
	setCommonName();
	setOrganizationLocation();
	setOrganizationEmailAddress();
}

int SSlWrap::mkcert(int bits, int serial, int days,bool isdevice, std::string& hostname)
{
	FILE* f_key = fopen(ROOT_KEY, "rb");
	PEM_read_PrivateKey(f_key, &m_CApkey, NULL, NULL);
	fclose (f_key);

	setCommonName (hostname);
	return mkcert(bits, serial, days, isdevice);
}

int SSlWrap::mkcert(int bits, int serial, int days, bool isdevice) {
#ifdef __BFIN__
	system("date");
#endif

	X509 *x;
	EVP_PKEY *pk;
	RSA *rsa;
	X509_NAME *name = NULL, *name2 = NULL;

	int set = -1;
	int loc = -1;
	int index = -1;

	int type = V_ASN1_PRINTABLESTRING;
	int len = 0;
	if ((pk = EVP_PKEY_new()) == NULL) {
		abort();
		return (0);
	}

	if ((x = X509_new()) == NULL)
		goto err;

	typedef void (*pointer)(int, int, void *);
	rsa = RSA_generate_key(bits, RSA_F4, callback, NULL);
	if (!EVP_PKEY_assign_RSA(pk,rsa)) {
		abort();
		goto err;
	}
	rsa = NULL;
	X509_set_version(x, 15);
	ASN1_INTEGER_set(X509_get_serialNumber(x), serial);

	X509_gmtime_adj(X509_get_notBefore(x), (long) -60 * 60 * 24 * 2 *days);
	X509_gmtime_adj(X509_get_notAfter(x), (long) 60 * 60 * 24 * days * 2);

	X509_set_pubkey(x, pk);

	name = X509_get_subject_name(x);

	X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)m_countryIssuerName, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "ST",  MBSTRING_ASC, (unsigned char *)m_organizationLocation, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "L",  MBSTRING_ASC, (unsigned char *)m_organizationLocation, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)m_organizationIssuerName, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (unsigned char *)m_organizationalIssuerUnit, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)m_CommonName, -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "emailAddress", MBSTRING_ASC, (unsigned char *)m_emailAddress, -1, -1, 0);

	X509_set_issuer_name(x, name);
	X509_set_subject_name(x, name);

	if (NULL == m_CApkey)
	{
		if (!X509_sign(x, pk, EVP_sha1()))
			goto err;
	}
	else
	{
		if (!X509_sign(x, m_CApkey, EVP_sha1()))
			goto err;
	}

	if (isdevice) {
		if ((m_ELpkey != NULL)) {
			EVP_PKEY_free(m_ELpkey);
		}
		if ((m_ELx509 != NULL)) {
			X509_free(m_ELx509);
			m_ELx509 = NULL;
		}
		m_ELx509 = x;
		m_ELpkey = pk;

#ifdef TEST
		FILE *fpPEM = fopen("m_ELx509In.pem", "w+");
		X509_print_fp(fpPEM,m_ELx509);
		fclose(fpPEM);
#endif
	} else {
		if ((m_LPpkey != NULL)) {
			EVP_PKEY_free(m_LPpkey);
		}
		if ((m_LPx509 != NULL)) {
			X509_free(m_ELx509);
			m_LPx509 = NULL;
		}
		m_LPx509 = x;
		m_LPpkey = pk;

#ifdef TEST
		FILE *fpPEM = fopen("m_LPx509In.pem", "w+");
		X509_print_fp(fpPEM,m_LPx509);
		fclose(fpPEM);
		fpPEM = fopen("m_LPpkeyIn.key", "w+");
		PEM_write_PrivateKey(fpPEM,m_LPpkey,NULL,NULL,0,NULL, NULL);
		fclose(fpPEM);
#endif
	}

	return (1);
	err: return (0);
}
char *SSlWrap::get_LP_PKEY() {
	return (char*) m_LPpkey;
}
char* SSlWrap::get_LP_X509Cert() {
	return (char*) m_LPx509;
}
char* SSlWrap::get_EL_PKEY() {
	return (char*) m_ELpkey;
}
char* SSlWrap::get_EL_X509Cert() {
	return (char*) m_ELx509;

}

void SSlWrap::get_LP_PKEY(std::string &key)
{
	int keylen;
	char *pem_key;

	BIO *bio = BIO_new(BIO_s_mem());
	PEM_write_bio_PrivateKey(bio, m_LPpkey, NULL, NULL, 0, NULL, NULL);
	keylen = BIO_pending(bio);
	pem_key = (char*)calloc(keylen+1, 1);
	BIO_read(bio, pem_key, keylen);
	key.resize(keylen);
	memcpy((void*)key.c_str(),pem_key,keylen);
	printf("%s", pem_key);

	BIO_free_all(bio);

}

void SSlWrap::get_LP_X509Cert(std::string &cert)
{
	int certlen;
	char *pem_cert;

	BIO *certbio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(certbio, m_LPx509);
	certlen = BIO_pending(certbio);
	pem_cert = (char*)calloc(certlen+1, 1);
	BIO_read(certbio, pem_cert, certlen);
	cert.resize(certlen);
	memcpy((void*)cert.c_str(),pem_cert,certlen);
	BIO_free_all(certbio);
}

void SSlWrap::get_EL_PKEY(std::string &key)
{
	int keylen;
	char *pem_key;

	BIO *bio = BIO_new(BIO_s_mem());
	PEM_write_bio_PrivateKey(bio, m_ELpkey, NULL, NULL, 0, NULL, NULL);
	keylen = BIO_pending(bio);
	pem_key = (char*)calloc(keylen+1, 1);
	BIO_read(bio, pem_key, keylen);
	key.resize(keylen);
	memcpy((void*)key.c_str(),pem_key,keylen);
	printf("%s", pem_key);

	BIO_free_all(bio);
}

void SSlWrap::get_EL_X509Cert(std::string &cert)
{
	int certlen;
	char *pem_cert;

	BIO *certbio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(certbio, m_ELx509);
	certlen = BIO_pending(certbio);
	pem_cert = (char*)calloc(certlen+1, 1);
	BIO_read(certbio, pem_cert, certlen);
	cert.resize(certlen);
	memcpy((void*)cert.c_str(),pem_cert,certlen);
	BIO_free_all(certbio);
}

int SSlWrap::add_ext(X509 *cert, int nid, char *value) {
	X509_EXTENSION *ex;
	X509V3_CTX ctx;
	X509V3_set_ctx_nodb(&ctx);
	X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
	ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
	if (!ex)
		return 0;
	X509_add_ext(cert, ex, -1);
	X509_EXTENSION_free(ex);
	return 1;
}
SSlWrap::~SSlWrap() {
	if (m_ELx509 != NULL) {
		X509_free(m_ELx509);
		EVP_PKEY_free(m_ELpkey);
	}
	if (m_LPx509 != NULL) {
		X509_free(m_LPx509);
		EVP_PKEY_free(m_LPpkey);
	}
#ifndef OPENSSL_NO_ENGINE
	ENGINE_cleanup();
#endif
	CRYPTO_cleanup_all_ex_data();
}
static void callback(int p, int n, void *arg) {
	char c = 'B';

	if (p == 0)
		c = '.';
	if (p == 1)
		c = '+';
	if (p == 2)
		c = '*';
	if (p == 3)
		c = '\n';
	fputc(c, stderr);
}
void SSlWrap::CreateBinFile(unsigned char * buffer) {

	CertKey *LPCERT, *LPKEY, *ELCERT;
	LPCERT = (CertKey *) malloc(sizeof(CertKey));
	fillCertStruct(LPCERT, m_LPx509);

	LPKEY = (CertKey *) malloc(sizeof(CertKey));
	fillKeyStruct(LPKEY, m_LPpkey);

	ELCERT = (CertKey *) malloc(sizeof(CertKey));
	fillCertStruct(ELCERT, m_ELx509);

	saveBinfile(ELCERT, LPCERT, LPKEY, buffer);
	free(LPCERT);
	free(LPKEY);
	free(ELCERT);

}

void SSlWrap::CreateBinFile(char * lpCerData, int lpcertLen, char *lpKeyData,
		int lpKeyLen, char* elcertData, int elcertLen, unsigned char * buffer) {

	CertKey *LPCERT, *LPKEY, *ELCERT;
	LPCERT = (CertKey *) malloc(sizeof(CertKey));
	memcpy(LPCERT->data, lpCerData, lpcertLen);
	LPKEY = (CertKey *) malloc(sizeof(CertKey));
	memcpy(LPKEY->data, lpKeyData, lpKeyLen);
	EVP_PKEY *pkey;
	//unsigned char *newbuf = (unsigned char*)key.c_str();
	d2i_PrivateKey(EVP_PKEY_RSA, &pkey, (const unsigned char **) &lpKeyData, lpKeyLen);
	FILE* fpPEM = fopen("CreateBinFile.key", "w+");
	//  RSA_print_fp(fp,m_pkeyp->pkey.rsa,0);
	PEM_write_PrivateKey(fpPEM,pkey,NULL,NULL,0,NULL, NULL);
	fclose(fpPEM);

	ELCERT = (CertKey *) malloc(sizeof(CertKey));
	memcpy(ELCERT->data, elcertData, elcertLen);
	saveBinfile(ELCERT, LPCERT, LPKEY, buffer);
	free(LPCERT);
	free(LPKEY);
	free(ELCERT);

}

void SSlWrap::CreateBinFile(char* elcertData, char * lpCerData, char *lpKeyData,
		unsigned char * buffer) {

	CertKey *LPCERT, *LPKEY, *ELCERT;
	LPCERT = (CertKey *) malloc(sizeof(CertKey));
	memcpy(LPCERT, lpCerData, sizeof(CertKey));
	LPKEY = (CertKey *) malloc(sizeof(CertKey));
	memcpy(LPKEY, lpKeyData, sizeof(CertKey));
	ELCERT = (CertKey *) malloc(sizeof(CertKey));
	memcpy(ELCERT, elcertData, sizeof(CertKey));
	saveBinfile(ELCERT, LPCERT, LPKEY, buffer);
	free(LPCERT);
	free(LPKEY);
	free(ELCERT);

}
void SSlWrap::saveBinfile(CertKey * p_ELCERT, CertKey * p_LPCERT,
		CertKey * p_LPKEY, unsigned char * outputbuffer) {
	DevKey devkey;
	devkey.ELCERT = *p_ELCERT;
	devkey.LPCERT = *p_LPCERT;
	devkey.LPKEY = *p_LPKEY;

	int binFileSize = CERTKEYFILESIZE;
	memcpy(outputbuffer, &devkey, sizeof(devkey));
	permuteBuffer(outputbuffer, sizeof(devkey));

	FILE *fp  =  fopen("CertKey.bin","wb");
	fwrite(outputbuffer ,sizeof(char), binFileSize, fp );
	fclose(fp);

}

void SSlWrap::fillCertKeyStruct (X509 *x, EVP_PKEY *k, bool isDevice )
{
	if (isDevice)
	{
		if (m_ELx509 != NULL)
		{
			X509_free(m_ELx509);
			EVP_PKEY_free(m_ELpkey);
		}
		m_ELx509 = x;
		m_ELpkey = k;
	}
	else
	{
		if (m_LPx509 != NULL)
		{
			X509_free(m_LPx509);
			EVP_PKEY_free(m_LPpkey);
		}
		m_LPx509 = x;
		m_LPpkey = k;
	}
}

void SSlWrap::fillCertStruct(CertKey *certKey, X509 *x) {

	unsigned char *buf;
	buf = NULL;
	int certlen = i2d_X509(x, &buf);
	memcpy(certKey->data, (void *) buf, certlen);
	certKey->size = certlen;
	for (int i = 0; i < (int) sizeof(certKey->randData); i++) {
		certKey->randData[i] = rand() % 255;
	}
	for (int i = certlen; i < (int) sizeof(certKey->data); i++) {
		certKey->data[i] = rand() % 255;
	}
	if(!buf) OPENSSL_free(buf);

}
void SSlWrap::fillKeyStruct(CertKey *certKey, EVP_PKEY *k) {

	unsigned char *buf;
	buf = NULL;
	int certlen = i2d_PrivateKey(k, NULL);
	i2d_PrivateKey(k, &buf);
	memcpy(certKey->data, (void *) buf, certlen);
	certKey->size = certlen;
	for (int i = 0; i < (int) sizeof(certKey->randData); i++) {
		certKey->randData[i] = rand() % 255;
	}
	for (int i = certlen; i < (int) sizeof(certKey->data); i++) {
		certKey->data[i] = rand() % 255;
	}
	if(buf)
		OPENSSL_free(buf);
}

void SSlWrap::ReadBinFile(unsigned char *bindata, int binCertKeySize) {
	if (m_ELx509 != NULL) {
		X509_free(m_ELx509);
		EVP_PKEY_free(m_ELpkey);
	}
	if (m_LPx509 != NULL) {
		X509_free(m_LPx509);
		EVP_PKEY_free(m_LPpkey);
	}

	m_ELx509 = NULL;
	m_ELpkey = NULL;
	m_LPx509 = NULL;
	m_LPpkey = NULL;
#ifdef TEST
	FILE *fpPEMs = fopen("bindata_bp", "w+");
	fwrite(bindata ,sizeof(char), binCertKeySize, fpPEMs );
	fclose (fpPEMs);
#endif
	recoverBuffer(bindata, binCertKeySize);
#ifdef TEST
	fpPEMs = fopen("bindata_ap", "w+");
	fwrite(bindata ,sizeof(char), binCertKeySize, fpPEMs );
	fclose (fpPEMs);
#endif
	DevKey *devkey;
	devkey = (DevKey *) bindata;
	unsigned char *input;
	input = devkey->LPCERT.data;
	d2i_X509(&m_LPx509, (const unsigned char **) &input, devkey->LPCERT.size);
	input = devkey->ELCERT.data;
	d2i_X509(&m_ELx509, (const unsigned char **) &input, devkey->ELCERT.size);
	input = devkey->LPKEY.data;
	d2i_PrivateKey(EVP_PKEY_RSA, &m_LPpkey, (const unsigned char **) &input,
			devkey->LPKEY.size);

#ifdef TEST
	FILE *fpPEM = fopen("m_LPx509.pem", "w+");
	//X509_print_fp(fpPEM,m_LPx509);
	PEM_write_X509 (fpPEM,m_LPx509);
	fclose(fpPEM);

	if (m_ELx509)
	{
		fpPEM = fopen("m_ELx509.pem", "w+");
		X509_print_fp(fpPEM,m_ELx509);
		fclose(fpPEM);
	}

	if (m_LPpkey)
	{
		//Code for verify Key
		fpPEM = fopen("m_LPpkey.key", "w+");
		//  RSA_print_fp(fp,m_pkeyp->pkey.rsa,0);
		PEM_write_PrivateKey(fpPEM,m_LPpkey,NULL,NULL,0,NULL, NULL);
		fclose(fpPEM);
	}
#endif
}
void SSlWrap::ReadBinFile(const char *filename) {

	int binCertKeySize = CERTKEYFILESIZE;
	unsigned char * bindata = (unsigned char *) malloc(binCertKeySize);
	FILE *fp = fopen(filename, "rb");
	fread(bindata, sizeof(char), binCertKeySize, fp);
	fclose(fp);
	ReadBinFile(bindata, binCertKeySize);
	free(bindata);

}
void SSlWrap::permuteBuffer(unsigned char * inputBuff, int length) {
	int featureSize = FEATURELENGTH;
	int binFileSize = CERTKEYFILESIZE;
	int iteration = binFileSize / featureSize;

	for (int i = length; i < binFileSize; i++) {
		inputBuff[i] = rand() % 255;
	}
	PermuteServer *perSerVer;
	perSerVer = new PermuteServer(featureSize, 1);
	unsigned char *outputBuff = (unsigned char *) malloc(sizeof(char) * (featureSize << 1));
	for (int i = 0; i < iteration >> 1; i++) {
		perSerVer->Permute(inputBuff + ((i * featureSize) << 1),
				inputBuff + (featureSize * ((i << 1) + 1)), outputBuff);
		memcpy(inputBuff + ((i * featureSize) << 1), outputBuff,
				featureSize << 1);
	}
	free(outputBuff);
	delete perSerVer;
}

void SSlWrap::recoverBuffer(unsigned char * EncrypBuffer, int length) {
	int featureSize = FEATURELENGTH;
	int binFileSize = CERTKEYFILESIZE;
	int iteration = binFileSize / featureSize;
	unsigned char * outputBuffer;
	PermuteServer *perSerVer = new PermuteServer(featureSize, 1);

	for (int i = 0; i < iteration >> 1; i++) {
		outputBuffer = (unsigned char *) malloc(
				sizeof(char) * (featureSize << 1));
		perSerVer->Recover(EncrypBuffer + ((i * featureSize) << 1),
				outputBuffer, outputBuffer + featureSize);
		memcpy(EncrypBuffer + ((i * featureSize) << 1), outputBuffer,
				featureSize << 1);
		free(outputBuffer);
	}
	delete perSerVer;
}
void SSlWrap::get_CertKeyPair(bool isDevice, char * output) {
	if (isDevice && m_ELx509 != NULL) {
		if (m_ELx509 != NULL) {
			fillCertStruct((CertKey *) output, m_ELx509);
			fillKeyStruct((CertKey *) (output + sizeof(CertKey)), m_ELpkey);
		}
	} else {
		if (m_LPx509 != NULL) {
			fillCertStruct((CertKey *) output, m_LPx509);
			fillKeyStruct((CertKey *) (output + sizeof(CertKey)), m_LPpkey);
		}
	}
}
void SSlWrap::set_CertKeyPair(char * data, bool isDevice) {
	X509 **X = NULL;
	EVP_PKEY **K = NULL;

	if (isDevice) {
		if (m_ELx509 != NULL) {
			X509_free(m_ELx509);
			EVP_PKEY_free(m_ELpkey);
			m_ELx509 = NULL;
			m_ELpkey = NULL;
		}

		X = &m_ELx509;
		K = &m_ELpkey;
	} else {
		if (m_LPx509 != NULL) {
			X509_free(m_LPx509);
			EVP_PKEY_free(m_LPpkey);
			m_LPx509 = NULL;
			m_LPpkey = NULL;
		}
		X = &m_LPx509;
		K = &m_LPpkey;
	}

	CertKey *m_certKey = (CertKey *) data;
	unsigned char *input;
	input = (unsigned char *) m_certKey->data;
	d2i_X509(X, (const unsigned char **) &input, m_certKey->size);

	m_certKey = (CertKey *) (data + sizeof(CertKey));
	input = (unsigned char *) m_certKey->data;
	d2i_PrivateKey(EVP_PKEY_RSA, K, (const unsigned char **) &input,
			m_certKey->size);

}
void SSlWrap::setOrganizationIssuerName(std::string orgName) {
	memset(m_organizationIssuerName, 0, 256);
	strcpy((char *) m_organizationIssuerName, orgName.c_str());
}

void SSlWrap::setOrganizationalIssuerUnit(std::string orgUnit) {
	memset(m_organizationalIssuerUnit, 0, 256);
	strcpy((char *) m_organizationalIssuerUnit, orgUnit.c_str());
}

void SSlWrap::setCountryIssuerName(std::string countryName) {
	memset(m_countryIssuerName, 0, 256);
	strcpy((char *) m_countryIssuerName, countryName.c_str());
}
void SSlWrap::setCommonName(std::string commonName) {
	memset(m_CommonName, 0, 256);
	strcpy((char *) m_CommonName, commonName.c_str());
}
void SSlWrap::setOrganizationLocation(std::string  location)
{
	memset(m_organizationLocation, 0, 256);
	strcpy((char *) m_organizationLocation, location.c_str());
}

void SSlWrap::setOrganizationEmailAddress(std::string  email)
{
	memset(m_emailAddress, 0, 256);
	strcpy((char *) m_emailAddress, email.c_str());
}

