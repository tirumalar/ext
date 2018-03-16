#pragma once

#include <cv.h>
#include <map>
#include <list>
#include "area_concom.h"
#ifdef __BFIN__
#define __EIGENREUSABLE__
#endif

	#include "EdgeImage_private.h"
	extern unsigned short SQRT_LUT_0_64[65*65];
	extern unsigned short SQRT_LUT_0_256[256*256];
	extern unsigned short SQRT_LUT_0_256_eyelid[256*256];


typedef struct
{
	CvPoint3D32f cp;
	float ic;	// integral cost
	float hc;	// histogram cost
	float sm;	// similarity measure
	float eph;	// estimated pupil height
	float epw;	// estimated pupil width
}	CircleParameters;

typedef struct
{
	CvPoint3D32f on;
	CvPoint3D32f in;
	CvPoint3D32f out;
}
CirclePoints;

class EyeSegmentationOutput
{
	public:
	CvPoint3D32f ip;
	CvPoint3D32f pp;
	IplImage *flatIris;
//	CvPoint3D32f uep[20];
	IplImage *UpperEyeLid;
//	CvPoint3D32f lep[20];
	IplImage *LowerEyeLid;
	IplImage *specMask;
	IplImage *flatMask;
};

inline float bilinear( float *gradptr, int gradStep, float py, float px );

class EyeSegmentServer
{
public:
	enum {LEFT_SIDE_EYELID, RIGHT_SIDE_EYELID, BOTH_SIDE_EYELID};
	CvPoint2D32f m_SegmentationCheck;
	bool m_nbhdCheckIris;
	bool m_nbhdCheckPupil;
	EyeSegmentationOutput m_eso;

	EyeSegmentServer(int w=640, int h=480);
	void compute_LUT_sqrtminmax(int gradientpupilmin,int gradientpupilmax,int gradientcirclemin,int gradientcirclemax);
	void CheckSimilarityMeasure(char* filename); // delete not required in final version
	virtual ~EyeSegmentServer(void);
	int checkForObstruction(bool pupil, float maxCost, float maxHistCost);
	EyeSegmentationOutput Process(IplImage* image,int index = 0);
	bool Getiseye( );
	void SetEyeLocationSearchArea(CvRect rect)
	{
		m_centerRect = rect;
	}
	void GetEyeLocationSearchArea(int& xo, int& yo, int& w, int& h)
	{
		xo = m_centerRect.x;
		yo = m_centerRect.y;
		w = m_centerRect.width;
		h = m_centerRect.height;
	}
	void SetIrisRadiusSearchRange(int min, int max)
	{
		m_startIrisRadius = (float) min;
		m_endIrisRadius = (float) max;
		m_irisRect = cvRect(cvRound(m_w/2 - m_endIrisRadius), cvRound(m_h/2 - m_endIrisRadius), cvRound(2*m_endIrisRadius), cvRound(2*m_endIrisRadius)); //radiis are not divided by two
	}
	void SetPupilRadiusSearchRange(int min, int max)
	{
		m_startPupilRadius = (float) min;
		m_endPupilRadius = (float) max;
	}
	void SetPupilAngleSearchRange(int min, int max)
	{
		m_minPupilAngle =  min*CV_PI/180;
		m_maxPupilAngle =  max*CV_PI/180;
	}
	void GetPupilAngleSearchRange(float& min, float& max)
	{
		min = m_minPupilAngle;
		max = m_maxPupilAngle;
	}
	void GetUpperEyelidCenterandRadius(CvPoint &cenPt, float &rad)
	{
		cenPt = m_centerptUpperEyelid;
		rad = m_radiusUpperEyelid;
	}
	void SetUpperEyelidCenterandRadius(CvPoint cenPt,float rad)
	{
		m_centerptUpperEyelid = cenPt;
		m_radiusUpperEyelid = rad;
	}
	void GetLowerEyelidCenterandRadius(CvPoint &cenPt, float &rad)
	{
		cenPt = m_centerptLowerEyelid;
		rad = m_radiusLowerEyelid;
	}
	void SetLowerEyelidCenterandRadius(CvPoint cenPt,float rad)
	{
		m_centerptLowerEyelid = cenPt;
		m_radiusLowerEyelid = rad;
	}

	void EnableEyelidSegmentation(bool enable = true)
	{
		m_enableEyelidSegmentation = enable;
	}
	void SetCoarseSearchSampling(double sampling)
	{
		m_coarseSearchSampling = sampling;
	}
	void SetFineSearchSampling(double sampling)
	{
		m_fineSearchSampling = sampling;
	}
	void EnableEyeQualityAssessment(bool enable = true)
	{
		m_enableEyeQualityAssessment = enable;
	}
	bool GetEnableEyeQualityAssessment()
	{
		return m_enableEyeQualityAssessment;
	}
	void EnableImprovedCostMetric(bool enable = true)
	{
		m_enableHistogramCost = enable;
	}
	void SetEyelidSearchSampling(double sampling)
	{
		m_eyelidSearchSampling = sampling;
	}

