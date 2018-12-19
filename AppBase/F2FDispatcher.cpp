/*
 * F2FDispatcher.cpp
 *
 *  Created on: 26-Oct-2016
 *      Author: fjia
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include "F2FDispatcher.h"
#include "Configuration.h"
#include "MatchedSignal.h"
#include "F2FSignal.h"
#include "WeignadSignal.h"
#include "MatchType.h"
#include "DualFactor.h"
#include "PortableTemplate.h"
#include "UtilityFunctions.h"
#include "SocketFactory.h"
#include "CommonDefs.h"
#include "LEDConsolidator.h"
#include "LEDDispatcher.h"
#include "logging.h"
#include "SDKDispatcher.h"
#include "OSDPMessage.h"
#include "time.h"
extern "C" {
#include "include/BobListener.h"
#include "sqlite3.h"
#include "file_manip.h"
#include <unistd.h>
}

using namespace std;

const char logger[30] = "F2FDispatcher";
extern char * getACSTestData(int & bytes, int & bitlen);


// Relay iff wiegand
F2FDispatcher::F2FDispatcher(Configuration& conf):ResultDispatcher(conf), m_pMatched(0), m_RelayTimeInMs(RELAY_TIME_IN_MS),m_Debug(false),m_ledConsolidator(NULL),
		m_socketFactory(NULL),m_socketFactoryTamper(NULL),m_tamperDestAddr(NULL),m_resultDestAddr(NULL), m_pMatchType(NULL), m_pinNumberRcvd(0)
{
#ifdef DEBUG_SESSION
	m_DebugTesting = conf.getValue("Eyelock.TestSystemPerformance",false);
	m_sessionDir = string(conf.getValue("Eyelock.DebugSessionDir","DebugSessions/Session"));
#endif
	m_SendEveryNSec = conf.getValue("Eyelock.SystemReadyInitialState", true);
	m_PreviousSendTS =0;
	m_testCode = false;
	m_f2f = conf.getValue("GRITrigger.F2FEnable",false);
	m_wiegand = conf.getValue("GRITrigger.WeigandEnable",false);
	m_pac = conf.getValue("GRITrigger.PACEnable",false);
	bool WigHID = conf.getValue("GRITrigger.WeigandHIDEnable",false);
	m_RelayTimeInMs = conf.getValue("GRITrigger.RelayTimeInMs", RELAY_TIME_IN_MS);
	m_osdpACSEnabled = conf.getValue("GRITrigger.OSDPEnable",false);
	m_osdpReaderEnable = conf.getValue("GRITrigger.OSDPInputEnable",false);
	isTamperSet = false;
	isMBTamper = false;
	isBoBTamper = false;
	isLedSet = 0;
	isSoundSet = false;

	if(m_f2f){
		EyelockLog(logger,  DEBUG, "Instantiating :F2F Signal");
		m_pMatched = new F2FSignal(conf);
		m_State = eF2FSTART;
	}else if(m_wiegand || m_pac || WigHID){
		//Came in constructor and not F2F means it has to be Weigand
		EyelockLog(logger,  DEBUG, "Instantiating :Weigand Signal");
		m_pMatched = new WeignadSignal(conf);
		m_State = eF2FSTART;
	} else {
		EyelockLog(logger,  WARN, "Invalid configuration w.r.t enabling of F2F or Weigand");
		throw("Invalid config for F2F or Weigand");
	}

	const char *str =  conf.getValue("Eyelock.SystemReadyCardData","0x0000");
	HTTPPOSTMsg healthMsg(256);
	MakeF2FMsg(str,HEALTH,healthMsg);
	int val;
	char *ptr = healthMsg.getF2F(val);
	m_f2fResult.init();
	m_f2fResult.setF2F(ptr);
	m_f2fResult.setState(HEALTH);
	m_NSec = conf.getValue("Eyelock.SystemReadyFrequencySec",0);
	m_SleepTime = conf.getValue("Eyelock.TimeDelayBetweenMsgMilliSec",200);
#if 0
	m_SleepTime = m_SleepTime*1000;//microsec
#else
	m_SleepTime = 10000; //microsec
#endif
	m_Debug = conf.getValue("Eyelock.SystemReadyDebug", false);
	int timeOutms = conf.getValue("Eyelock.WeiganSocketTimeOutMillis", 200);
	m_timeOutSend.tv_sec = timeOutms / 1000;
	m_timeOutSend.tv_usec = (timeOutms % 1000) * 1000;

	const char *svrAddr = conf.getValue("Eyelock.WeigandDestAddr", "NONE");
	if(strcmp(svrAddr,"NONE")!= 0){
		m_resultDestAddr = HostAddress::MakeHost(svrAddr, eIPv4, false);
		m_socketFactory = new SocketFactory(conf);
	}

	m_sendMatchToBOB = conf.getValue("Eyelock.SendMatchToBOB",true);
	m_sendLoteringToBOB = conf.getValue("Eyelock.SendLoiteringToBOB",true);
	m_sendHealthToBOB = conf.getValue("Eyelock.SendHealthToBOB",true);
	m_sendConfusionToBOB = conf.getValue("Eyelock.SendConfusionToBOB",true);
	m_dbReloadToBOB = conf.getValue("Eyelock.DBReloadToBOB",true);

	m_sendMatchToNw = conf.getValue("Eyelock.SendMatchToNw",true);
	m_sendLoteringToNw = conf.getValue("Eyelock.SendLoiteringToNw",true);
	m_sendHealthToNw = conf.getValue("Eyelock.SendHealthToNw",true);
	m_sendConfusionToNw = conf.getValue("Eyelock.SendConfusionToNw",true);
	m_dbReloadToNw = conf.getValue("Eyelock.DBReloadToNw",true);
	m_initialState = (char) conf.getValue("GRI.LEDInitial", 0x1D);
	m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);

	// authentication mode
	m_passThrough = false;
	m_dualAuth = false;
	m_pinAuth = false;

	// TODO: refactor.
	// Code duplication, same code: LEDDispatcher.cpp, NwMatchManager.cpp, OSDPMessage.cpp, F2FDispatcher.cpp
	// use one function from EyelockConfiguration for all?
	// get rid of bool value?
	m_authMode = conf.getValue("Eyelock.AuthenticationMode",0);

	if (m_authMode){
		switch (m_authMode)
		{
			case CARD_OR_IRIS:
				m_passThrough = true;
				break;
			case CARD_AND_IRIS:
				m_dualAuth = true;
				break;
			case CARD_AND_IRIS_PIN_PASS:
				m_dualAuth = true;
				break;
			case PIN_AND_IRIS:
			case PIN_AND_IRIS_DURESS:
				m_dualAuth = true;
				m_pinAuth = true;
				break;
			case CARD_AND_PIN_AND_IRIS:
			case CARD_AND_PIN_AND_IRIS_DURESS:
				m_dualAuth = true;
				m_multiAuth = true;
				break;
			default:
				m_passThrough = false;
				m_dualAuth = false;
				m_pinAuth = false;
				break;
		}
	}
	else {
		m_passThrough = conf.getValue("GRITrigger.PassThroughMode",false);
		m_dualAuth = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	}

	m_dualAuthWaitIrisTime = conf.getValue("GRITrigger.DualAuthNCardMatchWaitIrisTime", 10000);
	m_ledControlledByInput = conf.getValue("GRITrigger.DualAuthNLedControlledByACS", false);
	m_DenyRelayTimeInMs = conf.getValue("GRITrigger.DenyRelayTimeInMs", 5000);
	m_RelayWithSignal = conf.getValue("GRITrigger.EnableRelayWithSignal", false);
	m_silencePeriodAfterMatchMs = conf.getValue("GRI.SilencePeriodAfterMatchMs", 2000); //2sec
	bool m_disablebob = conf.getValue("Eyelock.BOBDisable",false); //2sec
	if (m_dualAuth || m_transTOC)
		m_initialState = 0;
	if (m_pac || m_f2f)
		m_accessDataLength = 96;
	else { // Wiegand
		m_accessDataLength = 26;
		int bytelen, bitlen;
		char * testData = getACSTestData(bytelen, bitlen);
		memcpy(m_testData, testData, bytelen+2);
		if (bitlen > 26){
			m_accessDataLength = bitlen;
		}
		if(testData) delete [] testData;
	}
	m_cardRead = CARD_INIT;
	m_acsChange = false;
	m_tamperChange = false;
	m_pollACS = false;
	m_temperature = false;
	m_updateACD = true;


	// Register BOB callback for status change
	BobSetInputCB(BoBStatusChangeCB);
	//BobSetMyCardStat((void *)&m_cardRead);
	BobSetMyPtr(this);

	// tamper
	m_tamperEnable = conf.getValue("Eyelock.TamperEnable", true);
	if (!m_tamperEnable) {
		if (access("tamper", F_OK ) != -1) {
			// system("rm /home/root/tamper");
			if( remove( "tamper" ) != 0 )
				EyelockLog(logger, ERROR, "Failed to remove tamper file");
			else
				EyelockLog(logger, DEBUG, "tamper file removed");
		}
	}
	else {
		if (access("tamper", F_OK ) != -1) {
			if (m_Debug)
				EyelockLog(logger, DEBUG, "tamper file existing");
			isTamperSet = true;
			isBoBTamper = true;
		}
	}

	m_tamperBitHighToLow = conf.getValue("Eyelock.TamperSignalHighToLow", true);
	m_tamperOutBitHighToLow = conf.getValue("Eyelock.TamperOutSignalHighToLow", false);
	// const char *tamperAddr = conf.getValue("Eyelock.TamperNotifyAddr", "192.168.1.136:8901");
	const char *tamperAddr = conf.getValue("Eyelock.TamperNotifyAddr", "");
	if(strcmp(tamperAddr,"")!= 0){
		m_tamperDestAddr = HostAddress::MakeHost(tamperAddr, eIPv4, false);
		m_socketFactoryTamper = new SocketFactory(conf);
	}
	else {
		m_tamperDestAddr = NULL;
		m_socketFactoryTamper = NULL;
	}
	const char *tmp	= conf.getValue("Eyelock.TamperNotifyMessage", "Tamper!");
	strcpy(m_tamperMsg, tmp);
	// sleep(1); // Anita

	if (m_osdpACSEnabled || m_osdpReaderEnable) {
		m_osdpMessage = new OSDPMessage(conf);
		m_osdpMessage->m_f2fDispatcher = this;
		if (m_dualAuth)
			m_osdpReaderEnable = true;
		else
			m_osdpReaderEnable = false;
	}



	if (m_transTOC) {
		m_passThrough = false;
		m_dualAuth = false;
		m_pMatchType = new PortableTemplate(conf);
	}
	else if (m_dualAuth) {
		m_pMatchType = new DualFactor(conf);
	}
	else {
		m_pMatchType = new MatchType(conf);
	}

	if (m_pinAuth || m_multiAuth) {
		m_pinBurstBits = conf.getValue("Eyelock.PinBurstBits", 4);
		m_pinWaitTime = conf.getValue("Eyelock.WaitPinTime", 10000);
	}
	else {
		m_pinBurstBits = 0;
	}

	m_pMatchType->m_dualAuthMatched = NOT_MATCHED;
	m_pMatchType->m_accessDataLength = m_accessDataLength;
	m_pMatchType->m_authMode = m_authMode;
	m_pMatchType->m_pinAuth = m_pinAuth;
	m_pMatchType->m_pinBurstBits = m_pinBurstBits;


	if(!m_disablebob){
		// Start the BOB Listener
		EyelockLog(logger,  DEBUG, "Start BOB Listener");
		BobInitComs();
		usleep(10000);

		// set configuration in BoB
		SendToBoB(conf);
	}

	pthread_mutex_init(&locateDeviceLock,NULL);

}

int F2FDispatcher::BoBOSDPCallback(void* my_data, int data_length)
{
	//EyelockLog(logger,  DEBUG, "BoBODSPCallback()");
	int result = 0;

	F2FDispatcher * myptr = (F2FDispatcher * )BobGetMyPtr();
	if(myptr == NULL)
		return result;

	result = myptr->m_osdpMessage->ProcessPanelMsg((unsigned char *)my_data, data_length);
	return result;
}



int F2FDispatcher::BoBReaderOSDPCallback(void* my_data, int data_length)
{
	//EyelockLog(logger,  DEBUG, "BoBReaderOSDPCallback()");
	bool result = false;

	F2FDispatcher * myptr = (F2FDispatcher * )BobGetMyPtr();
	if(myptr == NULL)
		return result;

	result = myptr->m_osdpMessage->ProcessReaderMsg((unsigned char *)my_data, data_length);
	return result;
}

void F2FDispatcher::BoBStatusChangeCB()
{
	F2FDispatcher * myptr = (F2FDispatcher * )BobGetMyPtr();
	if (myptr) {
		//if (myptr->m_Debug)
			//EyelockLog(logger, DEBUG, "BoBStatusChangeCB()");

		if (BoBStatusChangeAcs()) {
			//if (myptr->m_Debug)
				//EyelockLog(logger, DEBUG, "F2FDispatcher - ACS Input change");
			myptr->m_acsChange = true;
		}

		if (BoBStatusChangeTamper()) {
			//if (myptr->m_Debug)
				//EyelockLog(logger, DEBUG, "F2FDispatcher - Tamper Input change");
			myptr->m_tamperChange = true;
		}

		if (myptr->m_dualAuth || myptr->m_transTOC) {
			if (BoBStatusChangeCard()){
				if (myptr->m_Debug)
					EyelockLog(logger, DEBUG, "F2FDispatcher - Card Reader Ready");
				myptr->m_cardRead = CARD_READY;
			}
		}
	}

}

void F2FDispatcher::ProcessTamper(bool value, bool mbTamper)
{
	if (m_Debug)
		EyelockLog(logger, TRACE, "F2FDispatcher::ProcessTamper() start");

	bool tamper_value;

	if (mbTamper)
		isMBTamper = value;
	else
		isBoBTamper = value;
	tamper_value = (isMBTamper || isBoBTamper) ? 1 : 0;

	if (m_Debug)
		EyelockLog(logger, DEBUG, "tamper value new=%d, current=%d, value=%d, MB=%d", tamper_value, isTamperSet, value, mbTamper);

	if (isTamperSet != tamper_value)
	{
		isTamperSet = tamper_value;
		// set OSDP message
		if (m_osdpACSEnabled) {
			m_osdpMessage->osdpStateChange = true;
			m_osdpMessage->isTamperSet = isTamperSet;
			//osdpUpdateACSChanges();
		}
	//	int polFlag =
		 BoBSetACSTamperOut(m_tamperOutBitHighToLow,tamper_value); // hw loopback, no need to set tamper out here

		if (m_temperature && isTamperSet) {
			RunSystemCmd("aplay /home/root/tones/tamper1.wav");
			sleep(1);
			SendTamperMsg();
			return;
		}
		// set tamper LED
		if (m_ledConsolidator)
		{
			EyelockLog(logger, DEBUG, "set LED ");
			LEDResult l;
			if (isTamperSet)
				l.setState(LED_TAMPER);
			else
				l.setState(LED_INITIAL);
			m_ledConsolidator->enqueMsg(l);
		}

		// set tamper state for sound
		MatchResult msg;
		if (isTamperSet)
			msg.setState(TAMPER);
		else
			msg.setState(INITIAL);
		callNext(msg);

		SendTamperMsg();

		if (m_sdkDispatcher)
		{
			SDKCallbackMsg msg;
			if (isTamperSet)
				msg = SDKCallbackMsg(TAMPERSET, "");
			else
				msg = SDKCallbackMsg(TAMPERCLEAR, "");

			m_sdkDispatcher->enqueMsg(msg);
		}
	}
}
void F2FDispatcher::ProcessBoBAcsChange()
{
	//if (m_Debug)
		//EyelockLog(logger, TRACE, "F2FDispatcher::ProcessBoBAcsChange()");

	bool value = 0;
#if 0	// only use PushButton to reset
	if (BoBGetACSFactoryReset())
	{
		// factory reset
		EyelockLog(logger, DEBUG, "Factory Reset Start:");
		//xxlog("%s\n", "Factory Reset Start:");
		EyelockEvent("%s\n", "Factory Reset Start:");
		BoBClearACSFactoryReset();
		usleep(1000);
		RunSystemCmd("/home/root/scripts/factoryReset.sh");
		exit(0);
	}
#endif

	// Tamper
	if (m_tamperEnable && m_tamperChange) {
		value = BoBGetACSTamperIn(m_tamperBitHighToLow?1:0);
		if (value != isTamperSet) {
			BoBLogACSTamperIn();
			ProcessTamper(value, false);
		}
		m_tamperChange = false;
	}

	if (m_acsChange) {

		m_acsChange = false;

		int ledChangeValue = -1;
		if (m_ledControlledByInput || m_passThrough) {
			// LED input: none = 0, green = 1, red = 2
			value = BoBGetACSGreenLedIn();	// 0 or 1
			//EyelockLog(logger,  DEBUG, "@@@ Green LED value %d, %d", value, isLedSet);
			if (isLedSet != value)
			{
				EyelockLog(logger, DEBUG, "LED value new = %d, current = %d", value, isLedSet);
				isLedSet = value;
				ledChangeValue = isLedSet ? 4 : m_initialState;
				if (m_passThrough) {
					if (value)
						BoBSetACSLedGreenOut();
					else
						BoBClearACSLedGreenOut();
				}
			}
			else
			{
				value = BoBGetACSRedLedIn();	// 0 or 2
				EyelockLog(logger, DEBUG, "Red LED value new = %d, current = %d", value, isLedSet);
				if (isLedSet != value && !isLedSet)
				{
					isLedSet = value;
					ledChangeValue = isLedSet ? 24 : m_initialState;
					if (m_passThrough) {
						if (value)
							BoBSetACSLedRedOut();
						else
							BoBClearACSLedRedOut();
					}
				}
			}
		}

		if (m_ledConsolidator && ledChangeValue != -1 && m_ledControlledByInput)
		{
			LEDResult l;
			l.setState(LED_NWSET);
			l.setGeneratedState(eREMOTEGEN);
			l.setNwValandSleep(ledChangeValue,86400000);	// 120000 = 2min, 86400000 = 24h
			m_ledConsolidator->enqueMsg(l);
			if (m_Debug)
				EyelockLog(logger, DEBUG, "ledChangeValue %d", ledChangeValue);

			if (ledChangeValue == 4 || ledChangeValue == 24){	// green
				if (m_ledConsolidator->m_changeLedState) {
					m_ledConsolidator->m_changeLedState = false;
					BoBSetTimer(LEDTimeoutCB, m_silencePeriodAfterMatchMs, 0, this);	// 2 sec
				}
			}
			else if (ledChangeValue == m_initialState)
				m_ledConsolidator->m_changeLedState = true;

			LEDDispatcher* ledDispatcher = m_ledConsolidator->GetLedDispatcher();
			ledDispatcher->SetInitialValue(ledChangeValue);

		}

		// Sound input: none = 0, green = 1, red = 2
		value = BoBGetACSSoundIn();	// 0 or 1
		if (isSoundSet != value)
		{
			EyelockLog(logger, DEBUG, "Sound signal input %s", (value?"set":"clear"));
			isSoundSet = value;
			if (value)
				BoBSetACSSoundOut(0, 0);
			else
				BoBClearACSSoundOut();
		}
	}

	static int count = 0;
	if (m_pollACS == true && m_ledConsolidator && count++ >= 3) {
		m_pollACS = false;
		BobSetPollACS(m_pollACS);
		if (m_Debug)
			EyelockLog(logger, DEBUG, "polling ACS disabled, m_pollACS=%d", m_pollACS);

		// start osdp poll
		if (m_osdpReaderEnable && m_dualAuth)
			m_osdpMessage->SendOSDPCommand(osdp_ID, NULL);

		// Device is in process state
		if (m_dualAuth)
			EyelockLog(logger, INFO, "Ready for Card");
		else if (!m_transTOC)
			EyelockLog(logger, INFO, "Ready for Eyes");
		EyelockEvent("System Ready for Authentication");
		EyelockLog(logger, DEBUG, "Setting PIM LED white");
		unsigned char regs[1];
		regs[0] = 7;
		BobSetData(regs,1);
		BobSetCommand(BOB_COMMAND_SET_LED);
		// testSend(); // Anita
	}
}

bool F2FDispatcher::modifyACD(string acd, int acdlen, string acdnop, string new_acd, string new_acdnop, string pin)
{
	bool result = true;
	
	if(m_pMatchType)
		result = m_pMatchType->modifyACD(acd, acdlen, acdnop, new_acd, new_acdnop, pin);

	return result;
}

bool F2FDispatcher::addACD(string acd, int acdlen, string acdnop, string name, string pin)
{
	bool result = true;
	
	if(m_pMatchType)
		result = m_pMatchType->addACD(acd, acdlen, acdnop, name, pin);

	return result;
}

bool F2FDispatcher::loadACD()
{
	bool result = true;
	if (m_dualAuth) {
		if(m_pMatchType)
			result = m_pMatchType->loadACD();
	
		EyelockEvent("DB is updated, Ready for Authentication");
	}
	return result;
}






void F2FDispatcher::CardMatchTimeoutCB(void *pThread)
{
	//EyelockLog(logger, TRACE, "F2FDispatcher::CardMatchTimeoutCB() ");
	F2FDispatcher * myptr = (F2FDispatcher * )BobGetMyPtr();
	if (myptr)
		myptr->m_cardRead = CARD_TIMEOUT;
}

void F2FDispatcher::LEDTimeoutCB(void *pThread)
{
	EyelockLog(logger, TRACE, "F2FDispatcher::LEDTimeoutCB() ");
	F2FDispatcher * myptr = (F2FDispatcher * )BobGetMyPtr();
	if (myptr && myptr->m_ledConsolidator && myptr->m_ledControlledByInput) {
		myptr->m_ledConsolidator->m_changeLedState = true;
	}
}

void F2FDispatcher::SendToBoB(Configuration& conf)
{
	bool WigHID = conf.getValue("GRITrigger.WeigandHIDEnable",false);
	int acsType = 0;
	if (m_osdpACSEnabled)
		acsType = BOB_ACCESS_TYPE_OSDP;
	else if(m_f2f)
		acsType = BOB_ACCESS_TYPE_F2F;
	else if(m_pac)
		acsType = BOB_ACCESS_TYPE_PAC;
	else if(WigHID)
		acsType = BOB_ACCESS_TYPE_HID;
	else if(m_wiegand)
		acsType = BOB_ACCESS_TYPE_WIEGAND;
	else
		acsType = BOB_ACCESS_TYPE_WIEGAND;

	if (m_transTOC)
		acsType += BOB_ACCESS_TYPE_TOC_BASE;
	else if (m_passThrough)
		acsType += BOB_ACCESS_TYPE_PASS_BASE;
	// support multi-factor
	// else if (m_authMode == CARD_AND_IRIS_PIN_PASS)

	BobSetACSType(acsType);

	if (!m_pinAuth)
		BobSetACSTypeBits(m_accessDataLength);
	else
		BobSetACSTypeBits(m_pinBurstBits);

	if (m_dualAuth)
	{	// set dual authentication
		BobSetCommand(BOB_COMMAND_READ);
		// set to Red LED in init
		BoBSetACSLedRedOut();
	}
	else
		BobSetCommand(BOB_COMMAND_NONE);

	usleep(100);
	BoBSetACSTamperPol(m_tamperBitHighToLow, 1);
	BoBSetACSTamperPol(m_tamperOutBitHighToLow, 0);

	// OSDP
	if (m_osdpACSEnabled || m_osdpReaderEnable) {

		if (m_osdpACSEnabled) {
			m_OSDPBRate = conf.getValue("Eyelock.OSDPBaudRate", 9600);
			BobSetOsdpCB(BoBOSDPCallback);
			BobSetOSDPBaudRate(m_OSDPBRate, 0);
		}
		else
			BobSetOSDPBaudRate(0, 0);

		if (m_dualAuth && m_osdpReaderEnable) {
			m_ReaderOSDPBRate = conf.getValue("Eyelock.ReaderOSDPBaudRate", 9600);
			BobSetReaderOsdpCB(BoBReaderOSDPCallback);
			BobSetOSDPBaudRate(m_ReaderOSDPBRate, 1);
		}
		else {
			BobSetOSDPBaudRate(0, 1);
		}
	}

	bool ignoreButton = conf.getValue("Eyelock.IgnoreResetButton", false);
	if (ignoreButton)
	{
		BobSetPollResetButton(0);
	}

	if(conf.getValue("Eyelock.Logging", false)){
		FILE *fp = fopen("dump.txt","a");
		fprintf(fp,"\nSet the configuration parameters on BoB:\n ");
		fprintf(fp,"BOB_ACCESS_COMMAND (1) value [%d]\n ",BOB_COMMAND_READ);
		fclose(fp);
	}
}

void F2FDispatcher::CheckBoBEvents() {
	//EyelockLog(logger, DEBUG, "\nF2FDispatcher::CheckBoBEvents() - ACS/Tamper  ");
	static bool pollInit = 1;

	if (pollInit && m_ledConsolidator) {
		m_pollACS = true;
		BobSetPollACS(m_pollACS);
		pollInit = 0;
	}

	if (!ShouldIQuit()) {
		// Check ACS event
		if (m_acsChange || m_tamperChange) {
			if(m_Debug)
			{
				//EyelockLog(logger, DEBUG, "\nF2FDispatcher::CheckBoBEvents() - ACS/Tamper  ");
			}
			ProcessBoBAcsChange();
		}

		// Check Card event
		if (m_cardRead == CARD_READY || m_cardRead == CARD_TIMEOUT) {
			if(m_Debug && m_cardRead == CARD_READY)
				EyelockLog(logger, DEBUG, "\nF2FDispatcher::CheckBoBEvents() - CARD ");
			ProcessBoBCardReader();
		}

		// Check OSDP LED Command
		if (m_osdpACSEnabled && m_ledControlledByInput) {	// check osdp_LED command
			if(m_Debug)
			{
				//EyelockLog(logger, TRACE, "\nF2FDispatcher::CheckBoBEvents() - OSDP LED  ");
			}
			m_osdpMessage->ProcessOSDPLedCommandFromACS(m_ledConsolidator);
		}
	}
	else {
		BoBSetQuit();

	}
}

// Check card reader
void F2FDispatcher::ProcessBoBCardReader() {

	if (!m_pMatched || !m_pMatchType)
	{
		m_cardRead = CARD_INIT;
		BobSetCardReadAck();
		return; 	// Not support Dual Authentication in Relay
	}

	// EyelockLog(logger,  DEBUG, "@@@@@ Check the card on BoB: %d, %d ",m_ledConsolidator->GetState(), m_pMatched->m_dualAuthMatched);
	if (m_cardRead == CARD_READY)
	{
		printf("in ProcessBoBCardReader...\n");
		bool pinState = false;
		m_cardRead = CARD_INIT;
		int len = GetCardData();
		if (len <= 0 || (!m_transTOC && len < 26)) {
			if (len == m_pinBurstBits && m_pinNumberRcvd < MAX_PIN_BYTE_LENGTH) {
				BoBCancelTimer(CardMatchTimeoutCB);
				m_pinNumber[m_pinNumberRcvd++] = m_pMatchType->m_pCardData[0];
				unsigned char pinValue = m_pMatchType->m_pCardData[0];
				if (pinValue == 0x5A || pinValue == 0x4B || pinValue == 0xA0 || pinValue == 0xB0){
					// get all pin number
					memcpy(m_pMatchType->m_pCardData, m_pinNumber, m_pinNumberRcvd);
					m_pMatchType->m_cardLen = m_pinNumberRcvd;
					len = m_pinNumberRcvd;
					m_pinNumberRcvd = 0;
				}
				else {
					BoBSetTimer(CardMatchTimeoutCB, 5000, 0, this);	// 5 sec)
					len = -1;
				}
			}
			if (len <= 0) {
				SetDualTransCommand(BOB_COMMAND_READ);
				return;
			}
		}


		if (m_pMatchType->m_dualAuthMatched == NOT_MATCHED) {
			if (!m_pinAuth) {
				if (m_pMatchType->ValidateCard()) {
					if (m_multiAuth){
						m_pMatchType->m_dualAuthMatched = WAIT_FOR_PIN;
						//BobSetACSTypeBits(m_pinNumberLength);
						BobSetACSTypeBits(m_pinBurstBits);
						SetDualTransCommand(BOB_COMMAND_READ);
						BoBSetTimer(CardMatchTimeoutCB, m_pinWaitTime, 0, this);	// 10 sec
						// set reader LED
						BoBSetACSLedGreenOut();
						return;
					}
					else
						m_pMatchType->m_dualAuthMatched = CARD_MATCHED;
				}
			}
			else {
				if (m_pMatchType->ValidatePin()) {
					m_pMatchType->m_dualAuthMatched = CARD_MATCHED;
				}
				pinState = true;
			}
		}
		else if (m_pMatchType->m_dualAuthMatched == WAIT_FOR_PIN) {
			if (m_pMatchType->ValidatePin()) {
				m_pMatchType->m_dualAuthMatched = CARD_MATCHED;
				memcpy(m_pMatchType->m_pCardData, m_pMatchType->matchedCardData, 20);
			}
			else {
				m_pMatchType->m_dualAuthMatched = NOT_MATCHED;
				BoBClearACSLedGreenOut();
				BoBSetACSLedRedOut();
			}
			BobSetACSTypeBits(m_accessDataLength);
			pinState = true;
		}

		if (m_pMatchType->m_dualAuthMatched == CARD_MATCHED) {
			if (m_dualAuth) {
				SetLED(DUAL_AUTHN_CARD);
				BoBSetTimer(CardMatchTimeoutCB, m_dualAuthWaitIrisTime, 0, this);	// 10 sec
			}
			else if (m_transTOC) {
				SetDualTransCommand(BOB_COMMAND_READ);	// keep reading new card data
				 
				if (m_pMatchType->m_numOfCard == 1) {
					SetLED(DUAL_AUTHN_CARD);
					BoBSetTimer(CardMatchTimeoutCB, CHECK_TOC_TIME, 999999, this);	// 1 sec, repeat 999999
				}
			}

			if (m_osdpReaderEnable) {
				m_osdpMessage->m_osdpReader.ledState = CARD_MATCHED;
				m_osdpMessage->m_osdpReader.buzState = CARD_MATCHED;
			}
			else {
				BoBSetACSLedGreenOut();
				BoBSetACSSoundOut(50, 3);	// 50ms
			}
			
			if(m_Debug)
				EyelockLog(logger, DEBUG, "Card match m_dualAuthMatched=%d", m_pMatchType->m_dualAuthMatched);

			EyelockLog(logger, INFO, "Ready for Eyes");

		}
		else if (!m_pMatchType->CheckReaderData())
		{
			if (m_transTOC) {
				EyelockEvent("Match failure, invalid card - Invalid card data");
				SDKCallbackMsg msg(MATCH, "Match failure, invalid card - Invalid card data");
				m_sdkDispatcher->enqueMsg(msg);
			}
			else {
				char *cardID = NULL;
				char tmp[256];
				if (pinState){
					char username[NAME_SIZE];
					memset(username, 0, NAME_SIZE);
					if (m_multiAuth)
						m_pMatchType->getUserNameFromCard(m_pMatchType->matchedCardData, username);
					EyelockEvent("Match failure, invalid PIN number - %s Invalid PIN number", username);
					sprintf(tmp, "Match failure, invalid PIN number - %s Invalid PIN number", username);
				}
				else {
					cardID = m_pMatchType->PrintCardData(m_pMatchType->m_pCardData, (m_accessDataLength+7)/8);
					EyelockEvent("Match failure, invalid card - Invalid card ID: %s", cardID);
					sprintf(tmp, "Match failure, invalid card - Invalid card ID: %s", cardID);
				}
				SDKCallbackMsg msg(MATCH, tmp);
				m_sdkDispatcher->enqueMsg(msg);
				if (cardID) free(cardID);
				m_pMatchType->clearCardData();
			}

			if (m_osdpReaderEnable) {
				m_osdpMessage->m_osdpReader.buzState = NOT_MATCHED;
			}
			else {
				BoBSetACSLedRedOut();
				BoBSetACSSoundOut(250, 5);
			}

			if(m_Debug)
				EyelockLog(logger, DEBUG, "Card match m_dualAuthMatched=%d", m_pMatchType->m_dualAuthMatched);

			if (m_pac)
				usleep(300000);		// delay 300 ms
			SetDualTransCommand(BOB_COMMAND_READ);

		}
	}
	else if (m_cardRead == CARD_TIMEOUT)
	{
		if(m_Debug)
			EyelockLog(logger, DEBUG, "Card timeout m_dualAuthMatched=%d", m_pMatchType->m_dualAuthMatched);

		if (m_pMatchType->timeoutUser())
		{
			if (m_dualAuth && m_pMatchType->m_dualAuthMatched == CARD_MATCHED) {
				char username[NAME_SIZE];
				char tmp[256];
				m_pMatchType->getUserNameFromCard(m_pMatchType->m_pCardData, username);
				EyelockEvent("Match failure, no iris present - %s", username);
				sprintf(tmp, "Match failure, no iris present - %s", username);
				SDKCallbackMsg msg(MATCH, tmp);
				m_sdkDispatcher->enqueMsg(msg);
			}

			ResetReaderLED();
			// set LED off
			m_initialState = 0;
			SetLED(INITIAL);
		}

		if (m_pMatchType->m_dualAuthMatched == WAIT_FOR_PIN) {
			m_pMatchType->m_dualAuthMatched = NOT_MATCHED;
			ResetReaderLED();
			BobSetACSTypeBits(m_accessDataLength);
			char username[NAME_SIZE];
			char tmp[256];
			m_pMatchType->getUserNameFromCard(m_pMatchType->matchedCardData, username);
			if (m_pinNumberRcvd == 0) {
				EyelockEvent("Match failure, no PIN present - %s", username);
				sprintf(tmp, "Match failure, no PIN present - %s", username);
			}
			else {
				EyelockEvent("Match failure, invalid PIN number - %s", username);
				sprintf(tmp, "Match failure, invalid PIN number - %s", username);
			}
			SDKCallbackMsg msg(MATCH, tmp);
			m_sdkDispatcher->enqueMsg(msg);
			m_pinNumberRcvd = 0;
		}
		else if (m_pinAuth){
			m_pinNumberRcvd = 0;
			char tmp[256];
			EyelockEvent("Match failure, invalid PIN number -  Invalid PIN number");
			sprintf(tmp, "Match failure, invalid PIN number -  Invalid PIN number");
			SDKCallbackMsg msg(MATCH, tmp);
			m_sdkDispatcher->enqueMsg(msg);
		}
		m_cardRead = CARD_INIT;
		m_pMatchType->clearCardData();
	}

}

int F2FDispatcher::GetCardData() {
	if(m_Debug)
		EyelockLog(logger, TRACE, "F2FDispatcher::GetCardData() start  ");

	unsigned char *cardData = m_pMatchType->m_pCardData;
	int bytes = (m_accessDataLength+7)/8;
	int len = 0;

	memset(cardData, 0, 20);
	if (m_osdpReaderEnable) {
		memcpy(cardData, m_osdpMessage->osdpCardData, bytes);
		len = m_accessDataLength;
	}
	else {
		if (m_transTOC || m_pMatchType->m_dualAuthMatched == NOT_MATCHED || m_pMatchType->m_dualAuthMatched == WAIT_FOR_PIN)
		{
			len = BobGetData(cardData, 0);
			if (len == -1)
			{
				EyelockLog(logger, ERROR, "Read card data failed");
				return len;
			}
		}
		if (BobSetCardReadAck())
			EyelockLog(logger, ERROR, "Send card read ACK failed");

		if (m_f2f) {
			// filter the keep-alive message
			int bytes = (m_accessDataLength+7)/8;
			int i = 0;
			for (i = 4; i < bytes; i++) {
				if (cardData[i])
					break;
			}
			if (i == bytes) {
				memset(cardData, 0, bytes);
				return 0;
			}
		}
	}
	m_pMatchType->m_cardLen = len;
	if(m_Debug && m_dualAuth) {
		char *cardID = m_pMatchType->PrintCardData(m_pMatchType->m_pCardData, (len+7)/8);
		EyelockLog(logger, DEBUG, "Card data read from BoB: %s, len %d", cardID, (len+7)/8);
		if(cardID) free(cardID);
	}
	
	return len;
}


void F2FDispatcher::SendTamperMsg(){
	BinMessage msg(256);
	char* ptr = msg.GetBuffer();
	memset(ptr, 0, 256);
	if (isTamperSet)
	{
		//system("touch /home/root/tamper");
		FILE *fp = fopen("tamper","a");
		fclose(fp);
		sprintf(ptr, "%s %s", "TAMPER SET: ", m_tamperMsg);
	}
	else
	{
		// system("rm /home/root/tamper");
		if( remove( "tamper" ) != 0 )
			EyelockLog(logger, ERROR, "Failed to remove tamper file");
		else
			EyelockLog(logger, DEBUG, "tamper file removed");
		sprintf(ptr, "%s %s", "TAMPER CLEAR: ", m_tamperMsg);
	}

	SendAlarmMsg(ptr);
}

void F2FDispatcher::SendAlarmMsg(char *string){
	BinMessage msg(256);
	char* ptr = msg.GetBuffer();
	strcpy(ptr, string);

	EyelockEvent("%s", ptr);
	msg.SetSize(strlen(ptr));
	if(m_Debug) {
		EyelockLog(logger, DEBUG, "F2FDispatcher::SendAlarmMsg->%d::%.*s",msg.GetSize(),msg.GetSize(),msg.GetBuffer());
	}
	try{
		if(m_socketFactoryTamper && m_tamperDestAddr){
			SocketClient client = m_socketFactoryTamper->createSocketClient("Eyelock.TamperMsg");
			client.SetTimeouts(m_timeOutSend);
			client.ConnectByHostname(*m_tamperDestAddr);
			//client.SetTimeouts(m_timeOutSend);
			client.SendAll(msg,MSG_DONTWAIT);
		}
	}
	catch(Exception& nex)
	{
		EyelockLog(logger, ERROR, "F2FDispatcher::SendTamperMessage failed"); fflush(stdout);
		nex.PrintException();
	}
	catch(const char *msg)
	{
		EyelockLog(logger, ERROR, "F2FDispatcher::SendTamperMessage failed 2"); fflush(stdout);
		std::cout<< msg <<endl;
	}
	catch(...)
	{
		std::cout<< "Unknown exception during SendTamperMsg" <<endl;
	}
}

void F2FDispatcher::SetAccessDataLength(int len, const char *parityMask, const char *dataMask)
{
	int bytelen, bitlen;
	char * testData = getACSTestData(bytelen, bitlen);
	memcpy(m_testData, testData, bytelen+2);
	if(testData) delete [] testData;

	if (!m_wiegand)
		return;

	if (len != m_accessDataLength) {
		if (!m_pinAuth) {
			if (BobSetACSTypeBits(len))
			{
				EyelockLog(logger, ERROR, "Failed to set BOB m_accessDataLength %d", m_accessDataLength); fflush(stdout);
			}
		}
		m_accessDataLength = len;
	}

	if(m_dualAuth && m_wiegand && m_pMatchType)
		m_pMatchType->SetAccessDataMask(len, dataMask);

}
int F2FDispatcher::getQueueSize(Configuration* conf){
	int qsz = conf->getValue("Eyelock.F2F-WeigandQueueSize", 10);
	return qsz;
}


void F2FDispatcher::RestartDispatcher(){
	EyelockLog(logger, TRACE, "Restart F2F Trigger");
	if(m_pMatched && (m_State == eF2FSTOP)) {
		m_pMatched->RestartSignal();
		m_State = eF2FSTART;
	}
}
void F2FDispatcher::CloseDispatcher(){
	EyelockLog(logger, TRACE, "Close F2F Timer");
	if(m_pMatched)
	{
		m_State = eF2FSTOP;
		m_pMatched->CloseSignal();
	}
}

F2FDispatcher::~F2FDispatcher()
{
	if(m_resultDestAddr)
		delete m_resultDestAddr;
	if(m_socketFactory)
		delete m_socketFactory;
	if(m_tamperDestAddr)
		delete m_tamperDestAddr;
	if(m_socketFactoryTamper)
		delete m_socketFactoryTamper;


	BobCloseComs();

	if (m_osdpMessage)
		delete m_osdpMessage;
	if (m_pMatchType)
		delete m_pMatchType;

}

void F2FDispatcher::ProcessOnEmptyQueue(){
	if(m_SendEveryNSec){
		CURR_TV_AS_SEC(currsec);
		if(m_NSec > 0){
			if((currsec - m_PreviousSendTS) > m_NSec){
				//if(m_Debug) EyelockLog(logger, DEBUG, "Processing on Empty Q in F2FDispatcher::");
				process(&m_f2fResult);
				m_PreviousSendTS = currsec;
			}
		}
	}
	Frequency();
}

void F2FDispatcher::MakeNwF2FMsg(MatchResult* mr, BinMessage* msg) {
	char* ptr = msg->GetBuffer();
	memset(ptr, 0, 256);
	ptr[0] = 'F';
	ptr[1] = '2';
	ptr[2] = 'F';
	ptr[3] = ';';
	ptr[4] = mr->getState();
	ptr[5] = ';';
	int len = -1, bits = -1;
	char* key = mr->getF2F(len, bits);
	len = MAX(len,0);
	len = len + 2;
	memcpy(ptr + 6, mr->getKey(), len);
	ptr[6 +len] = ';';
	msg->SetSize(6 + len + 1);
}

void F2FDispatcher::SendMsg(MatchResult *mr){
	BinMessage msg(256);
	MakeNwF2FMsg(mr,&msg);
	if(m_Debug) {
		EyelockLog(logger, DEBUG, "F2FDispatcher->%d::%.*s",msg.GetSize(),msg.GetSize(),msg.GetBuffer());
	}
	try{
		if(m_socketFactory&&m_resultDestAddr){
			SocketClient client = m_socketFactory->createSocketClient("Eyelock.WeigandDispatcherSecure");
			client.SetTimeouts(m_timeOutSend);
			client.ConnectByHostname(*m_resultDestAddr);
			client.SetTimeouts(m_timeOutSend);
			client.SendAll(msg,MSG_DONTWAIT);
		}
	}
	catch(Exception& nex)
	{
		EyelockLog(logger, ERROR, "F2FDispatcher::SendMessage failed"); fflush(stdout);
		nex.PrintException();
	}
	catch(const char *msg)
	{
		std::cout<< msg <<endl;
	}
	catch(...)
	{
		std::cout<< "Unknown exception during SendF2FMsg" <<endl;
	}
}

bool F2FDispatcher::checkToBeSendToBOB( MatchResultState val){
	bool send = false;
	if(	m_sendMatchToBOB && (val == PASSED)){
		send = true;
	}else if(m_sendLoteringToBOB &&(val == LOITERING)){
		send = true;
	}else if(m_sendHealthToBOB && (val == HEALTH)){
		send = true;
	}else if(m_sendConfusionToBOB && (val == CONFUSION)){
		send = true;
	}else if(m_dbReloadToBOB && (val == DBRELOAD)){
		send = true;
	}
	return send;
}

bool F2FDispatcher::checkToBeSendToNw( MatchResultState val){
	bool send = false;
	if(	m_sendMatchToNw && (val == PASSED)){
		send = true;
	}else if(m_sendLoteringToNw &&(val == LOITERING)){
		send = true;
	}else if(m_sendHealthToNw && (val == HEALTH)){
		send = true;
	}else if(m_sendConfusionToNw && (val == CONFUSION)){
		send = true;
	}else if(m_dbReloadToNw && (val == DBRELOAD)){
		send = true;
	}
	return send;
}



void F2FDispatcher::SendData(MatchResult *msg)
{
	if(m_Debug)
	{
		EyelockLog(logger, TRACE, "F2FDispatcher::SendData() start");
	}

	if (m_pinAuth)
		return;

	int bytes, bitlen;
	char *ptr = msg->getF2F(bytes,bitlen);
	MatchResultState state = msg->getState();

	if (state == FAILED || state == CONFUSION)
	{
		msg->setF2F(m_testData);
	}

	if (m_osdpACSEnabled)
	{
		m_osdpMessage->SaveMatchData(ptr, bytes, bitlen);
	}
	else
	{
		m_pMatched->Send(msg->getKey());
	}
}

#define bcd2bin(x)	(((x) & 0x0f) + ((x) >> 4) * 10)
#define bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)
void F2FDispatcher::settime()
{
	EyelockLog(logger, TRACE, "Hardware clock sync");
	unsigned char regs[8];
	time_t rawtime;
	struct tm* tm1;

	printf("setting time...\n");
	time(&rawtime);
	tm1 = localtime(&rawtime);

	printf("setting to %s\n",asctime(tm1));

		regs[0] = 0;
		/* This will purposely overwrite REG_SECONDS_OS */
		regs[1] = bin2bcd(tm1->tm_sec);
		regs[2] = bin2bcd(tm1->tm_min);
		regs[3] = bin2bcd(tm1->tm_hour);
		regs[4] = bin2bcd(tm1->tm_mday);
		regs[5] = tm1->tm_wday;
		regs[6] = bin2bcd(tm1->tm_mon + 1);
		regs[7] = bin2bcd(tm1->tm_year - 100);
		printf("setting time to 0x%0x: 0x%0x: 0x%0x,0x%0x: 0x%0x,0x%0x: 0x%0x...\n",regs[1],regs[2],regs[3],regs[4],regs[5],regs[6],regs[7]);
		BobSetDataAndRunCommand(regs,8,BOB_COMMAND_RTCWRITE_CMD);
}
void setLED(int x)
{
	unsigned char regs[1];
	printf("setting LED to %d\n",x);
	regs[0] = x;
	BobSetData(regs,1);
}
void F2FDispatcher::gettime()
{
	EyelockLog(logger, TRACE, "Retrieving hardware time");
	unsigned char regs[8];
	time_t rawtime;
	struct tm* tm1,tm2;

	tm1 = &tm2;
	printf("getting time...\n");
	BobGetData(regs,8);


	printf("date reced for time: 0x%0x, 0x%0x, 0x%0x, 0x%0x\n",regs[0],regs[1],regs[2],regs[3]);
		tm1->tm_sec = bcd2bin(regs[1] & 0x7f);
		tm1->tm_min = bcd2bin(regs[2] & 0x7f);
		tm1->tm_hour = bcd2bin(regs[3] & 0x3f);
		tm1->tm_mday = bcd2bin(regs[4] & 0x3f);
		tm1->tm_wday = regs[5] & 0x7;
		tm1->tm_mon = bcd2bin(regs[6] & 0x1f) - 1;
		tm1->tm_year = bcd2bin(regs[7]) + 100;
		printf("get time : %s\n",asctime(tm1));


}

