#ifndef _IRIS_SELECT_SERVER_H
#define _IRIS_SELECT_SERVER_H
					
#include <opencv/cxcore.h>
#include <map>

class SpecularityBasedSpoofDetector
{
public:
	SpecularityBasedSpoofDetector(int width=0, int height=0);

	virtual ~SpecularityBasedSpoofDetector(void);
	bool check(char *img, int w, int h, int widthStep);
	bool check2(char *img, int w, int h, int widthStep);
	void reset() {	min_ratio = 100.0; max_ratio = 0.0; min_pupil_size = 1000.0;}
	void set_min_max_threshold(double min_thresh, double max_thresh) {m_minThresh = min_thresh; m_maxThresh = max_thresh;}

private:
	
	int m_width, m_height;
	IplImage *m_eigenVV;
	unsigned char *m_pBuffer;
	int m_extraStep;
	IplImage *pyrImage;
	IplImage *pyrImage2;
	IplImage *pyrImage3;
	double min_ratio;
	double max_ratio;
	double m_minThresh;
	double m_maxThresh;
	int m_maxPupilDiameter;
	double min_pupil_size;
	char *m_TempBuffer;

	void ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius=14);
//	void GetEigenValues(IplImage *img, IppiRect rect, Ipp32f *hist);
	std::pair<double, double> ComputeHistogram(IplImage *img, CvPoint pt, CvSize sz);
};

#endif

