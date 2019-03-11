#pragma once

#include <opencv/cxcore.h>

class NanoFocusSpecularityMeasure
{
public:
	NanoFocusSpecularityMeasure();
	virtual ~NanoFocusSpecularityMeasure(void);
	float ComputeHaloScore(IplImage *img);
	float ComputeHaloScoreTopPointsNano(IplImage *img1, int specValue=255, int noOfPixelsToConsider=6, float topPixelsPercentage=25.0f, int intensityThreshBP=90, int HaloThresh=180); 
	float ComputeHaloScoreTopPointsPico(IplImage *img1, int specValue=255, int noOfPixelsToConsider=15, float topPixelsPercentage=25.0f, int intensityThreshBP=115, int HaloThresh=200); 
	void SetSpecularityValue(unsigned char val) {m_expectedMaxValue = val; }
	CvPoint2D32f GetSpecularityCentroid() {return m_specularityCentroid;}
	int GetHaloPixelCount() {return m_HaloPixelCount;}
	CvRect GetSpecularityROI() {return m_specularityROI;}
	float GetModifiedBottomPointsDiff() {return m_modifiedBottomPointsDiff;}
	float GetAvgIntensityTP() {return m_avgIntensityTP;}
	int GetNoOfBottomPoints() {return m_noOfBottomPoints;}
	int GetNoOfTopPoints() {return m_noOfTopPoints;}
	float GetOldHaloScore() {return m_oldHaloScore;}
	
	
protected:
	int check_image(IplImage *img);
	int ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius);
	unsigned char m_expectedMaxValue;
	
private:
	CvPoint2D32f m_specularityCentroid;
	CvRect m_specularityROI;
	int m_HaloPixelCount;
	float m_modifiedBottomPointsDiff;
	int m_noOfBottomPoints;
	int m_noOfTopPoints;
	float m_avgIntensityTP;
	float m_oldHaloScore;
};

