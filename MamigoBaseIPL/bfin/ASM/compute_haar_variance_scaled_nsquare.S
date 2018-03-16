#ifdef __BFIN__

.text;

.align 4;

.global _compute_haar_variance_scaled_nsquare;
.type _compute_haar_variance_scaled_nsquare, STT_FUNC;

#define LOAD_15 [FP-12]
#define LOAD_MEAN_SHIFT [FP-16]
#define LOAD_VARIANCE_SCALE [FP-20]

_compute_haar_variance_scaled_nsquare:


LINK 20;
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
	dstParam[3] = ;
	dstParam[4] = p[0];
	dstParam[5] = p[1];
	dstParam[6] = p[2];
	dstParam[7] = p[3];
	dstParam[8] = extra_input_width_step;
*/	

R0 = 15;
LOAD_15 = R0;
	
P4 = [P0++];	// width of the image
P4 += -1;		// one less due to  pipeliing

R0 = [P0++];	// height
LC0 = R0;

P3 = [P0++];	// extra output widthstep;
P3 += 4;	

R7 = [P0++];
R7 = -R7;
LOAD_MEAN_SHIFT = R7;

R4 = [P0++];	// 2^16/(window_area)	16 bits of precision
LOAD_VARIANCE_SCALE = R4;

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

R5 = R3 + R0 (NS) || R7 = LOAD_15; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];

R4.L = SIGNBITS R5 ;
R4.L = R4.L - R7.L (S) || R6 = LOAD_MEAN_SHIFT;
R5 = LSHIFT R5 BY R4.L;
R6.L = R6.L - R4.L (S);
R5 = R5.L * R5.L (FU);
R4.L = R6.L - R4.L (S);
R5 = LSHIFT R5 BY R4.L;


R6 = R3 - R1 (NS);
R6 = R6 + R0 (NS) || R7 = LOAD_VARIANCE_SCALE;
R2 = R6 - R2 (NS) || R0=[I0++] || R3 = [I3++];
R2 *= R7;

R6 = R2 - R5 (NS) || R1=[I1++] || R2 = [I2++];	// scale*(sum(x^2) - mean * mean);

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

// 0 | 1
// 2 | 3  // 4 being estimated

R5 = R3 + R0 (NS) || [P1++] = R6; 
R5 = R5 - R1 (NS) || R0=[I0++] || R3 = [I3++];
R5 = R5 - R2 (NS) || R1=[I1++] || R2 = [I2++];

R4.L = SIGNBITS R5 || R7 = LOAD_15;
R4.L = R4.L - R7.L (S) || R6 = LOAD_MEAN_SHIFT;
R5 = LSHIFT R5 BY R4.L;
R6.L = R6.L - R4.L (S);
R5 = R5.L * R5.L (FU);
R4.L = R6.L - R4.L (S);
R5 = LSHIFT R5 BY R4.L;

R6 = R3 - R1 (NS);
R6 = R6 + R0 (NS) || R7 = LOAD_VARIANCE_SCALE;
R2 = R6 - R2 (NS) || R0=[I0++] || R3 = [I3++];

R2 *= R7;

R6 = R2 - R5 (NS) || R1=[I1++] || R2 = [I2++];	// scale*(sum(x^2) - mean * mean);

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

_compute_haar_variance_scaled_nsquare.END:


#endif //__BFIN__
