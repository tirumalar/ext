/*
 * MatchProcessor.h
 * Reads the input queue, processes and writes to output Queue
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef MATCHPROCESSOR_H_
#define MATCHPROCESSOR_H_

#include "HThread.h"
#include "CircularAccess.h"
#include "Safe.h"
#include "ProcessorChain.h"
#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include "EyeSegmentationInterface.h"
#include "MatchManagerInterface.h"

class BiOmega;
class Configuration;
class HTTPPOSTMsg;
class LEDDispatcher;
class NwDispatcher;
class SpoofDetector;
class F2FDispatcher;
class VarianceBasedDetection;
class NwMatchManager;
class IrisData;
typedef CircularAccess<Safe<HTTPPOSTMsg*> > MsgQueue;
typedef citerator< MsgQueue,	Safe< HTTPPOSTMsg* > &> MsgQueueIterator;


class MatchProcessor: public HThread, public ProcessorChain {
public:
	MatchProcessor(Configuration& conf);
	virtual ~MatchProcessor();
	bool enqueMsg(Copyable& msg);
	virtual unsigned int MainLoop();
	MatchManagerInterface * GetMatchManager(){ return m_matchManager;}
	const char *getName(){
			return "MatchProcessor";
	}
	void LogDetect(){ m_matchManager->LogDetect();}

	bool DoMatch(HTTPPOSTMsg *msg);
	void Matched();
	bool GetIriscode(unsigned char *frame,float* robostFeatureVar);
	bool Match(bool domatch = true);
	IrisData* SegmentEye(HTTPPOSTMsg *msg,float *var);
	void UpdateIrisData(HTTPPOSTMsg *msg,IrisData *irisData);

	void process(HTTPPOSTMsg *msg);
	bool MatchStatus();
	void SetLedDispatcher(LEDDispatcher *ptr){m_ledDispatcher = ptr;}
	void SetNwDispatcher(NwDispatcher *ptr){m_nwDispatcher = ptr;}

	void SetF2FDispatcher(F2FDispatcher *ptr){m_f2fDispatcher = ptr;}
	void SetNwMatchManager(NwMatchManager *ptr){m_nwMatchManager = ptr;}
	void ClearNwMatcherQ();

	LEDDispatcher* GetLedDispatcher(void){return m_ledDispatcher;}
	F2FDispatcher* GetF2FDispatcher(void){return m_f2fDispatcher;}

	MatchResult* GetMatchResult(){ return &mr;}
	void UpdateDB(char* dbptr,char*keyptr);
	void ReloadDB();
    void GetCropWH(int *width,int *height){
    	*width = m_width;
    	*height = m_height;
    }

    int AppendDB(char* left,char* right,char* name, char* Dbfilename);
	BiOmega* GetbioInstance(){return m_bioInstance;}
	float getScoreThresh(){ return m_scoreThresh;}

	float getScaleRatio();//expected/actual

	unsigned char *ResizeFrame(int width, int height, unsigned char *frame);

    int GetPCMatcher(){ return m_PCMatcher;}
    void FlushQueue(bool onlyEyes);
    bool CheckMessage(HTTPPOSTMsg *msg);
    bool CheckIrisFromSameFrame();
    void SendIrisFromSameFrame();
    void ClearIrisDataIndex(){ m_IrisDataIndex =0;}
    void DoSegmentation(HTTPPOSTMsg *msg);
    void CheckSegmentationThresholds(IrisData *irisData);
    int getIrisDataIndex() { return m_IrisDataIndex; }
    IrisData* getIrisDataIndexElement(int index) { return m_IrisData[index]; }
#ifdef UNITTEST
public:
#else
protected:
#endif
    virtual bool shouldWait();
    HTTPPOSTMsg *getNextMsgToProcess();
    bool SendForMatching(IrisData *irisData, float* varience);
    void MatchDetected(int indx, float res,uint64_t timestamp);

    void RunHDMDiagnostics();
    MsgQueue m_inQueue;
    MsgQueueIterator m_sendIter;
    HTTPPOSTMsg *m_HTTPMsg;
    QueueFullBehaviour m_queueFullBehaviour;
    BiOmega *m_bioInstance;
    SpoofDetector *m_spoofDetector;
    MatchManagerInterface *m_matchManager;
    LEDDispatcher *m_ledDispatcher;
    NwDispatcher *m_nwDispatcher;
    F2FDispatcher *m_f2fDispatcher;
    NwMatchManager *m_nwMatchManager;
    int m_width, m_height;
    float m_scoreThresh;
    unsigned int m_maskval;
    IplImage *m_scaleDest;
    IplImage *m_scaleSrcHeader;
    bool m_saveScaledImage;

	uint64_t m_TimeofLastAuthorization,m_RepeatAuthorizationPeriod;
    int m_LastAuthorizationID;
    struct timeval m_timer;
    enum ResizeType{ RESIZE_NONE, RESIZE_OPENCV, RESIZE_ASM_2x};
    ResizeType resizeType;
    int m_expectedIrisWidth;
    int m_actualIrisWidth;
    char *m_IrisCode;
    unsigned char *m_scratch;
    MatchResult mr;
	bool m_enablelog;
    bool m_flushQueueOnMatch;
	bool m_MatcherHDMStatus;
	int m_PingTimeStamp, m_PingInterval;
	bool m_Debug;
	VarianceBasedDetection *m_fVDetection;
	int m_PCMatcher;
	bool m_Master;
	bool m_spoofEnable;
	uint64_t m_CurrTimeStamp; // used by matchProcessor
	int m_dbIndex;	// the head of the queue matched with this
	float m_matchScore; // the head of the queue matched with this score
	unsigned char* m_IrisBuff;
	IrisData **m_IrisData;
	int m_irisQSize;
	int m_IrisDataIndex;
	int m_irisMinThreshX,m_irisMaxThreshX,m_irisMinThreshRad,m_irisMaxThreshRad;
	bool m_logging;
	bool m_SaveMatchInfo;
	bool m_OutdoorMatching;
	float m_OutdoorMatchThresh;
	int m_FeatureMask;
	float m_pupilzz;
	IplImage *m_EyeCrop;
	SoftwareType m_softwareType;
	bool m_EnableAusSeg;

};



#endif /* MATCHPROCESSOR_H_ */
