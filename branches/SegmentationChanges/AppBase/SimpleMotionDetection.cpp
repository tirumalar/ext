/*
 * SimpleMotionDetection.cpp
 *
 *  Created on: 05-Feb-2010
 *      Author: akhil
 */

#include "SimpleMotionDetection.h"


SimpleMotionDetection::SimpleMotionDetection(Configuration* pConf,int w, int h):
	m_Queuefull(0) {
	// TODO Auto-generated constructor stub
	m_MinRatioMovingFramePixels = pConf->getValue("GRIMotion.MinRatioMovingFramePixels",0.1f);
	m_AbsDiffThreshold = pConf->getValue("GRIMotion.AbsDiffThreshold",50);
	m_KeyFrameTimeInterval = pConf->getValue("GRIMotion.KeyFrameTimeIntervalMilliSec",0)*1000;
	m_Level = pConf->getValue("GRIMotion.Level",4);
	m_Debug = pConf->getValue("GRIMotion.Debug",false);

	int level = 3;
	int width = w>>level;
	int height = h>>level;

	for(int i=0;i<MOTION_IMAGE_LEVELS;i++) // DJH: replaced 10 with MOTION_IMAGE_LEVELS to fix array bounds issue
		m_Image[i] =  0;

	if(m_Debug)	printf("[L W H] [%d %d %d]\n",3,width,height);

	// Find the number of level which we need to pyramid down between
	width = (int)floor(width/2)+1;
	height = (int)floor(height/2)+1;

	for(int i=4;i<m_Level;i++){
		m_Image[i-4] = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
		cvSetZero(m_Image[i-4]);
		if(m_Debug)	printf("[L W H] [%d %d %d]\n",i,width,height);
		width = (int)floor(width/2)+1;
		height = (int)floor(height/2)+1;
	}
	// Get the history to check and depth of buffer
	m_History = pConf->getValue("GRIMotion.History",10);
	m_ImageQueueSize = m_History;//pConf->getValue("GRIMotion.QSize",20);
	m_ImageQueue(m_ImageQueueSize);
	if(m_Level <= 3){
		width = w>>m_Level;
		height = h>>m_Level;
	}
	if(m_Debug)	printf("Motion Buffer [W H] [%d %d]\n",width,height);

	for (int i = 0; i < m_ImageQueueSize; i++) {
		m_ImageQueue[i] = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
		cvSetZero(m_ImageQueue[i]);
	}
//	m_MinRatioMovingFramePixels *= width*height;
	gettimeofday(&m_LastTimestamp,0);
}

SimpleMotionDetection::~SimpleMotionDetection() {
	// TODO Auto-generated destructor stub
	for (int i = 0; i < m_ImageQueueSize; i++){
		cvReleaseImage(&m_ImageQueue[i]);
	}
	for(int i=4;i<m_Level;i++){
		cvReleaseImage(&m_Image[i-4]);
	}
}

bool SimpleMotionDetection::Difference(IplImage *img1,IplImage *img2){
	assert(img1->width == img2->width ||
			img1->widthStep == img2->widthStep ||
			img1->height == img2->height);
	bool ret = false;
//	char buff[] = "/mnt/mmc/Image%03d.pgm";
//	char name[100];
//	static int i=0;
	int width = img1->width;
	int height = img1->height-1;
	int step = img1->widthStep;
	int sum = 0;
	for(int i =0;i< height;i++){
		unsigned char *in1 = (unsigned char*)img1->imageData + i*step;
		unsigned char *in2 = (unsigned char*)img2->imageData + i*step;
		for(int j=0;j<width;j++){
			int a = *in1++ - *in2++;
			a = a>0?a:(-a);
			sum+=a>m_AbsDiffThreshold?1:0;
		}
	}
	if(m_Debug){
		printf("Th = %d,Sum > Ratio %d %f\n",m_AbsDiffThreshold,sum,m_MinRatioMovingFramePixels*width*height);
/*		sprintf(name,buff,i++);
		cvSaveImage(name,img1);
		sprintf(name,buff,i++);
		cvSaveImage(name,img2);
*/	}
	if((sum*1.0) > m_MinRatioMovingFramePixels*width*height) ret = true;
	return ret;
}

bool SimpleMotionDetection::ProcessImage(IplImage *img,timeval* pCurrtime)
{
	bool ret = false;
	IplImage* ptr = img;
	unsigned long diff = tvdelta(pCurrtime,&m_LastTimestamp);
	if(m_Debug)	printf("Diff %li %li \n",diff,m_KeyFrameTimeInterval);
	if(diff >= m_KeyFrameTimeInterval){
		m_LastTimestamp = *pCurrtime;

		for(int i=4;i<m_Level;i++){
			cvPyrDown(ptr,m_Image[i-4]);
			ptr = m_Image[i-4];
		}

		if(m_Level >=4 ){
			cvPyrDown(ptr,m_ImageQueue.getCurr());
		}

		if(m_Level <= 3){
			cvCopy(ptr,m_ImageQueue.getCurr());
		}

		//cvSaveImage("/mnt/mmc/Imageinp.pgm",img);
		//cvSaveImage("/mnt/mmc/Imageout.pgm",m_ImageQueue.getCurr());

		if(m_Debug)	printf("Queue number  %d %d\n",m_Queuefull,m_ImageQueue.curPos());

		m_ImageQueue++;
		//Try to push the image
		if(m_Queuefull == 1){
			for(int j=1;j<(m_History);j++){
				bool ret = Difference(m_ImageQueue.getPrev(),m_ImageQueue.getPrev(j));
				//if(m_Debug)	printf("First val of %d prev  = %#x\n",m_History-j,*((unsigned int *)m_ImageQueue.getPrev(m_History-j)->imageData));
				if(ret){
					if(m_Debug)	printf("send the MOTION Msg\n");
					return ret;
				}
			}
		}else{// Queue is not full lets push the image in queue
			if(m_ImageQueue.curPos() == (m_ImageQueueSize-1) ) m_Queuefull++;
		}
	}
	return ret;
}


