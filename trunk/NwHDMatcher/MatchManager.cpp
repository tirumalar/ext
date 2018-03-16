/*
 * MatchManager.cpp
 *
 *  Created on: 26-Feb-2010
 *      Author: akhil
 */
#include <iostream>
#include "MatchManager.h"
#include "HDMatcher.h"
#include "HDMatcherFactory.h"
#include "DBAdapter.h"
#include "logging.h"
#include "DBMap.h"
#include "UtilityFunctions.h"
#include <unistd.h>

extern "C" {
#include "file_manip.h"
}
using namespace std;

const char logger[30] = "MatchManager";

MatchManager::MatchManager(Configuration& conf,BiOmega *bio):MatchManagerInterface(conf,bio)
{

}

MatchManager::~MatchManager() {

}

bool CompareSize (HDMatcher* first, HDMatcher* second){
  return (first->GetSize() < second->GetSize());
}

//TODO: bookmark for creatematchers
void MatchManager::CreateMatchers(Configuration & conf){
	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
	gettimeofday(&m_StartTime, 0);
	m_MatchingLogEnable = conf.getValue("GRI.MatcherLogEnable", false);
#ifdef HBOX_PG
	m_MatchingLogFileName = (char *)conf.getValue("GRI.MatcherLogFile","Result.txt");
#else
	m_MatchingLogFileName = (char *)conf.getValue("GRI.MatcherLogFile","/mnt/mmc/Result.txt");
#endif
	if(m_MatchingLogEnable){
		int logbuffsz = conf.getValue("GRI.MatcherLogBuffSize",1024*100);
		m_LogBufferRW = new FixedMemBufferFRR(m_MatchingLogFileName,logbuffsz);
		m_LogBufferRW->m_Debug = conf.getValue("GRI.MatcherLogDebug",false);

		FILE *fp = fopen(m_MatchingLogFileName,"a");
		if(fp != NULL){
			fprintf(fp,"<%11lu,%11lu>",m_StartTime.tv_sec,m_StartTime.tv_usec);
			fprintf(fp,":Start of Matcher;\n");
			EyelockLog(logger, DEBUG, "<%11lu,%11lu>:Start of Matcher;\n",m_StartTime.tv_sec,m_StartTime.tv_usec);
			fclose(fp);
		}
	}
	m_HDCount = conf.getValue("GRI.HDMatcherCount", 0);
    HDMatcherFactory HDMFactory(conf, this);
    char *typestr = "GRI.HDMatcher.%d.Type";
    char *buffsizestr = "GRI.HDMatcher.%d.BuffSize";
    char *addressstr = "GRI.HDMatcher.%d.Address";
    for(int i=0;i<m_HDCount;i++){
		 //LOCAL,REMOTE
		char str[100];
		sprintf(str,typestr,i);
		int matchertype = conf.getValueIndex(str,LOCAL, PCMATCHER,LOCAL,"LOCAL","REMOTE","NWMATCHER","PCMATCHER");
		sprintf(str,buffsizestr,i);
		int buffsize = conf.getValue(str,1024*1024);
		sprintf(str,addressstr,i);
		const char* addr= conf.getValue(str,"");
		bool matchFlag = false;
#ifdef OTO_MATCH
		bool dualAuth = conf.getValue("GRITrigger.DualAuthenticationMode",false);
		matchFlag = dualAuth;
#else
		bool m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);
		matchFlag = m_transTOC;
#endif
		if (matchFlag){
			printf("MatchManager::CreateMatchers() %d, type %d\n", i, matchertype);

			m_HDCount =1;
			matchertype = LOCAL;
			addr = NULL;
		}

		int invalid = false;
		if (addr == NULL)
			invalid = true;
		else {
			unsigned int i;
			// check IP addr and port number
			for(i = 0; i < sizeof(addr); i++)
			{
				if(addr[i] == ':')
					break;
			}
			// check IP address "0.0.0.0"
			if (strncmp(addr, "0.0.0.0", i) == 0 || atoi(addr+i+1) == 0) {
				invalid = true;
			}
		}

		if((invalid == true)&&((matchertype==REMOTEPROXY)||(matchertype==PCMATCHER))){
			EyelockLog(logger, DEBUG, "Input Address for matcher %d is Null or invalid",i);
			continue;
		}
		if(matchertype == PCMATCHER){
			EyelockLog(logger, DEBUG, "Pcmatcher");
			buffsize = 0;
			m_PCMatcherCard = HDMFactory.Create(matchertype,1280,buffsize,99,addr);
		}
		EyelockLog(logger, DEBUG, "Creating HDMatcher%d with size %d",i,buffsize);
		HDMatcher *temp = HDMFactory.Create(matchertype,1280,buffsize,i,addr);
		if((matchertype == LOCAL)&&(m_MatchingLogEnable))
			temp->SetLog();
		EyelockLog(logger, DEBUG, "Register HDMatcher%d",i);
		RegisterHDMatcher(temp);
	}
}