void F2FDispatcher::testSend(void)
{
	return;
	printf("sending test data\n");

	//settime();
	//BobSetCommand(BOB_COMMAND_RTCWRITE_CMD);
	//BobSetData(void *ptr, int len);
	sleep(1);
	BobSetCommand(BOB_COMMAND_RTCREAD_CMD);
	usleep(5000);
	gettime();

	sleep(5);
	BobSetCommand(BOB_COMMAND_RTCREAD_CMD);
	usleep(5000);
	gettime();
	//BobGetData(void *ptr, int len);

	for(int i=0;i<8 ;i++)
	{
		setLED(i);
		BobSetCommand(BOB_COMMAND_SET_LED);
		sleep(1);
	}

	BobSetCommand(BOB_COMMAND_OIM_OFF);

	sleep(2);

	BobSetCommand(BOB_COMMAND_OIM_ON);

	sleep(2);


	/*for(int i = 0;i<5;i++)
	{
		BoBSetACSLedGreenOut();
		usleep(200000);
		BoBClearACSLedGreenOut();
		usleep(200000);
		BoBSetACSLedRedOut();
		usleep(200000);
		BoBClearACSLedRedOut();
		usleep(200000);
		BoBSetACSSoundOut(250, 3);
		sleep(2);
		BoBClearACSSoundOut();
		sleep(1);
	}*/
	printf("tests done\n");

}

