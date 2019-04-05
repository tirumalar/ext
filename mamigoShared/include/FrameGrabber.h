/*
 * FrameGrabber.h
 *
 *  Created on: Jan 8, 2011
 *      Author: developer1
 */

#ifndef FRAMEGRABBER_H_
#define FRAMEGRABBER_H_
#include <stddef.h>
#include <sys/types.h>

class Configuration;

class FrameGrabber{

public:
	FrameGrabber():fd(0),imagebits(8),m_ill0(0),m_frameIndex(0){}
	virtual ~FrameGrabber(){}
	// life cycle methods
	virtual void init(Configuration *pCfg=0){}
	virtual void term(){}
	virtual bool start(bool bStillFrames=false){ return false;}
	virtual bool stop(){return false;}

	// must implement
	virtual void getDims(int& width, int& height) const{}
	virtual char *getLatestFrame_raw_nowait(){return NULL;}
	virtual char *getLatestFrame_raw(){return 0;}
	//virtual void getIlluminator(int& il0,int& il1){ il0 = m_ill0;il1 = m_frameIndex;}
	virtual void getIlluminatorState(__int64_t& ts,int &il0,int& il1){ ts = m_ts,il0 = m_ill0; il1 = m_frameIndex;}

	virtual size_t readFrame(void *buf, size_t count){return 0;}
	virtual int update(unsigned char regis , unsigned short value){return 0;}


	// defaulted
	virtual bool isBayer() const {return false;}
	virtual bool isITU656Source() const {return false;}
	virtual void getPPIParams(int& pixels_per_line, int& lines_per_frame, int& ppiControl) const{}
	virtual bool isRunning(){ return fd>0;}
	void SetImageBits(int bits){imagebits=bits;}
	int GetImageBits(){ return imagebits;}
	virtual void setLatestFrame_raw(char *ptr){}
	virtual void SetShiftAndOffset(unsigned short dc,int shift){}
	virtual bool SetPWM(unsigned char addr, unsigned char value){return true;}
	virtual void FlushAllInput(){}
	virtual void clearFrameBuffer(){}
protected:
	int fd;
	int imagebits;
	virtual void update_extra_regs(char *str);
	int m_ill0,m_frameIndex;
	__int64_t m_ts;
};


#endif /* FRAMEGRABBER_H_ */
