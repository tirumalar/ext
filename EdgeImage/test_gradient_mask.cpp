#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

extern "C"{
#include "test_fw.h"
#include "EdgeImage_private.h"
};
#include "image.h"

extern unsigned short SQRT_LUT_0_64[65*65];
extern unsigned short SQRT_LUT_0_256[65*65];
extern unsigned short SQRT_LUT_0_256_eyelid[256*256];

static short output [256] = {0};
static int output1 [256] = {0};
static unsigned short testoutput [256] = {0};
static unsigned int testoutput1 [256] = {0};

#ifdef __BFIN__
void reduce_gauss5_4x(unsigned char* inp,unsigned char* out, int *param){}
void reduce_gauss5_2x(unsigned char* inp,unsigned char* out, int *param){}
#endif


void test_horizontal_gradient_image_mask_64_advance()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,0,0,1536,11520,1536,5632,8448,7168,0,8448,7168,7168,9984,0,0,0,0,
0,0,0,0,5376,0,2304,9216,1792,2048,3328,11008,14080,6400,2816,6912,0,0,0,0,
0,0,0,0,0,1536,0,1536,0,0,7168,0,8448,7168,7168,9984,0,0,0,0,
0,0,0,0,5376,5120,2304,9216,1792,2048,3328,11008,14080,6400,2816,0,0,0,0,0,
};


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_64(&ref[4+inputstep], &testoutput[4], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_64_advance - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_64_advance1()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};


unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,1536,11520,1536,5632,8448,7168,0,8448,0,7168,9984,0,0,
0,0,5376,0,2304,0,0,2048,0,0,0,0,2816,6912,0,0,
0,0,0,1536,0,1536,0,0,7168,0,0,0,7168,9984,0,0,
0,0,5376,5120,2304,9216,1792,0,3328,11008,14080,6400,2816,0,0,0,
};

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_64(&ref[4+inputstep], &testoutput[2], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_64_advance1 - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_64_advance2()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short ix[]={
0,0,0,0,0,16384,0,16384,16384,16384,16384,0,16384,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,0,16384,16384,0,16384,0,0,0,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,16384,0,0,0,0,0,0,16384,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,0,16384,16384,16384,16384,16384,0,0,16384,0,0,0,0,0,0,
};


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_64(&ref[4+inputstep], &testoutput[4], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_64_advance2 - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_64_advance3()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short ix[]={
0,0,0,16384,16384,0,16384,16384,0,0,16384,0,16384,16384,0,0,
0,0,16384,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,16384,0,0,0,0,0,0,0,0,16384,16384,0,0,
0,0,16384,0,0,16384,0,0,16384,0,0,16384,0,0,0,0,
};

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_64(&ref[4+inputstep], &testoutput[2], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_64_advance3 - output match:", ix, (short*)testoutput, w*h));
}


void test_vertical_gradient_image_mask_64_advance()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,0,8448,3072,7424,7680,3072,3584,7168,2048,2816,6912,0,0,0,0,0,
0,0,0,0,5120,0,0,1536,0,2816,0,3584,6400,0,3072,3840,0,0,0,0,
0,0,0,0,2560,0,0,3328,0,0,5376,10240,2560,14080,7424,8448,0,0,0,0,
0,0,0,0,6400,15360,2560,4352,10752,9984,7168,12032,1536,1536,5888,0,0,0,0,0,
};

	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	compute_vertical_gradient_image_mask_64(&ref[4+inputstep], &testoutput[4], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_64_advance - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}

void test_vertical_gradient_image_mask_64_advance1()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,0,8448,3072,7424,7680,3072,3584,0,2048,0,6912,0,0,0,
0,0,5120,0,0,0,0,2816,0,0,0,0,3072,3840,0,0,
0,0,0,0,0,3328,0,0,5376,0,0,0,7424,8448,0,0,
0,0,6400,15360,2560,4352,10752,0,7168,12032,1536,1536,5888,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	compute_vertical_gradient_image_mask_64(&ref[4+inputstep], &testoutput[2], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_64_advance1 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}


void test_vertical_gradient_image_mask_64_advance2()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,5120,0,0,1536,0,2816,0,0,0,0,3072,3840,0,0,0,0,
0,0,0,0,2560,0,0,0,0,0,0,0,2560,14080,7424,8448,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	compute_vertical_gradient_image_mask_64(&ref[4+inputstep], &testoutput[4], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_64_advance2 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}



void test_vertical_gradient_image_mask_64_advance3()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,16384,0,0,16384,0,0,0,0,16384,16384,0,0,
0,0,0,16384,0,0,0,0,0,0,0,0,16384,16384,0,0,
0,0,16384,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_64;

	compute_vertical_gradient_image_mask_64(&ref[4+inputstep], &testoutput[2], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_64_advance3 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}


void test_magnitude_gradient_image_mask_64_advance()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned short mag[]={
0,0,0,0,0,8587,11923,7581,9524,8989,8014,7209,8693,7701,9958,9987,0,0,0,0,
0,0,0,0,7424,0,2360,9343,1950,3482,3367,11577,15466,6405,4167,7907,0,0,0,0,
0,0,0,0,2611,1717,0,3665,0,0,8960,10269,8827,15800,10320,13079,0,0,0,0,
0,0,0,0,8358,16191,3444,10192,10900,10192,7903,16308,14164,6582,6527,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_64;
	dstParam[6] = (int)&mask[4+inputstep];

	compute_magnitude_gradient_image_mask_64(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_64_advance - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_64_advance1()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,8587,11923,7581,9524,8989,8014,0,8693,0,9958,9987,0,0,
0,0,7424,0,2360,0,0,3482,0,0,0,0,4167,7907,0,0,
0,0,0,1717,0,3665,0,0,8960,0,0,0,10320,13079,0,0,
0,0,8358,16191,3444,10192,10900,0,7903,16308,14164,6582,6527,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_64;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask_64(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_64_advance1 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}


void test_magnitude_gradient_image_mask_64_advance2()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned short mag[]={
0,0,0,0,0,16384,0,16384,16384,16384,16384,0,16384,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,0,16384,16384,0,16384,0,0,0,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,16384,0,0,0,0,0,0,16384,16384,16384,16384,0,0,0,0,
0,0,0,0,16384,0,16384,16384,16384,16384,16384,0,0,16384,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_64;
	dstParam[6] = (int)&mask[4+inputstep];

	compute_magnitude_gradient_image_mask_64(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_64_advance2 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_64_advance3()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,16384,16384,0,16384,16384,0,0,16384,0,16384,16384,0,0,
0,0,16384,0,16384,0,0,16384,0,0,0,0,16384,16384,0,0,
0,0,0,16384,0,0,0,0,0,0,0,0,16384,16384,0,0,
0,0,16384,0,0,16384,0,0,16384,0,0,16384,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_64;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask_64(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_64_advance3 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}



//256 wala
void test_horizontal_gradient_image_mask_256_advance()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,0,0,768,5760,768,2816,4224,3584,0,4224,3584,3584,4992,0,0,0,0,
0,0,0,0,2688,0,1152,4608,896,1024,1664,5504,7040,3200,1408,3456,0,0,0,0,
0,0,0,0,0,768,0,768,0,0,3584,0,4224,3584,3584,4992,0,0,0,0,
0,0,0,0,2688,2560,1152,4608,896,1024,1664,5504,7040,3200,1408,0,0,0,0,0,
};



	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask(&ref[4+inputstep], &testoutput[4], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_256_advance - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_256_advance1()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};


unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,768,5760,768,2816,4224,3584,0,4224,0,3584,4992,0,0,
0,0,2688,0,1152,0,0,1024,0,0,0,0,1408,3456,0,0,
0,0,0,768,0,768,0,0,3584,0,0,0,3584,4992,0,0,
0,0,2688,2560,1152,4608,896,0,1664,5504,7040,3200,1408,0,0,0,
};


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask(&ref[4+inputstep], &testoutput[2], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_256_advance1 - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_256_advance2()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short ix[]={
0,0,0,0,0,27008,0,27264,27264,27648,28672,0,26880,31104,26752,29568,0,0,0,0,
0,0,0,0,28800,0,29184,25600,0,27392,0,0,0,28800,24576,32512,0,0,0,0,
0,0,0,0,24832,27008,0,0,0,0,0,0,26880,31104,26752,29568,0,0,0,0,
0,0,0,0,28800,0,29184,25600,27520,27392,26496,0,0,28800,0,0,0,0,0,0,
};



	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask(&ref[4+inputstep], &testoutput[4], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_256_advance2 - output match:", ix, (short*)testoutput, w*h));
}

void test_horizontal_gradient_image_mask_256_advance3()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short ix[]={
0,0,0,32640,32640,0,32640,32640,0,0,32640,0,32640,32640,0,0,
0,0,32640,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,32640,0,0,0,0,0,0,0,0,32640,32640,0,0,
0,0,32640,0,0,32640,0,0,32640,0,0,32640,0,0,0,0,
};

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask(&ref[4+inputstep], &testoutput[2], dstParam);
	tut::ensure(ensure_results_short("test_horizontal_gradient_image_mask_64_advance3 - output match:", ix, (short*)testoutput, w*h));
}


void test_vertical_gradient_image_mask_256_advance()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,0,4224,1536,3712,3840,1536,1792,3584,1024,1408,3456,0,0,0,0,0,
0,0,0,0,2560,0,0,768,0,1408,0,1792,3200,0,1536,1920,0,0,0,0,
0,0,0,0,1280,0,0,1664,0,0,2688,5120,1280,7040,3712,4224,0,0,0,0,
0,0,0,0,3200,7680,1280,2176,5376,4992,3584,6016,768,768,2944,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	compute_vertical_gradient_image_mask(&ref[4+inputstep], &testoutput[4], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_256_advance - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}

void test_vertical_gradient_image_mask_256_advance1()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,0,4224,1536,3712,3840,1536,1792,0,1024,0,3456,0,0,0,
0,0,2560,0,0,0,0,1408,0,0,0,0,1536,1920,0,0,
0,0,0,0,0,1664,0,0,2688,0,0,0,3712,4224,0,0,
0,0,3200,7680,1280,2176,5376,0,3584,6016,768,768,2944,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	compute_vertical_gradient_image_mask(&ref[4+inputstep], &testoutput[2], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_256_advance1 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}


void test_vertical_gradient_image_mask_256_advance2()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,2560,0,0,768,0,1408,0,0,0,0,1536,1920,0,0,0,0,
0,0,0,0,1280,0,0,0,0,0,0,0,1280,7040,3712,4224,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	compute_vertical_gradient_image_mask(&ref[4+inputstep], &testoutput[4], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_256_advance2 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}



void test_vertical_gradient_image_mask_256_advance3()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,32640,0,0,32640,0,0,0,0,32640,32640,0,0,
0,0,0,32640,0,0,0,0,0,0,0,0,32640,32640,0,0,
0,0,32640,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256;

	compute_vertical_gradient_image_mask(&ref[4+inputstep], &testoutput[2], dstParam);

	tut::ensure(ensure_results_short("test_vertical_edge_image_mask_256_advance3 - output match:", iy, (short*)testoutput, (outputstep*h)>>1));
}


void test_magnitude_gradient_image_mask_256_advance()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned short mag[]={
0,0,0,0,0,4293,5961,3791,4762,4495,4007,3605,4346,3851,4979,4994,0,0,0,0,
0,0,0,0,3712,0,1180,4672,975,1741,1684,5788,7733,3203,2084,3954,0,0,0,0,
0,0,0,0,1305,859,0,1833,0,0,4480,5134,4414,7900,5160,6539,0,0,0,0,
0,0,0,0,4179,8095,1722,5096,5450,5096,3951,8154,7082,3291,3263,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_256_advance - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_256_advance1()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,4293,5961,3791,4762,4495,4007,0,4346,0,4979,4994,0,0,
0,0,3712,0,1180,0,0,1741,0,0,0,0,2084,3954,0,0,
0,0,0,859,0,1833,0,0,4480,0,0,0,5160,6539,0,0,
0,0,4179,8095,1722,5096,5450,0,3951,8154,7082,3291,3263,0,0,0,
};

	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_64_advance1 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}


void test_magnitude_gradient_image_mask_256_advance2()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned short mag[]={
0,0,0,0,0,27008,0,27264,27264,27648,28672,0,26880,31104,26752,29568,0,0,0,0,
0,0,0,0,28914,0,29185,25612,0,27428,0,0,0,28800,24624,32569,0,0,0,0,
0,0,0,0,24865,27011,0,0,0,0,0,0,26910,31891,27008,29868,0,0,0,0,
0,0,0,0,28800,0,29184,25600,27520,27392,26496,0,0,28800,0,0,0,0,0,0,
};




	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256;
	dstParam[6] = (int)&mask[4+inputstep];

	compute_magnitude_gradient_image_mask(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_256_advance2 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_256_advance3()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,32640,32640,0,32640,32640,0,0,32640,0,32640,32640,0,0,
0,0,32640,0,32640,0,0,32640,0,0,0,0,32640,32640,0,0,
0,0,0,46160,0,0,0,0,0,0,0,0,46160,46160,0,0,
0,0,46160,0,0,32640,0,0,32640,0,0,32640,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256;
	dstParam[6] = (int)&mask[4+inputstep];

	compute_magnitude_gradient_image_mask(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_256_advance3 - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

//paste




void test_horizontal_gradient_image_mask_advance_eyelid()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,0,768,768,5760,0,2816,4224,0,384,4224,0,3584,3584,0,0,0,0,
0,0,0,0,0,0,1152,4608,0,0,1664,0,0,0,1408,1408,0,0,0,0,
0,0,0,0,0,0,0,768,0,0,3584,0,0,3584,0,0,0,0,0,0,
0,0,0,0,2560,2560,0,0,896,1024,0,5504,7040,3200,0,0,0,0,0,0,
};


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[4], dstParam);
	ensure_results_short("test_horizontal_gradient_image_mask_advance_eyelid - output match:", ix, (short*)testoutput, w*h);
}


void test_horizontal_gradient_image_mask_advance1_eyelid()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};


unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,768,768,5760,0,2816,4224,0,0,4224,0,3584,3584,0,0,
0,0,0,0,1152,0,0,0,0,0,0,0,1408,1408,0,0,
0,0,0,0,0,768,0,0,3584,0,0,0,0,0,0,0,
0,0,2560,2560,0,0,896,0,0,5504,7040,3200,0,0,0,0,
};

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[2], dstParam);
	ensure_results_short("test_horizontal_gradient_image_mask_advance1_eyelid - output match:", ix, (short*)testoutput, w*h);
}

