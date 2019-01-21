// EyeSortingLib.cpp : Defines the exported functions for the DLL application.
//
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include "EyeSorting.h"
#include "IrisSelector.h"
#include "ImageQualityChecker.h"
#include "EyeSegmentationInterface.h"
#include "NanoFocusSpecularityMeasure.h"
#include "getdata_bwCPP.h"
#include "LaplacianBasedFocusDetector.h"
#include "detectCorrupt.h"
#include "eyeSideDetectorLib.h"
#ifndef EL_IOS
#include <apr.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#endif
#include "opencv/highgui.h"


#ifdef _WIN32
	#if _MSC_VER == 1500
		#include "log4cxx/logger.h"
		#include "log4cxx/basicconfigurator.h"
		#include "log4cxx/helpers/exception.h"
		#include "log4cxx/propertyconfigurator.h"
		using namespace log4cxx;
		using namespace log4cxx::helpers;
		LoggerPtr logger(Logger::getLogger("EyeSortingLib"));
	#endif
#endif


EnrollmentServer::EnrollmentServer(int FeatureScale, IrisMatchInterface *pMatcher, int maxSpecularityValue, bool myris_Enroll) : m_EarlyTimeout(ENROLLMENT_EARLY_TIMEOUT)
	, m_GlobalTimeout(ENROLLMENT_GLOBAL_TIMEOUT)
	, m_pQchecker(0), m_bEnableQualityBasedRejection(false) 
	, m_MaxGrayScaleSpec(maxSpecularityValue), m_qualThreshScore(9.5), m_qualRatioMax(1.3), m_minIrisDiameter(190)
	, m_maxIrisDiameter(235), m_eyeSortingLogEnable(false)
	, m_minPupilDiameter(20), m_maxPupilDiameter(95)
	,m_maxXdelta(15),m_maxYdelta(30),m_irisimagecentremaxdist_X(120),m_irisimagecentremaxdist_Y(90)
	, m_corruptBitsPercentageThresh(60),m_featureVarianceAbsoluteThresh(1)	
{
	// Not integrated as part of the API at this point.  Can be changed via SortingLog.ini file
	m_LoggingDaysToKeep = 10;  //default

	// For Bigger Pupil
	temp_pEyeSegmentationInterface = new EyeSegmentationInterface(); 
	try		
	{
		temp_pEyeSegmentationInterface->init(FeatureScale);
	}
	catch (std::exception ex)
	{ 
		const char *pmessage = ex.what();

	}

	temp_pEyeSegmentationInterface->SetFeatureNormalization(false);
	temp_pEyeSegmentationInterface->SetPupilRadiusSearchRange(20,90);
	number_BigPupil = 0;
	imageCount=0;

	m_pIrisSelector = new IrisSelector(pMatcher, myris_Enroll);
	m_pIrisSelector->SetHDThreshold(0.2f);

	
	
	m_pIrisSelector->SetFeatureVarianceScaleIndex(0);
	m_focusServer = new NanoFocusSpecularityMeasure();
	m_focusServer->SetSpecularityValue(maxSpecularityValue);

	m_occlusionSoftThreshold = 15;
	m_occlusionHardThreshold = 25;
	m_calibrationPercentileThreshold = 25;
	m_calibrationAverageThreshold = 175;
	m_irisPupilRatioThreshold = 3;
	m_Feedback = new EyeSortingFeedback;

//	char temp[50];
//	sprintf_s(temp, 50, "%x", m_pIrisSelector);
//	OutputDebugStringA(temp);

#if 0 // Old code.		

	// Create our debug output dirctory... brute force, it never changes anyway...
	char* path = getenv("ALLUSERSPROFILE");
	char szFullPath[256];
	sprintf(szFullPath, "%s\\Eyelock Corporation", path);
	CreateDirectory(szFullPath, NULL);

	sprintf(szFullPath, "%s\\Eyelock Corporation\\EyeSortingImagesLog", path);
	CreateDirectory(szFullPath, NULL);

	sprintf(szFullPath, "%s\\Eyelock Corporation\\EyeSortingImagesLog\\Output", path);
	CreateDirectory(szFullPath, NULL);

	// Delete any existing files...
	char szDeletePath[256];

	sprintf(szDeletePath, "%s\\Cluster1.txt", szFullPath);
	remove(szDeletePath);
	sprintf(szDeletePath, "%s\\Cluster2.txt", szFullPath);
	remove(szDeletePath);
	sprintf(szDeletePath, "%s\\discarded.txt", szFullPath);
	remove(szDeletePath);
	sprintf(szDeletePath, "%s\\dump.txt", szFullPath);
	remove(szDeletePath);
	sprintf(szDeletePath, "%s\\BestEyes.txt", szFullPath);
	remove(szDeletePath);
#endif
}
void EnrollmentServer::SetEyeSegmentationInterface(EyeSegmentationInterface *pEyeSegmentationInterface) 
	{
		m_pEyeSegmentationInterface = pEyeSegmentationInterface;
	}
void EnrollmentServer::SetHaloScoreTopPoints(int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh)
	{
		m_noOfPixelsToConsider=noOfPixelsToConsider;
        m_topPixelsPercentage=topPixelsPercentage; 
        m_intensityThreshBP=intensityThreshBP; 
	    m_HaloThresh=HaloThresh; 
	}
EnrollmentServer::~EnrollmentServer() 
{
	if(temp_pEyeSegmentationInterface)
	{
		delete temp_pEyeSegmentationInterface;
	}

	if(m_pIrisSelector)
	{
		delete m_pIrisSelector;
	}
	if(m_pQchecker)
	{
		delete m_pQchecker;
	}
	if(m_focusServer)
		delete m_focusServer;

#ifdef _WIN32
	#if _MSC_VER == 1500
		LOG4CXX_INFO(logger, "Exiting EnrollmentServer Destructor.");
	#endif
#endif
}

void EnrollmentServer::SetEyeQualityClassifierWeights(float haloRankWeight, float fvRankWeight, float cbMaskRankWeight)
{
	m_pIrisSelector->SetEyeQualityClassifierWeights(haloRankWeight, fvRankWeight, cbMaskRankWeight);		
}
void EnrollmentServer::SetEyeSortingLogEnable(bool enable)
{
	m_eyeSortingLogEnable = enable;
	m_pIrisSelector->SetEyeSortingLogEnable(enable);		
}
void EnrollmentServer::SetOldHaloRankWeight(float oldHaloRankWeight)
{
	//char temp[50];
	//sprintf_s(temp, 50, "%x", m_pIrisSelector);
	//OutputDebugStringA(temp);
	m_pIrisSelector->SetOldHaloRankWeight(oldHaloRankWeight);		
}
void EnrollmentServer::SetLaplacianRankWeight(float laplacianRankWeight)
{
	m_pIrisSelector->SetLaplacianRankWeight(laplacianRankWeight);
}
void EnrollmentServer::SetMinEyesInCluster(int minEyesInCluster)
{
	m_pIrisSelector->SetMinEyesInCluster(minEyesInCluster);		
}
void EnrollmentServer::SetSpoofParams(bool enableSpoof, float threshX, float threshY, int SpoofPairDepth)
{
	m_pIrisSelector->SetSpoofParams(enableSpoof, threshX, threshY, SpoofPairDepth);
}
bool EnrollmentServer::ScaleFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio)
{
	try
	{
		IplImage dst;
		IplImage src;

		cvInitImageHeader(&dst, cvSize(width, height), IPL_DEPTH_8U, 1);
		cvSetData(&dst, output, widthStep);

		cvInitImageHeader(&src, cvSize(width, height), IPL_DEPTH_8U, 1);
		cvSetData(&src, input, widthStep);

		cvSetZero(&dst);

		CvRect scaleROI;
		scaleROI.width = width;
		scaleROI.height = height;
		scaleROI.x = 0;
		scaleROI.y = 0;
		
		CvRect scaleROI2;

		if (ratio > 1.0) 
		{
			scaleROI2.width = width / ratio;
			scaleROI2.height = height / ratio;
			scaleROI2.x = (width - scaleROI2.width) >> 1;
			scaleROI2.y = (height - scaleROI2.height) >> 1;

			// scaleROI2 is now smaller than scaleROI
			cvSetImageROI(&src, scaleROI2);
			cvSetImageROI(&dst, scaleROI);
		}
		else
		{
			scaleROI2.width = width * ratio;
			scaleROI2.height = height * ratio;
			scaleROI2.x = (width - scaleROI2.width) >> 1;
			scaleROI2.y = (height - scaleROI2.height) >> 1;

			// scaleROI2 is now smaller than scaleROI
			cvSetImageROI(&src, scaleROI);
			cvSetImageROI(&dst, scaleROI2);
		}

		cvResize(&src, &dst,2);  // can use different interpolation value here
#ifdef _WIN32
	#if _MSC_VER == 1500
		LOG4CXX_INFO(logger, "Scaling Frame");
	#endif	
#endif
		return true;
	}
	catch (...) 
	{
		return false;
	}
}

