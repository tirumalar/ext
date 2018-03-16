/*
 * MatchDispatcher.h
 *
 *  Created on: Nov 29, 2012
 *      Author: mamigo
 */

#ifndef MATCHDISPATCHER_H_
#define MATCHDISPATCHER_H_

#include "Synchronization.h"
#include "ResultDispatcher.h"
#include <vector>
using namespace std;

class F2FDispatcher;
class NwMatchManager;
class MatchResult;
class SocketFactory;
class HTTPPOSTMsg;
class HostAddress;
class NwLEDDispatcher;
class LEDConsolidator;
class SDKDispatcher;

class MatchDispatcher: public ResultDispatcher {
public:
	MatchDispatcher(Configuration& conf);
	virtual ~MatchDispatcher();

	virtual bool enqueMsg(Copyable& msg);
	const char *getName(){
			return "MatchDispatcher";
		}

    void SetF2FDispatcher(F2FDispatcher *ptr){m_f2fDispatcher = ptr;}
    MatchResult *getMatchResult(){ return &m_mr;}
    void SetNwMatchMgr(NwMatchManager* ptr){m_nwMatchMgr = ptr;}
    void SetLEDConsolidator(LEDConsolidator *ptr){ m_ledConsolidator = ptr;}
    void SetEyelockThread(ProcessorChain *ptr){m_imageProcessor = ptr;}
	virtual unsigned int MainLoop();
    void ProcessMatchedItem();
    void SetDualEyeMatch(bool set){m_dualMatcherPolicy = set;}
    void MakeNwF2FMsg(MatchResult* mr,BinMessage *msg);
    void SetNwLEDDispatcher(NwLEDDispatcher *ptr){m_nwLedDispatcher = ptr;}
    void SetSDKDispatcher(SDKDispatcher *ptr){m_sdkDispatcher = ptr;}
protected:

	virtual int getQueueSize(Configuration* conf);
	virtual Copyable *createNewQueueItem();

private:
    F2FDispatcher *m_f2fDispatcher;
    LEDConsolidator *m_ledConsolidator;
    SDKDispatcher *m_sdkDispatcher;
	HostAddress *m_statusDestAddr;
	HostAddress *m_weigandDestAddr;
	char *m_clientAddr;
	HTTPPOSTMsg *m_ResetMsg;
	uint64_t m_TimeofLastAuthorization,m_RepeatAuthorizationPeriod;
	bool m_checkUID;
	std::string m_prevUID;
	int m_LastAuthorizationID;
	bool m_logging;
	MatchResult m_mr;
	NwMatchManager *m_nwMatchMgr;
    bool m_Debug;
    struct timeval m_timeOutSend;
    ProcessorChain *m_imageProcessor;
    bool m_dualMatcherPolicy;
    std::vector<MatchResult> m_matchedItems;
    Mutex m_matchLock;
    uint64_t m_dualEyeMatchThresholdTime;//in micro second
    uint64_t m_matchDispSleepusec;//in micro second
    bool m_tsDestAddrpresent;
    NwLEDDispatcher *m_nwLedDispatcher;


    void SendResetMsg();
    void SendF2FMsg(MatchResult *mr);
    void MatchDetected(MatchResult *result,int frindx = -1,int eyeindx=-1);
    bool EnforceDualMatchPolicy();
    bool GetOneResult(MatchResult& res);
    void ClearMatchedItems();
    void SendToMaster(MatchResult *mr);

	struct timeval m_timeOut;
	HostAddress *m_resultDestAddr;
	SocketFactory *m_socketFactory;
};

#endif /* MATCHDISPATCHER_H_ */
