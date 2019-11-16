#include "SpecularityBasedSpoofDetector.h"
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <stdio.h>
extern "C" {
#include "file_manip.h"
#include "EdgeImage_private.h"

}
SpecularityBasedSpoofDetector::SpecularityBasedSpoofDetector(int width, int height): m_width(width), m_height(height),
m_eigenVV(0), pyrImage(0), pyrImage2(0), pyrImage3(0), m_minThresh(1.25f), m_maxThresh(1.75f), m_maxPupilDiameter(150)
{
	m_extraStep = 6;
	m_eigenVV = cvCreateImage(cvSize(width*m_extraStep/8, height/8), IPL_DEPTH_32F, 1);

	CvRect roiSize = {0,0,width/8, height/8};
/*
	  	int apertureSize = 3;
//	IppiKernelType kernType = ippKernelSobel;
	int avgWindow = 5;
	int bufferSize = 0;

	if(m_extraStep == 1)
		ippiMinEigenValGetBufferSize_8u32f_C1R(roiSize, apertureSize, avgWindow, &bufferSize);
	else
		ippiEigenValsVecsGetBufferSize_32f_C1R(roiSize, apertureSize, avgWindow, &bufferSize);

	m_pBuffer = (Ipp8u *) malloc(bufferSize);
*/
	pyrImage = cvCreateImage(cvSize(width/2, height/2), IPL_DEPTH_8U, 1);
	pyrImage2 = cvCreateImage(cvSize(width/4, height/4), IPL_DEPTH_8U, 1);
	pyrImage3 = cvCreateImage(cvSize(width/8, height/8), IPL_DEPTH_8U, 1);
	m_TempBuffer = (char *)malloc(32*4*width/8);
//	m_Eigen = cvCreateImage(cvSize(width,height/8),  IPL_DEPTH_32F, 1);

	reset();
}

SpecularityBasedSpoofDetector::~SpecularityBasedSpoofDetector(void)
{
	if(m_eigenVV)		cvReleaseImage(&m_eigenVV);
	if(m_pBuffer)		free(m_pBuffer);	m_pBuffer = 0;
	if(pyrImage)		cvReleaseImage(&pyrImage);	pyrImage = 0;
	if(pyrImage2)		cvReleaseImage(&pyrImage2);	pyrImage2 = 0;
	if(pyrImage3)		cvReleaseImage(&pyrImage3);	pyrImage3 = 0;
}

/*
void SpecularityBasedSpoofDetector::GetEigenValues(IplImage *img, IppiRect rect, Ipp32f *hist)
{
	int srcStep = img->widthStep;
	Ipp8u *imgPtr = (Ipp8u *) img->imageData + (rect.y - 2) * srcStep + (rect.x - 2);

	int eigStep = m_eigenVV->widthStep;
	Ipp32f *pEigenVV = (Ipp32f *) (m_eigenVV->imageData + (rect.y - 2) * eigStep) + m_extraStep*(rect.x - 2);

	int apertureSize = 3;
	IppiKernelType kernType = ippKernelSobel;
	int avgWindow = 5;
	IppiSize roiSize = {rect.width+4, rect.height+4};

	IppStatus status;
	
	if(m_extraStep == 1)
		status = ippiMinEigenVal_8u32f_C1R(imgPtr, srcStep, pEigenVV, eigStep, roiSize, kernType, apertureSize, avgWindow, m_pBuffer);
	else
		status = ippiEigenValsVecs_8u32f_C1R(imgPtr, srcStep, pEigenVV, eigStep, roiSize, kernType, apertureSize, avgWindow, m_pBuffer);

	rect.x += rect.width >> 1;
	rect.y += rect.height >> 1;

	pEigenVV = (Ipp32f *) (m_eigenVV->imageData + rect.y * eigStep) + m_extraStep*rect.x;

	hist[0] = pEigenVV[0];
	hist[1] = pEigenVV[1];
}
*/

