#ifdef __BFIN__
/******************************************************************************

* _scale_warp_c
*******************************************************************************/

#include "mamigo_asm_helper.h"

#ifdef WARP_IMAGE_CHAR
#define FNAME _scale_warp_c
#define READ_MODIFIER X
#define MULT_MODIFIER_XFRAC IS
#define MULT_MODIFIER_YFRAC IS

#else
#define FNAME _scale_warp_b
#define READ_MODIFIER Z
#define MULT_MODIFIER_XFRAC IU
#define MULT_MODIFIER_YFRAC FU
#endif

#define LOAD_INPUT_WIDTHSTEP		[FP-12]
#define LOAD_OUTPUT_EXTRA_WIDTHSTEP	[FP-16]
#define LOAD_X_TABLE_START			[FP-20]
#define LOAD_Z1						[FP-24]
#define LOAD_Z2						[FP-28]
#define LOAD_X_COUNT				[FP-32]
#define LOAD_END_ZERO_COUNT			[FP-36]
#define LOAD_INITIAL_ZERO_COUNT		[FP-40]
#define LOAD_DECIMAL_EXTRACTION		[FP-44]
#define LOAD_ONE					[FP-48]
#define LOAD_LAST_YS				[FP-52]

ASM_HEADER

FUNC_DEF(FNAME)

FUNC_START(FNAME)

LINK 56;
[ -- SP] = (R7:0,P5:0);


P4 = R0;		// input, for fptr1, assuming this is at ystartSrc
P2 = R2;		// output pointer

P0 = R1;		// parameters
L0 = 0;
L1 = 0;
L2 = 0;
L3 = 0;
R0 = [P0++];	// widthDest
R1 = [P0++];	// heightDest
R2 = [P0++];	// actual input width step;
LOAD_INPUT_WIDTHSTEP = R2;
R2 = [P0++];	// incremental output width step;
LOAD_OUTPUT_EXTRA_WIDTHSTEP = R2;

P5 = [P0++];	// scratch
LOAD_X_TABLE_START = P5;

R2 = [P0++];	// z1		// assumed 32 bit unsigned, 10 bits integer part, 22 bits of decimal
LOAD_Z1 = R2;
R2 = [P0++];	// z2;
LOAD_Z2 = R2;

R2 = [P0++];	// xstartSrc;	// assumed 32 bit unsigned, 10 bits of whole, 22 bits of decimal
R3 = [P0++];	// ystartSrc;	// assumed 32 bit unsigned, 10 bits of whole, 22 bits of decimal
LOAD_LAST_YS = R3;


R6 = R2 >> 22;
P1 = R6;		// putting the integer part of xstartSrc into P1


R4 = [P0++];	// xstartDestOff;	// number of iitial zeroes
R6 = [P0++];	// xcountDest;		// integer
LOAD_X_COUNT = R6;
R0 = R0 - R6;
R0 = R0 - R4;	// this is number of pixels that need to be zeroed out at the end
R0 += 1;		// delibrately added a one
LOAD_END_ZERO_COUNT = R0;

P4 = P4 + P1;	// offsetting input ptr by xstartSrc;

R6 += 1;
R6 >>= 1;
LC1 = R6;

R4 += 1;	// delibrately adding a one

LOAD_INITIAL_ZERO_COUNT = R4;

R6 = [P0++];	// ystartDestOff - read but disregarded, assumed to be zero
R6 = [P0++];	// ycountDest
LC0 = R6;

// now lets construct the offset xfrac _xfrac table
R5 = R2;	// R5 is new value	
			// R2 is old value 
			
R0 = LOAD_Z1;			// R0 is z1
R1.L = 0xFFFF;
R1.H = 0x003F;
LOAD_DECIMAL_EXTRACTION = R1;
R3.L = 0x100;
R3.H = 0x100;
LOAD_ONE = R3;

LOOP CONSTRUCT_X_TABLE LC1;
LOOP_BEGIN CONSTRUCT_X_TABLE;
R6 = R2 >> 22;
R7 = R5 >> 22;
R6 = R7 - R6;	// this is the offset
				// need to compute xfrac and _xfrac
R7 = R5 & R1;	// getting the last 22 bits
R6 = R7 << 2 || [P5++] = R6 ;
				// shifting left by 2 bits so that we can round off and get our desired 8 bits of xfrac
				// saving the offset
R6.L = R6 (RND);	// this gives xfrac
R6.H = R3.L - R6.L (S);	// now R6 = [_xfrac xfrac]
  				
R2 = R5 + R0 (NS);	// now R2 is new and R5 is old

R6 = R2 >> 22 || [P5++] = R6;
R7 = R5 >> 22;
R6 = R6 - R7;	// this is the offset

R7 = R2 & R1;	// getting the last 22 bits
R6 = R7 << 2 || [P5++] = R6 ;
				// shifting left by 2 bits so that we can round off and get our desired 8 bits of xfrac
				// saving the offset
R6.L = R6 (RND);	// this gives xfrac
R6.H = R3.L - R6.L (S);	// now R6 = [_xfrac xfrac]

