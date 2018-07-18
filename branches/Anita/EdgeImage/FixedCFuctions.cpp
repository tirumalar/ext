/*
 * FixedCFuctions.cpp
 *
 *  Created on: 07-Dec-2010
 *      Author: madhav.shanbhag@mamigo.us
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <list>
#include <map>
#include "image.h"
#include <assert.h>
// #include "EyeSegmentServer_fixed.h"

extern "C" {
#include "test_fw.h"
#include "file_manip.h"
#include "EdgeImage_private.h"
}

#ifdef __ARM__
#include <arm_neon.h>
#endif

// extern void printHexBytes(char * fileName , int lineNo, void * ptr);

#define Q15 (1.0*(1<<15))

#define MAX2(a,b) (a)>(b)?(a):(b)
#define MAX3(a,b,c) (MAX2(MAX2((a),(b)),(c)))
#define MIN2(a,b) (a)<(b)?(a):(b)
#define MIN3(a,b,c) (MIN2(MIN2((a),(b)),(c)))
#ifndef MIN
	#define MIN MIN2
#endif
#define CONTENT(ptr,step,i,j) (*((unsigned int*)ptr + j*step +i))

#ifndef __BFIN__
int Gauss4xBuffer[2592*4];
void dilate_i(unsigned int *ptr,unsigned int *output,unsigned int *params)
{
	int width,height,inpstep,outstep;
	int i,j;
	unsigned int *buff0,*buff1,*rowc,*rowc_p1,*temp,*buff,*maxbuff0,*maxbuff1,*maxbuff2;

	width = params[0];
	height = params[1];
	inpstep = params[2]>>2;
	outstep = params[3]>>2;
	buff = (unsigned int*)params[4];
	maxbuff0 = buff+width*2;
	maxbuff1 = maxbuff0+width;
	maxbuff2 = maxbuff1+width;

	buff0 = &buff[0];
	buff1 = &buff[width];

// row 0 Max values
	j = 0;
	maxbuff0[0] = MAX2(*(ptr+j*width+0),*(ptr+j*width+1));
	for(i=1;i<width-1;i++)
	{
		maxbuff0[i] = MAX3(*(ptr+j*width+i-1),*(ptr+j*width+i),*(ptr+j*width+i+1)); // MAX(-1,0)
	}
	maxbuff0[width-1] = MAX2(*(ptr+j*width+i-1),*(ptr+j*width+i));

// row 1 Max values
	j = 1;
	maxbuff1[0] = MAX2(*(ptr+j*inpstep+0),*(ptr+j*inpstep+1));
	for(i=1;i<width-1;i++)
	{
		maxbuff1[i] = MAX3(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i),*(ptr+j*inpstep+i+1)); // MAX(-1,0)
	}
	maxbuff1[width-1] = MAX2(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i));

// Max between row0 and row1
// output first row
	for(i=0;i<width;i++)
	{
		//write the output
		output[i] = MAX2(maxbuff0[i],maxbuff1[i]);
		buff0[i]= output[i];
	}

	for(j=2;j<height;j++)
	{
		// row 2 Max values
		maxbuff2[0] = MAX2(*(ptr+j*inpstep+0),*(ptr+j*inpstep+1));
		for(i=1;i<width-1;i++)
		{
			maxbuff2[i] = MAX3(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i),*(ptr+j*inpstep+i+1));
		}
		maxbuff2[width-1] = MAX2(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i));

		for(i=0;i<width;i++)
		{
			//write the output
			output[(j-1)*outstep + i] = MAX2(buff0[i],maxbuff2[i]);
			// Max between row1 and row2
			buff1[i] = MAX2(maxbuff1[i],maxbuff2[i]);
		}

		temp = buff0;
		buff0 = buff1;
		buff1 = temp;

		temp = maxbuff0;
		maxbuff0 = maxbuff1;
		maxbuff1 = maxbuff2;
		maxbuff2 = temp;
	}

	for(i=0;i<width;i++)
	{
		//write the output
		output[(j-1)*outstep + i] = buff0[i];
	}
}


short pdilate_i_3(int **inputs, Point2D16i *output, int *params)
{
	unsigned int *pa,*pb,*pc,*pd;
	short int stepa,stepb,stepc,stepd;
	short int width,height,i,j,k =0;
	Point2D16i temp;

	pa = (unsigned int*)inputs[0];
	pb = (unsigned int*)inputs[1];
	pc = (unsigned int*)inputs[2];
	pd = (unsigned int*)inputs[3];

	width = params[0];
	height = params[1];

	stepa = params[2]>>2;
	stepb = params[3]>>2;
	stepc = params[4]>>2;
	stepd = params[5]>>2;

	int max  = params[6];//MAX
	int *outscore = (int*)params[7];
	int x_off = params[8];//inp x offset
	int y_off = params[9];//inp y offset

	for(j =0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			unsigned int val = CONTENT(pd,stepd,i,j);
			if((MAX3(CONTENT(pa,stepa,i,j),CONTENT(pb,stepb,i,j),CONTENT(pc,stepc,i,j))==val)
					&&(val!=0))
			{
				temp.x = i+x_off;
				temp.y = j+y_off;
				*output++ = temp;
				*outscore++ = val;
				k++;
			}
		}
		if(!(k+width < max)){
			k = -1;
			break;
		}
	}
	return(k);
}

short pdilate_i_2(int **inputs, Point2D16i *output, int *params)
{
	unsigned int *pa,*pb,*pd;
	short int stepa,stepb,stepd;
	short int width,height,i,j,k =0;
	Point2D16i temp;

	pa = (unsigned int *)inputs[0];
	pb = (unsigned int *)inputs[1];
	pd = (unsigned int *)inputs[2];

	width = params[0];
	height = params[1];

	stepa = params[2]>>2;
	stepb = params[3]>>2;
	stepd = params[4]>>2;

	int max  = params[5];//MAX
	int *outscore = (int*)params[6];
	int x_off = params[7];//inp x offset
	int y_off = params[8];//inp y offset

	for(j =0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			unsigned int val = CONTENT(pd,stepd,i,j);
			if(((MAX2(CONTENT(pa,stepa,i,j),CONTENT(pb,stepb,i,j)))==val)
					&&(val!=0))
			{
				temp.x = i+x_off;
				temp.y = j+y_off;
				*output++ = temp;
				*outscore++ = val;
				k++;
			}
		}
		if(!(k+width < max)){
			k = -1;
			break;
		}
	}
	return(k);
}

void FindMax(unsigned int *ptr,int* params){

	unsigned int *pa;
	short int stepa;
	int width,height,i,j,max_x=-1,max_y=-1;

	pa = (unsigned int *)ptr;

	width = params[0];
	height = params[1];
	stepa = params[2]>>2;

	unsigned int maxval = CONTENT(pa,stepa,0,0);

	for(j =0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			unsigned int val = CONTENT(pa,stepa,i,j);
			if(MAX2(val,maxval)> maxval)
			{
				max_x = i;
				max_y = j;
				maxval = val;
			}
		}
	}

	params[3] = max_x;
	params[4] = max_y;
	params[5] = maxval;

}

void compute_erode_image(unsigned char * ptr,unsigned char *output,int *params){
	int width,height,inpstep,outstep;
	int i,j;
	unsigned int *buff0,*buff1,*rowc,*rowc_p1,*temp,*buff,*maxbuff0,*maxbuff1,*maxbuff2;

	width = params[0];
	height = params[1];
	inpstep = params[2];
	outstep = params[3];
	buff = (unsigned int*)params[4];
	maxbuff0 = buff+width*2;
	maxbuff1 = maxbuff0+width;
	maxbuff2 = maxbuff1+width;

	buff0 = &buff[0];
	buff1 = &buff[width];

// row 0 Max values
	j = 0;
	maxbuff0[0] = MIN2(*(ptr+j*width+0),*(ptr+j*width+1));
	for(i=1;i<width-1;i++)
	{
		maxbuff0[i] = MIN3(*(ptr+j*width+i-1),*(ptr+j*width+i),*(ptr+j*width+i+1)); // MAX(-1,0)
	}
	maxbuff0[width-1] = MIN2(*(ptr+j*width+i-1),*(ptr+j*width+i));

// row 1 Max values
	j = 1;
	maxbuff1[0] = MIN2(*(ptr+j*inpstep+0),*(ptr+j*inpstep+1));
	for(i=1;i<width-1;i++)
	{
		maxbuff1[i] = MIN3(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i),*(ptr+j*inpstep+i+1)); // MAX(-1,0)
	}
	maxbuff1[width-1] = MIN2(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i));

// Max between row0 and row1
// output first row
	for(i=0;i<width;i++)
	{
		//write the output
		output[i] = MIN2(maxbuff0[i],maxbuff1[i]);
		buff0[i]= output[i];
	}

	for(j=2;j<height;j++)
	{
		// row 2 Max values
		maxbuff2[0] = MIN2(*(ptr+j*inpstep+0),*(ptr+j*inpstep+1));
		for(i=1;i<width-1;i++)
		{
			maxbuff2[i] = MIN3(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i),*(ptr+j*inpstep+i+1));
		}
		maxbuff2[width-1] = MIN2(*(ptr+j*inpstep+i-1),*(ptr+j*inpstep+i));

		for(i=0;i<width;i++)
		{
			//write the output
			output[(j-1)*outstep + i] = MIN2(buff0[i],maxbuff2[i]);
			// Max between row1 and row2
			buff1[i] = MIN2(maxbuff1[i],maxbuff2[i]);
		}

		temp = buff0;
		buff0 = buff1;
		buff1 = temp;

		temp = maxbuff0;
		maxbuff0 = maxbuff1;
		maxbuff1 = maxbuff2;
		maxbuff2 = temp;
	}

	for(i=0;i<width;i++)
	{
		//write the output
		output[(j-1)*outstep + i] = buff0[i];
	}
}
void zoom_2x(unsigned char * in,unsigned char *out,int *params){


	int width,height,inpstep,outstep;
	int i,j;
	width = params[1];
	height = params[0];
	inpstep = params[2];
	outstep = params[3];
	unsigned char *row0,*row1,*rtemp;

	for(j=0;j<height;j++)
	{
		unsigned char *ptr = in +j* inpstep;
		row0 = out+j*2*outstep;
		rtemp = row0;
		row1 = row0+outstep;
		for(i=0;i<width;i++)
		{
			*row0++=*ptr;
			*row0++=*ptr++;
		}
		memcpy(row1,rtemp,outstep);
	}
}


void compute_polar_warping(ucImage *img, ucImage *output,unsigned int *param)
{
	Point3D_fix irc, puc;
	irc.x = param[0]&0xFFFF;
	puc.x = (param[0]&0xFFFF0000)>>16;
	irc.y = param[1]&0xFFFF;
	puc.y = (param[1]&0xFFFF0000)>>16;
	irc.z = param[2]&0xFFFF;
	puc.z = (param[2]&0xFFFF0000)>>16;

	unsigned int radint =param[3];
	unsigned int *sinCosTable = (unsigned int *)param[4];

	unsigned char *inptr = img->data, *outptr = output->data;
	int inStep = img->widthstep, outStep = output->widthstep;

	unsigned int w = img->width;
	unsigned int h = img->height;
	int i,j;

	unsigned int radloop = 0;

	for( i=0; i< output->height; i++)
	{
		unsigned short rad = radloop>>16;
		unsigned char *outC1 = outptr + i * outStep;

		//C0X = PUC.X+R*(IRC.X-PUC.X)
		int cox = irc.x-puc.x;
		cox = rad*cox;
		cox = (((int)puc.x)<<15) + cox +0x100;
		cox = cox>>9; //10q6

		//C0Y = PUC.Y+R*(IRC.Y-PUC.Y)
		int coy = irc.y-puc.y;
		coy = rad*coy;
		coy = (((int)puc.y)<<15) + coy +0x100;
		coy = coy>>9; //10q6

		//C0Z = PUC.Z+R*(IRC.Z-PUC.Z)
		int coz = irc.z-puc.z;
		coz = rad*coz;
		coz = (((int)puc.z)<<15) + coz +0x100;
		coz = coz>>9; //10q6

		for(j=0; j<output->width; j++, outC1++)
		{
			unsigned int temp2 = sinCosTable[j];
			short ca = 0;//CosTable[j];//cos(angle);//Q14
			short sa = 0;//SinTable[j];//sin(angle);//Q14
			ca = ((int)temp2>>16);
			sa = (((int)(temp2<<16)>>16));

			//A0 = CA*C0Z+C0X,A1 = SA*C0Z+C0Y

			int a0,a1;
			a0 = ca*coz;
			a0+= (cox<<14);
			a0+= (1<<10);
			a0 = a0>>12;
			a1 = sa*coz;
			a1+= (coy<<14);
			a1+= (1<<10);
			a1 = a1>>12;
			// use bilinear interpolation
			short intx = (a0>>8);
			short inty = (a1>>8);
			unsigned short yfrac = a1&0xff; //Q16
			unsigned char *fptr1= inptr + inty*inStep;
			unsigned char *fptr2= inptr + (inty+1)*inStep;
			if (((unsigned short) intx) < (w-1) && ((unsigned short) inty) < (h-1))
			{
				short xfrac = a0&0xff;
				int y1val= (0x100-xfrac)*fptr1[intx]+ xfrac*fptr1[intx+1];
				int y2val= (0x100-xfrac)*fptr2[intx]+ xfrac*fptr2[intx+1];
				int temp = (0x100-yfrac)*y1val + yfrac *y2val;
				temp += 1<<14;//add0.5
				*outC1 =  (temp)>>16;
			}
			else
			{
				*outC1 =  0;
			}
		}
		radloop += radint; //Q14
	}
}

void compute_polar_warping_NN(ucImage *img, ucImage *output,unsigned int *param){
	Point3D_fix irc, puc;
	irc.x = param[0]&0xFFFF;
	puc.x = (param[0]&0xFFFF0000)>>16;
	irc.y = param[1]&0xFFFF;
	puc.y = (param[1]&0xFFFF0000)>>16;
	irc.z = param[2]&0xFFFF;
	puc.z = (param[2]&0xFFFF0000)>>16;

	unsigned int radint = param[3];
	unsigned int *sinCosTable = (unsigned int *)param[4];

	unsigned char *inptr = img->data, *outptr = output->data;
	int inStep = img->widthstep, outStep = output->widthstep;

	unsigned int w = img->width;
	unsigned int h = img->height;
	int i,j;

	unsigned int radloop =0;

	for( i=0; i< output->height; i++)
	{
		unsigned short rad = radloop>>16;
		unsigned char *outC1 = outptr + i * outStep;

		//C0X = PUC.X+R*(IRC.X-PUC.X)
		int cox = irc.x-puc.x;
		cox = rad*cox;
		cox = (((int)puc.x)<<15) + cox +0x100;
		cox = cox>>9; //10q6

		//C0Y = PUC.Y+R*(IRC.Y-PUC.Y)
		int coy = irc.y-puc.y;
		coy = rad*coy;
		coy = (((int)puc.y)<<15) + coy +0x100;
		coy = coy>>9; //10q6

		//C0Z = PUC.Z+R*(IRC.Z-PUC.Z)
		int coz = irc.z-puc.z;
		coz = rad*coz;
		coz = (((int)puc.z)<<15) + coz+0x100;
		coz = coz>>9; //10q6

		for(j=0; j<output->width; j++, outC1++)
		{
			unsigned int temp2 = sinCosTable[j];
			short ca = 0;//CosTable[j];//cos(angle);//Q14
			short sa = 0;//SinTable[j];//sin(angle);//Q14
			ca = ((int)temp2>>16);
			sa = (((int)(temp2<<16)>>16));

			//A0 = CA*C0Z+C0X,A1 = SA*C0Z+C0Y

			int a0,a1;
			a0 = ca*coz;//q6*q14=q20
			a0+= (cox<<14);
			a0+=(1<<19);
			a0 = a0>>20;
			a1 = sa*coz;
			a1+= (coy<<14);
			a1+=(1<<19);
			a1 = a1>>20;
			// use bilinear interpolation
			short intx = (a0);
			short inty = (a1);
			if (((unsigned short) intx) < (w-1) && ((unsigned short) inty) < (h-1))
			{
				unsigned char *fptr1= inptr + inty*inStep+intx;
				*outC1 = *fptr1;
			}
			else
			{
				*outC1 =  0;
			}
		}
		radloop += radint; //Q14
	}
}
int compute_EigenVals(unsigned char *inptr,int *outptr,int* Param) // ,void * m_ccompServer)
{
	//printHexBytes(__FILE__,__LINE__, m_ccompServer);
	int width = Param[0];//input w
	int height = Param[1];//input h
	int inpstep = Param[2]; //inputstep
	int outstep= Param[3]>>2; //Actual outputstep for other int buffer //int buffers
	char *temp = (char *)Param[4]; //tempbuffer
	int k = Param[5]; //constant K

	int maxval = -1;
//	unsigned char *inprow1 = inptr-4; // 4 cols behind
	short *ix = (short*)temp;
	short *iy = ix +width;
	int *sqgx = (int*)(iy+width);
	int *sqgy = sqgx+width;
	int *gxgy = sqgy+width;

	int *sumsqgx0 = gxgy+width;
	int *sumsqgx1 = sumsqgx0+width;
	int *sumsqgx2 = sumsqgx1+width;
	int *sumsqgx3 = sumsqgx2+width;
	int *sumsqgx4 = sumsqgx3+width;

	int *sumsqgy0 = sumsqgx4+width;
	int *sumsqgy1 = sumsqgy0+width;
	int *sumsqgy2 = sumsqgy1+width;
	int *sumsqgy3 = sumsqgy2+width;
	int *sumsqgy4 = sumsqgy3+width;

	int *sumgxgy0 = sumsqgy4+width;
	int *sumgxgy1 = sumgxgy0+width;
	int *sumgxgy2 = sumgxgy1+width;
	int *sumgxgy3 = sumgxgy2+width;
	int *sumgxgy4 = sumgxgy3+width;

	int *outgx = sumgxgy4+width;
	int *outgy = outgx+width;
	int *outgxgy = outgy+width;

	unsigned char *row0,*row2,*row1;
	int *out;
//	printHexBytes(__FILE__,__LINE__, m_ccompServer);
	for(int i=0;i<height;i++){
		row1 = inptr + i*inpstep;
		row0 = inptr + i*inpstep - inpstep;
		row2 = inptr + i*inpstep + inpstep;

		//GENERATE_GY(INPROW0,INPROW2,IY,P0,KERNEL_GY)
		for(int j=0;j< width;j++){
			iy[j] = *row2++ - *row0++;
		}
		//GENERATE_GX(INPROW1,IX,P0,KERNEL_GX)
		for(int j=0;j< width;j++){
			ix[j] = row1[j+1] - row1[j-1];
		}
		//GENERATE_SQUARE(IX,IY,SQGXARRAY,SQGYARRAY,GXGYARRAY,P0,KERNEL_SQ)
		for(int j=0;j< width;j++){
			sqgx[j] = ix[j]*ix[j]; //9Q0*9Q0 = 18Q0 u
			sqgy[j] = iy[j]*iy[j]; //9Q0*9Q0 = 18Q0 u
			gxgy[j] = ix[j]*iy[j]; //9Q0*9Q0 = 18Q0 s
		}

		//SUM_HOR(SQGXARRAY,SUMSQGXARRAY4,P0,KERNEL_SUMSQGX)
		//SUM_HOR(SQGYARRAY,SUMSQGYARRAY4,P0,KERNEL_SUMSQGY)
		//SUM_HOR(GXGYARRAY,SUMGXGYARRAY4,P0,KERNEL_SUMGXGY)

		for(int j=0;j< width-4;j++){
			sumsqgx4[j+2] = sqgx[j]+sqgx[j+1]+sqgx[j+2]+sqgx[j+3]+sqgx[j+4]; //18Q0*5 = 21Q0 u
			sumsqgy4[j+2] = sqgy[j]+sqgy[j+1]+sqgy[j+2]+sqgy[j+3]+sqgy[j+4]; //18Q0*5 = 21Q0 u
			sumgxgy4[j+2] = gxgy[j]+gxgy[j+1]+gxgy[j+2]+gxgy[j+3]+gxgy[j+4]; //18Q0*5 = 21Q0 u
		}
		//Not written were 0 1 and -2 -1 directly replicate
		sumsqgx4[0] = sumsqgx4[1] = sumsqgx4[2];
		sumsqgx4[width -1] = sumsqgx4[width -2] = sumsqgx4[width -3];
		sumsqgy4[0] = sumsqgy4[1] = sumsqgy4[2];
		sumsqgy4[width -1] = sumsqgy4[width -2] = sumsqgy4[width -3];
		sumgxgy4[0] = sumgxgy4[1] = sumgxgy4[2];
		sumgxgy4[width -1] = sumgxgy4[width -2] = sumgxgy4[width -3];

		//SUM_5_ROWS(SUMSQGXARRAY0,SUMSQGXARRAY1,SUMSQGXARRAY2,SUMSQGXARRAY3,SUMSQGXARRAY4,OUTPUTGX ,P0,KERNEL_VSUMSQGX)
		//SUM_5_ROWS(SUMSQGYARRAY0,SUMSQGYARRAY1,SUMSQGYARRAY2,SUMSQGYARRAY3,SUMSQGYARRAY4,OUTPUTGY ,P0,KERNEL_VSUMSQGY)
		//SUM_5_ROWS(SUMGXGYARRAY0,SUMGXGYARRAY1,SUMGXGYARRAY2,SUMGXGYARRAY3,SUMGXGYARRAY4,OUTPUTGXGY ,P0,KERNEL_VSUMGXGY)
		for(int j=0;j< width;j++){
			outgx[j] = (sumsqgx0[j] +sumsqgx1[j]+ sumsqgx2[j] + sumsqgx3[j] + sumsqgx4[j]+32)>>6; //21Q0 >>6 = 15Q0
			outgy[j] = (sumsqgy0[j] +sumsqgy1[j]+ sumsqgy2[j] + sumsqgy3[j] + sumsqgy4[j]+32)>>6; //21Q0 >>6 = 15Q0
			outgxgy[j] = (sumgxgy0[j] +sumgxgy1[j]+ sumgxgy2[j] + sumgxgy3[j] + sumgxgy4[j]+32)>>6; //21Q0 >>6 = 15Q0
		}

		//EIGEN_GEN(OUTPUTGX,OUTPUTGY,OUTPUTGXGY,OUTPUT_LIST,MAXVAL,THRESHOLD,P0,EIGEN_LOOP)
		//ab-c*c-k*(a+b)*(a+b)
		//15q0*15q0 - 15q0*15q0 - 15Q0*15Q0*Q16
		out = outptr + i*outstep;
		for(int j=0;j< width;j++){
			int sum = (outgx[j]+outgy[j])>>1;
			int val = outgx[j]*outgy[j] - outgxgy[j]*outgxgy[j] - ((((k*sum)>>12)*sum)>>2);
			out[j] = val;
			if(val > maxval)
				maxval = val;
		}

		int *temp = sumsqgx0;
		sumsqgx0 = sumsqgx1;
		sumsqgx1 = sumsqgx2;
		sumsqgx2 = sumsqgx3;
		sumsqgx3 = sumsqgx4;
		sumsqgx4 = temp;

		temp = sumsqgy0;
		sumsqgy0 = sumsqgy1;
		sumsqgy1 = sumsqgy2;
		sumsqgy2 = sumsqgy3;
		sumsqgy3 = sumsqgy4;
		sumsqgy4 = temp;

		temp = sumgxgy0;
		sumgxgy0 = sumgxgy1;
		sumgxgy1 = sumgxgy2;
		sumgxgy2 = sumgxgy3;
		sumgxgy3 = sumgxgy4;
		sumgxgy4 = temp;
		//ROTATE_BUFFERS (SUMSQGXARRAY0,SUMSQGXARRAY1,SUMSQGXARRAY2,SUMSQGXARRAY3,SUMSQGXARRAY4)
		//ROTATE_BUFFERS (SUMSQGYARRAY0,SUMSQGYARRAY1,SUMSQGYARRAY2,SUMSQGYARRAY3,SUMSQGYARRAY4)
		//ROTATE_BUFFERS (SUMGXGYARRAY0,SUMGXGYARRAY1,SUMGXGYARRAY2,SUMGXGYARRAY3,SUMGXGYARRAY4)
	}
	//printHexBytes(__FILE__,__LINE__, m_ccompServer);
	return maxval;
}

void compute_EigenMask(int* inp,unsigned char *out,int* Param){

	int width = Param[0];
	int height = Param[1];
	int ws = Param[2]>>2;
	int maskstep = Param[3];
	int t_eigenTH = Param[4];

	for(int i=0;i<height;i++){
		int *ptr = inp + i*ws;
		unsigned char *outptr = out+ i*maskstep;
		for(int j=0;j<width;j++){
			*outptr++ = *ptr++ > t_eigenTH ? 0xFF:0;
		}
	}
}


void compute_correlate_gradient_image(usImage *ptrin, uiImage *ptrout,Pointparam *param){
	int i,j;
	unsigned short *gptr;
	unsigned char *iptr;
	unsigned int *optr;

	unsigned short *grad = ptrin->data;
	int inpws = ptrin->widthstep;

	unsigned int *out = ptrout->data;
	int outws = ptrout->widthstep;

	unsigned char *image = param->inpimage;
	int imagews = param->widthstep;

#ifdef __ARM__
	unsigned int buff[8]={0};
	uint16x4_t c0,c1,c2,c3;
	uint8x8_t one;
	buff[0]= 0x1010101;
	buff[1]= 0x0;
	one = vld1_u8((const uint8_t *)buff);

	c0 = vdup_n_u16((uint16_t)param->C00);
	c1 = vdup_n_u16((uint16_t)param->C01);
	c2 = vdup_n_u16((uint16_t)param->C10);
	c3 = vdup_n_u16((uint16_t)param->C11);

	uint32x4_t out1,out2;
	uint32x2_t temp,temp1;
	uint16x4_t inp1,inp2,inp3,inp4;
	uint8x8_t left,right,val;
	uint16x8_t val16x8;

	uint16x4_t val16x4,vals16x4;
	uint32x4_t val32x4,vals32x4;
#endif

	for(i=0;i<param->sy;i++)
	{
		gptr = grad + i*(inpws>>1);
		optr = out + i*(outws>>2);
		iptr = image + i*imagews;

#ifdef __ARM__

		int xcnt = ((param->sx>>2)<<2);
		int prologcnt = param->sx - xcnt;
		if(param->sx >= 4){
			for(j=0;j<xcnt;j+=4,iptr+=4, gptr+=4, optr+=4)
			{
				inp1 = vld1_u16((const uint16_t *)gptr); // a b c d
				inp2 = vld1_u16((const uint16_t *)(gptr+(inpws>>1))); // a1 b1 c1 d1
				out1 = vmull_u16(inp1,c0); //00*a
				out1 = vmlal_u16 (out1, inp2, c2);//10*a1

				inp3 = vld1_u16((const uint16_t *)gptr+1); // a b c d
				inp4 = vld1_u16((const uint16_t *)(gptr+(inpws>>1)+1)); // a1 b1 c1 d1
				out1 = vmlal_u16 (out1, inp3, c1);//01*a
				out1 = vmlal_u16 (out1, inp4, c3);//11*a1

				out2 = vshrq_n_u32 (out1,6);

				left = vld1_u8((const uint8_t *)iptr+ param->left_offset);
				right = vld1_u8((const uint8_t *)iptr+param->right_offset);
				val = vcgt_u8 (right, left);

				val16x8 = vmovl_u8 (val);
				val16x4 = vget_low_u16 (val16x8);
				vals16x4 = vshl_n_u16 (val16x4,8);
				val16x4 = vorr_u16(val16x4,vals16x4);
				val32x4 = vmovl_u16 (val16x4);
				//val32x4 = vmlaq_n_u32(val32x4, val32x4,0x10000);
				vals32x4 = vshlq_n_u32 (val32x4,16);
				val32x4 = vorrq_u32(val32x4,vals32x4);

				out1 = vld1q_u32((const uint32_t *)optr);
				out2 = vandq_u32(out2,val32x4);
				out1 = vaddq_u32(out1,out2);
				vst1q_u32((uint32_t *)optr,out1);
			}
		}
#else
		int prologcnt = param->sx;
#endif //#ifdef __ARM__

		for(j=0;j<prologcnt;j++,iptr++, gptr++, optr++)
		{
			unsigned int val = 0,val1 = 0,val2 = 0,val3=0;
			if(iptr[param->left_offset] < iptr[param->right_offset])
			{
				//9Q7*2Q14 = 11Q21
				val = param->C00 * gptr[0] ;
				val1 = param->C01 * gptr[1] ;
				val2 = param->C10 * gptr[inpws>>1];
				val3 = param->C11 * gptr[(inpws>>1)+1];
				val+=(val1+val2+val3);
				val = val >> 6; // no rounding required
			}
			*optr = (*optr) + val;
		}
	}
}

void compute_horizontal_gradient_image_mask(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:-ix;
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
	}
}


void compute_vertical_gradient_image_mask(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row = img + i*inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++)
		{

			short int ix = row[1] - row[-1];
			ix = ix>0 ?ix:-ix;
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
	}
}

void compute_magnitude_gradient_image_mask(unsigned char *img,unsigned short int *output,  int* dstParam){
	int i,j;
	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[6];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row =  img + i*inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:-ix;
			short int iy = row[1] - row[-1];
			iy = iy>0 ?iy:-iy;
			*out++ = *maskrow++ != 0 ? 0: lut[iy*256+ix];
		}
	}
}

void compute_horizontal_gradient_image_mask_64(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:-ix;
			ix = MIN(ix,64);
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
	}
}


void compute_vertical_gradient_image_mask_64(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row = img + i*inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++)
		{
			short int ix = row[1] - row[-1];
			ix = ix>0 ?ix:-ix;
			ix = MIN(ix,64);
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
	}
}

void compute_magnitude_gradient_image_mask_64(unsigned char *img,unsigned short int *output,  int* dstParam){
	int i,j;
	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[6];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row =  img + i*inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:-ix;
			ix = MIN(ix,64);
			short int iy = row[1] - row[-1];
			iy = iy>0 ?iy:-iy;
			iy = MIN(iy,64);
			*out++ = *maskrow++ != 0 ? 0: lut[iy*65+ix];
		}
	}
}

void compute_horizontal_gradient_image_mask_eyelid(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:0;
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
		out = output + i*(outstep>>1);
		out[0] = out[1];
		out[w-1] = out[w-2];
	}
}


void compute_vertical_gradient_image_mask_eyelid(unsigned char *img,unsigned short  *output,  int* dstParam)
{
	int i,j;

	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[4];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row = img + i*inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++){
			short int ix = row[1] - row[-1];
			ix = ix>0 ?ix:-ix;
			*out++ = *maskrow++ != 0 ? 0: lut[ix];
		}
		out = output + i*(outstep>>1);
		out[0] = out[1];
		out[w-1] = out[w-2];
	}
}

void compute_magnitude_gradient_image_mask_eyelid(unsigned char *img,unsigned short int *output,  int* dstParam){
	int i,j;
	int w = dstParam[0];
	int h = dstParam[1];
	int inpstep = dstParam[2];
	int outstep = dstParam[3];
	unsigned char *maskptr = (unsigned char *)dstParam[6];
	unsigned short *lut = (unsigned short *)dstParam[5];

	unsigned char *row1,*row3,*row,*maskrow;
	unsigned short *out;

	for(i=0;i<h;i++)
	{
		row1 = img + i*inpstep - inpstep;
		row =  img + i*inpstep;
		row3 = img + i*inpstep + inpstep;
		maskrow = maskptr + i*inpstep;
		out = output + i*(outstep>>1);

		for(j=0;j<w;j++,row++)
		{
			short int ix = *row3++ - *row1++;
			ix = ix>0 ?ix:0;
			short int iy = row[1] - row[-1];
			iy = iy>0 ?iy:-iy;
			*out++ = *maskrow++ != 0 ? 0: lut[iy*256+ix];
		}
		out = output + i*(outstep>>1);
		out[0] = out[1];
		out[w-1] = out[w-2];
	}
}


void compute_correlate_gradient_image_eyelid_C(usImage *ptrin, uiImage *ptrout,Pointparam *param){
	int i,j;
	unsigned short *gptr,*gptr_1;
	unsigned char *cptr;
	unsigned int *optr;

	unsigned short *grad = ptrin->data;
	int inpws = ptrin->widthstep;

	unsigned int *out = ptrout->data;
	int outws = ptrout->widthstep;

	unsigned char *cntBuff = (unsigned char*)param->left_offset;
	for(i=0;i<param->sy;i++)
	{
		gptr = grad + i*(inpws>>1);
		optr = out + i*(outws>>2);
		cptr = cntBuff + i*param->right_offset;

		int prologcnt = param->sx;

		for(j=0;j<prologcnt;j++,cptr++, gptr++, optr++)
		{
			unsigned int val = 0,val1 = 0,val2 = 0,val3=0;
			//9Q7*2Q14 = 11Q21
			val = param->C00 * gptr[0] ;
			val1 = param->C01 * gptr[1] ;
			val2 = param->C10 * gptr[inpws>>1];
			val3 = param->C11 * gptr[(inpws>>1)+1];
			val+=(val1+val2+val3);
			val = val >> 6; // no rounding required
			*optr = (*optr) + val;
			*cptr +=1;
		}
	}
}

void compute_correlate_gradient_image_eyelid(usImage *ptrin, uiImage *ptrout,Pointparam *param){
	int i,j;
	unsigned short *__restrict gptr,*__restrict gptr_1;
	unsigned char *__restrict cptr;
	unsigned int *__restrict optr;

	unsigned short *grad = ptrin->data;
	int inpws = ptrin->widthstep;

	unsigned int *out = ptrout->data;
	int outws = ptrout->widthstep;

#ifdef __ARM__
	unsigned int buff[8]={0};
	uint16x4_t c0,c1,c2,c3;
	uint8x8_t one;
	buff[0]= 0x1010101;
	buff[1]= 0x0;
	one = vld1_u8((const uint8_t *)buff);

	c0 = vdup_n_u16((uint16_t)param->C00);
	c1 = vdup_n_u16((uint16_t)param->C01);
	c2 = vdup_n_u16((uint16_t)param->C10);
	c3 = vdup_n_u16((uint16_t)param->C11);
	uint32x4_t out1,out2;
	uint16x4_t inp1,inp2,inp3,inp4;
	uint8x8_t val;
#endif

	unsigned char *__restrict cntBuff = (unsigned char*)param->left_offset;
	for(i=0;i<param->sy;i++)
	{
		gptr = grad + i*(inpws>>1);
		optr = out + i*(outws>>2);
		cptr = cntBuff + i*param->right_offset;
		gptr_1 = gptr+(inpws>>1);

#ifdef __ARM__
		int xcnt = ((param->sx>>2)<<2);
		int prologcnt = param->sx - xcnt;
		if(param->sx >= 4){
			for(j=0;j<xcnt;j+=4,cptr+=4, gptr+=4, optr+=4,gptr_1+=4)
			{
				inp1 = vld1_u16((const uint16_t *)gptr); // a b c d
				inp2 = vld1_u16((const uint16_t *)gptr_1); // a1 b1 c1 d1
				out1 = vmull_u16(inp1,c0); //00*a
				out1 = vmlal_u16 (out1, inp2, c2);//10*a1

				inp3 = vld1_u16((const uint16_t *)gptr+1); // a b c d
				inp4 = vld1_u16((const uint16_t *)gptr_1+1); // a1 b1 c1 d1
				out1 = vmlal_u16 (out1, inp3, c1);//10*a
				out1 = vmlal_u16 (out1, inp4, c3);//11*a

				out2 = vshrq_n_u32 (out1,6);

				out1 = vld1q_u32((const uint32_t *)optr);
				out2 = vaddq_u32(out1,out2);
				vst1q_u32((uint32_t *)optr,out2);

				val = vld1_u8((const uint8_t *)cptr);
				val = vadd_u8 (val, one) ;
				vst1_u8((uint8_t*)cptr,val);
			}
		}
#else
		int prologcnt = param->sx;
#endif //#ifdef __ARM__

		for(j=0;j<prologcnt;j++,cptr++, gptr++, optr++)
		{
			unsigned int val = 0,val1 = 0,val2 = 0,val3=0;
			//9Q7*2Q14 = 11Q21
			val = param->C00 * gptr[0] ;
			val1 = param->C01 * gptr[1] ;
			val2 = param->C10 * gptr[inpws>>1];
			val3 = param->C11 * gptr[(inpws>>1)+1];
			val+=(val1+val2+val3);
			val = val >> 6; // no rounding required
			*optr = (*optr) + val;
			*cptr +=1;
		}
	}
}

void special_divide_ASM(int * param){
	unsigned int *costMatrix = (unsigned int *)param[0];
	int costStep = param[1]>>2;
	unsigned int *costMatrixFloat = (unsigned int *)param[2];
	int costFloatStep = param[3]>>2;
	unsigned char *countMatrix = (unsigned char *)param[4];
	int countStep = param[5];
	unsigned short *div_table = (unsigned short*)param[6];
	int width = param[7];
	int height = param[8];

	for(int i=0;i<height;i++)
	{
		unsigned int *optr = (costMatrix + i*costStep);
		unsigned int *ofptr = (costMatrixFloat + i*costFloatStep);
		unsigned char *cptr =  (countMatrix + i*countStep);

		for(int j=0;j<width;j++){
			unsigned int num = *optr++;
			unsigned short onebyden = div_table[*cptr++];
			unsigned int val1,val2;
			val1 = (num&0xFFFFu)*onebyden + 0x8000u;
			val2 = ((num>>16)&0xFFFFu)*onebyden;
			unsigned int val = val2 + (val1>>16);
			*ofptr++ = val; //value/
		}
	}
}


//void reduce_gauss5_4x(unsigned char* inp,unsigned char* out, int *param)
//{
//	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;
//
//	inputwidth = param[0];
//	inputstep = param[1];
//	inputheight = param[2];
//	outputstep = param[3];
//	outputwidth = inputwidth>>2;
//	outputheight = inputheight>>2;
//
//
//	unsigned char *buffer = (unsigned char*)param[4];
//	unsigned char *row0,*row1,*row2,*row3,*row4;
//	row0 = buffer;
//	row1 = row0 + (outputwidth) ; //for reflection
//	row2 = row1 + (outputwidth) ; //for reflection
//	row3 = row2 + (outputwidth) ; //for reflection
//	row4 = row3 + (outputwidth) ; //for reflection
//
//	//comp row0
//	unsigned char* inptr = (unsigned char*)inp;
//	for(i=1;i<outputwidth-1;i++){
//		short a = inptr[4*i-4] + inptr[4*i+4];
//		short b = inptr[4*i-2] + inptr[4*i+2];
//		int c = a + (b<<2)+inptr[4*i]*6 +8;
//		row2[i] = (unsigned char)(c>>4);
//	}
//	i=0;
//	short a = inptr[4*i+4]<<1;
//	short b = inptr[4*i+2]<<1;
//	int c = a + (b<<2)+ inptr[4*i]*6 +8;
//	row2[0] = (unsigned char)(c>>4);//reflect
//	i = outputwidth-1;
//
//	a = inptr[4*i-4]+inptr[4*i];
//	b = inptr[4*i-2]+inptr[4*i+2];
//	c = a + (b<<2)+ inptr[4*i]*6 +8;
//	row2[outputwidth-1] = (unsigned char)(c>>4);//reflect
//
//	inptr = (unsigned char*)inp + 2*inputstep;
//	for(i=1;i<outputwidth-1;i++){
//		short a = inptr[4*i-4] + inptr[4*i+4];
//		short b = inptr[4*i-2] + inptr[4*i+2];
//		int c = a + (b<<2)+inptr[4*i]*6 +8;
//		row3[i] = (unsigned char)(c>>4);
//	}
//
//	i=0;
//	a = inptr[4*i+4]<<1;
//	b = inptr[4*i+2]<<1;
//	c = a + (b<<2)+ inptr[4*i]*6 +8;
//	row3[0] = (unsigned char)(c>>4);//reflect
//	i = outputwidth-1;
//	a = inptr[4*i-4]+inptr[4*i];
//	b = inptr[4*i-2]+inptr[4*i+2];
//	c = a + (b<<2)+ inptr[4*i]*6 +8;
//	row3[outputwidth-1] = (unsigned char)(c>>4);//reflect
//
//	inptr = (unsigned char*)inp + 4*inputstep;
//	for(i=1;i<outputwidth-1;i++){
//		short a = inptr[4*i-4] + inptr[4*i+4];
//		short b = inptr[4*i-2] + inptr[4*i+2];
//		int c = a + (b<<2)+inptr[4*i]*6 +8;
//		row4[i] = (unsigned char)(c>>4);
//	}
//	i=0;
//	a = inptr[4*i+4]<<1;
//	b = inptr[4*i+2]<<1;
//	c = a + (b<<2)+ inptr[4*i]*6 +8;
//	row4[0] = (unsigned char)(c>>4);//reflect
//	i = outputwidth-1;
//	a = inptr[4*i-4]+inptr[4*i];
//	b = inptr[4*i-2]+inptr[4*i+2];
//	c = a + (b<<2)+ inptr[4*i]*6 +8 ;
//	row4[outputwidth-1] = (unsigned char)(c>>4);//reflect
//
//	memcpy(row1,row3,outputwidth);
//	memcpy(row0,row4,outputwidth);
//
//	unsigned char *baseptr = (unsigned char*)inp + 6*inputstep;
//
//	for(int j=0;j<outputheight-1;j++)
//	{
//		//write one out row;
//		unsigned char *outptr = (unsigned char*)out + j*outputstep;
//		for(i=0;i<outputwidth;i++){
//			short a = row0[i] + row4[i];
//			short b = row1[i] + row3[i];
//			int c = a + (b<<2)+row2[i]*6 +8;
//			*outptr++ = (unsigned char)(c>>4);
//		}
//
//		unsigned char *temp0 = row0,*temp1 = row1,*temp2 = row2,*temp3 = row3,*temp4 = row4;
//		row0 = temp2;
//		row1 = temp3;
//		row2 = temp4;
//		row3 = temp0;
//		row4 = temp1;
//
//		inptr = (unsigned char*)baseptr + (4*j)*inputstep;
//		for(i=1;i<outputwidth-1;i++){
//			short a = inptr[4*i-4] + inptr[4*i+4];
//			short b = inptr[4*i-2] + inptr[4*i+2];
//			int c = a + (b<<2)+inptr[4*i]*6 +8;
//			row3[i] = (unsigned char)(c>>4);
//		}
//		i=0;
//		a = inptr[4*i+4]<<1;
//		b = inptr[4*i+2]<<1;
//		c = a + (b<<2)+ inptr[4*i]*6 +8;
//		row3[0] = (unsigned char)(c>>4);//reflect
//		i = outputwidth-1;
//		a = inptr[4*i-4]+inptr[4*i];
//		b = inptr[4*i-2]+inptr[4*i+2];
//		c = a + (b<<2)+ inptr[4*i]*6 +8;
//		row3[outputwidth-1] = (unsigned char)(c>>4);//reflect
//
//		if( (outputheight-2) != j) // for last row skip it.
//		{
//			inptr = (unsigned char*)baseptr + (4*j+2)*inputstep;
//			for(i=1;i<outputwidth-1;i++){
//				short a = inptr[4*i-4] + inptr[4*i+4];
//				short b = inptr[4*i-2] + inptr[4*i+2];
//				int c = a + (b<<2)+inptr[4*i]*6 +8;
//				row4[i] = (unsigned char)(c>>4);
//			}
//			i=0;
//			a = inptr[4*i+4]<<1;
//			b = inptr[4*i+2]<<1;
//			c = a + (b<<2)+ inptr[4*i]*6 +8;
//			row4[0] = (unsigned char)(c>>4);//reflect
//			i = outputwidth-1;
//			a = inptr[4*i-4]+inptr[4*i];
//			b = inptr[4*i-2]+inptr[4*i+2];
//			c = a + (b<<2)+ inptr[4*i]*6 +8;
//			row4[outputwidth-1] = (unsigned char)(c>>4);//reflect
//		}
//	}
//
//	unsigned char *outptr = (unsigned char*)out + (outputheight -1)*outputstep;
//	for(int i=0;i<outputwidth;i++){
//		short a = row0[i] + row2[i];
//		short b = row1[i] + row3[i];
//		int c = a + (b<<2)+row2[i]*6 +8;
//		*outptr++ = (unsigned char)(c>>4);
//	}
//}

void reduce_gauss5_2x(unsigned char* inp,unsigned char* out, int *param)
{
	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;

	short a,b;
	int c;

	inputwidth = param[0];
	inputstep = param[1];
	inputheight = param[2];
	outputstep = param[3];
	outputwidth = inputwidth>>1;
	outputheight = inputheight>>1;

	unsigned char *buffer = (unsigned char*)param[4];
	unsigned char *row0,*row1,*row2,*row3,*row4;
	row0 = buffer;
	row1 = row0 + (outputwidth) ; //for reflection
	row2 = row1 + (outputwidth) ; //for reflection
	row3 = row2 + (outputwidth) ; //for reflection
	row4 = row3 + (outputwidth) ; //for reflection

	//comp row0
	unsigned char* inptr = (unsigned char*)inp;

	{
		a = inptr[2] + inptr[2];
		b = inptr[1] + inptr[1];
		c = a + (b<<2)+inptr[0]*6 +8;
		row2[0] = (unsigned char)(c>>4);
	}
	for(i=1;i<outputwidth-1;i++){
		a = inptr[2*i-2] + inptr[2*i+2];
		b = inptr[2*i-1] + inptr[2*i+1];
		c = a + (b<<2)+inptr[2*i]*6 +8;
		row2[i] = (unsigned char)(c>>4);
	}
	{
		// 0 1 2 3 4 5 6 7 8(6)
		//         1 4 6 4 1
		a = inptr[2*outputwidth-2] + inptr[2*outputwidth-4];
		b = inptr[2*outputwidth-1] + inptr[2*outputwidth-3];
		c = a + (b<<2)+inptr[2*outputwidth-2]*6 +8;
		row2[outputwidth-1] = (unsigned char)(c>>4);
	}

	inptr = (unsigned char*)inp + 1*inputstep;
	{
		a = inptr[2] + inptr[2];
		b = inptr[1] + inptr[1];
		c = a + (b<<2)+inptr[0]*6 +8;
		row3[0] = (unsigned char)(c>>4);
	}
	for( i=1;i<outputwidth-1;i++){
		a = inptr[2*i-2] + inptr[2*i+2];
		b = inptr[2*i-1] + inptr[2*i+1];
		c = a + (b<<2)+inptr[2*i]*6 +8;
		row3[i] = (unsigned char)(c>>4);
	}
	{
		a = inptr[2*outputwidth-2] + inptr[2*outputwidth-4];
		b = inptr[2*outputwidth-1] + inptr[2*outputwidth-3];
		c = a + (b<<2)+inptr[2*outputwidth-2]*6 +8;
		row3[outputwidth-1] = (unsigned char)(c>>4);
	}

	inptr = (unsigned char*)inp + 2*inputstep;
	{
		a = inptr[2] + inptr[2];
		b = inptr[1] + inptr[1];
		c = a + (b<<2)+inptr[0]*6 +8;
		row4[0] = (unsigned char)(c>>4);
	}

	for(i=1;i<outputwidth-1;i++){
		a = inptr[2*i-2] + inptr[2*i+2];
		b = inptr[2*i-1] + inptr[2*i+1];
		c = a + (b<<2)+inptr[2*i]*6 +8;
		row4[i] = (unsigned char)(c>>4);
	}
	{
		a = inptr[2*outputwidth-2] + inptr[2*outputwidth-4];
		b = inptr[2*outputwidth-1] + inptr[2*outputwidth-3];
		c = a + (b<<2)+inptr[2*outputwidth-2]*6 +8;
		row4[outputwidth-1] = (unsigned char)(c>>4);
	}

	memcpy(row1,row3,outputwidth);
	memcpy(row0,row4,outputwidth);

	unsigned char *baseptr = (unsigned char*)inp + 3*inputstep;

	for(int j=0;j<outputheight-1;j++)
	{
		//write one out row;
		unsigned char *outptr = (unsigned char*)out + j*outputstep;
		for(i=0;i<outputwidth;i++){
			a = row0[i] + row4[i];
			b = row1[i] + row3[i];
			c = a + (b<<2)+row2[i]*6 +8;
			*outptr++ = (unsigned char)(c>>4);
		}

		unsigned char *temp0 = row0,*temp1 = row1,*temp2 = row2,*temp3 = row3,*temp4 = row4;
		row0 = temp2;
		row1 = temp3;
		row2 = temp4;
		row3 = temp0;
		row4 = temp1;

		inptr = (unsigned char*)baseptr + (2*j)*inputstep;

		{
			a = inptr[2] + inptr[2];
			b = inptr[1] + inptr[1];
			c = a + (b<<2)+inptr[0]*6 +8;
			row3[0] = (unsigned char)(c>>4);
		}

		for(i=1;i<outputwidth-1;i++){
			a = inptr[2*i-2] + inptr[2*i+2];
			b = inptr[2*i-1] + inptr[2*i+1];
			c = a + (b<<2)+inptr[2*i]*6 +8;
			row3[i] = (unsigned char)(c>>4);
		}
		{
			a = inptr[2*outputwidth-2] + inptr[2*outputwidth-4];
			b = inptr[2*outputwidth-1] + inptr[2*outputwidth-3];
			c = a + (b<<2)+inptr[2*outputwidth-2]*6 +8;
			row3[outputwidth-1] = (unsigned char)(c>>4);
		}

		if( (outputheight-2) != j) // for last row skip it.
		{
			inptr = (unsigned char*)baseptr + (2*j+1)*inputstep;
			{
				a = inptr[2] + inptr[2];
				b = inptr[1] + inptr[1];
				c = a + (b<<2)+inptr[0]*6 +8;
				row4[0] = (unsigned char)(c>>4);
			}

			for(i=1;i<outputwidth-1;i++){
				a = inptr[2*i-2] + inptr[2*i+2];
				b = inptr[2*i-1] + inptr[2*i+1];
				c = a + (b<<2)+inptr[2*i]*6 +8;
				row4[i] = (unsigned char)(c>>4);
			}
			{
				a = inptr[2*outputwidth-2] + inptr[2*outputwidth-4];
				b = inptr[2*outputwidth-1] + inptr[2*outputwidth-3];
				c = a + (b<<2)+inptr[2*outputwidth-2]*6 +8;
				row4[outputwidth-1] = (unsigned char)(c>>4);
			}
		}
	}

	unsigned char *outptr = (unsigned char*)out + (outputheight -1)*outputstep;
	for(i=0;i<outputwidth;i++){
		a = row0[i] + row2[i];
		b = row1[i] + row3[i];
		c = a + (b<<2)+row2[i]*6 +8;
		*outptr++ = (unsigned char)(c>>4);
	}
}


//Let me write it in C first
short detect_single_specularity_4(unsigned char*inp, Point2D16i *buff, int* params){
	int width = params[0];
	int height = params[1];
	int widthstep = params[2];
	unsigned char specmag = (unsigned char )(params[3]&0xFF);
	int maxbuffcnt = params[4];
	int totalcnt = 0;

	int widthloop = width-8;
	int heightloop = height-8;
	//a0 b0 c0 d0 e0 f0 g0 h0 i0
	//a1 b1 c1 d1 e1 f1 g1 h1 i1
	//a2 b2 c2 d2 e2 f2 g2 h2 i2
	//a3 b3 c3 d3 e3 f3 g3 h3 i3
	//a4 b4 c4 d4 e4 f4 g4 h4 i4
	//a5 b5 c5 d5 e5 f5 g5 h5 i5
	//a6 b6 c6 d6 e6 f6 g6 h6 i6
	//a7 b7 c7 d7 e7 f7 g7 h7 i7
	//a8 b8 c8 d8 e8 f8 g8 h8 i8

	// first check if the current pixel is > the adjacent pixel by the threshold..
	for(int j = 0;j<heightloop;j++){
		unsigned char *center = inp+j*widthstep; //e4
		unsigned char *left = center -4 ; //a4
		unsigned char *top = center -4*widthstep;//e0
		unsigned char *right = center + 4; //i4
		unsigned char *bottom = center +4*widthstep;//e8

		for(int i=0;i<widthloop;i++,left++,right++,top++,bottom++){
			unsigned char val = *(center+i);
			//printf("\nVal %d ",val);
			if(!(((val - *left)>specmag)&&
				((val - *right)>specmag)&&
				((val - *top)>specmag)&&
				((val - *bottom)>specmag)
				)){
				continue;
			}
			//printf("INITITAL %d %d %d %d ",*left,*right,*top,*bottom);

			// INVESTIGATE_LIKELY_SPECULARITY
			unsigned char *a0 = top-4;
			unsigned char *f0 = top+1;
			if(!(((val - a0[0])>specmag)&&
				((val - a0[1])>specmag)&&
				((val - a0[2])>specmag)&&
				((val - a0[3])>specmag)
				)){
				continue;
			}
			if(!(((val - f0[0])>specmag)&&
				((val - f0[1])>specmag)&&
				((val - f0[2])>specmag)&&
				((val - f0[3])>specmag)
				)){
				continue;
			}

			unsigned char *a8 = bottom-4;
			unsigned char *f8 = bottom+1;
			if(!(((val - a8[0])>specmag)&&
				((val - a8[1])>specmag)&&
				((val - a8[2])>specmag)&&
				((val - a8[3])>specmag)
				)){
				continue;
			}
			if(!(((val - f8[0])>specmag)&&
				((val - f8[1])>specmag)&&
				((val - f8[2])>specmag)&&
				((val - f8[3])>specmag)
				)){
				continue;
			}

			unsigned char *a1 = top-4+widthstep;
			unsigned char *i1 = a1+8;
			int pass = 1;
			for(int k=0;k<7;k++,a1+=widthstep,i1+=widthstep){
				//printf("[%d %d]",a1[0],i1[0]);
				if(!(((val - a1[0])>specmag)&&
					((val - i1[0])>specmag)
					)){
					pass=0;
				}
			}

			if(pass){
				//It means the point is specularity store the x and y
				Point2D16i temp;
				temp.x = i+4;
				temp.y = j+4;
				if(totalcnt >= maxbuffcnt){
					return totalcnt;
				}
				buff[totalcnt]=temp;
				//printf("%d %d %d\n",val,temp.x,temp.y);
				totalcnt+=1;
			}
		}
	}
	return totalcnt;
}


//Let me write it in C first
short detect_single_specularity(unsigned char*inp, Point2D16i *buff, int* params){
	int width = params[0];
	int height = params[1];
	int widthstep = params[2];
	unsigned char specmag = (unsigned char )(params[3]&0xFF);
	int maxbuffcnt = params[4];
	int radius = params[5];
	int totalcnt = 0;

	int widthloop = width-2*radius;
	int heightloop = height-2*radius;
	//a0 b0 c0 d0 e0 f0 g0 h0 i0
	//a1 b1 c1 d1 e1 f1 g1 h1 i1
	//a2 b2 c2 d2 e2 f2 g2 h2 i2
	//a3 b3 c3 d3 e3 f3 g3 h3 i3
	//a4 b4 c4 d4 e4 f4 g4 h4 i4
	//a5 b5 c5 d5 e5 f5 g5 h5 i5
	//a6 b6 c6 d6 e6 f6 g6 h6 i6
	//a7 b7 c7 d7 e7 f7 g7 h7 i7
	//a8 b8 c8 d8 e8 f8 g8 h8 i8

	// first check if the current pixel is > the adjacent pixel by the threshold..
	for(int j = 0;j<heightloop;j++){
		unsigned char *center = inp+j*widthstep; //e4
		unsigned char *left = center -radius ; //a4
		unsigned char *top = center -radius*widthstep;//e0
		unsigned char *right = center + radius; //i4
		unsigned char *bottom = center +radius*widthstep;//e8

		for(int i=0;i<widthloop;i++,left++,right++,top++,bottom++){
			unsigned char val = *(center+i);
			if(!(((val - *left)>specmag)&&
				((val - *right)>specmag)&&
				((val - *top)>specmag)&&
				((val - *bottom)>specmag)
				)){
				continue;
			}
			//printf("INITITAL %d %d %d %d ",*left,*right,*top,*bottom);

			// INVESTIGATE_LIKELY_SPECULARITY
			unsigned char *a0 = top-radius;
			unsigned char *f0 = top+1;
			int pass = 1;
			for(int k=0;k<radius;k++){
				if(!(((val - a0[k])>specmag)&&
					 ((val - f0[k])>specmag))){
					pass = 0;
				}
			}
			if(pass == 0)
				continue;

			unsigned char *a8 = bottom-radius;
			unsigned char *f8 = bottom+1;
			for(int k=0;k<radius;k++){
				if(!(((val - a8[k])>specmag)&&
					 ((val - f8[k])>specmag))){
					pass = 0;
				}
			}
			if(pass == 0)
				continue;

			unsigned char *a1 = top-radius+widthstep;
			unsigned char *i1 = a1+2*radius;
			for(int k=0;k<(2*radius-1);k++,a1+=widthstep,i1+=widthstep){
				if(!(((val - a1[0])>specmag)&&
					((val - i1[0])>specmag)
					)){
					pass=0;
				}
			}

			if(pass){
				//It means the point is specularity store the x and y
				Point2D16i temp;
				temp.x = i+radius;
				temp.y = j+radius;
				if(totalcnt >= maxbuffcnt){
					return totalcnt;
				}
				buff[totalcnt]=temp;
				//printf("%d %d %d\n",val,temp.x,temp.y);
				totalcnt+=1;
			}
		}
	}
	return totalcnt;
}



#if 1
void reduce_gauss5_4x(unsigned char* inp,unsigned char* out, int *param)
{
	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;

	inputwidth = param[0];
	inputstep = param[1];
	inputheight = param[2];
	outputstep = param[3];
	outputwidth = inputwidth>>2;
	outputheight = inputheight>>2;

#ifdef __ARM__
	uint16x8_t res;
	uint8x8_t r0,r1,r2,r3,r4;
	uint8x8_t six,four,one;
	six = vdup_n_u8((uint8_t)6);
	four = vdup_n_u8((uint8_t)4);
	one = vdup_n_u8((uint8_t)1);

	uint16x4_t v1,v2;
	uint32x2_t v;
	uint64x1_t resv;
	uint8x8_t inpbytes;
	uint16x8_t sum1,sum2;

#endif

	unsigned char *buffer = (unsigned char*)param[4];
	unsigned char *row0,*row1,*row2,*row3,*row4;
	row0 = buffer;
	row1 = row0 + (outputwidth) ; //for reflection
	row2 = row1 + (outputwidth) ; //for reflection
	row3 = row2 + (outputwidth) ; //for reflection
	row4 = row3 + (outputwidth) ; //for reflection

	//comp row0
	unsigned char* inptr = (unsigned char*)inp;
	for(i=1;i<outputwidth-1;i++){
		short a = inptr[4*i-4] + inptr[4*i+4];
		short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row2[i] = (unsigned char)(c>>4);
	}
	i=0;
	short a = inptr[4*i+4]<<1;
	short b = inptr[4*i+2]<<1;
	int c = a + (b<<2)+ inptr[4*i]*6 +8;
	row2[0] = (unsigned char)(c>>4);//reflect
	i = outputwidth-1;

	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row2[outputwidth-1] = (unsigned char)(c>>4);//reflect

	inptr = (unsigned char*)inp + 2*inputstep;
	for(i=1;i<outputwidth-1;i++){
		short a = inptr[4*i-4] + inptr[4*i+4];
		short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row3[i] = (unsigned char)(c>>4);
	}

	i=0;
	a = inptr[4*i+4]<<1;
	b = inptr[4*i+2]<<1;
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row3[0] = (unsigned char)(c>>4);//reflect
	i = outputwidth-1;
	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row3[outputwidth-1] = (unsigned char)(c>>4);//reflect

	inptr = (unsigned char*)inp + 4*inputstep;
	for(i=1;i<outputwidth-1;i++){
		short a = inptr[4*i-4] + inptr[4*i+4];
		short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row4[i] = (unsigned char)(c>>4);
	}
	i=0;
	a = inptr[4*i+4]<<1;
	b = inptr[4*i+2]<<1;
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row4[0] = (unsigned char)(c>>4);//reflect
	i = outputwidth-1;
	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8 ;
	row4[outputwidth-1] = (unsigned char)(c>>4);//reflect

	memcpy(row1,row3,outputwidth);
	memcpy(row0,row4,outputwidth);

	unsigned char *baseptr = (unsigned char*)inp + 6*inputstep;

	int loopvar;
	for(int j=0;j<outputheight-1;j++)
	{

		unsigned char *outptr = (unsigned char*)out + j*outputstep;

#ifdef __ARM__
		int wloop = (outputwidth>>3);
		loopvar = outputwidth -(wloop<<3);

		int k = 0;

		for(i=0;i<wloop;i++){
			r0 = vld1_u8((const uint8_t *)row0+k);
			r4 = vld1_u8((const uint8_t *)row4+k);
			r1 = vld1_u8((const uint8_t *)row1+k);
			r3 = vld1_u8((const uint8_t *)row3+k);
			r2 = vld1_u8((const uint8_t *)row2+k);

			res = vmull_u8 (r0, one);
			res = vmlal_u8 (res,r4, one);
			res = vmlal_u8 (res,r1, four);
			res = vmlal_u8 (res,r3, four);
			res = vmlal_u8 (res,r2, six);
			r4 = vrshrn_n_u16 (res,4);
			vst1_u8((uint8_t *)(outptr+k),r4);
			k+=8;
		}
#else
		loopvar = outputwidth;
#endif
		for(i=0;i<loopvar;i++){
			short a = row0[i] + row4[i];
			short b = row1[i] + row3[i];
			int c = a + (b<<2)+row2[i]*6 +8;
			*outptr++ = (unsigned char)(c>>4);
		}

		unsigned char *temp0 = row0,*temp1 = row1,*temp2 = row2,*temp3 = row3,*temp4 = row4;
		row0 = temp2;
		row1 = temp3;
		row2 = temp4;
		row3 = temp0;
		row4 = temp1;

		inptr = (unsigned char*)baseptr + (4*j)*inputstep;

#if 0//def __ARM__

		unsigned char *onebuff = inptr;
		unsigned short *fourbuff = (unsigned short *)Gauss4xBuffer;
		unsigned short *sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

		for(i=0;i<inputstep;i+=8,onebuff+=8,fourbuff+=8,sixbuff+=8){
			inpbytes = vld1_u8((const uint8_t *)onebuff);
			res = vmull_u8 (inpbytes, four);
			vst1q_u16 ((uint16_t *)fourbuff, res);
			res = vmull_u8 (inpbytes, six);
			vst1q_u16 ((uint16_t *)sixbuff, res);
		}

		onebuff = inptr;
		fourbuff = (unsigned short *)Gauss4xBuffer;
		sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

		unsigned char *p1 = onebuff,*p2 =onebuff+4;
		unsigned short *p3 = fourbuff+2,*p4 =fourbuff+6;
		unsigned short *p5 = sixbuff+4;

		for(i=1;i<outputwidth-1;i++,p1+=4,p2+=4,p3+=4,p4+=4,p5+=4){
			unsigned short c = *p1 + *p2 + *p3 + *p4 + *p5 + 8;
			row3[i] =(c>>4);
		}

		i=0;
		a = onebuff[4*i+4]<<1;
		b = fourbuff[4*i+2]<<1;
		c = a + b+ sixbuff[4*i] +8;
		row3[0] = (unsigned char)(c>>4);//reflect
		i = outputwidth-1;
		a = onebuff[4*i-4]+inptr[4*i];
		b = fourbuff[4*i-2]+fourbuff[4*i+2];
		c = a + b+ sixbuff[4*i]+8;
		row3[outputwidth-1] = (unsigned char)(c>>4);//reflect

#else
		for(i=1;i<outputwidth-1;i++){
			short a = inptr[4*i-4] + inptr[4*i+4];
			short b = inptr[4*i-2] + inptr[4*i+2];
			int c = a + (b<<2)+inptr[4*i]*6 +8;
			row3[i] = (unsigned char)(c>>4);
		}

		i=0;
		a = inptr[4*i+4]<<1;
		b = inptr[4*i+2]<<1;
		c = a + (b<<2)+ inptr[4*i]*6 +8;
		row3[0] = (unsigned char)(c>>4);//reflect
		i = outputwidth-1;
		a = inptr[4*i-4]+inptr[4*i];
		b = inptr[4*i-2]+inptr[4*i+2];
		c = a + (b<<2)+ inptr[4*i]*6 +8;
		row3[outputwidth-1] = (unsigned char)(c>>4);//reflect
#endif

		if( (outputheight-2) != j) // for last row skip it.
		{
			inptr = (unsigned char*)baseptr + (4*j+2)*inputstep;

	#if 0 //def __ARM__

			unsigned char *onebuff = inptr;
			unsigned short *fourbuff = (unsigned short *)Gauss4xBuffer;
			unsigned short *sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

			for(i=0;i<inputstep;i+=8,onebuff+=8,fourbuff+=8,sixbuff+=8){
				inpbytes = vld1_u8((const uint8_t *)onebuff);
				res = vmull_u8 (inpbytes, four);
				vst1q_u16 ((uint16_t *)fourbuff, res);
				res = vmull_u8 (inpbytes, six);
				vst1q_u16 ((uint16_t *)sixbuff, res);
			}

			onebuff = inptr;
			fourbuff = (unsigned short *)Gauss4xBuffer;
			sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

			unsigned char *p1 = onebuff,*p2 =onebuff+4;
			unsigned short *p3 = fourbuff+2,*p4 =fourbuff+6;
			unsigned short *p5 = sixbuff+4;

			#pragma unroll
			for(i=1;i<outputwidth-1;i++,p1+=4,p2+=4,p3+=4,p4+=4,p5+=4){
				unsigned short c = *p1 + *p2 + *p3 + *p4 + *p5 + 8;
				row4[i] =(c>>4);
			}

			i=0;
			a = onebuff[4*i+4]<<1;
			b = fourbuff[4*i+2]<<1;
			c = a + b+ sixbuff[4*i] +8;
			row4[0] = (unsigned char)(c>>4);//reflect
			i = outputwidth-1;
			a = onebuff[4*i-4]+inptr[4*i];
			b = fourbuff[4*i-2]+fourbuff[4*i+2];
			c = a + b+ sixbuff[4*i]+8;
			row4[outputwidth-1] = (unsigned char)(c>>4);//reflect


	#else

			for(i=1;i<outputwidth-1;i++){
				short a = inptr[4*i-4] + inptr[4*i+4];
				short b = inptr[4*i-2] + inptr[4*i+2];
				int c = a + (b<<2)+inptr[4*i]*6 +8;
				row4[i] = (unsigned char)(c>>4);
			}
			i=0;
			a = inptr[4*i+4]<<1;
			b = inptr[4*i+2]<<1;
			c = a + (b<<2)+ inptr[4*i]*6 +8;
			row4[0] = (unsigned char)(c>>4);//reflect
			i = outputwidth-1;
			a = inptr[4*i-4]+inptr[4*i];
			b = inptr[4*i-2]+inptr[4*i+2];
			c = a + (b<<2)+ inptr[4*i]*6 +8;
			row4[outputwidth-1] = (unsigned char)(c>>4);//reflect
	#endif
		}
	}

		unsigned char *outptr = (unsigned char*)out + (outputheight -1)*outputstep;

#ifdef __ARM__
	int wloop = (outputwidth>>3);
	loopvar = outputwidth -(wloop<<3);

	int k = 0;

	for(i=0;i<wloop;i++){
		r0 = vld1_u8((const uint8_t *)row0+k);
		r1 = vld1_u8((const uint8_t *)row1+k);
		r3 = vld1_u8((const uint8_t *)row3+k);
		r2 = vld1_u8((const uint8_t *)row2+k);

		res = vmull_u8 (r0, one);
		res = vmlal_u8 (res,r2, one);
		res = vmlal_u8 (res,r1, four);
		res = vmlal_u8 (res,r3, four);
		res = vmlal_u8 (res,r2, six);
		r4 = vrshrn_n_u16 (res,4);
		vst1_u8((uint8_t *)(outptr+k),r4);
		k+=8;
	}
#else
	loopvar = outputwidth;
#endif
	for(i=0;i<loopvar;i++){
		short a = row0[i] + row2[i];
		short b = row1[i] + row3[i];
		int c = a + (b<<2)+row2[i]*6 +8;
		*outptr++ = (unsigned char)(c>>4);
	}
}
#else

extern "C" void convert_asm_neon(unsigned char *minus4,unsigned short* oxp,int outputwidth);

// scr - height = in->height/2, and width = out->width or in->width/4
void reduce_gauss5_4x(unsigned char* inp,unsigned char* out, int *param)
{
	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;
	inputwidth = param[0];
	inputstep = param[1];
	inputheight = param[2];
	outputstep = param[3];
	unsigned char* scratch = (unsigned char*)param[4];
	outputwidth = inputwidth>>2;
	outputheight = inputheight>>2;
	int scrwidthstep = inputwidth<<1;

	uint8x8_t six8 = vdup_n_u8((uint8_t)6);
	uint8x8_t four8 = vdup_n_u8((uint8_t)4);
	uint8x8_t one8 = vdup_n_u8((uint8_t)1);

	unsigned char *inPtr=inp;

	/* Horizontal filter */
	for(int j=1;j<outputheight*2-1;j++)
	{
		// every alternate row of the input data is filtered, note the factor of 2
		register unsigned char* ixp = (unsigned char *) inPtr + 2*j*inputstep;
		unsigned short* oxp = (unsigned short *) (scratch + j*scrwidthstep);
		// 1 0 4 0 6 0 4 0 1 kernel is used to skip alternate pixels

#ifdef __ARM__

		unsigned char *onebuff = ixp;
		unsigned char *minus4 = (unsigned char *)Gauss4xBuffer;
		unsigned char *plus4,*plus2,*zero;
		unsigned char *minus2 = minus4 + outputwidth*2;

		ExtractEvenBytes(onebuff,minus4,minus2,outputwidth);

		minus4 = (unsigned char *)Gauss4xBuffer;
	#ifndef ASM
		minus2 = minus4 + 2*outputwidth;
		zero = minus4 +1;

		plus2 = minus2 + 1;
		plus4 = minus4 + 2;

		register uint16x8_t res;
		//minus4+=8,minus2+=8,plus4+=8,plus2+=8,zero+=8
		for(int i=0;i<outputwidth;i+=8,oxp+=8){
			res = vmull_u8 (vld1_u8(minus4+=8), one8);
			res = vmlal_u8 (res,vld1_u8(plus4+=8), one8);
			res = vmlal_u8 (res,vld1_u8(minus2+=8), four8);
			res = vmlal_u8 (res,vld1_u8(plus2+=8), four8);
			res = vmlal_u8 (res,vld1_u8(zero+=8), six8);
			vst1q_u16(oxp,res);
		}
	#else
		convert_asm_neon(minus4,oxp,outputwidth);
	#endif
#else
		for(int il=0;il<outputwidth;il++,ixp+=4) {
			*oxp++ = *ixp * 6  + (( ixp[-2]+ ixp[2] ) << 2 ) +	ixp[-4] + ixp[4];
		}
#endif

	}

	/* Vertical Filter */
	for(int jl=0,j=0;jl<outputheight;jl++,j+=2)
	{
		unsigned char *oxp  = (unsigned char *) out + jl*outputstep;
		unsigned short *xp   = (unsigned short *) (scratch + j*scrwidthstep);
		unsigned short *xp2_ = xp -  scrwidthstep;
		unsigned short *xp1_ = xp - (scrwidthstep >> 1);
		unsigned short *xp1  = xp + (scrwidthstep >> 1);
		unsigned short *xp2  = xp +  scrwidthstep;

	#ifdef __ARM__
		int wloop = (outputwidth>>3);
		int loopvar = outputwidth - (wloop<<3);
		uint16x8_t one=vdupq_n_u16(1),four=vdupq_n_u16(4),six=vdupq_n_u16(6),res,temp;
		uint8x8_t valo;

		for(int i=0;i<wloop;i++,xp+=8,xp1+=8,xp1_+=8,xp2+=8,xp2_+=8,oxp+=8){
			res = vmulq_u16 (vld1q_u16(xp2),one);
			res = vmlaq_u16 (res,vld1q_u16(xp2_),one);
			res = vmlaq_u16 (res,vld1q_u16(xp1),four);
			res = vmlaq_u16 (res,vld1q_u16(xp1_),four);
			res = vmlaq_u16 (res,vld1q_u16(xp),six);
			valo = vrshrn_n_u16 (res,8);
			vst1_u8(oxp,valo);
		}
	#else
		int loopvar = outputwidth;
	#endif

		for(int i=0;i<loopvar;i++,xp++,xp1++,xp1_++,xp2++,xp2_++) {
			register unsigned short val;
			val =  *xp * 6 + (( *xp1 + *xp1_ ) << 2 ) +	*xp2 + *xp2_ + 128;
			*oxp++ = (unsigned char) ( val >> 8 );
		}
	}

}
#endif


