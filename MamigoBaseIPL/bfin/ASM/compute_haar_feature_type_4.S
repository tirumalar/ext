#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits

.text;

.align 4;

.global _compute_haar_feature_type_4;
.type _compute_haar_feature_type_4, STT_FUNC;

_compute_haar_feature_type_4:

#define LOAD_WEIGHT [FP-12]
#define LOAD_INPUT_WIDTH_STEP [FP-16]
#define LOAD_OUTPUT_WIDTH_STEP [FP-20]
#define LOAD_INNER_LOOP_COUNT [FP-24]
#define LOAD_E_POINTER [FP-28]
#define LOAD_NORM_EXP [FP-32]

LINK 32;
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
LOAD_INNER_LOOP_COUNT = R0;

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
I1 = R0;
I2 = R0;	// x1	


R1 = [P4++];	// x2 skip
P0 = R1;
P1 = R1;


R2 = [P4++];	
I3 = R2;
P2 = R2;
P3 = R2; // x3


R3 = [P4++];	// x4 skip


[-- SP] = P5;

R4 = [P4++];	// y1 must be 0
R4 = [P4++];	// y2
P5 = R4;
R4 <<= 2;
M1 = R4;

P5 = P0 + (P5 << 2);
LOAD_E_POINTER = P5;

R0 = [P4++];  	// y3

R1 = [P4++];	// y4, skip

R0 <<= 2;
M2 = R0;
P5 = R0;

I1 += M1;
I3 += M1;

P3 = P3 + P5;
P1 = P1 + P5;

I2 += M2;

P5 = [SP ++];

R0 = [P4++];	// extra input width step;
M0 = R0;
LOAD_INPUT_WIDTH_STEP = R0;

R0 = 0 (Z);
LOAD_NORM_EXP = R0;
/*

		I0 ------- I1 -------- I2
		|          |          |
		P0 ------- E -------- P1
		|          |          |
		P2 ------- I3 --------P3

		A ------- B -------- C
		|         |          |
		D ------- E -------- F
		|         |          |
		G ------- H -------- I

		Type 4 features,
		Feature value is ABDE+EFHI-BCEF-DEGH 
		= ACGI - 2(BCEF) - 2(DEGH)
		= 4E - 2(B+D-H-F)+A+C+G+I
*/
		
LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++] || R7 = [P3++];

R6 = R0 + R7 (NS) || R2 = [I2++] || R5 = [P2++];	// A + I
R7 = R2 + R5 (NS) || P4 = LOAD_E_POINTER;			// C + G
R7 = R6 + R7 (NS) || R1 = [I1++] || R3 = [P0++];	// (A + I) + (C + G)

R0 = R1 + R3 (NS)  || R6 = [I3++] || R4 = [P1++];	// B+D
R2 = R4 + R6 (NS)  || R3 = LOAD_WEIGHT;									// H+F
R6 = R0 + R2 (NS)  || R2 = [P4++];					// (B+D) + (H+F)
R6 = R6 + R6 (S)  || LOAD_E_POINTER = P4;				// 2*((B+D) + (H+F))
R2 = R2 << 2 || P4 = LOAD_INNER_LOOP_COUNT;
R6 = R7 - R6 (S) || R0 = [I0++] || R7 = [P3++];			// (A + I) + (C + G) - 2*((B+D) + (H+F))
R6 = R6 + R2 (S) || R2 = [I2++] || R5 = [P2++];			// 4E - 2*((B+D) + (H+F)) + A+C+G+I
R1 = ABS R6 || R4 = LOAD_NORM_EXP;
R6 *= R3;
R6 *= R1;
R4 = MAX(R1, R4);

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

LOAD_NORM_EXP = R4;
R6 = R0 + R7 (NS) || [P5++] = R6;					// A + I
R7 = R2 + R5 (NS)  || P4 = LOAD_E_POINTER;			// C + G
R7 = R6 + R7 (NS) || R1 = [I1++] || R3 = [P0++];	// (A + I) + (C + G)

R0 = R1 + R3 (NS)  || R6 = [I3++] || R4 = [P1++];	// B+D
R2 = R4 + R6 (NS)  || R3 = LOAD_WEIGHT;				// H+F
R6 = R0 + R2 (NS)  || R2 = [P4++];					// (B+D) + (H+F)
R6 = R6 + R6 (S)  || LOAD_E_POINTER = P4;				// 2*((B+D) + (H+F))
R2 = R2 << 2 || P4 = LOAD_INPUT_WIDTH_STEP;
R6 = R7 - R6 (S) || R0 = [I0++] || R7 = [P3++];		// (A + I) + (C + G) - 2*((B+D) + (H+F))
R6 = R6 + R2 (S) || R2 = [I2++] || R5 = [P2++];		// 4E - 2*((B+D) - (H+F)) + A+C+G+I
R1 = ABS R6 || R4 = LOAD_NORM_EXP;
R6 *= R3;
R6 *= R1;
R4 = MAX(R1, R4);

LOOP_END INNER_WIDTH_LOOP;

LOAD_NORM_EXP = R4;

[-- SP] = P5;
P5 = LOAD_E_POINTER;
P2 = P2 + P4;
P0 = P0 + P4;
P1 = P1 + P4;
P5 = P5 + P4;
P5 += 4;
P3 = P3 + P4;
P0 += 4;
P1 += 4;
LOAD_E_POINTER = P5;
P5 = [SP ++];

P4 = LOAD_OUTPUT_WIDTH_STEP;
I3 += M0;
I2 += M0;
I0 += M0;
I1 += M0;
I3 += 4;
I1 += 4;

[P5 ++ P4] = R6;

LOOP_END OUTER_HEIGHT_LOOP;

R0 = LOAD_NORM_EXP;
(R7:1,P5:0) = [SP ++];

UNLINK;
RTS;

_compute_haar_feature_type_4.END:


#endif //__BFIN__
