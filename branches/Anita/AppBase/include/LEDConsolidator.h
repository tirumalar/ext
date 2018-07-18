/*
 * LEDConsolidator.h
 *
 *  Created on: Jan 25, 2013
 *      Author: mamigo
 */

#ifndef LEDCONSOLIDATOR_H_
#define LEDCONSOLIDATOR_H_
#include <list>
#include "Synchronization.h"
#include "HTTPPOSTMsg.h"
#include "GenericProcessor.h"
#include "socket.h"
#include "CommonDefs.h"

class Configuration;
class LEDDispatcher;
class NwLEDDispatcher;
class F2FDispatcher;

class LEDConsolidator: public GenericProcessor {
public:
	LEDConsolidator(Configuration& conf);
	virtual ~LEDConsolidator();
	virtual void init(){};
	virtual unsigned int MainLoop();
    const virtual char *getName(){return "LEDConsolidator";}
    virtual bool enqueMsg(Copyable & msg);
    void ConsolidateState(void);
    LedState GetState(){ return m_State;}
    LedState GetPrevState(){ return m_PrevState;}
    void DispatchToAll(void);
    void SetLedDispatcher(LEDDispatcher *ptr){m_ledDispatcher= ptr;}
    void SetF2FDispatcher(F2FDispatcher *ptr){m_f2fDispatcher= ptr;}
    void SetNwLedDispatcher(NwLEDDispatcher *ptr){m_nwLedDispatcher= ptr;}
    LEDDispatcher* GetLedDispatcher(void){return m_ledDispatcher;}
    bool m_changeLedState;
protected:
    int getQueueSize(Configuration *conf){return 0;}
    Copyable *createNewQueueItem(){return NULL;}
    virtual Copyable *getNextMsgToProcess(){return NULL;}
	virtual void process(Copyable *msg){}
	Mutex m_vectorLock;
	vector<LEDResult> m_inputVector;
	int m_sleepTimeuSec;
	bool m_debug;
	LedState m_State,m_PrevState;
	LEDDispatcher *m_ledDispatcher;
	NwLEDDispatcher *m_nwLedDispatcher;
	F2FDispatcher *m_f2fDispatcher;
	MatchResult m_mr,m_loiteringResult,m_confusionResult;
	bool m_tsDestAddrpresent;
	int m_sleepinTSMasterBeforeLED;
	SoftwareType m_softwareType;
};

#endif /* LEDCONSOLIDATOR_H_ */
