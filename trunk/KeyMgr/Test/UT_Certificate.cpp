/*
 * UT_Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "DBAdapter.h"

#include <string.h>
#include "SSLWrap.h"
#include "KeyMgr.h"
#include "AESFileEncDec.h"
#include "PermuteServer.h"
#include "asn1_locl.h"
#include <unistd.h>

namespace tut {

bool ConvertValidity (X509 *cert,  int64_t& certvalidity)
{
	time_t certtime = -1;
    char *str = (char*)ASN1_STRING_data(X509_get_notAfter(cert));
    tm certTM;
	char dig1[2];
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
	certvalidity = (int64_t)timegm (&certTM);
}
int TestBinFile (const int& indexOfDevice, const int&indexOfPC, const std::string& binFile)
{
	DBAdapter db;
	db.OpenFile("keys.db");

	int64_t validity1 = -1, certvalidity = -1;
	bool isDevice1 = false;
	string cert1("123");
	string key1("456");
	string host1("456");

	SSlWrap testWrap;
    testWrap.ReadBinFile(binFile.c_str());
	int certlen;
	char *pem_cert;

	BIO *certbio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(certbio, testWrap.m_LPx509);
	certlen = BIO_pending(certbio);
	pem_cert = (char*)calloc(certlen+1, 1); /* Null-terminate */
	BIO_read(certbio, pem_cert, certlen);
	//printf("%s", pem_key);

	BIO_free_all(certbio);
	FILE *fpcert = fopen("pctest.crt", "w+");
	fwrite (pem_cert, certlen, 1, fpcert);
	fclose (fpcert);

	int ret = db.GetData(host1,indexOfPC,validity1,isDevice1,cert1,key1);
	ConvertValidity (testWrap.m_LPx509,  certvalidity);
	printf("\nPC cert validity match, cert valid %lld, db valid : %lld\n", certvalidity, validity1);
	ensure ("\nPC cert validity match\n", certvalidity == validity1);

	char *pem_dcert;

	BIO *dcertbio = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(dcertbio, testWrap.m_ELx509);
	certlen = BIO_pending(dcertbio);
	pem_dcert = (char*)calloc(certlen+1, 1); /* Null-terminate */
	BIO_read(dcertbio, pem_dcert, certlen);

	ret = db.GetData(host1,indexOfDevice,validity1,isDevice1,cert1,key1);
	ConvertValidity (testWrap.m_ELx509,  certvalidity);
	printf("\nDevice cert validity match, cert valid %lld, db valid : %lld\n", certvalidity, validity1);
	ensure ("\nDevice cert validity match\n", certvalidity == validity1);

	BIO_free_all(dcertbio);
	FILE *fpdcert = fopen("devtest.crt", "w+");
	fwrite (pem_dcert, certlen, 1, fpdcert);
	fclose (fpdcert);

	FILE *fpPEM = fopen("pctest.key", "w+");
	ensure ("Pctest.key creation", fpPEM != NULL);
	PEM_write_PrivateKey(fpPEM,(EVP_PKEY *)testWrap.get_LP_PKEY(),NULL,NULL,0,NULL, NULL);
	fclose(fpPEM);
	string delFile="rm pctest*";
	system (delFile.c_str());
	delFile="rm devtest*";
	system (delFile.c_str());
}

struct CertData {
	CertData()
	{
		AESFileEncDec::CopyFile("../Eyelock/data/keys.db","keys.db");
		FILE *fp = fopen("./rootCert/CA/index.txt.attr","wb");
		fclose(fp);
	}
	~CertData()
	{
	    remove("keys.db");
	}
};

typedef test_group<CertData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("CertData TESTS");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	 SSlWrap ct;
	 ct.mkcert(2048,123456,365*15,false);

	 FILE *fpPEM = fopen("cert.pem", "w+");
	 PEM_write_X509 (fpPEM, (X509 *)ct.get_LP_X509Cert());
	 PEM_write_PrivateKey(fpPEM,(EVP_PKEY *)ct.get_LP_PKEY(),NULL,NULL,0,NULL, NULL);
 	 fclose(fpPEM);
	 FILE *fpcert = fopen("cert.crt", "w+");
	 PEM_write_X509 (fpcert,(X509 *)ct.get_LP_X509Cert());
	 fclose (fpcert);

	 fpPEM = fopen("cert.key", "w+");
	 PEM_write_PrivateKey(fpPEM,(EVP_PKEY *)ct.get_LP_PKEY(),NULL,NULL,0,NULL, NULL);
	 fclose(fpPEM);
	 string delFile= "rm cert*";
	 system (delFile.c_str());

}
template<>
template<>
void testobject::test<2>() {
	KeyMgr km1;
	std::string _return;
	DBAdapter db;
	db.OpenFile("keys.db");
	km1.GenerateAndAddCertKey(-1, "Kiran-PC", 123456, "3650", false);
	db.DeleteData(1);

	}
