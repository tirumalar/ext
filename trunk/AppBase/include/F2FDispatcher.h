/*
 * F2FDispatcher.h
 *
 *  Created on: 26-Oct-2016
 *      Author: fjia
 */

#ifndef F2FDISPATCHER_H_
#define F2FDISPATCHER_H_

#include "ResultDispatcher.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include "MatchType.h"

#define RELAY_TIME_IN_MS 	1000
#define CHECK_TOC_TIME		1000	// 1 sec

#define MAX_PIN_BYTE_LENGTH 100+1

class MatchedSignal;
class SocketFactory;
class LEDConsolidator;
class MatchManagerInterface;
class SDKDispatcher;
class OSDPMessage;
class MatchType;
class HTTPPostMessageHandler;

enum F2FSTATE{eF2FSTART=0, eF2FSTOP};



class F2FDispatcher: public ResultDispatcher {
public:
	F2FDispatcher(Configuration& conf);
	void SetLEDConsolidator(LEDConsolidator *ptr){ m_ledConsolidator = ptr;}
	void SetMatchManager(MatchManagerInterface *ptr);
	void SetSDKDispatcher(SDKDispatcher *ptr){m_sdkDispatcher = ptr;}
	void SetHTTPPostMessageHandler(HTTPPostMessageHandler *ptr){pHTTPPostMessageHandler = ptr;}
	virtual ~F2FDispatcher();
	virtual int End();
	const char *getName(){
		return "F2FDispatcher";
	}
	virtual void RestartDispatcher();
	virtual void CloseDispatcher();
	void SetSendingEveryNSec(bool val=false){
		m_SendEveryNSec = val;
		if(m_Debug)printf("SetSendingEveryNSec %d \n",val?1:0);
	}
	virtual void ProcessOnEmptyQueue();
	unsigned int getPreviousTS(){ return  m_PreviousSendTS;}
	void CheckBoBEvents();
	void ProcessBoBCardReader();
	int GetCardData();
	bool ValidateTOCData();
	void SendTamperMsg();
	void SendAlarmMsg(char *string);
	void SetAccessDataLength(int len, const char *parityMask, const char *dataMask);
	void ProcessBoBAcsChange();
	void ProcessTamper(bool value, bool mbTamper);
	bool addUser(string perid, string name, string acd);
	void testSend(void);

	int m_cardRead;
	bool m_acsChange;
	bool m_tamperChange;
	int m_accessDataLength;
	bool isTamperSet;
	bool isMBTamper;
	bool isBoBTamper;
	int isLedSet;
	bool isSoundSet;
	bool m_Debug;
	bool m_tamperEnable;
	bool m_temperature;
	bool m_testCode;
	//static void GetBoBStatusCB ();
	bool killCurrentDriver;
	//OSDP message data

	int m_OSDPBRate;
	int m_ReaderOSDPBRate;
	bool m_updateACD;
	OSDPMessage *m_osdpMessage;

	LEDConsolidator * getLEDConsolidator() {return m_ledConsolidator;}
	bool loadACD();
	bool addACD(string acd, int acdlen, string acdnop, string name,string pin);
	bool modifyACD(string acd, int acdlen, string acdnop, string new_acd, string new_acdnop, string ping);

	void gettime();
	int settime();
	void StartLocateDevice();
	void StopLocateDevice();
	static void* LocateDeviceLoop(void *f2fDispatcherPtr);
	pthread_t locateDeviceThread;
	pthread_mutex_t locateDeviceLock;
	bool stopLocateDevice;

protected:
	bool checkToBeSendToNw( MatchResultState val);
	bool checkToBeSendToBOB( MatchResultState val);
	void MakeNwF2FMsg(MatchResult* mr, BinMessage* msg);
	void SendMsg(MatchResult *mr);
	virtual int getQueueSize(Configuration* conf);
	void SendToBoB(Configuration& conf);
	virtual void process(MatchResult *msg);
	bool findCardInNwMatch(char *card);
	bool MatchCard(char *card);
	void setCardMatchState();
	static void BoBStatusChangeCB();
	static int BoBOSDPCallback(void *, int);
	static int BoBReaderOSDPCallback(void *, int);
	static void CardMatchTimeoutCB(void *pthread);
	static void LEDTimeoutCB(void *pthread);

	bool SetDualTransCommand(int command);
	void OSDPBuzCommand(int state);
	void OSDPLedCommand(int state);
	void osdpUpdateACSChanges();
	void LogMatchResult(MatchResult *msg);
	void SetLED(MatchResultState state);
	void SetRelay(MatchResultState state);
	void SendData(MatchResult *msg);
	void ResetReaderLED();
	void SetLedInitState(int initLed);

	void StopEyesProcessing();
	void StartEyesProcessing();
	void ChangeEyeLockMode(int mode);

	MatchedSignal *m_pMatched;
	MatchType *m_pMatchType;
	bool m_pac;
	bool m_osdpACSEnabled;
	bool m_osdpReaderEnable;
	bool m_wiegand;
	bool m_f2f;
	int m_RelayTimeInMs;
	int m_DenyRelayTimeInMs;
	int m_State;

	//char **m_userData;





	MatchResult m_f2fResult;
	bool m_SendEveryNSec;
	int m_NSec;
	unsigned int m_PreviousSendTS;
	int m_SleepTime;

    HostAddress *m_resultDestAddr;
    SocketFactory *m_socketFactory;
	struct timeval m_timeOut,m_timeOutSend;
	bool m_sendMatchToBOB,m_sendLoteringToBOB,m_sendHealthToBOB,m_sendConfusionToBOB,m_dbReloadToBOB;
	bool m_sendMatchToNw,m_sendLoteringToNw,m_sendHealthToNw,m_sendConfusionToNw,m_dbReloadToNw;
	bool m_dualAuth;
	bool m_passThrough;
	bool m_transTOC;
	bool m_pinAuth;
	bool m_multiAuth;
	int m_dualAuthWaitIrisTime;
	int m_authMode;
	int m_pinBurstBits;
	int m_pinWaitTime;

	int m_ledControlledByInput;
	bool m_RelayWithSignal;
	LEDConsolidator *m_ledConsolidator;
	HostAddress *m_tamperDestAddr;
	SocketFactory *m_socketFactoryTamper;
	MatchManagerInterface *m_matchManager;
	SDKDispatcher *m_sdkDispatcher;
	HTTPPostMessageHandler *pHTTPPostMessageHandler;
	char m_tamperMsg[256];
	int m_initialState;
	int m_silencePeriodAfterMatchMs;
	bool m_pollACS;
	bool m_tamperBitHighToLow;
	bool m_tamperOutBitHighToLow;
	char m_testData[100];
	char m_pinNumber[MAX_PIN_BYTE_LENGTH];
	int m_pinNumberRcvd;


	bool m_DebugTesting;
	std::string m_sessionDir;

	//pthread_t osdp_thread;
};

#endif /* F2FDISPATCHER_H_ */
