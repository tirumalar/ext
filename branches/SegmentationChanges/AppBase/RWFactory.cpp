/*
 * RWFactory.cpp
 *
 *  Created on: 10-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "RWFactory.h"
#include "ReaderWriter.h"
#include "FileRW.h"
#include "MemRW.h"

RWFactory::RWFactory() {
}

RWFactory::~RWFactory() {
}

ReaderWriter* RWFactory::Create(int rwtype,void* ptr,int offset)
{
	if (FILE_RW == rwtype){
		ReaderWriter *in = new FileRW((char*)ptr,offset);
		return in;
	}
	else if(MEM_RW == rwtype ){
		ReaderWriter* in = new MemRW((char*)ptr);
		return in;
	}
	else
	{
		throw "Input is incorrect w.r.t FILE_RW/MEM_RW";
	}
}


