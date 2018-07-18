#ifndef _DISTANCE_METRICS_H_
#define _DISTANCE_METRICS_H_

#include <cv.h>

float emdDistance(float *iptr, float *optr, int sz, CvMat *cost=0, CvMat *flow=0);
float kullBackDistance(float *iptr, float *optr, int sz);
float findBestEMDDistance(float **piptr, float **poptr, int sz, CvPoint *bestOffset, CvMat *cost=0);

#endif