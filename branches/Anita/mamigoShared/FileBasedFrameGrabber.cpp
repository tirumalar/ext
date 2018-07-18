/*
 * FileBasedFrameGrabber.cpp
 *
 *  Created on: 19 Dec, 2008
 *      Author: akhil
 */
#include <highgui.h>
#include "FileBasedFrameGrabber.h"
#include "Configuration.h"
extern "C"{
#include "file_manip.h"
}

#include <stdio.h>
#include "mamigo/calc.h"
#include <unistd.h>
FileBasedFrameGrabber::FileBasedFrameGrabber(const char* frameFileNameFmt, int maxFrames){
	_frameFileNameFmt=frameFileNameFmt;
	_maxFrames=maxFrames;
	_curImage=0;
	_nextFrameIdx=0;
}

void FileBasedFrameGrabber::init(Configuration *pCfg)
{
  	// read the first file and initialize structures
	int w,h,bits;
	ReadPGM5WHandBits(getCurFileName(),&w,&h,&bits);
	printf("Reading File %s %d %d %d \n",getCurFileName(),w,h,bits);
	_curImage = cvCreateImage(cvSize(w,h),bits,1);
	int ret = ReadPGM5(getCurFileName(),(unsigned char *)_curImage->imageData,&w,&h,_curImage->imageSize);
	if(ret){
		printf("Unable to Read file\n");
	}
	printf("FileBasedFrameGrabber::%s \nInputImage Depth -> %d %d \n",getCurFileName(),_curImage->depth,_curImage->widthStep);
	SetImageBits(_curImage->depth);
}

void FileBasedFrameGrabber::term()
{
	if(_curImage) cvReleaseImage(&_curImage);
	_curImage=0;
}
void FileBasedFrameGrabber::getDims(int & width, int & height) const
{
	width=_curImage->width;
	height=_curImage->height;
}

char *FileBasedFrameGrabber::getLatestFrame_raw()
{
	static int cntr=0;
	if(cntr &0x1){
		m_ill0 = 1;
	}else{
		m_ill0 = 0;
	}
	m_frameIndex = cntr;
	cntr++;
	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);
	m_ts = starttimestamp;

	if(_curImage) cvReleaseImage(&_curImage); _curImage=0;
	char* fName=getCurFileName();
	int w,h,bits;
	ReadPGM5WHandBits(fName,&w,&h,&bits);
	_curImage = cvCreateImage(cvSize(w,h),bits,1);
	int ret = ReadPGM5(fName,(unsigned char *)_curImage->imageData,&w,&h,_curImage->imageSize);
	if(ret){
		printf("Unable to Read file\n");
	}

	if(_curImage==0)
		printf("FileBasedFrameGrabber::failed to load %s\n",fName);
	_nextFrameIdx++;
	_nextFrameIdx%=_maxFrames;	// rotate
	printf("GetLatest %d ",GetImageBits());
	return _curImage->imageData;
}

char *FileBasedFrameGrabber::getFileNameFor(int index)
{
	sprintf(_fileName,_frameFileNameFmt,index);
	return _fileName;
}