bool EnrollmentServer::ResizeFrame(unsigned char *input, int inwidth, int inheight, int instride, unsigned char *output, int outwidth, int outheight)
{
	try
	{
		int xi, yi, xo, yo, wi, hi;

		if (inwidth > outwidth)
		{
			xi = (inwidth - outwidth) / 2;
			wi = outwidth;
			xo = 0;
		}
		else
		{
			xi = 0;
			wi = inwidth;
			xo = (outwidth - inwidth) / 2;
		}

		if (inheight > outheight)
		{
			yi = (inheight - outheight) / 2;
			hi = outheight;
			yo = 0;
		}
		else
		{
			yi = 0;
			hi = inheight;
			yo = (outheight - inheight) / 2;
		}

		int outstride = outwidth;

		IplImage dst;
		IplImage src;

		cvInitImageHeader(&dst, cvSize(wi, hi), IPL_DEPTH_8U, 1);
		cvSetData(&dst, &output[yo*outstride+xo], outstride);

		cvInitImageHeader(&src, cvSize(wi, hi), IPL_DEPTH_8U, 1);
		cvSetData(&src, &input[yi*instride+xi], instride);

		cvSetZero(&dst);
		cvCopy(&src, &dst);
#ifdef _WIN32
#if _MSC_VER == 1500
		LOG4CXX_INFO(logger, "Resizing Frame");
#endif
#endif
		return true;
	}
	catch (...)
	{
		return false;
	}
}

int EnrollmentServer::GetMinIrisDiameter(void){ return m_minIrisDiameter;}
	int EnrollmentServer::GetMaxIrisDiameter(void){ return m_maxIrisDiameter;}

	int EnrollmentServer::GetMinPupilDiameter(void){ return m_minPupilDiameter;}
	int EnrollmentServer::GetMaxPupilDiameter(void){ return m_maxPupilDiameter;}

	int EnrollmentServer::GetMaxXdelta(void){ return m_maxXdelta;}
	int EnrollmentServer::GetMaxYdelta(void){ return m_maxYdelta;}

	int EnrollmentServer::GetMaxIrisImagecentreDistX(void){ return m_irisimagecentremaxdist_X;}
    int EnrollmentServer::GetMaxIrisImagecentreDistY(void){ return m_irisimagecentremaxdist_Y;}

	float EnrollmentServer::GetCorruptBitsPercentageThresh(void){ return m_corruptBitsPercentageThresh;}
	float EnrollmentServer::GetFeatureVarianceAbsoluteThresh(void){ return m_featureVarianceAbsoluteThresh;}
	void EnrollmentServer::SetCorruptBitsPercentageThresh(float corruptBitsAbsoluteThresh){ m_corruptBitsPercentageThresh = corruptBitsAbsoluteThresh; }
	void EnrollmentServer::SetFeatureVarianceAbsoluteThresh(float featureVarianceAbsoluteThresh){ m_featureVarianceAbsoluteThresh = featureVarianceAbsoluteThresh; }

void EnrollmentServer::SetIrisDiameterThresholds(int minIrisDiameter, int maxIrisDiameter)
	{
		m_minIrisDiameter = minIrisDiameter;
		m_maxIrisDiameter = maxIrisDiameter;
	}

void EnrollmentServer::SetPupilDiameterThresholds(int minPupilDiameter, int maxPupilDiameter)
	{
		m_minPupilDiameter = minPupilDiameter;
		m_maxPupilDiameter = maxPupilDiameter;
	}

void EnrollmentServer::SetXYDeltaThresholds(int maxXdelta, int maxYdelta)
	{
		m_maxXdelta = maxXdelta;
		m_maxYdelta = maxYdelta;
	}
void EnrollmentServer::SetIrisImagecentremaxDistance(int Irisimagecentramaxdistance_X ,int Irisimagecentramaxdistance_Y)
	{
		m_irisimagecentremaxdist_X = Irisimagecentramaxdistance_X;
		m_irisimagecentremaxdist_Y = Irisimagecentramaxdistance_Y;
	}


	void EnrollmentServer::EnableQualityBasedRejection(bool bEnable)
	{
		m_bEnableQualityBasedRejection=bEnable;
	}
void EnrollmentServer::SetQualThreshScore (float g) 
	{
		m_qualThreshScore=g;
	}
void EnrollmentServer::SetEarlyTimeout(uint64_t time) 
	{
		m_EarlyTimeout = time;
	}

	uint64_t EnrollmentServer::GetEarlyTimeout() const 
	{
		return m_EarlyTimeout;
	}

	void EnrollmentServer::SetGlobalTimeout(uint64_t time) 
	{
		m_GlobalTimeout = time;
	}

	uint64_t EnrollmentServer::GetGlobalTimeout() const 
	{
		return m_GlobalTimeout;
	}

	void EnrollmentServer::SetMaxGrayScaleSpec (int g) 
	{
		m_MaxGrayScaleSpec=g;
	}

	int EnrollmentServer::GetMaxGrayScaleSpec() 
	{
		return m_MaxGrayScaleSpec;
	}

	void EnrollmentServer::SetQualRatioMax(float g) 
	{
		m_qualRatioMax=g;
	}


EyeSegmentationInterface * EnrollmentServer::GetEyeSegmentationInterface() 
	{
		return m_pEyeSegmentationInterface;
	}
void EnrollmentServer::printIrisSelector()
	{
		//char temp[50];
		//sprintf_s(temp, 50, "->%x e->%x\r\n", m_pIrisSelector , m_pEyeSegmentationInterface);
		//OutputDebugStringA(temp);
	}
#if 0
void EyeSorting::Init()
{
}

void EyeSorting::Term()
{
	m_bIsRunning = false;
}

void EyeSorting::ScaleFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio)
{
	IplImage dst;
	IplImage src;

	cvInitImageHeader(&dst, cvSize(width, height), IPL_DEPTH_8U, 1);
	cvSetData(&dst, output, widthStep);

	cvInitImageHeader(&src, cvSize(width, height), IPL_DEPTH_8U, 1);
	cvSetData(&src, input, widthStep);

	cvSetZero(&dst);

	CvRect scaleROI;
	scaleROI.width = width;
	scaleROI.height = height;
	scaleROI.x = 0;
	scaleROI.y = 0;

	if (ratio > 1.0) 
	{
		CvRect scaleROI2;
		scaleROI2.width = width / ratio;
		scaleROI2.height = height / ratio;
		scaleROI2.x = (width - scaleROI2.width) >> 1;
		scaleROI2.y = (height - scaleROI2.height) >> 1;

		// scaleROI2 is now smaller than scaleROI
		cvSetImageROI(&src, scaleROI2);
		cvSetImageROI(&dst, scaleROI);
	}
	else
	{
		CvRect scaleROI2;
		scaleROI2.width = width * ratio;
		scaleROI2.height = height * ratio;
		scaleROI2.x = (width - scaleROI2.width) >> 1;
		scaleROI2.y = (height - scaleROI2.height) >> 1;

		// scaleROI2 is now smaller than scaleROI
		cvSetImageROI(&src, scaleROI);
		cvSetImageROI(&dst, scaleROI2);
	}

	cvResize(&src, &dst);  // can use different interpolation value here
}

// This is the constructor of a class that has been exported.
// see EyeSortingLib.h for the class definition
EyeSorting::EyeSorting() : m_Timestamp(0), m_GlobalTimeout(10000), m_LocalTimeout(2000), m_BestEye1(0), m_BestEye2(0), m_bIsRunning(false)
{
	return;
}

