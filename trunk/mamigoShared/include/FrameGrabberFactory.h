/*
 * FrameGrabberFactory.h
 *
 *  Created on: 19 Dec, 2008
 *      Author: akhil
 */

#ifndef FRAMEGRABBERFACTORY_H_
#define FRAMEGRABBERFACTORY_H_

#include "Configurable.h"
#include "FrameGrabber.h"

class FrameGrabberFactory : public Configurable{
public:
	FrameGrabberFactory(Configuration *pConf);
	virtual ~FrameGrabberFactory(){}
	FrameGrabber* create();
	static short getSensorType(Configuration *& pConf);
protected:
	short sensorType;
	const char *testFileNamePattern;
	int testFileCount;
	int binType;
	int m_numberofBits;
};

#endif /* FRAMEGRABBERFACTORY_H_ */