void test_horizontal_gradient_image_mask_advance2_eyelid()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short ix[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,27008,27008,0,0,0,0,0,0,26880,31104,26752,26752,0,0,0,0,
0,0,0,0,0,0,29184,25600,27520,27392,26496,0,0,28800,0,0,0,0,0,0,
};


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[4], dstParam);
	ensure_results_short("test_horizontal_gradient_image_mask_advance2_eyelid - output match:", ix, (short*)testoutput, w*h);
}

void test_horizontal_gradient_image_mask_advance3_eyelid()
{
	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;
	short output[256];

	int dstParam[6];

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short ix[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,32640,32640,0,0,0,0,0,0,0,0,32640,32640,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}

	compute_horizontal_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[2], dstParam);
	ensure_results_short("test_horizontal_gradient_image_mask_advance3_eyelid - output match:", ix, (short*)testoutput, w*h);
}



void test_vertical_gradient_image_mask_advance_eyelid()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,4224,4224,1536,3712,3840,1536,1792,3584,1024,1408,3456,3456,0,0,0,0,
0,0,0,0,0,0,256,768,384,1408,256,1792,3200,128,1536,1536,0,0,0,0,
0,0,0,0,384,384,0,1664,0,0,2688,5120,1280,7040,3712,3712,0,0,0,0,
0,0,0,0,7680,7680,1280,2176,5376,4992,3584,6016,768,768,2944,2944,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	compute_vertical_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[4], dstParam);

	ensure_results_short("test_vertical_edge_image_mask_advance_eyelid - output match:", iy, (short*)testoutput, (outputstep*h)>>1);
}


void test_vertical_gradient_image_mask_advance1_eyelid()
{

unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,4224,4224,1536,3712,3840,1536,1792,0,1024,0,3456,3456,0,0,
0,0,0,0,256,0,0,1408,0,0,0,0,1536,1536,0,0,
0,0,384,384,0,1664,0,0,2688,0,0,0,3712,3712,0,0,
0,0,7680,7680,1280,2176,5376,0,3584,6016,768,768,2944,2944,0,0,
};





	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	compute_vertical_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[2], dstParam);

	ensure_results_short("test_vertical_edge_image_mask_advance1_eyelid - output match:", iy, (short*)testoutput, (outputstep*h)>>1);
}


void test_vertical_gradient_image_mask_advance2_eyelid()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,256,768,0,1408,0,0,0,128,1536,1536,0,0,0,0,
0,0,0,0,384,384,0,0,0,0,0,0,1280,7040,3712,3712,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	compute_vertical_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[4], dstParam);

	ensure_results_short("test_vertical_edge_image_mask_advance2_eyelid - output match:", iy, (short*)testoutput, (outputstep*h)>>1);
}



void test_vertical_gradient_image_mask_advance3_eyelid()
{

unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


short iy[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,32640,0,0,32640,0,0,0,0,32640,32640,0,0,
0,0,32640,32640,0,0,0,0,0,0,0,0,32640,32640,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)&mask[4+inputstep];
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;

	compute_vertical_gradient_image_mask_eyelid(&ref[4+inputstep], &testoutput[2], dstParam);

	ensure_results_short("test_vertical_edge_image_mask_advance3_eyelid - output match:", iy, (short*)testoutput, (outputstep*h)>>1);
}



