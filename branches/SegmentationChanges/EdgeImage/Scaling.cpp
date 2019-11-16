#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#include <tut/tut.hpp>
//#include <tut/tut_reporter.hpp>

#ifdef __BFIN__
#include <cv.h>
#endif

#include "image.h"

extern "C" {
#include "test_fw.h"
#include "EdgeImage_private.h"
}


/*Requirements to use the below function */

//Zoom2x
//input ptr should be align 2
//ouput ptr should be align 4
//width should be align 2 and min 4

//Erode
//Input ptr should be align 4
//Output ptr should be align 2
//Tempbuff should be align 4
//Width should be align 2 and min 6




void test_scaling_basic(void)
{
	unsigned char outasm[512] = {0};
	
	unsigned char inp[]={
	9,3,5,15,2,4,12,3,
	4,1,6,8,2,12,5,9,
	3,5,7,9,2,4,8,10,
	4,6,5,10,1,6,12,11,
	5,7,9,11,6,9,10,12,
	6,8,10,12,8,9,11,13,
	7,9,11,13,9,10,12,14,
	8,1,3,14,11,11,13,15,
	9,11,13,15,10,12,14,16,
	};

	unsigned char out[]={
	9,9,3,3,5,5,15,15,2,2,4,4,12,12,3,3,
	9,9,3,3,5,5,15,15,2,2,4,4,12,12,3,3,
	4,4,1,1,6,6,8,8,2,2,12,12,5,5,9,9,
	4,4,1,1,6,6,8,8,2,2,12,12,5,5,9,9,
	3,3,5,5,7,7,9,9,2,2,4,4,8,8,10,10,
	3,3,5,5,7,7,9,9,2,2,4,4,8,8,10,10,
	4,4,6,6,5,5,10,10,1,1,6,6,12,12,11,11,
	4,4,6,6,5,5,10,10,1,1,6,6,12,12,11,11,
	5,5,7,7,9,9,11,11,6,6,9,9,10,10,12,12,
	5,5,7,7,9,9,11,11,6,6,9,9,10,10,12,12,
	6,6,8,8,10,10,12,12,8,8,9,9,11,11,13,13,
	6,6,8,8,10,10,12,12,8,8,9,9,11,11,13,13,
	7,7,9,9,11,11,13,13,9,9,10,10,12,12,14,14,
	7,7,9,9,11,11,13,13,9,9,10,10,12,12,14,14,
	8,8,1,1,3,3,14,14,11,11,11,11,13,13,15,15,
	8,8,1,1,3,3,14,14,11,11,11,11,13,13,15,15,
	9,9,11,11,13,13,15,15,10,10,12,12,14,14,16,16,
	9,9,11,11,13,13,15,15,10,10,12,12,14,14,16,16,
	};
	
	int height = 9;
	int width = 8;
	int inpstep = 8;
	int outstep =16;
	int param[10];
	
	param[0] = height;
	param[1] = width;
	param[2] = inpstep;
	param[3] = outstep;
	zoom_2x(inp,outasm,param);

	//tut::ensure(ensure_results("Zoom 2x Basic :", out, outasm, height*2*outstep));
	
}



void test_scaling_advance(void)
{
	unsigned char outasm[512] = {0};
	
	unsigned char inp[]={
	9,3,12,3,
	4,1,5,9,
	3,5,8,10,
	4,6,12,11,
	5,7,10,12,
	7,9,12,14,
	8,1,13,15,
	9,11,14,16,
	};

	unsigned char out[]={
	9,9,3,3,12,12,3,3,
	9,9,3,3,12,12,3,3,
	4,4,1,1,5,5,9,9,
	4,4,1,1,5,5,9,9,
	3,3,5,5,8,8,10,10,
	3,3,5,5,8,8,10,10,
	4,4,6,6,12,12,11,11,
	4,4,6,6,12,12,11,11,
	5,5,7,7,10,10,12,12,
	5,5,7,7,10,10,12,12,
	7,7,9,9,12,12,14,14,
	7,7,9,9,12,12,14,14,
	8,8,1,1,13,13,15,15,
	8,8,1,1,13,13,15,15,
	9,9,11,11,14,14,16,16,
	9,9,11,11,14,14,16,16,
	};
	
	int height = 8;
	int width = 4;
	int inpstep = 4;
	int outstep =8;
	int param[10];
	
	param[0] = height;
	param[1] = width;
	param[2] = inpstep;
	param[3] = outstep;
	zoom_2x(inp,outasm,param);

	//tut::ensure(ensure_results("Zoom 2x Advance :", out, outasm, height*2*outstep));
	
}

