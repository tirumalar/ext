/*
 * HDMLocal.cpp
 *
 *  Created on: 02-Mar-2010
 *      Author: akhil
 */
#include "HDMatcher.h"
#include "HDMLocal.h"
#include "MatchManagerInterface.h"
#include "HDMatcherFactory.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "FileRW.h"
#include "PermRW.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include "DBAdapter.h"

extern "C" {
#include "file_manip.h"
}

HDMLocal::HDMLocal(int size,int id,bool useCoarseFine,int featureMask,int irissz):
	HDMatcher(size,id,useCoarseFine,featureMask){
	m_CoarseDb = NULL;
	m_data = NULL;
	m_type = LOCAL;
	m_numIris = 0;//(2*m_size/pheader->GetOneRecSizeinDB()) & ~1;
	m_data=(unsigned char *)malloc(size);
	if(m_data==0) {
		printf("Could not allocate a db buffer of size %d",size);
		exit(1);
	}
	if(useCoarseFine){
		int sz = size>>2;
		m_CoarseDb =(unsigned char*) malloc(IRIS_SIZE_INCLUDING_MASK>sz?IRIS_SIZE_INCLUDING_MASK:sz);
		if(m_CoarseDb==0){
			printf("Could not allocate a db buffer of size %d",irissz);
			exit(1);
		}
	}
}

HDMLocal::~HDMLocal() {
	if(m_data){
		free(m_data);
		m_data = NULL;
	}
	if(m_CoarseDb){
		free(m_CoarseDb);
		m_CoarseDb = NULL;
	}
}

void HDMLocal::StartMatchInterface(int shift,bool greedyMatch,float threshold,float coarseThresh,int irissz){
	HDMatcher::StartMatchInterface(shift,greedyMatch,threshold,coarseThresh,irissz);
	m_Status= REGISTERED;
}
bool  HDMLocal::StartMatch(unsigned char *iriscode, int taskid){
	m_Status=BUSY;
	SetAssigned(1);
	XTIME_OP("DoMatch",
			DoMatch(iriscode,taskid)
	);
	return true;
}
void HDMLocal::DoMatch(unsigned char* inp, int taskid){
	NwMatcherSerialzer nw;
	bool ret = nw.ExtractNwMsg(m_irisData,(char*)inp);
	if(!ret){
		printf("HDMLocal::ERROR in IRIS CODE\n");
	}
	std::pair<int,float> result= MatchIrisCode(m_irisData->getIris(),m_data, m_irisData->getPupilCircle(), m_CoarseDb);
	std::string key = GetMatchGUID(m_data,result.first);
	std::string keyret;
	keyret.resize(2);
	memset((void*)keyret.c_str(),0,2);
	keyret.append(key);
	m_pMatchManagerInterface->RegisterResult(GetID(),taskid,result,(unsigned char *)keyret.c_str());
	m_Status=AVAILABLE;
}

void HDMLocal::StartMatch(unsigned char* inp, int taskid,int *numdenBuffer){
	m_Status=BUSY;
	MatchIrisCodeExhaustiveNumDen(inp,m_data,numdenBuffer);
}
void HDMLocal::SetResultMatch(std::pair<int,float> result,int taskid){
	unsigned char* key = GetF2FAndIDKey(m_data,result.first);
	m_pMatchManagerInterface->RegisterResult(GetID(),taskid,result,key);
	m_Status=AVAILABLE;
}

bool HDMLocal::AddSingleUserOnly(string perid,string leftiris,string rightiris){
	unsigned char *cdata = GetCoarseIrisFromDB(m_CoarseDb,0);
	UpdateBuffer(m_data,cdata,m_numIris>>1,(unsigned char*)leftiris.c_str(),(unsigned char*)rightiris.c_str(),(unsigned char*)perid.c_str(),true);
	if(m_Status == REGISTERED){
		m_Status=AVAILABLE;
	}
	return true;
}

bool HDMLocal::FindSingleUserOnly(unsigned char * perid){
	for(int i=0;i<m_numIris;i+=2){
		if(0 == memcmp(GetMatchGUID(m_data,i).c_str(), perid, GUID_SIZE)){
			return true;
		}
	}
}

bool HDMLocal::DeleteSingleUserOnly(unsigned char * perid){
	return DeleteSingleUser(perid,m_data,m_CoarseDb);
}

bool HDMLocal::UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris){
	return UpdateSingleUser(perid,leftiris,rightiris,m_data,m_CoarseDb);
}
void HDMLocal::PrintGUIDs(){
	PrintDBGUID(m_data);
}
void HDMLocal::AssignDB(DBAdapter *dbAdapter){
	if(dbAdapter){
		if (InitializeDb(dbAdapter, m_data, m_CoarseDb)) {
			m_Status = AVAILABLE;
		} else {
			printf("HDMLocal:: Unable to Read Data base \n");
		}
	}
}

void HDMLocal::AssignDB(char *fname, int memio) {
	//todo: sqlite 3 yo!
	ReaderWriter *reader;
	{
		if (memio) {
			printf("Using MemIO\n");
			memset(m_data, 0, m_size);
#ifdef MADHAV
			reader = new MemRW(fname + GetIrisDBHeader()->GetHeaderSize());
#endif
		} else {
#ifdef MADHAV
			reader = new FileRW((char*) fname,
					GetIrisDBHeader()->GetHeaderSize());
#endif
		}
#ifdef MADHAV
		if (GetIrisDBHeader()->GetPermutationKey() != 0) {
			PermRW dbRdr(new AesRW(reader));
			if (InitializeDb(&dbRdr, m_data, m_CoarseDb)) {
				m_Status = AVAILABLE;
			} else {
				printf("HDMLocal:: Unable to Read Data base from File %s\n",
						fname);
			}
		} else
#endif
			{
			//if no encrypt...pass the reader directly as there is no encryption on it
			if (InitializeDb(reader, m_data, m_CoarseDb)) {
				m_Status = AVAILABLE;
			} else {
				printf("HDMLocal:: Unable to Read Data base from File %s\n",
						fname);
			}

		}
	}
}

