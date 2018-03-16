#include "EyeDetectAndMatchServer.h"
#include "CropWarp.h"


extern "C"{
	#include "file_manip.h"
	}
#ifdef __BFIN__
	#include <bfin_sram.h>
#endif

EyeDetectAndMatchServer::~EyeDetectAndMatchServer(void)
{
	DeleteImageThings();

#if DO_HAAR_EYE_CLASSIFIER
	if(m_pHaarEyeDetectionServer)
	{
		delete m_pHaarEyeDetectionServer;
		m_pHaarEyeDetectionServer = 0;
	}

	if(m_HaarSearchImage)
	{
		delete m_HaarSearchImage;
		m_HaarSearchImage = 0;;
	}

	if(m_pEyeImage)
	{
		delete m_pEyeImage;
		m_pEyeImage = 0;
	}
#endif

	if(m_pBigEyeImage)
	{
		delete m_pBigEyeImage;
		m_pBigEyeImage = 0;
	}

#ifdef __BFIN__
	if(scratch) sram_free(scratch);
#else
	if(scratch) free(scratch);
#endif
	scratch=0;


}

void EyeDetectAndMatchServer::DeleteImageThings()
{

	for(int level = 0; level < NUMBER_OF_LEVELS; level++)
	{
		if(m_pEyeDetectionServer[level])
		{
			delete m_pEyeDetectionServer[level];
			m_pEyeDetectionServer[level] = 0;
		}
	}
}


#if DO_HAAR_EYE_CLASSIFIER
#ifdef __BFIN__
void EyeDetectAndMatchServer::LoadHaarClassifier(const char *filename)
{
	m_sHaarClassifierFilename = filename;
	int dim = 24 + 2*m_haarImageShifts*m_haarImageSampling;
	if(!(m_pEyeImage && (m_pEyeImage->GetWidth() == dim) && (m_pEyeImage->GetHeight() == dim)))
	{
		m_pEyeImage	= new Image8u(dim, dim);
	}

	m_HaarSearchImageWidth = 13;
	m_HaarSearchImageHeight = 13;
	m_HaarSearchImage = 0;
	m_miplHaar.init(scratch,scratch_size,cvSize(dim,dim),cvSize(5,5),filename);
}


bool EyeDetectAndMatchServer::DetectHaarEye(CSampleFrame *videoFrame, CEyeCenterPoint &eye)
{
	bool isPointAHaarEye = false;
	float zoom = m_DoDynamicZoom ? ((m_HaarEyeZoom * videoFrame->GetFocusDistance()) / 120.0) : m_HaarEyeZoom;
	IppiPoint haarCenter;

	haarCenter.x = haarCenter.y = m_haarImageShifts*m_haarImageSampling;

	CropAndWarp(*videoFrame->GetPyramid(), eye.m_nCenterPointX, eye.m_nCenterPointY, (1.0f / zoom), *m_pEyeImage ,scratch);

	isPointAHaarEye=m_miplHaar.isHaarEye(m_pEyeImage->GetData());


	eye.m_HaarCenter.Set( haarCenter.x + eye.m_nCenterPointX - m_haarImageSampling*m_haarImageShifts,
						  haarCenter.y + eye.m_nCenterPointY - m_haarImageSampling*m_haarImageShifts);

	eye.m_HasHaarCenter = isPointAHaarEye;

	return isPointAHaarEye;
}

#else
void EyeDetectAndMatchServer::LoadHaarClassifier(const char *filename)
{
//	m_sHaarClassifierFilename.clear();
	m_sHaarClassifierFilename = filename;

	int dim = 24 + 2*m_haarImageShifts*m_haarImageSampling;

	if(!(m_pEyeImage && (m_pEyeImage->GetWidth() == dim) && (m_pEyeImage->GetHeight() == dim)))
	{
		m_pEyeImage	= new Image8u(dim, dim);
	}


	m_HaarSearchImageWidth = 13;
	m_HaarSearchImageHeight = 13;
	m_HaarSearchImage = 0;

//	m_HaarSearchImage = new Image32f(m_HaarSearchImageWidth, m_HaarSearchImageHeight);

	if(!m_pHaarEyeDetectionServer)
	{
		m_pHaarEyeDetectionServer = new EyeDetectionServer(scratch);

	}

	bool resp = m_pHaarEyeDetectionServer->SetNumFeatures(MAX_FEATURE_SELECTIONS);
	if(resp && m_sHaarClassifierFilename.c_str())
	{
		m_pHaarEyeDetectionServer->LoadFinalClassifiers( (char *)m_sHaarClassifierFilename.c_str());
	}
	m_pHaarEyeDetectionServer->InitValidationMode();
}

