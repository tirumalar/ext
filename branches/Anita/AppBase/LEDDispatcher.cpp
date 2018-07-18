/*
 * LEDDispatcher.cpp
 *
 *  Created on: 17-Sep-2009
 *      Author: mamigo
 */

#include "LEDDispatcher.h"
#include "Configuration.h"
#include "MatchType.h"
#include <iostream>
extern "C" {
#include "i2c.h"
#include "file_manip.h"
}
#include "logging.h"

const char logger[30] = "LEDDispatcher";

#define I2C_DEVICE      "/dev/i2c-0"
#define LED_DEVICE_ADDRESS  (0x54>>1)

#define LEDINDICATOR_DEVICE_ADDRESS  (0x1A)

using namespace std;
#if DO_RGB_ABSTRACTION
void RGBControllerMini::SetRGB(unsigned char mask) {
}
RGBControllerPico::RGBControllerPico() {
	i2cWriteByteOnByteAddres(I2C_DEVICE, LED_DEVICE_ADDRESS, 0x9, 0x5);
}

void RGBControllerPico::SetRGB(unsigned char mask) {
	i2cWriteByteOnByteAddres(I2C_DEVICE, LED_DEVICE_ADDRESS, 0x1, 0x20);
	i2cWriteByteOnByteAddres(I2C_DEVICE, LED_DEVICE_ADDRESS, 0x5, mask);
	i2cWriteByteOnByteAddres(I2C_DEVICE, LED_DEVICE_ADDRESS, 0x1, 0x21);
}

#endif

LEDDispatcher::LEDDispatcher(Configuration& conf) :m_currentval(0),
		ResultDispatcher(conf), m_indicatorBoardSupport(false), m_periodForNWSet(
				0) {

	/*
	 * Pin		15  13  12  11  10  9       Def
	 * Use		Rel R   E   G   Z   B
	 * Init     0   1   1   1   0   1       0x1D
	 * Detect   0   0   1   0   0   1       0x09
	 * Match    1   0   1   1   1   0       0x2E
	 * Fail     0   1   1   0   0   0       0x18
	 */
	m_dbLoadState = (char) conf.getValue("GRI.LEDDBLoad", 0x14);	//changed from 0x18 red
	m_initialState = (char) conf.getValue("GRI.LEDInitial", 0x1D);
	m_DetectState = (char) conf.getValue("GRI.LEDDetect", 0x09);
	m_matchState = (char) conf.getValue("GRI.LEDMatch", 0x2E);
	m_loiteringState = (char) conf.getValue("GRI.LEDLoitering", 0x1B);
	m_confusionState = (char) conf.getValue("GRI.LEDConfusion", 0x1C);
	m_tamperState = (char) conf.getValue("GRI.LEDTamper", 0x11);
	m_dualAuthCardState = (char) conf.getValue("GRI.LEDDualAuthN", 55);

	m_noMatchState = (char) conf.getValue("GRI.LEDNoMatch", 0x18);

#if DO_RGB_ABSTRACTION
	m_pRGBController = 0; // Must initialize before SetState() is called
#endif

	m_queueFullBehaviour = OVERWRIE_OLD;
	m_LEDSwitchTimemSec = conf.getValue("GRI.LEDSwitchTimeMilliSec", 500);
	m_LEDDetectSwitchTimemSec = conf.getValue("GRI.LEDDetectSwitchTimeMilliSec",
			1500);
	m_silencePeriodAfterMatchMs = conf.getValue("GRI.SilencePeriodAfterMatchMs",
			2000); //2sec
	m_silencePeriodAfterLoiteringorConfusion = conf.getValue(
			"GRI.SilencePeriodAfterLoiteringorConfusion", 2000); //2sec

	m_flashPeriodAfterMatchMs = conf.getValue("GRI.FlashPeriodAfterMatchMs",250); 	// 250 ms
	m_flashColorAfterMatch1 = conf.getValue("GRI.FlashColorAfterMatch1",55); 		// white
	m_flashColorAfterMatch2 = conf.getValue("GRI.FlashColorAfterMatch2",0); 		// off
	m_flashCountAfterMatch = conf.getValue("GRI.FlashCountAfterMatch",10); 			// 5 flash
	m_flashPeriodAfterFailMs = conf.getValue("GRI.FlashPeriodAfterFailMs",250); 	// 250 ms
	m_flashColorAfterFail1 = conf.getValue("GRI.FlashColorAfterFail1",24); 			// red
#ifdef LED_FLASH
	m_flashColorAfterFail2 = conf.getValue("GRI.FlashColorAfterFail2",0); 			// off
	m_flashCountAfterFail = conf.getValue("GRI.FlashCountAfterFail",10); 			// 5 flash
	m_initialState = 4;																// green
	m_silencePeriodAfterMatchMs = m_flashPeriodAfterMatchMs * (m_flashCountAfterMatch+2);				// increase to 3 sec
	m_silencePeriodAfterLoiteringorConfusion = m_flashPeriodAfterFailMs * (m_flashCountAfterFail+2);	// increase to 3 sec
#else 
	m_flashColorAfterFail2 = m_initialState; 										// m_initial
	m_flashCountAfterFail = conf.getValue("GRI.FlashCountAfterFail",6); 			// 3 flash
#endif

	// TODO: refactor.
	// Code duplication, same code: LEDDispatcher.cpp, NwMatchManager.cpp, OSDPMessage.cpp, F2FDispatcher.cpp
	// use one function from EyelockConfiguration for all?
	// get rid of bool value?
	bool dualAuthN = false;
	int authenticationMode = conf.getValue("Eyelock.AuthenticationMode",0);
	if (authenticationMode) {
		switch (authenticationMode)
		{
			case CARD_OR_IRIS:
				break;
			case CARD_AND_IRIS:
				dualAuthN = true;
				break;
			case CARD_AND_IRIS_PIN_PASS:
				dualAuthN = true;
				break;
			case PIN_AND_IRIS:
			case PIN_AND_IRIS_DURESS:
				dualAuthN = true;
				break;
			case CARD_AND_PIN_AND_IRIS:
			case CARD_AND_PIN_AND_IRIS_DURESS:
				dualAuthN = true;
				break;
			default:
				dualAuthN = false;
				break;
		}
	}
	else {
		dualAuthN = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	}

	bool m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);
	if (dualAuthN || m_transTOC)
		m_initialState = 0;
	SetInitialState();

	m_ledDebug = conf.getValue("GRI.LEDDebug", 0);
	m_indicatorBoardSupport = conf.getValue("Eyelock.LEDIndicatorBoardSupport",
			false);
}

