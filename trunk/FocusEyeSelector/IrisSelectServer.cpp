#include "IrisSelectServer.h"
#include <cxcore.h>
#include <stdio.h>

#define IRIS_SPECULARITY_COVARIANCE_EIGENVALUE_THRESHOLD	7.0


#if (defined(__BFIN__) ||defined(__linux__))
extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}

extern "C"
{
#include "EdgeImage_private.h"
};
#define IppiSize CvSize

#else
#include "ippi.h"
#include "ippcv.h"

#define XTIME_OP(m, op) { \
	op;\
	}


#endif


IrisSelectServer::IrisSelectServer(int width, int height): m_width(width), m_height(height)
, m_EigenvalueThreshold( IRIS_SPECULARITY_COVARIANCE_EIGENVALUE_THRESHOLD )
, m_eigenVV(0)
{
	m_extraStep = 1;
	m_eigenVV = cvCreateImage(cvSize(width*m_extraStep, height), IPL_DEPTH_32F, 1);
	m_hist = (Ipp32f *) calloc(1000, sizeof(Ipp32f));

	IppiSize roiSize = {width, height};
	int apertureSize = 3;

#if !((defined(__BFIN__) ||defined(__linux__)))
	IppiKernelType kernType = ippKernelSobel;
#endif
	int avgWindow = 5;
	int bufferSize = 0;

#if (defined(__BFIN__) ||defined(__linux__))
#ifdef __C_FIXED__
	bufferSize = width*height*apertureSize*5; //decide later
#endif


#else
	if(m_extraStep == 1)
		ippiMinEigenValGetBufferSize_8u32f_C1R(roiSize, apertureSize, avgWindow, &bufferSize);
	else
		ippiEigenValsVecsGetBufferSize_32f_C1R(roiSize, apertureSize, avgWindow, &bufferSize);
#endif

	m_pBuffer = (Ipp8u *) malloc(bufferSize);

}

IrisSelectServer::~IrisSelectServer(void)
{
	if(m_eigenVV)		cvReleaseImage(&m_eigenVV);
	if(m_hist)			free(m_hist);	m_hist = 0;
	if(m_pBuffer)		free(m_pBuffer);	m_pBuffer = 0;
}

void IrisSelectServer::ComputeFeatureVector(IplImage *pImage, CvRect cr, int *pFeatureVector9)
{
	IppiRect rect;

	rect.x = cr.x; rect.y = cr.y;
	rect.width = cr.width;
	rect.height = cr.width;

	if(m_extraStep == 1)
	{
	XTIME_OP("EigenHist",
		GetEigenValueHistograms(pImage, rect, m_hist)
		);
	}
	else
		GetEigenValueRatioHistograms(pImage, rect, m_hist);


	ExtractHistogramFeatures(m_hist, pFeatureVector9);
}

int IrisSelectServer::ExtractHistogramFeatures(Ipp32f *hist, int *feature_vector)
{
/*	8 octaves

	feature_vector[0] = // energy in top 1;
	feature_vector[1] = // energy in next 2 - 6 band
	feature_vector[2] = // energy in next 7 - 14 band
	feature_vector[3] = // energy in the 15 - 30 band
	feature_vector[4] = // energy in the 31 - 61 band
	feature_vector[5] = // energy in the 62 - 124 band
	feature_vector[6] = // energy in the 125 - 249 band
	feature_vector[7] = // energy in the 250 - 499 band
	feature_vector[8] = // energy in the 500 -  band
*/

	memset(feature_vector, 0, sizeof(int)*9);

	for(int i=0;i<1000;i++)
	{
		if(i < 1)			feature_vector[0]+=hist[i];
		else if(i < 6)		feature_vector[1]+=hist[i];
		else if(i < 14)		feature_vector[2]+=hist[i];
		else if(i < 30)		feature_vector[3]+=hist[i];
		else if(i < 61)		feature_vector[4]+=hist[i];
		else if(i < 124)	feature_vector[5]+=hist[i];
		else if(i < 249)	feature_vector[6]+=hist[i];
		else if(i < 499)	feature_vector[7]+=hist[i];
		else				feature_vector[8]+=hist[i];
	}

	return 1;
}