#ifdef __ARM__

void reduce_gauss5_16bit_4x(unsigned short* inp,unsigned short* out, int *param)
{
	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;

	inputwidth = param[0];
	inputstep = param[1]>>1;
	inputheight = param[2];
	outputstep = param[3]>>1;
	outputwidth = inputwidth>>2;
	outputheight = inputheight>>2;

#ifdef __ARM__
	uint16x8_t res;
	uint16x8_t r0,r1,r2,r3,r4;
	uint16x8_t six,four,one;
	six = vdupq_n_u16((uint16_t)6);
	four = vdupq_n_u16((uint16_t)4);
	one = vdupq_n_u16((uint16_t)1);

	uint16x4_t v1,v2;
	uint32x2_t v;
	uint64x1_t resv;
	uint8x8_t inpbytes;
	uint16x8_t sum1,sum2;

#endif

	unsigned short *buffer = (unsigned short*)param[4];
	unsigned short *row0,*row1,*row2,*row3,*row4;
	row0 = buffer;
	row1 = row0 + (outputwidth) ; //for reflection
	row2 = row1 + (outputwidth) ; //for reflection
	row3 = row2 + (outputwidth) ; //for reflection
	row4 = row3 + (outputwidth) ; //for reflection

	//comp row0
	unsigned short* inptr = (unsigned short*)inp;
	for(i=1;i<outputwidth-1;i++){
		unsigned short a = inptr[4*i-4] + inptr[4*i+4];
		unsigned short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row2[i] = (unsigned short)(c>>4);
	}
	i=0;
	unsigned short a = inptr[4*i+4]<<1;
	unsigned short b = inptr[4*i+2]<<1;
	int c = a + (b<<2)+ inptr[4*i]*6 +8;
	row2[0] = (unsigned short)(c>>4);//reflect
	i = outputwidth-1;

	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row2[outputwidth-1] = (unsigned short)(c>>4);//reflect

	inptr = (unsigned short*)inp + 2*inputstep;
	for(i=1;i<outputwidth-1;i++){
		short a = inptr[4*i-4] + inptr[4*i+4];
		short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row3[i] = (unsigned short)(c>>4);
	}

	i=0;
	a = inptr[4*i+4]<<1;
	b = inptr[4*i+2]<<1;
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row3[0] = (unsigned short)(c>>4);//reflect
	i = outputwidth-1;
	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row3[outputwidth-1] = (unsigned short)(c>>4);//reflect

	inptr = (unsigned short*)inp + 4*inputstep;
	for(i=1;i<outputwidth-1;i++){
		unsigned short a = inptr[4*i-4] + inptr[4*i+4];
		unsigned short b = inptr[4*i-2] + inptr[4*i+2];
		int c = a + (b<<2)+inptr[4*i]*6 +8;
		row4[i] = (unsigned short)(c>>4);
	}
	i=0;
	a = inptr[4*i+4]<<1;
	b = inptr[4*i+2]<<1;
	c = a + (b<<2)+ inptr[4*i]*6 +8;
	row4[0] = (unsigned short)(c>>4);//reflect
	i = outputwidth-1;
	a = inptr[4*i-4]+inptr[4*i];
	b = inptr[4*i-2]+inptr[4*i+2];
	c = a + (b<<2)+ inptr[4*i]*6 +8 ;
	row4[outputwidth-1] = (unsigned short)(c>>4);//reflect

	memcpy(row1,row3,outputwidth*2);
	memcpy(row0,row4,outputwidth*2);

	unsigned short *baseptr = (unsigned short*)inp + 6*inputstep;

	int loopvar;
	for(int j=0;j<outputheight-1;j++)
	{
		unsigned short *outptr = (unsigned short*)out + j*outputstep;

#ifdef __ARM__
		int wloop = (outputwidth>>3);
		loopvar = outputwidth -(wloop<<3);

		int k = 0;

		for(i=0;i<wloop;i++){
			r0 = vld1q_u16((const uint16_t *)row0+k);
			r4 = vld1q_u16((const uint16_t *)row4+k);
			r1 = vld1q_u16((const uint16_t *)row1+k);
			r3 = vld1q_u16((const uint16_t *)row3+k);
			r2 = vld1q_u16((const uint16_t *)row2+k);

			res = vmulq_u16 (r0, one);
			res = vmlaq_u16 (res,r4, one);
			res = vmlaq_u16 (res,r1, four);
			res = vmlaq_u16 (res,r3, four);
			res = vmlaq_u16 (res,r2, six);
			r4 = vrshrq_n_u16 (res,4);
			vst1q_u16((uint16_t *)(outptr+k),r4);
			k+=8;
		}
#else
		loopvar = outputwidth;
#endif
		for(i=0;i<loopvar;i++){
			unsigned short a = row0[i] + row4[i];
			unsigned short b = row1[i] + row3[i];
			int c = a + (b<<2)+row2[i]*6 +8;
			*outptr++ = (unsigned short)(c>>4);
		}

		unsigned short *temp0 = row0,*temp1 = row1,*temp2 = row2,*temp3 = row3,*temp4 = row4;
		row0 = temp2;
		row1 = temp3;
		row2 = temp4;
		row3 = temp0;
		row4 = temp1;

		inptr = (unsigned short*)baseptr + (4*j)*inputstep;

#ifndef __ARM__

		unsigned short *onebuff = inptr;
		unsigned short *fourbuff = (unsigned short *)Gauss4xBuffer;
		unsigned short *sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

		for(i=0;i<inputstep;i+=8,onebuff+=8,fourbuff+=8,sixbuff+=8){
			inpbytes = vld1_u8((const uint8_t *)onebuff);
			res = vmull_u8 (inpbytes, four);
			vst1q_u16 ((uint16_t *)fourbuff, res);
			res = vmull_u8 (inpbytes, six);
			vst1q_u16 ((uint16_t *)sixbuff, res);
		}

		onebuff = inptr;
		fourbuff = (unsigned short *)Gauss4xBuffer;
		sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

		unsigned char *p1 = onebuff,*p2 =onebuff+4;
		unsigned short *p3 = fourbuff+2,*p4 =fourbuff+6;
		unsigned short *p5 = sixbuff+4;

		for(i=1;i<outputwidth-1;i++,p1+=4,p2+=4,p3+=4,p4+=4,p5+=4){
			unsigned short c = *p1 + *p2 + *p3 + *p4 + *p5 + 8;
			row3[i] =(c>>4);
		}

		i=0;
		a = onebuff[4*i+4]<<1;
		b = fourbuff[4*i+2]<<1;
		c = a + b+ sixbuff[4*i] +8;
		row3[0] = (unsigned char)(c>>4);//reflect
		i = outputwidth-1;
		a = onebuff[4*i-4]+inptr[4*i];
		b = fourbuff[4*i-2]+fourbuff[4*i+2];
		c = a + b+ sixbuff[4*i]+8;
		row3[outputwidth-1] = (unsigned char)(c>>4);//reflect

#else
		for(i=1;i<outputwidth-1;i++){
			unsigned short a = inptr[4*i-4] + inptr[4*i+4];
			unsigned short b = inptr[4*i-2] + inptr[4*i+2];
			int c = a + (b<<2)+inptr[4*i]*6 +8;
			row3[i] = (unsigned short)(c>>4);
		}

		i=0;
		a = inptr[4*i+4]<<1;
		b = inptr[4*i+2]<<1;
		c = a + (b<<2)+ inptr[4*i]*6 +8;
		row3[0] = (unsigned short)(c>>4);//reflect
		i = outputwidth-1;
		a = inptr[4*i-4]+inptr[4*i];
		b = inptr[4*i-2]+inptr[4*i+2];
		c = a + (b<<2)+ inptr[4*i]*6 +8;
		row3[outputwidth-1] = (unsigned short)(c>>4);//reflect
#endif

		if( (outputheight-2) != j) // for last row skip it.
		{
			inptr = (unsigned short*)baseptr + (4*j+2)*inputstep;

	#if 0 //def __ARM__

			unsigned char *onebuff = inptr;
			unsigned short *fourbuff = (unsigned short *)Gauss4xBuffer;
			unsigned short *sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

			for(i=0;i<inputstep;i+=8,onebuff+=8,fourbuff+=8,sixbuff+=8){
				inpbytes = vld1_u8((const uint8_t *)onebuff);
				res = vmull_u8 (inpbytes, four);
				vst1q_u16 ((uint16_t *)fourbuff, res);
				res = vmull_u8 (inpbytes, six);
				vst1q_u16 ((uint16_t *)sixbuff, res);
			}

			onebuff = inptr;
			fourbuff = (unsigned short *)Gauss4xBuffer;
			sixbuff = (unsigned short *)Gauss4xBuffer+(inputstep);

			unsigned char *p1 = onebuff,*p2 =onebuff+4;
			unsigned short *p3 = fourbuff+2,*p4 =fourbuff+6;
			unsigned short *p5 = sixbuff+4;

			#pragma unroll
			for(i=1;i<outputwidth-1;i++,p1+=4,p2+=4,p3+=4,p4+=4,p5+=4){
				unsigned short c = *p1 + *p2 + *p3 + *p4 + *p5 + 8;
				row4[i] =(c>>4);
			}

			i=0;
			a = onebuff[4*i+4]<<1;
			b = fourbuff[4*i+2]<<1;
			c = a + b+ sixbuff[4*i] +8;
			row4[0] = (unsigned char)(c>>4);//reflect
			i = outputwidth-1;
			a = onebuff[4*i-4]+inptr[4*i];
			b = fourbuff[4*i-2]+fourbuff[4*i+2];
			c = a + b+ sixbuff[4*i]+8;
			row4[outputwidth-1] = (unsigned char)(c>>4);//reflect


	#else

			for(i=1;i<outputwidth-1;i++){
				unsigned short a = inptr[4*i-4] + inptr[4*i+4];
				unsigned short b = inptr[4*i-2] + inptr[4*i+2];
				int c = a + (b<<2)+inptr[4*i]*6 +8;
				row4[i] = (unsigned short)(c>>4);
			}
			i=0;
			a = inptr[4*i+4]<<1;
			b = inptr[4*i+2]<<1;
			c = a + (b<<2)+ inptr[4*i]*6 +8;
			row4[0] = (unsigned short)(c>>4);//reflect
			i = outputwidth-1;
			a = inptr[4*i-4]+inptr[4*i];
			b = inptr[4*i-2]+inptr[4*i+2];
			c = a + (b<<2)+ inptr[4*i]*6 +8;
			row4[outputwidth-1] = (unsigned short)(c>>4);//reflect
	#endif
		}
	}

	unsigned short *outptr = (unsigned short*)out + (outputheight -1)*outputstep;

#ifdef __ARM__
	int wloop = (outputwidth>>3);
	loopvar = outputwidth -(wloop<<3);

	int k = 0;

	for(i=0;i<wloop;i++){
		r0 = vld1q_u16((const uint16_t *)row0+k);
		r1 = vld1q_u16((const uint16_t *)row1+k);
		r3 = vld1q_u16((const uint16_t *)row3+k);
		r2 = vld1q_u16((const uint16_t *)row2+k);

		res = vmulq_u16 (r0, one);
		res = vmlaq_u16 (res,r2, one);
		res = vmlaq_u16 (res,r1, four);
		res = vmlaq_u16 (res,r3, four);
		res = vmlaq_u16 (res,r2, six);
		r4 = vrshrq_n_u16 (res,4);
		vst1q_u16((uint16_t *)(outptr+k),r4);
		k+=8;
	}

#else
	loopvar = outputwidth;
#endif
	for(i=0;i<loopvar;i++){
		unsigned short a = row0[i] + row2[i];
		unsigned short b = row1[i] + row3[i];
		int c = a + (b<<2)+row2[i]*6 +8;
		*outptr++ = (unsigned short)(c>>4);
	}
}
#endif


