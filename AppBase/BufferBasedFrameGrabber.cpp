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

const char logger[30] = "BufferBasedFrameGrabber";

BufferBasedFrameGrabber::BufferBasedFrameGrabber():m_Width(0),m_Height(0),m_WidthStep(0),m_pImageBuffer(0) {
	m_ImageSize = 0;
	m_Debug = false; //true;
}

BufferBasedFrameGrabber::~BufferBasedFrameGrabber() {
}

void BufferBasedFrameGrabber::init(Configuration *pConf){
	if (m_Debug)
		EyelockLog(logger, DEBUG, "BufferBasedFrameGrabber::init() Start");
// Read from FILE and update Width and Height
	m_Width=pConf->getValue("FrameSize.width",1200); //1984);
	m_Height=pConf->getValue("FrameSize.height",960);	//1392);
	m_WidthStep=pConf->getValue("FrameSize.widthstep",0);
	imagebits=pConf->getValue("FrameSize.bits",8);

	m_ImageSize = m_Width * m_Height;
	//m_ImageSize = 2 * m_Width * m_Height;
	if(m_Debug)
	{
		//  W H S = 1984 1392 5523456
		EyelockLog(logger, DEBUG, "W H S = %d %d %d ",m_Width,m_Height,m_ImageSize);
	}
	m_pImageBuffer = new char[m_ImageSize];


	int numbits = pConf->getValue("Eyelock.NumBits", 8);
	m_numbits = numbits > 8 ? 16 : numbits;
	SetImageBits(m_numbits);
#ifndef HBOX_PG
	m_pRingBuffer = new RingBufferImageQueue(20); /* Allocate ring buffer to be frame buffer - 1 */
	m_RingBufferOffset = new RingBufferQueueOffset(20);
#else
	m_pRingBuffer = new RingBufferImageQueue(10); /* Allocate ring buffer to be frame buffer - 1 */
	m_RingBufferOffset = new RingBufferQueueOffset(10);
#endif	
	m_ill0 = 0;
	m_frameIndex = 0;
	m_ts = 0;
}

void BufferBasedFrameGrabber::getDims(int & width, int & height) const
{
	width = m_Width;
	height = m_Height;
}

#if 1
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

	ImageQueue val;
	//while(!ShouldIQuit()&&!m_pRingBuffer->TryPop(val))

	bool status = m_pRingBuffer->TryPop(val);
	if (status == true) {
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
	else {
		memset(m_pImageBuffer, 0, length);
	}
#if 0
	if (m_Debug)
		EyelockLog(logger, DEBUG, "get image queue m_ill0 %d, m_frameIndex %d, time %llu", m_ill0,m_frameIndex, m_ts);
#endif		
	return m_pImageBuffer;
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
	ImageQueue val;
	static int cntr=0;
	int length = (m_Width * m_Height);
	if (m_numbits != 8)
		length = length * 2;

	if(cntr & 0x1){
		val.m_ill0 = 1;
	}else{
		val.m_ill0 = 0;
	}
	val.m_frameIndex = cntr;
	cntr++;

	unsigned char *pdata = val.m_ptr;
	memcpy(pdata, ptr, length);

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);
	val.m_startTime = starttimestamp;
	val.m_endTime = val.m_startTime;

	m_pRingBuffer->TryPush(val);
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
}

void BufferBasedFrameGrabber::clearFrameBuffer(){
	if (m_Debug)
		EyelockLog(logger, TRACE, "BufferBasedFrameGrabber::ClearRing() Start");

	m_pRingBuffer->Clear();
}