int IrisSelectServer::GetEigenValueHistograms(IplImage *img, IppiRect rect, Ipp32f *hist)
{
	int srcStep = img->widthStep;
	Ipp8u *imgPtr = (Ipp8u *) img->imageData + (rect.y - 2) * srcStep + (rect.x - 2);

	int eigStep = m_eigenVV->widthStep;
	Ipp32f *pEigenVV = (Ipp32f *) (m_eigenVV->imageData + (rect.y - 2) * eigStep) + m_extraStep*(rect.x - 2);

	int apertureSize = 3;

#if (!((defined(__BFIN__) ||defined(__linux__))))
	IppiKernelType kernType = ippKernelSobel;
#endif

	int avgWindow = 5;
	IppiSize roiSize;
	roiSize.width = rect.width+4;
	roiSize.height = rect.height+4;

#ifdef __BFIN__

{
	int Param[10];
	CvRect rect_asm;
	char *imagedata;
	int k = (int)((256.0f*4*4*25)/(200.0f*128));
	int *temp = (int*)m_eigenVV->imageData; //Array which is temp
	rect_asm.x = MAX(4,(rect.x>>2)<<2);
	rect_asm.width = (rect.width+3)&(~3);
	rect_asm.y = MAX(1,rect.y);
	rect_asm.height = MIN(img->height-1,rect.height);

	memset(temp, 0, (rect.width*4)*25);

	Param[0] = rect.width;//input w
	Param[1] = rect.height;//input h
	Param[2] = img->widthStep; //inputstep
	Param[3] = rect.width*4; //Actual outputstep for other int buffer //int buffers
	Param[4] = (int)temp; //tempbuffer
	Param[5] = (int)k; //constant K

	imagedata = img->imageData+(rect_asm.y*img->widthStep)+rect_asm.x;
	int  hist1 = compute_EigenValsHist1((unsigned char*)imagedata,Param);

//	printf("COUNT with ASM %d \n",hist1);

	hist[1] = hist1;
}
#else

#ifndef IPP
	CvRect rect_asm;
	int count =0;
	rect_asm.x = MAX(4,(rect.x>>2)<<2);
	rect_asm.width = (rect.width+3)&(~3);
	rect_asm.y = MAX(1,rect.y);
	rect_asm.height = MIN(img->height-1,rect.height);


    int x = rect_asm.x;
	int y = rect_asm.y;
	int width = rect_asm.width;
	int height = rect_asm.height;
	char *imageData = img->imageData+(rect_asm.y*img->widthStep)+rect_asm.x;
	int widthStep = img->widthStep;
    unsigned char *temp= (unsigned char *)m_eigenVV->imageData;
	int threshold = (int)((256.0f*4*4*25)/(200.0f*128));

	short *Gx,*Gy;
	int *sqGx,*sqGy,*GxGy,*sumSqGx,*sumSqGy,*sumGxGy;
	int *sqGxArr[5],*sqGyArr[5],*GxGyArr[5];

	Gx = (short*)temp;
	Gy = (short*)((unsigned char *)Gx + width*2);
	sqGx = (int *)((unsigned char *)Gy + width*2);
	sqGy = (int *)((unsigned char *)sqGx + width*4);

	int *sqGxA = sqGy + width;
	int *sqGyA = sqGxA + 5*width;

	for(int i=0;i<5;i++)
	{
		sqGxArr[i] = (int *)(sqGxA + width*i);
		sqGyArr[i] = (int *)(sqGyA + width*i);
	}

	sumSqGx =  (int *)(((unsigned char *)sqGyArr[4]) + width*4);
	sumSqGy =  (int *)(((unsigned char *)sumSqGx) + width*4);

	memset(temp, 0, width*21*4);
	int k =0;
	for(int i=0;i< height;i++)
	{
		unsigned char *_i1ptr = (unsigned char *) imageData + (i-1)*widthStep;
		unsigned char *i0ptr = (unsigned char *) imageData + (i)*widthStep;
		unsigned char *i1ptr = (unsigned char *) imageData + (i+1)*widthStep;

		for(int j=0;j<width; j++, i0ptr++)
		{
			Gx[j] = i0ptr[1] - i0ptr[-1];
			Gy[j] = *i1ptr++ - *_i1ptr++;

			sqGx[j] = Gx[j] * Gx[j];
			sqGy[j] = Gy[j] * Gy[j];
		}
		int *kGx,*kGy,*kGxGy;
		kGx = sqGxArr[k];
		kGy = sqGyArr[k];
		k++;
		if (k == 5)
		{
		 k=0;
		}

		int w = width;
		for(int j=0;j<width; j++, i0ptr++)
		{
			if (j <= 1)
			{
				kGx[j] = sqGx[0] + sqGx[1] + sqGx[2] + sqGx[3] + sqGx[4];
				kGy[j] = sqGy[0] + sqGy[1] + sqGy[2] + sqGy[3] + sqGy[4];
			}
			else if ((j>=2) && (j<w-2))
			{
			 kGx[j] = sqGx[j-2] + sqGx[j-1] + sqGx[j] + sqGx[j+1] + sqGx[j+2];
			 kGy[j] = sqGy[j-2] + sqGy[j-1] + sqGy[j] + sqGy[j+1] + sqGy[j+2];
			}
			else
			{
			 kGx[j] = sqGx[w-1] + sqGx[w-2] + sqGx[w-3] + sqGx[w-4] + sqGx[w-5];
			 kGy[j] = sqGy[w-1] + sqGy[w-2] + sqGy[w-3] + sqGy[w-4] + sqGy[w-5];
			}

			int vxx = *(sqGxArr[0] + j)+ *(sqGxArr[1] + j) + *(sqGxArr[2] + j) + *(sqGxArr[3] + j) + *(sqGxArr[4] + j);
			int vyy = *(sqGyArr[0] + j)+ *(sqGyArr[1] + j) + *(sqGyArr[2] + j) + *(sqGyArr[3] + j) + *(sqGyArr[4] + j);

			vxx = (vxx + 32)>>6;    // necessary as the squares in minEig calculation go beyond range
			vyy = (vyy + 32)>>6;

			if(vxx+vyy < threshold)
				count++;
		}

	}

	hist[1] = count;

#else
	IppStatus status = ippiMinEigenVal_8u32f_C1R(imgPtr, srcStep, pEigenVV, eigStep, roiSize, kernType, apertureSize, avgWindow, m_pBuffer);

	memset(hist, 0, sizeof(float)*1000);

	for(int i=0;i<rect.height;i++)
	{
		Ipp32f *eig = (Ipp32f *) (m_eigenVV->imageData + (rect.y+i) * eigStep) + rect.x;

		for(int j=0;j<rect.width; j++, eig++)
		{
			int index = cvRound((*eig) * 200 + 0.5);
			if(index < 1000)
				hist[index]++;
		}
	}
	printf("Hist value is %f\n", hist[1]);
#endif

#endif

	return 1;
}

