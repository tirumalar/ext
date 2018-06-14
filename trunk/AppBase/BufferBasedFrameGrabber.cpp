/*
 * BufferBasedFrameGrabber.cpp
 *
 *  Created on: Apr 7, 2011
 *      Author: developer1
 */
#include <stdio.h>
#include <unistd.h>
#include "BufferBasedFrameGrabber.h"
#include "Configurable.h"
#include "logging.h"
extern "C"{
#include "file_manip.h"
}

#define FREE_BUFF_SIZE 5

const char logger[30] = "BufferBasedFrameGrabber";

BufferBasedFrameGrabber::BufferBasedFrameGrabber():m_Width(0),m_Height(0),m_WidthStep(0),m_pImageBuffer(0) {
	m_ImageSize = 0;
	m_Debug = false; //true;
}

BufferBasedFrameGrabber::~BufferBasedFrameGrabber() {
	ImageQueueItem qi;
	while (!m_FreeBuffer->Empty())
	{
		if (m_FreeBuffer->TryPop(qi))
		{
			delete[] qi.m_ptr;
		}
	}
}

void BufferBasedFrameGrabber::ReleaseProcessBuffer(ImageQueueItem m)
{
	m_FreeBuffer->Push(m);
}

ImageQueueItem BufferBasedFrameGrabber::GetFreeBuffer()
{
	ImageQueueItem qi;
	if (m_FreeBuffer->TryPop(qi))
		return qi;
	qi.m_ptr=0;
//	printf("Free buffer returns %d\n",qi.item_id);
	return qi;
}

void BufferBasedFrameGrabber::PushProcessBuffer (ImageQueueItem m)
{
//	printf("pushing image buffer %d\n",m.item_id);
	static int cntr=0;


	if(cntr & 0x1){
		m.m_ill0 = 1;
	}else{
		m.m_ill0 = 0;
	}
	m.m_frameIndex = cntr;
	cntr++;

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);
	m.m_startTime = starttimestamp;
	m.m_endTime = m.m_startTime;

	m_ProcessBuffer->Push(m);
}
void BufferBasedFrameGrabber::init(Configuration *pConf){
	if (m_Debug)
		EyelockLog(logger, DEBUG, "BufferBasedFrameGrabber::init() Start");
// Read from FILE and update Width and Height
	m_Width=pConf->getValue("FrameSize.width",1200); //1984);
	m_Height=pConf->getValue("FrameSize.height",960);	//1392);
	m_WidthStep=pConf->getValue("FrameSize.widthstep",0);
	imagebits=pConf->getValue("FrameSize.bits",8);
	int buff_size;
	buff_size = pConf->getValue("BuffFrameGrab.size",2);
	printf("Queue size = %d\n",buff_size);

	m_ImageSize = m_Width * m_Height;
	//m_ImageSize = 2 * m_Width * m_Height;
	if(m_Debug)
	{
		//  W H S = 1984 1392 5523456
		EyelockLog(logger, DEBUG, "W H S = %d %d %d ",m_Width,m_Height,m_ImageSize);
	}
	m_pImageBuffer = new char[m_ImageSize];
	m_ImageQueueItem.m_ptr = new unsigned char[m_ImageSize];

	int numbits = pConf->getValue("Eyelock.NumBits", 8);
	m_numbits = numbits > 8 ? 16 : numbits;
	SetImageBits(m_numbits);
#ifndef HBOX_PG
	m_pRingBuffer = new RingBufferImageQueue(buff_size); /* Allocate ring buffer to be frame buffer - 1 */
	m_RingBufferOffset = new RingBufferQueueOffset(buff_size);
#else
	m_pRingBuffer = new RingBufferImageQueue(5); /* Allocate ring buffer to be frame buffer - 1 */
	m_RingBufferOffset = new RingBufferQueueOffset(5);
#endif	
	m_ill0 = 0;
	m_frameIndex = 0;
	m_ts = 0;

	m_FreeBuffer = new RingBufferImageQueue(FREE_BUFF_SIZE);
	m_ProcessBuffer = new RingBufferImageQueue(FREE_BUFF_SIZE);
	for (int x = 0 ; x< FREE_BUFF_SIZE; x++)
	{
		m_ImageQueueItem.m_ptr = new unsigned char[m_ImageSize];
		m_ImageQueueItem.item_id=x;
		m_FreeBuffer->Push(m_ImageQueueItem);

	}
	m_current_process_queue_item.m_ptr=0;
}

void BufferBasedFrameGrabber::getDims(int & width, int & height) const
{
	width = m_Width;
	height = m_Height;
}

