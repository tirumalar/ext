/*
 * EyeSegmentServer.h
 *
 *  Created on: 28 Apr, 2009
 *      Author: madhav
 */

#ifndef EYESEGMENTSERVER_H_
#define EYESEGMENTSERVER_H_
#include <list>

#include <cv.h>
extern "C" {
#include "test_fw.h"
#include "EdgeImage_private.h"
}
typedef struct
{
 CvPoint3D32f cp;
 float ic; // integral cost
 float hc; // histogram cost
 float sm; // similarity measure
 float eph; // estimated pupil height
 float epw; // estimated pupil width
} CircleParameters;

class EyeSegmentServer {
public:
	EyeSegmentServer();
	virtual ~EyeSegmentServer();

	int buildTrignoTabs(double min_angle,double max_angle,double angleSample);
	void computeIntegralContributionFixed(IplImage * gradImage, IplImage *image, IplImage * out,
			int trindx, int rad, int deltaRad,int sxbsx, int sybsy,
			const CvSize & searchArea, const unsigned int & iscale);


	std::list<CircleParameters> findcircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc,
			CvRect loc, float lradius, float uradius, float scaling, float sigma,
			float vert, float horz, float lambda, float min_angle, float max_angle,
			bool pupil, int level);


	IplImage *findEyelidCircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc,
			CvRect loc, float lradius, float uradius, float scaling, float sigma,
			float vert, float horz, float lambda, float min_angle, float max_angle,
			bool pupil, int level);

	void GeneratePolarLookupTable(int w,unsigned int *sincostable);
	void FlattenIris_Fix(IplImage *inImage, IplImage *outImage,CvPoint3D32f irispt,CvPoint3D32f pupilpt,unsigned int *lut);
	void PolarWarping_Fix_NN(IplImage *in, IplImage *out,CvPoint3D32f irispt,CvPoint3D32f pupilpt,unsigned int *LUT);
	void MaskSpecularitiesFlat_Fix(IplImage *inImage,IplImage *in1Image, IplImage *outImage,CvPoint3D32f irispt,CvPoint3D32f pupilpt,unsigned int *lut);
	void MaskSpecularities_Fix(IplImage *image, CvRect rect, IplImage *mask,IplImage **eigenvv, int level, bool pupil,void * ptr);
	void ComputeEyelidIntegralContribution(IplImage *gradImage, IplImage *out, IplImage *count, CvPoint2D32f ptf1, CvPoint2D32f ptf2, CvSize searchArea);
	void special_divide(IplImage *cost, IplImage *count, IplImage *costFloat, float *div_table);
	float RegionHistogramDistance(IplImage *img, IplImage *specularityMask, CvPoint2D32f pt, float radIn, float rad, float radOut);
	void get_ranges(CvPoint2D32f *pt, CvSize *searchArea, int w, int h, int minTop, int maxTop);
	int getLevels();
	int getCoarseSearchSampling();
	int getFineSearchSampling();
	int getNumMatrices();
	IplImage *getCostMatrix();
	IplImage *getCountMatrix();
	IplImage **getCostMatrixFloat();
	IplImage **getCostMatrixFloatMax();
	IplImage *getMask(int level);
	IplImage *getImgPyr(int level);
	int *getFullCostData();
	int *getFullCostDataMax();
	int *getPScore();
	Point2D16i *getPPeak2d();
	CvPoint3D32f *getPPeak();
	unsigned int *getTemp();
	int *getCosval();
	int *getSinval();
	int getEyelidSearchSampling();
	CvHistogram *getIrisHistogram();
	std::pair<float,float> *getEstimatedPupil();
	void ComputeAnnularSectionHistograms(IplImage *img, IplImage *mask, CvPoint3D32f iris, int level);
	void EstimatePupilCenter(IplImage *img, CvPoint3D32f iris, CvHistogram *histogram);

	int getfLevels();
	float getMinLidAngle();
	float getMaxLidAngle();
	IplImage *getUpperEyelidMask();
	IplImage *getLowerEyelidMask();
	unsigned short *getDivLUT();

#ifdef EDGEIMAGE_TEST
	void Reset(){
		m_peakCountTotal =-1;
		m_numCircle =-1;
	}
	int GetPeakCnt(){ return m_peakCountTotal;}
	int GetCircleCnt(){ return m_numCircle;	}
#endif
protected:
    int m_levels;
    int m_coarseSearchSampling;
    int m_fineSearchSampling;
    int numMatrices;
    IplImage *m_costMatrix;
    IplImage *m_countMatrix;
    IplImage **m_costMatrixFloat;
    IplImage **m_costMatrixFloatMax;
    int *m_fullCostData;
    int *m_fullCostDataMax;
    int *m_pScore;
    Point2D16i *m_pPeak2d;
    CvPoint3D32f *m_pPeak;
    unsigned int m_temp[320 * 5];
    int cosval[500];
    int sinval[500];
    int m_eyelidSearchSampling;
#ifdef EDGEIMAGE_TEST
    unsigned short div_table[256];
    int m_peakCountTotal,m_numCircle;
#endif
};

#endif /* EYESEGMENTSERVER_H_ */
