#include "LaplacianBasedFocusDetector.h"
#include <highgui.h>
#include <cv.h>
#include <stdio.h>

extern "C" {
//#include<file_manip.h>
}

//FILE * fp = fopen("time.csv","a+");
LaplacianBasedFocusDetector::LaplacianBasedFocusDetector(int width, int height): m_width(width), m_height(height),
pyrImage1(0), pyrImage2(0), pyrImage3(0), pyrImage4(0)
{
	pyrImage1 = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_32S, 1);
	//pyrImage2 = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_32S, 1);
	pyrImage2 = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_16U, 1);
	pyrImage3 = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_32S, 1);
	pyrImage4 = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_32S, 1);
    maskImage = cvCreateImage(cvSize(m_width,m_height), IPL_DEPTH_8U, 1);

	pyrImage1->depth =32;
    pyrImage2->depth =16;
    //pyrImage2->depth =32;
	pyrImage3->depth =32;
	pyrImage4->depth =32;

}

LaplacianBasedFocusDetector::~LaplacianBasedFocusDetector(void)
{
	if(pyrImage1)		cvReleaseImage(&pyrImage1);	pyrImage1 = 0;
	if(pyrImage2)		cvReleaseImage(&pyrImage2);	pyrImage2 = 0;
	if(pyrImage3)		cvReleaseImage(&pyrImage3);	pyrImage3 = 0;
	if(pyrImage4)		cvReleaseImage(&pyrImage4);	pyrImage4 = 0;
	if(maskImage)       cvReleaseImage(&maskImage);	pyrImage4 = 0;
}

static void GaussianBlur(IplImage* iframe, IplImage *oframe, CvPoint border)
{
	for(int i = 0; i < iframe->height; i++)
	{

	    int *iptr = (int *) (iframe->imageData + i*iframe->widthStep);
		int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
		
		for(int j = border.x; j < iframe->width - border.x; j++)
		{
		//	optr[j] = iptr[j-2]/16 + iptr[j-1]/4 + (iptr[j]*3)/8 + iptr[j+1]/4 + iptr[j+2]/16;
			optr[j] = (iptr[j-2] + iptr[j-1]*4 + (iptr[j]*6) + iptr[j+1]*4 + iptr[j+2] + 8)>>4;
		}
	/*	float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
		float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		for(int j = border.x; j < iframe->width - border.x; j++)
			optr[j] = iptr[j-2]*0.0625f + iptr[j-1]*0.25f + iptr[j]*0.375f + iptr[j+1]*0.25f + iptr[j+2]*0.0625f;*/

	}
}


static void GaussianBlur3x3(IplImage* iframe, IplImage *oframe, CvPoint border)
{
	for(int i = 0; i < iframe->height; i++)
	{
		float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
		float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		for(int j = border.x; j < iframe->width - border.x; j++)
			optr[j] = iptr[j-1]*0.25f + iptr[j]*0.5f + iptr[j+1]*0.25f;

	}
}

static void absDiffMask(IplImage* i1frame, IplImage *i2frame, IplImage *oframe, int dval=0)
{
	for(int i = 0; i < oframe->height; i++)
	{
		int *i1ptr = (int *) (i1frame->imageData + i*i1frame->widthStep);
		int *i2ptr = (int *) (i2frame->imageData + i*i2frame->widthStep);
		int *optr = (int *) (oframe->imageData + i*oframe->widthStep);

		for(int j = 0; j < oframe->width; j++)
		{
			int v = i1ptr[j] - i2ptr[j];
			int mask = v >> 31;
			optr[j] = (v+mask)^mask;

//			optr[j] = (abs(i1ptr[j] - i2ptr[j]));
		}
	}
}