void MatchManager::AssignDB(){
//Sort by size
	m_dbAdapter->ReOpenConnection();
	m_HDList.sort(CompareSize);

	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		EyelockLog(logger, DEBUG, "Ping ID %d to check if up",(*it)->GetID());
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

	int eyesleft = m_dbAdapter->GetUserCount()*2;
	if(eyesleft<0)eyesleft=0;
	m_numeyes=eyesleft;
	EyelockLog(logger, DEBUG, "Total NumEyes found in the DB is %d ",eyesleft);
	if(2 == eyesleft){
		EyelockLog(logger, DEBUG, "Create a copy of db for each of the matchers'.");
		SpecialCaseOfOneUser();
	}
	else
	{
		int numMatcher = m_HDList.size();
		int avgperMatcher = AvgPerMatcher(eyesleft,numMatcher);
		int startindx=0;

		//Assign HDMatcher for all
		for(it = m_HDList.begin();it != m_HDList.end(); it++){
			HDMatcher *item = (*it);
			// num eyes holder in the HD
			m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
			int num = item->GetNumEyesPossibleInBuffer();
			int numeyes = MIN(num,avgperMatcher) & ~1;
			// make num eyes even
			EyelockLog(logger, DEBUG, "BefNumEyes: %d ,StartIndx: %d ,NumEyes %d , Size %d",num,startindx,numeyes,item->GetSize());
			item->UpdateHD(startindx,numeyes);
			eyesleft -= numeyes;
			startindx += numeyes;
			numMatcher--;
			avgperMatcher = AvgPerMatcher(eyesleft,numMatcher);
			EyelockLog(logger, DEBUG, "Avg PerMatcher: %d ",avgperMatcher);
			if(!item->isReadyForDB()) {
				item->DeclareBad();
				continue;
			}
			if(numeyes != 0){
				//item->AssignDB((char*)m_irisCodeDatabaseFile,0);
				TIME_OP("ASSIGNDB",
						item->AssignDB(m_dbAdapter)
				);
			}
		}
	}
	UpdateDBMap();

	m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
}

bool MatchManager::SpecialCaseOfOneUser()
{
	bool bSucess = true;
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++)
	{
		HDMatcher *item = (*it);
		m_dbAdapter->OpenFile((char*)m_irisCodeDatabaseFile);
		item->UpdateHD(0,m_numeyes);
		if(!item->isReadyForDB())
		{
			item->DeclareBad();
			continue;
		}
		if(0 < item->GetNumEyesPossibleInBuffer()){
			//item->AssignDB((char*)m_irisCodeDatabaseFile,0);
			item->AssignDB(m_dbAdapter);
		}
	}

	return bSucess;
}

void MatchManager::AssignDB(int id,char *filename){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetID() == id){
			EyelockLog(logger, DEBUG, "Ping ID %d to check if up",item->GetID());
			bool bIsBad=true;
			for(int i=0;i<m_WaitSecForNWMatchers;i++)
			{
				if(item->SendPing())
				{
					bIsBad=false;
					break;
				}
				else sleep(1);
			}
			if ((*it)->isReboot(bIsBad))
				RunSystemCmd("reboot");

			if(bIsBad)
				(*it)->DeclareBad();
			else
				item->SetRegistered();
		}
	}

	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		if(item->GetID() == id){
			if(!item->isReadyForDB()) {
				item->DeclareBad();
				continue;
			}
			item->AssignDB(filename);
		}
	}
}