template<>
template<>
void testobject::test<3>() {
	int featureSize = 2560;
	int binFileSize = 2560*4;
	int iteration = binFileSize / featureSize;
	unsigned char inputBuff[2560*4];
	memset(inputBuff,0x5A,2560*4);
	unsigned char *permBuffer = (unsigned char *) malloc(sizeof(char) * (featureSize << 1));
	{
		PermuteServer *perSerVer;
		perSerVer = new PermuteServer(featureSize, 1);

		for (int i = 0; i < iteration >> 1; i++) {
			perSerVer->Permute(inputBuff + ((i * featureSize) << 1),
					inputBuff + (featureSize * ((i << 1) + 1)), permBuffer);
			memcpy(inputBuff + ((i * featureSize) << 1), permBuffer,
					featureSize << 1);
		}
		delete perSerVer;
	}
	FILE *fp = fopen("Out.bin", "wb");
	fwrite(inputBuff, sizeof(char), 2560*4, fp);
	fclose(fp);

	FILE *fp1 = fopen("Out.bin", "rb");
	fread(inputBuff, sizeof(char), 2560*4, fp1);
	fclose(fp1);

	{
		PermuteServer *perSerVer = new PermuteServer(featureSize, 1);
		for (int i = 0; i < iteration >> 1; i++) {
			perSerVer->Recover(inputBuff + ((i * featureSize) << 1),permBuffer, permBuffer + featureSize);
			memcpy(inputBuff + ((i * featureSize) << 1), permBuffer,featureSize << 1);
		}
		delete perSerVer;
	}

	for(int i=0;i<2560*4;i++){
		ensure("Unpermuted ",inputBuff[i] == 0x5A);
	}
	free(permBuffer);
	remove("Out.bin");
	}
template<>
template<>
void testobject::test<4>() {
	KeyMgr km1;
	bool ret = false;
	km1.GenerateAndAddCertKey(1, "nanonxt141.local", 123456, "3650", true);
	km1.GenerateAndAddCertKey(2, "Kiran-PC", 123456, "3650", false);
	ret = km1.GenerateBinFile(1,2, "CertKeys.bin");
	ensure (ret);
	TestBinFile(1, 2, "CertKeys.bin");
	FILE *fpcert = fopen("pctest.crt", "rb");
	ensure ("pctest.crt", fpcert==NULL);
	FILE *fpdcert = fopen("devtest.crt", "rb");
	ensure ("devtest.crt", fpdcert==NULL);
	FILE *fpPEM = fopen("pctest.key", "rb");
	ensure ("pctest.key", fpPEM==NULL);
	remove ("CertKeys.bin");
	}
template<>
template<>
void testobject::test<5>()
{
//	KeyMgr km1;
//	km1.TestBinFile("CertKeys.bin");

}
template<>
template<>
void testobject::test<6>() {
/*	  time_t rawtime;
	  struct tm * timeinfo;
	  int year, month ,day;
//	struct tm t;
//	time_t t_of_day;
	/*t.tm_year = 2011-1900;
	t.tm_mon = 7; // Month, 0 - jan
	t.tm_mday = 8; // Day of the month
	t.tm_hour = 16;
	t.tm_min = 11;
	t.tm_sec = 42;
	t.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown

	  time ( &rawtime );
	  timeinfo = localtime ( &rawtime );

	  rawtime = mktime ( timeinfo );
	  char buffer2 [80];
    strftime (buffer2,80,"Now it's %F:%I:%M%p.",timeinfo);
    puts (buffer2);

	printf("seconds since the Epoch: %ld\n", (long) rawtime);

	  timeinfo->tm_isdst = -1;
	  timeinfo->tm_min += 365*24*60;
	  rawtime = mktime ( timeinfo );

	printf("365 days seconds since the Epoch: %ld\n", (long) rawtime);
	  char buffer [80];
    strftime (buffer,80,"365 days it's %F:%I:%M%p.",timeinfo);
    puts (buffer);

*/
	//return (long) t_of_day;

}
}