/*
void SendTestData(void)
{
	MatchResult *msg = new MatchResult;
	int bytelen, bitlen;
	char * testData = getACSTestData(bytelen, bitlen);
	memcpy(testData1, testData, bytelen+2);
	if (bitlen > 26){
		m_accessDataLength = bitlen;
	}
	msg->setF2F(testData1);
	m_pMatched->Send(msg->getKey());
}
*/

void F2FDispatcher::SetLED(MatchResultState state)
{
	if(!m_ledConsolidator)
		return;

	LEDResult l;

	if (m_dualAuth) {
		m_initialState = 0;
		m_ledConsolidator->GetLedDispatcher()->m_initialState = (char)m_initialState;
	}


	if (state == PASSED) {
		if (m_ledControlledByInput)
			return;
#ifdef LED_FLASH
		l.setState(LED_PASSED_FLASH);
#else
		l.setState(LED_PASSED);
#endif
	}
	else if (state == CONFUSION) {
		l.setState(LED_CONFUSION_FLASH); //Red flash
	}
	else if (state == FAILED) {
		l.setState(LED_CONFUSION); //Red solid
	}
	else if (state == INITIAL) {
		SetLedInitState(m_initialState);
		l.setState(LED_INITIAL);
	}
	else if (state == DUAL_AUTHN_CARD) {
		m_initialState = 55;	// set LED to white
		SetLedInitState(m_initialState);
		l.setState(LED_DUAL_AUTHN_CARD);
	}
	m_ledConsolidator->enqueMsg(l);
}

