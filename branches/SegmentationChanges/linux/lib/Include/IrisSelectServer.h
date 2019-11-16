#ifndef _IRIS_SELECT_SERVER_H
#define _IRIS_SELECT_SERVER_H

#include <opencv/cxcore.h>
#if (defined(__BFIN__)||(defined(__linux__)))
#define Ipp32f float
#define Ipp8u  unsigned char
#define IppiRect CvRect
#else
#include "ippdefs.h"
#endif
class IrisSelectServer
{
public:
	IrisSelectServer(int width=0, int height=0);

	virtual ~IrisSelectServer(void);

	float GetEigenvalueThreshold() const { return m_EigenvalueThreshold; }
	void SetEigenvalueThreshold( float threshold ) { m_EigenvalueThreshold = threshold; }

	void ComputeFeatureVector(IplImage *pImage, CvRect cr, int *pFeatureVector9);
	static void ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center,CvPoint2D32f &var,int radius=14);

protected:

	int m_width, m_height;
	IplImage *m_eigenVV;
	Ipp32f *m_hist;
	Ipp8u *m_pBuffer;
	int m_extraStep;

	float m_EigenvalueThreshold;

	int GetEigenValueHistograms(IplImage *img, IppiRect rect, Ipp32f *histSpace);
	int GetEigenValueRatioHistograms(IplImage *img, IppiRect rect, Ipp32f *hist);
	int ExtractHistogramFeatures(Ipp32f *hist, int *feature_vector);
};

#endif

