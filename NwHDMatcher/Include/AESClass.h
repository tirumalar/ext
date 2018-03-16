/*
 * AES.h
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#ifndef AES_H_
#define AES_H_

#include <sys/types.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <string>

#define AES_ENCRYPTION
//#define DES_ENCRYPTION
//#define NO_ENCRYPTION

class AES {
public:
	AES();
	virtual ~AES();
	int Encrypt(const unsigned char *inp,unsigned char*out,int numbytes);
	int Decrypt(const unsigned char *inp,unsigned char*out,int numbytes);
	int DecryptFile(const char *filename,unsigned char*out);
#ifdef AES_ENCRYPTION
	int EncryptAes(const unsigned char *inp,unsigned char*out,int numbytes);
	int DecryptAes(const unsigned char *inp,unsigned char*out,int numbytes);
	int EncryptAes128(const unsigned char *inp,unsigned char*out,int numbytes);
	int DecryptAes128(const unsigned char *inp,unsigned char*out,int numbytes);
#endif

#ifdef DES_ENCRYPTION
	int EncryptDes(const unsigned char *inp,unsigned char*out,int numbytes);
	int DecryptDes(const unsigned char *inp,unsigned char*out,int numbytes);
#endif

	int GenerateKey(unsigned char *salt,unsigned char *key,int keysize);
	void SetKey(const unsigned char *ptr,int sz);
	void SetIV(const unsigned char *ptr,int sz);
	void SetDebug(int val){ m_Debug = val;}
#ifndef UNITTEST
protected:
#endif
	bool m_Debug;
	unsigned char m_Key32[32],m_IV32[32];
#ifdef AES_ENCRYPTION
	EVP_CIPHER_CTX m_Enc,m_Dec;
#endif
#ifdef DES_ENCRYPTION
	EVP_CIPHER_CTX m_DESEnc,m_DESDec;
#endif
};

#endif /* AES_H_ */