void GetExtractedLine(unsigned short *inptr,unsigned short *even16,unsigned short *even4,int outwidth){


#ifdef __ARM__

	GetExtractedLineASM(inptr,even16+1,even4+1,outwidth);
	*even16 = *(even16+2); //first replicated
	*even4 = *(even4+1);   //first replicated

	*(even16+outwidth+1)= *(even16+outwidth);//last replicated
	*(even4+outwidth+1) = *(even4+outwidth);//last replicated
#else
	unsigned short *e16=even16,*e4=even4;
	unsigned short *ptr1 = inptr;
	unsigned short *ptr2 = inptr+2;

	*e16++ = 0;
	*e4++ = 0;

	for(int i=0;i<outwidth;i++,ptr1+=4,ptr2+=4){
		*e16++ = *ptr1;
		*e4++ = *ptr2;
	}
	*even16 = *(even16+2); //first replicated
	*even4 = *(even4+1);   //first replicated

	*(even16+outwidth+1)= *(e16-1);//last replicated
	*(even4+outwidth+1) = *(e4-1);//last replicated

#endif

}

#ifdef __ARM__
void print_uint16 (uint16x8_t data, char* name) {
    int i;
    static uint16_t p[16];

    vst1q_u16 (p, data);

    printf ("%s = ", name);
    for (i = 0; i < 8; i++) {
    	printf ("%04d ", p[i]);
    }
    printf ("\n");
}
#endif


