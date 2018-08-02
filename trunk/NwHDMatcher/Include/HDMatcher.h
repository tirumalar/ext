/*
 * MatchIrisDB.h
 *
 *  Created on: 26-Feb-2010
 *      Author: akhil
 */

#ifndef HDMATCHER_H_
#define HDMATCHER_H_
#include <utility>
#include <vector>
#include "Configurable.h"
#include "IrisMacros.h"
class IrisMatchInterface;
class MatchManagerInterface;
class ReaderWriter;
class FrameMatchResult;
class IrisData;
class Configuration;
class DBAdapter;
using std::string;

class HDMatcher: public Configurable{
public:
	enum Status{INIT, REGISTERED, AVAILABLE, BUSY, NOTAVAILABLE, CARDMATCH};
	HDMatcher(int size,int id, bool useCoarseFine, int featureMask);
	virtual ~HDMatcher();
	virtual void InitSSL(){}

	void UpdateHD(int startIndx ,int numeye){
		m_numIris = numeye;
		m_startIndx = startIndx;
	}
	void SetMatchManager( MatchManagerInterface *ptr){
		m_pMatchManagerInterface = ptr;
	}
	void Init(int irissz=1280);

	MatchManagerInterface *GetMatchManager(){ return m_pMatchManagerInterface;}

	int GetID(){return m_ID;}
	int GetSize(){return m_size;}
	int GetStartIndx(){return m_startIndx;}
	int GetNumEyes(){return m_numIris;}
	void SetCommonBits(int nominalCommonBits, int minCommonBitsFine, int minCommonBitsCoarse);

//extension points:
	virtual bool StartMatch(unsigned char *iriscode, int taskid){return false;}
	virtual bool CheckIfDone(bool check = true){return m_Status!=BUSY;}
	virtual bool CheckIfAvailable(){return m_Status==AVAILABLE;}
	virtual int  GetNumEyesPossibleInBuffer();
	//TODO
	virtual void StartMatch(unsigned char *iriscode, int taskid, int *numdenbuf){}
	virtual void AssignDB(char *fname,int memio=0){}
	virtual void AssignDB(DBAdapter *dbAdapter){}
	void SetStatus(int status){m_Status=(Status)status;}
	virtual bool SendPing(){ return true;}
	virtual bool isReboot(bool){ return false;}
	virtual void SetDebug(){m_debug= true;}
	bool InitializeDb(ReaderWriter* dbRdr, unsigned char *db, unsigned char *coarsedb);
	bool InitializeDb(DBAdapter *dbAdapter, unsigned char *db, unsigned char *coarsedb);
	std::pair<int, float> MatchIrisCode(unsigned char * IrisCode,unsigned char* DB, unsigned char *coarseDB=0);
	unsigned char* GetF2FAndIDKey(unsigned char* DB,int indx);
	std::string GetMatchGUID(unsigned char* DB,int indx);
	virtual int GetType(){return 0;}
	virtual void StartMatchInterface(int shift,bool greedyMatch,float threshold, float coarseThreshold, int irissz=1280);
	void DeclareBad(){ m_Status=NOTAVAILABLE;}
	void SetRegistered(){m_Status = REGISTERED;}
	char *GetStatus();
	int GetHDStatus(){ return m_Status;}
	int GetAssigned(){ return m_assigned;}
	void SetAssigned(int val){m_assigned = val;}
	bool IsGoodState(){
		if((m_Status==REGISTERED)||(m_Status == AVAILABLE))
			return true;
		else
			return false;
	}
	bool IsBad(){
		if(m_Status==NOTAVAILABLE)
			return true;
		else
			return false;
	}
	void CheckCorruptBits(unsigned char *db,int i);
	bool isReadyForDB();

