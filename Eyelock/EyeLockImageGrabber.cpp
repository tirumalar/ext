/*
 * EyeLockImageGrabber.cpp
 *
 *  Created on: 04-Oct-2010
 *      Author: mhadhv.shanbhag@mamigo.us
 */

#include "EyeLockImageGrabber.h"
#include "Aquisition.h"
#include "FrameGrabberFactory.h"
#include "string.h"
#include "logging.h"

const char logger[30] = "EyeLockImageGrabber";

EyeLockImageGrabber::EyeLockImageGrabber(Configuration* pConf, CircularAccess<SafeFrameMsg *>& outMsgQueue, SafeFrameMsg& detectedMsg, SafeFrameMsg& motionMsg)
:ImageProcessor(pConf,outMsgQueue,detectedMsg,motionMsg),m_Input(0),mWidth(0),mHeight(0),mSingleframe(false),m_FileImg(0){
	EyelockLog(logger, DEBUG, "EyeLockImageGrabber::EyeLockImageGrabber: basic constructor stuff"); fflush(stdout);

	m_LogFileName = "Eyelock.log";
#ifndef COMMENT
	if(m_pSrv)
		m_pSrv->SetLogFile(m_LogFileName);
	if(FrameGrabberFactory::getSensorType(pConf)== 0){
		mSingleframe = pConf->getValue("GRI.Singleframe",false);
	}
	aquisition->getDims(mWidth,mHeight);
	EyelockLog(logger, DEBUG, "Width %d  Height %d",mWidth,mHeight);
	if(mSingleframe){
		m_bufSize= (mWidth*mHeight)+2048;
		m_Input = cvCreateImage(cvSize(mWidth,mHeight),IPL_DEPTH_8U,1);
	}
#endif
}

EyeLockImageGrabber::~EyeLockImageGrabber() {
#ifndef COMMENT
	if(mSingleframe)
		cvReleaseImage(&m_Input);
	if(m_FileImg)
		cvReleaseImage(&m_FileImg);
#endif
}

IplImage* EyeLockImageGrabber::GetFrame(){
	IplImage* temp;
	if(mSingleframe){
		size_t retcnt = aquisition->getSingleFrame(m_Input->imageData,(mWidth*mHeight));
		temp = m_Input;
	}
	else
	{
		temp = ImageProcessor::GetFrame();
	}
	return temp;
}
