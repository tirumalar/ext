/*
 * ReaderDecorator.cpp
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#include "RWDecorator.h"

RWDecorator::RWDecorator(ReaderWriter *r) {
	m_ReaderWriter = r;
}

RWDecorator::~RWDecorator() {
	delete m_ReaderWriter;
}

int RWDecorator::Read(unsigned char *ptr,int numbytes,int startpos){
	return m_ReaderWriter->Read(ptr,numbytes,startpos);
}

int RWDecorator::Write(unsigned char *ptr,int numbytes,int startpos){
	return m_ReaderWriter->Write(ptr,numbytes,startpos);
}
void RWDecorator::Init(void *ptr){
	return m_ReaderWriter->Init(ptr);
}

