#ifndef EYESORTING_H
#define EYESORTING_H

#include <vector>
#include <algorithm>
#pragma managed(push, off)
#include <opencv/cv.h>
#pragma managed(pop)
#include <opencv/cxcore.h>
#ifdef __linux__
#include <stdint.h>
#endif

#ifdef EL_IOS
    #include "IrisSelector.h"
#endif
#include "EyeSegmentationInterface.h"

#define IRIS_IMAGE_WIDTH 640
#define IRIS_IMAGE_HEIGHT 480
#define IRIS_CODE_LENGTH 2560

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EYESORTINGLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EYESORTINGLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef EL_IOS
class Iris;
class IrisSelector;
#endif
class ImageQualityChecker;
class NanoFocusSpecularityMeasure;

#define ENROLLMENT_EARLY_TIMEOUT 5000
#define ENROLLMENT_GLOBAL_TIMEOUT 12000




class EnrollmentServer {
public:
struct EyeSortingFeedback
{
	bool softOcclusion;
	bool hardOcclusion;
	bool calibration;
	bool irisPupilRatio;
};

	std::vector<Iris*> m_EnrollmentEyes;
	IrisMatchInterface *m_pMatchInterface;
	EyeSegmentationInterface *m_pEyeSegmentationInterface;
	EyeSegmentationInterface *temp_pEyeSegmentationInterface;

	std::pair<Iris *, Iris *> m_BestPairOfEyes;
	uint64_t m_BeginTime;
	uint64_t m_LastUpdateTime;
	uint64_t m_GlobalTimeout;
	uint64_t m_EarlyTimeout;

	

	EnrollmentServer(int FeatureScale, IrisMatchInterface *pMatcher, int maxSpecularityValue, bool myris_Enroll = true);
	~EnrollmentServer();

	void SetEyeSegmentationInterface(EyeSegmentationInterface *pEyeSegmentationInterface) ;
	//{
	//	m_pEyeSegmentationInterface = pEyeSegmentationInterface;
	//}
	void printIrisSelector();
	/*{
		char temp[50];
		sprintf_s(temp, 50, "%x", m_pIrisSelector);
		OutputDebugStringA(temp);
	}*/
	EyeSegmentationInterface * GetEyeSegmentationInterface() ;
	/*{
		return m_pEyeSegmentationInterface;
	}*/


	/*
	* Moved to constructor
	*
	void SetMatchInterface(IrisMatchInterface *pMatchInterface) 
	{
		m_pMatchInterface = pMatchInterface;
	}
	IrisMatchInterface * GetMatchInterface() 
	{
		return m_pMatchInterface;
	} */

	void SetEarlyTimeout(uint64_t time) ;
	/*{
		m_EarlyTimeout = time;
	}*/

	uint64_t GetEarlyTimeout() const ;
	/*{
		return m_EarlyTimeout;
	}*/

	void SetGlobalTimeout(uint64_t time) ;
	/*{
		m_GlobalTimeout = time;
	}*/

	uint64_t GetGlobalTimeout() const ;
	/*{
		return m_GlobalTimeout;
	}*/

	void SetMaxGrayScaleSpec (int g) ;
	/*{
		m_MaxGrayScaleSpec=g;
	}*/

	int GetMaxGrayScaleSpec() ;
	/*{
		return m_MaxGrayScaleSpec;
	}*/

	void SetQualRatioMax(float g) ;
	/*{
		m_qualRatioMax=g;
	}*/
	void SetHaloScoreTopPoints(int noOfPixelsToConsider=6, float topPixelsPercentage=25.0f, int intensityThreshBP=90, int HaloThresh=180);
	/*{
		m_noOfPixelsToConsider=noOfPixelsToConsider;
        m_topPixelsPercentage=topPixelsPercentage; 
        m_intensityThreshBP=intensityThreshBP; 
	    m_HaloThresh=HaloThresh; 
	}*/
	void SetQualThreshScore (float g) ;
	/*{
		m_qualThreshScore=g;
	}*/
	void SetSpoofParams(bool enableSpoof, float threshX, float threshY, int SpoofPairDepth);

	void EnableQualityBasedRejection(bool bEnable);
	/*{
		m_bEnableQualityBasedRejection=bEnable;
	}*/
	void SetEyeQualityClassifierWeights(float haloRankWeight, float fvRankWeight, float cbMaskRankWeight);
	void SetEyeSortingLogEnable(bool enable);
	void SetOldHaloRankWeight(float oldHaloRankWeight);
	void SetLaplacianRankWeight(float laplacianRankWeight);
	void SetMinEyesInCluster(int minEyesInCluster);
	void SetIrisDiameterThresholds(int minIrisDiameter, int maxIrisDiameter);
	/*{
		m_minIrisDiameter = minIrisDiameter;
		m_maxIrisDiameter = maxIrisDiameter;
	}*/
	float ComputeSwapHaloScoreTopPointsPico(unsigned char * frame, int specValue=255, int noOfPixelsToConsider=15, float topPixelsPercentage=25.0f, int intensityThreshBP=115, int HaloThresh=200); 
	float ComputeHaloScoreTopPointsPico(unsigned char * frame, int specValue=255, int noOfPixelsToConsider=15, float topPixelsPercentage=25.0f, int intensityThreshBP=115, int HaloThresh=200); 
	double CalculateSwapLaplacianScore(unsigned char * frame, int Width, int Height);
	double CalculateLaplacianScore(unsigned char * frame, int Width, int Height);
	void SetPupilDiameterThresholds(int minPupilDiameter, int maxPupilDiameter);
	void SetXYDeltaThresholds(int maxXdelta, int maxYdelta);