void DoHorzSubSampling(unsigned short *even16,unsigned short *even4, unsigned short *out,int width){
	//evan16 = e a e i m q
	//even4  = c c g k o s
	//out = e1 c4 a6 c4 e1 , a1 c4 e6 g4 i1 , e1 g4 i6 k4 m1, i1 k4 m6 o4 q1
#ifdef __ARM__
	uint16x8_t six8 = vdupq_n_u16((uint16_t)6);
	uint16x8_t four8 = vdupq_n_u16((uint16_t)4);
	uint16x8_t one8 = vdupq_n_u16((uint16_t)1);
	unsigned short *minus4,*minus2,*zero,*plus2,*plus4;

	minus4 = even16;
	minus2 = even4;
	zero = minus4 +1;
	plus2 = minus2 + 1;
	plus4 = minus4 + 2;

	register uint16x8_t res;
	for(int i=0;i<width;i+=8,out+=8,minus4+=8,plus4+=8,minus2+=8,plus2+=8,zero+=8){
		res = vmulq_u16 (vld1q_u16(minus4), one8);
		res = vmlaq_u16 (res,vld1q_u16(plus4), one8);
		res = vmlaq_u16 (res,vld1q_u16(minus2), four8);
		res = vmlaq_u16 (res,vld1q_u16(plus2), four8);
		res = vmlaq_u16 (res,vld1q_u16(zero), six8);
		vst1q_u16(out,res);
	}
#else
	for(int i=0;i<width;i++){
		unsigned short val = even16[i] + 6*even16[i+1] + even16[i+2] + 4*even4[i] + 4*even4[i+1];
		*out++ = val;
	}
#endif
}