void MatchManager::InitResult(){
	assert(m_Status == WORKFINISHED);
	std::pair<int,float> result;
	bool alldballocated = true;
	int unallocatedid= -1;
	ActivityTask dummyItem(-1,-1,-1);
	ActivityTask *bestItem =  NULL;

	m_ActivityList.lock();
	ActivityList& l=m_ActivityList.get();
	int sz = l.size();
	if(sz != 0){
		ActivityList::iterator it=l.begin();
		bestItem= (*it);

		for(;it != l.end(); it++){
			ActivityTask *item = (*it);
			if(item->GetResult().second == 2.0){
				alldballocated = false;
				unallocatedid = item->GetID();
			}
			if(item->GetResult().second < bestItem->GetResult().second)
			{
				bestItem=item;
			}
		}

	}else{
		unsigned char buff[F2FKEY_MAX_SIZE] = {0};
		bestItem = &dummyItem;
		dummyItem.SetKey(buff,F2FKEY_MAX_SIZE);
	}
	m_ActivityList.unlock();

	if(!alldballocated){
		EyelockLog(logger, DEBUG, "De-registering ID %d ",unallocatedid);
		DeregisterHDMatcher(unallocatedid);
	}

	assert(bestItem!=0);
	result = bestItem->GetResult();

//	EyelockLog(logger, DEBUG, "Now from GUID Lets get acd data");
	string acddata;
	string acdappendeddata;
	string username;
	PERSON_INFO p =DUAL;
	int acdlen=-1;

	char *key = bestItem->GetF2FKey();
	//lets get acdlen
	acdlen = (int)((((unsigned int) key[1])<<8) + (unsigned char) key[0]);
	int offbytestoGUID = (acdlen+7)>>3;
	//lets get GUID
	std::string guid(key+2+offbytestoGUID);
	if(acdlen){
		acdappendeddata.resize(offbytestoGUID +2);
		memcpy((unsigned char *)acdappendeddata.c_str(),key,offbytestoGUID+2);
	}else{
		int ret=-1;
//		XTIME_OP("GetUserACDData",
//			ret = m_dbAdapter->GetUserACDData(guid,acddata,acdlen)
//		);
		if(m_mapEnable){
			string gui(guid);

			bool r ;
			XTIME_OP("DBMAPGET",
				r = m_dbMap->GetAcdData(gui,username,acddata,acdlen,p)
			);
			if(r) ret = 0;
//			printf("GUID %s Name %s AcdLen %d Per %d\n ",gui.c_str(),username.c_str(),acdlen,p==DUAL?0:1);
//			for(int i=0;i<acddata.length();i++){
//				printf("%02x ",(unsigned char )*(acddata.c_str()+i));
//			}
//			printf("\n");
		}
		if(ret == 0){
			acdappendeddata.resize(acddata.length());
			//*(((unsigned char *)acdappendeddata.c_str())+ 0) = (unsigned char )(acdlen&0xFF);
			//*(((unsigned char *)acdappendeddata.c_str())+ 1) = (unsigned char )((acdlen>>8)&0xFF);
			//acdappendeddata.append(acddata);
			memcpy((void*)(acdappendeddata.c_str()),(void*)acddata.c_str(),acddata.length());
		}else{
			acdappendeddata.resize(2);
			*(((unsigned char *)acdappendeddata.c_str())+ 0) = (unsigned char )(0);
			*(((unsigned char *)acdappendeddata.c_str())+ 1) = (unsigned char )(0);
		}
	}
//	printf("GUID %s Name %s AcdLen %d Per %d\n ",guid.c_str(),username.c_str(),acdlen,p==DUAL?0:1);
//	for(int i=0;i<acdappendeddata.length();i++){
//		printf("%02x ",(unsigned char )*(acdappendeddata.c_str()+i));
//	}
//	printf("\n");

	m_result->init(result.first,result.second,acdappendeddata.length(),m_IDLength,false);
	m_result->setKey((char*)acdappendeddata.c_str(),acdappendeddata.length());
//	string name;
//	TIME_OP("GetUsername",
//		m_dbAdapter->GetUsername(guid,name)
//	);
//	printf("UserName %s \n",username.c_str());
	m_result->setGUID(guid);
	m_result->setName(username);
	if (m_PCmatcher == true)
		m_result->setName(guid);
	m_result->SetPersonIrisInfo((int)p);
}


bool MatchManager::HDMDiagnostics(){
	std::list<HDMatcher *>::iterator it;
	for(it = m_HDList.begin();it != m_HDList.end(); it++){
		HDMatcher *item = (*it);
		int id=-1,start=-1,numeyes=-1;
		id = start = numeyes= 0;
		if(!item->GetPong(id,start,numeyes))
		{
			if(numeyes == 0 ){
				EyelockLog(logger, DEBUG, "Diagnostics::Deregistering ID %d ",id);
				DeregisterHDMatcher(item->GetID());
				return false;
			}
		}
//		printf("%d -> %d %d\n",id,start,numeyes);
//		item->PrintGUIDs();
	}
	return true;
}