bool EyeSorting::FindBest(Iris *InputEye, Iris *&OutputEye1, Iris *&OutputEye2, unsigned int timestamp)
{
	if (!m_bIsRunning)
	{
		m_StartTime = timestamp;
		m_IrisSet.clear();
		m_bIsRunning = true;
	}

	// Add the iris pointer to the collection
	m_IrisSet.push_back(InputEye);

	IrisMatchInterface oIrisMatchInterface;
	oIrisMatchInterface.init();

	IrisSelector oIrisSelector(&oIrisMatchInterface);
	std::pair<Iris *, Iris *> eyes = oIrisSelector.Select( m_IrisSet );

	oIrisMatchInterface.term();

	// Check if we have observed a change in the best images to date
	bool updated = false;

	if(eyes.first != m_BestEye1)
	{
		updated = true;
		m_BestEye1 = eyes.first;
	}
	if(eyes.second != m_BestEye2)
	{
		updated = true;
		m_BestEye2 = eyes.second;
	}

	OutputEye1 = m_BestEye1;
	OutputEye2 = m_BestEye2;

	// If we haven't seen an improvement (i.e., update=false) in m_LocalTimeout ms then stop process
	// otherwise continue
	// If ( timestamp - m_StartTime > m_GlobalTimeout) then terminate
	// otherwise continue

	unsigned int elapsedSinceStart = timestamp - m_StartTime;
	if(elapsedSinceStart > m_GlobalTimeout)
		return m_bIsRunning;

	unsigned int elapsedSinceBest = timestamp - m_Timestamp;
	if(elapsedSinceBest > m_LocalTimeout)
		return m_bIsRunning;

	m_bIsRunning = false;

	return m_bIsRunning;
}


#endif

void EnrollmentServer::Clear() 
{
	m_BestPairOfEyes.first = 0;
	m_BestPairOfEyes.second = 0;

	for (int i = 0; i < m_EnrollmentEyes.size(); i++) 
	{
		delete[] m_EnrollmentEyes[i]->GetImage();
		delete[] m_EnrollmentEyes[i]->GetCode();
		delete m_EnrollmentEyes[i];
	}

	m_EnrollmentEyes.clear();
	if (m_pIrisSelector!= NULL) {
		m_pIrisSelector->ClearAll();
	}
#ifdef _WIN32
#if _MSC_VER == 1500
	LOG4CXX_INFO(logger, "Cleared all the Eyes in Vector of Iris's");
#endif
#endif
}

void EnrollmentServer::Begin(uint64_t time) 
{
#ifdef _WIN32
#if _MSC_VER == 1500
	LOG4CXX_INFO(logger, "Begin function Called");
#endif
#endif
	m_BeginTime = time;
	m_LastUpdateTime = 0;
	Clear();
}

bool EnrollmentServer::GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris)
{
	bool firstUpdated, secondUpdated = false;
	return GetBestPairOfEyes(image, time, output, iris, firstUpdated, secondUpdated);
}

bool EnrollmentServer::GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris, bool &firstUpdated, bool &secondUpdated) 
{
	return GetBestPairOfEyes(image, -1, -1, -1, -1, -1, -1.0f, -1, -1, -1, -1, -1, -1, 0, -1, time, output, iris, firstUpdated, secondUpdated, -1.0f,true, 0, -1);
}

// Added as a global because I didn't want to alter the class...
// This allows the dll to be used without recompile everywhere...
char m_UniqueFolderName[256];

bool EnrollmentServer::GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, 
	char *fileName, int diameter, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris, bool &firstUpdated, bool &secondUpdated
	,float haloScore,bool illumState, int side, float laplacianScore)
{
#ifdef _WIN32
#if _MSC_VER == 1500
	LOG4CXX_INFO(logger, "Entered GetBestPairOfEyes");
#endif
#endif
	firstUpdated = false;
	secondUpdated = false;

	bool terminate = false;
	bool hasEnrollmentAlready = (m_BestPairOfEyes.first != 0);
	
	if (image != NULL) 
	{
		if (!hasEnrollmentAlready) 
		{
			m_BeginTime = time; // reset the start time	
		}

		std::pair<Iris *, Iris *> result = GetBestPairOfEyesHelper(image, id, cameraId, frameId, imageId, numberOfImages, scale, x, y, width, height, score, maxValue, fileName, diameter, iris,haloScore,illumState, side, laplacianScore);

		// Check for any new results
		if (!((result.first == m_BestPairOfEyes.first) && (result.second == m_BestPairOfEyes.second))) 
		{
			if (result.first)
			{
				if (m_BestPairOfEyes.first || m_BestPairOfEyes.second)
				{
					if (result.first != m_BestPairOfEyes.first && result.first != m_BestPairOfEyes.second)
						firstUpdated = true;
				}
				else
					firstUpdated = true;
			}

			if (result.second)
			{
				if (m_BestPairOfEyes.first || m_BestPairOfEyes.second)
				{
					if (result.second != m_BestPairOfEyes.first && result.second != m_BestPairOfEyes.second)
						secondUpdated = true;
				}
				else
					secondUpdated = true;
			}

			output = result;
			m_BestPairOfEyes = result; // update our best result
			m_LastUpdateTime = time;
		}
	}

	// Check for global and early timeouts
	uint64_t elapsedGlobal = time - m_BeginTime;
	uint64_t elapsedEarly = time - m_LastUpdateTime;

	//printf("Elapsed global: %llu (now %llu)\n", elapsedGlobal, time);
	//fflush( stdout);

	if ((elapsedGlobal > m_GlobalTimeout) || (hasEnrollmentAlready && (elapsedEarly > m_EarlyTimeout))) 
	{
		terminate = true;
		
		std::vector<Iris*> temp_Iris = getEnrollment_Eyes();
		
		//Feedback printing (Occlusion)
		double Ave_Occ = 0,Ave_CalibPercentile=0,Ave_CalibAve=0;
		for (int i = 0; i < temp_Iris.size(); i++)
		{
			std::pair<double,double> CalibValues = temp_Iris[i]->GetCalib();

			Ave_Occ = Ave_Occ + temp_Iris[i]->GetOcclusion();
			Ave_CalibPercentile = Ave_CalibPercentile + CalibValues.first;
			Ave_CalibAve = Ave_CalibAve + CalibValues.second;
		}

		Ave_Occ = Ave_Occ/temp_Iris.size();
		Ave_CalibAve /= temp_Iris.size();
		Ave_CalibPercentile /= temp_Iris.size();

	
		if ((Ave_Occ > m_occlusionSoftThreshold) && (Ave_Occ < m_occlusionHardThreshold))
			m_Feedback->softOcclusion = true;
		else
			m_Feedback->softOcclusion = false;
		
		if (Ave_Occ > m_occlusionHardThreshold)
			m_Feedback->hardOcclusion = true;
		else
			m_Feedback->hardOcclusion = false;
		
		if ((Ave_CalibPercentile < m_calibrationPercentileThreshold) && (Ave_CalibAve < m_calibrationAverageThreshold))
			m_Feedback->calibration = true;
		else
			m_Feedback->calibration = false;

		IrisSelectorCircles circle1;
		IrisSelectorCircles circle2;

		if (m_BestPairOfEyes.first)
			circle1 = m_BestPairOfEyes.first->GetIrisSelectorCircles();
		if (m_BestPairOfEyes.second)
			circle2 = m_BestPairOfEyes.second->GetIrisSelectorCircles();
		
		if ((getnumber_BigPupil() > m_irisPupilRatioThreshold) && ((m_BestPairOfEyes.first && (circle1.IrisCircle.r / circle1.PupilCircle.r < 2.00)) || (m_BestPairOfEyes.second && (circle2.IrisCircle.r / circle2.PupilCircle.r < 2.00))))
			m_Feedback->irisPupilRatio = true;
		else
			m_Feedback->irisPupilRatio = false;

		if (m_eyeSortingLogEnable)
		{
			char buffer[200];
			char szFullPath[256];
			char szFullFilename[256];
			
#ifdef _WIN32
			sprintf(szFullPath, "%sGoodImages\\", GetLoggingBasePath());
			sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#else
			sprintf(szFullPath, GetLoggingBasePath());
			sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#endif
//			FILE *fDump = fopen(szFullFilename, "a");

//			fprintf(fDump, "\n HERE!  fullpath = %s;;;; imageCount=%d;  first=%x, second=%x", szFullPath, imageCount, m_BestPairOfEyes.first, m_BestPairOfEyes.second);
			// Write the ImageID and FrameID of the one or two BEST images...
			// Along with the resulting Feedback information...
			if (m_BestPairOfEyes.first)
			{
//						fprintf(fDump, "\n HERE first!");

				IplImage *imgHeader = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
				cvSetData(imgHeader, m_BestPairOfEyes.first->GetImage(), 640);

				sprintf(buffer, "%sBestImage_%d_first.pgm", szFullPath, m_BestPairOfEyes.first->GetImageId());
				if (!cvSaveImage(buffer, imgHeader)) ;//fprintf(fDump, "\n Unable to save Image:%s", buffer);
				else // Add info to our text file...
				{
//					fprintf(fDump, "\nFirst Image Saved:%s", buffer);
				}
				cvReleaseImageHeader(&imgHeader);
			}

			if (m_BestPairOfEyes.second)
			{
//						fprintf(fDump, "\n HEREsecond!");

				IplImage *imgHeader = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
				cvSetData(imgHeader, m_BestPairOfEyes.second->GetImage(), 640);

				sprintf(buffer, "%sBestImage_%d_second.pgm", szFullPath, m_BestPairOfEyes.second->GetImageId());
				if (!cvSaveImage(buffer, imgHeader)) ;//fprintf(fDump, "\n Unable to save Image:%s", buffer);
				else // Add info to our text file...
				{
//					fprintf(fDump, "\nSecond Image Saved:%s", buffer);
				}

				cvReleaseImageHeader(&imgHeader);
			}

//			fprintf(fDump, "\nDone", buffer);

//			fclose(fDump);

			//reset
			imageCount=0;
		}
	}

	return terminate;
}
bool swapColumn(unsigned char *data, int width,  int height, int step)
{   

	for (int i = 0; i < width-1; i = i+2)
	{
		char loc[100];
		for (int j = 0; j < height; j++)
		{
			//sprintf(loc,"i %d j %d\n", i, j);
			// OutputDebugStringA(loc);
			unsigned char tmp = data[ i  + j*step];
			data[i  + j*step] = data[ i +1 + j*step];
			data[i +1 + j*step]  = tmp;

		}
	}
	return true;
}
float EnrollmentServer::ComputeSwapHaloScoreTopPointsPico(unsigned char * frame, int specValue, int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh)
{
	//OutputDebugStringA("a");
	unsigned char * swapBuffer = new unsigned char[640*480];
	memcpy(swapBuffer, frame, (640 * 480));
	//	OutputDebugStringA("b");
	swapColumn(frame, 640, 480, 640);
	//	OutputDebugStringA("c");
	float ret = ComputeHaloScoreTopPointsPico(frame,specValue,noOfPixelsToConsider,topPixelsPercentage,intensityThreshBP,HaloThresh);
	//	OutputDebugStringA("d\n");
	delete [] swapBuffer;
	return ret;
}
float EnrollmentServer::ComputeHaloScoreTopPointsPico(unsigned char * frame, int specValue, int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh)
{
	IplImage * iplFrame = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
	cvSetData(iplFrame, frame, 640);
	float ret = m_focusServer->ComputeHaloScoreTopPointsPico(iplFrame,specValue,noOfPixelsToConsider,topPixelsPercentage,intensityThreshBP,HaloThresh);
	cvReleaseImageHeader(&iplFrame);
	return ret;
}
bool EnrollmentServer::DetectCorruptImage(unsigned char *img,int Width, int Height, double threshold, int lineLengthThresh, int borderX  , int borderY )
{
	IplImage * iplFrame = cvCreateImageHeader(cvSize(Width, Height), IPL_DEPTH_8U, 1);
	cvSetData(iplFrame, img, Width);

	bool ret = detectCorruptImage(iplFrame,  threshold,  lineLengthThresh,  borderX ,  borderY );

	cvReleaseImageHeader(&iplFrame);
	return ret;
}

