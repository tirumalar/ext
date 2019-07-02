/*
 * eyelock_com.h
 *
 *  Created on: Sep 6, 2017
 *      Author: root
 */

#ifndef EYELOCK_COM_H_
#define EYELOCK_COM_H_



#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

//DMO Queue Processing
#include "Synchronization.h"
#include "FaceMap.h"
#include <string.h>



#define PORT    50
#define MAXMSG  512
#define NO_EYES_THRESHOLD 10
//#define CENTER_POS 164
#define BRIGHTNESS_MAX 100
#define BRIGHTNESS_MELLOW 40
#define BRIGHTNESS_MIN 10


typedef struct _tagOIMQueueItem{
	char m_Message[MAXMSG];		// Message text... expand to MSG defines later with separate payloads...
	int m_frameIndex; 	// Unused for now
	unsigned short m_RandomSeed;
}OIMQueueItem;

typedef RingBuffer<OIMQueueItem> OIMQueue;

extern OIMQueue *g_pOIMQueue;

 #ifdef __cplusplus
     extern "C" {
 #endif
     void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
     void MoveTo(int v);
     char* GetTimeStamp();
     void SetFaceMode();
     void RecoverModeDrop();
     void AllocateOIMQueue(int nSize);
     void AllocateFaceQueue(int nSize);
     void deallocateOIMQueue();
 #ifdef __cplusplus
     }
 #endif

#define MATCHER_STAT_MATCH   1
#define MATCHER_STAT_DETECT  2
#define MATCHER_STAT_FAIL    3
#define MATCHER_STAT_UNKNOWN 4
extern int g_MatchState ;

#endif /* EYELOCK_COM_H_ */
