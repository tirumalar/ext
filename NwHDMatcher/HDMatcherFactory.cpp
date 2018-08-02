/*
 * HDMatchFactory.cpp
 *
 *  Created on: 02-Mar-2010
 *      Author: akhil
 */
#include "Configuration.h"
#include "HDMatcher.h"
#include "HDMLocal.h"
#include "HDMRemote.h"
#include "HDMPCRemote.h"
#include "HDMatcherFactory.h"

HDMatcherFactory::HDMatcherFactory(Configuration& conf, MatchManagerInterface *ptr):m_Mgr(0){
	setConf(&conf);
	m_Mgr = ptr;

	int timeOutms = conf.getValue("GRI.socketTimeOutMillis", 500);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
}

HDMatcherFactory::~HDMatcherFactory(){
}

HDMatcher* HDMatcherFactory::Create(int matchtype,int irissz,int size,int id,const char* add ){
	int shift = getConf()->getValue("GRI.HDMatcherShift", 12);
	bool greedy = getConf()->getValue("GRI.GreedyMatch",false);
	bool enablelog = getConf()->getValue("GRI.MatcherLogEnable",false);
	if(enablelog && greedy && (LOCAL == matchtype)){
		printf("Log is enabled for HD Matcher and thus greedy match cannot be used.\n");
		printf("Changing mode to non greedy match for LOCAL\n");
		greedy = false;
	}

	float thresh = getConf()->getValue("GRI.matchScoreThresh", 0.13f);
	float coarsethresh = getConf()->getValue("GRI.matchCoarseThresh",0.35f);
	bool coarseFineMatch=getConf()->getValue("GRI.useCoarseFineMatch",false);
	if(getConf()->getValue("GRITrigger.TransTOCMode", false))
		coarseFineMatch = false;

	int nominalCommonBits = getConf()->getValue("GRI.Match.NominalCommonBits",4100);
	int minCommonBitsFine = getConf()->getValue("GRI.Match.MinCommonBits",0);
	int minCommonBitsCoarse = getConf()->getValue("GRI.Match.Coarse.MinCommonBits",(minCommonBitsFine)/4);
	int maxCorrBitPer = getConf()->getValue("GRI.MaxCorruptBitPercentageEnrollment",70);
	bool compressedMatching = getConf()->getValue("GRI.CompressedMatching",false);
	char *cameraID = (char*)getConf()->getValue("GRI.cameraID","Unknown");

	bool debug=getConf()->getValue("GRI.HDDebug",false);
	printf("Debug For Matcher %d \n",debug?1:0);

	unsigned int maskcode = getConf()->getValue("GRI.MatcherFeatureMask",255);
	unsigned int maskval = (maskcode<<24)|(maskcode<<16)|(maskcode<<8)|(maskcode);
	bool lowernibble = getConf()->getValue("GRI.MatcherUseLowerNibble",true);

	if (LOCAL == matchtype){
		HDMLocal* inp = new HDMLocal(size,id,coarseFineMatch,maskcode,irissz);
		inp->setConf(getConf());
		inp->SetMatchManager(m_Mgr);
		inp->SetCommonBits(nominalCommonBits,minCommonBitsFine,minCommonBitsCoarse);
		inp->SetMaxCorruptBitsPercAllowed(maxCorrBitPer*1.0f);
		inp->SetMaskCode(maskval);
		inp->SetlowerNibble(lowernibble);
		inp->SetCompressedMatching(compressedMatching);
		if(debug) inp->SetDebug();
		inp->StartMatchInterface(shift,greedy,thresh,coarsethresh,irissz);
		return inp;
	}
	else if(REMOTEPROXY == matchtype ){
		HDMRemote* inp = new HDMRemote(size,id,maskcode,add);
		inp->setConf(getConf());
		inp->SetMatchManager(m_Mgr);
		inp->SetTimeOut(m_timeOut);
		inp->InitSSL();
		return inp;
	}
	else if(PCMATCHER == matchtype ){
		HDMPCRemote* inp = new HDMPCRemote(0,id,add);
		inp->setConf(getConf());
		inp->SetMatchManager(m_Mgr);
		inp->SetTimeOut(m_timeOut);
		inp->InitSSL();
		return inp;
	}
	else if(NWMATCHER==matchtype )
	{
		HDMatcher *inp= new HDMatcher(size,id,coarseFineMatch,maskcode);
		inp->setConf(getConf());
		inp->SetCommonBits(nominalCommonBits,minCommonBitsFine,minCommonBitsCoarse);
		inp->SetMaxCorruptBitsPercAllowed(maxCorrBitPer*1.0f);
		inp->SetMaskCode(maskval);
		inp->SetlowerNibble(lowernibble);
		inp->SetCompressedMatching(compressedMatching);
		if(debug) inp->SetDebug();
		inp->InitSSL();
		inp->StartMatchInterface(shift,greedy,thresh,coarsethresh,irissz);
		return inp;
	}
	else
	{
		throw "INI input is incorrect w.r.t REMOTE/LOCAL";
	}

}

