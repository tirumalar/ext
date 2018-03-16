/*
 * DetectedEye.h
 * A holder for the results of EyeDetection till they are handed off to network
 *  Created on: 22-Oct-2009
 *      Author: mamigo
 */

#ifndef DETECTEDEYE_H_
#define DETECTEDEYE_H_
#include <cv.h>
#include "CircularAccess.h"

class DetectedEye {
public:
	DetectedEye(int cwidth, int cheight,int numbits = 8):m_prev(-1),m_eyeCrop(0),m_eyeIndex(0),m_faceIndex(0),m_numEyes(0),m_eyeLeft(0),m_eyeTop(0),m_isUpdated(false),m_ill0(0),m_tobeSentAsPair(false),m_alreadySent(false)
	{
		m_eyeCrop=cvCreateImage(cvSize(cwidth,cheight),numbits,1);
		m_numbits = numbits;
	}
	virtual ~DetectedEye() {
		cvReleaseImage(&m_eyeCrop);
	}
	void init(int eyeIndex, int faceIndex, int numEyes, int eyeLeft, int eyeTop,CvPoint2D32f iris ,int ill0=0,int spoof = 0,float areaScore=0.0f, float focusScore=0.0f,float blacklevel=0.0,float halo=-1.0,bool tobeSentAsPair=false){
		m_eyeIndex=eyeIndex;
		m_faceIndex=faceIndex;
		m_numEyes=numEyes;
		m_eyeLeft=eyeLeft;
		m_eyeTop=eyeTop;
		m_score =-1;
		m_ill0 = ill0;
		m_spoofEye = spoof;
		m_areaScore = areaScore;
		m_focusScore = focusScore;
		m_blackLevel = blacklevel;
		m_tobeSentAsPair = tobeSentAsPair;
		m_irisCentroid.x = iris.x;
		m_irisCentroid.y = iris.y;
		m_halo = halo;
		m_timeStamp = 0;
		m_alreadySent = false;
	}
	void setAlreadySent(bool val){ m_alreadySent = val;}
	bool getSentStatus(){ return m_alreadySent;}
	void setSpoof(int val){m_spoofEye = val;}
	int getSpoof(){return m_spoofEye;}
	int getScore(){ return m_score;}
	void setScore(int score){ m_score = score;}
	bool isSame(int faceIndex,int eyeIndex){
		if(faceIndex!=m_faceIndex) return false;
		if(eyeIndex!=m_eyeIndex) return false;
		return true;
	}
	CvPoint2D32f getIrisCentroid(){return m_irisCentroid;}
	float getBlackLevel(){ return m_blackLevel;}
	void setBlackLevel(float bl){ m_blackLevel = bl;}
	float getAreaScore(){ return m_areaScore;}
	void setAreaScore(float sc){ m_areaScore = sc;}
	float getFocusScore(){ return m_focusScore;}
	float getHalo(){ return m_halo;}
	void setHalo(float halo){  m_halo = halo;}
	int getFaceIndex(){ return m_faceIndex;}
	int getEyeIndex(){ return m_eyeIndex;}
	int getNumEyes(){ return m_numEyes;}
	int getEyeLeft() { return m_eyeLeft;}
	int getEyeTop() { return m_eyeTop;}
	IplImage *getEyeCrop(){ return m_eyeCrop;}
	int getPrev(){return m_prev;}
	bool isUpdated(){return m_isUpdated;}
	void setUpdated(bool isUpdated){m_isUpdated=isUpdated;}
	void swapLeftTop()
	{
		int temp=m_eyeLeft;
		m_eyeLeft=m_eyeTop;
		m_eyeTop=temp;
	}
	void flipOnYAxis(int width, int height)
	{
		m_eyeLeft = width - (m_eyeLeft+m_eyeCrop->width);
	}
	void flipOnXAxis(int width, int height)
	{
		m_eyeTop = height - (m_eyeTop+m_eyeCrop->height);
	}
	int getIlluminatorStatus(){ return m_ill0;}
	void setPrev(int index){
		m_prev = index;
	}
	void setTimeStamp(uint64_t ts){ m_timeStamp = ts;}
	uint64_t getTimeStamp(){ return m_timeStamp;}
	void setNumBits(int bits){ m_numbits = bits;}
	int getNumBits(){ return m_numbits;}
protected:
	IplImage *m_eyeCrop;
	int m_eyeIndex;
	int m_faceIndex;
	int m_numEyes;
	int m_eyeLeft;
	int m_eyeTop;
	int m_score;
	bool m_isUpdated;
	int m_ill0;
	int m_spoofEye;
	float m_areaScore;
	float m_focusScore;
	float m_blackLevel;
	float m_halo;
	uint64_t m_timeStamp;
	CvPoint2D32f m_irisCentroid;
	bool m_tobeSentAsPair;
	int m_prev;
	bool m_alreadySent;
	int m_numbits;
};

typedef CircularAccess<DetectedEye *> DetectedEyesQueue;
typedef citerator<CircularAccess<DetectedEye *>, DetectedEye *> DetectedEyesIterator;

#endif /* DETECTEDEYE_H_ */
