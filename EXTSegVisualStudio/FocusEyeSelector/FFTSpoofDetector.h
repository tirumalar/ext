#pragma once
#include <cxcore.h>
#include <fftw3.h>

#if (defined(__BFIN__) ||defined(__linux__))
extern "C"
{
#include "EdgeImage_private.h"
};
#endif

class FFTSpoofDetector
{
public:
	FFTSpoofDetector(int width=640, int height=480, int level=0, int pw = 256, int ph = 256);
public:
	~FFTSpoofDetector(void);
	
	int check(IplImage *image);

	void SetPeakSearchOffsets(int w, int h, double threshold);

#ifndef UNITTEST
protected:
#endif
	void PopulateData(IplImage* img1,IplImage* out);
	void ComputeMagnitude(unsigned int * outptr1,int iw,int ih);
	void ComputeMagnitude(complex_fract16 *ptr ,unsigned int * outptr1,int iw,int ih);
	void Copy8to16(IplImage* image,IplImage *out);
	void SwapQuad(void);
	long int ComputeSum(unsigned int *ptr, int widthstep,int width,int height);
	int m_width, m_height;
	int m_xOffset;
	int m_yOffset;
	double m_peakThreshold;

#ifdef IPP
	Ipp8u *m_pBuffer;
	Image32f *m_fftImage;
	Image32f *m_fftMagImage;
	Image32f *m_fftMagIntImage;
	Image32f *m_fImage;
	IppiMomentState_64f *m_momentsState;
	IppiFFTSpec_R_32f *m_lowSpec;
#else
	IplImage *m_fftImage;
	IplImage *m_fftOutImage;
	IplImage *m_fftMagImage;
	complex_fract16* m_Twiddle;
	int m_maxPeakCount;
	CvPoint *m_PeakBuf;
#endif

#ifdef __linux__
	fftw_complex *m_fftIn,*m_fftOut;
	fftw_plan    m_planFft;
#endif

};