	IplImage** GetImagePyramid(){ return m_imgPyr;}

	void SetDefaultEyelidAngles(float upperEyelidAngle, float lowerEyelidAngle)
	{
		m_defaultUpperEyelidAngle = (float) (CV_PI*upperEyelidAngle/180);
		m_defaultLowerEyelidAngle = (float) (CV_PI*lowerEyelidAngle/180);
	}
	bool IsEyelidPresent() { return m_eyelidDetected; }
	float RegionHistogramDistance(IplImage *img, IplImage *specularityMask, CvPoint2D32f pt, float radIn, float rad, float radOut);
	void ComputeAnnularSectionHistograms(IplImage *img, IplImage *mask, CvPoint3D32f iris, int level);
	void EstimatePupilCenter(IplImage *img, CvPoint3D32f iris, CvHistogram *histogram);
	void SetStartPyramidLevel(int level) {m_availablePyramidLevel = level; }
protected:
	void find_common_points(IplImage *ref, IplImage *ins, CvPoint3D32f *peaks, float *score, int *peakCount, CvPoint3D32f off, float threshold=0);
	bool checkNbhdForMaxima(IplImage* image, IplImage* mask,CvPoint3D32f CirclePoint, bool pupil, int level);
	void MaskSpecularities(IplImage *image, CvRect rect, IplImage *mask, int level, bool pupil=false);
	void FindSpecMask(IplImage* img, IplImage* Mask,int tag);
	CvPoint2D32f checkForGoodSegmentation(CvPoint3D32f Iris, CvPoint3D32f Pupil);
	void writeMaximaFile(bool pupil, int level,CvPoint3D32f pt, float integralCost, float histCost, float similarityMeasure);
	void putMaskonRing(IplImage* mask,CvPoint2D32f Pt,float radOut, int tag);
	void generateSinCosTable(float angleSampleTable, float min_angle,float max_angle, float rad, float deltaRad, CirclePoints* sinCosTable);

	std::list<CircleParameters> FindIrisMaximas(IplImage *image,int tag);
	std::list<CircleParameters> FindPupilMaximas(IplImage *image,int tag);
	void FindIrisAndPupil(std::list<CircleParameters> irisCircleList, std::list<CircleParameters> &pupilCircleList); // ,IplImage *);
	IplImage * FindUpperEyelidBoundaries(IplImage *image, IplImage *mask);
	IplImage * FindLowerEyelidBoundaries(IplImage *image, IplImage *mask);
	int findEyeLashes(IplImage* Iris,IplImage* mask);
	float ComputeLeftRightSimilarity(IplImage *img, IplImage *mask, CvPoint3D32f iris, int level, bool pupil);
	std::multimap<float, CvPoint3D32f> findLocalMaxima(float *fcm, float sx, float ex, float sy, float ey, float lradsc, float uradsc, float bias, bool tag);
	float GetIntegralEyeLid(IplImage *costField, CvPoint2D32f point, float rad, float min_angle, float max_angle, float max_grad, CirclePoints* sinCosTable, float angleSample, int numSamples, float scaling, int level);
	bool ComputeEyelidIntegralContribution(IplImage *gradImage, IplImage *out, IplImage *count, CvPoint2D32f ptf1, CvPoint2D32f ptf2, CvSize searchArea);
	std::list<CircleParameters> findcircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc,
			CvRect loc, float lradius, float uradius, float scaling, float sigma,
				float vert, float horz, float lambda, float min_angle, float max_angle,
				bool pupil, int level);

	IplImage *findEyelidCircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc,
			CvRect loc, float lradius, float uradius, float scaling, float sigma,
			float vert, float horz, float lambda, float min_angle, float max_angle,
			bool pupil, int level);
	float GetIntegral(IplImage *costField, IplImage *image, CvPoint2D32f point, float rad, float min_angle, float max_angle, float max_grad, CirclePoints* sinCosTable, float angleSample, int numSamples, bool pupil=false);
	void FindBoundary(CvPoint3D32f Point, int level, bool pupil);
	void ReturnBadSegmentation( );
	void RefineCircle1( IplImage *image, int level, bool pupil,	CvPoint3D32f pt, CvPoint3D32f & ptnew );
	void RefineCircle( IplImage *gradImage, IplImage *image, IplImage *mask, float sx, float ex, float sy, float ey,
									float lradsc, float uradsc, float min_angle, float max_angle, float lambda, int level,
									bool pupil, CvPoint3D32f pt, CvPoint2D32f & pt2d, float & bestRad );

	void ComputeGradientsForQualityMetric(IplImage *image, IplImage *mask, IplImage *gradH, IplImage *gradV, CvPoint3D32f circ, int level);
	CvPoint2D32f CheckSegmentationQuality(IplImage *gradH, IplImage *gradV, CvPoint3D32f circ, double min_angle, double max_angle, double angle_threshold, int level, bool pupil);
	IplImage* GenerateMask();



	bool ComputeIntegralContribution(IplImage *gradImage, IplImage *img, IplImage *out,
			CvPoint2D32f ptf, CvPoint2D32f ptin, CvPoint2D32f ptout, CvSize searchArea);
	void miplRemap(IplImage *mask, IplImage *flatMask, IplImage *dxTable, IplImage *dyTable);
	void GeneratePolarWarpTable(CvPoint3D32f iris, CvPoint3D32f pupil);
	IplImage *FlattenIris(IplImage *img, CvPoint3D32f iris, CvPoint3D32f pupil);
