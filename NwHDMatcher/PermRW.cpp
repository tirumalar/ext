/*
 * DatabaseRW.cpp
 *
 *  Created on: 08-May-2010
 *      Author: madhav.shanbhag@mamigo.us
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PermRW.h"
#include "IrisDBHeader.h"
#include "PermuteServer.h"

PermRW::PermRW(ReaderWriter *reader):RWDecorator(reader),m_Buffer(0),m_PermServer(0){
	m_Buffer = 0;
}

PermRW::~PermRW() {
	if(m_Buffer)
		free(m_Buffer);
	if(m_PermServer)
		delete m_PermServer;
}

void PermRW::Init(void *ptr){
	m_DBHeader = (IrisDBHeader *)ptr;
	m_Buffer = (unsigned char*)malloc(m_DBHeader->GetOneRecSizeinDBFile());//to be safe
	RWDecorator::Init(ptr);
	m_PermServer = new PermuteServer(m_DBHeader->GetIrisSize(), m_DBHeader->GetPermutationKey());

}

int PermRW::Read(unsigned char*inp,int numbytes,int position){
	int ret = RWDecorator::Read(m_Buffer,numbytes,position);
	if(m_PermServer->GetPermutationKey() == 0){
		memcpy(inp,m_Buffer,m_DBHeader->GetOneRecSizeinDB());
		return ret;
	}

	int irissz = m_DBHeader->GetIrisSize();

	memcpy(inp,m_Buffer,4);
	unsigned char *in,*out;
	in = m_Buffer+irissz*4+4;
	out = inp+irissz*4+4;

	m_PermServer->Recover(m_Buffer+4, inp+4, inp+4+irissz);
	m_PermServer->Recover(m_Buffer+4+irissz*2, inp+4+irissz*2, inp+4+irissz*3);

	memcpy(inp+irissz*4+4,m_Buffer+4+irissz*4,m_DBHeader->GetOneRecSizeinDB()-(irissz*4+4));
	return ret;
}

int PermRW::Write(unsigned char*inp,int numbytes,int position){

}

