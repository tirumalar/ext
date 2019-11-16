#include "GaussPyr.h"
#include <algorithm>
#include <opencv/cv.h>
using std::max;
using std::min;
apr_status_t set_horizontal_border(IplImage *in, int border, int borderType, int val)
{
	switch(borderType)
	{
	case IPL_BORDER_REPLICATE:
		{
			int pix_size = (((~IPL_DEPTH_SIGN) & in->depth) >> 3)*in->nChannels;
			for(int i=0;i<in->height;i++)
			{
				char *in1ptr = in->imageData + in->widthStep*i;
				char *in2ptr = in1ptr + (in->width-1)*pix_size;

				for(int j=1;j<=border; j++)
				{
					memcpy(in1ptr - j*pix_size, in1ptr, pix_size);
					memcpy(in2ptr + j*pix_size, in2ptr, pix_size);
				}
			}
		}
		break;
	case IPL_BORDER_REFLECT:
		{
			int pix_size = (((~IPL_DEPTH_SIGN) & in->depth) >> 3)*in->nChannels;
			char *in1ptr = in->imageData;
			char *in2ptr = in->imageData + (in->width - 1)*pix_size;
			int area = pix_size*border;
			for(int i=0;i<in->height;i++, in1ptr += in->widthStep, in2ptr += in->widthStep)
			{
				for(int j=-area; j<0; j+=pix_size)
				{
					memcpy(in1ptr+j, in1ptr-j, pix_size);
					memcpy(in2ptr-j, in2ptr+j, pix_size);
				}
			}
		}
		break;
	case IPL_BORDER_CONSTANT:
		{
			int pix_size = (((~IPL_DEPTH_SIGN) & in->depth) >> 3)*in->nChannels;
			int area = pix_size*border;
			char *in1ptr = in->imageData - area;
			char *in2ptr = in->imageData + in->width*pix_size ;

			for(int i=0;i<in->height;i++, in1ptr += in->widthStep, in2ptr += in->widthStep)
			{
				memset(in1ptr, val, area);
				memset(in2ptr, val, area);
			}
		}
	default:
		break;
	}

	return APR_SUCCESS;
}

apr_status_t set_vertical_border(IplImage *in, int border, int borderType, int val)
{
	int pix_size = (((~IPL_DEPTH_SIGN) & in->depth) >> 3)*in->nChannels;

	switch(borderType)
	{
	case IPL_BORDER_REPLICATE:
		{
			char *in1ptr = in->imageData - in->widthStep * border - pix_size*border;
			char *in2ptr = in->imageData + in->widthStep*in->height - pix_size*border;

			for(int i=0;i<border;i++, in1ptr += in->widthStep, in2ptr += in->widthStep)
			{
				memcpy(in1ptr, in->imageData - pix_size*border, in->widthStep);
				memcpy(in2ptr, in->imageData + in->widthStep*(in->height-1) - pix_size*border, in->widthStep);
			}
		}
		break;
	case IPL_BORDER_REFLECT:
		{
			int step = in->widthStep;
			char *in1ptr = in->imageData - pix_size*border;
			char *in2ptr = in->imageData + in->widthStep*(in->height-1) - pix_size*border;

			for(int i=1;i<=border;i++)
			{
				memcpy(in1ptr - i*step, in1ptr + i*step, step);
				memcpy(in2ptr + i*step, in2ptr - i*step, step);
			}
		}
		break;
	case IPL_BORDER_CONSTANT:
		{
			int area = in->widthStep*border;
			char *in1ptr = in->imageData - area - pix_size*border;
			char *in2ptr = in->imageData + in->widthStep*in->height - pix_size*border;

			memset(in1ptr, val, area);
			memset(in2ptr, val, area);
		}
	default:
		break;
	}

	return APR_SUCCESS;
}

apr_status_t set_border(IplImage *in, int border, int borderType, int val)
{
	apr_status_t rv= APR_SUCCESS;
	rv |= set_horizontal_border(in, border, borderType, val);
	rv |= set_vertical_border(in, border, borderType, val);

	return rv;
}

