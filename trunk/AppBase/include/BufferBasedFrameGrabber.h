/*
 * BufferBasedFrameGrabber.h
 *
 *  Created on: Apr 7, 2011
 *      Author: developer1
 */

#ifndef BUFFERBASEDFRAMEGRABBER_H_
#define BUFFERBASEDFRAMEGRABBER_H_

#include "FrameGrabber.h"
#include "Synchronization.h"
#include <opencv/cv.h>

typedef struct ImageQueueItem{
	// unsigned char m_ptr[5038848];	// image size 1152000
	unsigned char *m_ptr;	// image size 1152000
	int m_ill0;
	unsigned int m_frameIndex;
	__int64_t m_startTime,m_endTime;
	int item_id;
	unsigned int m_ExtCameraIndex;
	unsigned int AuxLeftFrameIndex;
	unsigned int AuxRightFrameIndex;
	unsigned int MainLeftFrameIndex;
	unsigned int MainRightFrameIndex;
	cv::Rect ScaledFaceRect;
	FaceImageQueue m_FaceInfo;
};


typedef RingBuffer<ImageQueueItem> RingBufferImageQueue;
typedef RingBuffer<int> RingBufferQueueOffset;

class BufferBasedFrameGrabber: public FrameGrabber {
public:
	BufferBasedFrameGrabber();
	virtual ~BufferBasedFrameGrabber();
	virtual void init(Configuration *pCfg=0);
	virtual void term();
	virtual bool start(bool bStillFrames=false){return true;}
	virtual bool stop(){return true;}
	virtual bool isRunning(){ return true;}
	virtual void getDims(int& width, int& height) const;
	virtual char *getLatestFrame_raw_nowait();
	virtual char *getLatestFrame_raw();
	void setLatestFrame_raw(char *ptr);
	void clearFrameBuffer();
	ImageQueueItem GetFreeBuffer();
	void ReleaseProcessBuffer(ImageQueueItem m);
	bool TryReleaseProcessBuffer(ImageQueueItem m);
	void PushProcessBuffer(ImageQueueItem m);
private:
	int m_Width,m_Height,m_WidthStep;
	char *m_pImageBuffer;
	bool m_Debug;
	int m_ImageSize;
	int m_numbits;
	RingBufferImageQueue *m_pRingBuffer;
	RingBufferQueueOffset *m_RingBufferOffset;

	RingBufferImageQueue *m_FreeBuffer;
	RingBufferImageQueue *m_ProcessBuffer;

	ImageQueueItem m_ImageQueueItem;
	ImageQueueItem m_current_process_queue_item;
	bool bIrisToFaceMapDebug;
};

#endif /* BUFFERBASEDFRAMEGRABBER_H_ */
