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

	//DMOLEAK
	if (0 != m_pImageBuffer)
		delete [] m_pImageBuffer;

	if (0 != m_pRingBuffer)
		delete m_pRingBuffer;

	if (0 != m_RingBufferOffset)
		delete m_RingBufferOffset;

	if (0 != m_FreeBuffer)
		delete m_FreeBuffer;

	if (0 != m_ProcessBuffer)
		delete m_ProcessBuffer;

}

void BufferBasedFrameGrabber::ReleaseProcessBuffer(ImageQueueItem m)
{
	// printf("\n\n BEFORE PUSH \n\n");
	m_FreeBuffer->Push(m);
	// printf("\n\n After PUSH \n\n");

}


bool BufferBasedFrameGrabber::TryReleaseProcessBuffer(ImageQueueItem m)
{
	return m_FreeBuffer->TryPush(m);

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
	static unsigned int cntr=0;
	static unsigned int AuxLeft=0;
	static unsigned int AuxRight=0;
	static unsigned int MainLeft=0;
	static unsigned int MainRight=0;

	m_ImageQueueItem.m_ExtCameraIndex = m.m_ptr[2]&0xff;

	// FrameIndex
	if(m_ImageQueueItem.m_ExtCameraIndex == 129){
		if(AuxLeft != 0)
			m_ImageQueueItem.AuxLeftFrameIndex = (255 * AuxLeft) + (m.m_ptr[3]&0xff);
		else
			m_ImageQueueItem.AuxLeftFrameIndex  =  m.m_ptr[3]&0xff;

		m.m_frameIndex = m_ImageQueueItem.AuxLeftFrameIndex;

	}
	if(m_ImageQueueItem.m_ExtCameraIndex == 130){
		if(AuxRight != 0)
			m_ImageQueueItem.AuxRightFrameIndex = (255 * AuxRight) + (m.m_ptr[3]&0xff);
		else
			m_ImageQueueItem.AuxRightFrameIndex  =  m.m_ptr[3]&0xff;

		m.m_frameIndex = m_ImageQueueItem.AuxRightFrameIndex;
	}
	if(m_ImageQueueItem.m_ExtCameraIndex == 01){
		if(MainLeft != 0)
			m_ImageQueueItem.MainLeftFrameIndex = (255 * MainLeft) + (m.m_ptr[3]&0xff);
		else
			m_ImageQueueItem.MainLeftFrameIndex  =  m.m_ptr[3]&0xff;

		m.m_frameIndex = m_ImageQueueItem.MainLeftFrameIndex;
	}
	if(m_ImageQueueItem.m_ExtCameraIndex == 02){
		if(MainRight != 0)
			m_ImageQueueItem.MainRightFrameIndex = (255 * MainRight) + (m.m_ptr[3]&0xff);
		else
			m_ImageQueueItem.MainRightFrameIndex  =  m.m_ptr[3]&0xff;

		m.m_frameIndex = m_ImageQueueItem.MainRightFrameIndex;
	}

	cntr = m.m_frameIndex;

	// Counter Increment
	if((m.m_ptr[3]&0xff) == 255 && m_ImageQueueItem.m_ExtCameraIndex == 129){
		AuxLeft++;
	}
	if((m.m_ptr[3]&0xff) == 255 && m_ImageQueueItem.m_ExtCameraIndex == 130){
		AuxRight++;
	}
	if((m.m_ptr[3]&0xff) == 255 && m_ImageQueueItem.m_ExtCameraIndex == 01){
		MainLeft++;
	}
	if((m.m_ptr[3]&0xff) == 255 && m_ImageQueueItem.m_ExtCameraIndex == 02){
		MainRight++;
	}

	if(cntr & 0x1){
		m.m_ill0 = 1;
	}else{
		m.m_ill0 = 0;
	}

#if 0 // Original Frame Index 
	m.m_frameIndex = cntr;
	cntr++;
#endif
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
//DMOLEAK	m_ImageQueueItem.m_ptr = new unsigned char[m_ImageSize];

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


    bool status = false; // reset

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

#if 0 // Dave code
char *BufferBasedFrameGrabber::getLatestFrame_raw_nowait(){
	//if (m_Debug)
		//EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() Start");

	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;


    bool status = true;

	// release the previously processed buffer it it is real;
    if (m_current_process_queue_item.m_ptr!=0)
    	status = TryReleaseProcessBuffer(m_current_process_queue_item);//DMO changed to "Try" so we don't block on a full buffer.

    if (!status)
    {
		//usleep(1000);
    	return NULL;
    }

    status = false; // reset

	while(1)
		{
		 if (m_ProcessBuffer->Empty())
		 {
			//usleep(1000);
			 return NULL;
		 }

		 status=m_ProcessBuffer->TryPop(m_current_process_queue_item);

		if (status)
			break; //something to process

	//	usleep(1000);
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

#else
// Anita changed it for choppy lines in frames
char *BufferBasedFrameGrabber::getLatestFrame_raw_nowait(){
	//if (m_Debug)
		//EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::getLatestFrame_raw() Start");

	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;

	// release the previously processed buffer it it is real;
    if (m_current_process_queue_item.m_ptr!=0)
    	 TryReleaseProcessBuffer(m_current_process_queue_item);//DMO changed to "Try" so we don't block on a full buffer.

    bool status = false; // reset

	while (1)
	{
		status = m_ProcessBuffer->TryPop(m_current_process_queue_item);
		if (status)
			break;
		// usleep(1000);
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
	static unsigned int cntr=0;
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
	unsigned int static cntr1=0;
	cntr1++;
	//printf("pushed so far .. %d and dropped %d\n",cntr1,frame_drop);

	if (m_Debug)
		EyelockLog(logger, DEBUG, "setLatestFrame_raw() Push to ring buffer");
}
void BufferBasedFrameGrabber::term()
{
	if(m_pImageBuffer)
	{
		delete [] m_pImageBuffer;
		m_pImageBuffer = 0;
	}
	if (m_RingBufferOffset)
	{
		delete m_RingBufferOffset;
		m_RingBufferOffset = 0;
	}
	if (m_pRingBuffer)
	{
		delete m_pRingBuffer;
		m_pRingBuffer = 0;
	}
//	if(m_ImageQueueItem.m_ptr)
//			delete [] m_ImageQueueItem.m_ptr;

	//TODO: destrutor for all allocated m_ImageQueueItem
}

void BufferBasedFrameGrabber::clearFrameBuffer(){
	if (m_Debug)
		EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::ClearRing() Start");

	m_pRingBuffer->Clear();
}