void SpecularityBasedSpoofDetector::ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius)
{
	// Do small search for max specularity:
	unsigned char maxValue = 0, peakThreshold = 0;
	int nx = frame->width >> 1, ny = frame->height >> 1;
	int oy, ox;

	for(int i = ny-4; i < ny+5; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-4; j < nx+5; j++)
		{
			unsigned char value = iptr[j]; 
			if(value > maxValue)
			{
				maxValue = value;
				oy = i;
				ox = j;
			}
		}
	}
	
	nx = ox; ny = oy;

	// computing the mean value 
	int meanValueX = 0, meanValueY = 0;
	int varX = 0, varY = 0;
	int count = 0;
	
	int peak_threshold = (31*maxValue) >> 5;

	for(int i = ny-radius; i <= ny+radius; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-radius; j < nx+radius; j++)
		{
			int value = iptr[j];

			if(value > peak_threshold)
			{
				meanValueX += j;
				meanValueY += i;
				varX += j*j;
				varY += i*i;
				count ++;
			}
		}
	}
	
	ny = (meanValueY + (count>>1))/count;
	nx = (meanValueX + (count>>1))/count;
	radius = radius>>1;

	peak_threshold = (7*maxValue) >> 3;
	meanValueX = 0; meanValueY = 0;
	varX = 0; varY = 0;
	count = 0;

	for(int i = ny-radius; i <= ny+radius; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-radius; j < nx+radius; j++)
		{
			int value = iptr[j];

			if(value > peak_threshold)
			{
				meanValueX += j;
				meanValueY += i;
				varX += j*j;
				varY += i*i;
				count ++;
			}
		}
	}

	center = cvPoint2D32f((float) meanValueX / count, (float) meanValueY / count);
	var = cvPoint2D32f((float) varX/count - center.x * center.x, (float) varY/count - center.y * center.y);
}

bool SpecularityBasedSpoofDetector::check(char *img, int w, int h, int widthStep)
{
	IplImage input;

	cvInitImageHeader(&input, cvSize(w,h), IPL_DEPTH_8U, 1);
	cvSetData(&input, img, widthStep);

	cvPyrDown( &input, pyrImage, CV_GAUSSIAN_5x5 );
	cvPyrDown( pyrImage, pyrImage2, CV_GAUSSIAN_5x5 );
	cvPyrDown( pyrImage2, pyrImage3, CV_GAUSSIAN_5x5 );
	
	CvPoint2D32f pt, var;

	ComputeSpecularityMetrics(pyrImage, pt, var, 14);

	float hist[2];

  	CvRect rect;
	rect.x = cvFloor(pt.x/4.0f - 4);
	rect.y = cvFloor(pt.y/4.0f - 4);
	rect.width = 8;
	rect.height = 8;
/*
	GetEigenValues(pyrImage3, rect, hist);
*/
//			printf("%f %f %f %f %f\n", pt.x*2, pt.y*2, var.y/var.x, hist[0], hist[1]);
	double ratio = var.y/var.x;

	if(min_ratio > ratio && hist[0] > 7.5f)	min_ratio = ratio;
	if(max_ratio < ratio && hist[0] > 7.5f)	max_ratio = ratio;

//	printf("%f %f\n", min_ratio, max_ratio);

	return (min_ratio < m_minThresh && max_ratio > m_maxThresh);
}

void filter_hist(int *hist, int *fhist, int width)
{
	for(int i=2;i<width-2;i++)
		fhist[i] = ((hist[i-2] + hist[i+2]) + (hist[i-1] + hist[i+1])*4 + 6*hist[i] + 8)>>4;

	fhist[0] = (hist[0]*6 + hist[1]*4 + hist[2] + 8)>>4;
	fhist[1] = (hist[1]*6 + (hist[0] + hist[2])*4 + hist[3] + 8)>>4;

	fhist[width-1] = (hist[width-1]*6 + hist[width-2]*4 + hist[width-3] + 8)>>4;
	fhist[width-2] = (hist[width-2]*6 + (hist[width-1] + hist[width-3])*4 + hist[width-4] + 8)>>4;
}

