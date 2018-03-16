#include <stdio.h>
#include "EyeSegmentServer.h"
#include "DistanceMetrics.h"
#include "useful.h"
#include <cv.h>
#include <map>
#include "useful.h"
#include "area_concom.h"
#include <algorithm>
using std::max;
using std::min;


// #define SEG
//#define PROFILE

#ifdef PROFILE
#include <windows.h>
LARGE_INTEGER profiler_freq;
//FILE * TimeInfo;
#define PROFILE_START(affine) LARGE_INTEGER start_time##affine; QueryPerformanceCounter(&start_time##affine);
#define PROFILE_END(affine)\
	LARGE_INTEGER end_time##affine;\
	static float total_time##affine = 0.0;\
	static int myCounter##affine=0;\
	myCounter##affine++;\
	QueryPerformanceCounter(&end_time##affine);\
	float time##affine =(float)(end_time##affine.QuadPart-start_time##affine.QuadPart)/profiler_freq.QuadPart*1000;\
	total_time##affine +=time##affine;\
	printf("\n%s = %fmS Total=%fmS iter = %d",#affine,time##affine, total_time##affine,myCounter##affine);\
	//	fprintf(TimeInfo,"\n%s = %fmS Total=%fmS iter = %d",#affine,time##affine, total_time##affine,myCounter##affine);
#else
#define PROFILE_START(affine)
#define PROFILE_END(affine)
#endif


extern "C" {
	#include "file_manip.h"
}

int getNumSamplesEyeLid(float s, float e, float sp)
{
	int index = 0;

	for(float x = s; x < e; x += sp)
		index++;

	return index;
}

int getNumSamplesInclusiveEyeLid(float s, float e, float sp)
{
	int index = 0;

	for(float x = s; x <= e; x += sp)
		index++;

	return index;
}

int find_disparities(IplImage *Iris, IplImage *input_mask, IplImage *output_mask)
{
#ifdef FINDIRIS
	FILE *fp = fopen("Eyelid.txt", "at");
	fprintf(fp,"Eyelid h=%d w=%d",Iris->width,Iris->height);
	fclose(fp);
#endif
	int *hist1 = (int *) calloc(64, sizeof(int));
	int *hist2 = (int *) calloc(64, sizeof(int));

	for(int i = 0;i<Iris->height;i++)
	{
		unsigned char *mptr = (unsigned char *) input_mask->imageData + i*input_mask->widthStep;
		unsigned char *iptr = (unsigned char *) Iris->imageData + i*Iris->widthStep;
		int w = input_mask->width;

		int j=0;

		for(;j<w/2;j++)
			if(!*mptr++)
				hist1[iptr[j] >> 2]++;

		for(; j<w; j++)
			if(!*mptr++)
			{
				if(i < 21)	hist1[iptr[j] >> 2]++;
				else		hist2[iptr[j] >> 2]++;
			}
	}

	cvSetZero(output_mask);

	/*for(int i = 0;i<Iris->height;i++)
	{
		int w = output_mask->width;
		unsigned char *mptr = (unsigned char *) output_mask->imageData + i*output_mask->widthStep + w/2;
		unsigned char *iptr = (unsigned char *) Iris->imageData + i*Iris->widthStep + w/2;

		for(int j=0;j<w/2;j++, mptr++)
		{
			int idx = *iptr++ >> 2;
			if(hist2[idx] > 3*hist1[idx])
				*mptr = 255;
		}
	}*/

	free(hist1);
	free(hist2);

	return 1;
}


extern int labelindx;
int EyeSegmentServer::findEyeLashes(IplImage* Iris, IplImage* mask)
{
	cvOr(m_lowerEyelidMask, m_maskFlatPyr[0], m_eyelashMask);
/*
	char fName[100];
	sprintf(fName,"EyelashMask_%d.pgm",labelindx);
	cvSaveImage(fName,m_eyelashMask );

	sprintf(fName,"Iris_%d.pgm",labelindx);
	cvSaveImage(fName,Iris );

	sprintf(fName,"Maskbef_%d.pgm",labelindx);
	cvSaveImage(fName,mask );
*/
	find_disparities(Iris, m_eyelashMask, m_eyelashMask);
/*
	sprintf(fName,"EyelashMaskDis_%d.pgm",labelindx);
	cvSaveImage(fName,m_eyelashMask );
*/
	bool tag = ccomp_label_regions(m_ccompServer,m_eyelashMask,1,1,m_eyelashMask->width-2,m_eyelashMask->height-2);

	int numBlobs = ccomp_get_num_blobs(m_ccompServer);

	CCOMP_BLOB* blob = ccomp_get_blobs(m_ccompServer);
/*	printf("\nNUMBER of BLOB %d",numBlobs);
	for(int i=0;i< numBlobs;i++)
	{
		printf("\nId %3d,Cx %6.2f,Cy %6.2f,Pix %5d,(%4d,%4d,%4d,%4d)\n",blob[i].id,blob[i].Cx,blob[i].Cy,blob[i].num_pixels,blob[i].roi.x,blob[i].roi.y,blob[i].roi.width,blob[i].roi.height);
	}
	printf("\n");
*/
	IplImage *labelImage = ccomp_get_labels(m_ccompServer);


	for(int i = 0;i<labelImage->height;i++)
	{
		unsigned char *mptr = (unsigned char *) mask->imageData + i*mask->widthStep;
		short *lptr = (short *) (labelImage->imageData + i*labelImage->widthStep);

		for(int j=0;j<mask->width;j++)
		{
			int idx = *lptr++;
			if(idx)
			{
				if(blob[idx].num_pixels > 30)
					mptr[j] = 255;
			}
		}
	}
/*
	sprintf(fName,"Maskaft_%d.pgm",labelindx);
	cvSaveImage(fName,mask );
*/
	return(1);
}

#if 1
void EyeSegmentServer::GeneratePolarWarpTable(CvPoint3D32f iris, CvPoint3D32f pupil)
{
	float radiusSampling = 1.0f/(m_flatIrisMask->height);

	int h = m_dxTable->height;
	int w = m_dxTable->width;

	float strad   = 0.0f;

	for(int i=0; i< h; i++)
	{
		float rad = strad + i * radiusSampling;
		float *dxptr = (float *) (m_dxTable->imageData + i * m_dxTable->widthStep);
		float *dyptr = (float *) (m_dyTable->imageData + i * m_dyTable->widthStep);

		float offx = rad * iris.x + (1.0f - rad)*(pupil.x);
		float offy = rad * iris.y + (1.0f - rad)*(pupil.y);
		float iprad = rad * iris.z + (1 - rad)*pupil.z;

		for(int j=0; j< w; j++)
		{
			*dxptr++ = offx + iprad * m_EyeLidCosTable[j];
			*dyptr++ = offy + iprad * m_EyeLidSinTable[j];
		}
	}
}


void EyeSegmentServer::miplRemap(IplImage *mask, IplImage *flatMask, IplImage *dxTable, IplImage *dyTable)
{
    unsigned char *mptr = (unsigned char *) mask->imageData;
    int ws = mask->widthStep;

    for(int i=0;i<flatMask->height;i++)
    {
        unsigned char *fptr = (unsigned char *) (flatMask->imageData + i*flatMask->widthStep);
        float *dxptr = (float *) (dxTable->imageData + i*dxTable->widthStep);
        float *dyptr = (float *) (dyTable->imageData + i*dyTable->widthStep);

        for(int j=0;j<flatMask->width;j++)
        {
            int ix = cvRound(*dxptr++);
            int iy = cvRound(*dyptr++);

            *fptr++ = mptr[iy*ws+ix];
        }
    }
}

void EyeSegmentServer::MaskSpecularitiesFlat(CvPoint3D32f iris, CvPoint3D32f pupil)
{
	cvResize(m_maskPyr[1], m_maskPyr[0], CV_INTER_NN);
/*
	char fName[100];
	sprintf(fName,"Pyr0_%d.pgm",labelindx);
	cvSaveImage(fName,m_maskPyr[0] );
*/
	cvErode(m_maskPyr[0], m_maskPyr[0]);
/*
	sprintf(fName,"Pyr0e_%d.pgm",labelindx);
	cvSaveImage(fName,m_maskPyr[0] );
*/
//	cvRemap(m_maskPyr[0], m_flatIrisMask, m_dxTable, m_dyTable, CV_INTER_NN | CV_WARP_FILL_OUTLIERS);
	miplRemap(m_maskPyr[0], m_flatIrisMask, m_dxTable, m_dyTable);
/*
	sprintf(fName,"flatIrisMask_%d.pgm",labelindx);
	cvSaveImage(fName,m_flatIrisMask );
*/
}
#endif

#if 1
bool EyeSegmentServer::ComputeEyelidIntegralContribution(IplImage *gradImage, IplImage *out, IplImage *count, CvPoint2D32f ptf1, CvPoint2D32f ptf2, CvSize searchArea)
{
	double dx = ptf2.x - floor(ptf2.x);
	double dy = ptf2.y - floor(ptf2.y);
	unsigned short C00 = min(65535, cvRound((1.0-dx) * (1.0-dy) * 65536));
	unsigned short C01 = min(65535, cvRound(dx * (1.0-dy) * 65536));
	unsigned short C10 = min(65535, cvRound((1.0-dx) * dy * 65536));
	unsigned short C11 = min(65535, cvRound(dx * dy * 65536));

	int sx = cvFloor(ptf2.x);
	int sy = cvFloor(ptf2.y);

	int ex = searchArea.width;
	int ey = sy + searchArea.height;

	int ooff = (cvFloor(ptf2.y) - cvFloor(ptf1.y))*out->widthStep + sizeof(int)*(cvFloor(ptf2.x) - cvFloor(ptf1.x));
	int coff = (cvFloor(ptf2.y) - cvFloor(ptf1.y))*count->widthStep + (cvFloor(ptf2.x) - cvFloor(ptf1.x));

	for(int i=sy;i<ey;i++)
	{
		unsigned short *gptr = (unsigned short *) (gradImage->imageData + i*gradImage->widthStep) + sx;
		unsigned short *gwptr = (unsigned short *) (gradImage->imageData + (i+1)*gradImage->widthStep) + sx;
		unsigned int *optr = (unsigned int *) (out->imageData + (i-sy)*out->widthStep + ooff);
		unsigned char *cptr = (unsigned char *) (count->imageData + (i-sy)*count->widthStep + coff);

		for(int j=0;j<ex;j++, gptr++, gwptr++, optr++, cptr++)
		{
			*optr += (C00 * gptr[0] + C01 * gptr[1] + C10 * gwptr[0] + C11 * gwptr[1] + 128)>>8;
			*cptr += 1;
		}
	}
	return true;
}
#endif

float GetIntegralEyeLidShortVersion(IplImage *costField, CvPoint2D32f point, float rad, float min_angle, float max_angle, float angleSample, int numSamples, float scaling, int level)
{
	float Xhead = 480*scaling/(1<<level);
	float YHead = 64*scaling/(1<<level);

	CvPoint3D32f *ptlist = (CvPoint3D32f *) malloc(numSamples*sizeof(CvPoint3D32f));

	if(ptlist == 0) printf("Memory error\n");

	int index = 0;
	for(float jj = min_angle; jj <= max_angle; jj+= angleSample)
	{
		CvPoint3D32f pt;

		pt.x = point.x + rad*cos(jj);
		pt.z = point.x - rad*cos(jj);
		pt.y = point.y + rad*sin(jj);

		if(pt.y >= 0 && pt.y <= YHead - 1)
			ptlist[index++] = pt;
	}

	unsigned short *cptr = 0;
	int cstep;

	cvGetRawData(costField, (unsigned char **) &cptr, &cstep);

	cstep /= sizeof(short);
	float acc = 0;
	int items = 0;

	//for(int i=0;i<numSamples;i++)
	for(int i=0;i<index;i++)
	{
		float cost, dx;
		unsigned short *ptr;
		int x;
		CvPoint3D32f pt = ptlist[i];

		int y = cvFloor(pt.y);
		float dy = pt.y - y;

		if(pt.x <= Xhead - 1)
		{
			x = cvFloor(pt.x);
			dx = pt.x - x;
			ptr = cptr + y*cstep + x;

			cost = (1.0f-dx)*(ptr[0]*(1.0f-dy) + ptr[cstep]*dy) + dx*(ptr[1]*(1.0f-dy) + ptr[1+cstep]*dy);

			acc += cost;
			items++;
		}

		if(pt.z >= 0)
		{
			x = cvFloor(pt.z);
			dx = pt.z - x;
			ptr = cptr + y*cstep + x;

			cost = (1.0f-dx)*(ptr[0]*(1.0f-dy) + ptr[cstep]*dy) + dx*(ptr[1]*(1.0f-dy) + ptr[1+cstep]*dy);

			acc += cost;
			items++;
		}

	}
	free(ptlist);

	return (items > 0)? acc/(items * 256) : 0;
}

void get_ranges(CvPoint2D32f *pt, CvSize *searchArea, int w, int h, int minTop, int maxTop)
{
	double min_x_allowed = max((double) pt->x, 0.0);
	double max_x_allowed = min( (double) pt->x + searchArea->width - 1.0, w - 1.0);

	double min_y_allowed = max((double) minTop, (double) pt->y);
	double max_y_allowed = min((double) maxTop, (double) pt->y + searchArea->height - 1.0);

	pt->x = (float) min_x_allowed;
	pt->y = (float) min_y_allowed;

	searchArea->width = cvFloor(max_x_allowed - min_x_allowed + 1.0);
	searchArea->height = cvFloor(max_y_allowed - min_y_allowed + 1.0);
}

void special_divide(IplImage *cost, IplImage *count, IplImage *costFloat, float *div_table)
{
	for(int i=0;i<cost->height;i++)
	{
		unsigned int *optr = (unsigned int *) (cost->imageData + i*cost->widthStep);
		float *ofptr = (float *) (costFloat->imageData + i*costFloat->widthStep);
		unsigned char *cptr = (unsigned char *) (count->imageData + i*count->widthStep);

		for(int j=0;j<cost->width;j++)
			*ofptr++ = (*optr++)*div_table[*cptr++];
	}
}







static int compare_float(const void * a, const void * b)
{
  return ( *(float*)b - *(float*)a );
}
#if 1
void PrintGrad(IplImage * gradMatrix,float scale)
{
	int h,w,ws;

	h = gradMatrix->height;
	w = gradMatrix->width;
	ws = gradMatrix->widthStep>>1;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	printf("\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		unsigned short *ptr = (unsigned short *)gradMatrix->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			unsigned short t = *(ptr+colctr);
			printf("%4.2f ",t/scale);
		}
		printf("\n");
	}
	printf("\n");
}

void PrintImg(IplImage * gradMatrix)
{
	int h,w,ws;

	h = gradMatrix->height;
	w = gradMatrix->width;
	ws = gradMatrix->widthStep;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	printf("\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		unsigned char *ptr = (unsigned char *)gradMatrix->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			unsigned char t = *(ptr+colctr);
			printf("%4d ,",t);
		}
		printf("\n");
	}
	printf("\n");
}

