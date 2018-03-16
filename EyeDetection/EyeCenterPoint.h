#pragma once

#include "CvInterfaceDefs.h"
#include "HPoint.h"

#include <vector>

#define MAXIMUM_HAMMING_DISTANCE 1.0

class CEyeCenterPoint
{
public:
	int m_nLeftSpecularityX;
	int m_nLeftSpecularityY;

	int m_nRightSpecularityX;
	int m_nRightSpecularityY;

	int m_nLeftSpecularityXL2;
	int m_nLeftSpecularityYL2;

	int m_nRightSpecularityXL2;
	int m_nRightSpecularityYL2;

	bool m_HasHaarCenter;
	HPoint2Di m_HaarCenter;

	int m_nCenterPointX;
	int m_nCenterPointY;

	int m_index;

	bool m_IsSpecularityEye;
	bool m_IsHaarEye;

	float m_fConfidence;
	float m_fHammingDistance;
	//used for matching the eye image to the orignal 2k x 2k image when writing to disk

	HU_MOMENTS m_LeftMoments;
	HU_MOMENTS m_RightMoments;
	HU_MOMENTS m_JointMoments;


	CEyeCenterPoint()
	{

		m_HasHaarCenter = false;
		m_IsSpecularityEye = false;
		m_IsHaarEye = false;

		m_nLeftSpecularityX = 0;
		m_nLeftSpecularityY = 0;

		m_nRightSpecularityX = 0;
		m_nRightSpecularityY = 0;

		m_nCenterPointX = 0;
		m_nCenterPointY = 0;

		m_fConfidence = 0;
		m_fHammingDistance = MAXIMUM_HAMMING_DISTANCE + 0.1;
	}

	CEyeCenterPoint(int nCenterX, int nCenterY, float confidence = 0.0, float score = MAXIMUM_HAMMING_DISTANCE)
	{

		m_HasHaarCenter = false;
		m_IsSpecularityEye = true;
		m_IsHaarEye = false;

		m_nLeftSpecularityX = nCenterX;
		m_nLeftSpecularityY = nCenterY;

		m_nRightSpecularityX = nCenterX;
		m_nRightSpecularityY = nCenterY;

		m_nCenterPointX = nCenterX;
		m_nCenterPointY = nCenterY;

		m_fConfidence = confidence;
		m_fHammingDistance = score;
	}

	void SetIsSpecularityEye(bool isSpecularityEye) { m_IsSpecularityEye = isSpecularityEye; }
	bool GetIsSpecularityEye() const { return m_IsSpecularityEye; }

	void SetIsHaarEye(bool isHaarEye) { m_IsHaarEye = isHaarEye; }
	bool GetIsHaarEye() const { return m_IsHaarEye; }

	float GetConfidence() const { return m_fConfidence; }
	void SetConfidence(float confidence) { m_fConfidence = confidence; }

	float GetHammingDistance() const { return m_fHammingDistance; }
	void SetHammingDistance(float hammingDistance) { m_fHammingDistance = hammingDistance; }

};

typedef std::vector<CEyeCenterPoint> EyeCenterPointList;
typedef EyeCenterPointList::iterator EyeCenterPointListIterator;
