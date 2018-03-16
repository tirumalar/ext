/*
 * AES.cpp
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#include "AESClass.h"
#include "string.h"

int AES::GenerateKey(unsigned char *salt,unsigned char *key,int keysize){
#ifdef UNITTEST
	int i, nrounds = 5;

	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt,key,keysize, nrounds, m_Key32, m_IV32);
	if (i != 32) {
		printf("Key size is %d bits - should be 256 bits\n", i);
		printf("Error in AES Encryption Init\n");
		throw("Error in AES Init");
		return -1;
	}
#endif
	return 0;
}

void AES::SetKey(const unsigned char *ptr,int sz){
	memcpy(m_Key32,ptr,sz);
}

void AES::SetIV(const unsigned char *ptr,int sz){
	memcpy(m_IV32,ptr,sz);
}

AES::AES():m_Debug(0){
	memset(m_IV32,0,32);
	memset(m_Key32,0,32);
	m_Debug = 0;

#ifdef AES_ENCRYPTION
	EVP_CIPHER_CTX_init(&m_Enc);
	EVP_CIPHER_CTX_init(&m_Dec);
#endif
#ifdef DES_ENCRYPTION
	EVP_CIPHER_CTX_init(&m_DESEnc);
	EVP_CIPHER_CTX_init(&m_DESDec);
#endif
}

AES::~AES() {
#ifdef AES_ENCRYPTION
	EVP_CIPHER_CTX_cleanup(&m_Enc);
	EVP_CIPHER_CTX_cleanup(&m_Dec);
#endif
#ifdef DES_ENCRYPTION
	EVP_CIPHER_CTX_cleanup(&m_DESEnc);
	EVP_CIPHER_CTX_cleanup(&m_DESDec);
#endif
}

#ifdef AES_ENCRYPTION
int AES::EncryptAes(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	int f_len=0;
	int len =numbytes;
	int c_len = 0;

	EVP_EncryptInit_ex(&m_Enc, EVP_aes_256_cbc(), NULL, m_Key32, m_IV32);
	EVP_EncryptUpdate(&m_Enc,out, &c_len, inp,len);
	EVP_EncryptFinal_ex(&m_Enc, out+c_len, &f_len);

	ret = c_len + f_len;

	if(m_Debug){
		printf("Enc::Ret:: %d\n",ret);
		printf("Enc::Inp:: %.*s\n",numbytes,inp);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)inp[i]);
		}
		printf("\n");
		printf("Enc::Out:: %.*s\n",ret,out);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)out[i]);
		}
		printf("\n");
	}
	return ret;
}

int AES::DecryptAes(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	/* plaintext will always be equal to or lesser than length of ciphertext*/
	int p_len = 0, f_len = 0;
	EVP_DecryptInit_ex(&m_Dec, EVP_aes_256_cbc(), NULL, m_Key32, m_IV32);
	EVP_DecryptUpdate(&m_Dec, out, &p_len, inp, numbytes);
	//if(numbytes > p_len)
	EVP_DecryptFinal_ex(&m_Dec, out+p_len, &f_len);
	ret = p_len + f_len;
	if(m_Debug){
		printf("Dec::Ret:: %d, %d, %d\n",p_len, f_len, ret);
	}
	return ret;
}
int AES::EncryptAes128(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	int f_len=0;
	int len =numbytes;
	int c_len = 0;
	unsigned char outtmp[256];

	EVP_EncryptInit_ex(&m_Enc, EVP_aes_128_cbc(), NULL, m_Key32, m_IV32);
	EVP_CIPHER_CTX_set_padding(&m_Dec, 0);
	EVP_EncryptUpdate(&m_Enc,outtmp, &c_len, inp,len);
	EVP_EncryptFinal_ex(&m_Enc, outtmp+c_len, &f_len);

	ret = c_len + f_len;
	memcpy(out, outtmp, numbytes);

	if(m_Debug){
		printf("Enc::Ret:: %d\n",ret);
		printf("Enc::Inp:: %.*s\n",numbytes,inp);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)inp[i]);
		}
		printf("\n");
		printf("Enc::Out:: %.*s\n",ret,out);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)out[i]);
		}
		printf("\n");
	}
	return ret;
}

