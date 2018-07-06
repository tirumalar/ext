/*
 * LEDConsolidator.cpp
 *
 *  Created on: Jan 25, 2013
 *      Author: mamigo
 */
#include "Configuration.h"
#include "LEDConsolidator.h"
#include "LEDDispatcher.h"
#include "NwLEDDispatcher.h"
#include "F2FDispatcher.h"
#include "UtilityFunctions.h"
#include "CommonDefs.h"
#include "logging.h"
#include <iostream>
#include <vector>
#include <unistd.h>
extern "C"{
#include "file_manip.h"
}

const char logger[30] = "LEDConsolidator";
LEDConsolidator::LEDConsolidator(Configuration& conf):GenericProcessor(conf),m_State(LED_FAILED),m_PrevState(LED_FAILED),m_ledDispatcher(NULL),
	m_f2fDispatcher(NULL),m_nwLedDispatcher(NULL){
	m_sleepTimeuSec = conf.getValue("Eyelock.LEDConsolidatorSleepMilliSec",250);
	m_sleepTimeuSec = m_sleepTimeuSec*1000;
	m_debug = conf.getValue("Eyelock.LEDConsolidatorDebug",false);
	m_mr.setState(FAILED);
	m_sleepinTSMasterBeforeLED = conf.getValue("Eyelock.SleeptimeBeforeLEDMilliSec",0);
	m_sleepinTSMasterBeforeLED = m_sleepinTSMasterBeforeLED*1000;

	m_softwareType = (SoftwareType)conf.getValueIndex("Eyelock.Type", ePC, eNTSGFS, eNANO, "PC", "NANO", "PICO","NTSGLM","NTSGLS","NTSGFM","NTSGFS");

	const char *svrAdd = conf.getValue("Eyelock.TSMasterDestAddr", "NONE");
	if(strcmp(svrAdd, "NONE") == 0){
    	m_tsDestAddrpresent = false;
	}else
		m_tsDestAddrpresent = true;
    HTTPPOSTMsg f2fMsg(256);
	const char *str =  conf.getValue("Eyelock.LoiteringCardData","0x0000");
	MakeF2FMsg(str,LOITERING,f2fMsg);
	m_loiteringResult.init();
	m_loiteringResult.setState(LOITERING);
	int type;
	char *ptr = f2fMsg.getF2F(type);
	EyelockLog(logger, DEBUG, "F2FLoiteringMsg::Update F2F %.*s\n",30,ptr);
	for(int i=0;i<f2fMsg.GetSize();i++ ){
			printf("%02x ",ptr[i]);
	}
	printf("\n");
	if(ptr){
		m_loiteringResult.setF2F(ptr);
	}

	HTTPPOSTMsg confusionMsg(256);
	const char *str1 =  conf.getValue("Eyelock.ConfusionCardData","0x0000");
	MakeF2FMsg(str1 ,CONFUSION,confusionMsg);
	m_confusionResult.init();
	m_confusionResult.setState(CONFUSION);
	char *ptr1 = confusionMsg.getF2F(type);
	EyelockLog(logger, DEBUG, "F2FConfusionMsg::Update F2F %.*s\n",30,ptr1);
	for(int i=0;i<confusionMsg.GetSize();i++ ){
			printf("%02x ",ptr1[i]);
	}
	printf("\n");
	if(ptr1){
		m_confusionResult.setF2F(ptr1);
	}

	m_changeLedState = true;
}

LEDConsolidator::~LEDConsolidator() {
	// TODO Auto-generated destructor stub
}

bool LEDConsolidator::enqueMsg(Copyable& msg) {
	LEDResult& mr = (LEDResult&) msg;
	if(m_debug)EyelockLog(logger, DEBUG, "LEDConsolidator::enqueMsg %d \n",mr.getState());

	if(m_softwareType != eNTSGLM){
		MatchResult res;
		res.setState((MatchResultState)mr.getState());
		if (m_changeLedState){
			if(m_ledDispatcher)
				m_ledDispatcher->enqueMsg(res);
		}

		if(mr.getGeneratedState()==eLOCALGEN){
			if(m_nwLedDispatcher){
				m_nwLedDispatcher->enqueMsg(mr);
			}
		}
	}else{
		MatchResult res;
		if(((mr.getState() == LED_DETECT))|| (mr.getState()== LED_PASSED)|| (mr.getState()== LED_CONFUSION_FLASH)){
			res.setState((MatchResultState)mr.getState());
			if (m_changeLedState){
				if(m_ledDispatcher)m_ledDispatcher->enqueMsg(res);
			}
		}
		ScopeLock lock(m_vectorLock);
		m_inputVector.push_back(mr);
	}

	return true;
}

