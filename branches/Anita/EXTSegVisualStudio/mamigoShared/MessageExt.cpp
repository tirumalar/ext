/*
 * MessageExt.cpp
 * Various reusable extns of Message Class
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MessageExt.h"
#include <stdexcept>
#include <iostream>
#include <malloc.h>
#include <time.h>
using std::cout;
using std::endl;

extern "C"{
	#include "file_manip.h"
}
///////////////////////////////////////////////////////////////////////////////
/// TextMessage
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
TextMessage::TextMessage(unsigned short Bytes)
{
	Buffer = new char[Bytes];
	Available = Size = Bytes;
}

TextMessage::TextMessage(const char* Msg, unsigned short Len)
{
	if ( Len == 0 )
		Len = strlen(Msg)+1;
	Buffer = new char[Len];
	memcpy(Buffer, Msg, Len);
	Available = Size = Len;
}

TextMessage::~TextMessage(void)
{
	delete Buffer;
}

//-----------------------------------------------------------------------------
//--- Operator overload support
void TextMessage::SetString(const char* str, int len)
{
	if ( len <= 0 )
		len = strlen(str)+1;
	if ( len >= Available )
	{
		delete Buffer;
		Buffer = new char[len];
		Available = len;
	}
	Size = len;
	strcpy(Buffer, str);
}

void TextMessage::Append(const char* str, int len)
{
	if ( len <= 0 )
		len = strlen(str);
	if ( Size+len > Available )
	{
		char *B = new char[Size+len];
		strcpy(B, Buffer);
		delete Buffer;
		Buffer = B;
		Available = Size+len;
	}
	Size += len;
	strcat(Buffer, str);
}

//-----------------------------------------------------------------------------
//--- Specific Implementation
char *TextMessage::Wrap(int& Bytes)
{
	Bytes = Size;
	char *data = new char[Size];
	strcpy(data, Buffer);
	return data;
}

bool TextMessage::Unwrap(char *Data, int Bytes, int MsgNum)
{
	if ( MsgNum > 1 )
		Append(Data, Bytes-1);
	else
		SetString(Data, Bytes);
	return (Data[Bytes-1] == 0);
}


///////////////////////////////////////////////////////////////////////////////
/// BinMessage
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
BinMessage::BinMessage():Timestamp(0),Buffer(0),Available(0),Size(0)
{
}
BinMessage::BinMessage(unsigned int Bytes):Timestamp(0),Buffer(0)
{
	if(Bytes!= 0){
		Buffer = (char *)malloc(Bytes);
	}
	Available = Size = Bytes;
}

BinMessage::BinMessage(const char* Msg, unsigned int Len):Timestamp(0)
{
	if ( Len == 0 )
		throw "Msg size should be non-zero";
	Buffer = (char *)malloc(Len);
	memcpy(Buffer, Msg, Len);
	Available = Size = Len;
}

BinMessage::~BinMessage(void)
{
	if(Buffer)
		free(Buffer);
}



//-----------------------------------------------------------------------------
//--- Operator overload support
void BinMessage::SetData(const char* str, int len)
{
	if ( len <= 0 )
			throw "Msg size should be non-zero";
	if ( len > Available )
	{
		free(Buffer);
		Buffer = (char *)malloc(len);
		Available = len;
	}
	Size = len;
	memcpy(Buffer, str,Size);
}

void BinMessage::Append(const char* str, int len)
{
	if ( len <= 0 )
		throw "Msg size should be non-zero";
	if ( Size+len > Available )
	{
		Available = Size+len;
		char *B = (char *)malloc(Available);
		memcpy(B, Buffer,Size);
		if(Buffer)free(Buffer);
		Buffer = B;
	}
	memcpy(Buffer+Size,str,len);
	Size += len;
}

//-----------------------------------------------------------------------------
//--- Specific Implementation
char *BinMessage::Wrap(int& Bytes)
{
	Bytes = Size;
	char *data =(char *)malloc(Size);
	memcpy(data, Buffer,Size);
	return data;
}

bool BinMessage::Unwrap(char *Data, int Bytes, int MsgNum)
{
	if ( MsgNum > 1 )
		Append(Data, Bytes);
	else
		SetData(Data, Bytes);
	return IsDone();
}




bool RawMsg::IsDone(){
	return true;
};

////// FileMsg /////

FileMsg::FileMsg(const char *fName, int usleeptime, int debug):m_Fprt(0),m_bytesWritten(0),m_uSleeptime(0),m_debug(debug){
	strncpy(m_Filename,fName,255);
	m_uSleeptime = usleeptime;
}
FileMsg::~FileMsg()
{
	Cleaup();
}
void FileMsg::SetFileName(char* fname){
	strncpy(m_Filename,fname,255);
}

void FileMsg::Save(char *db, int size) {
	try {
		if (!m_Fprt) {
			// make sure file is opened and is writable
			m_Fprt = fopen(m_Filename, "wb");
			m_bytesWritten = 0;
		}
		if(!m_Fprt)	printf("File Path not available %s\n",m_Filename);

		int retval = fwrite(db, 1, size, m_Fprt);
		m_bytesWritten += retval;
		if(retval!=size)
			throw std::domain_error("could not write all bytes");
	}
	catch (std::exception& ex) {
		cout << "Exception while saving File: " << m_Filename << endl;
		Cleaup();
	}
}
void FileMsg::Cleaup(){
	if(m_Fprt)
		fclose(m_Fprt);
	m_Fprt=0;
	m_bytesWritten=0;
	if(m_fstream.is_open()){
		m_fstream.close();
	}
}
bool  FileMsg::Unwrap(char* package, int Bytes, int MsgNum)
{
	struct timeval timer;
	gettimeofday(&timer,0);
	TV_AS_USEC(timer,u);
	Save(package,Bytes);
	if(m_debug) printf("%llu Bytes Recd ( %d) %d of total %d \n",u,Bytes,m_bytesWritten,m_fileSize);
	usleep(m_uSleeptime);// 100 milli sec
	if(m_bytesWritten>=m_fileSize){
		printf("Bytes Recd %d %d \n",m_bytesWritten,m_fileSize);
		Cleaup();
		return true;
	}
	return false;
}

int FileMsg::Send(int size){
//	printf("m_bytesWritten %d\n",m_bytesWritten);
	try {
		int retval = 0;
		if (m_bytesWritten == 0) {
			if (!m_Fprt) {
				// make sure file is opened and is writable
				m_Fprt = fopen(m_Filename, "r");
				m_bytesWritten = 0;
			}
			if(!m_Fprt)	printf("File Path not available %s\n",m_Filename);

//			m_fstream.open (m_Filename, std::fstream::in);
//			if (!m_fstream.is_open()){
//				  printf("File Path not available %s\n",m_Filename);
//				  return 0;
//            }
			m_fileSize = FileSize(m_Filename);
			printf("m_fileSize %d\n",m_fileSize);
			retval = sprintf(m_Buffer,"DelayedLog;%08d;",m_fileSize);
			printf("Header Size %d \n",retval);
			//m_bytesWritten = initialoff;
			m_fileSize += retval;
		}
//		printf("m_bytesWritten < m_fileSize %d < %d\n",m_bytesWritten,m_fileSize);
		if(m_bytesWritten < m_fileSize){

			int bytesreqd =  m_fileSize - m_bytesWritten;
			if(bytesreqd > size)
				bytesreqd = size - 1024;
//			printf("bytes reqd %d\n",bytesreqd);
			do{
				int spaceavailable =  bytesreqd-retval;
				//m_fstream.getline(m_Buffer+retval,spaceavailable+1024);
				//retval += m_fstream.gcount();
				fread (m_Buffer+retval,1,spaceavailable,m_Fprt);
				retval +=spaceavailable;
//				printf("retval %d\n",retval);
			}while(retval< bytesreqd);
			m_bytesWritten += retval;
			size = retval;
//			printf("m_bytesWritten %d\n",m_bytesWritten);
		}else{
			size = 0;
			printf("Cleaning up\n");
			Cleaup();
		}
//		printf("size %d\n",size);
	}
	catch (std::exception& ex) {
		cout << "Exception while reading File: " << m_Filename << endl;
		Cleaup();
	}
	return size;
}

char* FileMsg::Wrap(int& Bytes){
	Bytes = Send(Bytes);
	usleep(m_uSleeptime);
//	printf("FileMsg:: Bytes %d\n",Bytes);
	return (char*)m_Buffer;
}

HDMatcherMsg::HDMatcherMsg():m_bytesWritten(0),m_RxSize(0){
	m_pBuffer = 0;
}

HDMatcherMsg::~HDMatcherMsg(){
	Cleaup();
}


void HDMatcherMsg::Cleaup(){
	m_bytesWritten=0;
	m_RxSize =0;
	m_pBuffer = 0;
}

bool HDMatcherMsg::Unwrap(char* ignore, int Bytes, int MsgNum){
	m_bytesWritten += Bytes;
	m_pBuffer+=Bytes;

	if(m_bytesWritten>=m_RxSize){
		//printf("Bytes Recd %d %d \n",m_bytesWritten,m_RxSize);
		Cleaup();
		return true;
	}
	return false;
}

FixedMsg::FixedMsg(const char *Msg, unsigned int Len)
{
	Buffer=(char *)Msg;
	Available=Size=Len;
}
void FixedMsg::SetData(const char* str, int len)
{
	Buffer=(char *)str;
	Available=Size=len;
}
