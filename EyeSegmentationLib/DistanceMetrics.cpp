#include "DistanceMetrics.h"

#include <stdio.h>

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

float emdDistance(float *iptr, float *optr, int sz, CvMat *cost, CvMat *flow)
{
	CvMat *m1 = cvCreateMat(sz, 2, CV_32FC1);
	CvMat *m2 = cvCreateMat(sz, 2, CV_32FC1);
	float *m1ptr = 0, *m2ptr = 0;
	int m1step = 0, m2step = 0;

	cvGetRawData(m1, (unsigned char **) &m1ptr, &m1step);
	cvGetRawData(m2, (unsigned char **) &m2ptr, &m2step);
	m1step /= sizeof(float);
	m2step /= sizeof(float);

	for(int i=0;i<sz;i++)
	{
		m1ptr[i*m1step] = iptr[i];
		m2ptr[i*m2step] = optr[i];
		m1ptr[i*m1step+1] = m2ptr[i*m2step+1] = i;
	}
	
	float dist = cvCalcEMD2(m1, m2, (cost)? CV_DIST_USER:CV_DIST_L1, NULL, cost, flow);

	cvReleaseMat(&m1);
	cvReleaseMat(&m2);

	return dist;
}

float findBestEMDDistance(float **piptr, float **poptr, int sz, CvPoint *bestOffset, CvMat *cost)
{
	#ifdef PROFILE
		QueryPerformanceFrequency(&profiler_freq);
	#endif

	PROFILE_START(EMD)
	float minDist = 100.0f;
	int best_i = 0, best_j = 0;

	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
			float dist = emdDistance(piptr[i], poptr[j], sz, cost);
			if(dist < minDist)
			{
				best_i = i;
				minDist = dist;
				best_j = j;
			}
		}
	}
	
	if(bestOffset)
		*bestOffset = cvPoint(best_i, best_j);

	PROFILE_END(EMD)
	return minDist;
}
float kullBackDistance(float *iptr, float *optr, int sz)
{
	float dist = 0;
	float uval = 1.0f/(100.0f*sz);  /// eliminating log(0) cases.
	
	for(int i=0;i<sz;i++)
		dist += (iptr[i]+uval)*log((iptr[i]+uval)/(optr[i]+uval));

	return dist;
}