static void absAliasedLaplacianMask(IplImage* i1frame, IplImage *oframe, int step, float dval=0)
{
	for(int i = 0; i < oframe->height; i++)
	{
		float *i1ptr = (float *) (i1frame->imageData + i*i1frame->widthStep);
		float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		for(int j = step; j < oframe->width - step; j++)
			optr[j] = (i1ptr[j] < 0 || i1ptr[j-step] < 0 || i1ptr[j+step] < 0)? dval: abs(i1ptr[j] - (i1ptr[j+step]+i1ptr[j-step])*0.5f);
	}
}

static void avgMask(IplImage* i1frame, IplImage *oframe)
{
	int w = i1frame->widthStep/sizeof(float);

	for(int i = 1; i < oframe->height-1; i++)
	{
		float *i1ptr = (float *) (i1frame->imageData + i*i1frame->widthStep);
		float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		for(int j = 1; j < oframe->width-1; j++)
		{
			float val = (i1ptr[j-w-1] + i1ptr[j-w] + i1ptr[j-w+1] + i1ptr[j-1] + i1ptr[j] + i1ptr[j+1] + i1ptr[j+w-1] + i1ptr[j+w] + i1ptr[j+w+1])/9.0f;
			optr[j] = (val < 0)? 0: val;
		}
	}
}

static void convertMask(IplImage* iframe, IplImage *oframe, unsigned char specVal)
{
	for(int i = 0; i < iframe->height; i++)
	{
		unsigned char *iptr = (unsigned char *) (iframe->imageData + i*iframe->widthStep);
		float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		for(int j = 0; j < iframe->width; j++)
			optr[j] = (iptr[j] != specVal && iptr[j] > 0)? (float) iptr[j] :-1000000;
	}
}

static void maskdilation_filter(IplImage *in, IplImage *tempData, CvSize sz)
{
	int yoffset = sz.height >> 1;

	int *tptr = (int *) (tempData->imageData + yoffset*tempData->widthStep);
	memset(tptr, 0, tempData->width);

	for(int i=0;i<sz.height;i++)
	{
		int *iptr = (int *) (in->imageData + i*in->widthStep);

		for(int j=0;j<in->width>>2;j++)
			tptr[j] += iptr[j];
	}

	// vertical filtering
	for(int i=1;i<in->height-sz.height;i++)
	{
		int *iptr = (int *) (in->imageData + (i-1)*in->widthStep);
		int *iptr_ = (int *) (in->imageData + (i+sz.height)*in->widthStep);
		int *tptr = (int *) (tempData->imageData + (i+yoffset)*tempData->widthStep);
		int *_tptr = (int *) (tempData->imageData + (i+yoffset-1)*tempData->widthStep);

		for(int j=0;j<in->width>>2;j++)
		{
			tptr[j] = (_tptr[j] - iptr[j]) + iptr_[j];
			_tptr[j] = ((_tptr[j] + 0x01010101)>>1) & 0x7F7F7F7F;
		}
	}

	int xoffset = sz.width >> 1;
	for(int i=0;i<in->height-sz.height;i++)
	{
		unsigned char *tptr = (unsigned char *) (tempData->imageData + (i+yoffset)*tempData->widthStep);
		unsigned char *mptr = (unsigned char *) (in->imageData + (i+yoffset)*in->widthStep);

		unsigned char val = 0;
		for(int j=0;j<sz.width;j++)
			val += tptr[j];

		mptr[xoffset] = val;

		for(int j=1;j<in->width-sz.width;j++)
			mptr[xoffset+j] = (mptr[xoffset+j-1] - tptr[j-1]) + tptr[j+sz.width];
	}
	// horizontal filtering

}

