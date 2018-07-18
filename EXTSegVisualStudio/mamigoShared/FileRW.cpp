/*
 * FileReader.cpp
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#include "FileRW.h"
#include "stdio.h"

FileRW::FileRW(char *fname,int off) {
	m_Filename = fname;
	m_StartOffset = off;
}

FileRW::~FileRW() {

}

int FileRW::Read(unsigned char *ptr,int numbytes, int startpos){
	FILE *fp = fopen(m_Filename,"rb");
	int retbytes = 0;
	if(fp == NULL){
		printf("Unable to Read File %s\n", m_Filename);
	}
	else
	{	retbytes = fseek(fp,startpos+m_StartOffset,SEEK_SET);
		if (retbytes != 0){
			fclose(fp);
			return (0);
		}
		retbytes = fread(ptr,1,numbytes,fp);
		if(retbytes!= numbytes){
			printf("Unable to get (%d) required bytes from the file %s\n",numbytes,m_Filename);
			printf("Got Bytes %d \n",retbytes);
			printf("Start(pos,off) = (%d,%d)\n",startpos,m_StartOffset);
		}
		fclose(fp);
	}
	return retbytes;
}

int FileRW::Write(unsigned char *ptr,int numbytes, int startpos){
	FILE *fp=0;
	int retbytes = 0;
	if(startpos == 0){
		fp = fopen(m_Filename,"wb");
		printf("%s \n",m_Filename);
	}else{
		fp = fopen(m_Filename,"a");
	}
	if(fp == NULL){
		printf("Unable to Write File %s\n", m_Filename);
	}
	retbytes = fwrite(ptr,1,numbytes,fp);
	fclose(fp);

	return retbytes;
}
