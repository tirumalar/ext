/*
 * Aquisition.h
 *
 *  Created on: 23-Oct-2009
 *      Author: mamigo
 */

#ifndef AQUISITION_H_
#define AQUISITION_H_


#include <opencv/cv.h>
#include "FrameGrabber.h"


class Aquisition{
public:
	Aquisition(Configuration *pConf,bool bSingleFrame=false);
	virtual ~Aquisition();
	virtual IplImage *getFrame();
	virtual IplImage *getFrame_nowait();
	virtual void getDims(int& width, int& height){framer->getDims(width,height);}
	size_t getSingleFrame(void * buff,size_t cnt);
	virtual void setLatestFrame_raw(char *ptr){}
	virtual FrameGrabber* getFrameGrabber(){return framer;}
	virtual void getIlluminatorState(__int64_t& ts,int &il0,int& il1){
		framer->getIlluminatorState(ts,il0,il1);
	}
	virtual void clearFrameBuffer(){};
	virtual cv::Rect getLatestScaledFaceRect(){
		return m_ScaledFaceRect;
	}

	virtual FaceImageQueue getLatestFaceInfo(){
		return m_FaceInfo;
	}
	bool bIrisToFaceMapDebug;
protected:
	Aquisition():temp_header(0){}
	FrameGrabber *framer;
	IplImage *temp_header;
	bool m_bSingleFrame;
	cv::Rect m_ScaledFaceRect;
	FaceImageQueue m_FaceInfo;

};

class AquisitionFile: public Aquisition{
public:
	AquisitionFile(Configuration *pConf, int binType);
	virtual IplImage *getFrame();
	virtual void getDims(int& width, int& height);
protected:
	int sleepPerFrameUSec;
	int binType;
};

class AquisitionBuffer: public Aquisition{
public:
	AquisitionBuffer(Configuration *pConf);
	virtual IplImage *getFrame();
	virtual void getDims(int& width, int& height);
	virtual void setLatestFrame_raw(char *ptr);
	virtual void clearFrameBuffer();
};

#endif /* AQUISITION_H_ */