void PrintCost_float(IplImage * costMatrixFloat,int scale)
{
	int h,w,ws;

	h = costMatrixFloat->height;
	w = costMatrixFloat->width;
	ws = costMatrixFloat->widthStep>>2;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	printf("\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		float *ptr = (float *)costMatrixFloat->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			float t = *(ptr+colctr);
			printf("%5.2f ",t/scale);
		}
		printf("\n");
	}
	printf("\n");
}


IplImage *EyeSegmentServer::findEyelidCircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc, CvRect loc, float lradius, float uradius, float scaling, float sigma, float vert, float horz, float lambda, float min_angle, float max_angle, bool pupil, int level)
{

#ifdef PROFILE
	QueryPerformanceFrequency(&profiler_freq);
#endif

	PROFILE_START(ONE)
	int nw = cvFloor(image->width * scaling);
	int nh = cvFloor(image->height * scaling);
	static int imageIdx = 0;

	unsigned short *gradptr=0;
	unsigned char *rsptr=0, *isptr;
	int gradStep=0, imStep = 0;

	IplImage *gradImage = m_flatGradImage;	// only one is allocated

	cvGetRawData(image, &isptr, &imStep);
	cvGetRawData(gradImage, (unsigned char **) &gradptr, &gradStep);
	gradStep /= sizeof(short);

	unsigned char *maskPtr = 0;
	int maskStep = 0;
	if(mask)
	{
		cvGetRawData(mask, &maskPtr, &maskStep);
	}
	else
	{
		maskPtr = (unsigned char *) calloc(gradImage->width, sizeof(char));
		maskStep = 0;
	}

	cvSetZero(gradImage);
//	PrintImg(image);
	// computing the gradient and the LBP image
	short hzi = (short) cvRound(horz);
	short vzi = (short) cvRound(vert);

	for(int i=1;i<nh-1;i++)
	{
		unsigned char *iptr = isptr + i * imStep + 1;
		unsigned char *iptr_p1 = isptr + (i+1) * imStep + 1;
		unsigned char *iptr_m1 = isptr + (i-1) * imStep + 1;
		unsigned short *gptr = gradptr + i * gradStep + 1;
		unsigned char *mptr = maskPtr + i *maskStep + 1;

		for(int j=1;j<nw-1;j++, iptr++, iptr_p1++, iptr_m1++)
		{
			short v = iptr[1] - iptr[-1];
			short h = iptr_p1[0] -  iptr_m1[0];
			if(h < 0) h = 0;

			*gptr++ = (*mptr++)? 0:cvRound(cvSqrt((float) (h*h*hzi + v*v*vzi))*256);
		}
		gradptr[i * gradStep] = gradptr[i * gradStep + 1];
		gptr[0] = gptr[-1];
	}

	// replicating on the top
	memcpy(gradptr, gradptr+gradStep, gradImage->widthStep);
	// replicating on the bottom
	memcpy(gradptr + (nh-1)*gradStep, gradptr + (nh-2)*gradStep, gradImage->widthStep);
	memset(gradptr + nh*gradStep, 0, gradImage->widthStep);

//	PrintGrad(gradImage,256.0);

//	IppiSize gradRoi; gradRoi.width = nw, gradRoi.height = nh;

//	IppStatus status = ippiSqrt_16u_C1IRSfs(gradptr, gradImage->widthStep, gradRoi, -8);

//	ippiThreshold_LTVal_16u_C1IR(gradptr, gradImage->widthStep, gradRoi, 10*(1<<8), 0);

	if(!mask) free(maskPtr);

	PROFILE_END(ONE)

	PROFILE_START(TWO)
	float sx = loc.x * scaling, sy = loc.y * scaling;
	float ex = (loc.x + loc.width)*scaling;
	float ey = (loc.y + loc.height)*scaling;

	int minTop = 0;
	int maxTop = (image->height - 1);

	float angleSampling = (float) (0.5f*CV_PI/180.0f);

	float bestrad = 0;
	CvPoint2D32f bestpt = {0, 0};
	float maxCost = 0;
	float max_grad = 0;
	float maxHistogramCost = 0;
	int m_index = 0;

	double bias = m_eyelidSearchSampling;

	int idx = 0;

	CvSize searchArea = cvSize((int) ceil(ex-sx), (int) ceil(ey - sy));

	int numMatrices = (int) ceil((uradius - lradius)/bias + 1);
	IplImage **costMatrixFloat = m_costMatrixFloat;
	IplImage **costMatrixFloatMax = m_costMatrixFloatMax;

	IplImage costMatrixHeader, countMatrixHeader;
	cvInitImageHeader(&costMatrixHeader, searchArea, IPL_DEPTH_32S, 1);
	cvInitImageHeader(&countMatrixHeader, searchArea, IPL_DEPTH_8U, 1);

	cvSetData(&costMatrixHeader, m_costMatrix->imageData, (searchArea.width << 2));
	cvSetData(&countMatrixHeader, m_countMatrix->imageData, ((searchArea.width+3)>> 2)<<2);

	IplImage *costMatrix = &costMatrixHeader;
	IplImage *countMatrix = &countMatrixHeader;

	// IMPLICIT ASSUMPTION HERE THAT NUMBER OF POINTS AROUND THE CIRCLE IS NEVER MORE THAN 255

	float div_table[256];

	memset(m_fullCostData, 0, (searchArea.height+2)*(searchArea.width+2)*3*sizeof(int));

	div_table[0] = 0;
	for(int i=1;i<256;i++)
		div_table[i] = (float) (1.0/(i*65536));

	m_pScore[0] = 0;

	int peakCountTotal = 0;

	CvSize sz; sz.width = searchArea.width; sz.height = searchArea.height;


	int offset = sizeof(int) * (sz.width + 2 + 1);

	CvPoint3D32f off = cvPoint3D32f(sx, sy, lradius-bias);

	for(double rad = lradius; rad <=uradius; rad+=bias, idx++, off.z += bias)
	{
		double angleSample = 0.5/rad;

		costMatrixFloat[idx] = cvInitImageHeader(costMatrixFloat[idx], searchArea, IPL_DEPTH_32F, 1);
		costMatrixFloatMax[idx] = cvInitImageHeader(costMatrixFloatMax[idx], searchArea, IPL_DEPTH_32F, 1);

		cvSetData(costMatrixFloat[idx], m_fullCostData + (idx%3)*(sz.height+2)*(sz.width+2), (sz.width + 2)* sizeof(float));
		cvSetData(costMatrixFloatMax[idx], m_fullCostDataMax + (idx%3)*(sz.height+2)*(sz.width+2), (sz.width + 2)* sizeof(float));

		costMatrixFloat[idx]->imageData += offset;
		costMatrixFloatMax[idx]->imageData += offset;

		int nang = 0;
		cvSetZero(costMatrix);
		cvSetZero(countMatrix);
		for(double ang = min_angle; ang <= max_angle; ang += angleSample, nang++)
		{
			double cs = cos(ang);
			double sn = sin(ang);
			CvPoint2D32f pt1, pt2;
			CvSize newSearchArea = searchArea;

			pt2.x = pt1.x = sx + cs*rad;
			pt2.y = pt1.y = sy + sn*rad;

			get_ranges(&pt2, &newSearchArea, nw, nh, minTop, maxTop);

			ComputeEyelidIntegralContribution(gradImage, costMatrix, countMatrix, pt1, pt2, newSearchArea);

			pt2.x = pt1.x = sx - cs*rad;
			pt2.y = pt1.y = sy + sn*rad;
			newSearchArea = searchArea;

			get_ranges(&pt2, &newSearchArea, nw, nh, minTop, maxTop);

			ComputeEyelidIntegralContribution(gradImage, costMatrix, countMatrix, pt1, pt2, newSearchArea);
		}

		special_divide(costMatrix, countMatrix, costMatrixFloat[idx], div_table);

//		PrintCost_float(costMatrixFloat[idx],2);

		cvDilate(costMatrixFloat[idx], costMatrixFloatMax[idx]);


		int peakCount = 0;

		if(idx==1)
		{
//			PrintCost_float(costMatrixFloatMax[idx],2);
//			PrintCost_float(costMatrixFloatMax[idx-1],2);

			cvMax(costMatrixFloatMax[idx], costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-1]);

			find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-1], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);

		}
		else if(idx > 1)
		{
			cvMax(costMatrixFloatMax[idx], costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-1]);

			cvMax(costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-2], costMatrixFloatMax[idx-2]);

			find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-2], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);
		}

		peakCountTotal += peakCount;
	}

	int bidx = 0, lidx = 0;
	float minScore = 0;

	{
		int peakCount = 0;
		find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-2], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);
		peakCountTotal += peakCount;