static double convertMaskErosion(IplImage* iframe, IplImage *oframe, IplImage *temp, unsigned char specVal,IplImage* maskImage)
{

	//laplacian for specularity not relible that y specularity needs to masked

	for(int i = 0; i < iframe->height; i++)
	{
		unsigned char *iptr = (unsigned char *) (iframe->imageData + i*iframe->widthStep);
		unsigned char *msptr = (unsigned char *) (maskImage->imageData + i*maskImage->widthStep);
	//	float *optr = (float *) (temp->imageData + i*temp->widthStep);
	//    int  *optr = (int *) (temp->imageData + i*temp->widthStep);
		for(int j = 0; j < iframe->width; j++)
		{
		//	optr[j] = (iptr[j] != specVal && iptr[j] > 0)? (int) iptr[j] :-1000000;
			msptr[j]= ((iptr[j] != specVal)&&(iptr[j] != 0))?0:1;

		}
	}

	//cvSaveImage("in.pgm",iframe);

	//cvErode(temp, oframe, 0, 16);//erosion
//cvSaveImage("bfrdilate.pgm",maskImage);
//clock_t t,t1;
#if 0
t = clock();

cvSmooth(maskImage,maskImage,CV_BLUR,33,33,0,0);
//cvDilate(maskImage, maskImage, 0, 16);//erosion
t1 = clock();
float timea = ((float)(t1-t))/CLOCKS_PER_SEC;
//fprintf(fp,"%f \n",timea);
printf("time dilate %f \n",timea);
#else

//t = clock();
//maskdilation_filter(maskImage, oframe, cvSize(33,33));
maskdilation_filter(maskImage, temp, cvSize(33,33));
//t1 = clock();
//float timea = ((float)(t1-t))/CLOCKS_PER_SEC;
//fprintf(fp,"%f \n",timea);
//printf("time dilate %f \n",timea);
#endif
// cvThreshold(maskImage, maskImage, 0, 255, CV_THRESH_BINARY);
// cvSaveImage("dilate.pgm",maskImage);
  //  	cvSaveImage("interinnew.pgm",temp);
  //  cvSaveImage("interoutnew.pgm",oframe);

	for(int i = 0; i < iframe->height; i++)
	{

		//int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
		//int *tptr = (int *) (temp->imageData + i*temp->widthStep);
		unsigned short *optr = (unsigned short *) (oframe->imageData + i*oframe->widthStep);
		//int *tptr = (int *) (temp->imageData + i*temp->widthStep);
        unsigned char *ibtr = (unsigned char *) (iframe->imageData + i*iframe->widthStep);
		for(int j = 0; j < iframe->width; j++)
		{
	
			//if(optr[j] >= 0)
			//{
				optr[j] = (unsigned short)ibtr[j];
			//}
		}
	}
	return 1.0;
}

/*static void VerticalGaussianBlur(IplImage* iframe, IplImage *oframe, CvPoint border)
{
	int w = (iframe->widthStep/sizeof(int));

	for(int i = border.y; i < iframe->height-border.y; i++)
	{
		//float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
		//float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		int *iptr = (int *) (iframe->imageData + i*iframe->widthStep);
		int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
		for(int j = 0; j < iframe->width; j++)
		{
			
		   optr[j] = iptr[j-2*w] + iptr[j-w]*4 + (iptr[j]*6) + iptr[j+w]*4 + iptr[j+2*w];
		}
	}
}*/


/*static void VerticalGaussianBlur2(IplImage* iframe, IplImage *oframe, CvPoint border)
{
	int w = (iframe->widthStep/sizeof(int));

	for(int i = border.y; i < iframe->height-border.y; i++)
	{
		//float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
		//float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		int *iptr = (int *) (iframe->imageData + i*iframe->widthStep);
		int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
		VerticalGaussianBlur2		for(int j = 0; j < iframe->width; j++)
		{
			
		   optr[j] = (iptr[j-2*w] + iptr[j-w]*4 + (iptr[j]*6) + iptr[j+w]*4 + iptr[j+2*w]);
		}
		

	}
}*/

