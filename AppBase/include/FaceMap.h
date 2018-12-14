/*
 * FaceMap.h
 *
 *  Created on: Dec 11, 2018
 *      Author: ext
 */

#ifndef FACEMAP_H_
#define FACEMAP_H_

#include "Synchronization.h"

typedef struct FaceImageQueue{
	bool FoundEyes;
	cv::Rect FaceCoord;
	char FaceFrameNo;
	__int64_t m_startTime;
};

typedef struct FaceMapping{
	cv::Rect IrisProj;
	bool bDoMapping;
};

typedef RingBuffer<FaceImageQueue> LeftCameraFaceQueue;
typedef RingBuffer<FaceImageQueue> RightCameraFaceQueue;
typedef RingBuffer<FaceImageQueue> FaceCameraQueue;

extern LeftCameraFaceQueue *g_pLeftCameraFaceQueue;
extern RightCameraFaceQueue *g_pRightCameraFaceQueue;
extern FaceCameraQueue *g_pCameraFaceQueue;

#endif /* FACEMAP_H_ */
