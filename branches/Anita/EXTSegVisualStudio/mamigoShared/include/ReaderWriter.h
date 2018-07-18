/*
 * Reader.h
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#ifndef READERWRITER_H_
#define READERWRITER_H_

#include <stdio.h>

class ReaderWriter {
public:
	ReaderWriter(){}
	virtual ~ReaderWriter(){}
	virtual int Read(unsigned char *ptr,int numbyte, int starpos)=0;
	virtual int Write(unsigned char *ptr,int numbyte, int starpos)=0;
	virtual void Init(void *ptr){}

};

#endif /* READERWRITER_H_ */