void F2FDispatcher::SetLedInitState(int initLed)
{
	m_ledConsolidator->GetLedDispatcher()->m_initialState = (char)initLed;
}

void F2FDispatcher::StartLocateDevice()
{
	EyelockLog(logger, DEBUG, "Start locating device");
	pthread_mutex_lock(&locateDeviceLock);
	stopLocateDevice = false;
	pthread_mutex_unlock(&locateDeviceLock);
	EyelockLog(logger, DEBUG, "StartLocateDevice: lock acquired");

	int rc = pthread_create(&locateDeviceThread, NULL, &F2FDispatcher::LocateDeviceLoop, this);
	if (rc != 0)
	{
		EyelockLog(logger, ERROR, "StartLocateDevice: cannot create thread (%d)", rc);
	}
	pthread_detach(locateDeviceThread);
}


void F2FDispatcher::StopLocateDevice()
{
	EyelockLog(logger, DEBUG, "Stop locating device");
	pthread_mutex_lock(&locateDeviceLock);
	stopLocateDevice = true;
	pthread_mutex_unlock(&locateDeviceLock);
	sleep(2);
	unsigned char color = 7;
	BobSetData(&color,1);
	BobSetCommand(BOB_COMMAND_SET_LED);
}


void* F2FDispatcher::LocateDeviceLoop(void *ptr)
{
	if (ptr == NULL)
	{
		EyelockLog(logger, ERROR, "Locate device loop: null pointer to F2FDispatcher object");
		pthread_exit(NULL);
	}

	F2FDispatcher *f2fDispatcherPtr = (F2FDispatcher *) ptr;

	EyelockLog(logger, DEBUG, "Locate device loop started");
	const int timeout = 2000;
	LEDResult l;
	l.setState(LED_NWSET);
	l.setGeneratedState(eREMOTEGEN);
	//std::vector<int> colors = {1,5,16};
	unsigned char color;
	while (1)
	{
		pthread_mutex_lock(&(f2fDispatcherPtr->locateDeviceLock));
		bool stopRequested = f2fDispatcherPtr->stopLocateDevice;
		pthread_mutex_unlock(&(f2fDispatcherPtr->locateDeviceLock));

		if (stopRequested)
		{
			EyelockLog(logger, DEBUG, "Locate device loop: stop requested");
			pthread_exit(NULL);
		}

		l.setNwValandSleep(1, timeout);
		f2fDispatcherPtr->m_ledConsolidator->enqueMsg(l);
		color = 1;
		BobSetData(&color,1);
		BobSetCommand(BOB_COMMAND_SET_LED);
		sleep((timeout-1000)/1000);

		l.setNwValandSleep(5, timeout);
		f2fDispatcherPtr->m_ledConsolidator->enqueMsg(l);
		color = 4;
		BobSetData(&color,1);
		BobSetCommand(BOB_COMMAND_SET_LED);
		sleep((timeout-1000)/1000);

		l.setNwValandSleep(16, timeout);
		f2fDispatcherPtr->m_ledConsolidator->enqueMsg(l);
		color = 2;
		BobSetData(&color,1);
		BobSetCommand(BOB_COMMAND_SET_LED);
		sleep((timeout-1000)/1000);
	}
}