void DoVertSubSampling(unsigned short *row0,unsigned short *row1,unsigned short *row2,unsigned short *row3,unsigned short *row4,unsigned short *out,int width){

#ifdef __ARM__
	int wloop = (width>>2);
	int k = 0;

	uint16x4_t six,four,one,r0,r1,r2,r3,r4;
	uint32x4_t res;
	six = vdup_n_u16((uint16_t)6);
	four = vdup_n_u16((uint16_t)4);
	one = vdup_n_u16((uint16_t)1);

	for(int i=0;i<wloop;i++){
		r0 = vld1_u16((const uint16_t *)row0+k);
		r1 = vld1_u16((const uint16_t *)row1+k);
		r3 = vld1_u16((const uint16_t *)row3+k);
		r2 = vld1_u16((const uint16_t *)row2+k);
		r4 = vld1_u16((const uint16_t *)row4+k);

		res = vmull_u16 (r0, one);
		res = vmlal_u16 (res,r4, one);
		res = vmlal_u16 (res,r1, four);
		res = vmlal_u16 (res,r3, four);
		res = vmlal_u16 (res,r2, six);
		r4 = vrshrn_n_u32 (res,8);
		vst1_u16((uint16_t *)(out+k),r4);
		k+=4;
	}
#else
	for(int i=0;i<width;i++){
		unsigned int val = row0[i] + 4*row1[i] + 6*row2[i] + 4*row3[i] + row4[i]+63;
		*out++ = val>>8;
	}
#endif
}


