#pragma once
					
#include <cxcore.h>


class LaplacianBasedFocusDetector
{
public:
	LaplacianBasedFocusDetector(int width=320, int height=240);

	virtual ~LaplacianBasedFocusDetector(void);
	CvPoint3D32f ComputeFocus(IplImage* frame, unsigned char specVal=255);
	CvPoint3D32f ComputeVerticalFocus(IplImage* frame, unsigned char specVal=255);
	CvPoint3D32f ComputeRegressionFocus(IplImage* frame, unsigned char specVal=255);

private:
	int m_width, m_height;
	IplImage *pyrImage1;
	IplImage *pyrImage2;
	IplImage *pyrImage3;
	IplImage *pyrImage4;
	IplImage *maskImage;
};