apr_status_t mipl_filt_gauss5_b(IplImage* in, IplImage* out, IplImage* scr)
{
	set_horizontal_border(in,3,IPL_BORDER_REFLECT);

	/* Horizontal filter */
	for(int j=0;j<in->height;j++)
	{
		register unsigned char* ixp = (unsigned char *) in->imageData + j*in->widthStep;
		unsigned short* oxp = (unsigned short *) (scr->imageData + j*scr->widthStep);
		for(int i=0;i<out->width;i++,ixp++)
		{
			*oxp++ = *ixp * 6  + (( ixp[-1]+ ixp[1] ) << 2 ) +	ixp[-2]+ ixp[2];
		}
	}

	/* Vertical Filter */
	set_vertical_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<out->height;j++)
	{
		unsigned char *oxp  = (unsigned char *) out->imageData + j*out->widthStep;
		unsigned short *xp   = (unsigned short *) (scr->imageData + j*scr->widthStep);
		unsigned short *xp2_ = xp -  scr->widthStep;
		unsigned short *xp1_ = xp - (scr->widthStep >> 1);
		unsigned short *xp1  = xp + (scr->widthStep >> 1);
		unsigned short *xp2  = xp +  scr->widthStep;

		for(int i=0;i<out->width;i++,xp++,xp1++,xp1_++,xp2++,xp2_++)
		{
			register unsigned short val;
			val =  *xp * 6 + (( *xp1 + *xp1_ ) << 2 ) + *xp2 + *xp2_ + 128;
			*oxp++ = (unsigned char) ( val >> 8 );
		}
	}

	return APR_SUCCESS;

}

// lap is laplacian at lev
// in1 is gaussian at lev+1
// in is gaussian at lev
// out = in + expand(in1)
apr_status_t mipl_expand_gauss5_add_cb(IplImage* in, IplImage *in1, IplImage* lap,IplImage* scr)
{
	/* Vertical Filter */
	set_vertical_border(in1,3,IPL_BORDER_REFLECT);

	for(int jj=0,j=0; jj < in->height ; j++, jj+=2 )
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
	set_horizontal_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<in->height;j++) {
		register unsigned char* oxp = (unsigned char *) in->imageData + j*in->widthStep;
		register char* iixp = lap->imageData + j*lap->widthStep;
		register unsigned short* ixp = (unsigned short *) (scr->imageData + j*scr->widthStep);

		for(int iout=0,i=0;iout<in->width;i++,iout+=2,ixp++)
		{
			register int val;
			val = (int)*iixp++ + (int)((*ixp * 6 +  ixp[-1] + ixp[1] + 32) >> 6);
			*oxp++ = (unsigned char) max(min(val, 255),0);

			val =  (int)*iixp++ + (int)( (*ixp + ixp[1] + 8 ) >> 4);
			*oxp++ = (unsigned char) max(min(val, 255),0);
		}
	}

	return APR_SUCCESS;
}

apr_status_t mipl_filt_gauss5_sub_clip_add_shift_b(IplImage* in,IplImage* out,
												   int cliplo,int cliphi,int add,int shift,
												   IplImage* scr)
{
	set_horizontal_border(in,3,IPL_BORDER_REFLECT);

	/* Horizontal filter */
	for(int j=0;j<in->height;j++)
	{
		register unsigned char* ixp = (unsigned char *) in->imageData + j*in->widthStep;
		unsigned short* oxp = (unsigned short *) (scr->imageData + j*scr->widthStep);
		for(int i=0;i<out->width;i++,ixp++)
		{
			*oxp++ = *ixp * 6  + (( ixp[-1]+ ixp[1] ) << 2 ) + ixp[-2]+ ixp[2];
		}
	}

	/* Vertical Filter */
	set_vertical_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<out->height;j++)
	{
		unsigned char *oxp  = (unsigned char *) out->imageData + j*out->widthStep;
		unsigned short *xp   = (unsigned short *) (scr->imageData + j*scr->widthStep);
		unsigned short *xp2_ = xp -  scr->widthStep;
		unsigned short *xp1_ = xp - (scr->widthStep >> 1);
		unsigned short *xp1  = xp + (scr->widthStep >> 1);
		unsigned short *xp2  = xp +  scr->widthStep;
		unsigned char *iixp  = (unsigned char *) in->imageData + j*in->widthStep;

		for(int i=0;i<out->width;i++,xp++,xp1++,xp1_++,xp2++,xp2_++, iixp++)
		{
			register int val;
			val =  (int)  *iixp - (int) ((*xp * 6 + (( *xp1 + *xp1_ ) << 2 ) + *xp2 + *xp2_ + 128) >> 8);
			val = max(min(val,cliphi),cliplo) + add;
			*oxp++ = (unsigned char)(val>>shift);
		}
	}

	return APR_SUCCESS;

}

