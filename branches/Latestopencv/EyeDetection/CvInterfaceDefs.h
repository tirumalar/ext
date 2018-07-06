#pragma once
#include <cv.h>
typedef CvSize IppiSize;
typedef CvPoint IppiPoint;
typedef CvRect IppiRect;

typedef unsigned char   Ipp8u;
typedef unsigned short  Ipp16u;
typedef unsigned int    Ipp32u;

typedef signed char    Ipp8s;
typedef signed short   Ipp16s;
typedef signed int     Ipp32s;
typedef float		   Ipp32f;
typedef double		   Ipp64f;

typedef struct {
	double mx, my;
	double i1, i2;
	double angle;
	double mean, var;
	double lambda1, lambda2;;
} HU_MOMENTS;
#if 0
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif 
