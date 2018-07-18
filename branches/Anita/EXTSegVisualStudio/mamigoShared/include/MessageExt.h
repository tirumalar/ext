/*
 * MessageExt.h
 * Various reusable extns of Message Class
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef MESSAGEEXT_H_
#define MESSAGEEXT_H_

#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include "Message.h"
///////////////////////////////////////////////////////////////////////////////
/// TextMessage
class TextMessage: public Message
{
	private:
		char *Buffer;
		unsigned short Available;
		unsigned short Size;

	public:
		TextMessage(unsigned short Bytes);
		TextMessage(const char *Msg, unsigned short Len=0);
		virtual ~TextMessage(void);

	private:
		void SetString(const char* str, int len=-1);
		void SetString(const TextMessage& s)				{ SetString(s.Buffer, s.Size); }
		void Append(const char* str, int len=-1);
		void Append(const TextMessage& s)					{ Append(s.Buffer, s.Size-1); }

	public:
		TextMessage& operator =(const char* str)			{ SetString(str); return *this; }
		TextMessage& operator =(const TextMessage& s)		{ SetString(s); return *this; }
		TextMessage& operator +=(const char* str)			{ Append(str); return *this; }
		TextMessage& operator +=(const TextMessage& s)		{ Append(s); return *this; }

	public:
		virtual char* GetBuffer(void) const					{ return Buffer; }
		virtual char* Wrap(int& Bytes);
		virtual bool  Unwrap(char* package, int Bytes, int MsgNum);
		virtual int   GetSize(void) const					{ return Size; }
		virtual void  SetSize(int Bytes)					{ Size = Bytes; }
		virtual int   GetAvailable(void) const				{ return Available; }
};


class Copyable {
public:
	Copyable(){}
	virtual ~Copyable(){}
	Copyable& operator =(const Copyable& other) {CopyFrom(other); return *this;}
	virtual void CopyFrom(const Copyable& other)=0;
};


///////////////////////////////////////////////////////////////////////////////
/// BinMessage
class BinMessage: public Message, public Copyable
{
	protected:
		char *Buffer;
		unsigned int Available;
		unsigned int Size;
		uint64_t Timestamp;
		BinMessage();
	public:
		BinMessage(unsigned int Bytes);
		BinMessage(const char *Msg, unsigned int Len);
		virtual ~BinMessage(void);

	public:
		virtual void SetData(const char* str, int len);
		virtual void Append(const char* str, int len);

		virtual void CopyFrom(const Copyable& other)		{CopyFrom((const BinMessage&) other);}
		virtual void CopyFrom(const BinMessage& other)		{ SetData(other.Buffer,other.Size);
																Timestamp=other.Timestamp;}
		virtual char* GetBuffer(void) const					{ return Buffer; }
		virtual char* Wrap(int& Bytes);
		virtual bool  Unwrap(char* package, int Bytes, int MsgNum);
		virtual int   GetSize(void) const					{ return Size; }
		virtual void  SetSize(int Bytes)					{ Size = Bytes; }
		virtual int   GetAvailable(void) const				{ return Available; }
		virtual bool  IsDone()								{ return true;}
		void SetTime(uint64_t timestamp) { Timestamp = timestamp;}
		uint64_t GetTime() { return Timestamp;}
};
class RawMsg: public BinMessage{
public:
	RawMsg(unsigned int Bytes):BinMessage(Bytes){}
	RawMsg(const char *Msg, unsigned int Len):BinMessage(Msg,Len){}
	virtual bool  IsDone();
};

class FileMsg: public Message{
public:
	FileMsg(const char* fname,int usleeptime, int debug=0);
	~FileMsg();
	virtual bool  Unwrap(char* package, int Bytes, int MsgNum);
	void SetFileSize(int size){m_fileSize = size;}
	void SetFileName(char *fname);
	void Save(char *db,int size);
	int Send(int size);
	// stub out unused functions
	virtual char* Wrap(int& Bytes);
	virtual char* GetBuffer(void) const{ return 0;}
	virtual int   GetSize(void) const{ return 0;}
	void Cleaup();
protected:
	char m_Buffer[65536];
	int m_fileSize;
	int m_bytesWritten;
	FILE *m_Fprt;
	std::fstream m_fstream;
	char m_Filename[256];
	int m_uSleeptime;
	bool m_debug;
};
/*
 * This variant only uses externally supplied buffer
 */
class FixedMsg: public BinMessage{
public:
	FixedMsg(const char *Msg=0, unsigned int Len=0);
	virtual void SetData(const char* str, int len);
	virtual void Append(const char* str, int len){}
	virtual bool isFixed(){ return true;}


};
class HDMatcherMsg: public Message{
public:
	HDMatcherMsg();
	~HDMatcherMsg();
	virtual bool  Unwrap(char* ignore, int Bytes, int MsgNum);
	virtual bool isFixed(){ return true;}
	void SetFileSize(int size){m_RxSize = size;}
	void SetWrittenSize(int size){m_bytesWritten = size;}
	void SetBuffer(char* buff) { m_pBuffer= buff;}
	virtual char* GetBuffer(void) const{ return m_pBuffer;}
	virtual char* Wrap(int& Bytes) { return 0;}
	virtual int GetSize(void) const{ return m_RxSize-m_bytesWritten;}
protected:
	void Cleaup();
	char *m_pBuffer;
	int m_bytesWritten;
	int m_RxSize;
};
#endif /* MESSAGEEXT_H_ */