static void VerticalGaussianBlur2(IplImage* iframe, IplImage *oframe, CvPoint border)
{
	/*int w = (iframe->widthStep/sizeof(int));
	int w4 =4*w;
	int w3 =3*w;
	int w2 =2*w;

	for(int i = 4; i < iframe->height-4; i++)
	{
		//float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
		//float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
		int *iptr = (int *) (iframe->imageData + i*iframe->widthStep);
		int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
		for(int j = 0; j < iframe->width; j++)
		{

		//   optr[j] = (iptr[j-2*w] + iptr[j-w]*4 + (iptr[j]*6) + iptr[j+w]*4 + iptr[j+2*w]);
		  // optr[j] = iptr[j-4*w]+iptr[j-3*w]*8+iptr[j-2*w]*28 + iptr[j-w]*56 + iptr[j]*70 + iptr[j+w]*56 + iptr[j+2*w]*28+iptr[j+3*w]*8+iptr[j+4*w];
			 optr[j] = iptr[j-w4]+iptr[j-w3]*8+iptr[j-w2]*28 + iptr[j-w]*56 + iptr[j]*70 + iptr[j+w]*56 + iptr[j+w2]*28+iptr[j+w3]*8+iptr[j+w4];
		}


	}*/

	 int w = (iframe->widthStep/sizeof(int));
	int w_4 = 4*w;
	int w_3 = 3*w;
	int w_2 = 2*w;


	   //memset(iframe->imageData,255,iframe->widthStep*iframe->height);
	   int temp;
	   int count;
	for(int i = 4; i < iframe->height-4; i++)
	{
	//float *iptr = (float *) (iframe->imageData + i*iframe->widthStep);
	//float *optr = (float *) (oframe->imageData + i*oframe->widthStep);
	//unsigned short *iptr = (unsigned short *) (iframe->imageData + i*iframe->widthStep);
	int *iptr = (int *) (iframe->imageData + i*iframe->widthStep);
	int *optr = (int *) (oframe->imageData + i*oframe->widthStep);
	count = 0;
	for(int j = 0; j < (iframe->width>>1); j++)
	{
	//   optr[j] = (iptr[j-2*w] + iptr[j-w]*4 + (iptr[j]*6) + iptr[j+w]*4 + iptr[j+2*w]);
	//   optr[j] = iptr[j-4*w]+iptr[j-3*w]*8+iptr[j-2*w]*28 + iptr[j-w]*56 + iptr[j]*70 + iptr[j+w]*56 + iptr[j+2*w]*28+iptr[j+3*w]*8+iptr[j+4*w];
	   temp = (iptr[j-w_4]+((iptr[j-w_3]+iptr[j+w_3])<<3)+(iptr[j-w_2]+iptr[j+w_2])*28 + (iptr[j-w]+iptr[j+w])*56 + iptr[j]*70  + iptr[j+w_4]);
	   optr[count++] = (temp&0xFFFF);
	   optr[count++] = (temp>>16)&0xFFFF;
	}
 }
}
static void VerticalGaussianBlur(IplImage* iframe, IplImage *oframe, CvPoint border, CvRect rect)
{

int w = (iframe->widthStep/sizeof(int));
int w2 = w*2;
int w3 = w*3;
int w4 = w*4;
for(int i = 0; i < rect.height; i++)
{
int *iptr = (int *) (iframe->imageData + (i+rect.y)*iframe->widthStep) + rect.x;
int *optr = (int *) (oframe->imageData + (i+rect.y)*oframe->widthStep) + rect.x;

for(int k=0;k<rect.width;k++)
{
//optr[j] = iptr[j-w2] + ((iptr[j-w]+ iptr[j+w])<<2) + (iptr[j]*6) + iptr[j+w2];
optr[k] = ((iptr[k-w4] + iptr[k+w4]) + (iptr[k-w3] + iptr[k+w3])*8 + (iptr[k-w2] + iptr[k+w2])*28 + (iptr[k-w] + iptr[k+w])*56 + iptr[k]*70);
}
}
}



