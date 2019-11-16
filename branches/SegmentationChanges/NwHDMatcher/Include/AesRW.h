/*
 * AESReader.h
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#ifndef AESRW_H_
#define AESRW_H_

#include "RWDecorator.h"
class AES;
class IrisDBHeader;

class AesRW: public RWDecorator {
public:
	AesRW(ReaderWriter *reader);
	virtual ~AesRW();
	virtual int Read(unsigned char*inp,int numbytes,int position);
	virtual int Write(unsigned char*inp,int numbytes,int position);
	void SetKeyAndIV(const unsigned char *key,const unsigned char *iv);
	void Init(void *ptr);
private:

	unsigned char *m_InBuffer;
	int m_BufferSize;
	unsigned char *m_OutBuffer;
	AES *m_Aes;
	IrisDBHeader *m_DBHeader;
	unsigned char m_IV[32];
	unsigned int m_EncSize;
	unsigned int m_NumEyes;
};

#endif /* AESRW_H_ */
