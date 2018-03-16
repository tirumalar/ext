#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits
// The differece between the three version is the precision 
// with which mean is computed.
// low_precision: mean is rounded off at 8 bits decimal and then squared
// med_precision: mean is kept with 16 bits decimal, and dual multiplication is used
//                  to recover mean^2, while keeping 16 bits of precision of decimal
// high_precision: mean is kept with 20 bits decimal and dual multiplication with bit 
//					manipulation is used to recover mean^2 while keepig 16 bits of precision of mean^2.
// However in all three version sum(x^2)/num_pixels is maintained at 16 bits precision
// Somehow we find the best version from precision perspective is medium_precision version
//

/*
positive	negative
<-------	----->

for mean
mean_shift + r4 + r4 - 16 + delta = 16
delta = 32 - mean_shift - 2r4


For variance

var_shift + r4 -16 + delta = 16
delta = 32 - var_shift - r4
*/

.text;

.align 4;

.global _compute_haar_variance;
.type _compute_haar_variance, STT_FUNC;

#define LOAD_MEAN_WEIGHT [FP-12]
#define LOAD_VARIANCE_WEIGHT [FP-16]
#define LOAD_MEAN_SHIFT [FP-20]
#define LOAD_VARIANCE_SHIFT [FP-24]
#define LOAD_15 [FP-28]

_compute_haar_variance:


LINK 28;
[ -- SP] = (R7:0,P5:0);

L0 = 0;
L1 = 0;
L2 = 0;
L3 = 0;

P0 = R0;
P1 = R1;

/*
	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = extra_output_width_step;	// extra widthstep
	dstParam[3] = weight;
	dstParam[4] = p[0];
	dstParam[5] = p[1];
	dstParam[6] = p[2];
	dstParam[7] = p[3];
	dstParam[8] = extra_input_width_step;
*/	

R0 = 0;
	
P4 = [P0++];	// width of the image
P4 += -1;		// one less due to  pipeliing

R0 = [P0++];	// height
LC0 = R0;

P3 = [P0++];	// extra output widthstep;
P3 += 4;	

R7 = [P0++];	// 2^16/(window_area * window_area)	16 bits of precision
LOAD_MEAN_WEIGHT = R7;

R4 = [P0++];	// 2^16/(window_area)	16 bits of precision
LOAD_VARIANCE_WEIGHT = R4;

R0 = [P0++];
R1 = [P0++];

R7 = 32;
R0 = R7 - R0;
LOAD_MEAN_SHIFT = R0;

R7 = 32; // 16;
R1 = R7 - R1;
LOAD_VARIANCE_SHIFT = R1;

R0 = 15;
LOAD_15 = R0;

R0 = [P0++];	I0 = R0;
R1 = [P0++];	I1 = R1;
R2 = [P0++];	I2 = R2;
R3 = [P0++];	I3 = R3;

P2 = [P0++];	// extra input width step;
M0 = 4;
M1 = P2;

P0 = FP;
P0 += -10;


LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++] || R3 = [I3++];
R2 = [I2++] || R1 = [I1++];

R5 = R3 + R0 (NS) || R7 = LOAD_15; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];
// R5 is unnormalized mean
R4.L = SIGNBITS R5 ;
R4.L = R7.L - R4.L (S) || R6 = LOAD_MEAN_SHIFT;
R5 = LSHIFT R5 BY R4.L;
R6.L = R6.L - R4.L (S);
R5 = R5.L * R5.L (FU) || R7 = LOAD_MEAN_WEIGHT;
R4.L = R6.L - R4.L (S);
R7 = R5.L * R7.L, R6 = R5.H * R7.L (FU);
R7 >>= 16;
R5 = R6 + R7 (NS);


R5 = LSHIFT R5 BY R4.L;
//R5.L = R5 (RND);	// this just the mean, 8 bits of integer, 8 bits of decimal
//R5 = R5.L * R5.L (FU);	// mean^2, 16 bits of integer - 16 bits of decimal

R6 = R3 - R1 (NS) || R7 = LOAD_15;
R6 = R6 + R0 (NS);
R6 = R6 - R2 (NS);

// R6 (sum (x^2) ) could be entire 32 bits
// R6 is unnormalized sum(x^2)
R1.L = SIGNBITS R6 || R4 = LOAD_VARIANCE_SHIFT;
R6 = LSHIFT R6 BY R1.L || R7 = LOAD_VARIANCE_WEIGHT;
R4.L = R4.L - R1.L (S) || R0=[I0++] || R3 = [I3++];
R6 = R6.L * R7.L, R7 = R6.H * R7.L (FU);
R6 >>= 16;
R6 = R6 + R7 (NS);
R6 = LSHIFT R6 BY R4.L || R1=[I1++] || R2 = [I2++];


R6 = R6 - R5 (NS) ;	// sum(x^2)/window_size - mean * mean;

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

// 0 | 1
// 2 | 3  // 4 being estimated

R5 = R3 + R0 (NS) || R7 = LOAD_15; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];
// R5 is unnormalized mean
R4.L = SIGNBITS R5 || [P1++] = R6;
R4.L = R7.L - R4.L (S) || R6 = LOAD_MEAN_SHIFT;
R5 = LSHIFT R5 BY R4.L;
R6.L = R6.L - R4.L (S);
R5 = R5.L * R5.L (FU) || R7 = LOAD_MEAN_WEIGHT;
R4.L = R6.L - R4.L (S);
R7 = R5.L * R7.L, R6 = R5.H * R7.L (FU);
R7 >>= 16;
R5 = R6 + R7 (NS);

R5 = LSHIFT R5 BY R4.L;
//R5.L = R5 (RND);	// this just the mean, 8 bits of integer, 8 bits of decimal
//R5 = R5.L * R5.L (FU);	// mean^2, 16 bits of integer - 16 bits of decimal

R6 = R3 - R1 (NS) || R7 = LOAD_15;
R6 = R6 + R0 (NS);
R6 = R6 - R2 (NS);

// R6 (sum (x^2) ) could be entire 32 bits
// R6 is unnormalized sum(x^2)
R1.L = SIGNBITS R6 || R4 = LOAD_VARIANCE_SHIFT;
R6 = LSHIFT R6 BY R1.L || R7 = LOAD_VARIANCE_WEIGHT;
R4.L = R4.L - R1.L (S) || R0=[I0++] || R3 = [I3++];
R6 = R6.L * R7.L, R7 = R6.H * R7.L (FU);
R6 >>= 16;
R6 = R6 + R7 (NS);
R6 = LSHIFT R6 BY R4.L || R1=[I1++] || R2 = [I2++];

R6 = R6 - R5 (NS) ;	// sum(x^2)/window_size - mean * mean;
LOOP_END INNER_WIDTH_LOOP;

[P1++P3] = R6;

I0 -= M0;
I1 -= M0;
I2 -= M0;
I3 -= M0;

I0 += M1;
I1 += M1;
I2 += M1;
I3 += M1;

LOOP_END OUTER_HEIGHT_LOOP;


(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_haar_variance.END:


#endif //__BFIN__
