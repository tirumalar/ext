/*
 * ImageGrabber.h
 *
 *  Created on: 29-Sep-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef IMAGEGRABBER_H_
#define IMAGEGRABBER_H_

#include <GenericProcessor.h>
#include "EyeLockImageGrabber.h"
#include "MatchProcessor.h"
#include "EyeDispatcher.h"
#include "socket.h"
#include "SafeFrameMsg.h"
#include "CircularAccess.h"
#include "NWHDMatcher.h"
#include "ELEyeMsg.h"
#include "LEDDispatcher.h"
#include "NwLEDDispatcher.h"
#include "CmxHandler.h"


// MT9P001FrameGrabber needed in order to set RGB status and camera recipe index
//   from EyelockThread methods, since a generic mechanism is lacking and there
//   isn't sufficient need to add this (I think) as it varies on a somewhat case
//   by case basis.
class MT9P001FrameGrabber;
class HTTPPOSTMsg;
class BinMsg;
class SocketFactory;

//0 = TCP EYE OUT
//1 = MATCHING
//2 = MATCHING + TCP EYE OUT
//3 = EMBEDDED ENROLLMENT
//4 = TCP EYE OUT w/ SECOND ILLUMINATION SETTINGS

enum EyelockProcessMode {
	EyelockCamera = 0, // 0
	EyelockMatcher,    // 1
	EyelockDual,       // 2
	EyelockEnroll,     // 3
	EyelockCamera2,     // 4
	EyelockSleep,       //5
	EyelockWakeUP       //6
};

/*
 * RGB HW abstraction for LEDDisplatcher class
 */

class RGBControllerEyelock : public RGBController
{
public:
	RGBControllerEyelock();
	~RGBControllerEyelock();
	void SetRGB(unsigned char mask);
};

class RGBControllerNano : public RGBController
{
public:
	RGBControllerNano(MT9P001FrameGrabber *pFrameGrabber, int brightnessR = 10, int brightnessG = 10, int brightnessB = 10);
	~RGBControllerNano();
	void SetRGB(unsigned char mask);
	//void SetBrightness(int brightness) { m_Brightness = brightness; }
	MT9P001FrameGrabber *m_pFrameGrabber;
	int m_BrightnessR;
	int m_BrightnessG;
	int m_BrightnessB;
	CmxHandler *m_pCmxHandler;
	void SetCmxHandler(CmxHandler *pCmxHandler) {m_pCmxHandler = pCmxHandler; }
};

// Begin: Enrollment stuff
class EnrollmentProcessor;
class EmbeddedEnrollment;
// End: Enrollment stuff

#include <strings.h>
#include "NwListener.h"

typedef struct ShiftHaloAndDC{
	short shift,dcoffset,maxspecval;
	float halothreshold;
};

class EyelockModeMessageHandler : public HTTPPostMessageHandler
{
public:
	EyelockModeMessageHandler();
	bool GetMode(EyelockProcessMode &value, std::string &address);
	bool Handle(HTTPPOSTMsg &message, Socket *client);
	void HandleEyeLockModeMsg(HTTPPOSTMsg &message, Socket *pClient);
	void SetEyeLockMode(string host,EyelockProcessMode value);
	bool getHaloDCState();
	void HandleHaloDcMsg(HTTPPOSTMsg &message);
	ShiftHaloAndDC getHaloDC();
	void ResetHaloDC();
	void Reset();
protected:
	std::string m_PeerAddress;
	Safe<int> m_Mode;
	Safe<ShiftHaloAndDC> m_ShiftHaloDC;
	Safe<int> m_Wakeup;
};

typedef struct SpoofVar{
	int m_index;
	int m_frIndex;
	int m_eyeIndex;
	int m_spoof;
	int m_score;
};

typedef struct SpoofInfo{
	int m_frameIndex;
	int m_eyeIndex;
	int m_score;
	int m_spoof;
};

typedef struct FrameInfo{
	int m_frameIndex;
	int m_eyeCount;
};