static CvRect GetROI(IplImage *img)
{

	int startRow = 0, startCol=0, endRow = img->height-1, endCol=img->width-1;
	int *csum = (int *) calloc(img->width, sizeof(int));
	int *rsum = (int *) calloc(img->height, sizeof(int));

	for(int i=1;i<img->height-1;i++)
	{
		unsigned char *iptr = (unsigned char *) img->imageData + img->widthStep*i;
		for(int j=1;j<img->width-1;j++)
		{
			csum[j] += iptr[j];
			rsum[i] += iptr[j];
		}
	}

	unsigned char *iptr = (unsigned char *) img->imageData + img->widthStep;
	int val = (img->width-2)*iptr[1];

	for(int i=1;i<img->height-1;i++)
	{
		if(val != rsum[i])
		{
			startRow = i-1;
			break;
		}
	}

	iptr = (unsigned char *) img->imageData + img->widthStep*(img->height-2);
	val = (img->width-2)*iptr[1];

	for(int i=img->height-2;i>0;i--)
	{
		if(val != rsum[i])
		{
			endRow = i+1;
			break;
		}
	}

	iptr = (unsigned char *) img->imageData + img->widthStep;
	val = (img->height-2)*iptr[1];

	for(int i=1;i<img->width-1;i++)
	{
		if(val != csum[i])
		{
			startCol = i-1;
			break;
		}
	}

	val = (img->height-2)*iptr[1];

	for(int i=img->width-2;i>0;i--)
	{
		if(val != csum[i])
		{
			endCol = i+1;
			break;
		}
	}

	free(csum);
	free(rsum);	
	
	return cvRect(startCol, startRow, endCol-startCol+1, endRow-startRow+1);

}



static CvRect intersectROI(CvRect a, CvRect b)
{
	a.x += 10;
	a.y += 10;
	a.width -= 20;
	a.height -= 20;

	int x = (a.x > b.x)? a.x: b.x;
	int y = (a.y > b.y)? a.y: b.y;
	int xe = (a.x+a.width < b.x+b.width)? a.x+a.width-1: b.x+b.width-1;
	int ye = (a.y+a.height < b.y+b.height)? a.y+a.height-1: b.y+b.height-1;

	return cvRect(x,y, xe-x+1, ye-y+1);
}

