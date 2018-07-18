/*
 * AESFileEncDec.h
 *
 *  Created on: Feb 24, 2014
 *      Author: developer
 */

#ifndef AESFILEENCDEC_H_
#define AESFILEENCDEC_H_

#include <vector>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
using namespace std;

class AESFileEncDec {
public:
	AESFileEncDec(std::string key);
	virtual ~AESFileEncDec();

	static int EncryptNxt(char* inpfilename,char* outfilename, char *key);
	int Encrypt(char* inpfilename,char* outfilename, char *key=NULL);
	static int DecryptNxt(char* inpfilename,char* outfilename);
	int Decrypt(char* inpfilename,char* outfilename,int offset=0);
	static int CopyFile(char* inpfilename,char* outfilename);
	static int FileSize( const char * szFileName );
	static std::vector<std::string> SplitFile(const char *inpfile,int splitsz);
	static std::string GetMD5(const char *inpfile);
	static std::string GetMD5Buffer(const std::vector<std::string> & update);
	bool DecryptAndMD5(char *inpfilename,std::vector<std::string> & update, std::string& md5);
	static std::string Base64Encode(const char* , unsigned int len);
	static std::string Base64Decode(std::string const& s);
	int EncryptString(std::string inp,std::string& out);
	int DecryptString(std::string inp,std::string& out);
private:
	void SetDecoder();
	void SetEncoder();
	bool Init(unsigned char *salt,unsigned char *key,int keysize);
	bool GenerateKey(unsigned char *salt,unsigned char *key,int keysize);
	EVP_CIPHER_CTX* m_Enc,*m_Dec;
	unsigned char m_Key32[32],m_IV32[32];
};

#endif /* AESFILEENCDEC_H_ */
