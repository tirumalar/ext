#pragma once

#ifdef _WIN32
#include "EyeSortingLib_def.h"
typedef unsigned long uint64_t;
#else
#define EYESORTINGLIB_API
#endif

#include <opencv/cxcore.h>
//#include "ippdefs.h"
#include <list>
#include <map>

typedef enum
{
	VERTICAL,
	HORIZONTAL
}StripDirection;

typedef struct
{
	int x;
	int y;
	int r;
} IrisParameters;

class EYESORTINGLIB_API ImageQualityChecker
{
public:
	typedef std::multimap<float, int > MapType;
	ImageQualityChecker(int width=0, int height=0, int maxGrayScaleSpec=230, float threshScore=9.5,float threshRatio=1.3);
	
	virtual ~ImageQualityChecker(void);
	int checkQualityNano2(unsigned char *imgBuff,int width,int height,int widthStep,IrisParameters irisParameters);
	void SetSpecularityValue(unsigned char val) {m_expectedMaxValue = val; m_MaxGrayScaleSpec = val; }
		
private:
	int check(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas = 0.0f);
	void reset();
	std::pair<float, float> check_image(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas=0.0f);
	std::pair<float, float> check_image2(char *img, int w, int h, int widthStep, CvPoint3D32f iris=cvPoint3D32f(0,0,0), CvPoint3D32f pp=cvPoint3D32f(0,0,0), float fmeas=0.0f);
	int ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius=14);
	std::list<CvPoint3D64f> ComputeMultiSpecularityMetrics(IplImage* frame, int radius=14, unsigned char maxVal=255);
	float strip_focus(IplImage *img, CvPoint pt, CvSize sz, float *hist);
//	float strip_focus2(IplImage *img, CvPoint pt, CvPoint2D32f var, CvSize sz, float *hist);
	float strip_focus3(IplImage *img, CvPoint pt, CvPoint2D32f var, CvSize sz, float *hist);

	
	MapType peak_detector(float *hist, float *temp, int len, int m=1, int l=2);
	CvPoint2D32f GetSpecularityCentroid() {return m_specularityCentroid;}
	std::pair<int, int> GetIrisPupilIntensities() {return m_irisPupilIntensities; }

	//Quality Metrics
	int CheckVerticalBlurring(IplImage* frame, CvPoint center, int radius, CvPoint3D64f *of, unsigned char specVal=230);
	int CheckHorizontalBlurring(IplImage* frame, CvPoint center, int radius, CvPoint3D64f *of, unsigned char specVal=230);
	int ConsolidateResultsVerticalBlurring(IplImage* frame);
	int ConsolidateResultsVerticalBlurringGeneralized(IplImage* frame);
	int ConsolidateResultsVerticalBlurringGeneralized2(IplImage* frame,StripDirection direction,int radius,IrisParameters irisParameters);
	StripDirection getDirection() { return m_direction; }
	void setDirection(StripDirection dir) { m_direction = dir; }
	int findEffectiveImageParameters(IplImage* imgOrig, StripDirection direction);
	int findEffectiveImageParameters2(IplImage* imgOrig, StripDirection direction, IrisParameters irisParameters);
private:
	int m_width, m_height;
	unsigned char m_expectedMaxValue;
	int m_extraStep;
	float *m_hist;
	int m_histSizeBytes;
	std::pair<float, float> low_threshold, high_threshold;
	std::pair<int, int> m_irisPupilIntensities;
	CvPoint2D32f m_specularityCentroid;
	IplImage *m_smallImage128x128;
	IplImage *m_smallImageExpand128x128;
	IplImage *m_smallImage256x256;
	unsigned char *m_scratchBuffer;

	//Quality Metrics
	typedef int (ImageQualityChecker::*blurringFunc)(IplImage *, CvPoint, int , CvPoint3D64f *, unsigned char); 
	std::list<std::pair<float, float> > score_list;
	StripDirection m_direction;
	blurringFunc ptrblurringFunc;
	int m_effectiveRegionLength;
	CvPoint m_effectiveRegionCenter;

	int m_MaxGrayScaleSpec;
	float m_threshScore;
	float m_threshRatio;

};

