/*
 * FileReader.h
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#ifndef FILEREADER_H_
#define FILEREADER_H_
#include "ReaderWriter.h"

class FileRW: public ReaderWriter {
public:
	FileRW(char *fname,int off=0);
	virtual ~FileRW();
	virtual int Read(unsigned char *ptr,int numBytes,int startPos);
	virtual int Write(unsigned char *ptr,int numBytes,int startPos);
protected:
	const char *m_Filename;
	int m_StartOffset;

};

#endif /* FILEREADER_H_ */