#ifdef SEG
	    printf("\npeakCountTotal = %3d \n",peakCountTotal);
#endif

	#ifdef PRINT_DEBUG
			printf(" x    y   score \n");
			for(int i = 0;i<peakCountTotal;i++)
			{
				printf("%4.2f  ",m_pPeak[i].x);
				printf("%4.2f  ",m_pPeak[i].y);
				printf("%4.2f ",m_pPeak[i].z);
				printf("%4.2f \n",m_pScore[i]);
			}
	#endif

		memcpy(m_pScore+peakCountTotal, m_pScore, sizeof(float)*peakCountTotal);

		qsort(m_pScore+peakCountTotal, peakCountTotal, sizeof(float), compare_float);

		for(int i=0;i<peakCountTotal; i++)
		{
			if(m_pScore[i] > m_pScore[bidx])
				bidx = i;
			if(m_pScore[i] < m_pScore[lidx])
				lidx = i;
		}
		if(peakCountTotal > 15) minScore = m_pScore[peakCountTotal+15];
	}

	PROFILE_END(TWO)

	PROFILE_START(THREE)

	std::map<float, CvPoint3D32f>	circleList;

	// lower eyelid occlusion constraint
	float lower = (image->height >> 1)*(1 << level);
	// upper eyelid occlusion constraint
	float upper = 4;
	float maxScore = 0;

	if(level==f_levels)
	{
		int processedPeaks = 0;
		for(int i=0;i<peakCountTotal;i++)
		{

//			printf("%f %f %f %f\n", m_pPeak[i].x, m_pPeak[i].y, m_pPeak[i].z, m_pScore[i]);

			if(m_pScore[i] < 10 || m_pScore[i] < minScore)
				continue;

			CvPoint3D32f pt = m_pPeak[i];
			float TotalCost =  m_pScore[i];

			CvPoint ptt = { cvRound(pt.x*(1<<level)), cvRound(pt.y*(1<<level))};

			float radius  = pt.z*(1<<level);

			// occlusion too small
			// will get covered by default anyway

			if(ptt.y - radius > (image->height << level) - 10)
				continue;

			// if upper eyelid && eyelid occlusion is above than a certain height then throw away that point
			if(pupil && (ptt.y - radius < upper))
				continue;

			if(!pupil && (ptt.y - radius < lower))
				continue;

			float DistEyelidIntersectLeft = (float)(ptt.x - sqrt((radius)*(radius)-(ptt.y-image->height*(1<<level))*(ptt.y-image->height*(1<<level))));
			float DistEyelidIntersectRight =(float)(ptt.x + sqrt((radius)*(radius)-(ptt.y-image->height*(1<<level))*(ptt.y-image->height*(1<<level))));

			if(!pupil&&(DistEyelidIntersectLeft<=40||DistEyelidIntersectRight>=225)) // pupil : true is right/upper Eyelid pupil : false is left/lower Eyelid
				continue;
			else if(pupil&&(DistEyelidIntersectLeft<=230||DistEyelidIntersectRight>=485))
				continue;

			// need verification that the detect circle has gradient all around, not just near the top
			// this implies large occlusion - lets do additional verification

			if(pupil && (ptt.y - radius < lower))
			{
				float angleSampleTable = 0.5f/pt.z;
				float minLidAngle = m_minLidAngle + 0.25*(m_maxLidAngle - m_minLidAngle);

				int numSampleTable = cvFloor((m_maxLidAngle - minLidAngle)/angleSampleTable) + 2; // MAGIC NUMBER;

				CvPoint2D32f pt2 = cvPoint2D32f(pt.x, pt.y);

				float cost = GetIntegralEyeLidShortVersion(gradImage, pt2, pt.z, minLidAngle, m_maxLidAngle, angleSampleTable ,numSampleTable,scaling, level);

				if( cost < 0.75*m_pScore[i])
					continue;
//				fprintf(fp, "%3.3f ", cost);
			}
			processedPeaks++;


			circleList[m_pScore[i]] = cvPoint3D32f(ptt.x, ptt.y, radius);

			if(m_pScore[i] > maxScore)
				maxScore = m_pScore[i];
		}
	}

	if(circleList.empty())
		circleList[100] = ( pupil? cvPoint3D32f(360, 190, 140) : cvPoint3D32f(130, 190, 135) );

	IplImage *CircleImage = (pupil)? m_upperEyelidMask:m_lowerEyelidMask;

	cvSetZero(CircleImage);
	CvScalar color = cvRealScalar(255);

	std::map<float, CvPoint3D32f>::iterator cit = circleList.begin();
	maxScore *= 0.5;

