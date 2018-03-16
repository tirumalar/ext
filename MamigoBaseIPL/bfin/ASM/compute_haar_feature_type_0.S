#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits

.text;

.align 4;

.global _compute_haar_feature_type_0;
.type _compute_haar_feature_type_0, STT_FUNC;

_compute_haar_feature_type_0:

#define LOAD_WEIGHT [FP-12]
#define LOAD_INPUT_WIDTH_STEP [FP-16]
#define LOAD_OUTPUT_WIDTH_STEP [FP-20]
#define LOAD_NORM_EXP [FP-24]

LINK 24;
[ -- SP] = (R7:1,P5:0);

L0 = 0;
L1 = 0;
L2 = 0;
L3 = 0;

P4 = R0;
P5 = R1;

/*
	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = extra_output_width_step;	// extra widthstep
	dstParam[3] = weight;
	dstParam[4] = p[0];
	dstParam[5] = p[1];
	dstParam[6] = p[2];
	dstParam[7] = p[3];
	dstParam[8] = p[4];
	dstParam[9] = p[5];
	dstParam[10] = p[6];
	dstParam[11] = p[7];	
	dstParam[12] = extra_input_width_step;
*/	
	
R0 = [P4++];	// width of the image
R0 += -1;		// one less due to  pipeliing
[ -- SP] = R0;

R0 = [P4++];	// height
LC0 = R0;

P3 = [P4++];	// extra output widthstep;
P3 += 4;
R0 = P3;
LOAD_OUTPUT_WIDTH_STEP = R0;	

R7 = [P4++];	// weight -
LOAD_WEIGHT = R7;

R0 = [P4++];	
I0 = R0;	
P0 = R0; // x1

R1 = [P4++];	
I1 = R1;	
P1 = R1; // x2

R2 = [P4++];	
I2 = R2;	
P2 = R2; // x3

R4 = [P4++];	// skip

R4 = [P4++];	// y1 must be 0
R4 = [P4++];	//skip
P3 = [P4++];  	// y3
R4 = [P4++];	//skip

P0 = P0 + (P3 << 2);
P1 = P1 + (P3 << 2);
P2 = P2 + (P3 << 2);

R0 = [P4++];	// extra input width step;
M0 = R0;
LOAD_INPUT_WIDTH_STEP = R0;
P4 = [SP ++];

R0 = 0 (Z);
LOAD_NORM_EXP = R0;

LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++] || R3 = [P0++];

R6 = R3 - R0 (NS) || R2 = [I2++] || R5 = [P2++];									// D - A
R7 = R5 - R2 (NS) || P3 = LOAD_INPUT_WIDTH_STEP;	// F - C
R6 = R6 + R7 (NS) || R1 = [I1++] || R4 = [P1++];	// (D - A) + (F - C)

R7 = R1 - R4 (S)  || R0 = [I0++] || R3 = [P0++];	// B - E
R7 = R7 << 1 	  || R2 = [I2++] || R5 = [P2++];	// 2*(B-E)
R7 = R6 + R7 (S)  || R1 = LOAD_WEIGHT;				// (D - A) + (F - C) + 2*(B-E)
R7 *= R1;
R4 = ABS R7 || R1 = LOAD_NORM_EXP;
R7 *= R4;
R1 = MAX(R1, R4);
 
LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

R6 = R3 - R0 (NS) || LOAD_NORM_EXP = R1;					// D - A
R7 = R5 - R2 (NS) || [P5++] = R7;	// F - C
R6 = R6 + R7 (NS) || R1 = [I1++] || R4 = [P1++];					// (D - A) + (F - C)

R7 = R1 - R4 (S) || R0 = [I0++] || R3 = [P0++];		// B - E
R7 = R7 << 1 	 || R2 = [I2++] || R5 = [P2++];		// 2*(B-E)
R7 = R6 + R7 (S) || R1 = LOAD_WEIGHT;				// (D - A) + (F - C) + 2*(B-E)
R7 *= R1;
R4 = ABS R7 || R1 = LOAD_NORM_EXP;
R7 *= R4;
R1 = MAX(R1, R4);

LOOP_END INNER_WIDTH_LOOP;

LOAD_NORM_EXP = R1;

P0 = P0 + P3;
P1 = P1 + P3;
P2 = P2 + P3;
P1 += 4;

P3 = LOAD_OUTPUT_WIDTH_STEP;
I0 += M0;
I1 += M0;
I2 += M0;
I1 += 4;

[P5 ++ P3] = R7;

LOOP_END OUTER_HEIGHT_LOOP;

R0 = LOAD_NORM_EXP;
(R7:1,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_haar_feature_type_0.END:


#endif