void F2FDispatcher::SetRelay(MatchResultState state)
{
	if (state == PASSED) {
		if (m_RelayWithSignal && m_RelayTimeInMs) {
			usleep(1000);
			if (BoBSetACSRelayOut(1) == 0) {
				// set relay timer
				BoBSetRelayTimerNew(m_RelayTimeInMs, 1);
			}
		}
	}

	if (m_authMode == PIN_AND_IRIS_DURESS || m_authMode == CARD_AND_PIN_AND_IRIS_DURESS) {
		if (m_pMatchType->m_duress && state == PASSED) {
			usleep(1000);
			if (BoBSetACSRelayOut(2) == 0){
				// set relay timer
				BoBSetRelayTimerNew(m_DenyRelayTimeInMs, 2);
			}
		}
	}
	else {
		if (state == FAILED || state == CONFUSION) {
			if (m_RelayWithSignal && m_DenyRelayTimeInMs)
			{
				usleep(1000);
				if (BoBSetACSRelayOut(2) == 0){
					// set relay timer
					BoBSetRelayTimerNew(m_DenyRelayTimeInMs, 2);
				}
			}
		}
	}
}

void F2FDispatcher::ResetReaderLED()
{
	// set reader LED
	if (m_pMatchType->m_numOfCard == 0) {
		if (m_osdpReaderEnable) {
			m_osdpMessage->m_osdpReader.ledState = NOT_MATCHED;
		}
		else {
			BoBClearACSLedGreenOut();
			BoBSetACSLedRedOut();
		}
		// set read card command
		if (m_pMatchType->m_dualAuthMatched == CARD_MATCHED) {

			BoBCancelTimer(CardMatchTimeoutCB);
			m_pMatchType->m_dualAuthMatched = NOT_MATCHED;
		}
	}

	SetDualTransCommand(BOB_COMMAND_READ);	// set dual authentication
	//m_pMatchType->clearCardData();
}

