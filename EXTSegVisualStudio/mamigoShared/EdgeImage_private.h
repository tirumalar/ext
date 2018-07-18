/*
 * EdgeImage_private.h
 *
 *  Created on: 27 Apr, 2009
 *      Author: madhav
 */

#ifndef EDGEIMAGE_PRIVATE_H_
#define EDGEIMAGE_PRIVATE_H_
#include "image.h"

#define FIX_Q(a,b) ((int)((a)*(1<<b)))

typedef struct
{
	short x,y;
}Point2D16i;


typedef struct
{
	int x,y,z;
}Point3D32i;

typedef struct
{
	int x,y;
}Point2D32i;

typedef short fract16;

typedef struct{
	fract16 re;
	fract16 im;
}complex_fract16;

extern void Convert16to8_ASM(unsigned short *pSrc, unsigned char *pDest, int count, unsigned short dc, const int shift);
extern unsigned int Match_ASM(unsigned char *f1,unsigned char *m1,unsigned char *f2,unsigned char *m2,int cnt);
extern void ScaleCodeLow(unsigned char *inp,unsigned char *out,int cnt);
extern void ScaleCodeHigh(unsigned char *inp,unsigned char *out,int cnt);
extern void ExtractEvenBytes(unsigned char *onebuff,unsigned char *minus4,unsigned char *minus2,int outputwidth);
extern void GetExtractedLineASM(unsigned short *onebuff,unsigned short *minus4,unsigned short *minus2,int outputwidth);
extern void reduce_gauss5_16bit_4x_To_8bit(unsigned short* inp,unsigned short* out, int *param);

extern void compute_LUT_sqrt(unsigned short *SQRT_LUT_0_64, unsigned short *SQRT_LUT_0_256);
extern void compute_LUT_sqrt_eyelid( unsigned short *SQRT_LUT_0_256);
extern void compute_LUT_sqrtminmax_C(unsigned short *SQRT_LUT_0_64, unsigned short *SQRT_LUT_0_256,int gradientpupilmin,int gradientpupilmax,int gradientcirclemin,int gradientcirclemax);
//extern void compute_correlate_gradient_image(usImage *ptrin, uiImage *ptrout,Pointparam *pt);
extern void compute_horizontal_gradient_image(unsigned char *img,short int *output, int*dstParam);
extern void compute_vertical_gradient_image(unsigned char *img,short int *output, int*dstParam);
extern void compute_magnitude_gradient_image(unsigned char *img,unsigned short int *output,  int*dstParam);

void  compute_magnitude_gradient_image_new(unsigned char *,unsigned short *,int *);
extern void dilate_i(unsigned int *ptr,unsigned int *output,unsigned int *params);
extern void dilate_i_c(unsigned int *ptr,unsigned int *output,unsigned int *params);
extern short pdilate_i_c(int **inputs, Point2D16i *output, int *params);
extern short pdilate_i_3(int **inputs, Point2D16i *output, int *params);
extern short pdilate_i_2(int **inputs, Point2D16i *output, int *params);
void FindMax(unsigned int *ptr,int* params);
short detect_single_specularity(unsigned char*, Point2D16i*, int*);
//void compute_polar_warping(ucImage *img,ucImage *output,unsigned int *buff);
//void compute_polar_warping_NN(ucImage *img,ucImage *output,unsigned int *buff);

void compute_erode_image(unsigned char * in,unsigned char *out,int *param);
void zoom_2x(unsigned char * in,unsigned char *out,int *param);

void compute_horizontal_gradient_image_mask_64(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_vertical_gradient_image_mask_64(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_magnitude_gradient_image_mask_64(unsigned char *img,unsigned short int *output,  int* dstParam);

void compute_horizontal_gradient_image_mask(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_vertical_gradient_image_mask(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_magnitude_gradient_image_mask(unsigned char *img,unsigned short int *output,  int* dstParam);

void compute_horizontal_gradient_image_mask_eyelid(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_vertical_gradient_image_mask_eyelid(unsigned char *img,unsigned short int *output,  int* dstParam);
void compute_magnitude_gradient_image_mask_eyelid(unsigned char *img,unsigned short int *output,  int* dstParam);

//extern void compute_correlate_gradient_image_eyelid_C(usImage *ptrin, uiImage *ptrout,Pointparam *pt);
//extern void compute_correlate_gradient_image_eyelid(usImage *ptrin, uiImage *ptrout,Pointparam *pt);
extern void special_divide_ASM(int * param);
int compute_EigenVals(unsigned char *inptr,int *outptr,int*param);
int compute_EigenValsHist1(unsigned char *inp,int *param);
void compute_EigenMask(int* eig,unsigned char *mask,int *Param);
void compute_MatchFeatures(unsigned char **ptr, int **ptr1,int *param);
void rfft2d_fr16 (const fract16 in[],
					complex_fract16 t[],
					complex_fract16 out[],
                   const complex_fract16 w[],
                   int wst,
                   int n,
                   int b,
                   int s );
fract16 cos_fr16(fract16 val,unsigned short *tab);
fract16 sin_fr16(fract16 val,unsigned short *tab);
int GetCoarseIrisCodeAsm(unsigned char *iris, unsigned char *coarse,int len);
int GetCoarseIrisCodeFromCompactDBAsm(unsigned char *iris, unsigned char *coarse,int len);
void reduce_gauss5_4x(unsigned char* inp,unsigned char* out, int *param);
void reduce_gauss5_2x(unsigned char* inp,unsigned char* out, int *param);
void reduce_gauss5_16bit_4x(unsigned short* inp,unsigned short* out, int *param);


typedef struct{
	unsigned char* output;
	short width;
	short height;
}dims;

void gaus5_expand_reflect(unsigned char *image, unsigned char* scratch, dims* pDims);


#endif /* EDGEIMAGE_PRIVATE_H_ */