int EnrollmentServer::eyeSideDetectionMyrisLib(unsigned char *img,int Width, int Height)
{
	IplImage * iplFrame = cvCreateImageHeader(cvSize(Width, Height), IPL_DEPTH_8U, 1);
	cvSetData(iplFrame, img, Width);

	int ret = eyeSideDetectionMyris(iplFrame);
	cvReleaseImageHeader(&iplFrame);
	return ret;
}
double EnrollmentServer::CalculateSwapLaplacianScore(unsigned char * frame, int Width, int Height)
{
	//OutputDebugStringA("a");
	unsigned char * swapBuffer = new unsigned char[Width*Height];
	memcpy(swapBuffer, frame, (Width * Height));
	//OutputDebugStringA("b");
	swapColumn(swapBuffer, Width, Height, Width);
	//OutputDebugStringA("c");
	double ret = CalculateLaplacianScore(swapBuffer,Width,Height);
	//OutputDebugStringA("d\n");
	delete [] swapBuffer;
	return ret;

}


double EnrollmentServer::CalculateLaplacianScore(unsigned char * frame, int Width, int Height)
{
	LaplacianBasedFocusDetector * calculator = new LaplacianBasedFocusDetector(Width, Height);

	
		
	IplImage * iplFrame = cvCreateImageHeader(cvSize(Width, Height), IPL_DEPTH_8U, 1);
	cvSetData(iplFrame, frame, Width);
	CvPoint3D32f pt = calculator ->ComputeRegressionFocus(iplFrame, 255);

	cvReleaseImageHeader(&iplFrame);
	delete calculator;
	return pt.x;
	//LaplacianScore  >   LAPLACIAN_THRESHOLD   ; // LAPLACIAN_THRESHOLD  = 0.2

}



// Assume 640x480 8bpp row major order image with 200 pixel iris
std::pair<Iris *, Iris *> EnrollmentServer::GetBestPairOfEyesHelper(unsigned char *image, Iris *&iris) 
{
	return GetBestPairOfEyesHelper(image, -1, -1, -1, -1, -1, -1.0f, -1, -1, -1, -1, -1, -1, 0, -1, iris, -1.0f,true, 0);
}

std::pair <double,double> EnrollmentServer::CalcCalibPara(int width,int height,unsigned char *dsty)
{
	//Percentile and Average Calculation
	unsigned char *dy = (unsigned char *)dsty;
	double total = 0,Ptotal = 0,percentile = 0,hist[256]={0},average=0;
	int pix=0,i,validPix=0;
	int n = width * height;
	int limit = 200;    // Lower limit for percentile calculation
	for (; n > 0; n--)
	{
		pix =(int) *dy;
		hist[pix]++;
		dy++;
		if (pix)
		{
			average=average+pix;
			validPix++;
		}
	}

	for(i=1;i<=255;i++)
	{
		float histValue = hist[i];
		total = total + (double)histValue;
		if(i>=limit)
			Ptotal = Ptotal + (double)histValue;
	}
	percentile = (Ptotal*100)/total;
	average=average/validPix;

	pair <double,double> Final;
	Final.first = percentile;
	Final.second = average;
	return Final;
	//printf("Average = %f Percentile =  %f \n",average,percentile);
}

bool EnrollmentServer::CheckForceLoggingFileExists(char *pszFullLoggingPath)
{
	//Attempt to open or special file
	FILE *handler = fopen("sortinglog.ini", "r");

	if (handler)
	{
		// Seek the last byte of the file
		fseek(handler, 0, SEEK_END);
		// Offset from the first to the last byte, or in other words, filesize
		int string_size = ftell(handler);
		// go back to the start of the file
		rewind(handler);

		// Read it all in one operation
		int read_size = fread(pszFullLoggingPath, sizeof(char), string_size, handler);

		// fread doesn't set it so put a \0 in the last position
		// and buffer is now officially a string
		*(pszFullLoggingPath+string_size) = '\0';

		return true;
	}
	else
		return false;
}


