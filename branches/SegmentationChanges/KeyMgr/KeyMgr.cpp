/*
 * KeyMgr.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: developer
 */

#include <fstream>
#include <malloc.h>
#include "KeyMgr.h"
#include "AESFileEncDec.h"
#include "DBAdapter.h"
#include <stdlib.h>
#include <time.h>       /* time_t, struct tm, time, mktime */
#include <openssl/rsa.h>
#include <string>
#include <unistd.h>

#define BITS 1024

KeyMgr::KeyMgr()
{
	m_sslWrap = new SSlWrap();

}

KeyMgr::~KeyMgr()
{
	if (m_sslWrap)
	{
		delete m_sslWrap;
		m_sslWrap = NULL;
	}
}

void KeyMgr::GenerateCertKey(const std::string& _hostname, const int serialNo, const std::string& numDays, const bool isDevice)
{
	//PRINTDEBUG
	std::string hostname = _hostname;
	RemoveSpecial(hostname);

	// Generate cert and private key
	m_sslWrap->mkcert(BITS, serialNo, atoi(numDays.c_str()), isDevice, hostname);

	// Generate the pem file
	std::string tempPemFilename = "temp.pem";
	FILE *fpPEM = fopen(tempPemFilename.c_str(), "wb+");
	if (isDevice)
	{
		 PEM_write_X509 (fpPEM, (X509 *)m_sslWrap->get_EL_X509Cert());
		 PEM_write_PrivateKey(fpPEM,(EVP_PKEY *)m_sslWrap->get_EL_PKEY(),NULL,NULL,0,NULL, NULL);
	}
	else
	{
		 PEM_write_X509 (fpPEM, (X509 *)m_sslWrap->get_LP_X509Cert());
		 PEM_write_PrivateKey(fpPEM,(EVP_PKEY *)m_sslWrap->get_LP_PKEY(),NULL,NULL,0,NULL, NULL);
	}
	fclose(fpPEM);

	// Generate the private key file
	std::string outPkeyFile = hostname + ".key";
	FILE *fpPK = fopen(outPkeyFile.c_str(), "wb+");
	if (isDevice)
	{
		 PEM_write_PrivateKey(fpPK,(EVP_PKEY *)m_sslWrap->get_EL_PKEY(),NULL,NULL,0,NULL, NULL);
	}
	else
	{
		 PEM_write_PrivateKey(fpPK,(EVP_PKEY *)m_sslWrap->get_LP_PKEY(),NULL,NULL,0,NULL, NULL);
	}
	fclose(fpPK);

	// Generate the cert file
	std::string tempCertFile = "temp.crt";
	FILE *fpCert = fopen(tempCertFile.c_str(), "wb+");
	std::string cert, certCmd;
	if (isDevice)
	{
		m_sslWrap->get_EL_X509Cert(cert);
	}
	else
	{
		m_sslWrap->get_LP_X509Cert(cert);
	}
	fwrite (cert.c_str(), cert.length(), 1, fpCert );
	fclose(fpCert);

	// Generate the csr file
	std::string csrFilename = hostname + ".csr";
	std::string csrCmd = "openssl x509 -in ";
	csrCmd += tempCertFile;
	csrCmd += " -signkey ";
	csrCmd += outPkeyFile;
	csrCmd += " -x509toreq -out ";
	csrCmd += csrFilename;
	system (csrCmd.c_str());

	// Generate the signed pem file using CA
	std::string outPemFile = hostname + ".pem";
	certCmd.empty();
	certCmd = "(echo y; echo y) | ./scripts/signwithca.sh ";
	certCmd += csrFilename;
	system (certCmd.c_str());

	// Generate the signed cert file using CA
	std::string outCertFile = hostname + ".crt";
	certCmd.empty();
	certCmd = "./scripts/signcertwithca.sh ";
	certCmd += csrFilename;
	certCmd += " ";
	certCmd += ROOT_CERT;
	certCmd += " ";
	certCmd += ROOT_KEY;
	certCmd += " ";
	certCmd += outCertFile;
	certCmd += " ";
	certCmd += numDays;
	system (certCmd.c_str());

	sync();
}

