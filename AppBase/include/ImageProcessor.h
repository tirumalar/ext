/*
 * ImageProcessor.h
 *
 *  Created on: 19 Feb, 2009
 *      Author: akhil
 *
 *     main action of this project
 *     1) use a grabber to acquire images
 *     2) send them to the server
 *     3) process the response
 */

#ifndef IMAGEPROCESSOR_H_
#define IMAGEPROCESSOR_H_

#include <cv.h>
#include "Configurable.h"
#include "ImageMsgWriter.h"
#include "EyeDetectAndMatchServer.h"
#include "SafeFrameMsg.h"
#include "CircularAccess.h"
#include "DetectedEye.h"
#include "SimpleMotionDetection.h"
#include "CmxHandler.h"
#include "socket.h"
#include "NanoFocusSpecularityBasedSpoofDetector.h"
#include "HTTPPOSTMsg.h"
#include "CommonDefs.h"
#ifdef HBOX_PG
#include "PostMessages.h"
#endif
class IrisSelectServer;
class EyeTracker;
class Aquisition;
class FrameGrabber;
class SocketFactory;
class ProcessorChain;
class LaplacianBasedFocusDetector;
//class DecodeZBar;

typedef struct SpoofTrackerResult{
	int m_frIdx1,m_frIdx2;
	int m_eyIdx1,m_eyIdx2;
	int m_score1,m_score2;
};

typedef struct NanoSpoofResult{
	int m_frIndx,m_eyIndx;
	std::pair<float, float> m_score;
};

// Simple base class to define interface for low latency callbacks
class ImageHandler
{
public:
	ImageHandler() {}
	virtual int Handle(IplImage *frame) = 0;
};

class ImageProcessor: public Configurable {
public:
	ImageProcessor(Configuration* pConf, CircularAccess<SafeFrameMsg *>& outMsgQueue, SafeFrameMsg& detectedMsg, SafeFrameMsg& motionMsg);
	virtual ~ImageProcessor();
	bool process(bool match= true);
	void ChangeShiftandOffset(bool test);
	void SetHaloRelatedConf(unsigned short dcoff,unsigned short shift,float halothresh,unsigned short maxspecval);
public:
	void CopyToOneFourth(IplImage *frame,IplImage *out);
    void sendLiveImages();
    virtual void InitializeMotionDetection(Configuration *& pConf, int w, int h);
    virtual bool SendHB(bool bSentSomething);
    virtual void ComputeMotion();
    bool ShouldDetectEyes() const { return m_shouldDetect; }
    void GenMsgAfterDetect(BinMessage& msg);
    void GenMsgToNormal(BinMessage& msg);
    virtual FrameGrabber *GetFrameGrabber();
    virtual IplImage* GetFrame();
    int defBuffSize;
    IplImage *m_src;
    void SetImage(IplImage *ptr);
    bool ProcessImage(IplImage *frame,bool matchmode=true);
    bool ProcessSpoofFlowImage(IplImage *frame,bool matchmode=true);
    bool ClassifySpoof(DetectedEye *& eyel,DetectedEye *& eyer);
    void RotateSpoofEyes(IplImage *inp,IplImage* out);
    int GetTrackedCount(){ return m_NumTrackedEyes;}
    void ClearTrackedCount(){ m_NumTrackedEyes=0;}
    int GetFaceIndex(){ return m_faceIndex;}
#ifdef UNITTEST
    IplImage *GetSmallCropped(){
    	return m_smallCroppedEye;
    }
    IplImage *GetRippedImage(){
    	return m_rippedImage;
    }
#endif
    void setLatestFrame_raw(char *ptr);
    void clearFrameBuffer();
    void FixCoordinates(int x, int y, int &u, int &v, int width, int height);
    int ClearEyes(int faceindex,bool all = false);
    void ClearSpoofQ(){	m_SpoofTrackerCount = 0;m_nanoSpoofDataCount = 0;}
    int getSpoofTrackerResult(SpoofTrackerResult**ptr){ *ptr = m_SpoofTrackerResult;	return m_SpoofTrackerCount;}
    bool SendSpoofCheckEyes(bool & bSentSomething,int faceindex);
    void CheckConfusion(DetectedEye *eye,bool tobesend);
    void AddImageHandler(ImageHandler *pHandler);
    __int64_t getTS() { return m_timestampBeforeGrabbing;}

    SpoofTrackerResult m_SpoofTrackerResult[100];
    int m_SpoofTrackerCount;
    NanoSpoofResult m_nanoSpoofData[100];
    int m_nanoSpoofDataCount;
    int m_NumTrackedEyes;
    void  SetNwLEDDispatcher(ProcessorChain* ptr) {m_nwLedDispatcher = ptr;}
    void  SetLEDConsolidator(ProcessorChain *ptr) {m_LedConsolidator = ptr;}

	void setFrameType(int val){ m_testImageSendLevel = val;}
	void setShouldDetectEyes(bool val){ m_shouldDetect = val;}
	int getFrameType() { return m_testImageSendLevel;}
	bool getShouldDetectEyes() { return m_shouldDetect;}
	//void SetCmxHandler(CmxHandler *pCmxHandler) {m_pCmxHandler = pCmxHandler;}

#ifndef UNITTEST    
protected:
#endif
	//CmxHandler *m_pCmxHandler;
    bool m_tsDestAddrpresent;
    ProcessorChain *m_nwLedDispatcher;
    ProcessorChain *m_LedConsolidator;

    std::vector<ImageHandler *> m_ImageHandlers;