void find_peaks(int *hist, int *fhist, int width)
{
	fhist[0] = 0;
	
	unsigned char *peak = (unsigned char *) calloc(width, 1);

	for(int i=1;i<width;i++)
		fhist[i] = hist[i] - hist[i-1];

	for(int i=1;i<width-1;i++)
	{
		if(fhist[i]*fhist[i-1] < 0)
			peak[i] = 1;
	}

	free(peak);
}

std::pair<double, double> SpecularityBasedSpoofDetector::ComputeHistogram(IplImage *img, CvPoint pt, CvSize sz)
{
	int *left_hist = (int *) calloc(sz.width, sizeof(int));
	int *right_hist = (int *) calloc(sz.width, sizeof(int));

	for(int i=0;i<sz.height;i++)
	{
		unsigned char *iptr = (unsigned char *) (img->imageData + img->widthStep * (pt.y-sz.height/2) + pt.x-sz.width);
		for(int j=0;j<sz.width;j++, iptr++)
		{
			if(*iptr > 0)
				left_hist[sz.width-1-j] += *iptr;
			else
				left_hist[sz.width-1-j] += 255;
		}

		for(int j=0;j<sz.width;j++, iptr++)
		{
			if(*iptr > 0)
				right_hist[j] += *iptr;
			else
				right_hist[j] += 255;
		}
	}
	
	int lidx = 0, ridx = 0;
	for(int i=0;i<sz.width;i++)
	{
		if(left_hist[lidx] > left_hist[i])
			lidx = i;

		if(right_hist[ridx] > right_hist[i])
			ridx = i;
	}

	int max_diff = MAX(150.0, cvCeil(0.2*left_hist[lidx]));
	int lidx_plus = lidx, lidx_minus = lidx;

	for(int i=lidx;i<sz.width-2;i++)
	{
		if(abs(left_hist[lidx] - left_hist[i]) > max_diff && abs(left_hist[lidx] - left_hist[i+1]) > max_diff
			&& abs(left_hist[lidx] - left_hist[i+2]) > max_diff)
		{
			lidx_plus = i;
			break;
		}
	}
	for(int i=lidx;i>1;i--)
	{
		if(abs(left_hist[lidx] - left_hist[i]) > max_diff && abs(left_hist[lidx] - left_hist[i-1]) > max_diff
			&& abs(left_hist[lidx] - left_hist[i-2]) > max_diff)
		{
			lidx_minus = i;
			break;
		}
	}

	max_diff = MAX(150.0, cvCeil(0.2*right_hist[ridx]));
	int ridx_plus = ridx, ridx_minus = ridx;

	for(int i=ridx;i<sz.width-2;i++)
	{
		if(abs(right_hist[ridx] - right_hist[i]) > max_diff && abs(right_hist[ridx] - right_hist[i+1]) > max_diff
			&& abs(right_hist[ridx] - right_hist[i+2]) > max_diff)
		{
			ridx_plus = i;
			break;
		}
	}
	for(int i=ridx;i>1;i--)
	{
		if(abs(right_hist[ridx] - right_hist[i]) > max_diff && abs(right_hist[ridx] - right_hist[i-1]) > max_diff
			&& abs(right_hist[ridx] - right_hist[i-2]) > max_diff)
		{
			ridx_minus = i;
			break;
		}
	}

	// detecting a specularity on the iris/pupil boundary

	double ratio = 0.0;

	double pupil_size = 0;

	if(right_hist[ridx] > 800 && left_hist[lidx] > 800)
	{
		printf("Pupil is too bright, can't be real\n");
	}
	else if(ridx_plus + lidx_plus > m_maxPupilDiameter || lidx_minus > 25 || ridx_minus > 25 
		|| right_hist[ridx] > 800 || left_hist[lidx] > 800
		|| (double) right_hist[ridx]/ (double) left_hist[lidx] > 1.25 || (double) left_hist[ridx]/ (double) right_hist[lidx] > 1.25)
	{
		if(right_hist[ridx] < left_hist[lidx])
		{
			printf("specularity at left boundary\n");
			pupil_size = ridx_plus - ridx_minus;
			ratio = 1.0;
		}
		else
		{
			printf("specularity at right boundary\n");
			pupil_size = lidx_plus - lidx_minus;
			ratio = 0.00000001;
		}
	}
	else
	{
//		ratio = (double) (ridx_plus - ridx_minus)/ (double) (lidx_plus - lidx_minus);
		ratio = (double) (ridx_plus) / (double) (lidx_plus + ridx_minus);
		pupil_size = ridx_plus + lidx_plus;
	}
	
	free(left_hist);
	free(right_hist);	
	
	return std::make_pair(ratio, pupil_size);
}