// scr must have scr->height == in->height and scr->width = out->width = (in->width + 1)>>1

apr_status_t mipl_reduce_gauss5_s(IplImage* in,IplImage* out,IplImage* scr)
{
	set_horizontal_border(in,3,IPL_BORDER_REFLECT);

	/* Horizontal filter */
	for(int j=0;j<in->height;j++)
	{
		register short* ixp = (short *) (in->imageData + j*in->widthStep);
		int* oxp = (int *) (scr->imageData + j*scr->widthStep);
		for(int il=0;il<out->width;il++,ixp+=2) {
			*oxp++ = *ixp * 6  + (( ixp[-1]+ ixp[1] ) << 2 ) +	ixp[-2] + ixp[2];
		}
	}

	set_vertical_border(scr,3,IPL_BORDER_REFLECT);

	/* Vertical Filter */
	for(int jl=0,j=0;jl<out->height;jl++,j+=2)
	{
		short *oxp  = (short *) (out->imageData + jl*out->widthStep);
		int *xp   = (int *) (scr->imageData + j*scr->widthStep);
		int *xp2_ = xp - (scr->widthStep >> 1);
		int *xp1_ = xp - (scr->widthStep >> 2);
		int *xp1  = xp + (scr->widthStep >> 2);
		int *xp2  = xp + (scr->widthStep >> 1);

		for(int i=0;i<out->width;i++,xp++,xp1++,xp1_++,xp2++,xp2_++) {
			register int val;
			val =  *xp * 6 + (( *xp1 + *xp1_ ) << 2 ) +	*xp2 + *xp2_;
			*oxp++ = (short) ( (val > 0 ) ? (val + 128)/256 : (val - 127)/256);
		}
	}

	return APR_SUCCESS;
}


// scr must have scr->height = 2*in1->height, scr->width = in1->width
apr_status_t mipl_expand_gauss5_s(IplImage* in, IplImage *in1, IplImage* scr, int scaleShift)
{
	/* Vertical Filter */
	set_vertical_border(in1,3,IPL_BORDER_REFLECT);

	for(int jj=0,j=0; jj < in->height ; j++, jj+=2 )
	{
		int *oxp   = (int *) (scr->imageData + jj*scr->widthStep);
		int * oxp1  = oxp + (scr->widthStep >> 2);
		short* ixp   = (short *) (in1->imageData + j*in1->widthStep);
		short* ixp1_ = ixp - (in1->widthStep >> 1);
		short* ixp1  = ixp + (in1->widthStep >> 1);

		for(int i=0; i< in1->width ; i++, ixp++,ixp1++,ixp1_++ )
		{
			*oxp++ = *ixp * 6 + *ixp1 + *ixp1_;
			*oxp1++ = ( (*ixp + *ixp1) * 4);
		}
	}

	/* Horizontal Filter */
	set_horizontal_border(scr,3,IPL_BORDER_REFLECT);
	int scale_1 = 6-scaleShift;
	int scale_2 = 4-scaleShift;
	int halfBit1 = 1<<(scale_1-1);
	int halfBit2 = 1<<(scale_2-1);
	scale_1=1<<scale_1;
	scale_2=1<<scale_2;

	for(int j=0;j<in->height;j++)
	{
		short* oxp = (short *) (in->imageData + j*in->widthStep);
		int* ixp = (int *) (scr->imageData + j*scr->widthStep);

		for(int iout=0;iout<in->width;iout+=2,ixp++)
		{
			register int val;
			val=*ixp * 6 +  ixp[-1] + ixp[1];
			*oxp++ = (short)((val>0)?((val+halfBit1)/scale_1):((val-halfBit1+1)/scale_1));
			val=*ixp + ixp[1];
			*oxp++ = (short)((val>0)?((val+halfBit2)/scale_2):((val-halfBit2+1)/scale_2));
		}
	}

	return APR_SUCCESS;
}

