#pragma once
#if 0
#include <cxtypes.h>
#else
#include <opencv2/core.hpp>
#endif
//#include "ippdefs.h"
#include <list>
#include <map>
#include <vector>
typedef float Ipp32f;
typedef unsigned char Ipp8u;
class NanoFocusSpecularityBasedSpoofDetector
{
public:
	typedef std::multimap<float, int > MapType;

	NanoFocusSpecularityBasedSpoofDetector(int width=0, int height=0);

	virtual ~NanoFocusSpecularityBasedSpoofDetector(void);
	int check(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas = 0.0f);
	void reset();
	std::pair<float, float> check_image(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas=0.0f);
	std::pair<float, float> check_image2(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas=0.0f);
	int ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius=14);
	float strip_focus(IplImage *img, CvPoint pt, CvSize sz, float *hist);
	float strip_focus2(IplImage *img, CvPoint pt, CvPoint2D32f var, CvSize sz, float *hist);
	float strip_focus3(IplImage *img, CvPoint pt, CvPoint2D32f var, CvSize sz, float *hist);
	CvPoint3D32f ComputeHaloScore(IplImage *img1,unsigned char maxValue=230);
	CvPoint3D32f ComputeTopPointsBasedHaloScore(IplImage *img1,unsigned char maxValue=230,int pixelsToConsider=6,
			float TopPixelsPercentage=25.0f,int BottomPixelsIntensityThresh=91, float HaloThreshold=180.0f,
			bool EnableHaloThresh=false, int MHaloNegationThresh=350);
	CvRect GetSpecularityROI() {return m_specularityROI;}

	void SetSpecularityValue(unsigned char val) {m_expectedMaxValue = val; }
	MapType peak_detector(float *hist, float *temp, int len, int m=1, int l=2);
	CvPoint2D32f GetSpecularityCentroid() {return m_specularityCentroid;}
	std::pair<int, int> GetIrisPupilIntensities() {return m_irisPupilIntensities; }
	//std::vector<int> HaloPoints;
	void SetBottomPointsDiff(float score) {m_BottomPointsDiff = score;}
	void SetNoOfBottomPoints(int number) {m_noOfBottomPoints = number;}
	void SetNoOfTopPoints(int number) {m_noOfTopPoints = number;}
	void SetAvgIntensityTP(float score) {m_avgIntensityTP = score;}

private:
	int m_width, m_height;
	unsigned char m_expectedMaxValue;
	int m_extraStep;
	Ipp32f *m_hist;
	std::pair<float, float> low_threshold, high_threshold;
	std::pair<int, int> m_irisPupilIntensities;
	CvPoint2D32f m_specularityCentroid;
	//Specularity ROI (Bounding Box)
	CvRect m_specularityROI;

	std::list<std::pair<float, float> > score_list;
	//float m_topPointsBasedHaloScore;
	float m_BottomPointsDiff;
	int m_noOfBottomPoints;
	int m_noOfTopPoints;
	float m_avgIntensityTP;
};

