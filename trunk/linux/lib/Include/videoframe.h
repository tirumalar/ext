#pragma once
#include "CvInterfaceDefs.h"
#include "Image.h"
#include "EyeCenterPoint.h"
#include "pyramid.h"

#include <list>
#include <vector>

#define MAXIMUM_HAMMING_DISTANCE 1.0


inline int reduce(int param, int level)
{
	for(int i = 0; i < level; i++)
	{
		param = (param + 1) / 2;
	}
	return param;
}

inline int expand(int param, int level)
{
	for(int i = 0; i < level; i++)
	{
		param *= 2;
	}
	return param;
}

class CSampleFrame{
public:
	CSampleFrame(Image8u *frame);
	CSampleFrame();
	~CSampleFrame();
	void setBinning(int bin){ m_Binning = bin;}
	Pyramid8u *GetPyramid() { return m_Pyramid; };
	EyeCenterPointList* GetEyeCenterPointList(){ return m_pEyeCenterPoints; }
	EyeCenterPointList* GetEyeCenterPoints() { return m_pEyeCenterPoints; }
	bool IsRealEye(bool debug);
	void SetNumberOfHaarEyes(int numberOfHaarEyes) { m_NumberOfHaarEyes = numberOfHaarEyes; }
	int GetNumberOfHaarEyes() const { return m_NumberOfHaarEyes; }

	void SetNumberOfSpecularityEyes(int numberOfSpecularityEyes) { m_NumberOfSpecularityEyes = numberOfSpecularityEyes; }
	int GetNumberOfSpecularityEyes() const { return m_NumberOfSpecularityEyes; }
//	const Image8u *GetImage() const { return m_Frame; }
//	Image8u *GetImage() { return m_Frame; }
	void SetImage(Image8u *frame);
	void GetCroppedEye(int index, IplImage *dest, int& left, int & top);
	float GetObjectDistance() const { return m_ObjectDistance; }
	void SetObjectDistance(float Object) { m_ObjectDistance = Object; }
	float GetFocusDistance() const { return m_FocusDistance; }
	void SetFocusDistance(float focus) { m_FocusDistance = focus; }
	void setScratch(char *scr) {m_Pyramid->setScratch(scr);}
	void getDims(int level, int& width, int& height);
	void saveImage(const char* prefix, int idx);
	void saveImage(const char* prefix, int idx,int level);
	int GetBinVal(){ return m_Binning;}
protected:
	EyeCenterPointList *m_pEyeCenterPoints;
	int m_NumberOfHaarEyes;
	int m_NumberOfSpecularityEyes;
	Image8u *m_Frame;
	float m_ObjectDistance;
	float m_FocusDistance;
	Pyramid8u *m_Pyramid;
	int m_Binning;
};
