/*
 * DatabaseRW.h
 *
 *  Created on: 08-May-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef DATABASERW_H_
#define DATABASERW_H_

#include "RWDecorator.h"
class IrisDBHeader;
class PermuteServer;
class PermRW: public RWDecorator {
public:
	PermRW(ReaderWriter *);
	virtual ~PermRW();
	virtual int Read(unsigned char*inp,int numbytes,int position);
	virtual int Write(unsigned char*inp,int numbytes,int position);
	virtual void Init(void *ptr);
private:
	unsigned char* m_Buffer;
	IrisDBHeader *m_DBHeader;
	PermuteServer *m_PermServer;

};

#endif /* DATABASERW_H_ */
