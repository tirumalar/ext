/*
 * NwMatchManager.h
 *
 *  Created on: Aug 12, 2011
 *      Author: developer1
 */

#ifndef NWMATCHMANAGER_H_
#define NWMATCHMANAGER_H_

#include <GenericProcessor.h>
#include "IrisData.h"
#include "Synchronization.h"
#include <list>
class BiOmega;
class IrisDBHeader;
class MatchManagerInterface;
class MatchResult;
class LEDDispatcher;
class LoiteringDetector;
class F2FDispatcher;
class SocketFactory;
class MatchDispatcher;

using namespace std;

enum CHECK_IRIS {CHECK_IRIS_NONE=1,CHECK_IRIS_LEFT=2,CHECK_IRIS_RIGHT=4,CHECK_IRIS_BOTH=8};
enum UT_IRIS_STATE
{
	UT_IRIS_NONE=0,
	UT_IRIS_MATCH_L=1,
	UT_IRIS_MATCH_R=2,
	UT_IRIS_NO_MATCH=3,
	UT_IRIS_ILLUM_L=4,
	UT_IRIS_ILLUM_R=5,
	UT_IRIS_RAD_L=6,
	UT_IRIS_RAD_R=7,
	UT_IRIS_MATCH_PREV_L=8,
	UT_IRIS_MATCH_PREV_R=9,
	UT_IRIS_CENTROID_L=10,
	UT_IRIS_CENTROID_R=11,
	UT_IRIS_MATCH_SUCCESS=12
};


struct IRIS_LIST
{
	IRIS_LIST()
	{
		m_iris.clear();
		m_matchedIris = NULL;
		m_prevTrackedIris = NULL;
		m_matchedResult = NULL;
	}

	~IRIS_LIST()
	{
		FlushIris();
	}

	void FlushIris()
	{
		std::list<IrisData*>::iterator iter = m_iris.begin();
		for(;iter!=m_iris.end();iter++)
		{
			delete (*iter);
		}

		m_iris.clear();
		m_matchedIris = NULL;
		m_prevTrackedIris = NULL;

		if(m_matchedResult)
			delete m_matchedResult;

		m_matchedResult = NULL;
	}

	void FlushAllButOne()
	{
		if(m_iris.size() <=1)
			return;
		else
		{
			IrisData* iris = NULL;
			MatchResult* result = NULL;
			if(m_matchedIris == m_iris.back())
			{
				iris = new IrisData(*m_matchedIris);
				result = new MatchResult(*m_matchedResult);

				FlushIris();
				m_iris.push_back(iris);
				m_matchedIris = iris;
				m_matchedResult = result;
			}
			else
			{
				iris = new IrisData(*m_iris.back());
				FlushIris();
				m_iris.push_back(iris);
			}
		}


	}

	void RefreshList()
	{
		std::list<IrisData*>::iterator iter = m_iris.begin();
		if(m_matchedIris == m_iris.front())
		{
			delete m_matchedIris;
			delete m_matchedResult;
			m_matchedIris = NULL;
			m_matchedResult = NULL;
			m_prevTrackedIris = NULL;
		}
		else
		{
			delete m_iris.front();
		}
		m_iris.erase(iter);
	}

	bool IsPrevMatchedReady()
	{
		if(m_prevTrackedIris)
		{
			if(1 == abs(m_prevTrackedIris->getFrameIndex() - m_matchedIris->getFrameIndex()))
			{
				if(m_matchedIris->getEyeIndex() ==  m_prevTrackedIris->getPrevIndex())
				{
					return true;
				}
			}
		}

		return false;
	}

	bool CheckInterEyeTimeWindowThreshold(const int& spoofThresh,const uint64_t& currTimeStamp,const bool strictlyFlushList)
	{
		if(m_iris.size()>0)
		{
			bool shallFlush = false;
			if((currTimeStamp - m_iris.back()->getTimeStamp()) > spoofThresh)
				shallFlush = true;

			if(shallFlush)
			{
				strictlyFlushList == true ? FlushIris() : FlushAllButOne();
				return true;
			}
		}

		return false;
	}

	std::list<IrisData*> m_iris;
	IrisData* m_matchedIris;
	IrisData* m_prevTrackedIris;
	MatchResult* m_matchedResult;
};

#define MAX_IRIS_LIST_SIZE 5

