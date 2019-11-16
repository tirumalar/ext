/*
 * Message.h
 *
 *  Created on: 24 Nov, 2008
 *      Author: akhil
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_
///////////////////////////////////////////////////////////////////////////////
/// Message
class Message
{
	public:
		virtual ~Message(void){}

	public:
		virtual char* Wrap(int& Bytes)=0;
		virtual bool  Unwrap(char* package, int Bytes, int MsgNum)=0;
		virtual char* GetBuffer(void) const=0;
		virtual int   GetSize(void) const=0;
		/*
		 * follwing means I work with a fixed buffer:
		 * Can not use append
		 */
		virtual bool isFixed(){ return false;}
};

#endif /* MESSAGE_H_ */