void EnrollmentServer::CleanupOldFolders(int nCurrentDayOfYear)
{
    apr_status_t rv;
    apr_finfo_t dirent;
    apr_dir_t *dir;
	apr_pool_t *mp;

	char szFullFilename[512];
	sprintf(szFullFilename, "%sCleaning.txt", "c:\\dev\\");
	FILE *fDump = fopen(szFullFilename, "a");

	fprintf(fDump, "\n1");
	fclose(fDump);

	apr_initialize();
    apr_pool_create(&mp, NULL);

fDump = fopen(szFullFilename, "a");

	fprintf(fDump, "\n2");
	fclose(fDump);
    if ((rv = apr_dir_open(&dir, GetLoggingBasePath(), mp)) != APR_SUCCESS)
	{
		apr_terminate();
        return;
	}


	fDump = fopen(szFullFilename, "a");
	fprintf(fDump, "\n3, Path = %s", GetLoggingBasePath());
	fclose(fDump);

    while ((apr_dir_read(&dirent, APR_FINFO_DIRENT|APR_FINFO_TYPE|APR_FINFO_NAME, dir)) == APR_SUCCESS)
	{
	fDump = fopen(szFullFilename, "a");
	fprintf(fDump, "\n4");
	fclose(fDump);


        if (dirent.filetype == APR_DIR)
		{
			fDump = fopen(szFullFilename, "a");
			fprintf(fDump, "\nFound Directory: %s", dirent.name);
	fclose(fDump);


            char *path;
            if (strcmp(dirent.name, ".") == 0 || strcmp(dirent.name, "..") == 0)
			{
                continue;
            }


            if ((rv = apr_filepath_merge(&path, GetLoggingBasePath(), dirent.name, 0, mp)) != APR_SUCCESS)
			{
				
			    apr_dir_close(dir);
				apr_terminate();
				return;/// failed
            }
			else // Ok, we have a directory... figure out if we need to recursively delete it...
			{
				fDump = fopen(szFullFilename, "a");
				fprintf(fDump, "\nFound Full Directory: %s", path);
				fclose(fDump);

				int nDaysOld;

				char szDayOfYear[4];

				if (strlen(dirent.name) < 3)
					continue;

				strncpy(szDayOfYear, dirent.name, 3);
				szDayOfYear[3] = '\0';

				fDump = fopen(szFullFilename, "a");
				fprintf(fDump, "\nDirectory Day: %s", szDayOfYear);
				fclose(fDump);

				int nFolderDay;
				try
				{
					nFolderDay = atoi(szDayOfYear);
									fDump = fopen(szFullFilename, "a");
				fprintf(fDump, "\natoi: %d", nFolderDay);
				fclose(fDump);

				}
				catch(std::exception ex)
				{
					continue;
				}

									fDump = fopen(szFullFilename, "a");
				fprintf(fDump, "\natoi SUCCESS: %d", nFolderDay);
				fclose(fDump);

				//A failure of some sort...
				if (nFolderDay == 0)
					continue;

				// Delete the older subdirectories and all of its contents...
				if (nCurrentDayOfYear < nFolderDay) // If we wrapped...
					nDaysOld = (365-nFolderDay) + nCurrentDayOfYear;
				else
					nDaysOld = nCurrentDayOfYear-nFolderDay;

							fDump = fopen(szFullFilename, "a");

				fprintf(fDump, "\nDayofYear=%d, nCurrentDayOfYear=%d, nDaysOld=%s", nFolderDay, nCurrentDayOfYear, nDaysOld);
				fclose(fDump);

				if (nDaysOld > m_LoggingDaysToKeep)
				{
							fDump = fopen(szFullFilename, "a");

					fprintf(fDump, "\nToo old... removing: %s", path);
			fclose(fDump);
								
					if (APR_SUCCESS != apr_dir_remove(path, mp))
					{
								fDump = fopen(szFullFilename, "a");

						fprintf(fDump, "\nFailed removing: %s", path);
						fclose(fDump);
						}
				}
			}

						fDump = fopen(szFullFilename, "a");

		fprintf(fDump, "\nFound NonDirectory: %s", dirent.name);
		fclose(fDump);

        }
	}

//	fclose(fDump);
	apr_dir_close(dir);
	apr_terminate();
}

 