void test_magnitude_gradient_image_mask_advance_eyelid()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};
unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned short mag[]={
0,0,0,0,4293,4293,5961,3712,4762,4495,1792,3605,4346,1408,4979,4979,0,0,0,0,
0,0,0,0,0,0,1180,4672,384,1408,1684,1792,3200,128,2084,2084,0,0,0,0,
0,0,0,0,384,384,0,1833,0,0,4480,5120,1280,7900,3712,3712,0,0,0,0,
0,0,0,0,8095,8095,1280,2176,5450,5096,3584,8154,7082,3291,2944,2944,0,0,0,0,
};


	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask_eyelid(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_advance_eyelid - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_advance1_eyelid()
{
unsigned char ref[]={
50,50,50,50,63,38,3,48,20,6,59,34,12,40,18,63,50,50,50,50,
63,55,52,28,51,31,18,19,47,49,35,63,63,55,52,28,51,31,18,19,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
12,40,18,63,63,38,3,48,20,6,59,34,12,40,18,63,63,38,3,48,
50,50,50,50,51,31,18,19,47,49,35,63,63,55,52,28,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,4293,4293,5961,3712,4762,4495,1792,0,4346,0,4979,4979,0,0,
0,0,0,0,1180,0,0,1408,0,0,0,0,2084,2084,0,0,
0,0,384,384,0,1833,0,0,4480,0,0,0,3712,3712,0,0,
0,0,8095,8095,1280,2176,5450,0,3584,8154,7082,3291,2944,2944,0,0,
};

	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask_eyelid(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_advance1_eyelid - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}


void test_magnitude_gradient_image_mask_advance2_eyelid()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
45,12,46,24,61,44,48,42,42,39,31,37,45,12,46,24,61,44,48,42,
8,30,63,1,30,11,27,55,40,41,48,20,8,30,63,1,30,11,27,55,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,256,768,0,1408,0,0,0,128,1536,1536,0,0,0,0,
0,0,0,0,27011,27011,0,0,0,0,0,0,26910,31891,27008,27008,0,0,0,0,
0,0,0,0,0,0,29184,25600,27520,27392,26496,0,0,28800,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 40;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;
	dstParam[6] = (int)&mask[4+inputstep];

	compute_magnitude_gradient_image_mask_eyelid(&ref[4+inputstep], &out[4], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_advance2_eyelid - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}

void test_magnitude_gradient_image_mask_advance3_eyelid()
{
unsigned char ref[]={
50,50,50,50,255,255,255,255,255,255,255,255,255,255,255,255,50,50,50,50,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,
255,255,255,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,
};

unsigned char mask[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,
0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


unsigned short mag[]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,32640,0,0,32640,0,0,0,0,32640,32640,0,0,
0,0,46160,46160,0,0,0,0,0,0,0,0,46160,46160,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



	int w = 12;
	int h = 4;
	int inputstep = 20;//((w+3)&(~3));
	int outputstep = 32;// (((w*2)+3)&(~3));

	int i,j;

	unsigned short int *out = (unsigned short int*)testoutput1;
	int dstParam[7];

	for(i=0;i<outputstep*h;i++)
	{
		testoutput1[i] = 0;
	}


	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = inputstep;
	dstParam[3] = outputstep;
	dstParam[4] = (int)output1;
	dstParam[5] = (int)SQRT_LUT_0_256_eyelid;
	dstParam[6] = (int)&mask[4+inputstep];



	compute_magnitude_gradient_image_mask_eyelid(&ref[4+inputstep], &out[2], dstParam);
	tut::ensure(ensure_results_short("test_magnitude_gradient_image_mask_advance3_eyelid - output match:", (short *)mag, (short*)out, (outputstep*h)>>1));
}



void test_correlate_gradient_image_eyelid_basic()
{
	int i,j;
	unsigned int Corr[256];
	unsigned int C[256];

	unsigned short Grad[256];
	unsigned char Inp[256];
	unsigned char CountBuff[256],CountBuffAsm[256];
	unsigned short *gptr;
	unsigned char *iptr,*cptr;
	unsigned int *optr;

	usImage gradImage;
	uiImage corrImage;
	Pointparam param;

	for(i=0;i<256;i++)
	{
		Corr[i] = 0;
		C[i] = 0;
		Grad[i] = rand();
		Inp[i] = rand();
		CountBuff[i] = 0;
		CountBuffAsm[i] = 0;
	}

	gradImage.data = Grad;
	gradImage.widthstep = 16*2;

	corrImage.data = Corr;
	corrImage.widthstep = 8;
	//q14 format 0.5
	param.C00 = 0x2000;
	param.C01 = 0x2000;
	param.C10 = 0x2000;
	param.C11 = 0x2000;

	//ptr to cnt buffer
	param.left_offset = (int)CountBuffAsm;
	//ptr to cnt buffer pitch
	param.right_offset = corrImage.widthstep>>2;

	param.sx = 2;
	param.sy = 2;

	param.inpimage = Inp;
	param.widthstep = 12;

	for(i=0;i<param.sy;i++)
	{
		gptr = gradImage.data + i*(gradImage.widthstep>>1);
		optr = corrImage.data + i*(corrImage.widthstep>>2);
		iptr = param.inpimage + i*param.widthstep;
		cptr = CountBuff + i*param.right_offset;
		for(j=0;j<param.sx;j++,iptr++, gptr++, optr++,cptr++)
		{
			unsigned int val = 0,val1 = 0,val2 = 0,val3=0;
			//9Q7*2Q14 = 11Q21
			val = param.C00 * gptr[0] ;
			val1 = param.C01 * gptr[1] ;
			val2 = param.C10 * gptr[gradImage.widthstep>>1];
			val3 = param.C11 * gptr[(gradImage.widthstep>>1)+1];
			val+=(val1+val2+val3);
			val = (val >> 6); // no rounding required
			*optr = (*optr) + val;
			*cptr += 1;
		}
	}
	corrImage.data = C;
	compute_correlate_gradient_image_eyelid(&gradImage, &corrImage, &param);
	tut::ensure(ensure_results_int_allowrounding("test_correlate_gradient_image_eyelid_basic - output match:", (int*)C, (int*)Corr, param.sx*param.sy));
	tut::ensure(ensure_results("test_correlate_gradient_image_eyelid_basic - count match:", CountBuffAsm, CountBuff, param.sx*param.sy));
}

void test_correlate_gradient_image_eyelid_advance()
{
	int i,j;
	unsigned short *gptr;
	unsigned char *iptr;
	unsigned int *optr;
	int C[16*16];
	unsigned char CountBuffAsm[256],CountBuff[256] = {1};

	usImage gradImage;
	uiImage corrImage;
	Pointparam param;

	unsigned short mag[]={
	0,0,3214,2456,2457,2458,2459,2460,2461,2462,2463,2464,0,0,
	0,0,4567,5667,5668,5669,5670,5671,5672,5673,5674,5675,0,0,
	0,0,4545,6987,7786,4545,4234,9888,4545,7786,6769,4234,0,0,
	0,0,8787,8976,4234,9888,6769,7786,1325,9888,5675,4545,0,0,
	0,0,9888,5675,4545,7786,4234,9888,5675,7786,9888,5667,0,0,
	0,0,7786,7897,9888,5667,1325,6769,4234,4545,4234,5675,0,0,
	0,0,1325,5432,5675,7786,9888,5667,1325,7786,5667,6769,0,0,
	0,0,9888,4234,1325,5675,6769,4234,5675,5667,6769,4234,0,0,
	0,0,6769,7678,7786,4234,7786,4545,1325,4234,9888,3214,0,0,
	0,0,9876,7987,5675,1325,6769,4234,4545,3214,1325,7786,0,0,
	};

	unsigned int out[]={
	1883776,2127872,2128320,2128768,2129216,2129664,2130112,2130560,2131008,
	2152320,2986816,3113344,2170112,2261600,3719104,2284160,3185152,2811264,
	3477792,3640480,2428704,3636160,2921504,3379456,1383488,3993408,2565152,
	4160128,2777024,2280992,3543712,2434784,3819200,2327168,3689632,3719328,
	3625920,3280992,3714560,2557440,1321088,3215808,2158784,2508320,2531584,
	1555584,2820544,3014144,3145216,3372896,2459808,1182656,2995520,2436768,
	3284256,1964864,1357152,2914880,3215552,1987424,2192096,2777504,2845600,
	3279904,2909280,2686656,2229952,3173120,1939424,1243232,2296448,3735904,
	3995392,3468096,2559744,1253824,2977856,1843552,1674496,1690912,1682848,
	};

	unsigned char  image[]={
	0,0,1,2,3,4,5,6,7,8,9,10,0,0,
	0,0,2,3,4,5,6,7,8,9,10,11,0,0,
	0,0,3,4,5,6,7,8,9,10,11,12,0,0,
	0,0,4,5,6,7,8,9,10,11,12,13,0,0,
	0,0,5,6,7,8,9,10,11,12,13,14,0,0,
	0,0,6,7,8,9,10,11,12,13,14,15,0,0,
	0,0,7,8,9,10,11,12,13,14,15,16,0,0,
	0,0,8,9,10,11,12,13,14,15,16,17,0,0,
	0,0,9,10,11,12,13,14,15,16,17,18,0,0,
	0,0,10,11,12,13,14,15,16,17,18,19,0,0,
	};

	gradImage.data = &mag[2];
	gradImage.widthstep = 28;

	//17Q15
	param.C00 = 6144;
	param.C01 = 2048;
	param.C10 = 18432;
	param.C11 = 2048;

	//ptr to cnt buffer
	param.left_offset = (int)CountBuffAsm;
	//ptr to cnt buffer pitch
	param.right_offset = 9;

	param.sx = 9;
	param.sy = 9;

	param.inpimage = &image[2];
	param.widthstep = 14;

	for(i=0;i<param.sx*param.sy;i++)
	{
		C[i] = 0;
		CountBuff[i] = 2;
		CountBuffAsm[i]=1;
	}

	corrImage.data = (unsigned int*)C;
	corrImage.widthstep = 36;

	compute_correlate_gradient_image_eyelid(&gradImage, &corrImage, &param);
	tut::ensure(ensure_results_int_allowrounding("test_correlate_gradient_image_eyelid_advance - output match:",(int*) C, (int*)out, param.sx*param.sy));
	tut::ensure(ensure_results("test_correlate_gradient_image_eyelid_advance - count match:", CountBuffAsm, CountBuff, param.sx*param.sy));
}


void test_correlate_gradient_image_eyelid_advance1()
{
	int i,j;
	unsigned short *gptr;
	unsigned char *iptr;
	unsigned int *optr;
	int C[16*16];
	unsigned char CountBuffAsm[256],CountBuff[256] = {1};

	usImage gradImage;
	uiImage corrImage;
	Pointparam param;

	unsigned short mag[]={
0,0,3214,2456,2457,2458,2459,2460,2461,2462,2463,2464,0,0,
0,0,4567,5667,5668,5669,5670,5671,5672,5673,5674,5675,0,0,
0,0,4545,6987,7786,4545,4234,9888,4545,7786,6769,4234,0,0,
0,0,8787,8976,4234,9888,6769,7786,1325,9888,5675,4545,0,0,
0,0,9888,5675,4545,7786,4234,9888,5675,7786,9888,5667,0,0,
0,0,7786,7897,9888,5667,1325,6769,4234,4545,4234,5675,0,0,
0,0,1325,5432,5675,7786,9888,5667,1325,7786,5667,6769,0,0,
0,0,9888,4234,1325,5675,6769,4234,5675,5667,6769,4234,0,0,
0,0,6769,7678,7786,4234,7786,4545,1325,4234,9888,3214,0,0,
0,0,9876,7987,5675,1325,6769,4234,4545,3214,1325,7786,0,0,
};


unsigned char  image[]={
0,0,1,2,3,4,5,6,7,8,9,10,0,0,
0,0,2,3,4,5,6,7,8,9,10,11,0,0,
0,0,3,4,5,6,7,8,9,10,11,12,0,0,
0,0,4,5,6,7,8,9,10,11,12,13,0,0,
0,0,5,6,7,8,9,10,11,12,13,14,0,0,
0,0,6,7,8,9,10,11,12,13,14,15,0,0,
0,0,7,8,9,10,11,12,13,14,15,16,0,0,
0,0,8,9,10,11,12,13,14,15,16,17,0,0,
0,0,9,10,11,12,13,14,15,16,17,18,0,0,
0,0,10,11,12,13,14,15,16,17,18,19,0,0,
};

unsigned int out[]={
64394,69937,69952,69968,69983,69998,70013,70029,70044,
87284,103748,91391,73414,100468,102047,91411,102675,84698,
125510,106620,94867,105974,123941,69597,96395,113421,73491,
124641,73594,112682,95583,111227,85860,118266,118668,102022,
108588,113312,118377,57197,91232,84896,84126,94867,79317,
74551,106657,97299,96448,113077,57149,69332,89753,91885,
99054,57127,75120,113421,84855,57138,96898,92011,90236,
95036,85193,89643,95385,84178,58645,57098,106026,88081,
129619,108940,57237,80591,79077,51365,60777,73235,62859,
};

	gradImage.data = &mag[2];
	gradImage.widthstep = 28;

	//17Q15
	param.C00 = 3;
	param.C01 = 324;
	param.C10 = 324;
	param.C11 = 324;

	param.left_offset = (int)CountBuffAsm;;
	param.right_offset = 9;

	param.sx = 9;
	param.sy = 9;

	param.inpimage = &image[2];
	param.widthstep = 14;


	for(i=0;i<param.sx*param.sy;i++)
	{
		C[i] = 0;
		CountBuff[i] = 1;
		CountBuffAsm[i]=0;
	}

	corrImage.data = (unsigned int*)C;
	corrImage.widthstep = 36;

	compute_correlate_gradient_image_eyelid(&gradImage, &corrImage, &param);
	tut::ensure(ensure_results_int_allowrounding("test_correlate_gradient_image_eyelid_advance1 - output match:",(int*) C, (int*)out, param.sx*param.sy));
	tut::ensure(ensure_results("test_correlate_gradient_image_eyelid_advance1 - count match:", CountBuffAsm, CountBuff, param.sx*param.sy));

}

#ifndef __BFIN__

void testGaussReduce4x_basic(void){


	unsigned char input[]={
	54,125,251,5,205,108,170,94,36,209,154,164,178,123,89,45,36,209,154,164,178,123,89,45,172,64,75,253,136,96,64,130,0,0,0,0,
	221,28,52,57,153,13,24,245,242,226,135,159,99,194,133,26,242,226,135,159,99,194,133,26,156,43,199,88,66,191,80,60,0,0,0,0,
	194,134,200,21,255,121,144,92,174,62,174,246,172,10,138,80,174,62,174,246,172,10,138,80,164,172,51,147,96,47,254,195,0,0,0,0,
	219,195,181,118,231,179,47,160,32,160,111,242,126,227,90,109,32,160,111,242,126,227,90,109,158,77,211,153,145,197,195,208,0,0,0,0,
	186,49,66,215,5,193,45,255,201,39,64,244,115,181,168,45,201,39,64,244,115,181,168,45,52,209,41,23,1,47,185,13,0,0,0,0,
	157,48,44,111,71,61,164,33,178,217,75,177,139,84,226,45,178,217,75,177,139,84,226,45,211,34,194,80,81,61,42,209,0,0,0,0,
	242,176,139,36,200,204,151,49,146,127,125,193,65,35,115,95,146,127,125,193,65,35,115,95,32,229,81,219,141,53,25,152,0,0,0,0,
	255,51,40,25,148,182,84,136,152,49,22,128,22,101,49,215,152,49,22,128,22,101,49,215,52,11,157,30,151,98,50,241,0,0,0,0,
	37,178,111,215,87,225,80,182,150,50,240,163,42,251,223,208,150,50,240,163,42,251,223,208,102,14,66,147,220,181,203,153,0,0,0,0,
	240,118,239,169,221,22,224,180,189,83,192,32,107,120,21,164,189,83,192,32,107,120,21,164,125,215,250,67,0,96,94,203,0,0,0,0,
	254,225,227,217,168,115,23,62,155,188,130,108,79,220,191,195,155,188,130,108,79,220,191,195,173,253,129,62,231,93,51,102,0,0,0,0,
	146,136,204,114,95,145,148,77,70,0,172,85,231,226,226,137,70,0,172,85,231,226,226,137,150,165,224,254,47,51,154,123,0,0,0,0,
	230,60,222,99,217,155,118,246,186,143,47,115,242,118,32,113,186,143,47,115,242,118,32,113,41,247,60,169,153,118,89,159,0,0,0,0,
	90,122,110,67,71,199,125,89,202,189,142,72,109,146,216,179,202,189,142,72,109,146,216,179,54,211,91,101,149,75,135,84,0,0,0,0,
	92,52,101,94,180,138,202,248,117,92,99,122,180,204,40,154,117,92,99,122,180,204,40,154,145,74,19,246,135,165,200,134,0,0,0,0,
	184,159,25,227,236,134,31,160,64,83,69,150,48,162,12,127,64,83,69,150,48,162,12,127,185,209,112,122,137,130,205,98,0,0,0,0,
	};

	short W=32;
	short H=16;
	short OW=8;
	short OH=4;

	unsigned char out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];

	unsigned char expected_out[]={
	180,179,144,147,137,149,120,111,0,0,0,0,
	152,132,136,132,144,127,95,94,0,0,0,0,
	154,131,133,131,151,127,116,136,0,0,0,0,
	187,167,133,129,126,126,99,127,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in;
	param[2] = H;
	param[3] = Wstep_out;
	param[4] = (int)scr;

	reduce_gauss5_4x(input,out,param);
	tut::ensure(ensure_results_byte_distance("testGaussReduce4x_basic",out,expected_out,Wstep_out*H/4,3));
}

void testGaussReduce4x_advance(void)
{
	unsigned char input[]={
	54,125,251,5,205,108,170,94,36,209,154,164,178,123,89,45,36,209,154,164,178,123,89,45,172,64,75,253,136,96,64,130,0,0,0,0,
	221,28,52,57,153,13,24,245,242,226,135,159,99,194,133,26,242,226,135,159,99,194,133,26,156,43,199,88,66,191,80,60,0,0,0,0,
	194,134,200,21,255,121,144,92,174,62,174,246,172,10,138,80,174,62,174,246,172,10,138,80,164,172,51,147,96,47,254,195,0,0,0,0,
	219,195,181,118,231,179,47,160,32,160,111,242,126,227,90,109,32,160,111,242,126,227,90,109,158,77,211,153,145,197,195,208,0,0,0,0,
	186,49,66,215,5,193,45,255,201,39,64,244,115,181,168,45,201,39,64,244,115,181,168,45,52,209,41,23,1,47,185,13,0,0,0,0,
	157,48,44,111,71,61,164,33,178,217,75,177,139,84,226,45,178,217,75,177,139,84,226,45,211,34,194,80,81,61,42,209,0,0,0,0,
	242,176,139,36,200,204,151,49,146,127,125,193,65,35,115,95,146,127,125,193,65,35,115,95,32,229,81,219,141,53,25,152,0,0,0,0,
	255,51,40,25,148,182,84,136,152,49,22,128,22,101,49,215,152,49,22,128,22,101,49,215,52,11,157,30,151,98,50,241,0,0,0,0,
	37,178,111,215,87,225,80,182,150,50,240,163,42,251,223,208,150,50,240,163,42,251,223,208,102,14,66,147,220,181,203,153,0,0,0,0,
	240,118,239,169,221,22,224,180,189,83,192,32,107,120,21,164,189,83,192,32,107,120,21,164,125,215,250,67,0,96,94,203,0,0,0,0,
	254,225,227,217,168,115,23,62,155,188,130,108,79,220,191,195,155,188,130,108,79,220,191,195,173,253,129,62,231,93,51,102,0,0,0,0,
	146,136,204,114,95,145,148,77,70,0,172,85,231,226,226,137,70,0,172,85,231,226,226,137,150,165,224,254,47,51,154,123,0,0,0,0,
	230,60,222,99,217,155,118,246,186,143,47,115,242,118,32,113,186,143,47,115,242,118,32,113,41,247,60,169,153,118,89,159,0,0,0,0,
	90,122,110,67,71,199,125,89,202,189,142,72,109,146,216,179,202,189,142,72,109,146,216,179,54,211,91,101,149,75,135,84,0,0,0,0,
	92,52,101,94,180,138,202,248,117,92,99,122,180,204,40,154,117,92,99,122,180,204,40,154,145,74,19,246,135,165,200,134,0,0,0,0,
	184,159,25,227,236,134,31,160,64,83,69,150,48,162,12,127,64,83,69,150,48,162,12,127,185,209,112,122,137,130,205,98,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=8;
	short OH=4;
	unsigned char out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];


	unsigned char expected_out[]={
	180,179,144,147,137,149,120,111,0,0,0,0,
	152,132,136,132,144,127,95,94,0,0,0,0,
	154,131,133,131,151,127,116,136,0,0,0,0,
	187,167,133,129,126,126,99,127,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in;
	param[2] = H;
	param[3] = Wstep_out;
	param[4] = (int)scr;

	reduce_gauss5_4x(input,out,param);
	tut::ensure(ensure_results_byte_distance("test_Guass4x_basic",out,expected_out,(Wstep_out*H)>>2,2));

}


void test_Guass4x_advance(void)
{

	unsigned char input[]={
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=8;
	short OH=4;
	unsigned char out[48];
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];

	unsigned char expected_out[]={
	255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,0,0,0,0,
	255,255,255,255,255,255,255,255,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in;
	param[2] = H;
	param[3] = Wstep_out;
	param[4] = (int)scr;

	reduce_gauss5_4x(input,out,param);
	tut::ensure(ensure_results_byte_distance("test_Guass4x_advance",out,expected_out,(Wstep_out*H)>>2,2));
	param[0]=0;
}


void testGaussReduce2x_basic(void)
{
	unsigned char input[]={
	54,125,251,5,205,108,170,94,36,209,154,164,178,123,89,45,36,209,154,164,178,123,89,45,172,64,75,253,136,96,64,130,0,0,0,0,
	221,28,52,57,153,13,24,245,242,226,135,159,99,194,133,26,242,226,135,159,99,194,133,26,156,43,199,88,66,191,80,60,0,0,0,0,
	194,134,200,21,255,121,144,92,174,62,174,246,172,10,138,80,174,62,174,246,172,10,138,80,164,172,51,147,96,47,254,195,0,0,0,0,
	219,195,181,118,231,179,47,160,32,160,111,242,126,227,90,109,32,160,111,242,126,227,90,109,158,77,211,153,145,197,195,208,0,0,0,0,
	186,49,66,215,5,193,45,255,201,39,64,244,115,181,168,45,201,39,64,244,115,181,168,45,52,209,41,23,1,47,185,13,0,0,0,0,
	157,48,44,111,71,61,164,33,178,217,75,177,139,84,226,45,178,217,75,177,139,84,226,45,211,34,194,80,81,61,42,209,0,0,0,0,
	242,176,139,36,200,204,151,49,146,127,125,193,65,35,115,95,146,127,125,193,65,35,115,95,32,229,81,219,141,53,25,152,0,0,0,0,
	255,51,40,25,148,182,84,136,152,49,22,128,22,101,49,215,152,49,22,128,22,101,49,215,52,11,157,30,151,98,50,241,0,0,0,0,
	37,178,111,215,87,225,80,182,150,50,240,163,42,251,223,208,150,50,240,163,42,251,223,208,102,14,66,147,220,181,203,153,0,0,0,0,
	240,118,239,169,221,22,224,180,189,83,192,32,107,120,21,164,189,83,192,32,107,120,21,164,125,215,250,67,0,96,94,203,0,0,0,0,
	254,225,227,217,168,115,23,62,155,188,130,108,79,220,191,195,155,188,130,108,79,220,191,195,173,253,129,62,231,93,51,102,0,0,0,0,
	146,136,204,114,95,145,148,77,70,0,172,85,231,226,226,137,70,0,172,85,231,226,226,137,150,165,224,254,47,51,154,123,0,0,0,0,
	230,60,222,99,217,155,118,246,186,143,47,115,242,118,32,113,186,143,47,115,242,118,32,113,41,247,60,169,153,118,89,159,0,0,0,0,
	90,122,110,67,71,199,125,89,202,189,142,72,109,146,216,179,202,189,142,72,109,146,216,179,54,211,91,101,149,75,135,84,0,0,0,0,
	92,52,101,94,180,138,202,248,117,92,99,122,180,204,40,154,117,92,99,122,180,204,40,154,145,74,19,246,135,165,200,134,0,0,0,0,
	184,159,25,227,236,134,31,160,64,83,69,150,48,162,12,127,64,83,69,150,48,162,12,127,185,209,112,122,137,130,205,98,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=16;
	short OH=8;
	unsigned char out[160]={0};
	short Wstep_in=36;
	short Wstep_out=16;
	short scr[5*32];


	unsigned char expected_out[]={
	115,103,108,114,166,166,147,108,135,166,147,109,104,123,124,108,

	152,128,136,121,143,159,155,114,123,159,155,114,119,125,120,153,

	134,117,125,125,135,137,155,130,118,137,155,128,114,114,88,123,

	141,98,123,129,125,122,107,116,133,122,107,112,106,118,107,96,

	147,140,146,141,134,121,110,148,143,121,110,144,123,114,125,138,

	187,188,151,118,125,124,138,163,141,124,138,163,167,164,116,108,

	138,139,139,144,141,117,153,150,139,117,153,145,135,150,129,116,

	106,107,146,154,139,112,133,126,126,112,133,126,132,122,144,148,
	};


	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in;
	param[2] = H;
	param[3] = Wstep_out;
	param[4] = (int)scr;

	reduce_gauss5_2x(input,out,param);
	tut::ensure(ensure_results_byte_distance("test_Guass2x_basic",out,expected_out,(Wstep_out*H)>>1,2));

}


void testGaussReduce16bit_4x_basic(void){

	unsigned short input[]={
	54,125,251,5,205,108,170,94,36,209,154,164,178,123,89,45,36,209,154,164,178,123,89,45,172,64,75,253,136,96,64,130,0,0,0,0,
	221,28,52,57,153,13,24,245,242,226,135,159,99,194,133,26,242,226,135,159,99,194,133,26,156,43,199,88,66,191,80,60,0,0,0,0,
	194,134,200,21,255,121,144,92,174,62,174,246,172,10,138,80,174,62,174,246,172,10,138,80,164,172,51,147,96,47,254,195,0,0,0,0,
	219,195,181,118,231,179,47,160,32,160,111,242,126,227,90,109,32,160,111,242,126,227,90,109,158,77,211,153,145,197,195,208,0,0,0,0,
	186,49,66,215,5,193,45,255,201,39,64,244,115,181,168,45,201,39,64,244,115,181,168,45,52,209,41,23,1,47,185,13,0,0,0,0,
	157,48,44,111,71,61,164,33,178,217,75,177,139,84,226,45,178,217,75,177,139,84,226,45,211,34,194,80,81,61,42,209,0,0,0,0,
	242,176,139,36,200,204,151,49,146,127,125,193,65,35,115,95,146,127,125,193,65,35,115,95,32,229,81,219,141,53,25,152,0,0,0,0,
	255,51,40,25,148,182,84,136,152,49,22,128,22,101,49,215,152,49,22,128,22,101,49,215,52,11,157,30,151,98,50,241,0,0,0,0,
	37,178,111,215,87,225,80,182,150,50,240,163,42,251,223,208,150,50,240,163,42,251,223,208,102,14,66,147,220,181,203,153,0,0,0,0,
	240,118,239,169,221,22,224,180,189,83,192,32,107,120,21,164,189,83,192,32,107,120,21,164,125,215,250,67,0,96,94,203,0,0,0,0,
	254,225,227,217,168,115,23,62,155,188,130,108,79,220,191,195,155,188,130,108,79,220,191,195,173,253,129,62,231,93,51,102,0,0,0,0,
	146,136,204,114,95,145,148,77,70,0,172,85,231,226,226,137,70,0,172,85,231,226,226,137,150,165,224,254,47,51,154,123,0,0,0,0,
	230,60,222,99,217,155,118,246,186,143,47,115,242,118,32,113,186,143,47,115,242,118,32,113,41,247,60,169,153,118,89,159,0,0,0,0,
	90,122,110,67,71,199,125,89,202,189,142,72,109,146,216,179,202,189,142,72,109,146,216,179,54,211,91,101,149,75,135,84,0,0,0,0,
	92,52,101,94,180,138,202,248,117,92,99,122,180,204,40,154,117,92,99,122,180,204,40,154,145,74,19,246,135,165,200,134,0,0,0,0,
	184,159,25,227,236,134,31,160,64,83,69,150,48,162,12,127,64,83,69,150,48,162,12,127,185,209,112,122,137,130,205,98,0,0,0,0,
	};

	short W=32;
	short H=16;
	short OW=8;
	short OH=4;

	unsigned short out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];

	unsigned short expected_out[]={
	180,179,144,147,137,149,120,111,0,0,0,0,
	152,132,136,132,144,127,95,94,0,0,0,0,
	154,131,133,131,151,127,116,136,0,0,0,0,
	187,167,133,129,126,126,99,127,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in*2;
	param[2] = H;
	param[3] = Wstep_out*2;
	param[4] = (int)scr;

	reduce_gauss5_16bit_4x_To_8bit(input,out,param);

//	printf("Output-> \n");
//	for(int i=0;i<OH;i++){
//		for(int j=0;j<Wstep_out;j++){
//			printf("%05d ",out[i*Wstep_out +j]);
//		}
//		printf("\n");
//	}
	tut::ensure(ensure_results_short_allowrounding("testGaussReduce16bit_4x_advance1",(short*)out,(short*)expected_out,(short)(Wstep_out*OH)));
}

void testGaussReduce16bit_4x_advance(void)
{
	unsigned short input[]={
	54,125,251,5,205,108,170,94,36,209,154,164,178,123,89,45,36,209,154,164,178,123,89,45,172,64,75,253,136,96,64,130,0,0,0,0,
	221,28,52,57,153,13,24,245,242,226,135,159,99,194,133,26,242,226,135,159,99,194,133,26,156,43,199,88,66,191,80,60,0,0,0,0,
	194,134,200,21,255,121,144,92,174,62,174,246,172,10,138,80,174,62,174,246,172,10,138,80,164,172,51,147,96,47,254,195,0,0,0,0,
	219,195,181,118,231,179,47,160,32,160,111,242,126,227,90,109,32,160,111,242,126,227,90,109,158,77,211,153,145,197,195,208,0,0,0,0,
	186,49,66,215,5,193,45,255,201,39,64,244,115,181,168,45,201,39,64,244,115,181,168,45,52,209,41,23,1,47,185,13,0,0,0,0,
	157,48,44,111,71,61,164,33,178,217,75,177,139,84,226,45,178,217,75,177,139,84,226,45,211,34,194,80,81,61,42,209,0,0,0,0,
	242,176,139,36,200,204,151,49,146,127,125,193,65,35,115,95,146,127,125,193,65,35,115,95,32,229,81,219,141,53,25,152,0,0,0,0,
	255,51,40,25,148,182,84,136,152,49,22,128,22,101,49,215,152,49,22,128,22,101,49,215,52,11,157,30,151,98,50,241,0,0,0,0,
	37,178,111,215,87,225,80,182,150,50,240,163,42,251,223,208,150,50,240,163,42,251,223,208,102,14,66,147,220,181,203,153,0,0,0,0,
	240,118,239,169,221,22,224,180,189,83,192,32,107,120,21,164,189,83,192,32,107,120,21,164,125,215,250,67,0,96,94,203,0,0,0,0,
	254,225,227,217,168,115,23,62,155,188,130,108,79,220,191,195,155,188,130,108,79,220,191,195,173,253,129,62,231,93,51,102,0,0,0,0,
	146,136,204,114,95,145,148,77,70,0,172,85,231,226,226,137,70,0,172,85,231,226,226,137,150,165,224,254,47,51,154,123,0,0,0,0,
	230,60,222,99,217,155,118,246,186,143,47,115,242,118,32,113,186,143,47,115,242,118,32,113,41,247,60,169,153,118,89,159,0,0,0,0,
	90,122,110,67,71,199,125,89,202,189,142,72,109,146,216,179,202,189,142,72,109,146,216,179,54,211,91,101,149,75,135,84,0,0,0,0,
	92,52,101,94,180,138,202,248,117,92,99,122,180,204,40,154,117,92,99,122,180,204,40,154,145,74,19,246,135,165,200,134,0,0,0,0,
	184,159,25,227,236,134,31,160,64,83,69,150,48,162,12,127,64,83,69,150,48,162,12,127,185,209,112,122,137,130,205,98,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=8;
	short OH=4;
	unsigned short out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];


	unsigned short expected_out[]={
	180,179,144,147,137,149,120,111,0,0,0,0,
	152,132,136,132,144,127,95,94,0,0,0,0,
	154,131,133,131,151,127,116,136,0,0,0,0,
	187,167,133,129,126,126,99,127,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in*2;
	param[2] = H;
	param[3] = Wstep_out*2;
	param[4] = (int)scr;

	reduce_gauss5_16bit_4x_To_8bit(input,out,param);

//	printf("Output-> \n");

//	for(int i=0;i<OH;i++){
//		for(int j=0;j<Wstep_out;j++){
//			printf("%05d ",out[i*Wstep_out +j]);
//		}
//		printf("\n");
//	}
	tut::ensure(ensure_results_short_allowrounding("testGaussReduce16bit_4x_advance",(short*)out,(short*)expected_out,(short)(Wstep_out*OH)));

}

void testGaussReduce16bit_4x_advance1(void)
{
	unsigned short input[]={
	4095,4095,1752,623,4095,4095,3525,4095,4095,131,4095,4095,4095,4095,4095,1885,3701,4095,678,4095,1940,3365,4095,1519,2042,8,4095,894,4095,3636,4095,946,0,0,0,0,
	4095,1258,4095,3476,2366,2444,4095,4095,4095,4095,4095,4095,4095,4095,4095,1031,3793,516,4095,2937,4095,4095,2687,1501,1010,2512,4095,2896,4095,4095,3574,1535,0,0,0,0,
	4095,709,4011,3538,1748,4095,3406,3279,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,1862,682,4095,2809,4095,2046,3840,3431,2563,4095,4095,0,0,0,0,
	4035,1051,4095,4095,4095,3397,2425,4095,4095,4095,4095,4095,2507,2538,4095,4095,4040,4095,1568,3407,812,4095,4095,4095,4095,4095,1326,4095,2153,308,4095,4095,0,0,0,0,
	4095,4095,3613,2059,3300,4095,4095,4095,4095,4095,3418,1856,4095,186,4095,4095,4095,4095,4095,4095,3149,2952,1154,1396,4095,3693,4095,4095,4095,4095,4095,1141,0,0,0,0,
	3693,1404,4095,1267,3753,4095,4095,4095,4095,4095,4095,4095,488,4095,4095,4095,1415,4095,3243,2742,4022,4095,3051,4095,4095,3237,4095,4095,4095,4095,3999,4095,0,0,0,0,
	4095,4095,196,3653,1563,569,4095,4095,4095,2105,4095,4095,4095,4095,4095,4095,4095,4095,3221,4095,4095,955,3532,4095,2097,561,2325,1095,1977,4095,4095,4095,0,0,0,0,
	4095,1466,3485,2962,4095,4095,4095,4095,4095,4095,929,4095,4095,3395,4095,4095,2055,4095,1007,4095,4095,789,1684,2811,4095,4095,4095,4095,4095,4095,1912,4095,0,0,0,0,
	3177,1863,4095,4095,3718,4095,4095,4095,4095,1237,4095,2355,4095,686,2868,4095,4095,4095,4095,4095,4095,1705,4095,400,4095,377,137,4095,2433,4095,4095,4095,0,0,0,0,
	4095,3024,4095,4095,4095,4095,1400,4095,4095,4095,4095,3426,4095,3282,4095,1332,4095,4095,635,4095,3845,4095,1893,4095,4095,2580,4095,3752,4095,3592,3480,4095,0,0,0,0,
	4095,1736,3875,2008,4095,4095,770,4095,3261,800,4095,4095,4095,4095,4095,2452,55,1609,2830,192,4095,4095,4095,1005,4095,4095,4095,4095,4039,2151,4095,4095,0,0,0,0,
	2991,4095,412,1590,3983,745,4095,4095,4095,4095,3975,4095,1435,4095,4095,1790,1754,419,4095,381,4095,2233,3974,4095,4095,3629,2837,157,3684,4095,2987,3877,0,0,0,0,
	1773,4095,4095,1003,4095,4095,203,4095,4095,2365,4095,4095,4095,251,4095,4095,1256,3163,4095,4095,4095,524,405,4095,421,4095,2120,2212,4095,2539,4095,4095,0,0,0,0,
	1171,1848,4095,4095,4095,4095,4095,720,4095,4095,3781,2765,3600,4095,4095,2512,4095,4095,4095,4095,4095,4095,1647,4095,3704,1898,4095,2244,3155,1082,4095,1409,0,0,0,0,
	4095,1166,2999,4095,1828,4095,4095,4095,4095,2762,4095,4095,4095,489,4095,1814,4095,4095,4095,4095,1889,4095,1437,4095,4095,4095,4095,4095,2511,3813,3567,4095,0,0,0,0,
	4095,4095,3851,4095,4095,4095,4095,1100,4095,4095,4095,4095,4095,492,3393,1358,160,4095,3257,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=8;
	short OH=4;
	unsigned short out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];

	unsigned short expected_out[]={
		3445,3233,3855,4065,3661,2811,2712,3605,0,0,0,0,
		3219,3131,3883,4011,3928,3218,2834,3385,0,0,0,0,
		3279,3209,3686,3882,3397,3681,3094,3026,0,0,0,0,
		3454,3078,3346,3922,3118,2998,2578,3459,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in*2;
	param[2] = H;
	param[3] = Wstep_out*2;
	param[4] = (int)scr;

	reduce_gauss5_16bit_4x_To_8bit(input,out,param);

//	printf("Output-> \n");
//	for(int i=0;i<OH;i++){
//		for(int j=0;j<Wstep_out;j++){
//			printf("%05d ",out[i*Wstep_out +j]);
//		}
//		printf("\n");
//	}
	tut::ensure(ensure_results_short_allowrounding("testGaussReduce16bit_4x_advance1",(short*)out,(short*)expected_out,(short)(Wstep_out*OH)));

}

void testGaussReduce16bit_4x_advance2(void)
{
	unsigned short input[]={
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
	};
	short W=32;
	short H=16;
	short OW=8;
	short OH=4;
	unsigned short out[48]={0};
	short Wstep_in=36;
	short Wstep_out=12;
	int scr[W*H];

	unsigned short expected_out[]={
		4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
		4095,4095,4095,4095,4095,4095,4095,4095,0,0,0,0,
	};

	int param[10]={0};

	param[0] = W;
	param[1] = Wstep_in*2;
	param[2] = H;
	param[3] = Wstep_out*2;
	param[4] = (int)scr;

	reduce_gauss5_16bit_4x_To_8bit(input,out,param);

//	printf("Output-> \n");
//	for(int i=0;i<OH;i++){
//		for(int j=0;j<Wstep_out;j++){
//			printf("%05d ",out[i*Wstep_out +j]);
//		}
//		printf("\n");
//	}
	tut::ensure(ensure_results_short_allowrounding("testGaussReduce16bit_4x_advance2",(short*)out,(short*)expected_out,(short)(Wstep_out*OH)));

}

#endif