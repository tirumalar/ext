/*
 * EyelockAPI.h
 *
 *  Created on: Apr 7, 2011
 *      Author: developer1
 */

#ifndef EYELOCKAPI_H_
#define EYELOCKAPI_H_
#include <sys/ioctl.h>
#include <iostream>
#include <vector>
#include <malloc.h>
#include <new>
#include "SafeFrameMsg.h"
#include "FileConfiguration.h"
#include "NwListener.h"
#include "CircularAccess.h"
#include "Safe.h"
#include "HTTPPOSTMsg.h"
#include "MatchProcessor.h"
#include "LEDDispatcher.h"
#include "NwDispatcher.h"
#include "F2FDispatcher.h"

#include "DBReceive.h"
#include "MessageExt.h"
#include "EyeLockThread.h"
#include "EyeDispatcher.h"

enum FunctionalType{ eImageProcessor, eMatchProcessor, eEyeLock};

#include <pthread.h>
#ifdef IRIS
class Iris;
#endif

class Mutex2
{
public:
	Mutex2()
	{
		pthread_mutex_init(&m_Lock, 0);
	}
	~Mutex2()
	{
		pthread_mutex_destroy(&m_Lock);
	}
	pthread_mutex_t & Get() { return m_Lock; }
protected:
	pthread_mutex_t m_Lock;
};

class EyelockAPI {
public:
	EyelockAPI(char* filename, FunctionalType pType=eEyeLock);
	virtual ~EyelockAPI();
	void Process(char *ptr,int *indx, float *score);
	int FindEyes(char *ptr);
	//assume my queue already has data
	int SaveBestPairOfEyes(const char *personName);
	int StoreBestPairOfEyes(const char *personName);

	void SaveEyes(int frIndx);
	void FlushAll(void);

	void GetLEDSpot(int *xy);
	int GetConfParam(const char *param);

	unsigned char *GetEnrollmentEye(int index);
	int GetIrisCode(unsigned char *imageBuffer, char *Iriscode, float * robustFeatureVariances=0);
	int DoMatch(float *score, char **personName);

	void EnableEyelidMask(int value);

	void SetDoSaveDetections(int save);

	MatchProcessor *pMatchProcessor;
	EyeLockImageGrabber *pImageProcessor;

	void CheckQueue(void);
	char *getBuffer(int i){ return (i==0)?mBuffer1:mBuffer2;}
	int AppendDB(char *ptr1,char* ptr2,char* fname, char* file);

	int DoMatch(unsigned char *inspCode, float *score, char **personName);

	void SetDoSaveDetections(bool save);

private:

	Mutex2 m_BioLock;
	FunctionalType mFunctType;
	CircularAccess<SafeFrameMsg *> outMsgQueue;
	SafeFrameMsg m_DetectedMsg;
	SafeFrameMsg m_MotionMsg;
	int m_outQSize;
	int m_Debug;
	ELEyeMsg m_bestEye;
	FileConfiguration conf;
	IplImage *m_Image;
	int m_SaveImg,m_ImgCntr;
	int m_cw, m_ch;
	float m_HDThreshold;
	float m_HDThresholdRaw;
	bool FindBest(int& pos);
	void SetUpdatedFalse(int pos);
	bool IsQueueFull(void);
	bool IsQueueEmpty(void);
	int CountEyes(void);
	char mBuffer1[640*480];
	char mBuffer2[640*480];
	char mBuffer3[640*480];

	int m_InWidth, m_InHeight, m_RoiX, m_RoiY;
	int m_Width, m_Height;
	unsigned char *m_ImageCrop;

	int m_LEDX, m_LEDY;

	pthread_mutex_t m_Lock;

	MatchResult m_MatchResult;

	int m_DoSaveDetections;
	bool m_DoEyelidMask;
	unsigned char * enrollmentEyes[3];
	char * enrollmentIrisCodes[2];
	bool m_HasEnrollment;
#ifdef IRIS
	std::pair< Iris *, Iris *> m_BestEnrollmentEyes;
	std::vector<Iris* > m_EnrollmentEyes;
#endif
};

class ConfAPI {
public:
	ConfAPI(char* filename);
	virtual ~ConfAPI();
	char* GetString(char* key);
	int GetInt(char* key);
	FileConfiguration conf;
};

void ResizeFrame(unsigned char *input, int w1, int h1, int stride1, unsigned char *output, int w2, int h2, int stride2, float ratio);

#endif /* EYELOCKAPI_H_ */