void KeyMgr::ReplaceCertKeyOnDevice(const std::string& certFilename, const std::string& pkeyFilename)
{
		//copy it to /home/root/./rootCert/certs/nanoNXTDefault.crt
		std::string destc("./rootCert/certs/nanoNXTDefault.crt");
		std::string destc_bkup("./rootCert/certs/nanoNXTDefault.crt.bkup");

		printf("copy %s -> %s \n",(char*)destc.c_str(),(char*)destc_bkup.c_str());
		int retval = AESFileEncDec::CopyFile((char*)destc.c_str(),(char*)destc_bkup.c_str());

		std::string destk("./rootCert/certs/nanoNXTDefault.key");
		std::string destk_bkup("./rootCert/certs/nanoNXTDefault.key.bkup");
		printf("copy %s -> %s \n",(char*)destk.c_str(),(char*)destk_bkup.c_str());
		retval = AESFileEncDec::CopyFile((char*)destk.c_str(),(char*)destk_bkup.c_str());

		printf("copy %s -> %s \n",(char*)pkeyFilename.c_str(),(char*)destk.c_str());
		retval = AESFileEncDec::CopyFile((char*)pkeyFilename.c_str(),(char*)destk.c_str());
		printf("copy %s -> %s \n",(char*)certFilename.c_str(),(char*)destc.c_str());
		retval = AESFileEncDec::CopyFile((char*)certFilename.c_str(),(char*)destc.c_str());
}

void KeyMgr::LoadCertKey(const std::string& certFilename, const std::string& pkeyFilename, const bool isDevice)
{
	// Loads the certificate and private key from the files given
	X509* x_cert = NULL;
	FILE* f_cert = fopen(certFilename.c_str(), "rb");
	PEM_read_X509(f_cert, &x_cert, NULL, NULL);
	fclose (f_cert);

	EVP_PKEY *pKey = NULL;
	FILE* f_key = fopen(pkeyFilename.c_str(), "rb");
	PEM_read_PrivateKey(f_key, &pKey, NULL, NULL);
	fclose (f_key);

	m_sslWrap->fillCertKeyStruct(x_cert, pKey, isDevice);
}

void KeyMgr::LoadCertKey(const char *pCert, int certLength, const char *pKey, int keyLength, const bool isDevice)
{
	// Loads the certificate and private key from the memory given
	X509 *x5_cert = NULL;
	BIO *certbio = BIO_new_mem_buf((void*)pCert, certLength);

	x5_cert = PEM_read_bio_X509(certbio, NULL, NULL, NULL);
	BIO_free_all(certbio);

	EVP_PKEY *evp_key = NULL;
	BIO *keybio = BIO_new_mem_buf((void*)pKey, keyLength);

	evp_key = PEM_read_bio_PrivateKey(keybio, NULL, NULL, NULL);
	BIO_free_all(keybio);

	m_sslWrap->fillCertKeyStruct (x5_cert, evp_key, isDevice);
}


void KeyMgr::AddToDatabase(const int& index, const std::string& hostname, bool isDevice)
{

	// Store this new record in the db
	DBAdapter db;
	db.OpenFile("keys.db");
	int64_t validity = -1;
	if (isDevice)
	{
		ConvertValidity ((X509*)m_sslWrap->get_EL_X509Cert(),  validity);
	}
	else
	{
		ConvertValidity ((X509*)m_sslWrap->get_LP_X509Cert(),  validity);
	}

	bool ret = false;
	if (isDevice)
	{
		std::string cert, key;
		m_sslWrap->get_EL_X509Cert(cert);
		m_sslWrap->get_EL_PKEY(key);

		ret = db.UpdateData(hostname, index,validity,isDevice,cert, key);
		if (ret < 0 )
			fprintf(stderr, "Error adding device certificate in database\n");
	}
	else
	{
		std::string cert, key;
		// Get the string version of cert and key
		m_sslWrap->get_LP_X509Cert(cert);
		m_sslWrap->get_LP_PKEY(key);

		ret = db.UpdateData(hostname,index,validity,isDevice,cert, key);
		if (ret < 0 )
			fprintf(stderr, "Error adding device certificate in database\n");
	}
#ifndef TEST
	std::string delFileCmd;
	//delFileCmd = "rm " + hostname + ".pem"; system(delFileCmd.c_str());
	delFileCmd = "rm " + hostname + ".csr"; system(delFileCmd.c_str());
	//delFileCmd = "rm " + hostname + ".crt"; system(delFileCmd.c_str());
	//delFileCmd = "rm " + hostname + ".key"; system(delFileCmd.c_str());
	delFileCmd = "rm temp.crt"; system(delFileCmd.c_str());
	delFileCmd = "rm temp.pem"; system(delFileCmd.c_str());
#endif
}