bool EyeDetectAndMatchServer::DetectHaarEye(CSampleFrame *videoFrame, CEyeCenterPoint &eye)
{
	bool isPointAHaarEye = false;
	float zoom = m_DoDynamicZoom ? ((m_HaarEyeZoom * videoFrame->GetFocusDistance()) / 120.0) : m_HaarEyeZoom;
	int bestHaarIndex = 0, haarHypothesis = 0;
	float haarScore[100], bestHaarScore = 0;
	IppiPoint location[100], haarCenter;

	haarCenter.x = haarCenter.y = m_haarImageShifts*m_haarImageSampling;
#if 1
	// Create grid of points for classifier search around specularity detection:
	int haarStep = m_haarImageSampling, ySearch = m_haarImageShifts, xSearch = m_haarImageShifts;
	for(int y = -ySearch, i = 0; y <= ySearch; y++)
	{
		for(int x = -xSearch; x <= xSearch; x++, i++)
		{
			location[i].y = (y+ySearch)*haarStep;
			location[i].x = (x+xSearch)*haarStep;
			haarHypothesis ++;
		}
	}
#else
	location[0].x = location[0].y = 0;
	haarHypothesis = 1;
#endif
	CropAndWarp(*videoFrame->GetPyramid(), eye.m_nCenterPointX, eye.m_nCenterPointY, (1.0f / zoom), *m_pEyeImage ,scratch);

	isPointAHaarEye = m_pHaarEyeDetectionServer->Classify(*m_pEyeImage, location, haarHypothesis, haarScore, &haarCenter);

	eye.m_HaarCenter.Set( haarCenter.x + eye.m_nCenterPointX - m_haarImageSampling*m_haarImageShifts,
						  haarCenter.y + eye.m_nCenterPointY - m_haarImageSampling*m_haarImageShifts);

	eye.m_HasHaarCenter = isPointAHaarEye;

	return isPointAHaarEye;
}

#endif
#endif


