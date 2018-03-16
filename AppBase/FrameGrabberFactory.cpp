/*
 * FrameGrabberFactory.cpp
 *
 *  Created on: 19 Dec, 2008
 *      Author: akhil
 */
#include <stdio.h>
#include "FrameGrabberFactory.h"
#include "FileBasedFrameGrabber.h"
#include "PGMFileGrabber.h"
#include "MamigoFrameGrabber.h"
#include "MT9P001FrameGrabber.h"
#include "BufferBasedFrameGrabber.h"
#include "CommonDefs.h"

enum sourceType {eMT9P031=0, eMT9V032, eADV7183_PAL, eADV7183_NTSC,eMT9P001,eBUFFER};

short FrameGrabberFactory::getSensorType(Configuration *& pConf)
{
    return pConf->getValueIndex("source.type", eMT9P031, eBUFFER, -1, "MT9P031", "MT9V032", "ADV7183_PAL", "ADV7183_NTSC","MT9P001","BUFFER");
}

FrameGrabberFactory::FrameGrabberFactory(Configuration *pConf)
{
	setConf(pConf);
	sensorType =getSensorType(pConf);
	testFileNamePattern=pConf->getValue("test.fileNamePattern","");
	testFileCount=pConf->getValue("test.fileCount",0);
	m_numberofBits = pConf->getValue("Eyelock.NumBits",8);

}

FrameGrabber *FrameGrabberFactory::create()
{
	FrameGrabber* framer=0;

	if(sensorType==-1){
		if(strstr(testFileNamePattern,".pgm")||strstr(testFileNamePattern,".PGM"))
		{
			printf("FrameGrabberFactory:: assuming pgm file based frame grabbing\n");
			framer=new PGMFileGrabber(testFileNamePattern,testFileCount);
		}
		else
		{
			printf("FrameGrabberFactory:: assuming file based frame grabbing\n");
			framer=new FileBasedFrameGrabber(testFileNamePattern,testFileCount);
		}
	}
	else if(sensorType==eMT9P001)
	{
#ifdef __ARM__
		framer=new MT9P001FrameGrabber();
#else
		bool _MamigoFrameGrabber_only_supported_on_arm=false;
		assert(_MamigoFrameGrabber_only_supported_on_arm);
		throw("_MamigoFrameGrabber_only_supported_on_arm");
#endif
	}
	else if(sensorType==eBUFFER)
	{
		framer=new BufferBasedFrameGrabber();
	}
	else
	{
#ifdef __BFIN__

		framer=new MamigoFrameGrabber(m_numberofBits);
#else
		bool _MamigoFrameGrabber_only_supported_on_bfin=false;
		assert(_MamigoFrameGrabber_only_supported_on_bfin);
		throw("_MamigoFrameGrabber_only_supported_on_bfin");
#endif

	}

	framer->init(getConf());
	return framer;
}