void KeyMgr::GenerateAndAddCertKey (const int& index, const std::string& hostname, const int serialNo,
									const std::string& numDays, const bool isDevice)
{
	std::string certFile = hostname + ".crt";
	std::string pkeyFile = hostname + ".key";

	GenerateCertKey(hostname, serialNo, numDays, isDevice);
	if (isDevice)
	{
		ReplaceCertKeyOnDevice(certFile, pkeyFile);
	}
	LoadCertKey(certFile, pkeyFile, isDevice);
	AddToDatabase(index, hostname, isDevice);

#ifndef TEST
	std::string delFileCmd;
	delFileCmd = "rm " + hostname + ".pem"; system(delFileCmd.c_str());
	delFileCmd = "rm " + hostname + ".csr"; system(delFileCmd.c_str());
	delFileCmd = "rm " + hostname + ".crt"; system(delFileCmd.c_str());
	delFileCmd = "rm " + hostname + ".key"; system(delFileCmd.c_str());
	delFileCmd = "rm temp.crt"; system(delFileCmd.c_str());
	delFileCmd = "rm temp.pem"; system(delFileCmd.c_str());
#endif
}

bool KeyMgr::DeleteAllKeys()
{
	DBAdapter db;
	db.OpenFile("keys.db");
	bool ret = false;
	int retVal = -1;
	// Delete all 'isDevice=0'-keys except host='eyelock-pc'
	retVal = db.DeleteAllKeys();
	if (retVal < 0)
	{
		fprintf(stderr, "Error deleting data");
		return ret;
	}
	return true;
}

int KeyMgr::GetKeyNumber()
{
	DBAdapter db;
	db.OpenFile("keys.db");
	bool ret = false;
	int retVal = -1;
	// Get number of certificates in database
	retVal = db.GetCertCount();
	if (retVal < 0)
	{
		fprintf(stderr, "Database error");
		return ret;
	}
	else
	{
		return retVal;
	}
}


void KeyMgr::WriteBinFile(const std::string& outputFile)
{
	// Generate the bin file
	int binFileSize = CERTKEYFILESIZE;
	unsigned char *bindata = (unsigned char *) (malloc(binFileSize));
	m_sslWrap->CreateBinFile(bindata);
	FILE *fp  =  fopen(outputFile.c_str(),"wb");
	if (fp != NULL)
	{
		fwrite(bindata, sizeof(char), binFileSize, fp);
		fclose(fp);
	}
	else
	{
		std::cerr << "Error opening file " << outputFile << std::endl;
	}
	free(bindata);
}

bool KeyMgr::GenerateBinFile (const int& indexOfDevice, const int& indexOfPC, const std::string& outputFile)
{
	//PRINTDEBUG
	DBAdapter db;
	db.OpenFile("keys.db");
	std::string deviceName, pcName;
	std::string deviceCert, deviceKey;
	std::string pcCert, pcKey;
	bool isDevice = false;
	int64_t validity = -1;
	bool ret = false;
	int retVal = -1;

	// Retrieve the db record of the device
	retVal = db.GetData(deviceName, indexOfDevice, validity, isDevice, deviceCert, deviceKey);

	if (retVal < 0)
	{
		fprintf(stderr, "Error retrieving data. Index of device record sent was : %d", indexOfDevice);
		return ret;
	}
	if (!isDevice)
	{
		fprintf(stderr, "Index of device record sent was : %d, record is not for device!!", indexOfDevice);
		return ret;
	}

	LoadCertKey(deviceCert.c_str(), deviceCert.length(), deviceKey.c_str(), deviceKey.length(), isDevice);

	// Retrieve the db record of the pc
	retVal = db.GetData(pcName, indexOfPC, validity, isDevice, pcCert, pcKey);

	if (retVal < 0)
	{
		fprintf(stderr, "Error retrieving data. Index of PC record sent was : %d", indexOfPC);
		return ret;
	}
	if (isDevice)
	{
		fprintf(stderr, "Index of PC record sent was : %d record is not for pc!!", indexOfPC);
		return ret;
	}

	LoadCertKey(pcCert.c_str(), pcCert.length(), pcKey.c_str(), pcKey.length(), isDevice);

	WriteBinFile(outputFile);
	return true;
}

