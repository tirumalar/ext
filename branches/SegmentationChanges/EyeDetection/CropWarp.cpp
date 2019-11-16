
#include "EyeCenterPoint.h"
#include "Image.h"
#include "pyramid.h"
#include <stdio.h>
#include <opencv/highgui.h>
#ifdef __BFIN__
#include "GaussPyr.h"

void CropAndWarp(const Image8u &src, int x, int y, double zoom, Image8u &dest, void *scratch)
{
	double parameters[4];

	parameters[0] = zoom;
	parameters[1] = parameters[0];
	parameters[2] = x - dest.GetWidth()*zoom/2.0;
	parameters[3] = y - dest.GetHeight()*zoom/2.0;

	mipl_scale_warp_b((IplImage *) src.GetData(), dest.GetData(), parameters, scratch);
}
#else
void scale_warp_b(IplImage *in, IplImage *out, double *parameters  )
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
void CropAndWarp(const Image8u &src, int x, int y, double zoom, Image8u &dest, void *scratch)
{
	double parameters[4];

	parameters[0] = zoom;
	parameters[1] = parameters[0];
	parameters[2] = x - dest.GetWidth()*zoom/2.0;
	parameters[3] = y - dest.GetHeight()*zoom/2.0;

	scale_warp_b((IplImage *) src.GetData(), dest.GetData(), parameters);
}
#endif

void CropAndWarp(const Image8u &src, const CEyeCenterPoint &point, double zoom, Image8u &dest, void *scratch)
{
	CropAndWarp(src, point.m_nCenterPointX, point.m_nCenterPointY, zoom, dest, scratch);
}


void CropAndWarp(Pyramid8u &pyramid, int x, int y, float scale, Image8u &output, void *scratch)
{
	float scaleInv = 1.0 / scale;
	int pyrLevelInv = int(log( scaleInv ) / log(2.0));
	float pyrScaleInv = power(2, pyrLevelInv);
	float pyrScale = 1.0 / pyrScaleInv;
	float scaleRes = scale / pyrScale;

	Image8u *pyrImage = pyramid.GetLevel(pyrLevelInv);

	output.Fill(0);
	CropAndWarp(*pyrImage, int(x * pyrScale + 0.5f), int(y * pyrScale + 0.5f), (1.0 / scaleRes), output, scratch);

	#ifdef MADHAV
		char Buff[1024];
		static int ctr =0;
		sprintf(Buff,"CropAndWarpInp%d.pgm",ctr);
		cvSaveImage(Buff,pyrImage->GetData());
		sprintf(Buff,"CropAndWarpOut%d.pgm",ctr++);
		cvSaveImage(Buff,output.GetData());
	#endif
}
