/*
 * Bfin specific parts of GausPyr.cpp
 *
 *  Created on: 14 Oct, 2008
 *      Author: akhil
 */
#ifdef __BFIN__

#ifdef __cplusplus
extern "C" {
#endif
#include "GausPyr_private.h"
#include "compute_integral_private.h"
#include "warpers_private.h"
#include <bfin_sram.h>
#include <cv.h>
#ifdef __cplusplus
}
#endif


#include "GaussPyr.h"



//allocate scratch from L1 DATA SRAM, and we create a pseudo pyramid where each layer uses the once allocated
//space... hence we also need a special function to free it

IplImage **mipl_alloc_scratch_asPeudoPyr(int fine_lev, int coarse_lev, int w, int h)
{
	IplImage **imgpyr = (IplImage **) calloc((coarse_lev+1), sizeof(IplImage *));
	IplImage *scr = cvCreateImageHeader(cvSize(w,h),IPL_DEPTH_8U,1);
	scr->widthStep=scr->width;
	char * scratch=(char *)sram_alloc(w*h, L1_DATA_SRAM);
	if(0==scratch) throw "could not allocate scratch in L1 SRAM";
	cvSetData(scr,scratch,w);
	for(int lev = 0; lev <= coarse_lev; lev++)
	{
		if(lev >= fine_lev)
			imgpyr[lev] = scr;	// same scratch every where
	}
	return imgpyr;
}

void mipl_free_scratch_asPeudoPyr(IplImage **pyr, int file_lev, int coarse_lev)
{
	sram_free(pyr[0]->imageData);
	cvReleaseImageHeader(pyr);
	free(pyr);
}


apr_status_t mipl_reduce_gauss5_b(IplImage* in,IplImage* out,IplImage* scr)
{
	return mipl_reduce_gauss5_b(in,out,scr->imageData);
}

apr_status_t mipl_reduce_gauss5_b(IplImage* in,IplImage* out,char *scr)
{
	dims d;
	d.width = in->width;
	d.height = in->height;
	d.output = (unsigned char*)(out->imageData);
	gaus5_reduce_reflect(
			(unsigned char*)in->imageData,
			(unsigned char*)scr,&d);
	return APR_SUCCESS;
}


apr_status_t mipl_reduce_gauss5_width_b(IplImage* in,IplImage* out,IplImage* scr)
{
	twoshorts ts;
	ts.lo=in->width;
	ts.hi=in->height;
	gaus5_reduce_width_reflect(
			(unsigned char*)in->imageData,
			(unsigned char*)out->imageData,ts);
	return APR_SUCCESS;
}