bool EyeDetectAndMatchServer::Detect(CSampleFrame *videoFrame, int level)
{
	int numberOfSpecularityEyes = 0, numberOfHaarEyes = 0;
	bool detectionStatus = false;
	EyeCenterPointList *pEyeList = &(*videoFrame->GetEyeCenterPoints());

	/*
	 * Perform specularity detection
	 */
	printf("Anita...level...%d\n", level);
	Image8u *pImage = videoFrame->GetPyramid()->GetLevel(level);

	if(m_SingleSpecMode)
	{
		XTIME_OP("SingleSpec",
		detectionStatus = m_pEyeDetectionServer[level]->IsFrameAnEyeSingleSpec( pImage, pEyeList, true, false )
		);

	}
	else
	{
		XTIME_OP(" IsFrameAnEyeDoubleSpec",
		detectionStatus = m_pEyeDetectionServer[level]->IsFrameAnEye( pImage, pEyeList )
		);
	}
	printf("Anita....detectionStatus..%d\n", detectionStatus);

	if(detectionStatus)
	{
		numberOfSpecularityEyes = pEyeList->size();
		printf("Anita....numberOfSpecularityEyes..%d\n", numberOfSpecularityEyes);
	//	printf("***************Number Of Spec Eyes ************%d\n",numberOfSpecularityEyes);
		EyeCenterPointList::iterator iter = pEyeList->begin();
		while(iter != pEyeList->end())
		{
			printf("Anita...Inside iter\n");

			CEyeCenterPoint &eye = (*iter);
			bool isPointAnEye = true;

			// scale to level 0
			eye.m_nCenterPointX = expand(eye.m_nCenterPointX, level);
			eye.m_nCenterPointY = expand(eye.m_nCenterPointY, level);
			eye.m_nLeftSpecularityX = expand(eye.m_nLeftSpecularityX, level);
			eye.m_nLeftSpecularityY = expand(eye.m_nLeftSpecularityY, level);
			eye.m_nRightSpecularityX = expand(eye.m_nRightSpecularityX, level);
			eye.m_nRightSpecularityY = expand(eye.m_nRightSpecularityY, level);

			/*
			* Perform haar feature (viola/jones) based eye classifier
			*/
		//	printf("before HAAR -- [%d %d %d ]\n",eye.m_nCenterPointX,eye.m_nCenterPointY,numberOfHaarEyes);
#if DO_HAAR_EYE_CLASSIFIER
			if(m_DoHaar)
			{
				printf("Anita...Inside DoHaar\n");
				bool isPointAHaarEye;
				isPointAHaarEye= DetectHaarEye(videoFrame, eye);


				if(isPointAHaarEye)
				{
					numberOfHaarEyes ++; // count the number of eyes per frame that are haar eyes
				}
				eye.SetIsHaarEye( isPointAHaarEye );
				if(!m_DoIgnoreHaar)
				{
					isPointAnEye = isPointAHaarEye;
				}
			}
#endif
		//	printf("AFTER   HAAR [%d %d %d ]\n",eye.m_nCenterPointX,eye.m_nCenterPointY,numberOfHaarEyes);

			/*
			* Added extra test for H-Cam to prune spurious detections, especially if we only want
			* two detections for eye sepearation based distance estimation.
			*/
			if(isPointAnEye && m_DoCovarianceTestForDetection)
			{
				{
					// For now we always compute specularity based moments
					int myLevel = 0;
					if(videoFrame->GetBinVal() != 0){
						//printf("Using Level 2\n");
						myLevel=2;
					}

					XTIME_OP(" ComputeSpecularityMetrics",
					(*iter).m_LeftMoments =	EyeDetectorServer::ComputeSpecularityMetrics(
						videoFrame->GetPyramid()->GetLevel(myLevel),
						(*iter).m_nLeftSpecularityX >> myLevel,
						(*iter).m_nLeftSpecularityY >> myLevel,
						myLevel,
						(reduce(SPEC_SIZE, myLevel) / 2) + 1 );

					(*iter).m_RightMoments = EyeDetectorServer::ComputeSpecularityMetrics(
						videoFrame->GetPyramid()->GetLevel(myLevel),
						(*iter).m_nRightSpecularityX >> myLevel,
						(*iter).m_nRightSpecularityY >> myLevel,
						myLevel,
						(reduce(SPEC_SIZE, myLevel) / 2) + 1 )
					);
				}
				// assume lambda1 > lambda2
				bool isSpecularlitySharp = false;
				if((*iter).m_LeftMoments.lambda1 < m_IrisSpecularityCovarianceEigenvalueThreshold)
				{
					float leftEccentricity = (*iter).m_LeftMoments.lambda1 / (*iter).m_LeftMoments.lambda2;
					if(leftEccentricity <= m_IrisSpecularityEccentricityThreshold)
					{
						if((*iter).m_RightMoments.lambda1 < m_IrisSpecularityCovarianceEigenvalueThreshold)
						{
							float rightEccentricity = (*iter).m_RightMoments.lambda1 / (*iter).m_RightMoments.lambda2;
							if(rightEccentricity <= m_IrisSpecularityEccentricityThreshold)
							{
								isSpecularlitySharp = true;
							}
						}
					}
				}
				isPointAnEye = isSpecularlitySharp;
			}
			if(isPointAnEye)
			{
				++iter;
			}
			else
			{
				iter = pEyeList->erase( iter );
			}
		}
		/*
		* Estimate distance between pairs of eyes (added for h-cam distance constraint):
		*/

		if(m_DoEnforceDetectionRange)
		{
			EstimateDistance(videoFrame);
		}
	}

	if(!m_DoCovarianceTestForDetection)
	{
		assert(numberOfHaarEyes==pEyeList->size());
	}
	videoFrame->SetNumberOfHaarEyes(numberOfHaarEyes);
	videoFrame->SetNumberOfSpecularityEyes( numberOfSpecularityEyes );

	return (videoFrame->GetEyeCenterPointList()->size() > 0);
}


void EyeDetectAndMatchServer::EstimateDistance(CSampleFrame *videoFrame)
{
	videoFrame->SetObjectDistance(0);

	// Find distance between eyes
	if(m_pSensorModel && (videoFrame->GetEyeCenterPointList()->size() > 1))
	{
		float modelEyeSeparation = 0.060325; // meters
		float focalLengthInPixels = m_pSensorModel->GetFocalLength().X();
		float maxDistance = 0.0;

		int w2,h2;
		videoFrame->getDims(2,w2,h2);
		EyeCenterPointList &eyes = (*videoFrame->GetEyeCenterPointList());

		for(EyeCenterPointList::iterator iter = eyes.begin(); iter != eyes.end(); iter++)
		{
			HPoint2D p1((*iter).m_nCenterPointX, (*iter).m_nCenterPointY);

			EyeCenterPointList::iterator jter = iter;
			for(jter++; jter != eyes.end(); jter++)
			{
				HPoint2D p2((*jter).m_nCenterPointX, (*jter).m_nCenterPointY);

				{

					// Get the mid point between two eyes:
					HPoint2D eyeMidPoint = (p1 + p2) / 2;
					float measuredEyeSeparation = (p1 - p2).L2Norm();

					HPoint3D xyz;
					xyz.Z() = modelEyeSeparation * focalLengthInPixels / measuredEyeSeparation;
					xyz.X() = float(eyeMidPoint.X() - w2) * xyz.Z() / focalLengthInPixels;
					xyz.Y() = float(eyeMidPoint.Y() - h2) * xyz.Z() / focalLengthInPixels;

					if(xyz.Z() > maxDistance)
					{
						maxDistance = xyz.Z();
					}

					//DebugOutput("Separation: %f => (%f, %f, %f)\n", measuredEyeSeparation, xyz.X(), xyz.Y(), xyz.Z());
				}
			}
		}
		videoFrame->SetObjectDistance(maxDistance);
	}
}

