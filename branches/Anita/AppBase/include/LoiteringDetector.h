/*
 * LoiteringDetector.h
 *
 *  Created on: Oct 11, 2012
 *      Author: mamigo
 */

#ifndef LOITERINGDETECTOR_H_
#define LOITERINGDETECTOR_H_
#include <list>
#include "Synchronization.h"
#include "HTTPPOSTMsg.h"
#include "GenericProcessor.h"
#include "socket.h"
#include "IrisData.h"

enum LOITERING_FLUSHED
{
	FLUSH_UNKNOWN=0,
	WITHIN_SILENT_TIME=1,
	WITHIN_STIPULATED_TIME=2,
	PERIODIC_CLEANING=3
};

class SocketFactory;
class Configuration;
class F2FDispatcher;
class LEDConsolidator;
class NwLEDDispatcher;
class LoiteringDetector: public GenericProcessor {
public:
	LoiteringDetector(Configuration& conf);
	virtual void init(){};
	virtual ~LoiteringDetector();
	virtual unsigned int MainLoop();
    const virtual char *getName(){return "LoiteringDetector";}
    virtual bool enqueMsg(Copyable & msg);
    bool ClearVector();
    void CheckLoitering(void);
    void PrintData();
    bool GetMasterLoiteringStatus(){ return m_sendmsgMaster;}
    bool GetSlaveLoiteringStatus(){ return m_sendmsgSlave;}
    void SetNwLedDispatcher(NwLEDDispatcher *ptr){m_nwLedDispatcher = ptr;}
    void SetLEDConsolidator(LEDConsolidator *ptr){m_LedConsolidator= ptr;}
    LOITERING_FLUSHED& FlushedHow(){return m_flushState;};
protected:
    void SendMessage();
    int getQueueSize(Configuration *conf){return 0;}
    Copyable *createNewQueueItem(){return NULL;}
    virtual Copyable *getNextMsgToProcess(){return NULL;}
    virtual void process(Copyable *msg){}
    bool DetectLoitering(vector<IrisData>& irisVector,BinMessage& outMsg);
    int64_t m_timeDiffence;
     HostAddress *m_resultDestAddr;
    SocketFactory *m_socketFactory;
    struct timeval m_timeOut;
    const char *m_msgFormat;
    BinMessage m_outMasterMsg,m_outSlaveMsg;
    int m_sleepTimeuSec;
    string m_CamStr;
    Mutex m_vectorLock;
    vector<IrisData> m_irisVectorMaster;
    vector<IrisData> m_irisVectorSlave;
    bool m_sendmsgMaster,m_sendmsgSlave,m_Debug;
    MatchResult m_f2fResult;
    int m_flushingTime;
    uint64_t m_matchedTimeStamp,m_silentTime;
    LOITERING_FLUSHED m_flushState;
    bool m_tsDestAddrpresent;
    NwLEDDispatcher *m_nwLedDispatcher;
    LEDConsolidator *m_LedConsolidator;

private:
    void ClearVectors(bool & flushdone);
};

#endif /* LOITERINGDETECTOR_H_ */