apr_status_t mipl_reduce_gauss5_width_skip_b(IplImage* in,IplImage* out,int skip, IplImage* scr)
{
	twoshorts ts;
	ts.lo=in->width;
	ts.hi=in->height;
	gaus5_reduce_width_sp_reflect(
			(unsigned char*)in->imageData,
			(unsigned char*)out->imageData,ts,skip);
	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_b(IplImage* in, IplImage *in1, IplImage* scr)
{
	dims d;
	d.width = in1->width;
	d.height = in1->height;
	d.output = (unsigned char*)(in->imageData);
	gaus5_expand_reflect(
			(unsigned char*)in1->imageData,
			(unsigned char*)scr->imageData,&d);
	return APR_SUCCESS;
}

// out = in - in1
apr_status_t mipl_sub_bc(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr)
{
	dims d;
	twoshorts ts;
	d.width = (out->width+1)>>1;
	d.height = (out->height+1)>>1;
	d.output = (unsigned char*) out->imageData;
	ts.hi = 127;
	ts.lo = -128;
	sub_clip(
			(unsigned char*)in1->imageData,
			(unsigned char*)in->imageData, &d, ts);
	return APR_SUCCESS;
}

/*
 * A variant of  mipl_expand_gauss5_sub_bc which saves the intermediate result
 * out_inter=expand(in1)
 * out = in - out_inter
 */
apr_status_t mipl_expand_gauss5_sub_bc_save_intermediate(IplImage* in, IplImage *in1, IplImage* out,IplImage* out_inter,IplImage* scr)
{
	dims d;
	twoshorts ts;
	d.width = in1->width;
	d.height = in1->height;
	d.output = (unsigned char *) out_inter->imageData;
	ts.hi = 127;
	ts.lo = -128;

	gaus5_expand_reflect(
			(unsigned char *)in1->imageData,
			(unsigned char *)scr->imageData, &d);

	d.output = (unsigned char*) out->imageData;

	sub_clip(
			(unsigned char *)out_inter->imageData,
			(unsigned char *)in->imageData, &d, ts);
	return APR_SUCCESS;
}

// the following is slow it needs a scratch with width border and input with height border
// howeevr it can work over arbitrary image widths
// out = in - expand(in1)
apr_status_t mipl_expand_gauss5_sub_bc_slow(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr)
{
	/* Vertical Filter */
	set_vertical_border(in1,1,IPL_BORDER_REFLECT);

	for(int jj=0,j=0; jj < out->height ; j++, jj+=2 )
	{
		unsigned short* oxp  = (unsigned short *) (scr->imageData + jj*scr->widthStep);
		unsigned short* oxp1 = oxp + (scr->widthStep >> 1);
		unsigned char* ixp   = (unsigned char *) in1->imageData + j*in1->widthStep;
		unsigned char* ixp1_ = ixp - in1->widthStep;
		unsigned char* ixp1  = ixp + in1->widthStep;
		for(int i=0; i< in1->width ; i++, ixp++,ixp1++,ixp1_++ )
		{
			*oxp++  = *ixp * 6 + *ixp1 + *ixp1_;
			*oxp1++ = ( (*ixp + *ixp1)<< 2 );
		}
	}

	/* Horizontal Filter */
	set_horizontal_border(scr,1,IPL_BORDER_REFLECT);

	for(int j=0;j<out->height;j++) {
		register char*  oxp = out->imageData + j*out->widthStep;
		register unsigned char* iixp= (unsigned char *) in->imageData + j*in->widthStep;
		register unsigned short* ixp = (unsigned short *) (scr->imageData + j*scr->widthStep);

		for(int iout=0,i=0;iout<out->width;i++,iout+=2,ixp++)
		{
			register int val;
			val = (int)*iixp++ - (int)((*ixp * 6 +  ixp[-1] + ixp[1] + 32) >> 6);
			*oxp++ = (char) MAX(MIN(val,127),-128);

			val =  (int)*iixp++ - (int)( (*ixp + ixp[1] + 8 ) >> 4);
			*oxp++ = (char) MAX(MIN(val,127),-128);
		}
	}

	return APR_SUCCESS;
}


// out = in - expand(in1)
apr_status_t mipl_expand_gauss5_sub_bc(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr)
{
	return mipl_expand_gauss5_sub_bc_save_intermediate(in,in1,out,out,scr);
}

// assume cliplo, cliphi,add are of the form -2^B, 2^B -1, 2^B
apr_status_t mipl_sub_clip_add_shift_b(IplImage* in, IplImage *in1, IplImage* out,int numbits,int shift,IplImage* scr)
{
	dims d;
	twoshorts ts;
	d.width = (out->width+1)>>1;
	d.height = (out->height+1)>>1;
	d.output = (unsigned char*) out->imageData;
	ts.hi = numbits;
	ts.lo = shift;
	sub_clip_add_shift(
			(unsigned char*)in1->imageData,
			(unsigned char*)in->imageData, &d, ts);
	return APR_SUCCESS;
}


// assume cliplo, cliphi,add are of the form -2^B, 2^B -1, 2^B
apr_status_t mipl_expand_gauss5_sub_clip_add_shift_b(IplImage* in, IplImage *in1, IplImage* out,
													 int cliplo,int cliphi,int add,int shift,
													 IplImage* scr)
{
	dims d;
	twoshorts ts;
	d.width = in1->width;
	d.height = in1->height;
	d.output = (unsigned char*) out->imageData;
	int numbits=0;
	while(add>1){
		add>>=1;
		numbits++;
	}
	ts.hi = numbits;
	ts.lo = shift;
	gaus5_expand_reflect(
			(unsigned char*)in1->imageData,
			(unsigned char*)scr->imageData, &d);
	sub_clip_add_shift(
			d.output,
			(unsigned char*)in->imageData, &d, ts);
	return APR_SUCCESS;
}

void mipl_setZeros_inImg(IplImage* img)
{
	cvSetZero(img);
}

// following four methods work on 2 interlaced streams of shorts (dx, dy)
apr_status_t mipl_reduce_gauss5_2s(IplImage* in,IplImage* out,IplImage* scr){
	int args[]={in->width,in->height,in->widthStep,out->widthStep,(int)(scr->imageData)};
	gaus5_reduce_reflect_2s((int *)in->imageData, (int *)out->imageData,args);
	return APR_SUCCESS;
}
/**
 * +ve scaleshift means upscale
 * -ve scaleshift means downscale
 */
apr_status_t mipl_expand_gauss5_2s(IplImage* out,IplImage* in,IplImage* scr, int scaleShift){
	int args[]={in->width,in->height,in->widthStep,out->widthStep,(int)(scr->imageData),scaleShift};
	gaus5_expand_reflect_2s((int *)in->imageData, (int *)out->imageData,args);
	return APR_SUCCESS;
}

/*
 * Reduces an image by 4x e.g. 2048x2048=>512x512. Following restrictions apply:
 * 1. input width should be 16X
 * 2. input widthstep should be 4X
 * 3. output widthstep should be 4X
 * 4. minimum scratch space needed: 5*(input width in bytes)/2 (2.5 rows of input)
 */
apr_status_t mipl_reduce_gauss5_4x_b(IplImage* in,IplImage* out,IplImage* scr){
	assert(scr->imageSize>=4+5*(in->width>>1));	// scratch should be 2.5*width +4 bytes for safety
	return mipl_reduce_gauss5_4x_b(in,out,scr->imageData);
}
apr_status_t mipl_reduce_gauss5_4x_b(IplImage* in,IplImage* out,char* scr){

	int args[]={in->width,in->height,in->widthStep,out->widthStep,(int)scr};

	assert((in->width>>4)<<4==in->width);	// in->width should be 16X
	assert((in->widthStep>>2)<<2==in->widthStep);	// in->width should be 4X
	assert((in->height>>2)<<2==in->height);	// in->height should be 4X


	gaus5_reduce_4x_b((int *)in->imageData, (int *)out->imageData,args);
	return APR_SUCCESS;

}

apr_status_t mipl_reduce_gauss5_VertBinning(IplImage* in,IplImage* out,IplImage* scr){
	assert(scr->imageSize>=5*(in->width));	// scratch should be 5*width bytes for safety
	return mipl_reduce_gauss5_VertBinning(in,out,scr->imageData);
}

apr_status_t mipl_reduce_gauss5_HorzBinning(IplImage* in,IplImage* out,IplImage* scr){
	assert(scr->imageSize>=5*(in->width));	// scratch should be 5*width bytes for safety
	return mipl_reduce_gauss5_HorzBinning(in,out,scr->imageData);
}

apr_status_t mipl_reduce_gauss5_VertBinning(IplImage* in,IplImage* out,char* scr){

	int args[]={in->widthStep,in->height,(int)scr};
	assert((in->width>>3)<<3==in->width);	// in->width should be 8X
	assert((in->widthStep>>3)<<3==in->widthStep);// in->width should be 8X
	assert((in->height>>4)<<4==in->height);	// in->height should be 16X
	compute_1296x1944To648x486((unsigned char*)in->imageData, (unsigned char*)out->imageData,args);
	return APR_SUCCESS;
}
apr_status_t mipl_reduce_gauss5_HorzBinning(IplImage* in,IplImage* out,char* scr){

	int args[]={in->widthStep,in->height,(int)scr};
	assert((in->width>>4)<<4==in->width);	// in->width should be 8X
	assert((in->widthStep>>4)<<4==in->widthStep);// in->width should be 8X
	assert((in->height>>3)<<3==in->height);	// in->height should be 16X
	compute_2592x972To648x486((unsigned char*)in->imageData, (unsigned char*)out->imageData,args);
	return APR_SUCCESS;
}


apr_status_t mipl_reduce_gauss5_wscr_2spyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
	for(int lev=flev;lev<clev;lev++)
		mipl_reduce_gauss5_2s(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_wscr_2spyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
  for(int lev=clev-1;lev >= flev; lev--)
		mipl_expand_gauss5_2s(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

/*
 * output needs to have a border of 1 (integer) on top and left
 * scratch has to be (w+1) integers = 4*(w+1) bytes
 *
 */
apr_status_t mipl_integral_image(IplImage *in, IplImage *out, void *scratch){

	assert(out->width==in->width+1);

	int params[]={
			in->width,
			in->height,
			in->widthStep - in->width,
			(int)scratch
		};
	compute_integral_image(
			(unsigned char *)in->imageData,
			(unsigned int *)out->imageData,
			params);
	return APR_SUCCESS;
}
/*
 * both outputs need to have a border of 1 (integer) on top and left
 * scratch has to be 2*(w+1) integers = 8*(w+1) bytes
 *
 */
apr_status_t mipl_integral_with_square_image(IplImage *in, IplImage *out, IplImage *out_sq, void *scratch){
	assert(out->width==in->width+1);
	assert(out_sq->width==out->width);

	int params[]={
			in->width,
			in->height,
			in->widthStep - in->width,
			(int)scratch,
			(int)(out_sq->imageData)
		};
	compute_integral_with_square_image(
			(unsigned char *)in->imageData,
			(unsigned int *)out->imageData,
			params);
	return APR_SUCCESS;


}
/*
 * scratch has to be 2*(wd+3) integers = 8*(wd+3) bytes where wd is output width
 * Make sure the output is zero filled is appropriate
 *
 */
void mipl_scale_warp_b(IplImage *in, IplImage *out, double *parameters, void *scratch){

	int xstartSrc = 0, ystartSrc = 0;
	int xstartDest = 0, ystartDest = 0;
	int wd=out->width, hd=out->height;
	int ws=in->width, hs=in->height;
	int xcount = wd, ycount = hd;
	float tx = 0, ty = 0;
	if(parameters[2] >= 0)
	{
		xstartSrc = (int) (parameters[2] * 0x400000 + 0.5);
		xstartDest = 0;
	}
	else
	{
		xstartDest = (int) ceil(-parameters[2]/parameters[0]);
		xstartSrc = (int) ((parameters[0]*xstartDest + parameters[2])*0x400000 + 0.5);
	}
	if(parameters[3] >= 0)
	{
		ystartSrc = (int) (parameters[3] * 0x400000 + 0.5);
		ystartDest = 0;
	}
	else
	{
		ystartDest = (int) ceil(-parameters[3]/parameters[1]);
		ystartSrc = (int) ((parameters[1]*ystartDest + parameters[3])*0x400000 + 0.5);;
	}
	tx = (wd - 1)*parameters[0] + parameters[2];
	if(tx <= ws - 1)
	{
		xcount = wd - xstartDest;
	}
	else
	{
		xcount = (int) (((ws - 1) - parameters[2])/parameters[0]) - xstartDest + 1;
	}

	ty = (hd - 1)*parameters[1] + parameters[3];
	if(ty <= hs - 1)
	{
		ycount = hd - ystartDest;
	}
	else
	{
		ycount = (int) (((hs - 1) - parameters[3])/parameters[1]) - ystartDest + 1;
	}
	int dimst[]={
		out->width,
		out->height,
		in->widthStep,
		out->widthStep-out->width,
		(int)scratch,
		(int) (parameters[0] * 0x400000 + 0.5),
		(int) (parameters[1] * 0x400000 + 0.5),
		(int) (xstartSrc),
		(int) (ystartSrc),
		xstartDest,
		xcount,
		ystartDest,
		ycount
	};
	int initialBytes=ystartDest*wd;
	scale_warp_b(
			(unsigned char *)(in->imageData),
			dimst,
			(unsigned char *)(out->imageData + initialBytes)
			);
}
#endif //_BFIN_