class EyeLockThread: public GenericProcessor {
public:
	EyeLockThread(Configuration& conf);
	virtual ~EyeLockThread();
	void process(Copyable *msg);
	bool enqueMsg(Copyable& msg);
	virtual const char *getName(){ return "ImageGrabber";}
	virtual Copyable *createNewQueueItem();
  	bool IsQueueFull(void);
	bool IsQueueEmpty(void);
	int GetAvailableImages(void);
	void SendDetectLED(void);
	virtual int Begin();
	virtual int End();
	int ExtractFrameInfo(void);
	int RemoveUntrackedEyes(int lastindex = 0x7FFFFFFF,int debug =1);
	void PrintSpoofInfo();
	void DetectEyes();
	void MatchBestEyes();
	void ExtractSpoof(void);
	bool SpoofFlow();
	void PrintTrackerResult(SpoofTrackerResult* ptr, int cnt);
	void PrintFrameInfo();
	void MatchEyesWithIntraSpoofComputation();
	EyelockModeMessageHandler * GetEyelockModeMessageHandler() { return m_pEyelockMessageHandler; }
	unsigned int ChangeProcessingMode(EyelockProcessMode mode, const char *address);
	bool CheckTrackedCount();
	EyelockModeMessageHandler *m_pEyelockMessageHandler;
	void setFrameType(int val){ m_ImageProcessor->setFrameType(val);}
	void setShouldDetectEyes(bool val){ m_ImageProcessor->setShouldDetectEyes(val);}
	int getFrameType() { return m_ImageProcessor->getFrameType();}
	bool getShouldDetectEyes() { return m_ImageProcessor->getShouldDetectEyes();}

	void setMode(int val){ m_Mode = (EyelockProcessMode)val;}

	void setSecureComm (bool secure) {	m_pEyeDispatcher->SetSecureComm (secure);}
	void resetSecureComm () {	m_pEyeDispatcher->ResetSecureComm ();}
	int getMode () {return m_Mode;}
	//EmbeddedEnrollment *m_pEmbeddedEnrollment;
	MatchProcessor *m_MatchProcessor;
	ProcessorChain *m_ledConsolidator;
	EyeDispatcher *m_pEyeDispatcher, *m_pEnrollmentEyeDispatcher;
	EyeLockImageGrabber *m_ImageProcessor;
	NwListener *m_pNwListener;
	NwLEDDispatcher *m_nwLedDispatcher;
	int m_MinTrackedCount;
	void SetNetworkListener(NwListener *pListener) { m_pNwListener = pListener; }
	void Enrollment(HTTPPOSTMsg *pMsg);
	void SendHBStatus();
	bool ShouldIDie() const { return m_MustDie; }
	int getFramcount(){ return m_FrameCount;}
#ifdef HBOX_PG
	HttpPostSender *m_HttpPostSender;
#endif
#ifndef UT_EYELOCK
protected:
#endif
	virtual int getQueueSize(Configuration* conf){ return 1;}
	virtual Copyable *getNextMsgToProcess();
	bool FindBest(int& pos);
	bool FindBestSpoofScore(int& position);
	void SetUpdatedFalse(int pos);
	void FlushAll(void);
	virtual unsigned int MainLoop();
	void ProcessEyelockMode();
	void sendLiveImages();
	void SendMessageToSlave(BinMessage& msg);
	int FormatProcessModeMessage(EyelockProcessMode mode, const char *address);
	CircularAccess<SafeFrameMsg *> outMsgQueue;
	SafeFrameMsg m_DetectedMsg;
	SafeFrameMsg m_MotionMsg;
	int m_outQSize;
	ELEyeMsg m_bestEye;
	int m_MaxFrames;
	bool m_Debug;
	LEDResult m_ledMR;
	EyelockProcessMode m_Mode, m_PreviousMode;
	int m_noImageMode;
	MatchResult m_matchResult;
	HostAddress *m_HBDestAddr;
	const char* m_HBMsgFormat;
	int m_HBFreqSec;
	int m_HBTimeOutmilliSec,m_segmentationMiliSecSleep;
	BinMessage m_HBMsg, m_ProcessModeMsg;
	bool m_Master;
	long int m_FuturisticTime;
	std::vector<HostAddress *> m_SlaveAddressList;
	FrameInfo m_FrameInfo[50];
	int m_FrameCount;
	double m_ScoreFactor;
	bool m_spoofEnable;
	bool m_shouldProcess;
	bool m_MustDie;
	bool m_logging;
	bool m_shouldSegment;
	bool m_trackingEnabled;
	bool m_doMatching;
	bool m_usbEnabled;
	bool m_dualAuth;
	bool m_transTOC;
	SocketFactory *m_socketFactory;
	HostAddress *m_detectDestAddr;
	BinMessage *m_DetectBinMsg;
	ELEyeMsg m_BestFirstEye;
	ELEyeMsg m_BestSecondEye;	
	bool m_ledPWMSwitching,m_currentState;
	uint64_t m_LastDetectedEyeTS;
	uint64_t m_TimeToNormalusec;
	bool m_tsDestAddrpresent;
	SoftwareType m_softwareType;
	int m_sleepTimeFlow;
#ifdef HBOX_PG
	bool m_SPAWAREnable;
	int m_HeartBeatFrequency;
#endif
};

#endif /* IMAGEGRABBER_H_ */
