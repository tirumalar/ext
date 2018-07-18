/*
 * AESReader.cpp
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#include "AesRW.h"
#include "AESClass.h"
#include <stdio.h>
#include <string.h>
#include "IrisDBHeader.h"
#include <malloc.h>
AesRW::AesRW(ReaderWriter *reader):RWDecorator(reader),m_Aes(0),m_DBHeader(0) {
	m_OutBuffer = 0;
	m_Aes = new AES();
	m_BufferSize = 25*1024;
	m_InBuffer = (unsigned char*)malloc(m_BufferSize+32);//Extra as the enc requires it

}

void AesRW::Init(void *ptr){
	m_DBHeader = (IrisDBHeader*)ptr;

}
void AesRW::SetKeyAndIV(const unsigned char *key,const unsigned char *iv){
	m_Aes->SetKey(key,m_DBHeader->GetKeySize());
	m_Aes->SetIV(iv,m_DBHeader->GetIVSize());
}

AesRW::~AesRW() {
	if (m_Aes){
		delete m_Aes;
		m_Aes =0;
	}
	if(m_InBuffer){
		free(m_InBuffer);
		m_InBuffer =0;
	}
	if(m_OutBuffer){
		free(m_OutBuffer);
		m_OutBuffer =0;
	}
}

int AesRW::Read(unsigned char*inp,int numbytes,int position){
//Read 32 starting Bytes and store it in IV[]
	int ret = RWDecorator::Read(m_IV,m_DBHeader->GetIVSize(),position);
	if(ret != m_DBHeader->GetIVSize()){
		printf("Either reached EOF or reading Error\n");
		printf("Bytes Read from files were %d rather than %d\n",ret,m_DBHeader->GetIVSize());
		printf("Error in AesRW Read m_IV \n");
		return 0;
	}
	position += m_DBHeader->GetIVSize();
	ret = RWDecorator::Read((unsigned char*)&m_EncSize,4,position);
	if(ret != 4){
		printf("Either reached EOF or reading Error\n");
		printf("Bytes Read from files were %d rather than %d\n",ret,4);
		printf("Error in AesRW Read m_EncSize\n");
		return 0;
	}
	position += 4;

	SetKeyAndIV(m_DBHeader->GetKey(),m_IV);


	if(m_BufferSize < m_EncSize){
		free(m_InBuffer);
		m_InBuffer = (unsigned char*)malloc(m_EncSize+32);//Extra as the enc requires it
		m_BufferSize = m_EncSize;
	}

//Try to find the input Buffer to read and check how many Blocks we need to read
	int readchunk = m_EncSize;
	ret = RWDecorator::Read(m_InBuffer,readchunk,position);

	if(ret != readchunk){
		printf("Either reached EOF or reading Error\n");
		printf("Bytes Read from files were %d rather than %d\n",ret,readchunk);
		printf("Error in AesRW Read \n");
		return 0;
	}
	int ret1 = m_Aes->Decrypt((const unsigned char *)m_InBuffer,inp,readchunk);
//	if(ret1 != readchunk){
//		printf("Error in AesRW Decrypt \n");
//		return 0;
//	}
	return m_DBHeader->GetIVSize()+4+m_EncSize;
}

int AesRW::Write(unsigned char*inp,int numbytes,int position){
	int byteswritten = 0;
	int writechunk = m_BufferSize;
	int bytestobewritten = numbytes;

	while(byteswritten < numbytes){
		if(bytestobewritten >= writechunk){
			int ret1 = m_Aes->Encrypt((const unsigned char *)inp+byteswritten,m_OutBuffer,writechunk);
			if(ret1 != writechunk){
				printf("Error in AesRW Decrypt \n");
				return 0;
			}
			int ret = RWDecorator::Write(m_OutBuffer,writechunk,position);
			if(ret != writechunk){
				printf("Bytes Write to files were %d\n",ret);
				printf("Error in AesRW Write \n");
				return 0;
			}
			position += writechunk;
			byteswritten += writechunk;
			bytestobewritten -= writechunk;
		}else{
			memcpy(m_InBuffer,inp+byteswritten,bytestobewritten);
			//Set zero
			memset(m_InBuffer+bytestobewritten,0,writechunk-bytestobewritten);

			int ret1 = m_Aes->Encrypt((const unsigned char *)m_InBuffer,m_OutBuffer,writechunk);
			if(ret1 != writechunk){
				printf("Error in AesRW Decrypt \n");
				return 0;
			}
			int ret = RWDecorator::Write(m_OutBuffer,writechunk,position);
			if(ret != writechunk){
				printf("Bytes Write to files were %d\n",ret);
				printf("Error in AesRW Write \n");
				return 0;
			}
			position += writechunk;
			byteswritten += writechunk;
			bytestobewritten -= bytestobewritten;
		}
	}
	return byteswritten;
}