void F2FDispatcher::LogMatchResult(MatchResult *msg)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "F2FDispatcher::LogMatchResult()");

	char tmp[256];
	char DebugSession[256];
	MatchResultState state = msg->getState();

#ifdef DEBUG_SESSION
	char time_str[100];
	char session_match_log[200];
	sprintf(session_match_log, "%s/Info.txt", m_sessionDir.c_str());
	FILE *file = NULL;
	struct timespec ts;
	if(m_DebugTesting){
		time_t timer;
		struct tm* tm1;
		time(&timer);
		tm1 = localtime(&timer);
		strftime(time_str, 100, "%Y %m %d %H:%M:%S", tm1);

		clock_gettime(CLOCK_REALTIME, &ts);
	}
#endif

	int frameNo,eyeIdx;
	string cam;
	int EXTCameraIndex;
	msg->getFrameInfo(frameNo,eyeIdx,cam, EXTCameraIndex);
	float MatchScore = msg->getScore();
	if (state == PASSED) {
			string name;
			name.assign(msg->getName());
			char *PersonName = strtok ((char*)name.c_str(),"|");
			if (m_pMatchType->m_duress && m_authMode >= PIN_AND_IRIS_DURESS) {
			EyelockLog(logger, DEBUG, "Match success ID is %s CameraId:%d FrameId:%d MatchScore:%f", msg->getName().c_str(), EXTCameraIndex, frameNo, MatchScore);
			EyelockEvent("Match success(Duress) ID is %s", msg->getName().c_str());
			sprintf(tmp, "Match success(Duress) ID is %s", msg->getName().c_str());
			sprintf(DebugSession, "Match success ID is %s CameraId:%d FrameId:%d MatchScore:%f", msg->getName().c_str(), EXTCameraIndex, frameNo, MatchScore);
		}
		else {
			EyelockLog(logger, DEBUG, "Match success ID is %s CameraId:%d FrameId:%d MatchScore:%f", PersonName, EXTCameraIndex, frameNo, MatchScore);
			EyelockEvent("Match success ID is %s  MatchScore:%f", PersonName, MatchScore);
			sprintf(tmp, "Match success ID is %s", PersonName);
			sprintf(DebugSession, "Match success ID is %s CameraId:%d FrameId:%d MatchScore:%f", PersonName, EXTCameraIndex, frameNo, MatchScore);
		}
		SDKCallbackMsg msg(MATCH, std::string(tmp));
		m_sdkDispatcher->enqueMsg(msg);
#ifdef DEBUG_SESSION
		if(m_DebugTesting){
			file = fopen(session_match_log, "a");
			if (file){
				fprintf(file, "%s %lu:%09lu Match_success[%s]\n", time_str, ts.tv_sec, ts.tv_nsec, DebugSession);
				fclose(file);
			}
		}
#endif
	}
	else if (state == CONFUSION) {
		string name;
		name.assign(msg->getName());
		char *PersonName = strtok ((char*)name.c_str(),"|");
		EyelockLog(logger, DEBUG, "Match failed %s CameraId:%d FrameId:%d MatchScore:%f", PersonName, EXTCameraIndex, frameNo, MatchScore);
		sprintf(DebugSession, "Match failed %s CameraId:%d FrameId:%d MatchScore:%f", PersonName, EXTCameraIndex, frameNo, MatchScore);
		EyelockEvent("Match failed");
		SDKCallbackMsg msg(MATCH, "Match failed");
		m_sdkDispatcher->enqueMsg(msg);
#ifdef DEBUG_SESSION
		if(m_DebugTesting){
			file = fopen(session_match_log, "a");
			if (file){
				fprintf(file, "%s %lu:%09lu Match_failed[%s]\n", time_str, ts.tv_sec, ts.tv_nsec, DebugSession);
				fclose(file);
			}
		}
#endif
	}
	else if (state == FAILED) {
		char username[NAME_SIZE];
		m_pMatchType->getUserNameFromCard(m_pMatchType->m_pCardData, username);
		EyelockLog(logger, DEBUG, "Match failure, iris mismatch - %s CameraId:%d FrameId:%d MatchScore:%f", username, EXTCameraIndex, frameNo, MatchScore);
		sprintf(DebugSession, "Match failure, iris mismatch - %s CameraId:%d FrameId:%d MatchScore:%f", username, EXTCameraIndex, frameNo, MatchScore);
		EyelockEvent("Match failure, iris mismatch - %s", username);
		sprintf(tmp, "Match failure, iris mismatch - %s", username);
		SDKCallbackMsg msg(MATCH, std::string(tmp));
		m_sdkDispatcher->enqueMsg(msg);
#ifdef DEBUG_SESSION
		if(m_DebugTesting){
			file = fopen(session_match_log, "a");
			if (file){
				fprintf(file, "%s %lu:%09lu Match_failed[%s]\n", time_str, ts.tv_sec, ts.tv_nsec, DebugSession);
				fclose(file);
			}
		}
#endif
	}
	else
	{
#ifdef DEBUG_SESSION
		if(m_DebugTesting){
			file = fopen(session_match_log, "a");
			if (file){
				fprintf(file, "%s %lu:%09lu Unknown_match_result\n", time_str, ts.tv_sec, ts.tv_nsec);
				fclose(file);
			}
		}
#endif
	}
}

