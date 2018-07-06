/*
 * UT_SafePtr.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "MatchResultHistory.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "HTTPPOSTMsg.h"
#include "MatchProcessor.h"
namespace tut {
struct FRRTestData {
	FRRTestData() {
	}
	~FRRTestData() {
	}
};
typedef test_group<FRRTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("FRR tests");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("Test MatchingResult class");
	MatchingResult a(-1,100.0);

	ensure_equals("Index",a.GetIndex(),-1);
	ensure_equals("Index",a.GetScore(),100.0);

}

template<>
template<>
void testobject::test<2>() {
	set_test_name("Test MatchResultHistory class");
	FrameMatchResult mrh;
	MatchingResult *mr;

	ensure_equals("Index",mrh.GetType(),eNONE);
	ensure_equals("List Size should be 0",mrh.GetMRL().size() ,0);
	ensure_equals("List Size should be 0",mrh.GetMRLCoarse().size() ,0);

	mrh.Append(0,0.0);
	mr = mrh.GetMRL().back();
	ensure_equals("Index",mr->GetIndex() ,0);
	ensure_equals("Score",mr->GetScore(),0.0);
	ensure_equals("List Size should be 1",mrh.GetMRL().size() ,1);
	ensure_equals("List Size should be 0",mrh.GetMRLCoarse().size() ,0);

	mrh.AppendCoarse(0,0.0);
	mr = mrh.GetMRLCoarse().back();
	ensure_equals("Index",mr->GetIndex() ,0);
	ensure_equals("Score",mr->GetScore(),0.0);
	ensure_equals("List Size should be 1",mrh.GetMRL().size() ,1);
	ensure_equals("List Size should be 0",mrh.GetMRLCoarse().size() ,1);


	mrh.Append(1,0.1f);
	mr = mrh.GetMRL().back();
	ensure_equals("Index",mr->GetIndex() ,1);
	ensure_equals("Score",mr->GetScore(),0.1f);
	ensure_equals("List Size should be 2",mrh.GetMRL().size() ,2);
	ensure_equals("List Size should be 1",mrh.GetMRLCoarse().size() ,1);
	mrh.AppendCoarse(1,0.1f);
	mr = mrh.GetMRLCoarse().back();
	ensure_equals("Index",mr->GetIndex() ,1);
	ensure_equals("Score",mr->GetScore(),0.1f);
	ensure_equals("List Size should be 2",mrh.GetMRL().size() ,2);
	ensure_equals("List Size should be 2",mrh.GetMRLCoarse().size() ,2);


	mrh.Append(2,0.2f);
	mr = mrh.GetMRL().back();
	ensure_equals("Index",mr->GetIndex() ,2);
	ensure_equals("Score",mr->GetScore(),0.2f);
	ensure_equals("List Size should be 3",mrh.GetMRL().size() ,3);

	mrh.Append(3,0.3f);
	mr = mrh.GetMRL().back();
	ensure_equals("Index",mr->GetIndex() ,3);
	ensure_equals("Score",mr->GetScore(),0.3f);
	ensure_equals("List Size should be 4",mrh.GetMRL().size() ,4);

	mrh.EmptyResultList();
	ensure_equals("List Size should be 0",mrh.GetMRL().size() ,0);

}
template<>
template<>
void testobject::test<3>() {
		set_test_name("Test FrameMatchResult list delete");
		MatchingResultHistory mrh;

		for(int i=0;i<10;i++){
			FrameMatchResult *fm = new FrameMatchResult;
			fm->Append(i,0.0);
			fm->Append(i,0.0);
			fm->AppendCoarse(i,0.0);
			fm->AppendCoarse(i,0.0);
			mrh.push_back(fm);
		}
		ensure_equals("List Size should be 0",mrh.size() ,10);

		for(int i = 0;i< 5;i++){
			FrameMatchResult *mr = mrh.front();
			printf("Del FMR\n");
			delete mr;
			mrh.pop_front();
			ensure_equals("List Size ",mrh.size(),10-1-i);
		}

	}
#if 1
template<>
template<>
void testobject::test<4>() {
	//skip();
	set_test_name("Test FixedMemBufferFRR write");
	FrameMatchResult mrh;
	MatchingResult *mr;

	mrh.SetType(eMATCH);
	mrh.Append(0,0.0);
	mrh.AppendCoarse(0,0.0);
	mrh.Append(1,0.1f);
	mrh.AppendCoarse(1,0.1f);
	mrh.Append(2,0.2f);
	char *m_CamID ="1234567890123456789012345678901234567890";

	int x=1,y=2;
	mrh.SetXYCam(x,y,m_CamID);
	mrh.SetCorruptBitPer(99.99);

	FixedMemBufferFRR *m_LogBufferRW = new FixedMemBufferFRR("FRR.txt",250);
	m_LogBufferRW->m_Debug =1;
	m_LogBufferRW->Write((void*)(&mrh));
	ensure_equals("Available will be 57 ",57,m_LogBufferRW->GetAvailable());
	m_LogBufferRW->Write((void*)(&mrh));
	ensure_equals("Available will be 57 ",57,m_LogBufferRW->GetAvailable());
	m_LogBufferRW->Flush();
	ensure_equals("Available will be 250 ",250,m_LogBufferRW->GetAvailable());
	m_LogBufferRW->Write((void*)(&mrh));
	ensure_equals("Available will be 57 ",57,m_LogBufferRW->GetAvailable());
	m_LogBufferRW->Flush();
	mrh.EmptyResultList();
	ensure_equals("List Size should be 0",mrh.GetMRL().size() ,0);
	ensure_equals("Index",mrh.GetType(),eNONE);

	m_LogBufferRW->Write((void*)(&mrh));
	ensure_equals("Available will be 250-25+7+14+12+2 ",250-(25+2),m_LogBufferRW->GetAvailable());
	remove("FRR.txt");
	}
#endif
}