LEDDispatcher::~LEDDispatcher() {
	SetLEDValue(0);
}
void LEDDispatcher::SetR() {
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xfb,
			0xffff); //Led1
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xdf,
			0xffff); //Led2
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfeff); //Led3
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xdfff); //Led4
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfffb); //Led5
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xffdf); //Led6
}
void LEDDispatcher::SetG() {
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xf7,
			0xffff); //Led1
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0x7f,
			0xffff); //Led2
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfdff); //Led3
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xbfff); //Led4
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfffd); //Led5
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xffef); //Led6
}

void LEDDispatcher::SetB() {
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xef,
			0xffff); //Led1
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xbf,
			0xffff); //Led2
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfbff); //Led3
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0x7fff); //Led4
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfffe); //Led5
	i2cWrite2ByteToByteAddress(I2C_DEVICE, LEDINDICATOR_DEVICE_ADDRESS, 0xff,
			0xfff7); //Led6
}

void LEDDispatcher::SetLEDValue(unsigned char mask) {
	printf("****** enter SetLEDValue *****\n");
#if DO_RGB_ABSTRACTION
	if (m_pRGBController)
		m_pRGBController->SetRGB(mask);

	if (m_indicatorBoardSupport) {
		printf("****** enter SetLEDValue:m_indicatorBoardSupport *****\n");
		if (mask == m_initialState) {
			//Set Red
			SetR();
			SetG();
			SetB();
		} else if (mask == m_DetectState) {
			//Set Blue
			SetB();
		} else if (mask == m_matchState) {
			//Set Green
			SetG();
		} else if (mask == m_dbLoadState) {
			//Set Green
			SetR();
		}
	}
#endif

}

void LEDDispatcher::UpdateState(LedState val) {
	ScopeLock lock(m_LedLock);
	CURR_TV_AS_MSEC(test);
	m_TimeStampOfLastState = test;
	m_testState = val;
}

void LEDDispatcher::GetState(LedState& val, uint64_t& ts) {
	ScopeLock lock(m_LedLock);
	ts = m_TimeStampOfLastState;
	val = m_testState;
}
uint8_t LEDDispatcher::GetCurrentValue(){
	return m_currentval;
}

