/*
 * ARMMamigoFrameGrabber.cpp
 *
 *  Created on: 13-Dec-2010
 *      Author: mamigo
 */
#include "MamigoFrameGrabber.h"

	MamigoFrameGrabber::MamigoFrameGrabber(short sType){}
	MamigoFrameGrabber::~MamigoFrameGrabber(){}
	void MamigoFrameGrabber::init(Configuration *pCfg){}
	void MamigoFrameGrabber::term(){}
	bool MamigoFrameGrabber::start(bool bStillFrames){ return false;}
	bool MamigoFrameGrabber::stop(){return false;}
	// the two sensors are producing Bayer
	bool MamigoFrameGrabber::isBayer()const{return false;}
	bool MamigoFrameGrabber::isITU656Source()const{return false;}
	void MamigoFrameGrabber::getDims(int& width, int& height)const{}
	void MamigoFrameGrabber::getPPIParams(int& pixels_per_line, int& lines_per_frame, int& ppiControl)const{}

	/*
	 * the returned pointer is over written frequesntly
	 * so it should not be used for a long time.
	 */
	char *MamigoFrameGrabber::getLatestFrame_raw(){return 0;}

	// typicall linux read e.g. http://linux.die.net/man/2/read
	size_t MamigoFrameGrabber::readFrame(void *buf, size_t count){return 0;}

