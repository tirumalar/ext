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

typedef struct ImageQueue{
	// unsigned char m_ptr[5038848];	// image size 1152000
	unsigned char *m_ptr;	// image size 1152000
	int m_ill0;
	int m_frameIndex;
	__int64_t m_startTime,m_endTime;
};


typedef RingBuffer<ImageQueue> RingBufferImageQueue;
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
	virtual char *getLatestFrame_raw();
	void setLatestFrame_raw(char *ptr);
	void clearFrameBuffer();
private:
	int m_Width,m_Height,m_WidthStep;
	char *m_pImageBuffer;
	bool m_Debug;
	int m_ImageSize;
	int m_numbits;
	RingBufferImageQueue *m_pRingBuffer;
	RingBufferQueueOffset *m_RingBufferOffset;
	ImageQueue m_ImageQueue;
};

#endif /* BUFFERBASEDFRAMEGRABBER_H_ */