void test_scaling_advance1(void)
{
	unsigned char outasm[512] = {0};	
	unsigned char inp[]={
	9,3,2,4,12,3,255,255,
	4,1,2,12,5,9,255,255,
	3,5,2,4,8,10,255,255,
	4,6,1,6,12,11,255,255,
	5,7,6,9,10,12,255,255,
	6,8,8,9,11,13,255,255,
	7,9,9,10,12,14,255,255,
	8,1,11,11,13,15,255,255,
	9,11,10,12,14,16,255,255,
	};

	unsigned char out[]={
	9,9,3,3,2,2,4,4,12,12,3,3,
	9,9,3,3,2,2,4,4,12,12,3,3,
	4,4,1,1,2,2,12,12,5,5,9,9,
	4,4,1,1,2,2,12,12,5,5,9,9,
	3,3,5,5,2,2,4,4,8,8,10,10,
	3,3,5,5,2,2,4,4,8,8,10,10,
	4,4,6,6,1,1,6,6,12,12,11,11,
	4,4,6,6,1,1,6,6,12,12,11,11,
	5,5,7,7,6,6,9,9,10,10,12,12,
	5,5,7,7,6,6,9,9,10,10,12,12,
	6,6,8,8,8,8,9,9,11,11,13,13,
	6,6,8,8,8,8,9,9,11,11,13,13,
	7,7,9,9,9,9,10,10,12,12,14,14,
	7,7,9,9,9,9,10,10,12,12,14,14,
	8,8,1,1,11,11,11,11,13,13,15,15,
	8,8,1,1,11,11,11,11,13,13,15,15,
	9,9,11,11,10,10,12,12,14,14,16,16,
	9,9,11,11,10,10,12,12,14,14,16,16,
	};



	
	int height = 9;
	int width = 6;
	int inpstep = 8;
	int outstep =12;
	int param[10];
	
	param[0] = height;
	param[1] = width;
	param[2] = inpstep;
	param[3] = outstep;
	zoom_2x(inp,outasm,param);

//	tut::ensure(ensure_results("Zoom 2x Advance1 :", out, outasm, height*2*outstep));
	
}



void test_erode_basic(void)
{
	unsigned char outasm[512] = {0};
	unsigned char TempBuff[512] = {0};
	unsigned char inp[]={
	9,2,3,4,5,12,15,3,
	4,2,1,12,6,5,8,9,
	3,2,5,4,7,8,9,10,
	4,1,6,6,5,12,10,11,
	5,6,7,9,9,10,11,12,
	6,8,8,9,10,11,12,13,
	7,9,9,10,11,12,13,14,
	8,11,1,11,3,13,14,15,
	9,10,11,12,13,14,15,16,
	};


	unsigned char out[]={
	2,1,1,1,4,5,3,3,
	2,1,1,1,4,5,3,3,
	1,1,1,1,4,5,5,8,
	1,1,1,4,4,5,8,9,
	1,1,1,5,5,5,10,10,
	5,5,6,7,9,9,10,11,
	6,1,1,1,3,3,11,12,
	7,1,1,1,3,3,12,13,
	8,1,1,1,3,3,13,14,
	};

	int height = 9;
	int width = 8;
	int inpstep = 8;
	int outstep =8;
	int param[10];
	
	param[0] = width;
	param[1] = height;
	param[2] = inpstep;
	param[3] = outstep;
	param[4] = (int)TempBuff;
	compute_erode_image(inp,outasm,param);
//	tut::ensure(ensure_results("Erode Basic :", out, outasm, height*outstep));
}


void test_erode_advance(void)
{
	unsigned char outasm[512] = {0};
	unsigned char TempBuff[512] = {0};
	
	unsigned char inp[]={
	255,66,99,100,54,65,1,7,66,1,0,0,
	0,1,55,22,241,43,2,8,67,2,0,0,
	56,4,11,9,3,21,3,9,68,3,0,0,
	78,5,22,81,5,12,4,10,69,4,0,0,
	90,9,43,66,67,9,5,11,70,5,0,0,
	23,11,9,5,54,23,6,12,71,6,0,0,
	43,22,6,1,6,11,7,13,72,89,0,0,
	12,54,55,27,0,0,8,14,73,93,0,0,
	0,33,88,43,21,55,9,15,74,242,0,0,
	};


	unsigned char out[]={
	0,0,1,22,22,1,1,1,1,1,0,0,
	0,0,1,3,3,1,1,1,1,1,0,0,
	0,0,1,3,3,2,2,2,2,2,0,0,
	4,4,4,3,3,3,3,3,3,3,0,0,
	5,5,5,5,5,4,4,4,4,4,0,0,
	9,6,1,1,1,5,5,5,5,5,0,0,
	11,6,1,0,0,0,0,6,6,6,0,0,
	0,0,1,0,0,0,0,7,13,72,0,0,
	0,0,27,0,0,0,0,8,14,73,0,0,
	};

	int height = 9;
	int width = 10;
	int inpstep = 12;
	int outstep =12;
	int param[10];
	
	param[0] = width;
	param[1] = height;
	param[2] = inpstep;
	param[3] = outstep;
	param[4] = (int)TempBuff;
	compute_erode_image(inp,outasm,param);
//	tut::ensure(ensure_results("Erode Advance :", out, outasm, height*outstep));
}


void test_erode_advance1(void)
{
	unsigned char outasm[512] = {0};
	unsigned char TempBuff[512] = {0};

	unsigned char inp[]={
	255,66,1,7,66,1,0,0,
	0,1,2,8,67,2,0,0,
	56,4,3,9,68,3,0,0,
	78,5,4,10,69,4,0,0,
	90,9,5,11,70,5,0,0,
	43,22,7,13,72,89,0,0,
	12,54,8,14,73,93,0,0,
	0,33,9,15,74,242,0,0,
	};
	
	unsigned char out[]={
	0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,1,2,2,2,0,0,0,0,0,0,0,0,0,0,
	4,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,
	5,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,
	9,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,
	0,0,7,7,13,72,0,0,0,0,0,0,0,0,0,0,
	0,0,8,8,14,73,0,0,0,0,0,0,0,0,0,0,
	};

	int height = 8;
	int width = 6;
	int inpstep = 8;
	int outstep =16;
	int param[10];
	
	param[0] = width;
	param[1] = height;
	param[2] = inpstep;
	param[3] = outstep;
	param[4] = (int)TempBuff;
	compute_erode_image(inp,outasm,param);
//	tut::ensure(ensure_results("Erode Advance1 :", out, outasm, height*outstep));
}
