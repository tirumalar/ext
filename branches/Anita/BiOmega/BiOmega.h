#pragma once

class IrisMatchInterface;
class EyeSegmentationInterface;

#include <utility>

#include "BiOmega_def.h"
#include "EyeSegmentationInterface.h"
struct _IplImage;
struct CvPoint2D32f;
struct CvPoint3D32f;

class BIOMEGA_DLL_EXPORTS BiOmega
{
public:
	BiOmega(int w=640, int h=480, int scale=1);
	~BiOmega();
	void SetEnableEyelidSegmentation(bool val);
	void SetUpperEyelidCenterandRadius(CvPoint cenPt,float rad);
	void SetLowerEyelidCenterandRadius(CvPoint cenPt,float rad);

	void SetNominalCommonBits(int bits);
	void SetMinCommonBits(int commonBits);
	void SetDoRawScore(bool raw);

	void SetLUT(int pupilmin5,int pupilmax64,int cirmin5,int cirmax255);
	int GetFeatureLength() const;
	bool SetEyeLocationSearchArea(int xo, int yo, int w, int h);
	void GetEyeLocationSearchArea(int& xo, int& yo, int& w, int& h);
	bool SetIrisRadiusSearchRange(int min, int max);
	bool SetPupilRadiusSearchRange(int min, int max);
	bool SetPupilAngleSearchRange(int min, int max);
	bool GetPupilAngleSearchRange(float& min, float& max);
	int GetMaxCorruptBitsPercAllowed();
	void SetMaxCorruptBitsPercAllowed(int perc);
	float GetCorruptBitsPerc(void);
	bool GetIrisCode(unsigned char *imageBuffer, int w, int h, int stride, char *Iriscode,IrisPupilCircles *pCircles=NULL, float *robustFetaureVariance=NULL );
	bool GetDefaultMaskCode(unsigned char *IrisCode);
	std::pair<int, float> MatchIris(unsigned char *imageBuffer, int w, int h, int stride, char *irisCodeDatabase, int numberOfEyes,IrisPupilCircles *pCircles=NULL,CvPoint2D32f *pRefCentre=NULL,CvPoint2D32f *pVar=NULL,int specrad =14);
//Special function for Segmentation
	std::pair<int, float> MatchIrisCode(char * IrisCode ,char *irisCodeDatabase, int numberOfEyes);
	std::pair<int, float> MatchIrisCodeSingle(char * IrisCode ,char *irisCodeDatabase,unsigned int maskval=0xFFFFFFFF);
	
	bool ProcessIris(unsigned char *imageBuffer, int w, int h, int stride,char *l_refIrisCode,IrisPupilCircles *pCircles=NULL,CvPoint2D32f *pRefCentre=NULL,CvPoint2D32f *pVar=NULL,int specrad=14);
	const char *GetVersion() const;
	IrisMatchInterface *GetMatchInterface(){ return m_pIrisMatchInterface;}
	EyeSegmentationInterface *GetEyeSegmentationInterface() { return m_pEyeSegmentInterface; }
private:
	IrisMatchInterface *m_pIrisMatchInterface;
	EyeSegmentationInterface *m_pEyeSegmentInterface;
	int m_MatchKeyByteLen;
	int m_MatchKeyBitLen;
	int m_IrisIDByteLen;
	static char *m_softwareVersion;
};
