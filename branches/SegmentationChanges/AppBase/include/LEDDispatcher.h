/*
 * LEDDispatcher.h
 *
 *  Created on: 17-Sep-2009
 *      Author: mamigo
 */

#ifndef LEDDISPATCHER_H_
#define LEDDISPATCHER_H_

#include "ResultDispatcher.h"
#include "CommonDefs.h"
#include "Synchronization.h"
#define DO_RGB_ABSTRACTION 1

#if DO_RGB_ABSTRACTION
// Define base class to support HW control of RGB state on various platforms:
// * RGBControllerMini
// * RGBControllerEyelock
// * RGBControllerNano
//
// For now use the existing 8 bit rgb state.

class RGBController
{
public:
	RGBController() {}
	virtual ~RGBController() {}
	virtual void SetRGB(unsigned char mask) = 0;
};

class RGBControllerMini : public RGBController
{
public:
	RGBControllerMini() {}
	virtual void SetRGB(unsigned char mask);
};

class RGBControllerPico : public RGBController
{
public:
	RGBControllerPico();
	virtual void SetRGB(unsigned char mask);
};
#endif

class LEDDispatcher: public ResultDispatcher {
public:
	LEDDispatcher(Configuration& conf);
	virtual ~LEDDispatcher();
	virtual bool enqueMsg(Copyable& msg);
	virtual int getQueueSize(Configuration& conf){ return 1;}
	LedState m_testState;
	const char *getName(){
		return "LEDDispatcher";
	}
	void SetDBUploadState();
	void SetInitialState();
	void SetLEDValue(unsigned char mask);
	void SetInitialValue(unsigned char mask) {m_initialState=mask;};
	virtual void ProcessOnEmptyQueue();
	uint8_t GetCurrentValue();
	void GetState(LedState& val,uint64_t& ts);
	virtual int End();
	char m_initialState;
protected:
	virtual void process(MatchResult *msg);
	void SetR();
	void SetG();
	void SetB();
	void UpdateState(LedState val);

	int m_LEDSwitchTimemSec,m_silencePeriodAfterMatchMs,m_LEDDetectSwitchTimemSec,m_silencePeriodAfterLoiteringorConfusion,m_periodForNWSet;
	int m_flashPeriodAfterMatchMs, m_flashPeriodAfterFailMs;
	char m_DetectState, m_matchState, m_noMatchState,m_dbLoadState,m_confusionState,m_loiteringState, m_dualAuthCardState, m_tamperState;
	// Duration after green when no blue will be accepted
	int m_ledDebug;
	Mutex m_LedLock;
	uint64_t m_TimeStampOfLastState;
	bool m_indicatorBoardSupport;
	int m_flashColorAfterMatch1,m_flashColorAfterMatch2,m_flashColorAfterFail1,m_flashColorAfterFail2;
	int m_flashCountAfterMatch, m_flashCountAfterFail;
#if DO_RGB_ABSTRACTION
public:
	void SetRGBController(RGBController *pRGBController) { m_pRGBController = pRGBController; }
protected:
	RGBController *m_pRGBController;
#endif
	uint8_t m_currentval;
private:
	bool CheckStateToUpdate();
};

#endif /* LEDDISPATCHER_H_ */
