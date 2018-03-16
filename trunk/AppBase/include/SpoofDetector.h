/*
 * SpoofDetector.h
 *
 *  Created on: 14-Jan-2010
 *      Author: akhil
 */

#ifndef SPOOFDETECTOR_H_
#define SPOOFDETECTOR_H_
#include "HTTPPOSTMsg.h"
#include "Configuration.h"
#include "BiOmega.h"

class MatchManagerInterface;
class IrisData;
class BinMessage;
enum SPOOF_STRATEGY{ SPOOF_NONE,SPOOF_PERM,SPOOF_PERM_LR,SPOOF_LINEAR,SPOOF_FFT};
class SpoofDetector {
public:
	SpoofDetector(Configuration& conf, int width, int height,MatchManagerInterface *ptr);
	virtual ~SpoofDetector() {
		if(m_IrisBuff)free(m_IrisBuff);

	}
	virtual bool process(HTTPPOSTMsg *msg, unsigned char *frame, BiOmega *bio,float *robostFeatureVar,int domatch=1);
	virtual int getDBIndex(){ return m_dbIndex;}
	virtual float getMatchScore(){ return m_matchScore;}
	virtual uint64_t getMatchTimeStamp(){ return m_CurrTimeStamp;}
	virtual char *getIris(){ return (char*)m_IrisBuff;}
	virtual bool DoMatch(BinMessage *ptr);
	virtual bool GetIris(uint64_t ts, unsigned char *frame,float* robostFeatureVar);
	IrisData *GetIrisData(){ return m_irisData;}
private :
	IrisData *m_irisData;

#ifdef UNITTEST
public:
#else
protected:
#endif
	MatchManagerInterface *m_MatchMgr;
	unsigned char* m_IrisBuff;
	uint64_t m_CurrTimeStamp; // used by matchProcessor
	int m_dbIndex;	// the head of the queue matched with this
	float m_matchScore; // the head of the queue matched with this score
	int m_width;
	int m_height;
	float m_matchScoreThresh;
	bool m_enablelog;

};

#endif /* SPOOFDETECTOR_H_ */
