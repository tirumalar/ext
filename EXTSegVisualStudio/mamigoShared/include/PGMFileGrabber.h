/*
 * PGMFileGrabber.h
 *
 *  Created on: 17 Mar, 2009
 *      Author: akhil
 */

#ifndef PGMFILEGRABBER_H_
#define PGMFILEGRABBER_H_

#include "FileBasedFrameGrabber.h"

class PGMFileGrabber: public FileBasedFrameGrabber {
public:
	PGMFileGrabber(const char* frameFilenameFormat, int maxFrames):FileBasedFrameGrabber(frameFilenameFormat,maxFrames){}
	virtual ~PGMFileGrabber(){}
	virtual char *getLatestFrame_raw();
};

#endif /* PGMFILEGRABBER_H_ */
