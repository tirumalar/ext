/*
 * MatchManagerInterface.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "MatchManagerInterface.h"
#include "HDMatcher.h"
#include "HDMatcherFactory.h"
#include "DBAdapter.h"
#include "DBMap.h"
#include "logging.h"
#include "HDMPCRemote.h"
#include "UtilityFunctions.h"

extern "C" {
#include "file_manip.h"
}
using namespace std;

const char logger[30] = "MatchManagerInterface";

ActivityTask::ActivityTask(int id,int taskid,int matcherType)
: m_EndTime(-1),m_Done(false),m_matcherType(matcherType){
	m_ID = id;
	m_TaskID = taskid;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,c);
	m_StartTime = c;
	m_EndTime = m_StartTime;
	m_Result = std::make_pair(-1, 1);
	m_f2fsz = 2;
	memset(m_F2FKey,0,F2FKEY_MAX_SIZE);
}

void ActivityTask::SetEndTime(){
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,c);
	m_EndTime = c;
}

ActivityTask::~ActivityTask(){

}

bool ActivityTask::cleanupIfOlderThan(uint64_t timeusec)
{
	if(GetStart()>timeusec) return false;
	std::pair<int,float> p;
	p.first=-1;
	p.second=1.0f;
	SetResult(p);

	return true;
}

FixedMemBufferFRR::FixedMemBufferFRR(const char*fname, int size):
	FixedMemBuffer(fname,size),m_Debug(0){

}

FixedMemBufferFRR:: ~FixedMemBufferFRR(){

}

MatchManagerInterface::MatchManagerInterface(Configuration& conf,BiOmega *bio):
m_IDLength(0),m_FileLen(0),m_fp(0),m_LogBufferRW(0),m_TaskCtr(0),m_Status(NOWORK),m_PCmatcher(false),m_mapEnable(false),
m_TimeoutActivity(1000000),m_MatchingLogEnable(0),m_SpecX(0),m_SpecY(0),m_dbAdapter(NULL),m_dbMap(NULL),m_f2fDispatcher(NULL), m_config(NULL){
	memset(m_CamID,0,32);
	m_bio = bio;
#ifndef HBOX_PG
	m_irisCodeDatabaseFile = conf.getValue("GRI.irisCodeDatabaseFile",
			"data/sqlite.db3");
#else				
	m_irisCodeDatabaseFile = conf.getValue("GRI.irisCodeDatabaseFile",
			"data/sqlite.db3");
#endif			
	m_IDLength= conf.getValue("GRI.IrisIdNumBytes", 0);

	m_EnableDebug = conf.getValue("GRI.MatchMgrDebug",false);
	m_Sleeptime = conf.getValue("GRI.MatchMgrSleepTimeusec",5000);
	m_TimeoutActivity = conf.getValue("GRI.TimeoutActivity",5000000);
	m_PCTimeOutActivity = conf.getValue("GRI.PCTimeoutActivity",1000000);
    m_WaitSecForNWMatchers = conf.getValue("GRI.WaitSecForNWMatchers",100);
    m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);
    m_Slave = conf.getValue("GRI.EyelockSlave",0);
    m_mapEnable = conf.getValue("Eyelock.MapEnable",true);
	m_result= new MatchResult();
	m_dbAdapter = new DBAdapter();
	if(m_mapEnable){
		m_dbMap = new DBMap();
	}
	m_config = &conf;
	m_PCMatcherCard = NULL;

	// set m_deviceID
	memset(m_deviceID, 0, 10);
#ifndef HBOX_PG
	FILE *fp = fopen("/home/root/id.txt","r");
#else
	FILE *fp = fopen("id.txt","r");
#endif	
	if(fp == NULL){
		printf("Unable to open device ID file\n");
	} else {
	      char *p = fgets(m_deviceID, 10, fp);
	      if (p != NULL) {
	           m_deviceID[strlen(m_deviceID)-1] = '\0';
	      }
	      fclose(fp);
	}

}

MatchManagerInterface::~MatchManagerInterface() {
	if(m_result)
		delete m_result;

	while (!m_HDList.empty()){
		HDMatcher *item =m_HDList.back();
		m_HDList.pop_back();
		delete item;
	}
	EmptyTaskList();

	if(m_LogBufferRW)
		delete m_LogBufferRW;
	if(m_fp)
		fclose(m_fp);
	if(m_dbAdapter){
		delete m_dbAdapter;
	}
	if(m_dbMap){
		delete m_dbMap;
	}
	if(m_PCMatcherCard)
		delete m_PCMatcherCard;
}

int MatchManagerInterface::getKeyLength()
{
#ifdef __MADHAV__
	return m_IrisDBHeader->GetF2FSize();
#endif
	return GUID_SIZE + 2; //First 2 bytes specify the no of bits bits(acdlen)
}



void MatchManagerInterface::RegisterHDMatcher(HDMatcher *inp){
	m_HDList.push_back(inp);
}

void MatchManagerInterface::DeregisterHDMatcher(int id){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetID() == id){
			item->DeclareBad();
		}
	}
}
void MatchManagerInterface::SetHDMatcherAvailable(int id){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetID() == id){
			item->SetStatus(HDMatcher::AVAILABLE);
		}
	}
}

int MatchManagerInterface::GetTaskListSize(){
	m_ActivityList.lock();
	int sz = m_ActivityList.get().size();
	m_ActivityList.unlock();
	return sz;
}
void MatchManagerInterface::EmptyTaskList(){
	m_ActivityList.lock();

	ActivityList& l=m_ActivityList.get();
	while (!l.empty()){
		ActivityTask *item =l.back();
		l.pop_back();
		delete item;
	}

	m_ActivityList.unlock();
}


bool CompareType (HDMatcher* first, HDMatcher* second){
  return (first->GetType() > second->GetType());
}


bool MatchManagerInterface::CheckHDMMatcherResult(){
	SetPCMatcherResult(CheckOnlyPCMatcherResult());
	bool pcResult = GetPCMatcherResult();
	// loop over all HD to checkIfDone
	if(!pcResult){
		std::list<HDMatcher *>::iterator ith;
		for(ith = m_HDList.begin();ith != m_HDList.end()&&(!pcResult); ith++){
//			struct timeval timer;
//			gettimeofday(&timer,0);
//			TV_AS_USEC(timer,c);
//			EyelockLog(logger, DEBUG, "Time:: %llu ",c);
			bool val = (*ith)->CheckIfDone(true);
//			EyelockLog(logger, DEBUG, "Val %s",val?"TRUE":"FALSE");
			if(val){
				if((*ith)->GetType()== PCMATCHER && ((*ith)->GetAssigned()== 1))
					pcResult = true;
			}
		}
	}
	return pcResult;
}




MatchResult*  MatchManagerInterface::DoMatch(unsigned char* iris){
// Check list
// if not empty crib and empty list
	if(m_ActivityList.get().size()!= 0){
		EyelockLog(logger, DEBUG, "Match Manager has not finished the Job discarding the previous tasks");
		EmptyTaskList();
	}

//Sort on Type to make all first remote and then local
	m_HDList.sort(CompareType);
	//Slave mode do not check it.
	if(m_Slave){
		SetAvailable();
	}else{
		if(!CheckValidityOfHDM()){
			m_Status = WORKFINISHED;
			return m_result;
		}
	}
// make entry into list
// start match for each match
	m_Status = WORKASSIGN;
	SetPCMatcherResult(false);

	struct timeval timer;
	gettimeofday(&timer,0);
	TV_AS_USEC(timer,c);

	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++,m_TaskCtr++){
		HDMatcher *item = (*it);
		item->SetAssigned(0);
	}
//Run all N/w First and then the Local
	for(it = m_HDList.begin();it != m_HDList.end(); it++,m_TaskCtr++){
		HDMatcher *item = (*it);
		ActivityTask *ptr=0;
		bool bAssigned=item->CheckIfAvailable();
		if(bAssigned){
			ptr = new ActivityTask(item->GetID(),m_TaskCtr,item->GetType());
			m_ActivityList.lock();
			m_ActivityList.get().push_back(ptr);
			m_ActivityList.unlock();
			if(item->GetType() == LOCAL){
				SetPCMatcherResult(CheckOnlyPCMatcherResult());
			}
			if(GetPCMatcherResult()){
				break;
			}
//			EyelockLog(logger, DEBUG, "Assign %d ",item->GetID());
			bAssigned=item->StartMatch(iris, m_TaskCtr);
		}

		if(!bAssigned)
		{
			EyelockLog(logger, INFO, "Could not assign match task %d to %d",m_TaskCtr,item->GetID());
			if(ptr)
			{
				m_ActivityList.lock();
				m_ActivityList.get().pop_back();
				m_ActivityList.unlock();
				delete ptr;
			}
		}
	}
	do{
		if(!GetPCMatcherResult())
			usleep(m_Sleeptime);
	}while(!CheckTimeOutActivity());

	gettimeofday(&timer,0);
	TV_AS_USEC(timer,c1);
//	EyelockLog(logger, DEBUG, "ST ET DIFF:: %llu %llu %llu ",c,c1,c1-c);

	if(m_EnableDebug)
		PrintResult();

	InitResult();
	EmptyTaskList();
	if(m_MatchingLogEnable)
		LogResults();

	return GetResult();
};

void MatchManagerInterface::LogResults(){
	m_MatchingLog.lock();
	MatchingResultHistory& matchlog=m_MatchingLog.get();
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		FrameMatchResult *mrh = item->GetMatcherHistory();
		if(mrh != 0){
			mrh->SetTimeStamp();
			mrh->SetXYCam(m_SpecX,m_SpecY,m_CamID);
			float a = -1.0;
			if(m_bio){
				a = m_bio->GetCorruptBitsPerc();
			}
			mrh->SetCorruptBitPer(a);
			matchlog.push_back(mrh);
		}
	}
	MatchingResultHistory matchlogCopy(matchlog);
	matchlog.clear();
	m_MatchingLog.unlock();

	std::list<FrameMatchResult *>::iterator it1;

	for(it1 = matchlogCopy.begin();it1 != matchlogCopy.end(); it1++){
		FrameMatchResult *item = (*it1);
		m_LogBufferRW->Write((void*)item);
		delete item;
	}

}

void MatchManagerInterface::LogDetect(){
	if(m_MatchingLogEnable){
		FrameMatchResult *mrh = new FrameMatchResult;
		mrh->SetType(eDETECT);
		m_MatchingLog.lock();
		mrh->SetTimeStamp();
		MatchingResultHistory& matchlog=m_MatchingLog.get();
		matchlog.push_back(mrh);
		m_MatchingLog.unlock();
	}
}

MatchResult *MatchManagerInterface::GetResult(){
	assert(m_Status == WORKFINISHED);
	return m_result;
}


bool MatchManagerInterface::CheckTimeOutActivity(){
	std::list<HDMatcher *>::iterator ith;
	bool pcResult = CheckHDMMatcherResult();
	bool ret = !pcResult;

	if(!pcResult){
		struct timeval timer;
		unsigned long oldestAllowedTaskTime = timer.tv_sec * 1000000 + timer.tv_usec -m_TimeoutActivity;
		m_ActivityList.lock();
		ActivityList& l=m_ActivityList.get();
		ActivityList::iterator it=l.begin();
		for(;it!=l.end();it++){
			if(!((*it)->IsDone())){

				gettimeofday(&timer,0);
				TV_AS_USEC(timer,c);
				c = c - m_TimeoutActivity;

				if((*it)->GetMatcherType() == PCMATCHER){
					c = c + m_TimeoutActivity -m_PCTimeOutActivity;
				}

				if((*it)->cleanupIfOlderThan(c)){
					if((*it)->GetMatcherType() == PCMATCHER){
						(*it)->SetDone();
						EyelockLog(logger, DEBUG, "Timout PCMatcher %d",(*it)->GetID());
						SetHDMatcherAvailable((*it)->GetID());
						continue;
					}
					EyelockLog(logger, DEBUG, "Deregistering ID %d ",(*it)->GetID());
					DeregisterHDMatcher((*it)->GetID());
				}
			}
		}
		//Loop over activity list to c if all results r arrived
		//then set status to WORKFINISHED

		for(it = l.begin();it != l.end(); it++){
			ActivityTask *item = (*it);
			if(!item->IsDone()) {
				ret = false;
				break;
			}
		}
	}

	if(ret||pcResult) m_Status = WORKFINISHED;
	m_ActivityList.unlock();

	// Check for PC Match
	if(pcResult){
		for(ith = m_HDList.begin();ith != m_HDList.end(); ith++){
			(*ith)->CheckIfDone(false);
		}
		ret = true;
		m_Status = WORKFINISHED;
	}
	return ret;
}



void MatchManagerInterface::RegisterResult(int id,int taskid,std::pair<int,float> inp,unsigned char* key,int keysz){
//Find ID+TaskID in the Activity List
	bool match=false;
	ActivityList::iterator it;
	int sz =0;
	m_ActivityList.lock();
	ActivityList& l=m_ActivityList.get();
	sz = l.size();
	for(it = l.begin();it != l.end(); it++){
		ActivityTask *item = (*it);
		if((item->GetID()==id) && (item->GetTaskID()==taskid)&&(!item->IsDone())){
			if(m_EnableDebug){
				EyelockLog(logger, DEBUG, "Register ID %d Task ID: %d Res < %d,%f> ",id,taskid,inp.first,inp.second);
//				unsigned char *p = key;
//				for(int i=0;i<10;i++){
//					EyelockLog(logger, DEBUG, "%3d",(unsigned char)(*p++) );
//				}
//				EyelockLog(logger, DEBUG, "");
			}
			item->SetResult(inp);
			int f2fsz = getKeyLength();
			if(keysz != -1)f2fsz = keysz;
			item->SetKey(key,f2fsz);
			match=true;
			if(item->GetMatcherType()== PCMATCHER)
				m_PCmatcher = true;
			break;
		}
	}

	m_ActivityList.unlock();
	if(!match) EyelockLog(logger, DEBUG, "Unable to Register ID(%d) with TaskID(%d) <%d>",id,taskid,sz);
}

int MatchManagerInterface::AvgPerMatcher(int totaleyes,int nummatcher){
	if ((nummatcher > 0) && (totaleyes > 0) ) return (totaleyes/nummatcher);
	return 0;
}

void MatchManagerInterface::PrintAllHDMatcher(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		EyelockLog(logger, DEBUG, "%2d  %5d  %5d %s",item->GetID(),item->GetStartIndx(),item->GetNumEyes(),item->GetStatus());
	}
}

void MatchManagerInterface::PrintResult(){
	ActivityList::iterator it;
	m_ActivityList.lock();
	ActivityList& l=m_ActivityList.get();
	//please do not put logger for these printf. These have to be rewriiten for log4cxx.
	printf("List %d",l.size());
	for(it = l.begin();it != l.end(); it++){
		ActivityTask *item = (*it);
		printf("ID %d  TaskID %d ST: %llu ET: %llu Diff %lu %d ",item->GetID(),item->GetTaskID(),item->GetStart(), item->GetEnd(), item->GetEnd()-item->GetStart(),item->IsDone()?1:0);
		printf("Res < %d,%f> ",item->GetResult().first,item->GetResult().second);
		if(item->IsDone()){
			printf("F2F : %d - ",item->GetF2fsz());fflush(stdout);
			unsigned char *ptr = (unsigned char *) item->GetF2FKey();
			for(int i=0;i<item->GetF2fsz();i++){
				printf("%02x ",(unsigned char)(*ptr++) );
			}
			printf("\nF2F : ");
			ptr = (unsigned char *) item->GetF2FKey();
			for(int i=0;i<item->GetF2fsz();i++){
				printf("%c ",(char)(*ptr++) );
			}
			printf("\n");
		}
	}
	m_ActivityList.unlock();
}



void MatchManagerInterface::RecoverFromBadState(){
	std::list<HDMatcher *>::iterator it;
	char *filename = (char*)m_irisCodeDatabaseFile;
	while(!CheckValidityOfHDM())
	{
		for(it = m_HDList.begin();it != m_HDList.end(); it++){
			HDMatcher *item = (*it);
			int id,start,numeyes;
			id = start = numeyes=-1;
			while(!item->IsGoodState()){
				EyelockLog(logger, DEBUG, "Matcher with ID %d is not in good state ",item->GetID());
				if(item->GetPong(id,start,numeyes))
				{
					if(m_EnableDebug )
						EyelockLog(logger, DEBUG, "We get from PONG ::%d %d %d",id,start,numeyes);
					item->SetRegistered();
				}
				else{
					sleep(1);
				}
			}
		}
	}
	// we are means all good, lets reload the db
	AssignDB();
}

void MatchManagerInterface::SetAvailable(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		item->SetStatus(HDMatcher::AVAILABLE);
	}
}


bool MatchManagerInterface::CheckValidityOfHDM(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->IsBad())
			return false;
	}
	return true;
}

void MatchManagerInterface::HDMDeclareBadforUT(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		item->DeclareBad();
	}
}



void MatchManagerInterface::SaveLogResult(){
	if(m_MatchingLogEnable){
		if(m_LogBufferRW->m_Debug){
			EyelockLog(logger, DEBUG, "Available %d",m_LogBufferRW->GetAvailable());
		}
		m_LogBufferRW->Flush();
	}
}
void MatchManagerInterface::ExtractLogData(HTTPPOSTMsg* msg){
	memset(m_CamID,0,32);
	msg->getCameraID(m_CamID);
	msg->getXY(m_SpecX,m_SpecY);
}

bool MatchManagerInterface::CheckOnlyPCMatcherResult(){
	bool pcResult= false;
	std::list<HDMatcher *>::iterator ith;
	for(ith = m_HDList.begin();ith != m_HDList.end(); ith++){
		if((*ith)->GetType()== PCMATCHER && ((*ith)->GetAssigned()== 1)){
//			struct timeval timer;
//			gettimeofday(&timer,0);
//			TV_AS_USEC(timer,c);
//			EyelockLog(logger, DEBUG, "Time:: %llu ",c);
			pcResult = (*ith)->CheckIfDone(true);
//			EyelockLog(logger, DEBUG, "MatchManager::CheckOnlyPCMatcherResult %s",pcResult?"TRUE":"FALSE");
		}
	}
	return pcResult;
}

bool FixedMemBufferFRR:: Write(void *ptr){

	FrameMatchResult *mrh = (FrameMatchResult *)ptr;
	int memreqd = 25;
	if(mrh->GetType() == eDETECT) memreqd+=7;
	if(mrh->GetType() == eMATCH) memreqd+=7;
	if(mrh->GetType() == eNOMATCH) memreqd+=9;
	if((mrh->GetType() == eMATCH) ||(mrh->GetType() == eNOMATCH)){
		memreqd+=7;
		memreqd+=46;
		memreqd+=14;
		memreqd+=16*mrh->GetMRLCoarse().size();
		memreqd+=12;
		memreqd+=16*mrh->GetMRL().size();
	}
	memreqd+=2;
	if(m_Debug)EyelockLog(logger, DEBUG, "SizeReqd = %d",memreqd);

	if(!IsAvailable(memreqd))
		Flush();

	if(IsAvailable(memreqd)){
		int cnt = 0;
		//Takes 1+11+1+11+1= 25 bytes
		cnt = sprintf(&m_CurrPos[cnt],"<%11lu,%11lu>",mrh->GetTime().tv_sec,mrh->GetTime().tv_usec);
		if(mrh->GetType() == eDETECT){
			//Takes 7 bytes
			cnt+=sprintf(&m_CurrPos[cnt],":DETECT");
		}
		if(mrh->GetType() == eMATCH){
			//Takes 7 bytes
			cnt+=sprintf(&m_CurrPos[cnt],":MATCH,");
		}
		if(mrh->GetType() == eNOMATCH){
			//Takes 9 bytes
			cnt+=sprintf(&m_CurrPos[cnt],":NOMATCH,");
		}

		if((mrh->GetType() == eMATCH) ||(mrh->GetType() == eNOMATCH)){
			//Takes 5+1+1 = 7 bytes
			cnt+=sprintf(&m_CurrPos[cnt],"%5.2f%%,",mrh->GetCorruptBitPer());
			//Takes 6+1+6+1+31+1 = 46 bytes
			cnt+=sprintf(&m_CurrPos[cnt],"%6d,%6d,%.*s,",mrh->GetX(),mrh->GetY(),31,mrh->GetCamID());
			MatchingResultList mrl = mrh->GetMRLCoarse();
			//Takes 7+6+1 = 14 bytes
			cnt+=sprintf(&m_CurrPos[cnt],"Coarse,%6d,",(int)(mrl.size()));
			MatchingResultList::iterator it;
			for(it = mrl.begin();it != mrl.end(); it++){
				MatchingResult *item = (*it);
				//Takes 6 +8+2 = 16 bytes
				cnt+=sprintf(&m_CurrPos[cnt],"%6d,%8.6f,",item->GetIndex(),item->GetScore());
			}
			//Takes 5+6+1 = 12 bytes
			mrl = mrh->GetMRL();
			cnt+=sprintf(&m_CurrPos[cnt],"Fine,%6d,",(int)(mrl.size()));
			for(it = mrl.begin();it != mrl.end(); it++){
				MatchingResult *item = (*it);
				//Takes 6 +8+2 = 16 bytes
				cnt+=sprintf(&m_CurrPos[cnt],"%6d,%8.6f,",item->GetIndex(),item->GetScore());
			}
		}
		//Takes 2 bytes
		cnt+=sprintf(&m_CurrPos[cnt],";\n");
		if(m_Debug) EyelockLog(logger, DEBUG, "SizeWritten = %d",cnt);
		m_CurrPos += cnt;
		return true;
	}
	EyelockLog(logger, DEBUG, "Can be issue with Logging");
	return false;
}

bool MatchManagerInterface::AddSingleUser(string perid,string leftiris,string rightiris){
	//Add only to Local
	std::list<HDMatcher *>::iterator it;
	bool update=false;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetType() == LOCAL){
			bool ret = item->AddSingleUserOnly(perid,leftiris,rightiris);
			if(ret) update = true;
		}
	}
	return update;
}


bool MatchManagerInterface::UpdateSingleUser(string perid,string leftiris,string rightiris){
	std::list<HDMatcher *>::iterator it;
	bool update=false;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		bool ret = item->UpdateSingleUserOnly((unsigned char *)perid.c_str(),(unsigned char *)leftiris.c_str(),(unsigned char *)rightiris.c_str());
		if(ret) update = true;
	}
	return update;
}

bool MatchManagerInterface::DeleteSingleUser(string perid){
	std::list<HDMatcher *>::iterator it;
	bool deleted=false;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		bool ret = item->DeleteSingleUserOnly((unsigned char *)perid.c_str());
		if(ret) deleted = true;
	}
	return deleted;
}

bool MatchManagerInterface::FindSingleUser(string perid){
	std::list<HDMatcher *>::iterator it;
	bool find=false;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetType() == LOCAL){
			bool ret = item->FindSingleUserOnly((unsigned char *)perid.c_str());
			if(ret) find = true;
		}
	}
	return find;
}
int MatchManagerInterface::GetNumEyes(){
	int eyes = 0;
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		eyes += item->GetNumEyes();
	}
	return eyes;
}

void MatchManagerInterface::UpdateDBMap(){
	if(m_mapEnable){
		m_dbAdapter->CloseConnection();
		m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
		m_dbAdapter->ReadDBForCheckingIris(m_dbMap);
	}
}

bool MatchManagerInterface::UpdateDB(DBMsgType msgtype,char* fname){
	bool ret = true;
	bool reloadACD = false;
	EyelockLog(logger, DEBUG, "MatchManagerInterface::UpdateDB: msgtype %d", msgtype);
	if(eREPLACEDB == msgtype){
		EyelockEvent("DB replaced");
		DBAdapter db;
		if(db.OpenFile(fname)){
			int checkIntegrityResult = db.CheckIntegrity();
			db.CloseConnection();
			if (checkIntegrityResult == 0) {
				EyelockLog(logger, DEBUG, "replacing database file with %s", fname);
				m_dbAdapter->CloseConnection();
				if(0 == rename(fname,m_irisCodeDatabaseFile)){
					sync();
					EyelockLog(logger, DEBUG, "Received File Now run sync()  Cmd ");
					ret = true;
				}else{
					ret = false;
				}
			}
			else
			{
				EyelockLog(logger, ERROR, "Database file %s corrupted", fname);
				ret = false;
			}
		}
		m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
		reloadACD = true;
	}else if(eUPDATEDB == msgtype){
		DBAdapter db;
		m_dbAdapter->CloseConnection();
		m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
		if(db.OpenFile(fname)){
			int acdlen;
			int pinlen = 0;
			bool supportPin = false;
			int rtn;
			string username,leftiris,rightiris,acd,acdnop,pin;
			vector<pair<int,string> > res = db.GetIncrementalData();
			if(res.size() == 0){
				ret = false;
			}
			for(int i=0;i<res.size();i++){
				pair<int,string> p = res[i];
				if(UPDATEPERSON == p.first){
					//Lets Add it
					EyelockLog(logger, DEBUG, "Adding User %s to DB ",p.second.c_str());

					rtn = db.GetUserData(p.second,username,leftiris,rightiris,acd,acdlen,acdnop,pin);
					if (rtn == 0){
						supportPin = true;
					}
					else if (0 == db.GetUserData(p.second,username,leftiris,rightiris,acd,acdlen,acdnop)){
						supportPin = false;
						rtn = 0;
					}
					if (rtn == 0) {
						// Get existing ACD before user data updated
						string old_acd, old_acdnop;
						int old_acdlen;
						m_dbAdapter->GetUserACDData(p.second,old_acd,old_acdlen,old_acdnop);

						int ispersonindb;
						if (supportPin)
							ispersonindb = m_dbAdapter->UpdateUser(p.second,username,leftiris,rightiris,acd,acdlen,acdnop,pin);
						else
							ispersonindb = m_dbAdapter->UpdateUser(p.second,username,leftiris,rightiris,acd,acdlen,acdnop);
						if(ispersonindb>=0){
							if(m_dbMap){
								m_dbMap->Insert(p.second,leftiris,rightiris,username,acd,acdlen);
							}

							EyelockLog(logger, DEBUG, "Added/Updated User %s to DB is done, acdlen %d",p.second.c_str(), acdlen);
							bool success=false;
							if(ispersonindb==0){
								success = AddSingleUser(p.second,leftiris,rightiris);
								EyelockEvent("DB updated: Adding User %s",username.c_str());
								EyelockLog(logger, DEBUG, "Added User %s to MEMDB %d",p.second.c_str(),success?1:0);
								if(m_f2fDispatcher){
									if(m_f2fDispatcher->addACD(acd, acdlen, acdnop, username, pin) == false)
										reloadACD = false;
								}

							}else if(ispersonindb==1){
								success = UpdateSingleUser(p.second,leftiris,rightiris);
								EyelockEvent("DB updated: Updating User %s",username.c_str());
								EyelockLog(logger, DEBUG, "Updating User %s to MEMDB %d",p.second.c_str(),success?1:0);
								if (1) {
									// update ACD
									EyelockEvent("DB updated: Updating ACD list ...");
									if(m_f2fDispatcher){
										if(m_f2fDispatcher->modifyACD(old_acd, old_acdlen, old_acdnop, acd, acdnop, pin) == false)
											reloadACD = false;
									}
									EyelockEvent("ACD table updated");
								}
							}
							if(!success){
								ret = false;
								EyelockLog(logger, DEBUG, "Unable To ADD User %s %s to DB ",p.second.c_str(),username.c_str());
							}
						}else{
							ret=false;
						}
					}else{
						EyelockLog(logger, WARN, "Unable To Fetch User %s from Incremental DB ",p.second.c_str());
						ret = false;
					}
				}else if(DELETEPERSON == p.first){
					//Lets Remove from DB
					if (m_dbAdapter->GetUsername(p.second,username) == 0) {
						EyelockEvent("DB updated: Deleting User %s",username.c_str());
						EyelockLog(logger, DEBUG, "Removing User %s from DB ",p.second.c_str());
						if( 0 != m_dbAdapter->DeleteUser(p.second)){
							EyelockLog(logger, ERROR, "Unable to Remove User %s from DB ",p.second.c_str());
							ret = false;
						}else{
							EyelockLog(logger, DEBUG, "Deleted User %s from DB is done",p.second.c_str());
							bool success = DeleteSingleUser(p.second);
							if(!success){
								EyelockLog(logger, DEBUG, "Unable to Deleted User %s from MEMDB is done",p.second.c_str());
								ret = false;
							}else{
								if(m_dbMap){
									m_dbMap->Erase(p.second);
								}
								EyelockLog(logger, DEBUG, "Deleting User %s to MEMDB %d",p.second.c_str(),success?1:0);
								reloadACD = true;
							}
						}
					} else {
						EyelockLog(logger, DEBUG, "Failed removing User %s from DB ",p.second.c_str());
						ret = false;
					}
				}
			}

			// update ACS data
			string parityMask, dataMask;
			char *parityMaskArray = NULL;	// Not used anymore
			char *dataMaskArray = NULL;
			int result = -1;
			bool newMask = db.CheckACSMaskColumn();
			bool localMask = m_dbAdapter->CheckACSMaskColumn();
			EyelockLog(logger, DEBUG, "Updated ACS: newMask %d, localMask %d\n", newMask, localMask);
			if (newMask == localMask) {
				if(newMask && db.GetACSData(username,acd,acdlen, parityMask, dataMask) == 0){
						result = m_dbAdapter->UpdateACS(username,acd,acdlen,parityMask,dataMask);
					dataMaskArray = (char *)dataMask.c_str();
				}
				else if(!newMask && db.GetACSData(username,acd,acdlen) == 0){
					result = m_dbAdapter->UpdateACS(username,acd,acdlen);
				}
			}
			else {
				if (newMask && db.GetACSData(username,acd,acdlen, parityMask, dataMask) == 0) {
					EyelockLog(logger, INFO, "Not found masks in local DB, add them now");
					m_dbAdapter->AddACSMaskColumn();
					result = m_dbAdapter->UpdateACS(username,acd,acdlen,parityMask,dataMask);
				}
				else if(!newMask && db.GetACSData(username,acd,acdlen) == 0){
					EyelockLog(logger, INFO, "Not found masks in DB, reset to default");
					int acdbytes = (acdlen+7)/8;
					dataMaskArray = (char *)malloc(acdbytes+1);
					memset((void *)dataMaskArray, 0xFF, acdbytes);
					dataMaskArray[acdbytes] = '\0';
					dataMask = std::string(dataMaskArray);
					result = m_dbAdapter->UpdateACS(username,acd,acdlen,parityMask,dataMask);
					dataMask = "hello";
					m_dbAdapter->GetACSData(username,acd,acdlen, parityMask, dataMask);
				}
			}

			if(result>=0){
				// update ACD length
				if(m_f2fDispatcher)
					m_f2fDispatcher->SetAccessDataLength(acdlen, parityMaskArray, dataMaskArray);
				EyelockLog(logger, DEBUG, "Updated ACS data %s, acdlen %d, parityMask %s, dataMask %s", username.c_str(), acdlen, parityMask.c_str(), dataMask.c_str());
			}
			else
				EyelockLog(logger, ERROR, "Updated ACS data failed");
		}
		db.CloseConnection();
		m_dbAdapter->CloseConnection();
		sync();
		m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
	}

	if (reloadACD && m_f2fDispatcher){
		m_f2fDispatcher->loadACD();
	}

	return ret;
}

bool MatchManagerInterface::PCMatcherMsg(char *msg, int len, char *user){
	EyelockLog(logger, DEBUG, "MatchManager::SendPCMatchMsg");
	bool pcResult= false;

	if (!m_PCMatcherCard)
		return pcResult;

	pcResult = m_PCMatcherCard->SendPCMatchMsg(msg, len);
	EyelockLog(logger, DEBUG, "MatchManager::SendPCMatchMsg %s",pcResult?"TRUE":"FALSE");
	if (pcResult)
		pcResult = m_PCMatcherCard->CheckIfDone(true);
	if (pcResult){
		char *name = m_PCMatcherCard->GetCardMatchName();
		strcpy(user, name);
	}
	EyelockLog(logger, DEBUG, "MatchManager::SendPCMatchMsg Response %s",pcResult?"TRUE":"FALSE");

	return pcResult;
}
bool MatchManagerInterface::TestPCMatcher(char *address){
	bool pcResult= false;
	struct timeval timeOut;
	timeOut.tv_sec = 500 / 1000;
	timeOut.tv_usec = (500 % 1000) * 1000;
	HDMPCRemote* inp = new HDMPCRemote(0,99,address);
	inp->setConf(m_config);
	inp->SetMatchManager(this);
	inp->SetTimeOut(timeOut);
	inp->InitSSL();

	pcResult = inp->SendPCMatchMsg(NULL, 0);
	//pcResult = m_PCMatcherCard->SendPCMatchMsg(NULL, 0);
	EyelockLog(logger, DEBUG, "MatchManager::TestPCMatcher %s",pcResult?"TRUE":"FALSE");
	if (pcResult)
		pcResult = inp->CheckIfDone(true);
	EyelockLog(logger, DEBUG, "MatchManager::TestPCMatcher %s",pcResult?"TRUE":"FALSE");
	if (inp)
		delete inp;
	return pcResult;
}

bool MatchManagerInterface::UpdateLocalMatchBuffer(string acd, int acdlen, int cardtype)
{
	string perid, username,leftiris,rightiris;

	EyelockLog(logger, TRACE, "MatchManagerInterface::UpdateLocalMatchBuffer()  ");


	m_dbAdapter->CloseConnection();
	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);

	if (0 == m_dbAdapter->GetUserDataFromACD(perid,username,leftiris,rightiris,acd,acdlen,cardtype))
	{
		AddSingleUser(perid, leftiris, rightiris);
		if(m_dbMap)
			m_dbMap->Insert(perid,leftiris,rightiris,username,acd,acdlen);
	}

	EyelockLog(logger, DEBUG, "User info: %s\n", username.c_str());

	return true;
}

bool MatchManagerInterface::ClearLocalMatchBuffer(){
	EyelockLog(logger, TRACE, "MatchManagerInterface::ClearLocalMatchBuffer() ");

	std::list<HDMatcher *>::iterator it;

	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetType() == LOCAL){
            if (!m_transTOC)
            	item->UpdateHD(0,0);
            else
            	item->UpdateHD(4,4);	// why it has to be 4 to start match?
		}
	}

	if(m_dbMap)
		m_dbMap->Clear();

	return true;
}

bool MatchManagerInterface::DeleteAllUser() {
	EyelockLog(logger, DEBUG, "MatchManagerInterface::DeleteAllUser() ");
	m_dbAdapter->CloseConnection();
	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);

	std::vector<std::string> outStringVec;
	int ret = m_dbAdapter->getAllUserIDs(outStringVec);
	if(ret == 0){
		for(std::vector<std::string>::iterator itr = outStringVec.begin();itr != outStringVec.end();itr++)
			DeleteSingleUser(*itr);
	}
	return true;
}

#if 0
bool MatchManagerInterface::SetLocalMatchBuffer(DBMsgType msgtype,char* fname)
{
	EyelockLog(logger, DEBUG, "MatchManagerInterface::SetLocalMatchBuffer()  ");
	bool ret = true;
	DBAdapter db;
	if(db.OpenFile(fname)){
		int acdlen;
		string username,leftiris,rightiris,acd,acdnop;
		vector<pair<int,string> > res = db.GetIncrementalData();
		if(res.size() == 0){
			ret = false;
		}

		pair<int,string> p = res[0];
		//Lets Add it
		EyelockLog(logger, DEBUG, "Adding User %s to match buffer ",p.second.c_str());

		if(0 == db.GetUserData(p.second,username,leftiris,rightiris,acd,acdlen,acdnop)){
			AddSingleUser(p.second, leftiris, rightiris);
			if(m_dbMap)
				m_dbMap->Insert(p.second,leftiris,rightiris,username,acd,acdlen);
		}
		db.CloseConnection();
		EyelockLog(logger, DEBUG, "User info: %s\n", username.c_str());
	}
	return ret;
}
#endif


bool MatchManagerInterface::AddUserInLocalMatchBuffer(unsigned char *pIrisData)
{
	EyelockLog(logger, TRACE, "MatchManagerInterface::AddUserInLocalMatchBuffer()  ");
	bool ret = false;
	unsigned char left_a[COMPRESS_IRIS_SIZE_INCLUDING_MASK*2], right_a[COMPRESS_IRIS_SIZE_INCLUDING_MASK*2];
	int index = 0;

	/*
	 * 	[ 100 bytes name/id]
		[1 byte acd length in bytes]  (n)
		[1 byte acd length in bits]
		[n bytes acd data with parity]
		[n bytes acd data with no parity]
		[n bytes deniedAccessData]
    	[1 byte HRdataLength - l]
    	[l bytes deniedAccesshumanReadbleData]
		[4 bytes unix start date for card validity]
		[4 bytes unix end date for card validity]
		[1280 bytes iris data]
	 *
	 */

	index += 100;	// fist 100 bytes for key
	string username = string(pIrisData, pIrisData+index);
	unsigned char acdbytes = pIrisData[index++];
	unsigned char acdbits = pIrisData[index++];
	string acd = string(pIrisData+index, pIrisData+index+acdbytes);
	index += acdbytes;
	string acdnop = string(pIrisData+index, pIrisData+index+acdbytes);
	index += acdbytes;
	// will add test code
	string acdtest = string(pIrisData+index, pIrisData+index+acdbytes);
	index += acdbytes;
	unsigned char teststrlen = pIrisData[index++];
	string teststr = string(pIrisData+index, pIrisData+index+teststrlen);
	index += teststrlen;
	// card time
	unsigned int starttime = *(unsigned int *)(pIrisData+index);	// int
	index += 4;
	unsigned int endtime = *(unsigned int *)(pIrisData+index);
	index += 4;
	// Iris code
	unsigned char *pLeftiris = pIrisData+index;
	index += 1280;
	unsigned char *pRightiris = pIrisData+index;
	index += 1280;

	// verify card time
	time_t currseconds = time(NULL);

	if ((unsigned int)currseconds < starttime || (unsigned int)currseconds > endtime) {
		printf("Error: start %u, end %u, curr %u\n", (unsigned int)starttime, (unsigned int)endtime, (unsigned int)currseconds);
		return ret;
	}


	int acdlen = (int) acdbits;
	int found = username.find_last_of("|");
	string perid = username.substr(found+1);
	found = perid.find_first_of('\0');
	perid = perid.substr(0, found);
	UnCompressIris(pLeftiris, left_a, COMPRESS_IRIS_SIZE_INCLUDING_MASK);
	UnCompressIris(pRightiris, right_a, COMPRESS_IRIS_SIZE_INCLUDING_MASK);
	string leftiris = string(left_a, left_a+COMPRESS_IRIS_SIZE_INCLUDING_MASK*2);
	string rightiris = string(right_a, right_a+COMPRESS_IRIS_SIZE_INCLUDING_MASK*2);
	printf("user name %s, acd bytes %d, bits %d\n", username.c_str(), acdbytes, acdbits);
	// update test code
	string parityMask="";
	string dataMask="";
	m_dbAdapter->CloseConnection();
	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
	int result = m_dbAdapter->UpdateACS(teststr,acdtest,acdbits);
	if(result>=0){
		// update ACD length
		if(m_f2fDispatcher)
			m_f2fDispatcher->SetAccessDataLength(acdlen, parityMask.c_str(), dataMask.c_str());
		EyelockLog(logger, DEBUG, "Updated ACS data %s, acdlen %d", teststr.c_str(), acdbits);
	}
	result = m_dbAdapter->UpdateUser(perid,username,leftiris,rightiris,acd,acdlen,acdnop);
	if(result==0){
		EyelockLog(logger, DEBUG, "Updated user data");
		if(m_dbMap){
			m_dbMap->Insert(perid,leftiris,rightiris,username,acd,acdlen);
		}

		EyelockLog(logger, DEBUG, "User Added - User info: %s", username.c_str());
		ret = AddSingleUser(perid,leftiris,rightiris);
		if(ret && m_f2fDispatcher)
			ret = m_f2fDispatcher->addUser(perid, username, acd);
		EyelockLog(logger, DEBUG, "User Added - User info: %s", username.c_str());
	}

	// set card data
	memcpy(pIrisData, acd.c_str(), acdbytes);
	pIrisData[acdbytes] = '\n';
	return ret;
}

bool MatchManagerInterface::DeleteUserInLocalMatchBuffer(string perid)
{
	EyelockLog(logger, TRACE, "MatchManagerInterface::DeleteUserInLocalMatchBuffer() %s ", perid.c_str());

	bool ret = false;
	m_dbAdapter->CloseConnection();
	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
	if( 0 == m_dbAdapter->DeleteUser(perid)){
		EyelockLog(logger, DEBUG, "Deleted User %s", perid.c_str());
		ret = DeleteSingleUser(perid);
		if(!ret){
			EyelockLog(logger, DEBUG, "Unable to Deleted User %s",perid.c_str());
		}else{
			if(m_dbMap){
				m_dbMap->Erase(perid);
			}
		}
	}
	return ret;
}


