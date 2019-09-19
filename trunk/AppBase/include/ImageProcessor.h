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

#include <opencv/cv.h>
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
#include "CommonDefs.h"
#include "HTTPPOSTMsg.h"

#ifdef IRIS_CAPTURE
	#include "PostMessages.h"
	#include "EyeSortingWrap.h"
#endif

#include "FileConfiguration.h"
#include "FaceMap.h"

class IrisSelectServer;
class EyeTracker;
class Aquisition;
class FrameGrabber;
class SocketFactory;
class ProcessorChain;
class LaplacianBasedFocusDetector;
//class DecodeZBar;
#ifdef IRIS_CAPTURE
class CmxHandler;
#endif

typedef struct SpoofTrackerResult{
	int m_frIdx1,m_frIdx2;
	int m_eyIdx1,m_eyIdx2;
	int m_score1,m_score2;
};

typedef struct NanoSpoofResult{
	int m_frIndx,m_eyIndx;
	std::pair<float, float> m_score;
};

#define IRISCAM_MAIN_LEFT  01
#define IRISCAM_MAIN_RIGHT 02
#define IRISCAM_AUX_LEFT   129 // 0x81
#define IRISCAM_AUX_RIGHT  130 // 0x82

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
    bool ProcessImageMatchMode(IplImage *frame,bool matchmode);
    bool ProcessImageAcquisitionMode(IplImage *frame,bool matchmode);
    bool ProcessImageAcquisitionModeBKUP(IplImage *frame,bool matchmode);
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
	void setShouldDetectEyes(bool val){ if (checkLicense()) m_shouldDetect = val; }
	int getFrameType() { return m_testImageSendLevel;}
	bool getShouldDetectEyes() { return m_shouldDetect;}

	bool checkLicense();

	cv::Point2i ProjectPtr2(float x, float y, cv::Point2f constant, float ConstDiv);
	cv::Point2i projectPointsPtr1(cv::Rect projFace, cv::Point2f constant, float ConstDiv);
	cv::Rect CeateRect(cv::Point2i ptr1, cv::Point2i ptr2, bool useOffest, float projOffset);
	cv::Rect projectRectNew(cv::Rect face, int CameraId);
	FaceMapping GetFaceInfoFromQueueFacetoIrisMapping(int CameraId, char IrisFrameNo);
	cv::Rect seacrhEyeArea(cv::Rect no_move_area);
#ifndef UNITTEST    
protected:
#endif
	
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
    unsigned int m_faceIndex;
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
	bool m_IrisCameraPreview;
	bool m_EnableOffsetCorrection;
	IplImage *m_OffsetImageMainCamera1;
	IplImage *m_OffsetImageMainCamera2;
	IplImage *m_OffsetImageAuxCamera1;
	IplImage *m_OffsetImageAuxCamera2;
	IplImage *m_OffsetOutputImage;
	IplImage *m_ProcessImageFrame;

	// Calibration Parallex Correction
	float baselineDistance;
	float faceLensEFL;
	float auxLensEFL;
	float mainLensEFL;
	float pixelSize;
	float avgHeadWidth;
	float faceDistanceScale;
	float magFaceBaseline;
	float magAuxBaseline;
	float magMainBaseline;

	float faceDistance;
	cv::Point2f angCompMainl, angCompMainR, angCompAuxl, angCompAuxR;

	cv::Point2f camOffsetMainl, camOffsetMainR, camOffsetAuxl, camOffsetAuxR;

	int m_CalibrationImageSize;
	bool m_bCalibImageSizeIs1280;
	bool m_ParallaxCorrection;

	bool m_DebugTesting;
	std::string m_sessionDir;

	IplImage *m_IrisProjImage;
	bool m_SaveProjImage;
	bool m_OIMFTPEnabled;
	int rectX, rectY, rectW, rectH;
	bool mb1200ImageCalibration;
	int m1280shiftval_y;
	float m1280shiftvalmaincam_x, m1280shiftvalauxcam_x;
	float magOffMainl, magOffMainR, magOffAuxl, magOffAuxR;
	float magOffMainlDiv, magOffMainRDiv, magOffAuxlDiv, magOffAuxRDiv;
	cv::Point2f constantMainl, constantMainR, constantAuxl, constantAuxR;
	bool useOffest_m, useOffest_a;
	float projOffset_m, projOffset_a;
	bool m_showProjection;
	bool m_FaceIrisMapping;
	bool m_FaceIrisMappingStrict;
	bool m_FaceIrisMappingBeforEyeDetection;
	bool m_projStatus;
	bool m_bFaceMapDebug;;
	int m_minIrisDiameter;
	int m_Imagewidth;
	int m_Imageheight;
	bool m_EnableExtHalo;

	unsigned int m_EyelockIrisMode; // Select between Authentication and Capture Mode, by default Authentication Mode; Authentication=1 Capture=2
	bool m_CameraMode;
