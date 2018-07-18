/*
 * GausPyr_private.h
 *
 *  Created on: 26 Aug, 2008
 *      Author: akhil
 */

#ifndef GAUSPYR_PRIVATE_H_
#define GAUSPYR_PRIVATE_H_

#include "twoshorts.h"
//common
typedef struct{
	unsigned char* output;
	short width;
	short height;
}dims;

unsigned char **allocPyr(short w, short h, int fine_lev, int coarse_lev);
void pyr_free(unsigned char **pyr, int levels);
dims **allocDims(short w, short h,int coarse_lev);
dims **allocDims_assignOut(short w, short h,int coarse_lev, unsigned char **out);
void dims_free(dims **d,int coarse_lev);

//gausPyr
extern void gaus5_reduce_reflect(unsigned char *image, unsigned char* scratch, dims* pDims);
extern void gaus5_reduce_width_reflect(unsigned char *image, unsigned char* output, twoshorts size);
extern void gaus5_reduce_width_sp_reflect(unsigned char *image, unsigned char* output, twoshorts size, int skip);
extern void gaus5_expand_reflect(unsigned char *image, unsigned char* scratch, dims* pDims);
extern void sub_clip(unsigned char *image, unsigned char *refimage,dims* pDims, twoshorts cliplimits);
extern void sub_clip_add_shift(unsigned char *image, unsigned char *refimage,dims* pDims, twoshorts adder_shifter);

// following two methods work on 2 interlaced streams of shorts (dx, dy)
extern void gaus5_reduce_reflect_2s(int *image, int *out, int* args);
extern void gaus5_expand_reflect_2s(int *image, int *out, int* args);

//following does a 4x reduction
extern void gaus5_reduce_4x_b(int *image, int *out, int* args);
extern void compute_1296x1944To648x486(unsigned char *image, unsigned char *refimage, int* args);
extern void compute_2592x972To648x486(unsigned char *image, unsigned char *refimage, int* args);

//NormalFlowHist

typedef struct{
	unsigned char* scr;
	unsigned int* hist;
	int thresh;
	twoshorts nT_grAdd;
	dims** d;
	unsigned char** out_pyr;
	short intensity;
	short histSize;
	int sum;
	int histSum;
	twoshorts def_x_roi;
}normalFlowHist;

extern int updateDistribution(int histsize, unsigned int* hist, twoshorts ts);
extern void getNormalFlowHist(unsigned char* ref, unsigned char* ins, unsigned char* scr,
			unsigned int* hist,int thresh, twoshorts nT_grAdd,dims d, twoshorts x_roi, short lstep);

extern void min_image_group_2(unsigned char *, unsigned char **, int *);
extern char *min_image_roi_group_2(unsigned char *, unsigned char **, int *);
extern char *min_image_roi_left_group_2(unsigned char *, unsigned char **, int *);
extern void min_image_group_3(unsigned char *, unsigned char **, int *);
extern void min_image_group_4(unsigned char *, unsigned char **, int *);

//BPFilter

typedef struct{
	short flev;
	short blev;
	short tlev;
	short numbinbits;
	short shifter;
	dims **d;
	unsigned char **pyr;
	unsigned char *scratch;
}BPFilter;

void mipl_setZeros(unsigned int* a,int s);

#endif /* GAUSPYR_PRIVATE_H_ */
