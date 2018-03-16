/*
 * MatchResultHistory.h
 *
 *  Created on: 06-Aug-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef MATCHRESULTHISTORY_H_
#define MATCHRESULTHISTORY_H_
#include <list>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <sys/time.h>
#else
	#include <time.h>
#endif


#ifdef _WIN32
typedef struct {
	long int tv_sec;
	long int tv_usec;
}timeval;
#endif


enum FrameMatchType { eDETECT=0,eMATCH,eNOMATCH,eNONE,eSTART};

class MatchingResult{
public:
	MatchingResult();
	virtual ~MatchingResult();
	MatchingResult(int indx,float score){
		m_Index = indx;
		m_Score = score;
	}
	int GetIndex(){ return m_Index;}
	float GetScore(){ return m_Score;}
private:
	int m_Index;
	float m_Score;
};

typedef std::list<MatchingResult*> MatchingResultList;

class FrameMatchResult {
public:
	FrameMatchResult();
	virtual ~FrameMatchResult();
	void EmptyResultList();
	void PrintHistory(void);
	void FilePrintHistory(FILE *fp);

	void Append(int indx,float score);
	void AppendCoarse(int indx,float score);
	bool IsResultListCoarseEmpty();
	FrameMatchType GetType(){ return m_Type;}
	void SetType(FrameMatchType inp){  m_Type = inp; }
	MatchingResultList GetMRL(){ return m_ResultList;}
	MatchingResultList GetMRLCoarse(){ return m_ResultListCoarse;}

	void SetTimeStamp();
	void SetTime(long int sec,long int usec){
		m_timestamp.tv_sec = sec;
		m_timestamp.tv_usec = usec;
	}
	timeval GetTime(){
		return m_timestamp;
	}
	void SetXYCam(int &X,int &Y, char* ptr){
		m_X = X;
		m_Y = Y;
		strncpy(&m_CamID[0],ptr,32);
	}
	void SetCorruptBitPer(float a){ m_CorrBitPerc = a;}
	int GetX(){ return m_X;}
	int GetY(){ return m_Y;}
	char* GetCamID(){ return m_CamID;}
	float GetCorruptBitPer(){ return m_CorrBitPerc;}
private:
	FrameMatchType m_Type;
	MatchingResultList m_ResultList;
	MatchingResultList m_ResultListCoarse;
	timeval m_timestamp;
	int m_X,m_Y;
	char m_CamID[32];
	float m_CorrBitPerc;
};

#endif /* MATCHRESULTHISTORY_H_ */
