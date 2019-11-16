/*
 * MT9P001FrameGrabber.h
 *
 *  Created on: Jan 8, 2011
 *      Author: developer1
 */

#ifndef MT9P001FRAMEGRABBER_H_
#define MT9P001FRAMEGRABBER_H_

#include "FrameGrabber.h"
#include "CamConfig.h"

#include <pthread.h>
#include <vector>
#include <deque>
#include "HThread.h"
#include "Synchronization.h"


class APRPool;
class SynchronizationServer;
class CmxHandler;
class SocketFactory;	// FJ
class HostAddress;

#define TCP_SYNCHRONIZE 0

class CameraRecipe
{
public:
	CameraRecipe(int exposure, int led) : m_Exposure(exposure), m_LedMask(led)
	{
	}
	int GetExposure() const { return m_Exposure; }
	int GetLEDMask() const { return m_LedMask; }
protected:
	int m_Exposure;
	int m_LedMask;
};

class RGBTriple
{
public:
	RGBTriple() : m_R(0), m_G(0), m_B(0) {}
	RGBTriple(int r, int g, int b) : m_R(r), m_G(g), m_B(b) {}
	void Set(int r, int g, int b)
	{
		m_R = r;
		m_G = g;
		m_B = b;
	}
	int R() const { return m_R; }
	int G() const { return m_G; }
	int B() const { return m_B; }
	int m_R, m_G, m_B;
};

typedef struct ImageProp{
	unsigned char *m_ptr;
	int m_ill0;
	int m_frameIndex;
	__int64_t m_startTime,m_endTime;
};


typedef RingBuffer<ImageProp> RingBufferImage;
typedef RingBuffer<int> RingBufferOffset;
enum FGSTATE{eREQUIREFRAME=0,eDUMMYCAPTURE};

class MT9P001FrameGrabber: public FrameGrabber, public HThread
{
public:
	MT9P001FrameGrabber();
	virtual ~MT9P001FrameGrabber();
	virtual void init(Configuration *pCfg);
	virtual void term();
	virtual bool start(bool bStillFrames);
	virtual bool stop();
	virtual void getDims(int& width, int& height)const;
	virtual char *getLatestFrame_raw();
	virtual size_t readFrame(void *buf, size_t count);
	virtual int update(unsigned char regis , unsigned short value);
	void update_all(camconfig* newconfig);
	void map_uservals_to_registers(Configuration *pCfg,camconfig* newconfig);
	void getPPIParams(int& pixRingBufferels_per_line, int& lines_per_frame, int& ppiControl)const;
	void SetDoAlternating(bool mode);

	bool GetDoAlternating();
	void FlashRGB(const RGBTriple &rgb);
	void SetRecipeIndex(int index);
	Safe<int> m_RecipeIndex;
	Safe<bool> m_DoAlternating;
	Safe<RGBTriple> m_RGB;
	int m_RGBBrightness;

	int m_LedMask;
	std::vector< std::vector<CameraRecipe> > m_Recipes;
	virtual void SetShiftAndOffset(unsigned short dc, int shift);
	virtual bool SetPWM(unsigned char addr, unsigned char value);
	void FlushAllInput();
	ImageProp GrabFrame(int flashUS, int triggerUS, int offset,int height, int led = 255); // public for now, could make MT9P001PushHandler a friend instead
	void PrintState(char *str);

	void SetState(FGSTATE state);
	FGSTATE GetState();

	void setShouldGrab(bool val);
//	ImageProp &GetImageProperties() { return m_ImageProperties; }
	void SetExposureandIlluminator(bool fromnw,int flashtime, int triggertime , int illval);
	bool getFromNetwork();

	// define FJ

	void SendCmxMessage(char *outMsg, int len);
	SocketFactory *m_socketFactory;
		HostAddress *m_resultDestAddr;
		struct timeval m_timeOutSend;

protected :
	virtual const char* getName() { return "MT9P001FrameGrabber"; }
	unsigned int MainLoop(); // iFrameGrabbermplement virtual
	bool shouldWait();
	void FlashRGB();
	void FlashRGBSetLEDMask();

	bool m_Debug;
	int m_BufferSize;
	int m_ImageSize;
	int m_Width;
	int m_Height,m_currHeight,m_dummyHeight;
	int m_Offset;

	unsigned short m_FlashTime;
	int m_RGBReset;
	unsigned char *m_pImageBuffer;
	int m_MasterMode;
	int m_FramePause;
	int m_IrLedBankIndex;
	int m_TriggerTimeUs;
	pthread_t m_Thread;
	RingBufferImage *m_pRingBuffer;
	RingBufferOffset *m_RingBufferOffset;
	unsigned char *m_Map_base;
	CameraConfig *m_camCfg;

	int m_RecipeIndexIndex;
	int m_BufferIndex;
	int m_counter;
	uint64_t m_timeelapsed;
	int64_t m_sleeptimeInDummyRead,m_DummySleep;
	Mutex m_stateLock;
	FGSTATE m_state;
	int m_SleepTime;
	int m_dummyOffset;
	int m_newDriverEnable;
	bool m_DummyOffsetFullyAllocated;
	Mutex m_ShiftAndDCLock;
	unsigned short m_ShiftRight,m_DcOffset;
	void GetShiftAndOffset(unsigned short& dc,int& shift);

	Mutex m_grabLock;
	bool m_dograb;
	void  Epilog();
	unsigned int process(int& index, int& i,int& recipeIndex );
	std::vector<std::pair<unsigned short, unsigned short> > m_shiftAndOffset;
	int m_NextShiftAndDCSec;
	bool m_diffIllumination;
	int m_masterExposure;
	int m_slaveExposure;
	bool m_resetusingpsoc;
	int m_numbits;

	Mutex m_nwMsgLock;
	int m_nwTriggerTime,m_nwFlashTime,m_nwIlluminatorvalue;
	bool m_fromNetwork;

#if TCP_SYNCHRONIZE
	SynchronizationServer *m_pSynchServer;
	APRPool *m_pPool;
#endif
};

#endif /* MT9P001FRAMEGRABBER_H_ */