void Copy16bitTo8bit(unsigned short *in,unsigned char *out,int width){
	#ifdef __ARM__
		register uint16x8_t res;
		register uint8x8_t res1;

		for(int i=0;i<width;i+=8,out+=8,in+=8){
			res = vld1q_u16(in);
			res1 = vrshrn_n_u16(res,4);
//			res1 = vmovn_u16(res);
			vst1_u8(out,res1);
		}
	#else
		for(int i=0;i<width;i++){
			*out++ = *in++;
		}
	#endif
}


void reduce_gauss5_16bit_4x_To_8bit(unsigned short* inp,unsigned short* out, int *param)
{
	int i,inputwidth,inputstep,inputheight,outputstep,outputwidth,outputheight;
	int outbits = param[5];

	inputwidth = param[0];
	inputstep = param[1]>>1;
	inputheight = param[2];
	if(outbits)
		outputstep = param[3];
	else
		outputstep = param[3]>>1;
	outputwidth = inputwidth>>2;
	outputheight = inputheight>>2;


	unsigned short *buffer = (unsigned short*)param[4];
	unsigned short *row0,*row1,*row2,*row3,*row4;
	row0 = buffer;
	row1 = row0 + (outputwidth) ; //for reflection
	row2 = row1 + (outputwidth) ; //for reflection
	row3 = row2 + (outputwidth) ; //for reflection
	row4 = row3 + (outputwidth) ; //for reflection

	unsigned short *even16,*even4;
	even16 = row4 + outputwidth + 4;
	even4 = even16 + outputwidth + 4;
	unsigned short *testline = even4 + outputwidth + 4;

	//comp row0
	unsigned short* inptr = (unsigned short*)inp;
	GetExtractedLine(inptr,even16,even4,outputwidth);
	DoHorzSubSampling(even16,even4,row2,outputwidth);

	inptr = (unsigned short*)inp + 2*inputstep;
	GetExtractedLine(inptr,even16,even4,outputwidth);
	DoHorzSubSampling(even16,even4,row3,outputwidth);


	inptr = (unsigned short*)inp + 4*inputstep;
	GetExtractedLine(inptr,even16,even4,outputwidth);
	DoHorzSubSampling(even16,even4,row4,outputwidth);

	memcpy(row1,row3,outputwidth*2);
	memcpy(row0,row4,outputwidth*2);

	unsigned short *baseptr = (unsigned short*)inp + 6*inputstep;

	int loopvar;
	for(int j=0;j<outputheight-1;j++)
	{
		unsigned short *USoutptr = (unsigned short*)out + j*outputstep;
		unsigned char *UCoutptr = (unsigned char*)out + j*outputstep;
		unsigned short *optr = USoutptr;
		if(outbits)
			optr = testline;

		DoVertSubSampling(row0,row1,row2,row3,row4,optr,outputwidth);

		if(outbits)
			Copy16bitTo8bit(optr,UCoutptr,outputwidth);


		unsigned short *temp0 = row0,*temp1 = row1,*temp2 = row2,*temp3 = row3,*temp4 = row4;
		row0 = temp2;
		row1 = temp3;
		row2 = temp4;
		row3 = temp0;
		row4 = temp1;

		inptr = (unsigned short*)baseptr + (4*j)*inputstep;

		GetExtractedLine(inptr,even16,even4,outputwidth);
		DoHorzSubSampling(even16,even4,row3,outputwidth);

		if( (outputheight-2) != j) // for last row skip it.
		{
			inptr = (unsigned short*)baseptr + (4*j+2)*inputstep;
			GetExtractedLine(inptr,even16,even4,outputwidth);
			DoHorzSubSampling(even16,even4,row4,outputwidth);
		}
	}

	unsigned short *USoutptr = (unsigned short*)out + (outputheight -1)*outputstep;
	unsigned char *UCoutptr = (unsigned char*)out + (outputheight -1)*outputstep;
	unsigned short *optr = USoutptr;
	if(outbits)
		optr = testline;

	DoVertSubSampling(row0,row1,row2,row3,row2,optr,outputwidth);

	if(outbits)
		Copy16bitTo8bit(optr,UCoutptr,outputwidth);

}