int IrisSelectServer::GetEigenValueRatioHistograms(IplImage *img, IppiRect rect, Ipp32f *hist)
{
	int srcStep = img->widthStep;
	Ipp8u *imgPtr = (Ipp8u *) img->imageData + (rect.y - 2) * srcStep + (rect.x - 2);

	int eigStep = m_eigenVV->widthStep;
	Ipp32f *pEigenVV = (Ipp32f *) (m_eigenVV->imageData + (rect.y - 2) * eigStep) + m_extraStep*(rect.x - 2);

	int apertureSize = 3;

#if !((defined(__BFIN__) ||defined(__linux__)))
	IppiKernelType kernType = ippKernelSobel;
#endif
	int avgWindow = 5;
	IppiSize roiSize = {rect.width+4, rect.height+4};

#if !((defined(__BFIN__) ||defined(__linux__)))
	IppStatus status = ippiEigenValsVecs_8u32f_C1R(imgPtr, srcStep, pEigenVV, eigStep, roiSize, kernType, apertureSize, avgWindow, m_pBuffer);
#endif
	memset(hist, 0, sizeof(float)*1000);

	for(int i=0;i<rect.height;i++)
	{
		Ipp32f *eig = (Ipp32f *) (m_eigenVV->imageData + (rect.y+i) * eigStep) + m_extraStep*rect.x;

		for(int j=0;j<rect.width; j++, eig+=m_extraStep)
		{
			int index = cvRound(eig[1]*100.0/eig[0] + 0.5);
			if(index < 1000)
				hist[index]++;
		}
	}

	return 1;
}


void IrisSelectServer::ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center,CvPoint2D32f &var, int radius)
{
	//	cvSaveImage("Level1.pgm",frame);
	// Do small search for max specularity:
	unsigned char maxValue = 0, peakThreshold = 0;
	int nx = frame->width >> 1, ny = frame->height >> 1;

	for(int i = ny-4; i < ny+5; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-4; j < nx+5; j++)
		{
			unsigned char value = iptr[j]; 
			if(value > maxValue)
			{
				maxValue = value;
				ny = i;
				nx = j;
			}
		}
	}
	
	// computing the mean value 
	int meanValueX = 0, meanValueY = 0;
	int varX = 0, varY = 0;
	int count = 0;
	
	int peak_threshold = (3*maxValue) >> 2;

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
/*
	printf("Mean (%d,%d)\n",meanValueX,meanValueY);
	printf("Var (%d,%d)\n",varX,varY);
	printf("Cnt (%d)\n",count);
	printf("Thresh (%d)\n",peak_threshold);
*/

	center = cvPoint2D32f((float) meanValueX / count, (float) meanValueY / count);
	var = cvPoint2D32f((float) varX/count - center.x * center.x, (float) varY/count - center.y * center.y);
}