void LEDDispatcher::SetDBUploadState() {
	if (m_ledDebug)
		EyelockLog(logger, DEBUG, "Setting DbReload LED");
	SetLEDValue(m_dbLoadState);
	UpdateState(LED_DBRELOAD);
}

void LEDDispatcher::SetInitialState() {
	if (m_ledDebug)
		EyelockLog(logger, DEBUG, "Setting Initial LED");
	SetLEDValue(m_initialState);
	UpdateState(LED_INITIAL);
}

bool LEDDispatcher::CheckStateToUpdate() {
	LedState state;
	uint64_t lastts;
	GetState(state, lastts);
	CURR_TV_AS_MSEC(timecurr);
	bool overwriteit = true;
	if (state == LED_PASSED || state == LED_PASSED_FLASH) {
		if ((lastts + m_silencePeriodAfterMatchMs) < timecurr) {
			overwriteit = true;
		} else
			overwriteit = false;
	} else if ((state == LED_LOITERING) || (state == LED_CONFUSION)|| (state == LED_CONFUSION_FLASH)) {
		if ((lastts + m_silencePeriodAfterLoiteringorConfusion) < timecurr) {
			overwriteit = true;
		} else
			overwriteit = false;
	}

	return overwriteit;
}

bool LEDDispatcher::enqueMsg(Copyable& msg) {
	if (m_ledDebug)
		EyelockLog(logger, TRACE, "enqueMsg() start");
	Safe<MatchResult *> & currMsg = m_inQueue.getCurr();
	bool wrote = false;
	bool changed = false;
	currMsg.lock();
	MatchResult *qmsg = currMsg.get();
	MatchResult& mr = (MatchResult&) msg;

	LedState lstate = (LedState)mr.getState();
	if (currMsg.isUpdated()) {
		if ((lstate == LED_PASSED)||(lstate == LED_PASSED_FLASH)||(lstate == LED_CONFUSION)||(lstate == LED_CONFUSION_FLASH)) {
			if (m_ledDebug)
				EyelockLog(logger, DEBUG,
						"LEDDispatcher::Overwriting last unserviced LED request due to high priority PASSED/CONFUSION_FLASH message");
			currMsg.get()->CopyFrom(msg);
			wrote = true;
		} else {
			if ((lstate == LED_DETECT) && (((LedState)qmsg->getState()) != LED_PASSED)) {
				bool overwriteit = CheckStateToUpdate();
				if (overwriteit) {
					if (m_ledDebug)
						EyelockLog(logger, DEBUG,
								"LEDDispatcher::Overwriting last unserviced LED request due to high priority DETECT message");
					currMsg.get()->CopyFrom(msg);
					wrote = true;
				} else {
					if (m_ledDebug)
						EyelockLog(logger, DEBUG,
								"Discarding LED MSG %d on account of MatchState",
								mr.getState());
				}
			} else {
				if (m_ledDebug) {
					EyelockLog(logger, WARN, "Discarding LED MSG %d on account of Q full",
							mr.getState());
				}
			}
		}
	} else {
		bool overwriteit = CheckStateToUpdate();

		if (overwriteit || (lstate == LED_PASSED) || (lstate == LED_CONFUSION_FLASH)) {
			if (m_ledDebug)
				EyelockLog(logger, DEBUG, "Writing Msg to LED Queue %d", mr.getState());
			// just normally write the new message
			currMsg.get()->CopyFrom(msg);
			wrote = true;
			changed = true;
			currMsg.setUpdated(true);
		}
	}
	m_inQueue++;
	currMsg.unlock();
	// if we changed the count
	if (changed) {
		m_inQueue.incrCounter();
	}
	// if we wrote something lets inform the others who may be interested
	if (wrote) {
		dataAvailable();
	}
	return true;
}

// go over the queue and set image messages update=false

