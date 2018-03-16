/*
 * Main.h
 *
 *  Created on: 28-Sep-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef EYELOCKMAIN_H_
#define EYELOCKMAIN_H_

#include <fcntl.h>

#ifndef __HAVE_ANDROID_WATCHDOG__
#include <linux/watchdog.h>
#endif

#include <sys/ioctl.h>
#include <iostream>
#include <vector>

#include "FileConfiguration.h"
#include "NwListener.h"
#include "CircularAccess.h"
#include "Safe.h"
#include "HTTPPOSTMsg.h"
#include "MatchProcessor.h"
#include "LEDDispatcher.h"
#include "LEDConsolidator.h"
#include "NwDispatcher.h"
#include "F2FDispatcher.h"

#include "DBReceive.h"
#include "MessageExt.h"
#include "EyeLockThread.h"
#include "EyeDispatcher.h"
#include "NwMatchManager.h"
#include "Safe.h"
#include "MatchDispatcher.h"
#include "EyelockNanoSdkThread.h"
#include "AudioDispatcher.h"
#include "SDKDispatcher.h"
#include "MasterSlaveNwListner.h"
#include "CmxHandler.h"

class RGBController;
class AudioDispatcher;
class LiquidLens;

using namespace std;
class EyeLockMain {
public:
	static int m_watchdogfd;
	static struct sigaction sa;

	EyeLockMain(char* filename);
	virtual ~EyeLockMain();
	void run();
	void startWithMatching();
	void stopEverything();
	void set_wd_counter();
	void PushAllThreads();
	bool IsAllHealthy();
	bool AreCamerasHealthy();
	void SendHBStatus();
	void reset_wd_counter();
	static void safe_exit(int);
	void startHDListener();
	void startTNIListener();
	void kill();
	int CheckTemperature();
	int CheckTamperSensor();
	int GetMotherBoardVersion();

	AudioDispatcher *GetAudioDispatcher(){ return pAudioDispatcher;}
	EyeLockThread *GetImageProcessor (){return pImageProcessor;}
	FileConfiguration conf;
private:
	NwListener nwListener;
	LEDDispatcher *pledDispatcher;
	LEDConsolidator *pledConsolidator;
	NwLEDDispatcher *pnwLEDDispatcher;
	MatchProcessor *pMatchProcessor;
	NwDispatcher   *pNwDispatcher;
	AudioDispatcher *pAudioDispatcher;
	F2FDispatcher *pF2FDispatcher;
	DBReceive *pDBReceive;
	EyeLockThread *pImageProcessor;
	EyelockNanoSdkThread *pEyelockNanoSdkThread;
	EyeDispatcher *pEyeDispatcher;
	NwMatchManager *pNwMatchManager;
	LiquidLens *m_pLiquidLens;
	LoiteringDetector *pLoiteringDetector;
	MatchDispatcher *m_matchDispatcher;
	SDKDispatcher *pSDKDispatcher;
	MasterSlaveNwListner *pMasterSlaveNwListener;
	CmxHandler *pCMXHandle;

	int m_watchDogTimeOutInSecs;
	bool m_enable_newWD;
	int m_mainSleepuSecs;
	bool m_watchDogIgnoreCtrlC;
	bool m_SendLed;
	vector <HThread *> m_Threads;
	int m_curval;

	RGBController *m_pRGBController;
    HostAddress *m_statusDestAddr;
	const char* m_msgFormat;
	int m_HBFreqSec;
	int m_HBTimeOutmilliSec;
	BinMessage m_outMsg;
	long int m_FuturisticTime;
	char *m_svrAddr;
	bool m_Debug;
	pthread_t hdThread;
	bool m_Master,m_Slave;
	SoftwareType m_softwareType;
	Safe<bool> m_KeepAlive;
	int m_EyelockType;
	int m_dualAuthCheckInterval;
	bool m_dualAuthN;
	bool m_temperature;
	bool m_tamper;
	char *m_timeSync;
	char *m_timeServAddr;
};

#endif /* EYELOCKMAIN_H_ */

