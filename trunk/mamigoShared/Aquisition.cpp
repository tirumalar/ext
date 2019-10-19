/*
 * Aquisition.cpp
 *
 *  Created on: 23-Oct-2009
 *      Author: mamigo
 */

#include "Aquisition.h"
#include "FrameGrabberFactory.h"
#include <stdio.h>
#include <unistd.h>

Aquisition::Aquisition(Configuration *pConf,bool bSingleFrame):temp_header(0),m_bSingleFrame(bSingleFrame)
{
	int width, height,temp;
	framer=FrameGrabberFactory(pConf).create();

#if 0
	framer->getPPIParams(width,height,temp);
	width=pConf->getValue("FrameSize.width",width);
	height=pConf->getValue("FrameSize.height",height);
	height=1+pCfg->getValue("MT9P031.row_size_val",height-1);
	width=1+pCfg->getValue("MT9P031.column_size_val",width-1);
#endif
	framer->getDims(width,height);
	printf("Aquisition::Getting Number of Bits %d\n",framer->GetImageBits());
	temp_header	= cvCreateImageHeader(cvSize(width,height),framer->GetImageBits(),1);
	framer->start(m_bSingleFrame);
}

Aquisition::~Aquisition()
{
	framer->term();
	delete framer; // stops and terminates
	if(temp_header)cvReleaseImageHeader(&temp_header);
}

size_t Aquisition::getSingleFrame(void * buff,size_t cnt)
{
	if(!framer->isRunning()){
		if(!framer->start(true)){
			throw "could not start framer";
		}
	}
	int readBytes=framer->readFrame(buff,cnt);
    if(readBytes<=0){
    	throw "could not grab a Single frame";
    }
    return readBytes;
}



IplImage *Aquisition::getFrame()
{
	if(!framer->isRunning()){
		if(!framer->start()){
			throw "could not start framer";
		}
	}
    char *raw = framer->getLatestFrame_raw();
    m_ScaledFaceRect = framer->getLatestScaledFaceRect();
    if(raw==0){
    	framer->stop();
    	throw "could not grab a frame";
    }

    temp_header->imageData=raw;
    return temp_header;

}


IplImage *Aquisition::getFrame_nowait()
{
	if(!framer->isRunning()){
		if(!framer->start()){
			throw "could not start framer";
		}
	}

	// printf ("\n\n Acquisition GetframeNoWait() \n\n");
	// return immediately even if queue empty
    char *raw = framer->getLatestFrame_raw_nowait();
    m_ScaledFaceRect = framer->getLatestScaledFaceRect();

    if (raw == NULL)
    	return NULL;

    temp_header->imageData=raw;
    return temp_header;
}


AquisitionFile::AquisitionFile(Configuration *pConf, int binType):Aquisition()
{
	int width, height;
	this->binType=binType;
	framer=FrameGrabberFactory(pConf).create();
	framer->getDims(width,height);
//	frame=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	sleepPerFrameUSec=1000*pConf->getValue("test.sleepPerFrameMS",50);
	temp_header	= cvCreateImageHeader(cvSize(width,height),framer->GetImageBits(),1);
	printf("AquisitionFile::Depth %d\n",framer->GetImageBits());
	framer->start();
}

IplImage *AquisitionFile::getFrame()
{
	if(!framer->isRunning()){
		if(!framer->start()){
			throw "could not start framer";
		}
	}
    char *raw = framer->getLatestFrame_raw();
    if(raw==0){
    	framer->stop();
    	throw "could not grab a frame";
    }
    temp_header->imageData=raw;
  //  cvCopyImage(temp_header,frame);
    usleep(sleepPerFrameUSec);
    return temp_header;
}

void AquisitionFile::getDims(int& width, int& height){
	framer->getDims(width,height);
	if(binType==1)
		height*=2;
	else if (binType==2)
		width*=2;

}

AquisitionBuffer::AquisitionBuffer(Configuration *pConf):Aquisition()
{
	int width, height;
	framer=FrameGrabberFactory(pConf).create();
	framer->getDims(width,height);
	temp_header	= cvCreateImageHeader(cvSize(width,height),framer->GetImageBits(),1);
	printf("AquisitionBuffer::%d %d %d\n",width,height,framer->GetImageBits());
	framer->start();
}

void AquisitionBuffer::setLatestFrame_raw(char *ptr){
	framer->setLatestFrame_raw(ptr);
}

void AquisitionBuffer::clearFrameBuffer(){
	framer->clearFrameBuffer();
}


IplImage *AquisitionBuffer::getFrame()
{
	if(!framer->isRunning()){
		if(!framer->start()){
			throw "could not start framer";
		}
	}
    char *raw = framer->getLatestFrame_raw();
    m_ScaledFaceRect = framer->getLatestScaledFaceRect();
    if(raw==0){
    	framer->stop();
    	throw "could not grab a frame";
    }
    temp_header->imageData=raw;
    return temp_header;
}


void AquisitionBuffer::getDims(int& width, int& height){
	framer->getDims(width,height);
}

