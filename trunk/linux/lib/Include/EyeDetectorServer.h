#pragma once

#include "Image.h"
#include "EyeCenterPoint.h"
#include <string>
#include <stdio.h>

#define NUMBER_OF_ORIENTATIONS 8

//used for finding an eye

#if 1
#define DETECTION_LEVEL 2
#define SPEC_MAG 15
#define SPEC_SIZE 15
#define STEP 2
#define BOX_X 15
#define BOX_Y 15
#define SEARCH_Y 10
#define SEARCH_X 15
#define SEPARATION 36
#define MASK_RADIUS 10
#else
#define DETECTION_LEVEL 0
#define SPEC_MAG 15
#define SPEC_SIZE 5
#define STEP 1
#define BOX_X 5
#define BOX_Y 5
#define SEARCH_Y 10
#define SEARCH_X 15
#define SEPARATION 36
#define MASK_RADIUS 10
#endif
#define MAX_SPEC_POINTS 1000
#define VARIANCE_THRESHOLD_MIN 1.5
#define VARIANCE_THRESHOLD_MAX 0.666

class HBOX_API EyeDetectorServer
{
public:
	EyeDetectorServer(void);
	EyeDetectorServer(int width, int height);
	~EyeDetectorServer(void);

	void Resize(int width, int height);
	void Alloc(int width, int height);
	void Free();

	void SetID(int id) { m_ID = id; }
	void SetIndex(int index) { m_Index = index; }

	bool AddDetection(int x1, int y1, int x2, int y2, float confidence, EyeCenterPointList *list);
	bool AddDetection_Simple(int x1, int y1, EyeCenterPointList *list);
	bool IsPointAnEye(Image8u* frame, const CEyeCenterPoint &point, bool singleSpec);

	bool IsFrameAnEye(Image8u* frame, EyeCenterPointList *list);
	bool IsFrameAnEyeSingleSpec(Image8u* frame, EyeCenterPointList *list, bool resetMask = true, bool doAvgThreshold = false, int halfWidth = 3, int threshold = 150);

	static HU_MOMENTS ComputeSpecularityMetrics(Image8u* frame, int x, int y, int level, int radius);
//	static HU_MOMENTS ComputeDualSpecularityMetrics(Image8u* frame, int x, int y, int level);

	void SetSpecularityMagnitude(int specularityMagnitude) { m_SpecularityMagnitude = specularityMagnitude; }
	int GetSpecularityMagnitude() const { return m_SpecularityMagnitude; }

	void SetSpecularitySize(int specularitySize) { 	printf("Spec sz = %d \n",specularitySize);
													m_SpecularitySize = specularitySize;
												}
	int GetSpecularitySize() const { return m_SpecularitySize; }

	void SetSeparation(int separation) { m_Separation = separation; }
	int GetSeparation() const { return m_Separation; }

	void SetSearchX(int searchX) { m_SearchX = searchX; }
	int GetSearchX() const { return m_SearchX; }

	void SetSearchY(int searchY) { m_SearchY = searchY; }
	int GetSearchY() const { return m_SearchY; }

	void SetBoxX(int boxX) { m_BoxX = boxX; }
	int GetBoxX() const { return m_BoxX; }

	void SetBoxY(int boxY) { m_BoxY = boxY; }
	int GetBoxY() const { return m_BoxY; }

	void SetStepSize(int stepSize) { m_StepSize = stepSize; }
	int GetStepSize() const { return m_StepSize; }

	void SetMaskRadius(int MaskRadius) { m_MaskRadius = MaskRadius; }
	int GetMaskRadius() const { return m_MaskRadius; }

	void SetVarianceThresholdMin(float varianceThresholdMin) { m_VarianceThresholdMin = varianceThresholdMin; }
	float GetVarianceThresholdMin() const { return m_VarianceThresholdMin; }

	void SetVarianceThresholdMax(float varianceThresholdMax) { m_VarianceThresholdMax = varianceThresholdMax; }
	float GetVarianceThresholdMax() const { return m_VarianceThresholdMax; }

	Image8u * GetBlurredImage() { return m_pBlur; }
	void SetLogFile(std::string a){m_LogFile = a;}

	int m_ID;
	int m_Index;
	int m_SpecularityMagnitude;
	int m_SpecularitySize;
	int m_Separation;
	int m_SearchX;
	int m_SearchY;
	int m_BoxX;
	int m_BoxY;
	int m_StepSize;
	int m_MaskRadius;

	Image8u *m_pEyeCenterMask;
	Image8u *m_pBlur;

	int m_ImageWidth;
	int m_ImageHeight;

	float m_VarianceThresholdMin;
	float m_VarianceThresholdMax;
	bool m_shouldLog;
	std::string m_LogFile;
private:
	void SingleSpectDetect(IplImage* image, EyeCenterPointList *list, bool doAvgThreshold=false, int threshold=150, bool SimpleAdd=false);
	int m_specPoints[MAX_SPEC_POINTS];	// memory for storing points
};
