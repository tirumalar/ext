/*
 * MemRW.cpp
 *
 *  Created on: 10-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "MemRW.h"
#include <string.h>

MemRW::MemRW(char* base) {
	m_Buffer = base;
}

MemRW::~MemRW() {
}

int MemRW::Read(unsigned char *ptr,int numbytes, int startpos){
	if(m_Buffer){
		memcpy(ptr,m_Buffer+startpos,numbytes);
	}else{
		numbytes=0;
	}
	return numbytes;
}

int MemRW::Write(unsigned char *ptr,int numbytes, int startpos){
	if(m_Buffer){
		memcpy((void *)(m_Buffer+startpos),(const void *)ptr,numbytes);
	}else{
		numbytes=0;
	}
	return numbytes;
}