std::pair<Iris *, Iris *> EnrollmentServer::GetBestPairOfEyesHelper(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, 
	int score, int maxValue, char *fileName, int diameter, Iris *&iris, float haloScore,bool illumState, int side, float laplacianScore) 
{
#ifdef _WIN32
#if _MSC_VER == 1500
	LOG4CXX_INFO(logger, "Entered GetBestPairOfEyesHelper");
#endif
#endif

	bool bBigPupil = false;


	if (imageCount == 0)
	{
		char szFullLoggingPath[1024];
		std::string newLogFilePath;

#ifdef _WIN32
		SetLoggingBasePath(getenv("ALLUSERSPROFILE")); // Default path is "c:\ProgramData"
		SetLoggingBasePath(strcat(GetLoggingBasePath(), "\\Eyelock Corporation\\EyeSortingImagesLog\\"));
#else
		SetLoggingBasePath("./EyeSortingImagesLog/");
#endif
		//Always check for the file, it may have a path override in it...
		if (CheckForceLoggingFileExists(szFullLoggingPath))
		{
			SetEyeSortingLogEnable(true); // Force on

			//Check for PATH...  format MUST be PATH=<theFullFolderPath>
			std::string key;
			std::istringstream thePath(szFullLoggingPath);

			if (std::getline(thePath, key, '='))
			{
				//For now we don't check the key, there is only expected to be one.
				// This code can be enhanced in the future to allow for other config options if necessary
				std::getline(thePath, newLogFilePath);
				if (newLogFilePath.length() > 0)
				{
#ifdef _WIN32
					newLogFilePath += "\\";
#else
					newLogFilePath += "/";
#endif
					SetLoggingBasePath(newLogFilePath.c_str());
				}
			}
		}
	}


	if (m_eyeSortingLogEnable)
	{
		//Generate unique foldername as rawtime, first time through only...
		if (imageCount == 0)
		{
			time_t rawtime;
			struct tm * timeinfo;

			time(&rawtime);
			timeinfo = localtime( &rawtime);

			// Also first time through only... Check all folder to see if any are too old to keep and delete them
		// DMOOUT for now... too dangerous given that path can be anything...	CleanupOldFolders(timeinfo->tm_yday); //[0-365]

#ifdef _WIN32
			sprintf(m_UniqueFolderName,"%03d%02d%02d%02d\\",timeinfo->tm_yday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); // NOTE: the ":" will not work, file name was "03-08-10 15" after output. Try using a different symbol, I'm going to use "+" in mine.
#else
			sprintf(m_UniqueFolderName,"%03d%02d%02d%02d/",timeinfo->tm_yday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); // NOTE: the ":" will not work, file name was "03-08-10 15" after output. Try using a different symbol, I'm going to use "+" in mine.
#endif
			SetLoggingBasePath(strcat(GetLoggingBasePath(), m_UniqueFolderName));

			m_pIrisSelector->SetLoggingBasePath(GetLoggingBasePath());
		}

		imageCount++;

#ifndef EL_IOS
		apr_pool_t *pool;
		apr_initialize();
		atexit(apr_terminate);
		apr_pool_create(&pool, NULL);
		apr_fileperms_t  perm = APR_FPROT_OS_DEFAULT;
		std::string theNewPath = GetLoggingBasePath();
		theNewPath += "GoodImages";
		apr_dir_make_recursive(theNewPath.c_str(),perm,pool);
		theNewPath = GetLoggingBasePath();
		theNewPath += "BadImages";
		apr_dir_make_recursive(theNewPath.c_str(),perm,pool);
		theNewPath = GetLoggingBasePath();
		theNewPath += "Clusters";
		apr_dir_make_recursive(theNewPath.c_str(),perm,pool);
		apr_pool_destroy(pool);
#endif
	}

	// Create an Iris record from the image
	int length = GetEyeSegmentationInterface()->GetFeatureLength();
	unsigned char *pCode = new unsigned char[length * 2];
	IrisPupilCircles irisPupilCircles;
	bool valid = GetEyeSegmentationInterface()->GetIrisCode(image, 640,	480, 640, pCode, pCode + length, &irisPupilCircles); /* circles optional, last */
	//Occlusion
	IplImage *FlatMask = cvCreateImage(cvSize(480,64),IPL_DEPTH_8U,1);
	GetEyeSegmentationInterface()->GetFlatMask((unsigned char*)FlatMask->imageData,FlatMask->width,FlatMask->height,FlatMask->widthStep);
	double Occlusion = ((double)cvCountNonZero(FlatMask))*100/(48*640);
	cvReleaseImage(&FlatMask);
	//Camera Calibration Check
	pair<double,double> CalibValues = CalcCalibPara(640,480,image);
	//
	int corruptBitCountMask = GetEyeSegmentationInterface()->checkBitCorruption(pCode+length);						
	/* If segmentation worked, then create a record and add it to our list */
	if (valid && GetEyeSegmentationInterface()->Getiseye()) 
	{
		if(m_bEnableQualityBasedRejection)
		{
			if(!IsImageAcceptable2(image,640,480,640,irisPupilCircles.ip))
			{
#ifdef _WIN32
#if _MSC_VER == 1500
				LOG4CXX_INFO(logger, "Rejecting image based on Image Quality Checker");
#endif
#endif
				std::cout << " Rejecting bad quality image" << std::endl;
				std::pair<Iris *, Iris *> temp = std::make_pair((Iris *)NULL,(Iris *)NULL);
				delete[] pCode; 
				return temp;
			}
		}
		unsigned char *pImage = new unsigned char[640 * 480];
		std::copy(image, image + 640 * 480, pImage);

		Iris *pIris = new Iris(pImage, pCode);


		pIris->GetIrisSelectorCircles().IrisCircle.x = irisPupilCircles.ip.x;
		pIris->GetIrisSelectorCircles().IrisCircle.y = irisPupilCircles.ip.y;
#ifdef _WIN32
		pIris->GetIrisSelectorCircles().IrisCircle.r = irisPupilCircles.ip.r;
#else
		pIris->GetIrisSelectorCircles().IrisCircle.r = irisPupilCircles.ip.r;
#endif
		int irisDiameter = irisPupilCircles.ip.r*2;
		int pupilDiameter = irisPupilCircles.pp.r*2;

		int maxXDelta = abs(irisPupilCircles.pp.x -irisPupilCircles.ip.x);
		int maxYDelta = abs(irisPupilCircles.pp.y -irisPupilCircles.ip.y);

		int irisImagecentre_distX = abs(320 - irisPupilCircles.ip.x);
		int irisImagecentre_distY = abs(240 - irisPupilCircles.ip.y);

		pIris->GetIrisSelectorCircles().PupilCircle.x = irisPupilCircles.pp.x;
		pIris->GetIrisSelectorCircles().PupilCircle.y = irisPupilCircles.pp.y;
#ifdef _WIN32
		pIris->GetIrisSelectorCircles().PupilCircle.r = irisPupilCircles.pp.r;
#else
		pIris->GetIrisSelectorCircles().PupilCircle.r = irisPupilCircles.pp.r;
#endif

		float variance[8];
#if EXP_FV
		GetEyeSegmentationInterface()->GetRatioFeatureVariances(variance);
#else
		GetEyeSegmentationInterface()->GetFeatureVariances(variance);
#endif
		IplImage *imgHeader = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, 1);
		cvSetData(imgHeader, image, 640);
		pIris->SetCorruptBit_Mask(corruptBitCountMask);
		float  fs;

		fs=haloScore; // Supplied from board
		float oldHalo=haloScore;
		if(fs==-3000.0f) // Pico EyeAssure Case
		{				
			fs = m_focusServer->ComputeHaloScoreTopPointsPico(imgHeader,GetMaxGrayScaleSpec(),m_noOfPixelsToConsider,m_topPixelsPercentage,m_intensityThreshBP,m_HaloThresh);							
			oldHalo = m_focusServer->GetOldHaloScore();
			pIris->SetOldHalo(oldHalo);
		}
		else // Nano Case EyEnroll
		{
			pIris->SetOldHalo(oldHalo); 
			fs = m_focusServer->ComputeHaloScoreTopPointsNano(imgHeader,GetMaxGrayScaleSpec(),m_noOfPixelsToConsider,m_topPixelsPercentage,m_intensityThreshBP,m_HaloThresh);			
		}

		if(fs==0.0f)
			fs=1.0f;

		CvPoint2D32f specCent=m_focusServer->GetSpecularityCentroid();
		//printf("Frame %d Halo %f\n",frameId,fs);	
		int corruptBitsAbsoluteThresh = (GetCorruptBitsPercentageThresh()/100)*10240;	
		bool flag = (haloScore!=-3000.0f || (fs!=-400.0f && oldHalo!=-1));	
		bool segmentOK = true;
		bool isBoundaryInValid = false;
#ifndef __linux__ //not implemented for embedded codebase yet
#ifndef EL_IOS
		segmentOK= GetEyeSegmentationInterface()->testSegmentation(image, width, height, 
			irisPupilCircles.pp.x, irisPupilCircles.pp.y, irisPupilCircles.pp.r, 
			irisPupilCircles.ip.x, irisPupilCircles.ip.y, irisPupilCircles.ip.r);
#endif
#ifdef __BOUNDARYCHECK__		
		IplImage *imageTmp = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, 1);
		cvSetImageData(imageTmp , image, 640);
		isBoundaryInValid = checkBoundary(imageTmp);
		cvReleaseImageHeader(&imageTmp);