    void SendHBStatus(long int timestamp);
    void configureDetector();
    void logFocusScore(int score);
    void logResults(int frameIndex);
    void logFocusResults(std::map<std::pair<int,int> ,void*> focusResults);
    bool shouldSendHearBeat();
    void *write_single_eye();
    DetectedEye *getNextAvailableEyeBuffer();
    void incrNwQueueToNextAvailableBuffer();
    void sendToNwQueue(DetectedEye *info = 0);
    bool CheckHaloAndBLC(DetectedEye *eye);
    bool sendHeartBeat();
    void CheckForHaloScoreThreshold(CvPoint3D32f& halo);
	void SetMaxSpecValue(int val ){ m_currMaxSpecValue = val;}
	void SetHaloThreshold(float val ){ m_currHaloThreshold = val;}

    Aquisition *aquisition;
    ImageMsgWriter *writer;
    EyeDetectAndMatchServer *m_pSrv;
    IrisSelectServer *m_IrisSelectSvr;
    int m_IrisSelectFeatVect[9];
    EyeTracker *m_EyeTracker;
    int m_FocusBasedRejectionType;
    CvRect m_FocusRoi;
    IplImage *m_smallCroppedEye;
    CSampleFrame m_sframe;
    IplImage *m_rotationBuff;
    bool m_shouldRotate;
    Image8u m_inputImg;
    int m_faceIndex;
    int m_id;
    bool m_shouldDetect;
    int m_testImageSendLevel;
    bool m_shouldLog;
    int m_saveCount;
    int m_outSaveCount;
    unsigned long m_hbIntervalUSecs;
    struct timeval m_lastHBSent, m_tempTime;
    int m_bufSize;
    CircularAccess<SafeFrameMsg*> & m_outMsgQueue;
    SafeFrameMsg & m_DetectedMsg;
    SafeFrameMsg & m_MotionMsg;
    DetectedEyesQueue m_DetectedEyesQueue;
    DetectedEyesIterator m_DetectedEyeWriteIterator;
    int m_FocusImageLevel;
    IplImage *m_rippedImage;
    char *m_scratch;
    SimpleMotionDetection *m_MotionDetection;

	HostAddress *m_statusDestAddr;
	const char* m_msgFormat;
	int m_HBFreqSec;
	int m_HBTimeOutmilliSec;
	BinMessage m_outMsg;
	long int m_FuturisticTime;
	char *m_svrAddr;
	bool m_Debug;
	std::string m_LogFileName;
	int m_binType;
	int m_eyeDetectionLevel;
	int m_FlipType;
	bool m_spoofEnable,m_spoofDebug;
	__int64_t m_timestampBeforeGrabbing,m_prevTS,m_timegapForStaleframe;
	NanoFocusSpecularityBasedSpoofDetector *m_nanoSpoofDetector;
	float m_areaThreshold,m_focusThreshold,m_blackThreshold,m_blackMinThreshold;
	float m_haloThreshold,m_haloMinThreshold,m_haloMinCount,m_haloMaxCount;
	int m_haloCountByBottomPixels;
	float m_haloTopPixelsPercentage;
	int m_haloBottomPixelsIntensityThresh;
	int m_MHaloNegationThresh;
	int m_maxEyes;
	bool m_enableBlackLevel,m_enableAreaFocus,m_enableHaloThreshold;
	const char *m_Imageformat;
	int m_ImageMatchTime;
	int m_fileIlluState,m_fileIndex,m_maxFramesReset;
	std::string m_CamStr;
	bool m_saveDiscardedEyes;
	int m_discardedSaveCount;
	int m_il0;
	IplImage *m_secondLevelImage;
	uint64_t m_prevConfusionTS,m_confusionTimeThresholduSec,m_lastEyeConfusionTS;
	bool m_checkConfusion,m_confusionDebug;
	unsigned char m_ledPWMAddress,m_ledPWMDefaultValue,m_ledPWMDefaultValueAfterDetect;
	int m_currMaxSpecValue,m_maxSpecValue,m_maxSpecValueAfterDetect;
	float m_currHaloThreshold,m_haloThresholdAfterDetect;
	unsigned short m_DcOffset,m_ShiftRight;
	unsigned short m_DcOffsetAfterDetect,m_ShiftRightAfterDetect;

	IplImage *m_centreEyecrop,*m_centreEyecropEnroll;
	IplImage *m_rotatedEyecrop;

	LaplacianBasedFocusDetector *m_lapdetector,*m_lapdetectorEnroll;
	bool m_enableLaplacian_focus_Threshold,m_enableLaplacian_focus_ThresholdEnroll;
	float m_laplacian_focusThreshold,m_laplacian_focusThresholdEnrollment;
	volatile bool m_matchingmode,m_lapdebug;
	bool m_SaveEyeCrops;
	bool m_SaveFullFrame;
	bool m_EnableOffsetCorrection;
	bool m_camera1_offset_image_loaded;
	bool m_camera2_offset_image_loaded;
	IplImage *m_OffsetImageCamera1;
	IplImage *m_OffsetImageCamera2;
	IplImage *m_OffsetOutputImage;
#ifdef HBOX_PG	
	bool m_SPAWAREnable;

	HttpPostSender *m_pHttpPostSender;
#endif	
	bool m_bShouldSend;
private:
    void extractSmallCropForFocus(int indx,DetectedEye * eye);
    SocketFactory *m_socketFactory;

};

#endif /* IMAGEPROCESSOR_H_ */
