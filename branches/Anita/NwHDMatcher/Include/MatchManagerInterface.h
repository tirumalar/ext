/*
 * MatchManagerInterface.h
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */

#ifndef MATCHMANAGERINTERFACE_H_
#define MATCHMANAGERINTERFACE_H_
#include "Configuration.h"
#include "HTTPPOSTMsg.h"
#include "BiOmega.h"
#include "Safe.h"
#include "CommonDefs.h"
#include "MatchResultHistory.h"
#include "F2FDispatcher.h"
#include "MemRW.h"
#include <list>
#include <utility>

enum MANAGER_STATUS{NOWORK,WORKASSIGN,WORKFINISHED};

using std::pair;

class HDMatcher;
class HDMatcherFactory;
class DBAdapter;
class DBMap;
class F2FDispatcher;

typedef std::list<FrameMatchResult*> MatchingResultHistory;

class ActivityTask {
public:
	ActivityTask(int id,int taskid,int matcherType);
	virtual ~ActivityTask();
	int GetID(){ return m_ID;}
	int GetTaskID(){ return m_TaskID;}
	void SetResult(std::pair<int,float> res){
		m_Result = res;
		SetEndTime();
		SetDone();
	}
	void SetEndTime();
	void SetDone(){m_Done = true;}
	bool IsDone(){
		return m_Done;
	}
	std::pair<int,float> GetResult(){ return m_Result;}
	uint64_t GetStart(){ return m_StartTime;}
	uint64_t GetEnd(){ return m_EndTime;}
	char* GetF2FKey(){ return m_F2FKey;}
	void GetMatcherType(int mtype){m_matcherType = mtype;}
	int GetMatcherType(void ){return m_matcherType;}
	void SetKey(unsigned char* key,int keyLen){
		m_f2fsz = keyLen;
		memset(m_F2FKey,0,F2FKEY_MAX_SIZE);
		memcpy(m_F2FKey,key,keyLen);
	}
	int GetF2fsz(){return m_f2fsz;}
	bool cleanupIfOlderThan(uint64_t timeusec);
#ifndef UNITTEST
protected:
#endif
	int m_ID;
	int m_TaskID;
	uint64_t m_StartTime;
	uint64_t m_EndTime;
	bool m_Done;
	std::pair<int,float> m_Result;
	char m_F2FKey[F2FKEY_MAX_SIZE];
	struct timeval m_timer;
	int m_matcherType;
	int m_f2fsz;
};

class FixedMemBufferFRR: public FixedMemBuffer {
public:
	FixedMemBufferFRR(const char*fname, int size);
	virtual ~FixedMemBufferFRR();
	virtual bool Write(void *ptr);
	bool m_Debug;
};

class MatchManagerInterface {
public:
	MatchManagerInterface(Configuration& conf,BiOmega *bio);
	virtual ~MatchManagerInterface();
	virtual void AssignDB()=0;
	virtual void CreateMatchers(Configuration & conf)=0;
	virtual int getKeyLength();
	virtual void RecoverFromBadState();
    virtual void InitResult()=0;
    virtual bool HDMDiagnostics()=0;
    virtual void RegisterResult(int id, int taskid, std::pair<int,float> inp, unsigned char *key, int keysz = -1);
    bool CheckHDMMatcherResult();
	MatchResult *DoMatch(unsigned char *iris);
    std::pair<int,float> DoMatch(unsigned char *iris, int startfrom, int cnt);
    bool CheckValidityOfHDM();
    void HDMDeclareBadforUT();
    BiOmega *GetBio(){
        return m_bio;
    }
    MatchResult *GetResult();
    void LogResults();
    void LogDetect();
    void SaveLogResult();
    bool GetLogEnable(){
    	return m_MatchingLogEnable;
    }
    void ExtractLogData(HTTPPOSTMsg *msg);
    void PrintAllHDMatcher();
    void SetAvailable();
    void SetPCMatcherResult(bool val){ m_PCmatcher= val;}
    bool GetPCMatcherResult(){ return m_PCmatcher;}
    bool UpdateDB(DBMsgType msgtype,char* fname);
    bool AddSingleUser(string perid,string leftiris,string rightiris);
    bool UpdateSingleUser(string perid,string leftiris,string rightiris);
    bool DeleteSingleUser(string perid);
    bool FindSingleUser(string perid);
    bool ClearLocalMatchBuffer();
    void UpdateDBMap();
    int GetNumEyes();
    void SetF2FDispatcher(F2FDispatcher *ptr) { m_f2fDispatcher = ptr;};
    bool PCMatcherMsg(char *msg, int len, char *user);
    bool TestPCMatcher(char *address);
    bool UpdateLocalMatchBuffer(string acd, int acdlen, int cardtype);
    bool DeleteAllUser();
    bool AddUserInLocalMatchBuffer(unsigned char *pIrisData);
    bool DeleteUserInLocalMatchBuffer(string perid);
    char *GetDeviceID() {return m_deviceID;}
    DBAdapter* GetDbAdapter() {return m_dbAdapter;}


#ifndef UNITTEST
protected:
#endif
    void RegisterHDMatcher(HDMatcher *inp);
    void DeregisterHDMatcher(int id);
    void SetHDMatcherAvailable(int id);
    bool CheckOnlyPCMatcherResult();
    bool CheckTimeOutActivity();
    int AvgPerMatcher(int totaleyes, int nummatcher);
    void EmptyTaskList();
    int GetTaskListSize();
    int GetNumHDMat(){
        return m_HDList.size();
    }
    void PrintResult();

    Safe<MatchingResultHistory> *GetMRH(){ return &m_MatchingLog ;}

    typedef std::list<ActivityTask*> ActivityList;
    BiOmega *m_bio;
    int m_numeyes;
    const char *m_irisCodeDatabaseFile;

    int m_IDLength;
    int m_FileLen;
    FILE *m_fp;
    std::list<HDMatcher*> m_HDList;
    Safe<ActivityList> m_ActivityList;
    int m_TaskCtr;
    int m_HDCount;
    MANAGER_STATUS m_Status;
    MatchResult *m_result;
    int m_Sleeptime;
    bool m_EnableDebug;
    int m_TimeoutActivity;
    int m_PCTimeOutActivity;
    int m_WaitSecForNWMatchers;
    Safe<ActivityList> *  GetActivityList(){return &m_ActivityList;}
    std::list<HDMatcher*> *GetMatcherList(){ return &m_HDList;}

    bool m_MatchingLogEnable;
    char *m_MatchingLogFileName;
    Safe<MatchingResultHistory> m_MatchingLog;
    FixedMemBufferFRR * m_LogBufferRW;
    timeval m_StartTime;
    int m_SpecX,m_SpecY;
    char m_CamID[32];
    int m_Slave;
    bool m_PCmatcher;
    DBAdapter* m_dbAdapter;
    DBMap *m_dbMap;
    bool m_mapEnable;
    F2FDispatcher *m_f2fDispatcher;
    Configuration *m_config;
    HDMatcher *m_PCMatcherCard;
    char m_deviceID[10];
    bool m_transTOC;

};

#endif /* MATCHMANAGERINTERFACE_H_ */
