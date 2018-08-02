#pragma once
#include <opencv/cxcore.h>

#ifdef __BFIN__
extern "C"{
	#include "EdgeImage_private.h"
}
#endif

class EyeMatchServer
{
public:
	EyeMatchServer(int featureLength, int numRows, int byteSize, int shift);
public:
	~EyeMatchServer(void);

	double Match(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval);
	std::pair<int,int> MatchNumDen(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr, unsigned int maskval);

	CvPoint GetBestMatchScore() { return m_bestMatchScore; }

	void SetShiftLeft(int left) { m_ShiftLeft = left; }
	void SetShiftRight(int right) { m_ShiftRight = right; }
	//float *GetHammingDistances() { return &m_HammingDistances[0]; }
	int GetNumberOfRotations() { return (m_ShiftRight - m_ShiftLeft + 1); }
	void SetNominalCommonBits(double numBits) { m_nominalCommonBits = numBits; }
	void SetMinCommonBits(int numBits) { m_minCommonBits = numBits; }

	void SetDoRawScore(bool mode) { m_DoRawScore = mode; }
	bool GetDoRawScore() const { return m_DoRawScore; }
	void MakeShifts(unsigned char *f1ptr, unsigned char *m1ptr);
	double MatchWithShifts(unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval);

	void MakeShifts(unsigned char *f1ptr, unsigned char *m1ptr,unsigned int uiMask);
	CvPoint GetHammingDistance_global(unsigned char *f1, unsigned char *m1, unsigned char *f2, unsigned char *m2, int featureMask);
	CvPoint GetHammingDistance_globalOpt(unsigned char *f1, unsigned char *m1, unsigned char *f2, unsigned char *m2, int featureMask);
	double Match(unsigned char* f2ptr, unsigned char* m2ptr,int featureMask);
	void PrintAllShift();

	void MakeShiftsInternal(unsigned char *codeTemp, unsigned char *maskTemp, int featureLength, int numRows, int shiftScale);
	void Compress_Shift(unsigned char *input, unsigned char *output, int rows, int cols);
	void MakeShifts_x0F(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask,void *scratch );
	void MakeShiftsCompress(unsigned char *f1ptr, unsigned char *m1ptr);
	void LeftShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift, int jump, int numRows);
	void RightShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift, int jump, int numRows);
	unsigned char* GetIris(unsigned char *DB,int eyenum);
	unsigned char* GetMask(unsigned char *DB,int eyenum);

//This requires the Database is not compressed
	std::pair<int, float> MatchDBRotation(unsigned char *f1ptr,unsigned char *m1ptr, unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask);
	void MakeShiftsForIris(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask);
	std::pair<int, float> MatchDBNewOpt(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask, unsigned char *shiftedCode,unsigned char *shiftedMask);

//These required for CompressDB Iris Matching
	unsigned char* GetCompressIris(unsigned char *DB,int eyenum);
	unsigned char* GetCompressMask(unsigned char *DB,int eyenum);
	std::pair<int, float> MatchDBNewCompress(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask);

protected:
	std::pair<int, float> MatchDBNew(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask);

	void LeftShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift);
	void RightShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift);
	CvPoint GetHammingDistanceNew(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval,int step);


	int UpdateWith32bytesCushion(unsigned char *f1ptr, unsigned char *m1ptr);
	CvPoint GetHammingDistance(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval);
	CvPoint GetHammingDistance1(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval,int step);

	int m_ShiftLeft;
	int m_ShiftRight;
	int m_ShiftIncrement,m_bestShiftIndex;

	float m_HammingDistances[1000];


	int m_featureLength;
	int m_numRows;
	int m_byteSize;
	CvPoint m_bestMatchScore;
	double m_nominalCommonBits;
	int m_minCommonBits;

	bool m_DoRawScore;


//	int *pbits,*pdist;
	unsigned char *m_temp[4];

	int m_lut[256];

	unsigned char **m_maskArray;
	unsigned char **m_codeArray;
	unsigned char *m_maskTemp;

	unsigned char *m_shiftedCode[36];
	unsigned char *m_shiftedMask[36];
	unsigned char *m_lut16;
	void *m_scratch;
};
