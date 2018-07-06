#pragma once
#include <string>
#include "videoframe.h"
#include "EyeDetectorServer.h"
#include "EyeDetection.h"
#include "HModels.h"
#ifdef __BFIN__
#include "HaarClassification.h"
#endif

#define NUMBER_OF_LEVELS	4

#define EYE_DETECTION_LEVEL				2

#define HAAR_EYE_ZOOM					7.0
#define DO_HAAR_EYE_CLASSIFIER			1
#define DO_IGNORE_HAAR_EYE_CLASSIFIER	0

#define MATCHER_NEEDS_SPECULARITY_EYE	1
#define MATCHER_NEEDS_HAAR_EYE			1

#define DO_IRIS_FOCUS_SELECTION			1
#define DO_ENROLLMENT_IRIS_VERIFICATION	0
#define DO_ENROLLMENT_VOICE				1

#define IRIS_QUALITY_THRESHOLD								0.3906
#define IRIS_SPECULARITY_COVARIANCE_EIGENVALUE_THRESHOLD	7.0
#define IRIS_SPECULARITY_ECCENTRICITY_THRESHOLD				4.0

#define DO_SPOOF_SHARPNESS_TEST			0
#define DO_SPOOF_FFT_TEST				0

#define DO_ENROLLMENT					0

#define MAX_IRIS_HAMMING_DISTANCE	0.31
#define DO_ENFORCE_DETECTION_RANGE	0
#define MIN_DETECTION_DISTANCE		0.27
#define MAX_DETECTION_DISTANCE		0.37

#define DO_AMBIENT_CANCELLATION			0
#define	DO_TEMPORAL_ANTISPOOF			0

#if DO_IRIS_MATCH_THREAD
# define VIDEO_BUFFER_SIZE 25
# define VIDEO_DETECTION_BUFFER_SIZE 5
#else
# define VIDEO_DETECTION_BUFFER_SIZE 30
#endif


#define MATCH_SEQ_SAVE 1

typedef unsigned char BYTE;

class EyeDetectAndMatchServer
{
protected:
	char * scratch;	 // a shared scratch area used by various modules on BFin
	int scratch_size;
	bool m_SingleSpecMode;
	int m_DoCovarianceTestForDetection;
	int m_DoLevel0SpecularitySaturationTest;
	EyeDetectorServer *m_pEyeDetectionServer[NUMBER_OF_LEVELS];
public:
	EyeDetectAndMatchServer(int imageWidth, int imageHeight, int detlevel,std::string logfile);
	~EyeDetectAndMatchServer(void);
	bool DetectHaarEye(CSampleFrame *videoFrame, CEyeCenterPoint &eye);
	bool Detect(CSampleFrame *videoFrame, int level=EYE_DETECTION_LEVEL);
	bool IsPointAnEyeHaar(CSampleFrame *videoFramme, const CEyeCenterPoint &point);
	void EstimateDistance(CSampleFrame *videoFrame);
	void AllocImageThings(int imageWidth, int imageHeight);
	void DeleteImageThings();
	char *GetScratch() { return scratch;}	// so that we can reuse it
	void SetSingleSpecMode(bool bSingle){m_SingleSpecMode=bSingle;}
	bool IsSingleSpecMode(){return m_SingleSpecMode;}
	EyeDetectorServer *GetEyeDetector() { return m_pEyeDetectionServer[m_DetectionLevel];}
public:
	float m_IrisSpecularityCovarianceEigenvalueThreshold;
	float m_IrisSpecularityEccentricityThreshold;

#if DO_HAAR_EYE_CLASSIFIER
public:
	float GetHaarEyeZoom() const { return m_HaarEyeZoom; }
	void SetHaarEyeZoom(float haarEyeZoom) { m_HaarEyeZoom = haarEyeZoom; }
	int GetHaarImageShifts() const { return m_haarImageShifts;}
	int GetHaarImageSampling() const { return m_haarImageSampling;}
	void SetHaarImageShifts(int value)  { m_haarImageShifts=value;}
	void SetHaarImageSampling(int value)  { m_haarImageSampling=value;}

	void LoadHaarClassifier(const char *filename);

	void SetDoHaar(bool doHaar) { m_DoHaar = doHaar; }
	void SetDoIgnoreHaar(bool doIgnoreHaar) { m_DoIgnoreHaar = doIgnoreHaar; }
	bool CheckThatAllEyesAreReal(CSampleFrame &videoFrame, bool matchedEyesOnly = true);
protected:
	std::string m_sHaarClassifierFilename;
	Image8u *m_pBigEyeImage;
	Image8u *m_pEyeImage;

	int m_haarImageShifts;
	int m_haarImageSampling;

	bool m_DoHaar;
	bool m_DoIgnoreHaar;
	float m_HaarEyeZoom;

	// only one of these will be instantiated
	EyeDetectionServer *m_pHaarEyeDetectionServer;
#ifdef __BFIN__
	HaarClassification m_miplHaar;
#endif
	int m_index;
	int m_ImageWidth[NUMBER_OF_LEVELS], m_ImageHeight[NUMBER_OF_LEVELS];
	int m_HaarSearchImageWidth;
	int m_HaarSearchImageHeight;
	Image32f *m_HaarSearchImage;
	H3DSensor *m_pSensorModel;

	float m_MaxIrisHammingDistance;
	float m_DoEnforceDetectionRange;
	float m_MinDetectionDistance;
	float m_MaxDetectionDistance;
#endif
	bool m_DoDynamicZoom;
	int m_DetectionLevel;
	bool m_DebugAntiSpoof;
	bool m_MatcherNeedsSpecularityEye ;
	bool m_MatcherNeedsHaarEye;
public:
	void SetDebugAntispoof(bool debug) { m_DebugAntiSpoof = debug; }
	void SetMatcherNeedsSpecularityEye(bool matcherNeedsSpecularityEye) { m_MatcherNeedsSpecularityEye = matcherNeedsSpecularityEye; }
	bool GetMatcherNeedsSpecularityEye() const { return m_MatcherNeedsSpecularityEye; }
	void SetMatcherNeedsHaarEye(bool matcherNeedsHaarEye) { m_MatcherNeedsHaarEye = matcherNeedsHaarEye; }
	bool GetMatcherNeedsHaarEye() const { return m_MatcherNeedsHaarEye; }

	void SetCovTestForDetection(int b){	m_DoCovarianceTestForDetection = b;}
	void SetSpecCovEigenThresh(float inp){	m_IrisSpecularityCovarianceEigenvalueThreshold = inp;}
	void SetSpecEccThresh(float inp){	m_IrisSpecularityEccentricityThreshold = inp;}
	float GetSpecCovEigenThresh() const{	return m_IrisSpecularityCovarianceEigenvalueThreshold;}
	float GetSpecEccThresh() const {	return m_IrisSpecularityEccentricityThreshold ;}
	void SetLogFile(std::string a){
		m_LogFile = a;
		m_pEyeDetectionServer[m_DetectionLevel]->SetLogFile(m_LogFile);
	}

public:
	bool GetDoDynamicZoom() const { return m_DoDynamicZoom; }
	void SetDoDynamicZoom(bool doDynamicZoom) { m_DoDynamicZoom = doDynamicZoom; }

	bool m_HasFirstTimestamp;
	double m_FirstTimestamp;
	std::string m_LogFile;
#if MATCH_SEQ_SAVE
	bool m_DoSaveNeighbors;
#endif
};
