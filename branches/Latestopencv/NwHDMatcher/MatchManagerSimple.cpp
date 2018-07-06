/*
 * MatchManagerSimple.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */

#include <iostream>
#include <unistd.h>
#include "MatchManagerSimple.h"
#include "HDMatcher.h"
#include "HDMatcherFactory.h"
#include "IrisDBHeader.h"
#include "UtilityFunctions.h"
extern "C" {
#include "file_manip.h"
}
using namespace std;



MatchManagerSimple::MatchManagerSimple(Configuration& conf,BiOmega *bio):MatchManagerInterface(conf,bio)
{
	m_KeyLength = conf.getValue("GRI.PersonIDSize",2);
	printf("Header ID Length %d\n",m_KeyLength);
	printf("ID Length %d\n",m_KeyLength);
}

MatchManagerSimple::~MatchManagerSimple() {

}


void MatchManagerSimple::CreateMatchers(Configuration & conf){

	m_HDCount = conf.getValue("GRI.HDMatcherCount", 0);
    HDMatcherFactory HDMFactory(conf, this);
    char *typestr = "GRI.HDMatcher.%d.Type";
    char *addressstr = "GRI.HDMatcher.%d.Address";
    for(int i=0;i<m_HDCount;i++){
		//ONLY REMOTE
		char str[100];
		sprintf(str,typestr,i);
		int matchertype = conf.getValueIndex(str,LOCAL, PCMATCHER,LOCAL,"LOCAL","REMOTE","NWMATCHER","PCMATCHER");
		sprintf(str,addressstr,i);
		const char* addr= conf.getValue(str,"");
		if((addr == NULL)&&((matchertype==REMOTEPROXY)||(matchertype==PCMATCHER))){
			printf("Input Address for matcher %d is Null\n",i);
			continue;
		}
		printf("Creating HD REMOTE MATCHER %d \n",i);
		if(matchertype != REMOTEPROXY){
			printf(" ERROR IN CONFIGRATION PLEASE VERFY w.r.t REMOTE MATCHER\n");
			matchertype = REMOTEPROXY;
		}
		HDMatcher *temp = HDMFactory.Create(matchertype,0,0,i,addr);
		printf("Register HDMatcher%d\n",i);
		RegisterHDMatcher(temp);
	}
}


void MatchManagerSimple::AssignDB(){

	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		printf("Ping ID %d to check if up\n",(*it)->GetID());
		bool bIsBad=true;
		for(int i=0;i<m_WaitSecForNWMatchers;i++)
		{
			if((*it)->SendPing())
			{
				bIsBad=false;
				break;
			}
			else sleep(1);
		}
		if ((*it)->isReboot(bIsBad))
			RunSystemCmd("reboot");

		if(bIsBad) (*it)->DeclareBad();
	}

	//Assign HDMatcher for all
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		// num eyes holder in the HD
		item->UpdateHD(0,0);
		if(!item->isReadyForDB()) {
			item->DeclareBad();
			continue;
		}
		item->SetStatus(HDMatcher::AVAILABLE);
	}
}

void MatchManagerSimple::RecoverFromBadState(){
	std::list<HDMatcher *>::iterator it;
	while(!CheckValidityOfHDM())
	{
		for(it = m_HDList.begin();it != m_HDList.end(); it++){
			HDMatcher *item = (*it);
			int id,start,numeyes;
			id = start = numeyes=-1;
			while(!item->IsGoodState()){
				printf("Matcher with ID %d is not in good state \n",item->GetID());
				if(item->GetPong(id,start,numeyes))
				{
					if(m_EnableDebug )
						printf("We get from PONG ::%d %d %d\n",id,start,numeyes);
					item->SetRegistered();
				}
				else{
					sleep(1);
				}
			}
		}
	}
	AssignDB();
}

void MatchManagerSimple::InitResult(){
	assert(m_Status == WORKFINISHED);
	std::pair<int,float> result;
	bool alldballocated = true;
	int unallocatedid= -1;
	m_ActivityList.lock();
	ActivityList& l=m_ActivityList.get();
	ActivityList::iterator it=l.begin();
	result.first = 0;
	result.second =1.0;
	char key[256]={0};
	char *keyptr = key;

	ActivityTask initial(-1,-1,-1);
	initial.SetResult(result);
	initial.SetKey((unsigned char*)key,2);

	ActivityTask *bestItem= &initial;

	for(;it != l.end(); it++){
		ActivityTask *item = (*it);
		if(item->GetResult().second < bestItem->GetResult().second)
		{
			bestItem=item;
		}
	}
	m_ActivityList.unlock();
	//assert(bestItem!=0);
	if(bestItem != NULL){
		result = bestItem->GetResult();
		keyptr = bestItem->GetF2FKey();
	}

	m_result->init(result.first,result.second,
			getKeyLength(),m_IDLength,false);
	m_result->setKey(keyptr,getKeyLength());
}

bool MatchManagerSimple::HDMDiagnostics(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		int id,start,numeyes;
		id = start = numeyes= 0;
		if(!item->GetPong(id,start,numeyes))
			return false;
	}
	return true;
}

void MatchManagerSimple::RegisterResult(int id,int taskid,std::pair<int,float> inp,unsigned char* key,int keysz){
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
				printf("Register ID %d Task ID: %d Res < %d,%f> ",id,taskid,inp.first,inp.second);
				printf("KEY :: ");
				for(int k=0;k<getKeyLength();k++)
					printf("%d ",key[k]);
				printf("\n");
			}
			item->SetResult(inp);
			int f2fsz = getKeyLength();
			if(keysz != -1)f2fsz = keysz;
			item->SetKey(key,f2fsz);
			match=true;
			break;
		}
	}

	m_ActivityList.unlock();
	if(!match) printf("Unable to Register ID(%d) with TaskID(%d) <%d>\n",id,taskid,sz);
}


