#pragma once

#if (defined(__BFIN__)||defined(__linux__))
#include <sys/types.h>
#include <pthread.h>
#include <stdint.h>
#define HBOX_API
#define __stdcall
#endif

#include "Safe.h"
class HBOX_API HThread
{
public:
	static bool m_FreqEnable;
	HThread(void);
	virtual ~HThread(void);
	virtual unsigned int MainLoop() = 0; // User needs to implement this
	virtual bool ShouldIQuit();
	virtual int Begin();
	virtual int End();
	virtual bool IsHealthy(uint64_t curTimeMs=0);
	static void * __stdcall BeginProxy(void *arg);
	virtual const char *getName()=0;
	void Frequency(void);
	void SetFreq(bool val){ m_FreqEnable = val;}
protected:
	bool waitForData();
	bool waitForDataAndTimeOut();
	virtual bool shouldWait(){return false;}
	bool waitForData(int secs );
	void dataAvailable();
	void setHealthy(bool bForceUnhealthy=false,int secsInFuture=0);
	Safe<bool> m_QuitStatus;
	pthread_t m_hThread;
	unsigned int m_Error;
	pthread_cond_t data_avlbl_cv;
	pthread_mutex_t data_avlbl_mutex;

	// used for making a wait with timeout
	struct timeval m_timeuSec;
	struct timespec m_timenSec;
	unsigned long m_timeOutuSec;
	uint64_t m_lastSeenHealthy;
	bool m_isUnhealthy;
	int m_counter;
	uint64_t m_timeelapsed;

};
