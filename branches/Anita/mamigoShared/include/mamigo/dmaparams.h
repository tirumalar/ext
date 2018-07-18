/***************************************************************************
 *   Copyright (C) 2008 by Akshay Mathur,,,   				*
 *   akshay.mathur@mamigo.in   						*
 *                                                  			*
 *   Mamigo Copyright protects the code.				*
 ***************************************************************************/

#ifndef __MAMIGO_DMAPARAMS_
#define __MAMIGO_DMAPARAMS_ 1

#ifndef MIN
#define MIN(x,y) (x<y)?x:y
#endif

#ifndef MAX
#define MAX(x,y) (x>y)?x:y
#endif

#ifndef ABS
#define ABS(x) (x>0)?x:(0-(x))
#endif

typedef struct{
	int width;
	int height;
}dimensions;

typedef struct _point{
	int x;
	int y;
} point;

typedef struct _rect{
	point origin;
	dimensions dims;
} rect;

typedef struct _dma_calculator{
	int operationMask;	/* 1=Sample, 2=crop, 3=sample+crop*/
	dimensions source;
	rect	crop;		/* should never be bigger than the image*/
	int samplingRate;	/*1,2,4*/ /* Rate=1: 1111111111111111
	Rate=2: 1010101010101010
	Rate=4: 1000100010001000
				*/

	dimensions filter;	/*3,3*/
	int bufferSize;		/*5200*/
	int shouldOverlap;	/*0,1  0=loose calcs of some rows 1=exhaustive*/
	int wordSize;		/*1,2  number of bytes to be read in a single DMA op */
} dma_calculator;

typedef  struct _dma_config{
	int X_COUNT;
	int X_MODIFY;
	int Y_COUNT;
	int Y_MODIFY;
}dma_config;

typedef struct _dma_config_output{
	dma_config sourceCfg;
	dma_config destCfg;
	int incr_source;				/* amount to increment the source counter*/
	int numIterations;				/* number of iteration */
	dimensions destReadDimension;	/* the dimentions of the data copied to L1*/
	int isRemainder;				/* 1 if this output is the remainder */
}dma_config_output;

typedef struct _image_read_params{
	int xstart;
	int xstep;
	int xend;
	int ystart;
	int ystep;
	int yend;
}image_read_params;

void calc_setSampling(dma_calculator *calc, int rate);

void calc_setCrop(dma_calculator *calc, int orig_x, int orig_y, int width, int height);

int calc_isCrop(dma_calculator *calc);

int calc_isSampling(dma_calculator *calc);

void calc_setCropSampling(dma_calculator *calc, int orig_x, int orig_y, int width, int height, int rate);

//deprecated
//void init(dma_calculator *calc, int crop_width_by); // init default values into

void calc_init(dma_calculator *calc, int width, int height); // init default values into

void calculate(dma_calculator *calc, dma_config_output *result, dma_config_output *remainder);

void calc_image_read_params(dma_calculator *calc, image_read_params* r);

#endif
