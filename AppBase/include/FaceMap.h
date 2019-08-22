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
	bool FoundFace;
	cv::Rect ScaledFaceCoord;
	unsigned char FaceFrameNo;
	bool projPtr;
	int FaceWidth;
	unsigned char *faceImagePtr;	// image size 1152000
};

typedef struct FaceMapping{
	cv::Rect IrisProj;
	cv::Rect SFaceCoord;
	bool bDoMapping;
};

typedef RingBuffer<FaceImageQueue> LeftCameraFaceQueue;
typedef RingBuffer<FaceImageQueue> RightCameraFaceQueue;
typedef RingBuffer<FaceImageQueue> FaceCameraQueue;

extern LeftCameraFaceQueue *g_pLeftCameraFaceQueue;
extern RightCameraFaceQueue *g_pRightCameraFaceQueue;
extern FaceCameraQueue *g_pCameraFaceQueue;

#endif /* FACEMAP_H_ */