void F2FDispatcher::osdpUpdateACSChanges()
{
	EyelockLog(logger, TRACE, "F2FDispatcher::osdpUpdateACSChanges()");
	if (m_osdpACSEnabled) {
		// call BoBOSDPCallback() directly 53-00-07-00-00-60-45
		char pollData[48];
		pollData[0] = 0x53;
		pollData[1] = 0x00;
		pollData[2] = 0x07;
		pollData[3] = 0x00;
		pollData[4] = 0x00;
		pollData[5] = 0x60;
		pollData[6] = 0x46;
		if (BoBOSDPCallback(pollData, 7))
			EyelockLog(logger, ERROR, "BoBOSDPCallback() error");
	}
}

void F2FDispatcher::process(MatchResult *msg)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "F2FDispatcher::process() start");

	try
	{
		MatchResultState state = msg->getState();
		EyelockLog(logger, DEBUG, "F2FDispatcher::process ==> state = %d, m_dualAuth=%d", state, m_dualAuth);
		if((state==PASSED) || (state== LOITERING) || (state== CONFUSION) || (state== DBRELOAD)||(state== HEALTH)){
			if(m_State == eF2FSTART) {
				if (m_Debug)
					EyelockLog(logger, DEBUG, "F2FDispatcher::process start ==> state = %d, m_dualAuth=%d, m_dualAuthMatched %d\n", state, m_dualAuth, m_pMatchType->m_dualAuthMatched);

				if (m_testCode) {
					m_pMatched->Send(msg->getKey());
					m_testCode = false;
					if (m_dualAuth || m_transTOC)
						SetDualTransCommand(BOB_COMMAND_READ);
					return;
				}

				m_pMatchType->UpdateMatchResult(msg);
				state = msg->getState();

				callNext(*msg);

				SendData(msg);
				SetLED(state);
				SetRelay(state);

				LogMatchResult(msg);
				if (m_dualAuth || m_transTOC) {
					ResetReaderLED();
					m_pMatchType->clearCardData();
				}

				if(m_Debug)
					msg->printF2FData();

				if(m_Debug)
					EyelockLog(logger, DEBUG, "F2FDispatcher::process() finished ==> state = %d, m_dualAuth=%d, m_dualAuthMatched %d\n", state, m_dualAuth, m_pMatchType->m_dualAuthMatched);

			} // loop end


			if(m_resultDestAddr && checkToBeSendToNw(state)){
				SendMsg(msg);
			}
		}
		usleep(m_SleepTime);
	}
	catch(const char *msg)
	{
		cout <<msg <<endl;
	}
	catch(exception ex)
	{
		cout <<ex.what()<<endl;
	}
}

