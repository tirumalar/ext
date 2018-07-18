/***************************************************************************
 *   Copyright (C) 2008 by Akshay Mathur,,,   				   *
 *   akshay.mathur@mamigo.in   						   *
 *                                                                         *
 *   Mamigo Copyright protects the code.				   *
 ***************************************************************************/


#ifndef _MAMIGO_CALC_
#define _MAMIGO_CALC_ 1

#ifndef MAMIGO_VERIFY
#include "dmaparams.h"
#else
#include "../Verifier/dmaparams.h"
#endif

#define RS_FULL   1
#define RS_SAMPLE 2
#define RS_CROP   3
#define RS_CRSP   4
#define RS_SLOPPY 5

#define ALG_NOOP  0
#define ALG_GRDT  1
#define ALG_LPLC  2

struct spec_results {
	short x, y, laplacian;
};

typedef struct _specularityResult{
	//constants
	int height;
	int value_limit;
	int laplacian_limit;
	struct spec_results *items;
	void *_reserved;

	int max_points;
	int num_points;
	int width;
} specularityResult;


enum sourceType {eMT9P031=0, eMT9V032, eADV7183_PAL, eADV7183_NTSC,eMT9P001};

struct ioctl_pass_args{
	enum sourceType sensorType;
	unsigned short algorithm_number;
	unsigned short ReadStyle;
	rect r;
	unsigned int samplingRate;
	unsigned short num_images;
	int ImageSize;
	specularityResult *sR;
};

#ifndef MAMIGO_VERIFY
//Their fast inner loops in asm
extern int sum_of_pixel_test1(unsigned char *, int );
extern int gradient_way1(unsigned char *, int , int );
extern unsigned long specularity_detection(unsigned char *src, int width, specularityResult *s  );
extern int noOp(unsigned char *, int , int );
#else
#include "../Verifier/mamigomisc.h"
#include "../Verifier/dummy_C_replc_asmAlgos.h"
#endif

//Getters and Setters
void set_crop_rectangle(rect r1);
void set_samplingRate(unsigned int i);
void set_specularityResultStartMemory(specularityResult *s_R, int width);
long get_Timing(void);

// The Algorithms
int dummy_algo( unsigned char *src, dma_calculator calc);
int specularity_wrapper (unsigned char *src, int width, int height);


//Loops available
int two_halves_loop ( unsigned char *src, dma_calculator calc);
int simple_loop ( unsigned char *src, dma_calculator calc);

int do_calculations( char *src, dimensions dims, int algo, int ReadStyle);

#endif

