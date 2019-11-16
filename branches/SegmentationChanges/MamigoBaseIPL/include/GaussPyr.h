#pragma once

#include "apr.h"
#include "apr_errno.h"
#include "MamigoBaseIPL_def.h"
#include <opencv/cxcore.h>

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT set_horizontal_border(IplImage *in, int border, int borderType, int val=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT set_vertical_border(IplImage *in, int border, int borderType, int val=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT set_border(IplImage *in, int border, int borderType, int val=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_filt_gauss5_b(IplImage* in, IplImage* out, IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_b(IplImage* in,IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_b(IplImage* in,IplImage* out,char *scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_sub_bc(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_add_cb(IplImage* in, IplImage *in1, IplImage* lap, IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_sub_clip_add_shift_b(IplImage* in, IplImage *in1, IplImage* out,
													 int cliplo,int cliphi,int add,int shift,
													 IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_filt_gauss5_sub_clip_add_shift_b(IplImage* in,IplImage* out,
												   int cliplo,int cliphi,int add,int shift,
												   IplImage* scr);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_b(IplImage* in, IplImage *in1, IplImage* scr);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_wscr_bpyr(IplImage** pyr,int flev,int clev, IplImage **scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_wscr_spyr(IplImage** pyr,int flev,int clev, IplImage **scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_wscr_bpyr(IplImage** pyr,int flev,int clev, IplImage **scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_wscr_spyr(IplImage** pyr,int flev,int clev, IplImage **scr);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_wscr_bcpyr(IplImage** inpyr, IplImage** outpyr,int flev,int clev, IplImage **scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_clip_add_shift_bpyr(IplImage** inpyr, IplImage** outpyr,
													int flev,int clev,int cliplo,int cliphi,int add,int shift);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_add_cbpyr(IplImage** gpyr, IplImage** lappyr,
																	 int flev,int clev, IplImage **scr);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_s(IplImage* in, IplImage *in1, IplImage* scr, int scaleShift=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_s(IplImage* in,IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_4x_b(IplImage* in,IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_HorzBinning(IplImage* in,IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_VertBinning(IplImage* in,IplImage* out,IplImage* scr);


IplImage MAMIGO_BASE_IPL_DLL_EXPORT *mipl_alloc_image(int w, int h, int border=0, int depth=IPL_DEPTH_8U, int channels=1);
IplImage MAMIGO_BASE_IPL_DLL_EXPORT *mipl_alloc_image_header(int w, int h, int border=0, int depth=IPL_DEPTH_8U, int channels=1);
IplImage MAMIGO_BASE_IPL_DLL_EXPORT **mipl_alloc_pyr(int flev, int clev, int w, int h, int border=0, int depth=IPL_DEPTH_8U, int channels=1);
IplImage MAMIGO_BASE_IPL_DLL_EXPORT **mipl_alloc_pyr_bottomBorder(int fine_lev, int coarse_lev, int w, int h, int bottomBorder, int depth=IPL_DEPTH_8U, int channels=1);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_free_image(IplImage **image);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_free_image_header(IplImage **image);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_free_pyr(IplImage **pyr, int flev, int clev);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_integral_image(IplImage *in, IplImage *out, void *scratch);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_integral_with_square_image(IplImage *in, IplImage *out, IplImage *out_sq, void *scratch);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_scale_warp_c(IplImage *in, IplImage *out, double *parameters, void *scratch);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_scale_warp_b(IplImage *in, IplImage *out, double *parameters, void *scratch);

#if defined (__BFIN__)
apr_status_t mipl_expand_gauss5_sub_bc_slow(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_sub_bc(IplImage* in, IplImage *in1, IplImage* out,IplImage* scr);
apr_status_t mipl_expand_gauss5_sub_bc_save_intermediate(IplImage* in, IplImage *in1, IplImage* out,IplImage* out_inter,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_sub_clip_add_shift_b(IplImage* in, IplImage *in1, IplImage* out,int numbits,int shift,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_width_b(IplImage* in,IplImage* out,IplImage* scr=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_width_skip_b(IplImage* in,IplImage* out,int skip=0, IplImage* scr=0);

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_2s(IplImage* in,IplImage* out,IplImage* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_4x_b(IplImage* in,IplImage* out,char* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_HorzBinning(IplImage* in,IplImage* out,char* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_VertBinning(IplImage* in,IplImage* out,char* scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_2s(IplImage* out,IplImage* in,IplImage* scr, int scaleShift=0);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_expand_gauss5_wscr_2spyr(IplImage** pyr,int flev,int clev, IplImage **scr);
apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_reduce_gauss5_wscr_2spyr(IplImage** pyr,int flev,int clev, IplImage **scr);
IplImage **mipl_alloc_scratch_asPeudoPyr(int fine_lev, int coarse_lev, int w, int h);
void MAMIGO_BASE_IPL_DLL_EXPORT mipl_free_scratch_asPeudoPyr(IplImage **pyr, int file_lev, int coarse_lev);

void MAMIGO_BASE_IPL_DLL_EXPORT mipl_setZeros_inImg(IplImage* img);
#ifdef __cplusplus
extern "C" {
#endif
void Compute_Histogram(unsigned char *inp,int *hist, int *param);
#ifdef __cplusplus
}
#endif

#endif