bool F2FDispatcher::SetDualTransCommand(int command){
	int count = 600;
	bool status1 = true, status2 = true;

	count = 2000;
	while (count--) {	// 2 sec
		int value = BoBGetCommand();
		if (value == command)
			return true;
		if (value == 0){
			status1 = false;
			break;
		}
		usleep(1000);
	}
	if (count == 0) {
		EyelockLog(logger, ERROR, "BoB failed to clear command");
	}

	if (BobSetCommand(command)) {
		EyelockLog(logger, ERROR, "Failed to set BOB dual/TOC command "); fflush(stdout);
	}

	count = 2000;
	while (count--) {	// 2 sec
		if (BoBGetCommand() == command){
			status2 = false;
			break;
		}
		usleep(1000);
	}
	if (count == 0) {
		BobSetCommand(command);
		EyelockLog(logger, ERROR, "Failed to read BOB dual/TOC command, reset again");
	}

	if(status1 || status2) {
		EyelockLog(logger, ERROR, "Failed to set BOB dual/TOC command: not cleared %d, not set %d ", status1, status2);
		return true;
	}

	return false;
}

void F2FDispatcher::SetMatchManager(MatchManagerInterface *ptr)
{
	if (m_pMatchType) {
		m_pMatchType->m_matchManager = ptr;
		m_pMatchType->initMatchBuffer();

		if (ptr && m_dualAuth) {
			m_pMatchType->m_dbAdapter = ptr->GetDbAdapter();
			loadACD();
			if(m_wiegand) {
				m_pMatchType->GetAccessDataMask();
			}

		}
	}

}

bool F2FDispatcher::addUser(string perid, string name, string acd)
{
	bool result = false;
	if (m_pMatchType)
		result = m_pMatchType->addUser(perid, name, acd);
	return result;
}

int F2FDispatcher::End()
{
	EyelockLog(logger, DEBUG, "F2FDispatcher::End()"); fflush(stdout);
	BoBSetQuit();
	BobCloseComs();
}