	virtual bool GetPong(int& id,int& start,int& numeyes){
		id = m_ID;
		numeyes = m_numIris;
		start = m_startIndx;
		return true;
	}
	FrameMatchResult *GetMatcherHistory() { return m_matchResultHistory;}
	void SetLog(){m_logResults = 1;}
	static float CheckBitCorruptionPercentage(unsigned char* tag, int len, int* m_lut);
	bool CheckFromCorruptList(int indx);
	void SetMaxCorruptBitsPercAllowed(float inp){	m_maxCorruptBitsPercAllowed = inp;}
	void PrintDB(unsigned char* DB);
	void SetMaskCode(unsigned int maskcode){ m_maskCode = maskcode;}
	unsigned int GetMaskCode(){return  m_maskCode;}
	bool IslowerNibble(){ return m_LowerNibble;}
	void SetlowerNibble(bool val){ m_LowerNibble = val;}
	void SetCompressedMatching(bool val){ m_compressedMatching = val;}
	virtual bool UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris){return false;}
	virtual bool AddSingleUserOnly(string perid,string leftiris,string rightiris){return false;}
	virtual bool DeleteSingleUserOnly(unsigned char *guid){return false;}
	virtual bool FindSingleUserOnly(unsigned char *guid){return false;}
	virtual bool UpdateSingleUser(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris,unsigned char *DB,unsigned char*coarsedb);
	virtual bool DeleteSingleUser(unsigned char *guid,unsigned char *DB,unsigned char* coarsedb);
	void PrintDBGUID(unsigned char *DB);
	virtual void PrintGUIDs(){}
	virtual bool SendPCMatchMsg(char *msg, int len){return 0;}
	char *GetCardMatchName(){return m_cardMatchName;}
#ifndef UNITTEST
protected:
#endif
	MatchManagerInterface *m_pMatchManagerInterface;
	IrisMatchInterface *m_pIrisMatchInterface;
	IrisMatchInterface *m_pIrisMatchInterfaceCoarse;
	int m_numIris;
	int m_startIndx;
	int m_ID;
	int m_size;
	int m_assigned;
	Status m_Status;
	bool m_GreedyMatch;
	bool m_CoarseFineMatch;
	float m_Threshold;
	float m_CoarseThreshold;
	int m_Shift;
	int m_CoarseIrisSize;
	unsigned char *m_CoarseBuff;
	unsigned char *m_Coarsemask;
	bool m_debug;
	bool m_LowerNibble;
	bool m_compressedMatching;
	int m_nominalCommonBits;
	int m_minCommonBitsFine;
	int m_minCommonBitsCoarse;
	FrameMatchResult *m_matchResultHistory;
	int m_logResults;
	std::vector<int> m_CorruptedBitsList;
	float m_maxCorruptBitsPercAllowed;
	int *m_Lut;
	std::vector< std::pair<int,int> > m_NumDenList;
	unsigned char *m_Mask,*m_Iris;
	unsigned int m_maskCode;
	IrisData *m_irisData;
	char m_cardMatchName[100];
	int m_FeatureMask;
	unsigned char* GetIris(unsigned char *DB,int eyenum);
	unsigned char* GetMask(unsigned char *DB,int eyenum);

	unsigned char* GetCoarseIris(unsigned char *DB,int eyenum);
	unsigned char* GetCoarseMask(unsigned char *DB,int eyenum);
	unsigned char* GetIrisFromDB(unsigned char *DB,int eyeindx);
	unsigned char* GetCoarseIrisFromDB(unsigned char *DB,int eyeindx);
	void MatchIrisCodeExhaustiveNumDen(unsigned char *IrisCode, unsigned char* DB,int *buffer);

	unsigned char * extractCoarseDbRecord(unsigned char *PersonRec, unsigned char *coarseRec);
	std::pair<int,float> MatchIrisCodeGreedy(unsigned char *IrisCode, unsigned char* DB);
	std::pair<int,float> MatchIrisCodeExhaustive(unsigned char *IrisCode, unsigned char* DB);
	std::pair<int,float> MatchIrisCodeExhaustiveCoarseFine(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB);
	std::pair<int,float> MatchIrisCodeGreedyCoarseFine(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB);
	std::pair<int,float> MatchIrisCodeExhaustiveCoarseFineDBUnCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB);
	unsigned char* GetCompressIris(unsigned char *DB,int eyenum);
	unsigned char* GetCoarseCompressIris(unsigned char *DB,int eyenum);
	std::pair<int,float> MatchIrisCodeExhaustiveCoarseFineDBCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB);
	std::pair<int,float> MatchIrisCodeExhaustiveDBCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB);
	void UpdateBuffer(unsigned char *DB,unsigned char*coarsedb,int indx,unsigned char* left,unsigned char* right,unsigned char* guid,bool inc);
	void SetIrisAndGUID(unsigned char *DB,int indx,unsigned char* data,unsigned char *coarsedb=NULL,unsigned char*coarsedata=NULL);

};





#endif /* HDMATCHER_H_ */