//
//void copy16to8(IplImage *src, IplImage *dst,CvRect *roi){
//
//	for(int i=0;i<roi->height;i++){
//		unsigned char *cptr = (unsigned char *)(dst->imageData + i*dst->widthStep);
//		unsigned short *sptr = (unsigned short *)(src->imageData + (i+roi->y)*src->widthStep + 2*roi->x);
//		for(int j=0;j<roi->width;j++){
//			unsigned short a = *sptr++;
//			*cptr++ = a>>8;
//		}
//	}
//}

void ScaleCodeLow(unsigned char *inp,unsigned char *out,int cnt){
	for(int i=0;i<cnt;i++){
		out[2*i] = (inp[i]&0xF);
		out[2*i+1] = ((inp[i]&0xF0)>>4);
	}
}
void ScaleCodeHigh(unsigned char *inp,unsigned char *out,int cnt){
	for(int i=0;i<cnt;i++){
		out[2*i] = (inp[i]&0xF)<<4;
		out[2*i+1] = ((inp[i]&0xF0)>>4)<<4;
//		printf("%#x %#x %#x %#x \n",inp[i]&0xF ,out[2*i],(inp[i]&0xF0)>>4,out[2*i+1]);
	}
}

int GetCoarseIrisCodeFromCompactDBAsm(unsigned char *iris, unsigned char *coarse,int len){
	unsigned char *ic = iris;
	for (int i = 0; i < (len>>2); i++, ic += 2) {
		coarse[i] = ((ic[0] & 0x0C)<<4) | ((ic[0] & 0xC0) >> 2) | (ic[1] & 0x0C) | ((ic[1] & 0xC0) >> 6);
	}
}

void Convert16to8_ASM(unsigned short *pSrc, unsigned char *pDest, int count, unsigned short dc, const int shift){
	unsigned char *p8 = pDest;
	unsigned short *p16 = (unsigned short *)pSrc;
	for(int i = 0; i < count; i++, p8++, p16++)	{
	//unsigned short tmp = ((*p16 - dc) >> shift);
		if(*p16<dc)
			*p8=0;
		else{
			unsigned short tmp = ((*p16 - dc));
			tmp = tmp >>shift;

			*p8 = (tmp & 0xff00) ? ((unsigned char)0xff): ((unsigned char) (tmp & 0x00ff)) ;
		}
	}
}

#define NOT_IMPL_FIXED 	bool fixed_version_not_done=false; assert(fixed_version_not_done)

void gaus5_expand_reflect(unsigned char *image, unsigned char* scratch, dims* pDims){
	NOT_IMPL_FIXED;
}

#endif
