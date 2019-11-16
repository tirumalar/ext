/*
 * AESFileEncDec.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: developer
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include "AESFileEncDec.h"
#include <unistd.h>

#define MIN(A,B) (A<B?(A):(B))
#define CHUNKSIZE 1024

int AESFileEncDec::FileSize( const char * szFileName ){
  struct stat fileStat;
  int err = stat( szFileName, &fileStat );
  if (0 != err) return 0;
  return fileStat.st_size;
}
std::string AESFileEncDec::GetMD5(const char * inpfile){
    FILE *inFile = fopen (inpfile, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	std::string md5;
	if(inFile == NULL){
		printf ("%s can't be opened.\n", inpfile);
		return md5;
	}
	unsigned char md5sum[MD5_DIGEST_LENGTH]={0};
	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (md5sum,&mdContext);

	char buff[32] ={0};
	int l=0;
	for(int i=0;i<MD5_DIGEST_LENGTH; i++) {
		l +=sprintf(buff+l,"%02x",*(md5sum+i));
	}
	md5.assign(buff,MD5_DIGEST_LENGTH*2);
	fclose(inFile);
	return md5;
}

std::vector<std::string> AESFileEncDec::SplitFile(const char *inpfile,int splitsz){
	std::vector<std::string> update;
    FILE *inFile = fopen (inpfile, "rb");
	int bytes;
	if(inFile == NULL){
		printf ("%s can't be opened.\n", inpfile);
		return update;
	}
	unsigned char *data = (unsigned char *)malloc(splitsz);
	while ((bytes = fread(data,1,splitsz,inFile)) != 0){
		update.push_back(std::string((char*)data,bytes));
	}
	free(data);
	return update;
}

std::string AESFileEncDec::GetMD5Buffer(const std::vector<std::string>& update){
	MD5_CTX mdContext;
	MD5_Init (&mdContext);
	for(size_t i=0;i < update.size();i++){
		MD5_Update(&mdContext, update[i].c_str(),update[i].length());
	}
    unsigned char md5sum[MD5_DIGEST_LENGTH]={0};
	MD5_Final(md5sum,&mdContext);
	char buff[32] ={0};
	int l=0;
	for(int i=0; i <MD5_DIGEST_LENGTH; i++) {
		l +=sprintf(buff+l,"%02x",*(md5sum+i));
	}
	std::string md5;
	md5.assign(buff,MD5_DIGEST_LENGTH*2);
	return md5;
}
AESFileEncDec::AESFileEncDec(std::string key):m_Enc(NULL),m_Dec(NULL){
	memset(m_Key32,0,32);
	memset(m_IV32,0,32);
	bool success = Init((unsigned char *)"eyelockinc",(unsigned char *)key.c_str(),key.length());
	if(!success){
		printf("Keys not Generated properly \n");
	}
}
void AESFileEncDec::SetDecoder(){
	if(m_Dec){
		EVP_CIPHER_CTX_cleanup(m_Dec);
	}else{
		m_Dec = EVP_CIPHER_CTX_new();
	}
	EVP_CIPHER_CTX_init(m_Dec);
	int success = EVP_DecryptInit_ex(m_Dec, EVP_aes_256_cbc(), NULL, m_Key32, m_IV32);
	if(!success){
		printf("EVP_DecryptInit_ex Init failed \n");
	}
}
void AESFileEncDec::SetEncoder(){
	if(m_Enc){
		EVP_CIPHER_CTX_cleanup(m_Enc);
	}else{
		m_Enc = EVP_CIPHER_CTX_new();
	}
	EVP_CIPHER_CTX_init(m_Enc);
	int success = EVP_EncryptInit_ex(m_Enc, EVP_aes_256_cbc(), NULL, m_Key32, m_IV32);
	if(!success){
		printf("EVP_EncryptInit_ex Init failed \n");
	}
}

bool AESFileEncDec::Init(unsigned char *salt,unsigned char *key,int keysize){
	return GenerateKey(salt,key,keysize);
}

bool AESFileEncDec::GenerateKey(unsigned char *salt,unsigned char *key,int keysize){
	int i, nrounds = 419;
	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt,key,keysize, nrounds, m_Key32, m_IV32);
	if (i != 32) {
		return false;
	}
	return true;
}

AESFileEncDec::~AESFileEncDec() {

	if(m_Enc){
		EVP_CIPHER_CTX_cleanup(m_Enc);
		EVP_CIPHER_CTX_free(m_Enc);
	}
	if(m_Dec){
		EVP_CIPHER_CTX_cleanup(m_Dec);
		EVP_CIPHER_CTX_free(m_Dec);
	}
}
int AESFileEncDec::EncryptNxt(char* inpfilename,char* outfilename, char *key){
	AESFileEncDec t(key);
	return t.Encrypt(inpfilename,outfilename,key);
}


int AESFileEncDec::CopyFile(char* inpfilename,char* outfilename){
	bool success=false;
	FILE *fi=fopen(inpfilename,"rb");
	FILE *fo=fopen(outfilename,"wb");
	if((fi == NULL) ||(fo == NULL)){
		return 0;
	}
	int bytesread=0,written=0;
	int chunk=CHUNKSIZE;
	unsigned char in[CHUNKSIZE]={0};
	unsigned char out[CHUNKSIZE+32]={0};
	int fs = FileSize(inpfilename);
	int clen = 0,flen=0;
	while(fs > bytesread){
		//Lets read chunk of 1k
		int len = fread(in,1,chunk,fi);
		len = MIN(len,chunk);
		int outlen = fwrite(in,1,len,fo);
		written +=outlen;
		bytesread +=len;
	}
	if(fi)
		fclose(fi);
	if(fo)
		fclose(fo);
	return written;
}

int AESFileEncDec::Encrypt(char* inpfilename,char* outfilename, char *key){
	SetEncoder();
	bool success=false;
	FILE *fi=fopen(inpfilename,"rb");
	FILE *fo=fopen(outfilename,"wb");
	int bytesread=0,written=0;
	int chunk=CHUNKSIZE;
	unsigned char in[CHUNKSIZE]={0};
	unsigned char out[CHUNKSIZE+32]={0};
	int fs = FileSize(inpfilename);
	int clen = 0,flen=0;
	if(key){
		AESFileEncDec t("000Athens9845282659");
		string inp,k(key);
		inp.resize(32);
		memset((void*)inp.c_str(),0,32);
		memcpy((void*)inp.c_str(),(void*)k.c_str(),k.length());
		string out;
		t.EncryptString(inp,out);
		int outlen = fwrite(out.c_str(),1,48,fo);
		written+= outlen;
	}
	while(fs > bytesread){
		//Lets read chunk of 1k
		int len = fread(in,1,chunk,fi);
		len = MIN(len,chunk);
		success = (EVP_EncryptUpdate(m_Enc,out,&clen,in,len) !=0 );
		if(!success){
			printf("EVP_EncryptUpdate failed \n");
			if(fi)fclose(fi);
			if(fo)fclose(fo);
			return 0;
		}
		int outlen = fwrite(out,1, clen,fo);
		written +=outlen;
		bytesread +=len;
	}
	success = (EVP_EncryptFinal_ex(m_Enc, out+clen, &flen) !=0 );
	if(!success){
		printf("EVP_EncryptFinal failed \n");
		if(fi)fclose(fi);
		if(fo)fclose(fo);
		return 0;
	}
	int outlen = fwrite(out+clen,1,flen,fo);
	written +=outlen;
	if(fi)
		fclose(fi);
	if(fo)
		fclose(fo);
	return written;
}

int AESFileEncDec::EncryptString(std::string inp,std::string& outstr){
	SetEncoder();
	bool success=false;
	int bytesread=0,written=0;
	int chunk=CHUNKSIZE;

	unsigned char out[CHUNKSIZE+32]={0};
	unsigned char *in=(unsigned char *)inp.c_str();
	int fs = inp.length();
	int clen = 0,flen=0;
	while(fs > bytesread){
		int len = MIN(fs-bytesread,chunk);
		success = (EVP_EncryptUpdate(m_Enc,out,&clen,in+bytesread,len) !=0 );
		if(!success){
			printf("EVP_EncryptUpdate failed \n");
			return 0;
		}
		outstr.append((char *)out,clen);
		written +=clen;
		bytesread +=len;
	}
	success = (EVP_EncryptFinal_ex(m_Enc, out+clen, &flen) !=0 );
	if(!success){
		printf("EVP_EncryptFinal failed \n");
		return 0;
	}
	outstr.append((char *)out+clen,flen);
	written +=flen;
	return written;
}

int AESFileEncDec::DecryptNxt(char* inpfilename,char* outfilename){
	FILE *fi=fopen(inpfilename,"rb");
	unsigned char in[CHUNKSIZE]={0};
	int fs = FileSize(inpfilename);
	int ret =0;
	if(fs>48 && fi){
		int len = fread(in,1,48,fi);
		AESFileEncDec t("000Athens9845282659");
		string inp;
		inp.resize(48);
		memcpy((void*)inp.c_str(),(void*)in,48);
		string out;
		t.DecryptString(inp,out);
		fclose(fi);
//		printf("ou %d %s \n",out.length(),out.c_str());
		string test(out.c_str());
//		printf("te %d %s \n",test.length(),test.c_str());

		AESFileEncDec t1(test);
		ret = t1.Decrypt(inpfilename,outfilename,48);
	}
	return ret;
}


int AESFileEncDec::Decrypt(char* inpfilename,char* outfilename,int offset){
	SetDecoder();
	bool success=false;
	FILE *fi=fopen(inpfilename,"rb");
	FILE *fo=fopen(outfilename,"wb");
	int bytesread=0,written=0;
	int chunk=CHUNKSIZE;
	unsigned char in[CHUNKSIZE]={0};
	unsigned char out[CHUNKSIZE+32]={0};
	int fs = FileSize(inpfilename);
	int clen = 0,flen=0;
	{
		int len = fread(in,1,offset,fi);
		bytesread +=len;
	}
	while(fs > bytesread){
		//Lets read chunk of 1k
		int len = fread(in,1,chunk,fi);
		bytesread +=len;
		len = MIN(len,chunk);
		success = (EVP_DecryptUpdate(m_Dec,out,&clen,in,len) !=0 );
		if(!success){
			printf("EVP_DecryptUpdate failed \n");
			if(fi)fclose(fi);
			if(fo)fclose(fo);
			return 0;
		}
		int outlen = fwrite(out,1, clen,fo);
		written +=outlen;
	}
    success = (EVP_DecryptFinal_ex(m_Dec,out+clen,&flen) !=0 );
	if(!success){
		printf("EVP_DecryptFinal failed \n");
		if(fi)fclose(fi);
		if(fo)fclose(fo);
		return 0;
	}
	int outlen = fwrite(out+clen,1,flen,fo);
	written +=outlen;
	if(fi)
		fclose(fi);
	if(fo)
		fclose(fo);
	return written;
}

int AESFileEncDec::DecryptString(std::string inp,std::string& outstr){
	SetDecoder();
	bool success=false;
	int bytesread=0,written=0;
	int chunk=CHUNKSIZE;
	unsigned char *in=(unsigned char *)inp.c_str();
	unsigned char out[CHUNKSIZE+32]={0};
	int fs = inp.length();
	int clen = 0,flen=0;
	while(fs > bytesread){
		int len = MIN(fs-bytesread,chunk);
		success = (EVP_DecryptUpdate(m_Dec,out,&clen,in+bytesread,len) !=0 );
		if(!success){
			printf("EVP_DecryptUpdate failed \n");
			return 0;
		}
		outstr.append((char *)out,clen);
		written +=clen;
		bytesread +=len;
	}
    success = (EVP_DecryptFinal_ex(m_Dec,out+clen,&flen) !=0 );
	if(!success){
		printf("EVP_DecryptFinal failed \n");
		return 0;
	}
	outstr.append((char *)out+clen,flen);
	written +=flen;
	return written;
}

bool AESFileEncDec::DecryptAndMD5(char *inpfilename,std::vector<std::string> & update,std::string& md5){
	bool success=false;
	update.clear();
	SetDecoder();
	FILE *fi=fopen(inpfilename,"rb");
	int bytesread=0;
	int chunk=CHUNKSIZE;
	unsigned char in[CHUNKSIZE]={0};
	unsigned char out[CHUNKSIZE+32]={0};
	int fs = FileSize(inpfilename);
	int clen = 0,flen=0;
	MD5_CTX mdContext;
	MD5_Init (&mdContext);
	while(fs > bytesread){
		//Lets read chunk of 1k
		int len = fread(in,1,chunk,fi);
		bytesread +=len;
		len = MIN(len,chunk);
		success = (EVP_DecryptUpdate(m_Dec,out,&clen,in,len) !=0 );
		if(clen >0){
			update.push_back(std::string((char*)out,clen));
		}
		if(!success){
			printf("EVP_DecryptUpdate failed \n");
			if(fi)fclose(fi);
			return false;
		}
		MD5_Update (&mdContext, out, clen);
	}
    success = (EVP_DecryptFinal_ex(m_Dec,out+clen,&flen) !=0 );
    if(flen >0){
		update.push_back(std::string((char*)out+clen,flen));
	}
    unsigned char md5sum[MD5_DIGEST_LENGTH]={0};
    MD5_Update (&mdContext,out+clen, flen);
	MD5_Final(md5sum,&mdContext);

	if(!success){
		printf("EVP_DecryptFinal failed \n");
		if(fi)fclose(fi);
		return false;
	}
	if(fi)
		fclose(fi);
	
	char buff[32] ={0};
	int l=0;
	for(int i=0; i <MD5_DIGEST_LENGTH; i++) {
		l +=sprintf(buff+l,"%02x",*(md5sum+i));
	}
	md5.assign(buff,MD5_DIGEST_LENGTH*2);
	return true;
}

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string AESFileEncDec::Base64Encode(const char *inp, unsigned int in_len) {
  const char* base64_chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned char const* bytes_to_encode = (unsigned char*)inp;
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';
  }
  return ret;

}

std::string AESFileEncDec::Base64Decode(std::string const& encoded_string) {
   const std::string base64_chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
