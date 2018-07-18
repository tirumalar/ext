/*
 * MemRW.h
 *
 *  Created on: 10-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef MEMRW_H_
#define MEMRW_H_

#include "ReaderWriter.h"
#include "malloc.h"
#include "string"

class MemRW: public ReaderWriter {
public:
	MemRW(char *baseAdd);
	virtual ~MemRW();
	virtual int Read(unsigned char *ptr,int numBytes,int startPos);
	virtual int Write(unsigned char *ptr,int numBytes,int startPos);
protected:
	const char *m_Buffer;

};


class FixedMemBuffer
{
public:
	FixedMemBuffer(const char* fname, int size){
		m_Buffer = (char*) malloc(sizeof(char)*size);
		m_CurrPos = m_Buffer;
		m_Size=0;
		if(size) m_Size = size;
		m_Fname = fname;
	}
	virtual ~FixedMemBuffer(){
		if(m_Buffer)
			free(m_Buffer);
	}
	bool IsAvailable(int sz){
		int current = (int)(m_CurrPos - m_Buffer);
		if((m_Size - current)> sz) return true;
		return false;
	}
	int GetAvailable(){
		int current = (int)(m_CurrPos - m_Buffer);
		return (m_Size - current);
	}

	bool Flush(){
		int current = (int)(m_CurrPos - m_Buffer);
		FILE *fp = fopen(m_Fname,"a");
		if(fp){
			fwrite(m_Buffer,current,1,fp);
			m_CurrPos=m_Buffer;
			fclose(fp);
			return true;
		}
		return false;
	}

	virtual bool Write(void *ptr)=0;

protected:
	char *m_CurrPos;
	char *m_Buffer;
	const char *m_Fname;
	int m_Size;

};


#endif /* MEMRW_H_ */
