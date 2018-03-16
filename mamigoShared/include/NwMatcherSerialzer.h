/*
 * NwMatcherSerialzer.h
 *
 *  Created on: Mar 29, 2012
 *      Author: developer1
 */

#ifndef NWMATCHERSERIALZER_H_
#define NWMATCHERSERIALZER_H_

#include <cxtypes.h>
#include <string>

class IrisData;
class NwMatcherSerialzer {
public:
	NwMatcherSerialzer();
	virtual ~NwMatcherSerialzer();
	void Append(char *ptr,void *inp);
	int MakeNwMsg(char* dst,IrisData *data,char* p= "IRISDATA");
	bool ExtractNwMsg(IrisData* data,char* buff,char* p="IRISDATA");
	int GetSizeOfNwMsg(IrisData *data,char * ptr="IRISDATA");
	static int getBitStreamSize(const char* buff);
public:
	std::string m_msgType;
private:
	int writeInt(char *ptr,int* variable);
	int writeFloat(char *dst,float* var);
	int writeLong64(char *dst,uint64* var);


	int writeBuffer(char *dst,char *src,int size);
};

#endif /* NWMATCHERSERIALZER_H_ */