bool KeyMgr::GenerateBinFile(const int& indexOfDevice, const std::string& pcCertFile, const std::string& pcKeyFile, const std::string& outputFile)
{
	//PRINTDEBUG
	DBAdapter db;
	db.OpenFile("keys.db");
	std::string deviceName;
	std::string deviceCert, deviceKey;
	bool isDevice;
	int64_t validity;
	bool ret = false;
	int retVal = -1;

	// Retrieve the db record of the device
	retVal = db.GetData(deviceName, indexOfDevice, validity, isDevice, deviceCert, deviceKey);

	if (retVal < 0)
	{
		fprintf(stderr, "Error retrieving data. Index of device record sent was : %d", indexOfDevice);
		return ret;
	}
	if (!isDevice)
	{
		fprintf(stderr, "Index of device record sent was : %d, record is not for device!!", indexOfDevice);
		return ret;
	}
	LoadCertKey(deviceCert.c_str(), deviceCert.length(), deviceKey.c_str(), deviceKey.length(), true);

	LoadCertKey(pcCertFile, pcKeyFile, false);

	WriteBinFile(outputFile);
	return true;
}

void KeyMgr::GetSerialNumber(std::string& _return)
{
	std::string result;//="Unable to Fetch";
	string m_serialNumberFile;
	FILE * pFile;
	size_t lSize;
	char * buffer;
	size_t Result;
	std::string Serial_num;
	m_serialNumberFile.assign("/mnt/mmc/id.txt");
	pFile = fopen(m_serialNumberFile.c_str() , "r" );
	if (pFile==NULL)
	{
		_return.assign(result);
		return;
	}
	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);
	buffer = (char*) malloc (sizeof(char)*lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error",stderr);
		_return.assign(result);
		return;
	}

	// copy the file into the buffer:
	Result = fread (buffer,1,lSize,pFile);
	if (Result != lSize)
	{
		fputs ("Reading error",stderr);
		_return.assign(result);
		return;
	}
	result.assign(buffer,Result);
	fclose (pFile);
	free (buffer);
	_return.assign(result);
}

bool KeyMgr::SetSerialNumber(const std::string& serialNum)
{
	bool bRet=false;
	std::fstream fs;
	string m_serialNumberFile;
	m_serialNumberFile.assign("/mnt/mmc/id.txt");
	fs.open (m_serialNumberFile.c_str(), std::fstream::out |std::fstream::binary| std::fstream::trunc);
	fs<<serialNum.c_str();
	fs.flush();
	fs.close();
	sync();
	std::string data;
	std::fstream fs1;
	fs1.open (m_serialNumberFile.c_str(), std::fstream::in | std::fstream::binary);
	fs1>>data;
	fs1.flush();
	fs1.close();
	if(serialNum.compare(data.c_str())==0)
		bRet=true;
	return bRet;
}

int KeyMgr::EncryptFile(char* inpfilename,char* outfilename, char* keyver)
{
	return AESFileEncDec::EncryptNxt(inpfilename, outfilename,keyver);
}

int KeyMgr::DecryptFile(char* inpfilename,char* outfilename)
{
	FILE *fi=fopen(inpfilename,"rb");
	if(fi){
		unsigned char in[2]={0};
		int len = fread(in,1,2,fi);
		fclose(fi);
		if((in[0] == 0x1f) && (in[1] == 0x8b)&&(len == 2)){
			return AESFileEncDec::CopyFile(inpfilename,outfilename);
		}
	}
	return AESFileEncDec::DecryptNxt(inpfilename, outfilename);
}

void KeyMgr::RemoveSpecial(std::string& str)
{
	char *ptr = (char*)str.c_str();
	for(unsigned int i=0;i<str.length();i++){
		if(ptr[i]=='/'){
			ptr[i] ='_';
		}
	}
}

int64_t KeyMgr::GetEpochTime(int days)
{

	  time_t rawtime;
	  struct tm * timeinfo;

	  time ( &rawtime );
	  timeinfo = localtime ( &rawtime );
	  timeinfo->tm_isdst = -1;
	  timeinfo->tm_min += days*24*60;

	  rawtime = mktime ( timeinfo );

	  printf("365 days seconds since the Epoch: %ld\n", (int64_t) rawtime);
	  return (int64_t) rawtime;
}

bool KeyMgr::ConvertValidity (const X509 *cert,  int64_t& certvalidity)
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