EyeDetectAndMatchServer::EyeDetectAndMatchServer(int imageWidth, int imageHeight,int detlevel,std::string logfile) : scratch(0),m_DoSaveNeighbors(0),m_LogFile(logfile)
{
	m_HasFirstTimestamp = false;
	m_FirstTimestamp = 0;
	m_DoDynamicZoom = false;
	m_pSensorModel = 0;
	m_DetectionLevel = detlevel;//EYE_DETECTION_LEVEL;
	m_haarImageShifts = 2;
	m_haarImageSampling = 2;
	m_HaarSearchImage = 0;
	m_HaarEyeZoom = HAAR_EYE_ZOOM;
	m_DoIgnoreHaar = DO_IGNORE_HAAR_EYE_CLASSIFIER;

	m_MatcherNeedsSpecularityEye = MATCHER_NEEDS_SPECULARITY_EYE;
	m_MatcherNeedsHaarEye = MATCHER_NEEDS_HAAR_EYE;

	m_SingleSpecMode = false;
	m_DoLevel0SpecularitySaturationTest = false;
	m_DoCovarianceTestForDetection = false;
	m_DoEnforceDetectionRange = false;

	m_index = 0;

	m_pBigEyeImage = 0;

#if DO_HAAR_EYE_CLASSIFIER
	m_pEyeImage	= 0;
	m_DoHaar = DO_HAAR_EYE_CLASSIFIER;
	m_pHaarEyeDetectionServer = 0;
#endif

	AllocImageThings(imageWidth, imageHeight);

	m_IrisSpecularityCovarianceEigenvalueThreshold = IRIS_SPECULARITY_COVARIANCE_EIGENVALUE_THRESHOLD;
	m_IrisSpecularityEccentricityThreshold = IRIS_SPECULARITY_ECCENTRICITY_THRESHOLD;
}


void EyeDetectAndMatchServer::AllocImageThings(int imageWidth, int imageHeight)
{
	for(int level = 0; level < NUMBER_OF_LEVELS; level++)
	{
		m_ImageWidth[level] = reduce(imageWidth, level);
		m_ImageHeight[level] = reduce(imageHeight, level);
	}


	for(int level = 0; level < NUMBER_OF_LEVELS; level++)
		m_pEyeDetectionServer[level] = 0;

	int level = m_DetectionLevel;
	{
		printf("detection Img W H %d %d \n",m_ImageWidth[level], m_ImageHeight[level]);

		m_pEyeDetectionServer[level] = new EyeDetectorServer(m_ImageWidth[level], m_ImageHeight[level]);
		{
			m_pEyeDetectionServer[level]->SetLogFile(m_LogFile);
			m_pEyeDetectionServer[level]->SetBoxX( reduce(BOX_X, level) );
			m_pEyeDetectionServer[level]->SetBoxY( reduce(BOX_Y, level) );
			m_pEyeDetectionServer[level]->SetSearchX( reduce(SEARCH_X, level) );
			m_pEyeDetectionServer[level]->SetSearchY( reduce(SEARCH_Y, level) );
			m_pEyeDetectionServer[level]->SetSeparation( reduce(SEPARATION, level));
			m_pEyeDetectionServer[level]->SetSpecularitySize( reduce(SPEC_SIZE, level) );
			m_pEyeDetectionServer[level]->SetStepSize( reduce(STEP, level) );
			m_pEyeDetectionServer[level]->SetMaskRadius( reduce(MASK_RADIUS, level) );
		}
		m_pEyeDetectionServer[level]->SetSpecularityMagnitude( SPEC_MAG );
		m_pEyeDetectionServer[level]->SetVarianceThresholdMax( VARIANCE_THRESHOLD_MAX );
		m_pEyeDetectionServer[level]->SetVarianceThresholdMin( VARIANCE_THRESHOLD_MIN );
	}

	scratch_size=0;
	scratch=0;
	scratch_size=MAX(10*(m_ImageWidth[1]),10000);
#ifdef __BFIN__
	// this would allocate 10K of scratch
	scratch=(char *)sram_alloc(scratch_size, L1_DATA_SRAM);
#else
	scratch=(char *)malloc(scratch_size);
#endif
	assert(scratch);

}
