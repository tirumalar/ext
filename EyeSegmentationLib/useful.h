#ifndef _USEFUL_MACROS_H
#define _USEFUL_MACROS_H


//#define SHOW_INPUT 1
//#define SHOW_IRIS_MAXIMA 1
//#define SHOW_PUPIL_MAXIMA 1
//#define SHOW_BEST_IRIS_PUPIL 1

//#define HOME_PATH "C:/Iris_outputs/Casia_fail/"

//#define TIME_PROFILE

#if 0
static __inline int QFTOL(double x)
{
	x += 68719476736.0*1.5; // Intel register magic
	return ((int*) &x)[0] >> 16;
}
#else
#define QFTOL(x) (cvFloor(x))
#endif
 

#if 0

static __inline int QFTOLRND(float x)
{
int tmp;
__asm {
	fld x
	fistp tmp
}
return tmp;
}

#else
#define QFTOLRND(x) cvRound(x)
#endif


void draw( IplImage* img1, CvPoint3D32f pt, CvScalar color );

#endif