#ifdef SEG
	printf("Max %6.3f  \n",maxScore);
#endif
	for(;cit != circleList.end(); cit++)
	{
		CvPoint pt = cvPoint(cvRound((cit->second).x), cvRound(cit->second.y));
#ifdef SEG
		printf("EYELID POINT\n");
		printf("%3d  ",pt.x);
		printf("%3d  ",pt.y);
		printf("%3d ",cvRound(cit->second.z));
		printf("%6.3f \n",cit->first);
#endif
		if(cit->first > maxScore)
		{
			cvCircle(CircleImage, pt, cvRound(cit->second.z), color, CV_FILLED);
		}
	}
	PROFILE_END(THREE)
#ifdef SEG
	printf("Number of circles  = %3d\n",(int)circleList.size());
#endif
	return(CircleImage);
}
#endif


IplImage *EyeSegmentServer::FindUpperEyelidBoundaries(IplImage *image, IplImage *mask)
{
	//f_levels = 3;
#ifdef PROFILE
	QueryPerformanceFrequency(&profiler_freq);
#endif

	//PROFILE_START(ONE)

	CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};

	//PROFILE_END(ONE)

	//PROFILE_START(TWO)
	float startRadius = m_startEyelidRadiusRight;
	float endRadius = m_endEyelidRadiusRight;

	CvRect cloc = m_LidRightRect;

	float scaling = 1.0f;
	float sigma = 0.5f;

	int i = f_levels;

	startRadius /= (1 << i);
	endRadius /= (1 << i);

	cloc.x = (cloc.x >> i);
	cloc.y = (cloc.y >> i);

	cloc.width = (cloc.width >> i);
	cloc.height = (cloc.height >> i);
	//PROFILE_END(TWO)

	IplImage *eyeLidMask = m_upperEyelidMask;


		if(m_enableEyelidSegmentation)
		{
			
			eyeLidMask = findEyelidCircle(image, mask, nullPt, cloc, startRadius, endRadius, scaling, sigma, 0.0f, 1.0f, 3.0f, m_minLidAngle, m_maxLidAngle, true, i);
		
		}
		else
		{
			// tiny default Upper EyeLid Mask

	//		createDefaultEyelid(eyeLidMask, m_defaultUpperEyelidAngle, m_Iris, m_Pupil);
			cvSetZero(eyeLidMask);
//			CvPoint depPt = {360,190};
//			float defRad = 140.0f;
			CvScalar color = cvRealScalar(255);
			//printf("Upper %d %d %f",m_centerptUpperEyelid.x,m_centerptUpperEyelid.y,m_radiusUpperEyelid);
			cvCircle(eyeLidMask, m_centerptUpperEyelid, cvRound(m_radiusUpperEyelid), color, CV_FILLED);
		}

		//return findEyelidCircle(m_imgFlatPyr[i], m_maskFlatPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 3.0f, m_minLidAngle, m_maxLidAngle, true, i);
	return eyeLidMask;
}
IplImage *EyeSegmentServer::FindLowerEyelidBoundaries(IplImage *image, IplImage *mask)
{
#ifdef FINDIRIS
	FILE *fp = fopen("Eyelid.txt", "at");
#endif
	//f_levels = 3;
#ifdef PROFILE
	QueryPerformanceFrequency(&profiler_freq);
#endif

	CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};

	float startRadius = m_startEyelidRadiusLeft;

	float endRadius = m_endEyelidRadiusLeft;

	CvRect cloc = m_LidLeftRect;

	float scaling = 1.0f;
	float sigma = 0.5f;

	int i = f_levels;
