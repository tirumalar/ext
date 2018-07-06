/*
 * FileBasedFrameGrabber.h
 *
 *  Created on: 19 Dec, 2008
 *      Author: akhil
 */

#ifndef FILEBASEDFRAMEGRABBER_H_
#define FILEBASEDFRAMEGRABBER_H_

#include <cxcore.h>
#include "FrameGrabber.h"

class FileBasedFrameGrabber : public FrameGrabber{
public:
	FileBasedFrameGrabber(const char* frameFilenameFormat, int maxFrames);
	virtual ~FileBasedFrameGrabber(){}
	virtual void init(Configuration *pCfg=0);
	virtual void term();
	virtual bool start(bool bStillFrames=false){return true;}
	virtual bool stop(){return true;}
	virtual bool isRunning(){ return true;}
	virtual void getDims(int& width, int& height) const;
	virtual char *getLatestFrame_raw();
protected:
	const char* _frameFileNameFmt;
	int _maxFrames;
	int _nextFrameIdx;
	char _fileName[100];
	char	*getFileNameFor(int index);
	inline char *getCurFileName() {return getFileNameFor(_nextFrameIdx);}
	IplImage* _curImage;
	int m_sleeptimems;
};

#endif /* FILEBASEDFRAMEGRABBER_H_ */