int AES::DecryptAes128(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	/* plaintext will always be equal to or lesser than length of ciphertext*/
	int p_len = 0, f_len = 0;
	unsigned char outtmp[numbytes];
	EVP_DecryptInit_ex(&m_Dec, EVP_aes_128_cbc(), NULL, m_Key32, m_IV32);
	EVP_CIPHER_CTX_set_padding(&m_Dec, 0);
	EVP_DecryptUpdate(&m_Dec, outtmp, &p_len, inp, numbytes);
	//if(numbytes > p_len)
		EVP_DecryptFinal_ex(&m_Dec, outtmp+p_len, &f_len);
	ret = p_len + f_len;
	memcpy(out, outtmp, numbytes);
	if(m_Debug){
		printf("Dec::Ret:: %d, %d, %d\n",p_len, f_len, ret);
	}

	return ret;
}
#endif

#ifdef DES_ENCRYPTION
int AES::EncryptDes(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	int f_len=0;
	int len =numbytes;
	int c_len = 0;
	EVP_EncryptInit_ex(&m_DESEnc, EVP_des_cbc(), NULL, m_Key32, m_IV32);
	EVP_EncryptUpdate(&m_DESEnc,out, &c_len, inp,len);
    EVP_EncryptFinal_ex(&m_DESEnc, out+c_len, &f_len);
	ret = c_len + f_len;

	if(m_Debug){
		printf("Enc::Ret:: %d\n",ret);
		printf("Enc::Inp:: %.*s\n",numbytes,inp);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)inp[i]);
		}
		printf("\n");
		printf("Enc::Out:: %.*s\n",ret,out);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)out[i]);
		}
		printf("\n");
	}
	return ret;
}

int AES::DecryptDes(const unsigned char *inp,unsigned char*out,int numbytes){
	int ret = 0;
	/* plaintext will always be equal to or lesser than length of ciphertext*/
	int p_len = 0, f_len = 0;
	EVP_DecryptInit_ex(&m_DESDec, EVP_des_cbc(), NULL, m_Key32, m_IV32);
	EVP_DecryptUpdate(&m_DESDec, out, &p_len, inp, numbytes);
	if(numbytes > p_len)
		EVP_DecryptFinal_ex(&m_DESDec, out+p_len, &f_len);

	ret = p_len + f_len;
	if(m_Debug){
		printf("Dec::Ret:: %d\n",ret);
		printf("Dec::Inp:: %.*s\n",numbytes,inp);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)out[i]);
		}
		printf("\n");
		printf("Dec::Out:: %.*s\n",numbytes,out);
		for(int i=0;i<numbytes;i++){
			printf("%02x ",(unsigned char)out[i]);
		}
		printf("\n");
	}
	return ret;
}
#endif

int AES::Decrypt(const unsigned char *inp,unsigned char*out,int numbytes){
#ifdef AES_ENCRYPTION
		return DecryptAes(inp,out,numbytes);
#endif
#ifdef DES_ENCRYPTION
		return DecryptDes(inp,out,numbytes);
#endif
#ifdef NO_ENCRYPTION
		memcpy(out,inp,numbytes);
		return numbytes;
#endif
}

int AES::DecryptFile(const char *filename,unsigned char*out){
	FILE * fp = fopen(filename,"rb");
	if(fp == NULL){
		printf("Unable to open file %s \n",filename);
		return -1;
	}
	int result;
	unsigned char in[1024];
	int numbytes = fread(in, 1, 1024, fp);
	fclose(fp);
	result = Decrypt((const unsigned char *)in, out, numbytes);
	return result;
}

int AES::Encrypt(const unsigned char *inp,unsigned char*out,int numbytes){
#ifdef AES_ENCRYPTION
		return EncryptAes(inp,out,numbytes);
#endif
#ifdef DES_ENCRYPTION
		return EncryptDes(inp,out,numbytes);
#endif
#ifdef NO_ENCRYPTION
		memcpy(out,inp,numbytes);
		return numbytes;
#endif
}