#ifdef FINDIRIS
fprintf(fp," sR=%f, eR=%f i=%d\n",startRadius,endRadius,i);
fprintf(fp," h=%d, h=%d w=%d, h=%d\n",image->width,image->height,mask->width,mask->height);
fclose(fp);
#endif
	startRadius /= (1 << i);
	endRadius /= (1 << i);

	cloc.x = (cloc.x >> i);
	cloc.y = (cloc.y >> i);

	cloc.width = (cloc.width >> i);
	cloc.height = (cloc.height >> i);

	IplImage *eyeLidMask = m_lowerEyelidMask;
	PROFILE_START(LOWEREYELID)

	if(m_enableEyelidSegmentation)
	{
		
		eyeLidMask = findEyelidCircle(image, mask, nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 3.0f, m_minLidAngle, m_maxLidAngle, false, i);
		
	}
	else
	{
//			createDefaultEyelid(eyeLidMask, m_defaultLowerEyelidAngle, m_Iris, m_Pupil);
			cvSetZero(eyeLidMask);
//			CvPoint depPt = {130,190};
//			float defRad = 135.0f;
			CvScalar color = cvRealScalar(255);
//			printf("Lower %d %d %f",m_centerptLowerEyelid.x,m_centerptLowerEyelid.y,m_radiusLowerEyelid);
			cvCircle(eyeLidMask, m_centerptLowerEyelid, cvRound(m_radiusLowerEyelid), color, CV_FILLED);
	}
	PROFILE_END(LOWEREYELID)

	return eyeLidMask;
	//return findEyelidCircle(m_imgFlatPyr[i], m_maskFlatPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 3.0f, m_minLidAngle, m_maxLidAngle, false, i);

}



void PrintImg2File(char * out,IplImage * gradMatrix)
{
	FILE * fp;
	fp = fopen(out,"w");
	int h,w,ws;

	h = gradMatrix->height;
	w = gradMatrix->width;
	ws = gradMatrix->widthStep;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	fprintf(fp,"\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		unsigned char *ptr = (unsigned char *)gradMatrix->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			unsigned char t = *(ptr+colctr);
			fprintf(fp,"%4d ",t);
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
	fclose(fp);
}
#if 1
IplImage *EyeSegmentServer::FlattenIris(IplImage *img, CvPoint3D32f iris, CvPoint3D32f pupil)
{
	cvRemap(img, m_flatIris, m_dxTable, m_dyTable);
//	PrintImg2File("input.txt",img);
//	PrintImg2File("warp.txt",m_flatIris);

	return m_flatIris;
}
#endif