// 	int createDefaultEyelid(IplImage *mask, float angle, CvPoint3D32f iris, CvPoint3D32f pupil);
	void MaskSpecularitiesFlat( CvPoint3D32f iris, CvPoint3D32f pupil);






	std::list<CircleParameters> m_irisCircleList, m_pupilCircleList;

	CvPoint m_centerptUpperEyelid;
	float m_radiusUpperEyelid;

	CvPoint m_centerptLowerEyelid;
	float m_radiusLowerEyelid;

	bool m_iseye;
	std::pair<float,float> m_EstimatedPupil;
	IplImage *m_reusableImage;
	IplImage *m_gradPyr[5];
	IplImage *m_smoothImage[5];
	IplImage *m_imgPyr[5];
	IplImage *m_eigenvv[5];
#ifdef __EIGENREUSABLE__
	IplImage *m_eigenvvReusableImage;
#endif
//Madhav	IplImage *m_eigenvf[5];
	IplImage *m_maskPyr[5];
	IplImage *m_imgFlatPyr[5];
	IplImage *m_maskFlatPyr[5];
//	IplImage *m_smoothImageFlat[5];
	float m_startIrisRadius, m_endIrisRadius;
	float m_startPupilRadius, m_endPupilRadius;
	CvRect m_centerRect;
	CvRect m_irisRect;
	CvRect m_LidRightRect;
	CvRect m_LidLeftRect;
	int m_w, m_h;
	int m_levels;
	int f_levels;
	int mask_cnt;
	float m_minIrisAngle, m_maxIrisAngle;
	float m_minPupilAngle, m_maxPupilAngle;

	int m_availablePyramidLevel;
	CvPoint3D32f m_Iris;
	CvPoint3D32f m_Pupil;

	CvHistogram *m_tmpHist, *m_innerRingHist, *m_outerRingHist, *m_irisHistogram;
	IplImage *m_maskImage;
	IplImage *m_flatIris, *m_flatIrisMask;
	CvHistogram **m_annularHist;
	int m_angleSamples;

	CvMat *m_emdFlow;
	CvMat *m_transposedEMDFlow;
	float m_eyelidStartAngleRight, m_eyelidStartAngleLeft, m_eyelidStartAngleBottom;
	float m_eyelidEndAngleRight, m_eyelidEndAngleLeft, m_eyelidEndAngleBottom ;

	float m_eyelidSimilarityMeasure[6];
	float EyeLidMaximas[4];

	IplImage *m_upperEyelidMask;
	IplImage *m_lowerEyelidMask;

	float m_startEyelidRadiusRight; // radius of the circle
	float m_endEyelidRadiusRight; // changed to 180

	float m_startEyelidRadiusLeft;
	float m_endEyelidRadiusLeft;

	float m_minLidAngle;
	float m_maxLidAngle;

	double m_measureIris;
	double m_measurePupil;

	CCOMP_STATE *m_ccompServer;
	IplImage *m_eyelashMask;

	bool m_enableEyelidSegmentation;


	double m_coarseSearchSampling;
	double m_fineSearchSampling;
	float *m_fullCostData;
	float *m_fullCostDataMax;
	float *m_pScore;
	double m_eyelidSearchSampling;
	IplImage *m_dxTable;
	IplImage *m_dyTable;
	float *m_EyeLidSinTable;
	float *m_EyeLidCosTable;

	bool m_enableEyeQualityAssessment;
	bool m_enableHistogramCost;

	float m_defaultUpperEyelidAngle;
	float m_defaultLowerEyelidAngle;

	bool m_eyelidDetected;



	CvPoint3D32f *m_pPeak;


	IplImage **m_costMatrixFloat;
	IplImage **m_costMatrixFloatMax;
	IplImage *m_costMatrix;
	IplImage *m_countMatrix;

	IplImage *m_flatGradImage;

};
