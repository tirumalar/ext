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
.text;

.align 4;

.global _compute_haar_variance;
.type _compute_haar_variance, STT_FUNC;

#define LOAD_ZERO [FP-12]

_compute_haar_variance:


LINK 12;
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
LOAD_ZERO = R0;
	
P4 = [P0++];	// width of the image
P4 += -1;		// one less due to  pipeliing

R0 = [P0++];	// height
LC0 = R0;

P3 = [P0++];	// extra output widthstep;
P3 += 4;	

R7 = [P0++];	// 2^8/(window_area)	16 bits of precision
R4 = [P0++];	// 2^16/(window_area)	16 bits of precision

R0 = [P0++];	I0 = R0;
R1 = [P0++];	I1 = R1;
R2 = [P0++];	I2 = R2;
R3 = [P0++];	I3 = R3;

P2 = [P0++];	// extra input width step;
M0 = 4;
M1 = P2;


LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++] || R3 = [I3++];
R2 = [I2++] || R1 = [I1++];

R5 = R3 + R0 (NS) ; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];
R5 *= R7;
R5.L = R5 (RND);	// this just the mean, 8 bits of integer, 8 bits of decimal
R5 = R5.L * R5.L (FU);	// mean^2, 16 bits of integer - 16 bits of decimal

R6 = R3 - R1 (NS);
R6 = R6 + R0 (NS);
R6 = R6 - R2 (NS);

// R6 (sum (x^2) ) could be entire 32 bits

R3 = R6.H * R4.L, R2 = R6.L * R4.L (FU);	// multpling the MSB and LSB 16 bits seperately with 16 bits of 1/window_area
															// R3 cannot have more than 16 bits because original data before square was 8 bits
R2.H = R2.H + R3.L (NS)  || R0=[I0++] || R3 = [I3++];	// taking those 16 bits and adding to MSB of R2
														// finally we have variance*weight in 16 bits iteger + 16 bits decimal

R6 = R2 - R5 (NS) || R1=[I1++] || R2 = [I2++];	// sum(x^2)/window_size - mean * mean;

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

// 0 | 1
// 2 | 3  // 4 being estimated

R5 = R3 + R0 (NS) || [P1++] = R6; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];
R5 *= R7;
R5.L = R5 (RND);	// this just the mean
R5 = R5.L * R5.L (FU);	// mean^2

R6 = R3 - R1 (NS);
R6 = R6 + R0 (NS);
R6 = R6 - R2 (NS);

R3 = R6.H * R4.L, R2 = R6.L * R4.L (FU);	// multpling the MSB and LSB 16 bits seperately with 16 bits of 1/window_area
															// R3 cannot have more than 16 bits because original data before square was 8 bits
R2.H = R2.H + R3.L (NS)  || R0=[I0++] || R3 = [I3++];	// taking those 16 bits and adding to MSB of R2
														// finally we have variance*weight in 16 bits iteger + 16 bits decimal

R6 = R2 - R5 (NS) || R1=[I1++] || R2 = [I2++];	// sum(x^2)/window_size - mean * mean;

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