#endif
#endif
		//throw "test";
		//printf("Frame %d segmentOK %d\n", frameId, segmentOK);

		//m_eyeSortingLogEnable = true; // Forced True for debug output...

		if(flag==true && segmentOK && (!isBoundaryInValid) && irisDiameter>GetMinIrisDiameter() && irisDiameter<GetMaxIrisDiameter() && corruptBitCountMask<corruptBitsAbsoluteThresh && variance[0]>GetFeatureVarianceAbsoluteThresh() && oldHalo<m_HaloThresh && pupilDiameter >GetMinPupilDiameter()  && pupilDiameter < GetMaxPupilDiameter() &&  maxXDelta < GetMaxXdelta() &&  maxYDelta < GetMaxYdelta() && irisImagecentre_distX < GetMaxIrisImagecentreDistX() && irisImagecentre_distY < GetMaxIrisImagecentreDistY() )
		{			
			//	printf("************ valid Iris Laplacian Score :%f\n",laplacianScore);
			//	fflush(stdout);
#ifdef _WIN32
#if _MSC_VER == 1500
			LOG4CXX_INFO(logger, "Image Passed all the criteria and passed to IrisSelector");
#endif
#endif
			cvReleaseImageHeader(&imgHeader);

			pIris->setFeatureVariances(variance);
			pIris->SetHaloScore(fs);
			pIris->SetLaplacianScore(laplacianScore);
			pIris->setSide(side);
			
			pIris->SetOcclusion(Occlusion);
			pIris->SetCalib(CalibValues.first,CalibValues.second);
			
			// Bigger Pupil Detection
			if (irisPupilCircles.ip.r / irisPupilCircles.pp.r < 2)
			{
				number_BigPupil++;
				bBigPupil = true;
			}

			IrisSelectorCircles circleTemp;
			circleTemp.IrisCircle.x = irisPupilCircles.ip.x;
			circleTemp.IrisCircle.y = irisPupilCircles.ip.y;
			circleTemp.IrisCircle.r = irisPupilCircles.ip.r;
			circleTemp.PupilCircle.x = irisPupilCircles.pp.x;
			circleTemp.PupilCircle.y = irisPupilCircles.pp.y;
			circleTemp.PupilCircle.r = irisPupilCircles.pp.r;
			pIris->SetIrisSelectorCircles(circleTemp);

			m_EnrollmentEyes.push_back(pIris);
			iris = pIris;
			iris->SetId(id);
			iris->SetCameraId(cameraId);
			iris->SetFrameId(frameId);
			iris->SetImageId(imageId);
			iris->SetNumberOfImages(numberOfImages);
			iris->SetScale(scale);
			iris->SetX(x);
			iris->SetY(y);
			iris->SetWidth(width);
			iris->SetHeight(height);
			iris->SetScore(score);
			iris->SetMaxValue(maxValue);
			iris->SetFileName(fileName);
			iris->SetDiameter(irisDiameter);
			iris->SetSpecCentroid(specCent.x,specCent.y);
			iris->SetPrevEyeIdx(-1);
			iris->SetIllumState(illumState);
			iris->setSide(side);
			iris->SetIrisSelectorCircles(circleTemp);

			if (m_eyeSortingLogEnable)
			{
				char buffer[200];
				char szFullPath[256];
				char szFullFilename[256];

#ifdef _WIN32
				sprintf(szFullPath, "%sGoodImages\\", GetLoggingBasePath());
				sprintf(szFullFilename, "%sGoodImages.txt", szFullPath);
		//DMOOLD		sprintf(szFullPath, "%s\\Eyelock Corporation\\EyeSortingImagesLog\\Output\\%s\\", path, m_UniqueFolderName);
		//DMOOLD		sprintf(szFullFilename, "%sGoodImages.txt", szFullPath);
#else
				sprintf(szFullPath, "%sGoodImages/", GetLoggingBasePath());
				sprintf(szFullFilename, "%sGoodImages.txt", szFullPath);
		//DMOOLD		sprintf(szFullPath, "./EyeSortingImagesLog/Output/");
		//DMOOLD		sprintf(szFullFilename, "%sGoodImages.txt", szFullPath);
#endif

				FILE *fDump = fopen(szFullFilename, "a");

#if 0 //DMO for now we don't save images...
				// Let it try to save the image... on Windows it'll fail... just write the dump file instead...					
//				sprintf(buffer, "./EyeSortingImagesLog/Output/GoodImages/Image_%u_%u_%u_%u_%f_%d_%f_%d.pgm", id, cameraId, frameId, imageId, laplacianScore, imageCount, fs, corruptBitCountMask);

				sprintf(buffer,"%sImage_%d_%d.pgm",szFullPath, imageCount, frameId);
				if (!cvSaveImage(buffer, imgHeader)) printf("\n Unable to save Image");
				else
#endif
				{
					fprintf(fDump, "===================================================\n"
							"Good Image not currently saved to disk\n"
							"ImageCount=%d\n"
							"FrameID=%d\n"
							"LaplacianScore=%f\n"
							"BigPupil=%s\n"
							"BigPupilCount=%d\n"
							"Iris Diameter=%d\n"
							"Pupil Diameter=%d\n"
							"deltax =%d\n"
							"deltaY =%d\n"
							"irisImagecentre_distX = %d\n"
							"irisImagecentre_distY = %d\n"
							"pupil diameterminthresh =%d\n"
							"pupil diametermaxthresh =%d\n"
							"deltaxthresh =%d\n"
							"deltaYthresh =%d\n"
							"irisImagecentre_distXthresh = %d\n"
							"irisImagecentre_distYthresh = %d\n"
							"OHalo=%f\n"
							"OHaloPC=%f\n"
							"MHalo=%f\n"
							"Count=%d\n"
							"FV0=%f\n"
							"CB_Mask=%d\n"
							"BPDiff=%f\n"
							"nBP=%d\n"
							"AvgIntTP=%f\n"
							"nTP=%d\n"
							"===================================================\n\n",
							imageCount, frameId, laplacianScore, bBigPupil ? "true":"false", number_BigPupil, 
							irisDiameter, pupilDiameter, maxXDelta, maxYDelta,
							irisImagecentre_distX, irisImagecentre_distY, m_minPupilDiameter,
							m_maxPupilDiameter, m_maxXdelta, m_maxYdelta,
							m_irisimagecentremaxdist_X, m_irisimagecentremaxdist_Y,
							oldHalo, m_focusServer->GetOldHaloScore(), fs,
							m_focusServer->GetHaloPixelCount(), variance[0], corruptBitCountMask,
							m_focusServer->GetModifiedBottomPointsDiff(),
							m_focusServer->GetNoOfBottomPoints(), m_focusServer->GetAvgIntensityTP(),
							m_focusServer->GetNoOfTopPoints());
				}
				fclose(fDump);
			}
		}
		else
		{
#ifdef _WIN32
#if _MSC_VER == 1500
			LOG4CXX_INFO(logger, "Image Discared due to Diameter or Halo or CorruptBits");
#endif
#endif
			//Probability of having large pupil
			int length1 = GetEyeSegmentationInterface()->GetFeatureLength();
			unsigned char *pCode1 = new unsigned char[length * 2];
			IrisPupilCircles irisPupilCircles1;
			bool valid1 = temp_pEyeSegmentationInterface->GetIrisCode(image, 640,	480, 640, pCode1, pCode1 + length1, &irisPupilCircles1); /* circles optional, last */
			
			int corruptBitCountMask1 = temp_pEyeSegmentationInterface->checkBitCorruption(pCode1+length1);						
			/* If segmentation worked, then create a record and add it to our list */
			if (valid1 && temp_pEyeSegmentationInterface->Getiseye()) 
			{
				int irisDiameter1 = irisPupilCircles1.ip.r*2;
				int pupilDiameter1 = irisPupilCircles1.pp.r*2;
				int maxXDelta1 = abs(irisPupilCircles1.pp.x -irisPupilCircles1.ip.x);
				int maxYDelta1 = abs(irisPupilCircles1.pp.y -irisPupilCircles1.ip.y);

				int irisImagecentre_distX1 = abs(320 - irisPupilCircles1.ip.x);
				int irisImagecentre_distY1 = abs(240 - irisPupilCircles1.ip.y);

				float variance1[8];
#if EXP_FViance
		temp_pEyeSegmentationInterface->GetRatioFeatureVariances(variance1);
#else
		temp_pEyeSegmentationInterface->GetFeatureVariances(variance1);
#endif

				bool segmentOK1 = true;
				bool isBoundaryInValid1 = false;

#ifndef __linux__ //not implemented for embedded codebase yet
#ifndef EL_IOS
				segmentOK1= temp_pEyeSegmentationInterface->testSegmentation(image, width, height, 
					irisPupilCircles1.pp.x, irisPupilCircles1.pp.y, irisPupilCircles1.pp.r, 
					irisPupilCircles1.ip.x, irisPupilCircles1.ip.y, irisPupilCircles1.ip.r);
#endif
#endif
				if (segmentOK1  /*&& irisDiameter1>GetMinIrisDiameter() && irisDiameter1<GetMaxIrisDiameter() && corruptBitCountMask1<corruptBitsAbsoluteThresh && variance1[0]>GetFeatureVarianceAbsoluteThresh() && oldHalo<m_HaloThresh && pupilDiameter1 >GetMinPupilDiameter()  && pupilDiameter1 < GetMaxPupilDiameter() &&  maxXDelta1 < GetMaxXdelta() &&  maxYDelta1 < GetMaxYdelta() && irisImagecentre_distX1 < GetMaxIrisImagecentreDistX() && irisImagecentre_distY1 < GetMaxIrisImagecentreDistY()*/ )
				{	
					if (irisPupilCircles1.ip.r / irisPupilCircles1.pp.r < 2)
					{
						bBigPupil = true;
						number_BigPupil++;
					}
				}
			}	
			delete[] pCode1;

			if (m_eyeSortingLogEnable)
			{
				char buffer[200];
				char szFullPath[256];
				char szFullFilename[256];


#ifdef _WIN32
				sprintf(szFullPath, "%sBadImages\\", GetLoggingBasePath());
				sprintf(szFullFilename, "%sBadImages.txt", szFullPath);
#else
				sprintf(szFullPath, "%sBadImages/", GetLoggingBasePath());
				sprintf(szFullFilename, "%sBadImages.txt", szFullPath);
#endif

#if 0
#ifdef _WIN32
				char* path = getenv("ALLUSERSPROFILE");

				sprintf(szFullPath, "%s\\Eyelock Corporation\\EyeSortingImagesLog\\Output\\%s\\", path, m_UniqueFolderName);
				sprintf(szFullFilename, "%sBadImages.txt", szFullPath);
#else
				sprintf(szFullPath, "./EyeSortingImagesLog/Output/");
				sprintf(szFullFilename, "%sBadImages.txt", szFullPath);
#endif
#endif

				FILE *fDump = fopen(szFullFilename,"a");

				//BadImages are saved
				sprintf(buffer,"%sImage_%d_%d.pgm",szFullPath, imageCount, frameId);

				if (!cvSaveImage(buffer,imgHeader))
					printf("\n Unable to save Image");
				else 
				{
					fprintf(fDump,"========================================================\n"
							"BadImageFileName: %s\n"
							"ImageCount=%d\n"
							"FrameID=%d\n"
							"LaplacianScore=%f\n"
							"BigPupil=%s\n"
							"BigPupilCount=%d\n"
							"Iris Diameter=%d\n"
							"Pupil Diameter =%d\n"
							"deltax =%d\n"
							"deltaY =%d\n"
							"irisImagecentre_distX = %d\n"
							"irisImagecentre_distY = %d\n"
							"OHalo=%f\n"
							"OHaloPC=%f\n"
							"MHalo=%f\n"
							"HaloPC=%d\n"
							"FV0=%f\n"
							"CB_Mask=%d\n"
							"BPDiff=%f\n"
							"nBP=%d\n"
							"AvgIntTP=%f\n"
							"nTP=%d\n"
							"===============================================================\n\n",
							buffer, imageCount, frameId, laplacianScore, 
							bBigPupil?"true":"false", number_BigPupil,
							irisDiameter, pupilDiameter,
							maxXDelta,maxYDelta,irisImagecentre_distX,irisImagecentre_distY,
							oldHalo,m_focusServer->GetOldHaloScore(),fs,
							m_focusServer->GetHaloPixelCount(),variance[0],
							corruptBitCountMask,m_focusServer->GetModifiedBottomPointsDiff(),
							m_focusServer->GetNoOfBottomPoints(),m_focusServer->GetAvgIntensityTP(),
							m_focusServer->GetNoOfTopPoints());

				//	fprintf(fDump,"\n flag==true: %d && !isBoundaryInValid %d && segmentOK %d && irisDiameter>GetMinIrisDiameter() %d && irisDiameter<GetMaxIrisDiameter() %d && pupilDiameter < GetMinPupilDiameter() %d && pupilDiameter > GetMaxPupilDiameter() %d && maxXDelta >GetMaxXdelta() %d && maxYDelta > GetMaxYdelta() %d && corruptBitCountMask<corruptBitsAbsoluteThresh %d && variance[0]>GetFeatureVarianceAbsoluteThresh() %d && oldHalo<m_HaloThresh %d\n\n",
				//		flag==true, (!isBoundaryInValid), segmentOK , irisDiameter>GetMinIrisDiameter(), irisDiameter<GetMaxIrisDiameter(), pupilDiameter < GetMinPupilDiameter(),pupilDiameter > GetMaxPupilDiameter(), maxXDelta >GetMaxXdelta(), maxYDelta >GetMaxYdelta(),corruptBitCountMask<corruptBitsAbsoluteThresh, variance[0]>GetFeatureVarianceAbsoluteThresh(), oldHalo<m_HaloThresh);
				}
				fclose(fDump);		
			}
			printf("\nImage Discarded due to Diameter or CorruptBits or HaloThresh\n");
			cvReleaseImageHeader(&imgHeader);
			delete[] pCode;
			delete[] pImage;
			delete pIris;
			iris = 0;
		}
	} 
	else 
	{  	
		delete[] pCode;
		iris = 0;
	}