#if 0
char *BufferBasedFrameGrabber::getLatestFrame_raw(){
	//if (m_Debug)
		//EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() Start");

	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;

	ImageQueue val;
	while(m_pRingBuffer->TryPop(val))
	{
#if 0
		if (m_Debug)
			EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() get image queue");
#endif
		unsigned char *ptr = val.m_ptr;
		m_ill0 = val.m_ill0;
		m_frameIndex = val.m_frameIndex;
		m_ts = val.m_endTime;
		memcpy(m_pImageBuffer, ptr, length);
	}

#if 0
	if (m_Debug)
		EyelockLog(logger, DEBUG, "get image queue m_ill0 %d, m_frameIndex %d, time %llu", m_ill0,m_frameIndex, m_ts);
#endif
	return m_pImageBuffer;
}
#else
char *BufferBasedFrameGrabber::getLatestFrame_raw(){
	//if (m_Debug)
		//EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() Start");

	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;


	// release the previously processed buffer it it is real;
    if (m_current_process_queue_item.m_ptr!=0)
    	ReleaseProcessBuffer(m_current_process_queue_item);

    bool status = false;

	while(1)
		{
		status=m_ProcessBuffer->TryPop(m_current_process_queue_item);
		if (status)
			break;
		usleep(1000);
		}
	m_ill0 = m_current_process_queue_item.m_ill0;
	m_frameIndex = m_current_process_queue_item.m_frameIndex;
	m_ts = m_current_process_queue_item.m_endTime;

	EyelockLog(logger, TRACE, "get image queue m_ill0 %d, m_frameIndex %d, time %llu", m_ill0,m_frameIndex, m_ts);

#if 0
	char filename[100];
	static int i;
	sprintf(filename,"image_%d.bin",i++);
	FILE *f = fopen(filename, "wb");
	fwrite(m_current_process_queue_item.m_ptr, length, 1, f);
	fclose(f);

#endif
	return m_current_process_queue_item.m_ptr;

#if 0
	// ImageQueue val;
	//while(!ShouldIQuit()&&!m_pRingBuffer->TryPop(val))

	bool status = false;

	while(1)
		{
		status=m_pRingBuffer->TryPop(m_ImageQueue);
		if (status)
			break;
		usleep(1000);
		}

	if (status == true) {
#if 0
		if (m_Debug)
			EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() get image queue");
#endif			
		// unsigned char *ptr = m_ImageQueue.m_ptr;
		m_ill0 = m_ImageQueue.m_ill0;
		m_frameIndex = m_ImageQueue.m_frameIndex;
		m_ts = m_ImageQueue.m_endTime;
		memcpy(m_pImageBuffer, m_ImageQueue.m_ptr, length);
		int static cntr=0;
		cntr++;
//		printf("popped so far .. %d\n",cntr);
	}
	else {
		memset(m_pImageBuffer, 0, length);
	}
#if 0
	if (m_Debug)
		EyelockLog(logger, DEBUG, "get image queue m_ill0 %d, m_frameIndex %d, time %llu", m_ill0,m_frameIndex, m_ts);
#endif		
	return m_pImageBuffer;

#endif
}
#endif

void BufferBasedFrameGrabber::setLatestFrame_raw(char *ptr){
#if 0
	if (m_Debug)
		EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::setLatestFrame_raw() Start");
#endif

	if (ptr == NULL)
		return;
#if 0
	char filename[100];
		static int i;
		sprintf(filename,"bbSF_%d.bin",i++);
		FILE *f = fopen(filename, "wb");
		fwrite(ptr, (m_Width * m_Height), 1, f);
#endif
	// ImageQueue val;
	static int cntr=0;
	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;

	if(cntr & 0x1){
		m_ImageQueueItem.m_ill0 = 1;
	}else{
		m_ImageQueueItem.m_ill0 = 0;
	}
	m_ImageQueueItem.m_frameIndex = cntr;
	cntr++;

	//unsigned char *pdata = m_ImageQueue.m_ptr;
	// memcpy(pdata, ptr, length);
	memcpy(m_ImageQueueItem.m_ptr, ptr, length);

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);
	m_ImageQueueItem.m_startTime = starttimestamp;
	m_ImageQueueItem.m_endTime = m_ImageQueueItem.m_startTime;

	static int frame_drop = 0;
	if (m_pRingBuffer->TryPush(m_ImageQueueItem)==false)
	{

		frame_drop++;
		//printf("Dropping Frames total %d\n",frame_drop);
	}
	int static cntr1=0;
	cntr1++;
	//printf("pushed so far .. %d and dropped %d\n",cntr1,frame_drop);

	if (m_Debug)
		EyelockLog(logger, DEBUG, "setLatestFrame_raw() Push to ring buffer");
}
void BufferBasedFrameGrabber::term()
{
	if(m_pImageBuffer)
		delete [] m_pImageBuffer;
	if (m_RingBufferOffset)
		delete m_RingBufferOffset;
	if (m_pRingBuffer)
		delete m_pRingBuffer;
	if(m_ImageQueueItem.m_ptr)
			delete [] m_ImageQueueItem.m_ptr;

	//TODO: destrutor for all allocated m_ImageQueueItem
}

void BufferBasedFrameGrabber::clearFrameBuffer(){
	if (m_Debug)
		EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::ClearRing() Start");

	m_pRingBuffer->Clear();
}
