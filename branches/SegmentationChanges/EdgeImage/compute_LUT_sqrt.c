#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//extern unsigned short SQRT_LUT_0_64[65*65];
//extern unsigned short SQRT_LUT_0_256[256*256];


//Compute Look up table SQRT_LUT_0_64 in 9Q7 format.
//Compute Look up table SQRT_LUT_0_256 in 9Q7 format.
void compute_LUT_sqrtminmax_C(unsigned short *SQRT_LUT_0_64, unsigned short *SQRT_LUT_0_256,int gradientpupilmin,int gradientpupilmax,int gradientcirclemin,int gradientcirclemax)
{
	short i,j;
	int value,min,max;
	min = gradientpupilmin*128;
	max = gradientpupilmax*128;
//	printf("LUT64 [%d %d]\n",gradientpupilmin,gradientpupilmax);
	for(i=0;i<65;i++)
	{
		for(j=0;j<65;j++)
		{
			value = (int)(sqrt(i*i+j*j)*128+0.5);
			if( value<=min)
			{
				value = 0;
			}
			if(value>max)
			{
				value = max;
			}
			SQRT_LUT_0_64[i*65+j] = (unsigned short)value;
		}
	}

	min = gradientcirclemin*128;
	max = (int)(sqrt(gradientcirclemax*gradientcirclemax+gradientcirclemax*gradientcirclemax)*128+0.5);
//	printf("LUT255 [%d %d]\n",gradientcirclemin,gradientcirclemax);

	for(i=0;i<256;i++)
	{
		for(j=0;j<256;j++)
		{
			value = (int)(sqrt(i*i+j*j)*128+0.5);

			if( value<=min)
			{
				value = 0;
			}
			if(value>max)
			{
				value = max;
			}
			SQRT_LUT_0_256[i*256+j] = (unsigned short)value;
		}
	}
}
//Compute Look up table SQRT_LUT_0_64 in 9Q7 format.
//Compute Look up table SQRT_LUT_0_256 in 9Q7 format.

void compute_LUT_sqrt(unsigned short *SQRT_LUT_0_64, unsigned short *SQRT_LUT_0_256)
{
	short i,j;
	int value;
#ifdef __ANDROID__
	int lThresh=2*128;
#else
	int lThresh=5*128;
#endif

	for(i=0;i<65;i++)
	{
		for(j=0;j<65;j++)
		{
			value = (int)(sqrt(i*i+j*j)*128+0.5);
			//Min = 5*128
			if( value<=640)
			{
				value = 0;
			}
			//Max = 64*128
			if(value>8192)
			{
				value = 8192;
			}
			SQRT_LUT_0_64[i*65+j] = (unsigned short)value;
		}
	}
	for(i=0;i<256;i++)
	{
		for(j=0;j<256;j++)
		{
			value = (int)(sqrt(i*i+j*j)*128+0.5);
			//Maddy

			if( value<=lThresh)
			{
				value = 0;
			}

			SQRT_LUT_0_256[i*256+j] = (unsigned short)value;
		}
	}
}

void compute_LUT_sqrt_eyelid(unsigned short *SQRT_LUT_0_256)
{
	short i,j;
	int value;

	for(i=0;i<256;i++)
	{
		for(j=0;j<256;j++)
		{
			value = (int)(sqrt(i*i+j*j)*128+0.5);
			//Maddy
			SQRT_LUT_0_256[i*256+j] = (unsigned short)value;
		}
	}
}