static CvPoint3D32f avgMaskRegression(IplImage* i1frame, IplImage *i2frame, double var, CvRect rect, IplImage *img,IplImage *maskImage)
{

	int w = (i1frame->widthStep/sizeof(int));

	double sx = 0, sy = 0;
	int count = 0;
	int64 sx_64=0;
	int64 sy_64=0;

	for(int i = 0; i < rect.height; i++)
	{
		int *i1ptr = (int *) (i1frame->imageData + (i+rect.y)*i1frame->widthStep) + rect.x;
		int *i2ptr = (int *) (i2frame->imageData + (i+rect.y)*i2frame->widthStep) + rect.x;
		unsigned char *msptr = (unsigned char *) (maskImage->imageData + (i+rect.y)*maskImage->widthStep) + rect.x;

		int sx_32 = 0, sy_32 = 0;
		for(int j = 0; j < rect.width; j++)
		{
			if(msptr[j] == 0)
			{
				int val1 = (i1ptr[j-w-1] + i1ptr[j-w] + i1ptr[j-w+1] + i1ptr[j-1] + i1ptr[j] + i1ptr[j+1] + i1ptr[j+w-1] + i1ptr[j+w] + i1ptr[j+w+1]);
				int val2 = (i2ptr[j-w-1] + i2ptr[j-w] + i2ptr[j-w+1] + i2ptr[j-1] + i2ptr[j] + i2ptr[j+1] + i2ptr[j+w-1] + i2ptr[j+w] + i2ptr[j+w+1]);

				i1ptr[j-2*w] = (val1 + 16)>>5;
				i2ptr[j-2*w] = (val2 + 16)>>5;

				if(val1 < 73728 && val2 < 73728 && val1 - val2 + 1728 > 0)	// replace 0.75*256*9 by 1728
				{
					//sx += (double) val1;
					//sy += (double) val2;
					sx_32 += val1;
					sy_32 += val2;
					count++;	
				}
				else
					msptr[j] = 255;
			}
		}

		sx_64 += (int64) sx_32;
		sy_64 += (int64) sy_32;
	}

	int CNT =count;

	//mean L0 L1 value
	sx = (double)sx_64;
	sy = (double)sy_64;

	sx /= count;
	sy /= count;

	int sx_32 = cvRound(sx/32.0);
	int sy_32 = cvRound(sy/32.0);

	int64 xx_64 = 0, yy_64 = 0, xy_64 = 0;

	count = 0;
	//next second level moments
	for(int i = 0; i < rect.height; i++)
	{
		unsigned char *mnptr = (unsigned char *) (maskImage->imageData + (i+rect.y)*maskImage->widthStep) + rect.x;
		int *i1ptr = (int *) (i1frame->imageData + (i+rect.y-2)*i1frame->widthStep) + rect.x;
		int *i2ptr = (int *) (i2frame->imageData + (i+rect.y-2)*i2frame->widthStep) + rect.x;

		int xx_inter=0;
		int xy_inter=0;
		int yy_inter=0;

		for(int j = 0; j < rect.width; j++)
		{
			if(mnptr [j] == 0)
			{
				int val1 = (i1ptr[j]);
				int val2 = (i2ptr[j]);

//				if(val1 - val2 + 1728 > 0) // replace 0.75*256*9 by 442368
				{
					int x_64 = (val1 - sx_32);
					int y_64 = (val2 - sy_32);

					xx_inter += x_64*x_64;
					yy_inter += y_64*y_64;
					xy_inter += x_64*y_64;

					count++;
				}
			}
		}

#if 0
		xx_inter = (xx_inter + 8) >> 4;
		xy_inter = (xy_inter + 8) >> 4;
		yy_inter = (yy_inter + 8) >> 4;
#endif
		xx_64 += xx_inter;
		xy_64 += xy_inter;
		yy_64 += yy_inter;


	}

	//xx /= (count);
	//yy /= (count);
	//xy /= (count);
	double xx = (double)xx_64 /(count);
	double yy = (double)yy_64 /(count);
	double xy = (double)xy_64 /(count);


	double delta = sqrt((xx-yy)*(xx-yy) + 4*xy*xy);
	double l1 = ((xx + yy) + delta)*0.5;//eighen value of major axis
	double l2 = ((xx + yy) - delta)*0.5;//eighen value of minor axis

	double sl1 = (l1 - xx)/xy;//orientation of major  axis
	double sl2 = (l2 - xx)/xy;//orientation of minor axis


	return cvPoint3D32f(sl1, l1, l2/(l1+l2));

}



