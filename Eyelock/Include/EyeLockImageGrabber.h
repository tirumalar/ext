/*
 * EyeLockImageGrabber.h
 *
 *  Created on: 04-Oct-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef EYELOCKIMAGEGRABBER_H_
#define EYELOCKIMAGEGRABBER_H_

#include <ImageProcessor.h>

class EyeLockImageGrabber: public ImageProcessor {
public:
	EyeLockImageGrabber(Configuration* pConf, CircularAccess<SafeFrameMsg *>& outMsgQueue, SafeFrameMsg& detectedMsg, SafeFrameMsg& motionMsg);
	virtual ~EyeLockImageGrabber();
	virtual void InitializeMotionDetection(Configuration *& pConf, int w, int h){}
	virtual void ComputeMotion(void){}
	virtual bool SendHB(bool bSentSomething){ return true;}
	virtual IplImage* GetFrame();
	void GrabSave(int i=0);
	void setFrameType(int val){ ImageProcessor::setFrameType (val);}
	void setShouldDetectEyes(bool val){ ImageProcessor::setShouldDetectEyes(val);}
	int getFrameType() { return ImageProcessor::getFrameType();}
	bool getShouldDetectEyes() { return ImageProcessor::getShouldDetectEyes();}

	IplImage *m_Input,*m_FileImg;
	int mWidth,mHeight;
	bool mSingleframe;
};

#endif /* EYELOCKIMAGEGRABBER_H_ */