void LEDConsolidator::ConsolidateState(){
	ScopeLock lock(m_vectorLock);
	int indexDetect=-1,indexMatch =-1,indexLoitering =-1,indexConf =-1,indexInit=-1,indexDbRel= -1,indexNwSet=-1,indexDualAuthN=-1,indexConfFlash =-1,indexTamper=-1,indexMatchFlash=-1;
	if(m_debug)
		if(m_inputVector.size())EyelockLog(logger, DEBUG, "LEDConsolidator Size  %d\n",m_inputVector.size());
	for(int i=0;i<m_inputVector.size();i++){
		if(LED_DETECT == m_inputVector[i].getState()){
			indexDetect = i;
		}
		if(LED_PASSED == m_inputVector[i].getState()){
			indexMatch = i;
		}
		if(LED_LOITERING == m_inputVector[i].getState()){
			indexLoitering = i;
		}
		if(LED_CONFUSION == m_inputVector[i].getState()){
			indexConf = i;
		}
		if(LED_DBRELOAD == m_inputVector[i].getState()){
			indexDbRel = i;
		}
		if(LED_INITIAL == m_inputVector[i].getState()){
			indexInit = i;
		}
		if(LED_DUAL_AUTHN_CARD == m_inputVector[i].getState()){
			indexDualAuthN = i;
		}
		if(LED_CONFUSION_FLASH == m_inputVector[i].getState()){
			indexConfFlash = i;
		}
		if(LED_PASSED_FLASH == m_inputVector[i].getState()){
			indexMatchFlash = i;
		}
		if(LED_TAMPER == m_inputVector[i].getState()){
			indexTamper = i;
		}
		if(LED_NWSET == m_inputVector[i].getState()){
			indexNwSet = i;
		}
	}

	if(indexTamper > -1)
		m_State = LED_TAMPER;
	else if(indexDbRel > -1)
		m_State = LED_DBRELOAD;
	else if(indexMatch > -1)
		m_State = LED_PASSED;
	else if(indexMatchFlash > -1)
		m_State = LED_PASSED_FLASH;
	else if(indexConfFlash > -1)
		m_State = LED_CONFUSION_FLASH;
	else if(indexLoitering > -1)
		m_State = LED_LOITERING;
	else if(indexConf > -1)
		m_State = LED_CONFUSION;
	else if(indexNwSet > -1)
		m_State = LED_NWSET;
	else if(indexDetect > -1)
		m_State = LED_DETECT;
	else if(indexInit > -1)
		m_State = LED_INITIAL;
	else if(indexDualAuthN > -1)
		m_State = LED_DUAL_AUTHN_CARD;
	else
		m_State = LED_FAILED;

	if(m_State == LED_NWSET){
		int val,sleeptime;
		m_inputVector[indexNwSet].getNwValandSleep(val,sleeptime);
		if(m_debug)EyelockLog(logger, DEBUG, "NWSET -> %d %d\n",val,sleeptime);
		m_mr.setNwValandSleep(val,sleeptime);
	}


	if(m_debug)if(m_inputVector.size())EyelockLog(logger, DEBUG, "State %d %d\n",m_State,m_PrevState);
	m_inputVector.clear();
}

void LEDConsolidator::DispatchToAll(){
	if(m_State != m_PrevState){
		m_mr.setState((MatchResultState)m_State);
		if(!m_tsDestAddrpresent){
			if(m_debug)EyelockLog(logger, DEBUG, "LEDConsolidator :: NWLEDDisp %#0x F2FDISP %#0x -> %0d\n",m_nwLedDispatcher,m_f2fDispatcher,m_State);
			//Send the consolidated result as this is TS Master to TS slaves
			if(m_nwLedDispatcher){
				LEDResult l;
				l.setState(m_State);
				m_nwLedDispatcher->enqueMsg(l);
			}
			if(m_f2fDispatcher){
				char* ptr= NULL;
				int len = -1;
				int bits = -1;
				if(m_State == LOITERING){
					ptr = m_loiteringResult.getF2F(len,bits);
					if(m_debug) EyelockLog(logger, DEBUG, "Sending %d on F2F For LOITERING\n",bits);
					if(bits)m_f2fDispatcher->enqueMsg(m_loiteringResult);
					//m_loiteringResult.printF2FData();
				}
//				if(m_State == CONFUSION){
//					ptr = m_confusionResult.getF2F(len,bits);
//					if(m_debug) EyelockLog(logger, DEBUG, "Sending %d on F2F For CONFUSION\n",bits);
//					if(bits)m_f2fDispatcher->enqueMsg(m_confusionResult);
//				}
			}
			usleep(m_sleepinTSMasterBeforeLED);
		}
		CURR_TV_AS_MSEC(timet);
		if(m_debug)
			EyelockLog(logger, DEBUG, "%llu::LEDConsolidator State %d %#x, %d\n",timet,m_State,m_f2fDispatcher, m_changeLedState);
		if(m_ledDispatcher && (m_changeLedState || m_State == LED_NWSET)){
			m_ledDispatcher->enqueMsg(m_mr);
		}
		m_PrevState = LED_FAILED;
		if(m_State == LED_PASSED)
			callNext(m_mr);
	}
}


unsigned int LEDConsolidator::MainLoop() {

	std::string name = "LEDConsolidator::";
	try {
		while (!ShouldIQuit()) {
			ConsolidateState();
			DispatchToAll();
			Frequency();
			usleep(m_sleepTimeuSec);
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}
	return 0;
}
