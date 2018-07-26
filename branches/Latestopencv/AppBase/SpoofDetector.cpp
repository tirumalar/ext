/*
 * SpoofDetector.cpp
 *
 *  Created on: 14-Jan-2010
 *      Author: akhil
 */
#include "SpoofDetector.h"
#include "MatchManagerInterface.h"
#include "CommonDefs.h"
#include "IrisData.h"
#include "NwMatcherSerialzer.h"
#include <utility>

extern "C" {
#include "file_manip.h"
}
;

using std::pair;
SpoofDetector::SpoofDetector(Configuration& conf, int width, int height,MatchManagerInterface* ptr):m_width(width), m_height(height),m_IrisBuff(0),m_irisData(0){
	m_matchScoreThresh = conf.getValue("GRI.matchScoreThresh", 0.13f);
	m_enablelog = conf.getValue("GRI.EnableLog",false);
	char *cameraID = (char*)conf.getValue("GRI.cameraID","Unknown");

	m_MatchMgr = ptr;
	m_dbIndex = -1;
	m_matchScore = 1.0;
	m_irisData =  new IrisData();
	m_irisData->setCamID(cameraID);
	m_IrisBuff = m_irisData->getIris();
}

bool SpoofDetector::GetIris(uint64_t ts, unsigned char *frame,float* robostFeatureVar){


	m_matchScore = 1.0;
	//Update the Time Stamp from the incoming image.
	m_CurrTimeStamp = ts;
	IrisPupilCircles Circles;
	bool test;
	XTIME_OP("GetIris",
		test = m_MatchMgr->GetBio()->GetIrisCode(frame, m_width, m_height, m_width,(char*)m_IrisBuff,&Circles,robostFeatureVar)
	);
	printf("Anita...........GetIris.....%s\n", m_IrisBuff);
	m_irisData->setSegmentation(test);
	m_irisData->setIrisRadiusCheck(true);
	m_irisData->setIrisCircle(Circles.ip.x,Circles.ip.y,Circles.ip.r);
	m_irisData->setPupilCircle(Circles.pp.x,Circles.pp.y,Circles.pp.r);
	m_irisData->setTimeStamp(ts);
	return test;
}

bool SpoofDetector::DoMatch(BinMessage *ptr){
	MatchResult *result;
	XTIME_OP("MATCH",
		result=m_MatchMgr->DoMatch((unsigned char *)ptr->GetBuffer())
	);

	if(m_MatchMgr->CheckValidityOfHDM())
	{
		if(m_enablelog)
			printf("Match Score %f \n",result->getScore());

		m_dbIndex=result->getEyeIndex();
		m_matchScore=result->getScore();

		return (m_matchScore<m_matchScoreThresh);
	}
	return false;
}


bool SpoofDetector::process(HTTPPOSTMsg *msg, unsigned char *frame, BiOmega *bio,float* robostFeatureVar,int domatch)
{
	bool test;
	test = GetIris(msg->GetTime(),frame,robostFeatureVar);

	if(test && (domatch)){
		NwMatcherSerialzer ns;
		int ret = ns.GetSizeOfNwMsg(m_irisData);
		BinMessage msg1(ret);
		ns.MakeNwMsg(msg1.GetBuffer(),m_irisData);

		return DoMatch(&msg1);
	}
	return false;
}
