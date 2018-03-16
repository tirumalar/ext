/*
 * SimpleMotionDetection.h
 *
 *  Created on: 05-Feb-2010
 *      Author: akhil
 */

#ifndef SIMPLEMOTIONDETECTION_H_
#define SIMPLEMOTIONDETECTION_H_
#include <cv.h>

#include "Configurable.h"
#include "CircularAccess.h"
extern "C"{
#include "file_manip.h"
}


#define MOTION_IMAGE_LEVELS 5
class SimpleMotionDetection: public Configurable {
public:
	SimpleMotionDetection(Configuration* pConf,int w,int h);
	virtual ~SimpleMotionDetection();
	bool Difference(IplImage *img1,IplImage *img2);
	bool ProcessImage(IplImage *img,timeval* pCurrtime);
#ifndef UNITTEST
protected:
#endif
	int m_AbsDiffThreshold;
	int m_History;
	int m_Level;
	float m_MinRatioMovingFramePixels;
	//int m_LastTimestamp;
	unsigned long m_KeyFrameTimeInterval;
	struct timeval m_LastTimestamp;
	int m_ImageQueueSize;
	bool m_Queuefull,m_Debug;
	IplImage *m_Image[MOTION_IMAGE_LEVELS];
	CircularAccess<IplImage *> m_ImageQueue;
};

#endif /* SIMPLEMOTIONDETECTION_H_ */
