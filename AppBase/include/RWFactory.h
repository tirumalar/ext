/*
 * RWFactory.h
 *
 *  Created on: 10-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef RWFACTORY_H_
#define RWFACTORY_H_
class ReaderWriter;

enum RW_TYPE{ FILE_RW,MEM_RW};

class RWFactory {
public:
	RWFactory();
	virtual ~RWFactory();
	ReaderWriter *Create(int matchtype,void* ptr,int off=0);

};

#endif /* RWFACTORY_H_ */