class NwMatchManager: public GenericProcessor {
public:
	NwMatchManager(Configuration& conf);
	virtual ~NwMatchManager();
	void process(Copyable *msg);
	Copyable *createNewQueueItem();
	int getQueueSize(Configuration* conf);
	const char *getName(){ return "NwMatchManager"; }
	MatchResult *getMatchResult(){ return &m_mr;}
	void SetLedDispatcher(LEDDispatcher *ptr){m_ledDispatcher = ptr;}
	void SetEyelockThread(ProcessorChain *ptr){m_imageProcessor = ptr;}
    void FlushQueue(bool onlyMatches);
    void SetLoitering(LoiteringDetector *test){ m_loiteringDetector = test;}
    void SetF2FDispatcher(F2FDispatcher *ptr);
    F2FDispatcher * GetF2FDispatcher() {return m_f2fDispatcher;}
    void FlushLoitering(void);
    void SetMatchDispatcher(MatchDispatcher* ptr){m_matchDispatcher = ptr;}
    void SetIrisState(UT_IRIS_STATE x){ m_irisState = x;}
    UT_IRIS_STATE m_irisState;
	IrisData* GetMatchedIris(CHECK_IRIS chk){return chk == CHECK_IRIS_LEFT ? m_leftiris.m_matchedIris:m_rightiris.m_matchedIris;}
	int GetListSize(CHECK_IRIS chk){return CHECK_IRIS_LEFT == chk ? m_leftiris.m_iris.size() : m_rightiris.m_iris.size();}
	void* timeoutThread(void *);
	MatchManagerInterface * GetMM() { return m_matchManager;}
	void ProcessReloadMsg(HTTPPOSTMsg* msg);
	bool UploadDB(char* fname,DBMsgType msgtype);
	bool timeoutThreadSpawned;
	void RunHDMDiagnostics();
	void RunR(){recoverFromBadState();}
	bool getHDMStatus(){ return m_MatcherHDMStatus;}
	void ResetNegativeMatch();
	void ProcessReceiveUsrFromSDK(HTTPPOSTMsg* msg);
	int GetUserCount(bool excludeDummies = false);

private:
	BiOmega *m_bioInstance;
	int m_queueSz;
	MatchManagerInterface *m_matchManager;
	bool m_singleIrisinDual;
	float m_matchScoreThresh,m_matchScoreThresh1,m_minTrackMatchThresh;
	MatchResult m_mr;
    struct timeval m_timer;
    bool m_bNegativeMatchTimerActive;
    int m_iWaitTime;
    int m_iResetTimer;  //debounce timer prevents re-triggering the negative match after a match occurs
    int m_tLastTimeOutWindowStarted;
    bool m_bNegativeTimeoutKill;
    bool m_bTimeoutActive;
    MatchResult failMatch;
    bool m_intraEyeTimeWindowEnable;
    bool  negativeMatchThreadRunning;

    bool m_MatcherHDMStatus;
    bool m_Debug;
    int m_PingTimeStamp, m_PingInterval;
    LEDDispatcher *m_ledDispatcher;
	ProcessorChain *m_imageProcessor;
	char *m_clientAddr;

    void MatchDetected(MatchResult *result);
    void ReloadDB();
	virtual void recoverFromBadState();
	virtual void runDiagnostics();
    virtual void afterEnque(Safe<Copyable*> & currMsg);
    bool CheckCentroidRatio(IrisData* prev ,IrisData* curr,MatchResult* res);
    void SendF2FMessage(HTTPPOSTMsg& outMsg);
	bool ExecuteAddUpdate( int msgtype,char* tempDbFileName);
	bool DecryptDB( char* tempDbFileName);

    bool m_Master;
    bool m_spoofEnable;
    uint64_t m_interIrisTimeWindowThresholdMilliSec;
    CvPoint2D32f m_SpecCentroidRatioThreshold;
    bool m_enableCentroidRatio;
    unsigned int m_maskval;
    bool m_logging;
	LoiteringDetector *m_loiteringDetector;
	F2FDispatcher *m_f2fDispatcher;
	HTTPPOSTMsg m_F2FDbDoneMsg;
	MatchResult m_f2fResult;
    HostAddress *m_resultDestAddr;
    SocketFactory *m_socketFactory;
	struct timeval m_timeOutSend;
	MatchDispatcher* m_matchDispatcher;
	bool m_dualMatchEnabled;
	uint64_t m_intraEyeTimeWindowThreshold;
	bool m_striclyFlushLists;
	char *m_irisCodeDatabaseFile;
	void processmatch(Copyable *inpmsg);

	void FlushIrisList(CHECK_IRIS chk);
	void RefreshIrisLists();
	bool CheckIlluminator(CHECK_IRIS chk);

	bool MatchPrevEntries(CHECK_IRIS chk);
	bool CheckCentroid(CHECK_IRIS chk,bool& bFlushList);

	bool PrevTracked(CHECK_IRIS chk,bool& bFlushLoitering);

	void SetMatchedIris(CHECK_IRIS chk,MatchResult *result);
	void FillIrisLists(MatchResult *result,IrisData* curr);
	bool DontProceed4UniIris(CHECK_IRIS chk);

	IRIS_LIST m_leftiris;
	IRIS_LIST m_rightiris;
	bool m_checkUID;
	bool CheckUID();
	void Check4IrisTimeWindowThreshold(IrisData* irisCurr);
	bool m_negativeMatchEnable;
	Mutex m_DBUpdateLock;
	int m_sleepTimeBetweenMatching;
	char *m_certFile;
};

#endif /* NWMATCHMANAGER_H_ */
