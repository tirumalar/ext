#pragma once

class EyeSegmentServer;
class EyeMatchServer;
class EyeFeatureServer;
class EyeSegmentationOutput;

struct _IplImage;

#include <utility>
#include <map>
#include "EyeSegmentationLib_def.h"
#include "cxcore.h"

typedef struct
{
	float x;
	float y;
	float r;
} CircleParameters1;

typedef struct
{
	CircleParameters1 ip;
	CircleParameters1 pp;

}IrisPupilCircles;

class EYESEGMENTATIONLIB_EXPORTS_DLL_EXPORT EyeSegmentationInterface
{
public:
	void init(int scale=1, int w=640, int h=480);
	void term();
	EyeSegmentationInterface();
	virtual ~EyeSegmentationInterface();
	IrisPupilCircles Segment(unsigned char *imageBuffer, int w, int h, int stride);
	bool GetIrisCode2(unsigned char *flatIris, unsigned char *flatMask, int w, int h, int stride, 
		unsigned char *Iriscode, unsigned char *Maskcode);
	bool GetIrisCode(unsigned char *imageBuffer, int w, int h, int stride, 
		unsigned char *Iriscode, unsigned char *Maskcode, IrisPupilCircles *pCircles = NULL );
	bool GetRandomIrisCode(unsigned char *imageBuffer, int w, int h, int stride, unsigned char *Iriscode, unsigned char *Maskcode);
	bool GetDefaultMaskCode(unsigned char *IrisCode, unsigned char *Maskcode);  // return mask corresponding to default iris code masks

	void GetFlatIris( unsigned char *flatIris, int w, int h, int stride);
	void GetFlatMask( unsigned char *flatMask, int w, int h, int stride);
	void GetSpecularityMask(unsigned char *Mask, int w, int h, int stride);
	bool Getiseye();
	bool IsEyelidPresent();
	void SetDefaultEyelidAngles(float upperEyelidAngle, float lowerEyelidAngle);
	int checkBitCorruption(unsigned char* tag);
	int GetFeatureVariances(float *var);
	void GetRobustFeatureVariances(float *var);
	void SetFineSearchSampling(double sampling);
	void SetEyelidSearchSampling(double sampling);
	void SetCoarseSearchSampling(double sampling);
	void EnableEyelidSegmentation(bool enable=true);
	void SetUpperEyelidCenterandRadius(CvPoint cenPt,float rad);
	void SetLowerEyelidCenterandRadius(CvPoint cenPt,float rad);

	void SetPupilRadiusSearchRange(int min, int max);
	void SetPupilAngleSearchRange(int min, int max);
	void GetPupilAngleSearchRange(float& min, float& max);
	void SetIrisRadiusSearchRange(int min, int max);
	void SetEyeLocationSearchArea(int xo, int yo, int w, int h);
	void GetEyeLocationSearchArea(int& xo, int& yo, int& w, int& h);
	void SetLUT(int pupilmin5= 5,int pupilmax64 = 64,int cirmin5=5,int cirmax255 = 255);

	void EnableEyeQualityAssessment(bool enable=true);
	void EnableImprovedCostMetric(bool enable=true);
	int GetMaxCorruptBitsPercAllowed();
	void SetMaxCorruptBitsPercAllowed(int perc);
	bool GetEnableEyeQualityAssessment();

	int GetFeatureLength() const;
	int GetFeatureByteSize() const;
	int GetFeatureNumRows() const;
	_IplImage** GetImagePyramid();
	float GetCorruptBitsPerc(){ return m_corruptBitcountPerc;}
private:
	EyeSegmentServer *m_pEyeSegmentServer;
	EyeFeatureServer *m_pEyeFeatureServer;
	EyeSegmentationOutput *m_eso;
	struct _IplImage *m_flatMask;
	struct _IplImage *m_flatMaskIntegral;
	int m_index;
	int m_maxCorruptBitsPercAllowed;
	float m_corruptBitcountPerc;
};

class EYESEGMENTATIONLIB_EXPORTS_DLL_EXPORT IrisMatchInterface
{
public:
	IrisMatchInterface(int featureLength=5120, int numRows=8, int byteSize=8,int shift=12);
	~IrisMatchInterface();
	void init();
	void term();
	std::pair<int, float> match_pair(unsigned char *c1, unsigned char *m1,unsigned char *c2, unsigned char *m2,unsigned int maskval=0xFFFFFFFF);
	std::pair<int, int> match_pairNumDen(unsigned char *c1, unsigned char *m1,unsigned char *c2, unsigned char *m2, unsigned int maskval=0xFFFFFFFF);
	std::pair<int, int> get_best_match_score();
	std::pair<int, float> match_database(unsigned char *c1,unsigned char *m1);
	std::pair<int, float> Match(unsigned char *Iriscode2, unsigned char *Maskcode2);
	std::pair<int, float> MatchDBRotation(unsigned char *f1ptr,unsigned char *m1ptr, unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask);
	std::pair<int, float> MatchDBNewOpt(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask);
	std::pair<int, float> MatchDBNewCompress(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask);

	void MakeShiftsForIris(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask);

	void SetShiftLeft(int left);
	void SetShiftRight(int right);
	//float *GetHammingDistances();
	int GetNumberOfRotations();
	void SetNominalCommonBits(int bits);
	void SetFeatureScale(int scale);
	void SetMinCommonBits(int commonBits);
	void SetDoRawScore(bool raw);
	static int GetCoarseIrisCode(unsigned char *irisCode, unsigned char *maskCode, int length, unsigned char *coarseIrisCode,unsigned char *coarseMaskCode);
	static int GetCoarseIrisCodeFromCompactDB(unsigned char *irisCode, unsigned char *maskCode, int length, unsigned char *coarseIrisCode,unsigned char *coarseMaskCode);
	void MakeShifts(unsigned char *f1ptr, unsigned char *m1ptr);
	void MakeShifts(unsigned char *Iriscode1, unsigned char *Maskcode1,unsigned int mask);
	std::pair<int, float> MatchWithShifts(unsigned char *f2ptr, unsigned char *m2ptr, unsigned int maskval);

private:
	EyeMatchServer *m_pEyeMatchServer;
	int m_featureLength, m_numRows, m_byteSize;
	int m_shift;
};