#ifdef IRIS_CAPTURE
public:
	HttpPostSender *m_pHttpPostSender;
	// Settings
	bool m_bIrisCapture;

	int m_IrisCaptureEarlyTimeout; //was sortingEarlyTimeout
	int m_IrisCaptureTimeout; // was sortingGlobalTimeout
	int m_IrisCaptureResetDelay; // was m_httpPostSenderDelay
	char *m_IrisImageFormat;
	int m_IrisCaptureDataMode;
	int m_IrisCaptureBestPairMax;
	int m_nMinIrisDiameter;
	int m_nMaxIrisDiameter;
	bool m_bIrisCaptureEnableResize;
	bool b_SaveBestEyes;
	int m_expectedIrisWidth;
	int m_actualIrisWidth;
	int m_MainCameraActualIrisWidth;
	bool m_bSortingLogEnable;

	int m_xPixelResolutionPerCM;
	int m_yPixelResolutionPerCM;

	int terminate;
	EyeSortingWrap *eyeSortingWrapObj;
	bool shouldIBeginSorting;

	bool m_EnableISOSharpness;
	int m_ISOSharpnessThreshold;

	IplImage *m_LastEyecrop;
	CmxHandler *m_pCMXHandler;

	IplImage *tmpImage;
	IplImage *imgHeader1;
	IplImage *imgHeader2;
	IplImage *m_scaleDest;
	IplImage *m_scaleSrcHeader;

	//functions
	IplImage * ResizeFrame(int width, int height, unsigned char *frame, int CameraId);
	void SetCMXHandler(CmxHandler *pCmxHandler);

#endif	// IRIS_CAPTURE
	bool m_bShouldSend;

	float ERROR_CHECK_EYES;

private:
    void extractSmallCropForFocus(int indx,DetectedEye * eye);
    FaceMapping sFaceMap;
    const char *eyeCropFileNamePattern;
	unsigned int m_eyeLabel;
    IplImage* OffsetImageCorrection(IplImage *frame, int cam_idd);
    void DebugSessionLogging(IplImage *frame, int cam_idd);
    FaceMapping DoFaceMapping(IplImage *frame, int cam_idd, int frame_number);
    bool ValidateEyeCropUsingFaceMapping(FaceMapping sFaceMap, int cam_idd, int m_faceIndex, int eyeIdx);
    void SaveEyeCrops(IplImage *eyeCrop, int cam_idd, int m_faceIndex, unsigned int eyeLabel);
    cv::Point2i projectPoints_IristoFace(cv::Point2i ptrI, cv::Point2f constant, float ConstDiv);
    bool GetFaceInfoForIristoFaceMapping(int CameraId, unsigned char IrisFrameNo, FaceImageQueue &FaceInfo);
    unsigned int validateLeftRightEyecrops(cv::Rect FaceCoord, cv::Point2i ptrI, int CameraId, unsigned char *faceImagePtr, int m_faceIndex);
	bool validateEyecrops_IrisToFaceMapping(cv::Rect projFace, cv::Point2i ptrI, int CameraId);
	bool ValidateEyeCropUsingIrisToFaceMapping(FaceMapping sFaceMap, int cam_idd, int m_faceIndex, int eyeIdx);
	int CalculateGainWithKH(int facewidth, int CameraId);
	bool m_activeEyeSideLabeling;
	bool m_IrisToFaceMapping;
	bool bIrisToFaceMapDebug;
	bool bIrisToFaceMapValid;
	unsigned int m_IrisToFaceMapCorrectionVal;
	bool m_DHSScreens;
	int m_LEDBrightness;
	cv::Mat Screen;
	IplImage *IplImageScreen1;
	IplImage *IplImageScreen2;
	IplImage *IplImageScreen3;
	unsigned int m_FaceFrameQueueSize;
    SocketFactory *m_socketFactory;
    bool n_bDebugFrameBuffer;
    bool m_AdaptiveGain;
    int m_AdaptiveGainFactor;
    float m_AdaptiveGainAuxAdjust;
    std::map<int,int> FaceWidthGainMap;
    void CreateFaceWidthGainMap();

    // Calibration Parallax
    float getUnscaledAngleComponent(float offsetBaseline, float cameraOffset, float magIrisBaseline, int irisSize, int faceSize);
    float getfaceDistance(cv::Rect FaceCoord);
    cv::Point2i projectPoints_IristoFace_Parallax(cv::Point2i ptrI, cv::Point2f camOffset, cv::Point2f angComp, float irisEFL, float distanceMM);
    int validateLeftRightEyecropsParallax(cv::Rect FaceCoord, cv::Point2i ptrI, int CameraId, unsigned char *faceImagePtr, int m_faceIndex);
};

#endif /* IMAGEPROCESSOR_H_ */