static CvPoint3D32f avgMaskRegression2(IplImage* i1frame, IplImage *i2frame, double var, CvRect rect, IplImage *img,IplImage *maskImage)
{

	int w = (i1frame->widthStep/sizeof(int));
	int w_2 = 2*w;

	double sx = 0, sy = 0;
	int count = 0;
	int64 sx_64=0;
	int64 sy_64=0;

	for(int i=1;i>=-1;i--)
	{
		int *iptr = (int *) (i1frame->imageData + (rect.y-i)*i1frame->widthStep);
		int *_i1ptr = (int *) (i1frame->imageData + (rect.y-i-1)*i1frame->widthStep);
		int *_i2ptr = (int *) (i2frame->imageData + (rect.y-i-1)*i2frame->widthStep);
		for(int j=4;j<i1frame->width-4;j++)
		{
			int l1 = (iptr[j-2] + iptr[j-1]*4 - iptr[j]*10 + iptr[j+1]*4 + iptr[j+2]);
			int l2 = (iptr[j-4] + iptr[j-3]*8 + iptr[j-2]*12 - iptr[j-1]*8 - iptr[j]*26 - iptr[j+1]*8 + iptr[j+2]*12 + iptr[j+3]*8 + iptr[j+4]);
			int m1 = l1 >> 31;
			int m2 = l2 >> 31;
			_i1ptr[j] = (((l1 + m1)^m1) + 8)>>4;
			_i2ptr[j] = (((l2 + m2)^m2) + 128)>>8;
		}
	}

	for(int i = 0; i < rect.height; i++)
	{
		int *i1ptr = (int *) (i1frame->imageData + (i+rect.y-1)*i1frame->widthStep) + rect.x;
		int *i2ptr = (int *) (i2frame->imageData + (i+rect.y-1)*i2frame->widthStep) + rect.x;
		int *iptr = (int *) (i1frame->imageData + (i+rect.y+2)*i1frame->widthStep) + rect.x;

		unsigned char *msptr = (unsigned char *) (maskImage->imageData + (i+rect.y)*maskImage->widthStep) + rect.x;

		for(int j=-1;j<rect.width+1;j++)
		{
			int l1 = (iptr[j-2] + iptr[j-1]*4 - iptr[j]*10 + iptr[j+1]*4 + iptr[j+2]);
			int l2 = (iptr[j-4] + iptr[j-3]*8 + iptr[j-2]*12 - iptr[j-1]*8 - iptr[j]*26 - iptr[j+1]*8 + iptr[j+2]*12 + iptr[j+3]*8 + iptr[j+4]);
			int m1 = l1 >> 31;
			int m2 = l2 >> 31;
			i1ptr[j+w_2] = (((l1 + m1)^m1) + 8)>>4;
			i2ptr[j+w_2] = (((l2 + m2)^m2) + 128)>>8;
		}

		int sx_32 = 0, sy_32 = 0;
		for(int j = 0; j < rect.width; j++)
		{
			if(msptr[j] == 0)
			{
				int val1 = (i1ptr[j-w-1] + i1ptr[j-w] + i1ptr[j-w+1] + i1ptr[j-1] + i1ptr[j] + i1ptr[j+1] + i1ptr[j+w-1] + i1ptr[j+w] + i1ptr[j+w+1]);
				int val2 = (i2ptr[j-w-1] + i2ptr[j-w] + i2ptr[j-w+1] + i2ptr[j-1] + i2ptr[j] + i2ptr[j+1] + i2ptr[j+w-1] + i2ptr[j+w] + i2ptr[j+w+1]);

				i1ptr[j-w_2] = (val1 + 16)>>5;
				i2ptr[j-w_2] = (val2 + 16)>>5;

				if(val1 < 73728 && val2 < 73728 && val1 - val2 + 1728 > 0)	// replace 0.75*256*9 by 1728
				{
					//sx += (double) val1;
					//sy += (double) val2;
					sx_32 += val1;
					sy_32 += val2;
					count++;	
				}
				else
					msptr[j] = 255;
			}
		}

		sx_64 += (int64) sx_32;
		sy_64 += (int64) sy_32;
	}

	int CNT =count;

	//mean L0 L1 value
	sx = (double)sx_64;
	sy = (double)sy_64;

	sx /= count;
	sy /= count;

	int sx_32 = cvRound(sx/32.0);
	int sy_32 = cvRound(sy/32.0);

	int64 xx_64 = 0, yy_64 = 0, xy_64 = 0;

	count = 0;
	//next second level moments
	for(int i = 0; i < rect.height; i++)
	{
		unsigned char *mnptr = (unsigned char *) (maskImage->imageData + (i+rect.y)*maskImage->widthStep) + rect.x;
		int *i1ptr = (int *) (i1frame->imageData + (i+rect.y-3)*i1frame->widthStep) + rect.x;
		int *i2ptr = (int *) (i2frame->imageData + (i+rect.y-3)*i2frame->widthStep) + rect.x;

		int xx_inter=0;
		int xy_inter=0;
		int yy_inter=0;

		for(int j = 0; j < rect.width; j++)
		{
			if(mnptr [j] == 0)
			{
				int val1 = (i1ptr[j]);
				int val2 = (i2ptr[j]);

//				if(val1 - val2 + 1728 > 0) // replace 0.75*256*9 by 442368
				{
					int x_64 = (val1 - sx_32);
					int y_64 = (val2 - sy_32);

					xx_inter += x_64*x_64;
					yy_inter += y_64*y_64;
					xy_inter += x_64*y_64;

					count++;
				}
			}
		}

#if 0
		xx_inter = (xx_inter + 8) >> 4;
		xy_inter = (xy_inter + 8) >> 4;
		yy_inter = (yy_inter + 8) >> 4;
#endif
		xx_64 += xx_inter;
		xy_64 += xy_inter;
		yy_64 += yy_inter;


	}

	//xx /= (count);
	//yy /= (count);
	//xy /= (count);
	double xx = (double)xx_64 /(count);
	double yy = (double)yy_64 /(count);
	double xy = (double)xy_64 /(count);


	double delta = sqrt((xx-yy)*(xx-yy) + 4*xy*xy);
	double l1 = ((xx + yy) + delta)*0.5;//eighen value of major axis
	double l2 = ((xx + yy) - delta)*0.5;//eighen value of minor axis

	double sl1 = (l1 - xx)/xy;//orientation of major  axis
	double sl2 = (l2 - xx)/xy;//orientation of minor axis


	return cvPoint3D32f(sl1, l1, l2/(l1+l2));

}