R5 = R2 + R0 (NS) || [P5++] = R6;	// now R5 is new and R5 is old
LOOP_END CONSTRUCT_X_TABLE;

// need one extra entry in the table
R5 = 0 (Z);
[P5++] = R5;
[P5++] = R6;

[P5++] = R5;

LOOP OUTER_HEIGHT_WARP_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_WARP_LOOP;

/****************************************************************/
/* first time */
/****************************************************************/
// First compute yfrac, _yfrac, and begin of the line ptr

P5 = LOAD_X_TABLE_START;
R0 = LOAD_LAST_YS;
R4 = LOAD_DECIMAL_EXTRACTION;
R1 = R0 >> 22 || R3 = LOAD_ONE;	// this the new Y
R7 = R0 & R4;
R2 = R7 << 2;
R5.L = R2 (RND);
R5.H = R3.L - R5.L (S) || R2 = LOAD_Z2;	// R5 = [_yfrac yfrac]

R0 = R0 + R2 (NS);
LOAD_LAST_YS = R0;

R4 = LOAD_INITIAL_ZERO_COUNT;
LC1 = R4;

R3 = LOAD_INPUT_WIDTHSTEP;
P1 = R3;		// widthstep
// R0 has 22 bits of decimal and 10 bits of regular
R3 *= R1;	// R3 is the y offset
P0 = R3;
R7 = 0;


LOOP INNER_INITIAL_ZERO_LOOP LC1;
LOOP_BEGIN INNER_INITIAL_ZERO_LOOP;
B[P2++] = R7;
LOOP_END INNER_INITIAL_ZERO_LOOP;

P0 = P4 + P0;	// first row
P1 = P0 + P1;	// second row

R0 = LOAD_X_COUNT;
LC1 = R0;

B[P2--] = R7;

P3 = [P5++];	// this is zero anyway - so doesn't matter
R2 = [P5++];	// load first [_xfrac xfrac]

P3 = [P5++];	// getting the next 
R6 = B [P0++] (READ_MODIFIER);
R7 = B [P1++] (READ_MODIFIER);
R0 = PACK(R7.L, R6.L) || R6 = B [P0--] (READ_MODIFIER);
P0 = P0 + P3;
R7 = B [P1--] (READ_MODIFIER);
P1 = P1 + P3;
R1 = PACK(R7.L, R6.L);


LOOP INNER_WIDTH_WARP_LOOP LC1;
LOOP_BEGIN INNER_WIDTH_WARP_LOOP;


// for past iterations
// R0 = [fptr2[0] fptr1[0]]
// R1 = [fptr2[1] fptr1[1]]
// R2 = [_xfrac xfrac]
// R5 = [_yfrac yfrac]

R0.H = R2.H * R0.H, R0.L = R2.H * R0.L (MULT_MODIFIER_XFRAC) || R6 = B[P0++] (READ_MODIFIER);	
														// R0 = [xfrac_* fptr2[0]   xfrac_*fptr1[0]]
																
R1.H = R2.L * R1.H, R1.L = R2.L * R1.L (MULT_MODIFIER_XFRAC) || R2 = [P5++]; 			
														// R1 = [xfrac* fptr2[1]   xfrac*fptr1[1]]
R0 = R0 +|+ R1 || P3 = [P5++];							// R0 = [xfrac_* fptr2[0] + xfrac * fptr2[1]
														//       xfrac_* fptr1[0] + xfrac * fptr1[1]]
														// R0 = [y2val   y1val]
														// R5 = [yfrac_  yfrac]
														
R0 = R5.H * R0.L, R1 = R5.L * R0.H (MULT_MODIFIER_YFRAC) || R7 = B [P1++] (READ_MODIFIER);		
														// R0 = [y2val*yfrac  y1val*yfrac_]

R4 = R0 + R1 (S) || R3 = B [P0--] (READ_MODIFIER);		// R0.L = val = y2val*yfrac + y1val*yfrac_

R0 = PACK(R7.L, R6.L) || R1 = B [P1--] (READ_MODIFIER);

R4.L = R4 (RND);

P0 = P0 + P3;
P1 = P1 + P3;

R1 = PACK(R1.L, R3.L) || B[P2++] = R4;

LOOP_END INNER_WIDTH_WARP_LOOP;

// NEED TO SET ZEROS to the output - do that here if necessary


P1 = LOAD_END_ZERO_COUNT;
P0 = LOAD_OUTPUT_EXTRA_WIDTHSTEP; // extra widthstep for output
R7 = 0;

LOOP INNER_END_ZERO_LOOP LC1=P1;
LOOP_BEGIN INNER_END_ZERO_LOOP;
B[P2++] = R7;
LOOP_END INNER_END_ZERO_LOOP;
B[P2--] = R7;

P2 = P2 + P0;	// taking care of extra widthstep for output


LOOP_END OUTER_HEIGHT_WARP_LOOP;


(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

FUNC_END(FNAME)

#endif //__BFIN__
