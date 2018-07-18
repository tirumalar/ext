/*
 * ReaderDecorator.h
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */

#ifndef RWDECORATOR_H_
#define RWDECORATOR_H_

#include "ReaderWriter.h"

class RWDecorator: public ReaderWriter {
public:
	RWDecorator(ReaderWriter *);
	virtual ~RWDecorator();
	virtual int Read(unsigned char *ptr,int numbytes,int starpos);
	virtual int Write(unsigned char *ptr,int numbyte, int starpos);
	virtual void Init(void *ptr);

private:
	ReaderWriter *m_ReaderWriter;
};

#endif /* RWDECORATOR_H_ */
