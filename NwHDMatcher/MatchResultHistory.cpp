/*
 * MatchResultHistory.cpp
 *
 *  Created on: 06-Aug-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "MatchResultHistory.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;

MatchingResult::MatchingResult(){

}

MatchingResult::~MatchingResult(){

}

FrameMatchResult::FrameMatchResult():m_Type(eNONE),m_X(0),m_Y(0),m_CorrBitPerc(0.0f) {
	memset(m_CamID,0,32);
}

FrameMatchResult::~FrameMatchResult() {
	EmptyResultList();
}

void FrameMatchResult::EmptyResultList(){
	while (!m_ResultList.empty()){
		MatchingResult *item =m_ResultList.back();
		m_ResultList.pop_back();
		delete item;
	}
	while (!m_ResultListCoarse.empty()){
		MatchingResult *item =m_ResultListCoarse.back();
		m_ResultListCoarse.pop_back();
		delete item;
	}
	SetType(eNONE);
}

bool FrameMatchResult::IsResultListCoarseEmpty(){
	if(m_ResultListCoarse.size() <= 0)  return true;
	return false;
}

void FrameMatchResult::Append(int indx,float score){
	MatchingResult *ptr = new MatchingResult(indx,score);
	m_ResultList.push_back(ptr);
}

void FrameMatchResult::AppendCoarse(int indx,float score){
	MatchingResult *ptr = new MatchingResult(indx,score);
	m_ResultListCoarse.push_back(ptr);
}

void FrameMatchResult::PrintHistory(){
	printf("Time::%lu %lu \n",m_timestamp.tv_sec,m_timestamp.tv_usec);
	if(GetType() == eDETECT){
		printf("Type::DETECT\n");
		printf("\n");
		return;
	}
	if(GetType() == eMATCH)
		printf("Type::MATCH\n");
	if(GetType() == eNOMATCH)
		printf("Type::NOMATCH\n");
	printf("Coarse List[%d]::",m_ResultListCoarse.size());
	MatchingResultList::iterator it;
	for(it = m_ResultListCoarse.begin();it != m_ResultListCoarse.end(); it++){
		MatchingResult *item = (*it);
		printf("<%d,%f>,",item->GetIndex(),item->GetScore());
	}
	printf("\n");
	printf("Fine List[%d]::",m_ResultList.size());
	for(it = m_ResultList.begin();it != m_ResultList.end(); it++){
		MatchingResult *item = (*it);
		printf("<%d,%f>,",item->GetIndex(),item->GetScore());
	}
	printf("\n");
}

void FrameMatchResult::FilePrintHistory(FILE *fp){
	fprintf(fp,"<%12lu,%12lu>",m_timestamp.tv_sec,m_timestamp.tv_usec);
	if(GetType() == eDETECT){
		fprintf(fp,":DETECT");
	}
	if(GetType() == eMATCH)
		fprintf(fp,":MATCH,");
	if(GetType() == eNOMATCH)
		fprintf(fp,":NOMATCH,");

	if((GetType() == eMATCH) ||(GetType() == eNOMATCH)){
		fprintf(fp,"Coarse,%d,",m_ResultListCoarse.size());
		MatchingResultList::iterator it;
		for(it = m_ResultListCoarse.begin();it != m_ResultListCoarse.end(); it++){
			MatchingResult *item = (*it);
			fprintf(fp,"%d,%f,",item->GetIndex(),item->GetScore());
		}
		fprintf(fp,"Fine,%d,",m_ResultList.size());
		for(it = m_ResultList.begin();it != m_ResultList.end(); it++){
			MatchingResult *item = (*it);
			fprintf(fp,"%d,%f,",item->GetIndex(),item->GetScore());
		}
	}
	fprintf(fp,";\n");
}

void FrameMatchResult::SetTimeStamp(){
#ifdef __linux__
	gettimeofday(&m_timestamp, 0);
#endif
}