	void SetIrisImagecentremaxDistance(int Irisimagecentramaxdistance_X ,int Irisimagecentramaxdistance_Y);
	int GetMaxIrisImagecentreDistX(void);
    int GetMaxIrisImagecentreDistY(void);
	int GetMinIrisDiameter(void);//{ return m_minIrisDiameter;}
	int GetMaxIrisDiameter(void);//{ return m_maxIrisDiameter;}


	int GetMinPupilDiameter(void);//{ return m_minIrisDiameter;}
	int GetMaxPupilDiameter(void);//{ return m_maxIrisDiameter;}

	int GetMaxXdelta(void);//{ return m_minIrisDiameter;}
	int GetMaxYdelta(void);//{ return m_maxIrisDiameter;}


	float GetCorruptBitsPercentageThresh(void);//{ return m_corruptBitsPercentageThresh;}
	float GetFeatureVarianceAbsoluteThresh(void);//{ return m_featureVarianceAbsoluteThresh;}
	void SetCorruptBitsPercentageThresh(float corruptBitsAbsoluteThresh);//{ m_corruptBitsPercentageThresh = corruptBitsAbsoluteThresh; }
	void SetFeatureVarianceAbsoluteThresh(float featureVarianceAbsoluteThresh);//{ m_featureVarianceAbsoluteThresh = featureVarianceAbsoluteThresh; }
	
	void Clear() ;
	void Begin(uint64_t time) ;
	bool CheckForceLoggingFileExists(char *pszFullLoggingPath);
	void CleanupOldFolders(int nCurrentDayOfYear);
	void SetLoggingBasePath(const char *thePath) { strcpy(m_szBaseLoggingPath, thePath); }
	char *GetLoggingBasePath() { return m_szBaseLoggingPath; }
	bool GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris);
	bool GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris, bool &firstUpdated, bool &secondUpdated);
	bool GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, 
							char *fileName, int diameter, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris, bool &firstUpdated, bool &secondUpdated
							,float haloScore, bool illumState, int side, float laplacianScore = -1.0f);
	bool ScaleFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio);
	bool ResizeFrame(unsigned char *input, int inwidth, int inheight, int instride, unsigned char *output, int outwidth, int outheight);
	bool IsImageAcceptable2(unsigned char *input, int inwidth, int inheight, int instride, CircleParameters1 irisParams);
	std::vector< std::vector<Iris *> * >& GetRankedEyeClusters();
	
	bool CheckSpoof();

	bool DetectCorruptImage(unsigned char *image,int Width, int Height, double threshold, int lineLengthThresh, int borderX  = 50, int borderY = 90);
	int eyeSideDetectionMyrisLib(unsigned char *image, int Width, int Height);
	std::vector<Iris*> getEnrollment_Eyes();
	int getnumber_BigPupil();
	EyeSortingFeedback *m_Feedback;
	float m_occlusionSoftThreshold;
	float m_occlusionHardThreshold;
	float m_calibrationPercentileThreshold;
	float m_calibrationAverageThreshold;
	float m_irisPupilRatioThreshold;
private:

	int m_MaxGrayScaleSpec;
	float m_qualThreshScore;
	float m_qualRatioMax;
	bool m_bEnableQualityBasedRejection;
	int m_minIrisDiameter;
	int m_maxIrisDiameter;
	int  m_minPupilDiameter;
	int m_maxPupilDiameter;
	int m_maxXdelta;
	int m_maxYdelta;

	int imageCount;

	int m_irisimagecentremaxdist_X;
    int m_irisimagecentremaxdist_Y;

	float m_corruptBitsPercentageThresh;
	float m_featureVarianceAbsoluteThresh;
	IrisSelector *m_pIrisSelector;
	int m_noOfPixelsToConsider;
    float m_topPixelsPercentage; 
	int m_intensityThreshBP; 
	int m_HaloThresh; 
	NanoFocusSpecularityMeasure *m_focusServer;
	ImageQualityChecker* m_pQchecker;
	// Assume 640x480 8bpp row major order image with 200 pixel iris
	std::pair<Iris *, Iris *> GetBestPairOfEyesHelper(unsigned char *image, Iris *&iris);
	std::pair<Iris *, Iris *> GetBestPairOfEyesHelper(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, 
														int score, int maxValue, char *fileName, int diameter, Iris *&iris
														,float haloScore,bool illumState, int side, float laplacianScore = -1.0f);

	std::pair <double,double> CalcCalibPara(int width,int height,unsigned char *dsty);
	bool m_eyeSortingLogEnable;
	int m_LoggingDaysToKeep;
	int number_BigPupil;

	char m_szBaseLoggingPath[1024];
#if 0 //-- duplicate removed... left the non-private values above in place
	float m_occlusionSoftThreshold;
	float m_occlusionHardThreshold;
	float m_calibrationPercentileThreshold;
	float m_calibrationAverageThreshold;
	float m_irisPupilRatioThreshold;
#endif
};

#endif // EYESORTING_H