void LEDDispatcher::ProcessOnEmptyQueue() {
//	if(m_ledDebug) printf("LEDDispatcher::ProcessOnEmptyQueue();\n"); fflush(stdout);
	CURR_TV_AS_MSEC(timecurr);
	//m_TimeStampOfLastState
	LedState state;
	uint64_t lastts;
	GetState(state, lastts);
	bool initialstate = false;

	if (state == PASSED) {
		if ((lastts + m_silencePeriodAfterMatchMs) < timecurr) {
			initialstate = true;
		}
	} else if (state == LED_PASSED_FLASH) {
		static int pass_count=1;
		if ((lastts + (m_flashPeriodAfterMatchMs * pass_count)) < timecurr) {
			if (pass_count < m_flashCountAfterMatch){
				if (pass_count % 2)
					SetLEDValue(m_flashColorAfterMatch2);		// LED off
				else
					SetLEDValue(m_flashColorAfterMatch1);		// LED white
				pass_count++;
			}
			else {
				initialstate = true;
				pass_count = 1;
			}
		}
	} else if (state == LED_DETECT) {
		if ((lastts + m_LEDDetectSwitchTimemSec) < timecurr) {
			initialstate = true;
		}
	} else if (state == LED_LOITERING) {
		if ((lastts + m_silencePeriodAfterLoiteringorConfusion) < timecurr) {
			initialstate = true;
		}
	} else if (state == LED_CONFUSION) {
		if ((lastts + m_silencePeriodAfterLoiteringorConfusion) < timecurr) {
			initialstate = true;
		}
	} else if (state == LED_CONFUSION_FLASH) {
		static int count=1;
		if ((lastts + (m_flashPeriodAfterFailMs * count)) < timecurr) {
			if (count < m_flashCountAfterFail){
				if (count % 2)
					SetLEDValue(m_flashColorAfterFail2);	// LED off
				else
					SetLEDValue(m_flashColorAfterFail1);	// LED red
				count++;
			}
			else {
				initialstate = true;
				count = 1;
			}
		}
	} else if (state == LED_DBRELOAD) {
		initialstate = false;
	} else if (state == LED_NWSET) {
		if ((lastts + m_periodForNWSet) < timecurr) {
			initialstate = true;
		}
	} else if (state == LED_DUAL_AUTHN_CARD) {
		initialstate = false;
	} else if (state == LED_TAMPER) {
		initialstate = false;
	} else if (state == LED_INITIAL) {
		//initialstate=true;
	} else {
		if ((lastts + m_LEDSwitchTimemSec) < timecurr) {
			initialstate = true;
		}
	}
	if (initialstate && (state != LED_INITIAL))
		SetInitialState();
	//if (m_ledDebug)
		//printf(" LEDDispatcher::%d %llu %llu \n", state, lastts, timecurr);
}

void LEDDispatcher::process(MatchResult *msg) {
	if(m_ledDebug)
		EyelockLog(logger, TRACE, "process() start, state %d\n",msg->getState());
	try {
		char val = m_initialState;
		LedState state = (LedState)msg->getState();

		switch (state) {
		case LED_PASSED:
			val = m_matchState;
			break;
		case LED_PASSED_FLASH:
			val = m_flashColorAfterMatch1;
			break;
		case LED_CONFUSION_FLASH:
			val = m_flashColorAfterFail1;
			break;
		case LED_DETECT:
			val = m_DetectState;
			break;
		case LED_LOITERING:
			val = m_loiteringState;
			break;
		case LED_CONFUSION:
			val = m_confusionState;
			break;
		case LED_TAMPER:
			val = m_tamperState;
			break;
		case LED_NWSET:
			int value, sleeptime;
			msg->getNwValandSleep(value, sleeptime);
			val = value;
			m_periodForNWSet = sleeptime / 1000;
			if (m_ledDebug)
				EyelockLog(logger, DEBUG, "Nw SleepTime %d", sleeptime);
			break;
		case LED_INITIAL:
			val = m_initialState;
			break;
		case LED_DBRELOAD:
			val = m_dbLoadState;
			break;
		case LED_DUAL_AUTHN_CARD:
			val = m_dualAuthCardState;
			break;
		default:
			return;
		}
		SetLEDValue(val);
		UpdateState(state);
		m_currentval = (uint8_t)val;
		if (m_ledDebug)
			EyelockLog(logger, DEBUG, "LEDDispatcher::LED State %llu::%#0x",
					m_TimeStampOfLastState, m_testState);
	} catch (const char *msg) {
		cout << msg << endl;
	} catch (exception ex) {
		cout << ex.what() << endl;
	}
}
int LEDDispatcher::End()
{
	EyelockLog(logger, DEBUG, "LEDDispatcher::End()"); fflush(stdout);
	SetLEDValue(0);
	HThread::End();
}