apr_status_t mipl_reduce_gauss5_wscr_bpyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
	for(int lev=flev;lev<clev;lev++)
		mipl_reduce_gauss5_b(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_wscr_bpyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
  for(int lev=clev-1;lev >= flev; lev--)
		mipl_expand_gauss5_b(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_reduce_gauss5_wscr_spyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
	for(int lev=flev;lev<clev;lev++)
		mipl_reduce_gauss5_s(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_wscr_spyr(IplImage** pyr,int flev,int clev, IplImage **scr)
{
  for(int lev=clev-1;lev >= flev; lev--)
		mipl_expand_gauss5_s(pyr[lev],pyr[lev+1],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_wscr_bcpyr(IplImage** inpyr, IplImage** outpyr,int flev,int clev, IplImage **scr)
{
  for(int lev=clev-1;lev >= flev; lev--)
		mipl_expand_gauss5_sub_bc(inpyr[lev],inpyr[lev+1],outpyr[lev],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_add_cbpyr(IplImage** gpyr, IplImage** lappyr,int flev,int clev, IplImage **scr)
{
  for(int lev=clev-1;lev >= flev; lev--)
		mipl_expand_gauss5_add_cb(gpyr[lev],gpyr[lev+1],lappyr[lev],scr[lev]);

	return APR_SUCCESS;
}

apr_status_t mipl_expand_gauss5_clip_add_shift_bpyr(IplImage** inpyr, IplImage** outpyr,
													int flev,int clev,int cliplo,int cliphi,int add,int shift)
{
	for(int lev=flev;lev < clev; lev++ )
		mipl_expand_gauss5_sub_clip_add_shift_b(inpyr[lev],inpyr[lev+1],outpyr[lev],cliplo,cliphi,add,shift,NULL);

	return APR_SUCCESS;
}


IplImage *mipl_alloc_image_header(int w, int h, int border, int depth, int channels)
{
	IplImage *image = cvCreateImageHeader(cvSize(w+2*border, h+2*border), depth, channels);
	if(image)
	{
		image->height = h;
		image->width = w;
	}
	return image;
}

IplImage *mipl_alloc_image(int w, int h, int border, int depth, int channels)
{
	IplImage *image = cvCreateImage(cvSize(w+2*border, h+2*border), depth, channels);
	if(image)
	{
		image->height = h;
		image->width = w;
		unsigned int pix_size = ((~IPL_DEPTH_SIGN) & image->depth) >> 3;
		image->imageData += border*channels*pix_size + image->widthStep * border;
	}

	return image;
}
IplImage **mipl_alloc_pyr(int fine_lev, int coarse_lev, int w, int h, int border, int depth, int channels)
{
	IplImage **imgpyr = (IplImage **) calloc((coarse_lev+1), sizeof(IplImage *));

	for(int lev = 0; lev <= coarse_lev; lev++)
	{
		if(lev >= fine_lev)
			imgpyr[lev] = mipl_alloc_image(w,h,border,depth,channels);

		w = (w + 1) >> 1;
		h = (h + 1) >> 1;
	}

	return imgpyr;
}

IplImage **mipl_alloc_pyr_bottomBorder(int fine_lev, int coarse_lev, int w, int h, int bottomBorder, int depth, int channels)
{
	IplImage **imgpyr = (IplImage **) calloc((coarse_lev+1), sizeof(IplImage *));

	for(int lev = 0; lev <= coarse_lev; lev++)
	{
		if(lev >= fine_lev)
		{
			IplImage *img=imgpyr[lev] = mipl_alloc_image(w,h+bottomBorder,0,depth,channels);
			cvSetZero(img);
			img->height=h;
			int pix_size = (((~IPL_DEPTH_SIGN) & img->depth) >> 3)*img->nChannels;
			img->widthStep=w*pix_size;
		}

		w = (w + 1) >> 1;
		h = (h + 1) >> 1;
	}

	return imgpyr;
}

void mipl_free_image(IplImage **image)
{
	cvReleaseImage(image);
}
void mipl_free_image_header(IplImage **image)
{
	cvReleaseImageHeader(image);
}
void mipl_free_pyr(IplImage **pyr, int fine_lev, int coarse_lev)
{
	if(pyr)
	{
		for(int i=0;i<=coarse_lev;i++){
			if(i >= fine_lev)
				if(pyr[i])	mipl_free_image(pyr+i);
		}
		free(pyr);
	}
}
/************************************************
 * Methods below this point compile conditionally based on the CPU
 * x86  (code here)
 * BFIN (found in BFin folder)
 ************************************************/

#ifndef __BFIN__

// scr must have scr->height == in->height and scr->width = out->width = (in->width + 1)>>1
apr_status_t mipl_reduce_gauss5_b(IplImage* in,IplImage* out,IplImage* scr)
{
	set_horizontal_border(in,3,IPL_BORDER_REFLECT);

	/* Horizontal filter */
	for(int j=0;j<in->height;j++)
	{
		register unsigned char* ixp = (unsigned char *) in->imageData + j*in->widthStep;
		unsigned short* oxp = (unsigned short *) (scr->imageData + j*scr->widthStep);
		for(int il=0;il<out->width;il++,ixp+=2) {
			*oxp++ = *ixp * 6  + (( ixp[-1]+ ixp[1] ) << 2 ) +	ixp[-2] + ixp[2];
		}
	}

	set_vertical_border(scr,3,IPL_BORDER_REFLECT);

	/* Vertical Filter */
	for(int jl=0,j=0;jl<out->height;jl++,j+=2)
	{
		unsigned char *oxp  = (unsigned char *) out->imageData + jl*out->widthStep;
		unsigned short *xp   = (unsigned short *) (scr->imageData + j*scr->widthStep);
		unsigned short *xp2_ = xp -  scr->widthStep;
		unsigned short *xp1_ = xp - (scr->widthStep >> 1);
		unsigned short *xp1  = xp + (scr->widthStep >> 1);
		unsigned short *xp2  = xp +  scr->widthStep;

		for(int i=0;i<out->width;i++,xp++,xp1++,xp1_++,xp2++,xp2_++) {
			register unsigned short val;
			val =  *xp * 6 + (( *xp1 + *xp1_ ) << 2 ) +	*xp2 + *xp2_ + 128;
			*oxp++ = (unsigned char) ( val >> 8 );
		}
	}

	return APR_SUCCESS;
}

// scr - height = in->height/2, and width = out->width or in->width/4
apr_status_t mipl_reduce_gauss5_4x_b(IplImage* in, IplImage* out, IplImage* scr)
{
	/* Vertical Filter */
	set_horizontal_border(in,4,IPL_BORDER_REFLECT);

	/* Horizontal filter */
	for(int j=0;j<out->height;j++)
	{
		// every alternate row of the input data is filtered, note the factor of 2
		register unsigned char* ixp = (unsigned char *) in->imageData + 2*j*in->widthStep;
		unsigned short* oxp = (unsigned short *) (scr->imageData + j*scr->widthStep);
		// 1 0 4 0 6 0 4 0 1 kernel is used to skip alternate pixels
		for(int il=0;il<out->width;il++,ixp+=4) {
			*oxp++ = *ixp * 6  + (( ixp[-2]+ ixp[2] ) << 2 ) +	ixp[-4] + ixp[4];
		}
	}

	// vertical filter is same as before
	set_vertical_border(scr,3,IPL_BORDER_REFLECT);

	/* Vertical Filter */
	for(int jl=0,j=0;jl<out->height;jl++,j+=2)
	{
		unsigned char *oxp  = (unsigned char *) out->imageData + jl*out->widthStep;
		unsigned short *xp   = (unsigned short *) (scr->imageData + j*scr->widthStep);
		unsigned short *xp2_ = xp -  scr->widthStep;
		unsigned short *xp1_ = xp - (scr->widthStep >> 1);
		unsigned short *xp1  = xp + (scr->widthStep >> 1);
		unsigned short *xp2  = xp +  scr->widthStep;

		for(int i=0;i<out->width;i++,xp++,xp1++,xp1_++,xp2++,xp2_++) {
			register unsigned short val;
			val =  *xp * 6 + (( *xp1 + *xp1_ ) << 2 ) +	*xp2 + *xp2_ + 128;
			*oxp++ = (unsigned char) ( val >> 8 );
		}
	}

	return APR_SUCCESS;
}
// scr must have scr->height = 2*in1->height, scr->width = in1->width
apr_status_t mipl_expand_gauss5_b(IplImage* in, IplImage *in1, IplImage* scr)
{
	/* Vertical Filter */
	set_vertical_border(in1,3,IPL_BORDER_REFLECT);

	for(int jj=0,j=0; jj < in->height ; j++, jj+=2 )
	{
		unsigned short* oxp   = (unsigned short *) (scr->imageData + jj*scr->widthStep);
		unsigned short* oxp1  = oxp + (scr->widthStep >> 1);
		unsigned char* ixp   = (unsigned char *) in1->imageData + j*in1->widthStep;
		unsigned char* ixp1_ = ixp - in1->widthStep;
		unsigned char* ixp1  = ixp + in1->widthStep;

		for(int i=0; i< in1->width ; i++, ixp++,ixp1++,ixp1_++ )
		{
			*oxp++ = *ixp * 6 + *ixp1 + *ixp1_;
			*oxp1++ = ( (*ixp + *ixp1) * 4);
		}
	}

	/* Horizontal Filter */
	set_horizontal_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<in->height;j++)
	{
		unsigned char* oxp = (unsigned char *) in->imageData + j*in->widthStep;
		unsigned short* ixp = (unsigned short *) (scr->imageData + j*scr->widthStep);

		for(int iout=0;iout<in->width;iout+=2,ixp++)
		{
			*oxp++ = (*ixp * 6 +  ixp[-1] + ixp[1]+32 ) >>6;
			*oxp++ = (*ixp + ixp[1]+8) >>4;
		}
	}

	return APR_SUCCESS;
}

// out = in - expand(in1)
apr_status_t mipl_expand_gauss5_sub_bc(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr)
{
	/* Vertical Filter */
	set_vertical_border(in1,3,IPL_BORDER_REFLECT);

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
	set_horizontal_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<out->height;j++) {
		register char*  oxp = out->imageData + j*out->widthStep;
		register unsigned char* iixp= (unsigned char *) in->imageData + j*in->widthStep;
		register unsigned short* ixp = (unsigned short *) (scr->imageData + j*scr->widthStep);

		for(int iout=0,i=0;iout<out->width;i++,iout+=2,ixp++)
		{
			register int val;
			val = (int)*iixp++ - (int)((*ixp * 6 +  ixp[-1] + ixp[1] + 32) >> 6);
			*oxp++ = (char) max(min(val,127),-128);

			val =  (int)*iixp++ - (int)( (*ixp + ixp[1] + 8 ) >> 4);
			*oxp++ = (char) max(min(val,127),-128);
		}
	}

	return APR_SUCCESS;
}
apr_status_t mipl_expand_gauss5_sub_clip_add_shift_b(IplImage* in, IplImage *in1, IplImage* out,
													 int cliplo,int cliphi,int add,int shift,
													 IplImage* scr)
{

	/* Vertical Filter */
	set_vertical_border(in1,3,IPL_BORDER_REFLECT);

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
	set_horizontal_border(scr,3,IPL_BORDER_REFLECT);

	for(int j=0;j<out->height;j++)
	{
		register unsigned char*  oxp = (unsigned char *) out->imageData + j*out->widthStep;
		register unsigned char* iixp= (unsigned char *) in->imageData + j*in->widthStep;
		register unsigned short* ixp = (unsigned short *) (scr->imageData + j*scr->widthStep);

		for(int iout=0;iout<out->width;iout+=2,ixp++)
		{
			int val;
			val = (int)*iixp++ - (int)((*ixp * 6 +  ixp[-1] + ixp[1] + 32) >> 6);
			val = max(min(val,cliphi),cliplo) + add;
			*oxp++ = (unsigned char)(val>>shift);

			val =  (int)*iixp++ - (int)( (*ixp + ixp[1] + 8 ) >> 4);
			val = max(min(val,cliphi),cliplo) + add;
			*oxp++ = (unsigned char)(val>>shift);
		}
	}

	return APR_SUCCESS;
}

apr_status_t mipl_integral_image(IplImage *in, IplImage *out, void *scratch=0){
	cvIntegral(in, out);
	return APR_SUCCESS;
}
apr_status_t mipl_integral_with_square_image(IplImage *in, IplImage *out, IplImage *out_sq, void *scratch){
	cvIntegral(in, out,out_sq);
	return APR_SUCCESS;
}
/*
 * scratch is unused here
 */
void mipl_scale_warp_c(IplImage *in, IplImage *out, double *parameters, void *scratch  )
{
	int i,j;

	for(i=0;i<out->height;i++)
	{
		char *optr = out->imageData + out->widthStep*i;

		float ys = parameters[1] * i + parameters[3];
		if(ys < 0 || ys > in->height - 1)
		{
			memset(optr, 0, out->widthStep);
			continue;
		}

		int ysi = (int) ys;
		int yfrac = (int) ((ys - ysi)*256 + 0.5);
		int yfrac_ = 0x100 - yfrac;

		char *fptr1 = in->imageData + ysi*in->widthStep;
		char *fptr2 = fptr1 + in->widthStep;

		for(j=0;j<out->width;j++)
		{
			float xs = parameters[0]*j + parameters[2];
			if(xs < 0 || xs > in->width - 1)
			{
				*optr++ = 0;
			}
			else
			{
				int xsi = (int) xs;
				int xfrac = (int) ((xs - xsi)*256 + 0.5);
				int xfrac_ = 0x100 - xfrac;

				int y1val = xfrac_*fptr1[xsi] + xfrac* fptr1[xsi+1];
				int y2val = xfrac_*fptr2[xsi] + xfrac* fptr2[xsi+1];

				int val = yfrac_*y1val + yfrac*y2val + 0x8000;

				*optr++ = val >> 16;
			}
		}
	}
}
/*
 * scratch is unused here
 */

void mipl_scale_warp_b(IplImage *in, IplImage *out, double *parameters, void *scratch  )
{
	int i,j;

	for(i=0;i<out->height;i++)
	{
		unsigned char *optr = (unsigned char *) out->imageData + out->widthStep*i;

		double ys = parameters[1] * i + parameters[3];
		if(ys < 0 || ys > in->height - 1)
		{
			memset(optr, 0, out->widthStep);
			continue;
		}

		int ysi = (int) ys;
		int yfrac = (int) ((ys - ysi)*256 + 0.5);
		int yfrac_ = 0x100 - yfrac;

		unsigned char *fptr1 = (unsigned char *) in->imageData + ysi*in->widthStep;
		unsigned char *fptr2 = (unsigned char *) fptr1 + in->widthStep;

		for(j=0;j<out->width;j++)
		{
			double xs = parameters[0]*j + parameters[2];
			if(xs < 0 || xs > in->width - 1)
			{
				*optr++ = 0;
			}
			else
			{
				int xsi = (int) xs;
				int xfrac = (int) ((xs - xsi)*256 + 0.5);
				int xfrac_ = 0x100 - xfrac;

				int y1val = xfrac_*fptr1[xsi] + xfrac* fptr1[xsi+1];
				int y2val = xfrac_*fptr2[xsi] + xfrac* fptr2[xsi+1];

				int val = yfrac_*y1val + yfrac*y2val + 0x8000;

				*optr++ = (unsigned char) (val >> 16);
			}
		}
	}
}
#endif //NOT BFIN