bool SpecularityBasedSpoofDetector::check2(char *img, int w, int h, int widthStep)
{
	IplImage input;

	cvInitImageHeader(&input, cvSize(w,h), IPL_DEPTH_8U, 1);
	cvSetData(&input, img, widthStep);

	XTIME_OP("PyrDown",
	{
		cvPyrDown( &input, pyrImage, CV_GAUSSIAN_5x5 );
		cvPyrDown( pyrImage, pyrImage2, CV_GAUSSIAN_5x5 );
		cvPyrDown( pyrImage2, pyrImage3, CV_GAUSSIAN_5x5 );
	}
	);
	
	CvPoint2D32f pt, var;
	XTIME_OP("CompSpecMatrics",
		ComputeSpecularityMetrics(pyrImage, pt, var, 14);
	);
	std::pair<double, double> ratio_size;
	XTIME_OP("CompHist",
	{
			ratio_size = ComputeHistogram(&input, cvPoint(cvRound(pt.x * 2.0f), cvRound(pt.y * 2.0f)), cvSize(160, 8));
	}
	);
	double ratio = ratio_size.first;
	double pupil_size = ratio_size.second;

	float hist[2]={0,0};
	CvRect rect;
	rect.x = cvFloor(pt.x/4.0f - 4); rect.y = cvFloor(pt.y/4.0f - 4); rect.width = 8; rect.height = 8;
	
	float k = 65536.0*3.0f/(64.0f);

	rect.x = MAX(4,((rect.x-4)>>2)<<2);
	rect.width = ((rect.width+4+4)+3)&(~3);
	rect.y = MAX(1,(rect.y-4));
	rect.height = MIN(pyrImage3->height-1,rect.height+4+4);

	int Param[10];

	IplImage *eigen;
	char *imagedata;

	memset(m_TempBuffer, 0,(rect.width*4)*25);

	Param[0] = rect.width;//input w
	Param[1] = rect.height;//input h
	Param[2] = pyrImage3->widthStep; //inputstep
	Param[3] = rect.width*4; //Actual outputstep for other int buffer //int buffers
	Param[4] = (int)m_TempBuffer; //tempbuffer
	Param[5] = (int)k; //constant K

	imagedata = pyrImage3->imageData+(rect.y*pyrImage3->widthStep)+rect.x;
	int  t_maxMinEigValue = compute_EigenVals((unsigned char*)imagedata,(int*)m_eigenVV->imageData, Param);

	int* pEigenVV = (int *) (m_eigenVV->imageData + (rect.height >> 1) * rect.width*4) + (rect.width>>1);

	if( pEigenVV[0]>0){
		hist[0] = pEigenVV[0];
		hist[1] = pEigenVV[1];
	}
//	GetEigenValues(pyrImage3, rect, hist);
	// at this stage -> eigen value are downscaled by 64
	// opencv uses downscaling of 256*256*4*25
	// extra scale = 256*4*4*25
	// 0.01 gets replaced by 0.01 * 256*4*4*25 =

	if(ratio > 0.0 && hist[0] > (7.5f*256*4*4*25))
	{
		if(min_pupil_size > pupil_size)	min_pupil_size = pupil_size;
		if(min_ratio > ratio)	min_ratio = ratio;
		if(max_ratio < ratio)	max_ratio = ratio;
	}

	double movement = min_pupil_size * (max_ratio - min_ratio);
	printf("Movement is %f\n", (float) movement);
	printf("specularity size is %f\n", hist[0]);

	return (movement > m_maxThresh);
}
