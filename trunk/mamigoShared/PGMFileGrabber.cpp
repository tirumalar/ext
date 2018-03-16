/*
 * PGMFileGrabber.cpp
 *
 *  Created on: 17 Mar, 2009
 *      Author: akhil
 */
#include <cxcore.h>
#include "PGMFileGrabber.h"
extern "C"{
#include "file_manip.h"
}

char *PGMFileGrabber::getLatestFrame_raw()
{
	// use our own mechanism to load the image into this
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

	if(!_curImage) init();
	assert(_curImage!=0);
	char* fName=getCurFileName();
	int w,h;
//	printf("Reading file %s \n",fName);
//	if(!_curImage){
		int rc=ReadPGM5(fName,(unsigned char *)_curImage->imageData,&w,&h,_curImage->imageSize);

		if(/*(rc!=0) ||*/w!=_curImage->width||h!=_curImage->height){
			printf("PGMFileGrabber::failed to load %s\n",fName);
			assert(false);
		}
//}
	_nextFrameIdx++;
	_nextFrameIdx%=_maxFrames;	// rotate
	return _curImage->imageData;
}