#if 0
	IrisSelector is(GetMatchInterface());
	is.SetHDThreshold(0.2f);
	is.SetFeatureVarianceScaleIndex(0);
	return is.Select(m_EnrollmentEyes);
#else
	// Move local IrisSelector to member variable to allow access to intermediate results:
	//m_pIrisSelector->Clear();
	if(false)//m_eyeSortingLogEnable)
	{
		char szFullPath[256];
		char szFullFilename[256];


#ifdef _WIN32
				sprintf(szFullPath, "%sGoodImages\\", GetLoggingBasePath());
				sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#else
				sprintf(szFullPath, "%sGoodImages/", GetLoggingBasePath());
				sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#endif

#if 0
#ifdef _WIN32
		char* path = getenv("ALLUSERSPROFILE");

		sprintf(szFullPath, "%s\\Eyelock Corporation\\EyeSortingImagesLog\\Output\\%s\\", path, m_UniqueFolderName);
		sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#else
		sprintf(szFullPath, "./EyeSortingImagesLog/Output/");
		sprintf(szFullFilename, "%sBestEyes.txt", szFullPath);
#endif
#endif

		std::pair<Iris *, Iris *> result = m_pIrisSelector->Select(m_EnrollmentEyes);
		FILE *fDump2 = fopen(szFullFilename,"a");

		if (NULL != fDump2)
		{
			if(result.first)
			{
				std::cout<<"\n Cluster1_Best ID="<< result.first->GetId()<< " Halo=" << 
					result.first->GetHaloScore();
				fprintf(fDump2,"\nCluster1_Best: Image_%d_%d.pgm",result.first->GetId(), imageCount);
			}
			if(result.second)
			{
				std::cout<<"\n Cluster2_Best ID="<< result.second->GetId()<< " Halo=" << 
					result.second->GetHaloScore();
				fprintf(fDump2,"\nCluster2_Best: Image_%d_%d.pgm",result.second->GetId(), imageCount);
			}
			fclose(fDump2);
		}
		return result;
	}
	else
	{
		return m_pIrisSelector->Select(m_EnrollmentEyes);
	}
#endif
}

std::vector< std::vector<Iris *> * >& EnrollmentServer::GetRankedEyeClusters()
{
	return m_pIrisSelector->GetRankedEyeClusters();
}

bool EnrollmentServer::IsImageAcceptable2(unsigned char *input, int inwidth, int inheight, int instride, CircleParameters1 irisCircleParams)
{
	if(m_pQchecker==NULL){
		m_pQchecker= new ImageQualityChecker(inwidth,inheight,
			m_MaxGrayScaleSpec,m_qualThreshScore,m_qualRatioMax);
	}

	IrisParameters irisParams;
	irisParams.x = irisCircleParams.x;
	irisParams.y = irisCircleParams.y;
	irisParams.r = irisCircleParams.r;
	return 0==m_pQchecker->checkQualityNano2(input,inwidth,inheight,instride,irisParams);
}

bool EnrollmentServer::CheckSpoof()
{
	return m_pIrisSelector->CheckSpoof();
}

std::vector<Iris*> EnrollmentServer::getEnrollment_Eyes()
{
	return m_EnrollmentEyes;
}

int EnrollmentServer::getnumber_BigPupil()
{
	return number_BigPupil;
}


/*
*  Sample block illustrating how to use this in an application
*/
#if 0
bool EyeLockThread::Enrollment(unsigned char *pImage) 
{
	std::pair<Iris *, Iris *> result(0, 0);
	bool terminate = m_pEnrollmentServer->GetBestPairOfEyes(pImage,	Timer::GetTimeInMilliseconds(), result);
	fflush(stdout);
	if (terminate) 
	{
		m_Mode = m_PreviousMode;
		if (result.first) 
		{
			// Save all of the enrollment results for debugging
			if (result.first)
				savefile_OfSize_asPGM(result.first->GetImage(), 640, 480, "/tmp/image0.pgm");
			if (result.second)
				savefile_OfSize_asPGM(result.second->GetImage(), 640, 480, "/tmp/image1.pgm");
		}
	}

	return terminate;
}


EyeLockthread::Init()
{	
	// create the server somewhere
	m_pEnrollmentServer = new EnrollmentServer();

	// then we initialize the sucker with the matcher and segmentation server from biomega
	if (m_pEnrollmentServer) 
	{
		m_pEnrollmentServer->SetMatchInterface(m_MatchProcessor->GetbioInstance()->GetMatchInterface());
		m_pEnrollmentServer->SetEyeSegmentationInterface(m_MatchProcessor->GetbioInstance()->GetEyeSegmentationInterface());
	}
}

EyeLockThread::Loop()
{
	for(;;)
	{
		m_StartEnrollment = SHOULD_WE_START(); /* This should only return true one time for each enrollment */

		// If we are in enrollment mode then tell the enrollment to begin
		if (m_StartEnrollment) 
		{
			m_pEnrollmentServer->Begin(Timer::GetTimeInMilliseconds());
			m_StartEnrollment = false;
			m_DoEnrollment = true;
		}

		pImage = GET_CURRENT_IMAGE(); /* 640 x 480 at 200 pixel resolution */

		// In processing loop we call enroll
		bool terminate = false;
		if(m_DoEnrollment)
		{
			if(pImage)
			{
				terminate = m_pEnrollmentServer->Enrollment(pImage); /* Note: Enrollment function creates timestamp */
			}
			else
			{
				terminate = m_pEnrollmentServer->Enrollment(0); 
			}
		}

		// Discontinue enrollment if we have received a termination signal from enrollment above
		m_DoEnrollment = !terminate;
	}
}

#endif