CvPoint3D32f LaplacianBasedFocusDetector::ComputeRegressionFocus(IplImage *img, unsigned char specVal)
{
	CvPoint border = {2,2};
	
	CvRect imgRect = GetROI(img);

	//CvRect rect = {80, 60, 480, 360};//region of interest remove black edge
	
   CvRect rect = {0, 0, 320, 240};
	rect = intersectROI(imgRect, rect);
	//rect =imgRect;
	//double var ;
//	convertMask(img, pyrImage4, specVal);
	//TIME_OP("MASK erosion",
	double var = convertMaskErosion(img, pyrImage2, pyrImage1, specVal,maskImage);//); //convert image to floating point 2.any thing specular spot is masked..
	//printf("%f ", var);
//	TIME_OP("gaussia blur",
	VerticalGaussianBlur2(pyrImage2, pyrImage1, border);//);	// reduce image noise

    
//	VerticalGaussianBlur(pyrImage2, pyrImage1, border,rect););	// reduce image noise

	
//	clock_t t,t1;
	CvPoint3D32f pt;
	//t = clock();

#if 0
	cvCopy(pyrImage1, pyrImage2);
	cvCopy(pyrImage1, pyrImage3);
	cvCopy(pyrImage1, pyrImage4);

	GaussianBlur(pyrImage1, pyrImage2, border);//horizontal blur
	GaussianBlur(pyrImage2, pyrImage3, border);  // 3 -> 4 ////horizontal blur
    


	absDiffMask(pyrImage1, pyrImage2, pyrImage1,-1000000);
	absDiffMask(pyrImage2, pyrImage3, pyrImage4,-1000000);	// L1

	for(int i=0;i<1;i++)
		pt = avgMaskRegression(pyrImage1, pyrImage4, var, rect, img, maskImage);
#else
	//TIME_OP("regression",
	for(int i=0;i<1;i++)
		pt = avgMaskRegression2(pyrImage1, pyrImage4, var, rect, img, maskImage);//);
#endif
	//t1 = clock();
	//printf("Timing is %f (%f %f %f)\n", ((float)(t1-t))/CLOCKS_PER_SEC, pt.x, pt.y, pt.z);

	return pt;

}

