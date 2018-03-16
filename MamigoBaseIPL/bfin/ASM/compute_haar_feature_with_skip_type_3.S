#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits

.text;

.align 4;

.global _compute_haar_feature_with_skip_type_3;
.type _compute_haar_feature_with_skip_type_3, STT_FUNC;

_compute_haar_feature_with_skip_type_3:

#define LOAD_WEIGHT [FP-12]
#define LOAD_INPUT_WIDTH_STEP [FP-16]
#define LOAD_OUTPUT_WIDTH_STEP [FP-20]
#define LOAD_INNER_LOOP_COUNT [FP-24]
#define LOAD_NORM_EXP [FP-28]
#define LOAD_SKIP [FP-32]

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
P0 = R0; // x1
P1 = R0;

R1 = [P4++];	// x2 skip


R2 = [P4++];	
I2 = R2;
I3 = R2;
P2 = R2; // x3
P3 = R2;

R3 = [P4++];	// x4 skip


[-- SP] = P5;

R4 = [P4++];	// y1 must be 0
R4 = [P4++];	// y2
R4 <<= 2;
M1 = R4;

P5 = [P4++];  	// y3

P0 = P0 + (P5 << 2);
P2 = P2 + (P5 << 2);

P5 = [P4++];	// y4

I1 += M1;
I3 += M1;

P1 = P1 + (P5 << 2);
P3 = P3 + (P5 << 2);

P5 = [SP ++];

R0 = [P4++];	// extra input width step;
M0 = R0;
LOAD_INPUT_WIDTH_STEP = R0;
R0 = [P4++];
M3 = R0;	// skip
P4 = R0;
LOAD_SKIP = R0;

R0 = 0 (Z);
LOAD_NORM_EXP = R0;
		/*
		A ------- B ------ C -------- D
		|         |        |          |
		E ------- F ------ G -------- H

		Type 3 features,
		Feature value is CDGH+ABEF-2BCFG = ADEH-3BCFG = A+H-D-E +3(C+F-B-G)
		
		*/
		
LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = LOAD_INNER_LOOP_COUNT;
LC1 = R0;

R0 = [I0++M3] || R3 = [P1++P4];

R6 = R0 - R3 (S) || R4 = [I2++M3] || R7 = [P3++P4];		// A - D
R7 = R7 - R4 (S);										// H - E
R7 = R6 + R7 (NS) || R1 = [I1++M3] || R2 = [P0++P4];	// (A - D) + (H - E)

R0 = R2 - R1 (S)  || R5 = [I3++M3] || R6 = [P2++P4];	// C - B
R3 = R5 - R6 (S);										// F - G
R6 = R0 + R3 (S)  || R0 = [I0++M3] || R3 = [P1++P4];	// (F - G) + (C - B)
R5 = R6 + R7 (S)  || R4 = [I2++M3] || R7 = [P3++P4];	// (A - D) + (H - E) + ((F - G) + (C - B))
R6 = R6 << 1;
R5 = R6 + R5 (S)  || R6 = LOAD_WEIGHT ;	// (A - D) + (H - E) + 3*((F - G) + (C - B))
R1 = ABS R5 || R2 = LOAD_NORM_EXP;
R5 *= R6;
R5 *= R1;
R2 = MAX(R1, R2);

LOOP INNER_WIDTH_LOOP LC1;
LOOP_BEGIN INNER_WIDTH_LOOP;

R6 = R0 - R3 (S) || [P5++] = R5;					// A - D
R7 = R7 - R4 (S) || LOAD_NORM_EXP = R2;									// H - E
R7 = R6 + R7 (NS) || R1 = [I1++M3] || R2 = [P0++P4];	// (A - D) + (H - E)

R0 = R2 - R1 (S)  || R5 = [I3++M3] || R6 = [P2++P4];									// C - B
R3 = R5 - R6 (S);										// F - G
R6 = R0 + R3 (S)  || R0 = [I0++M3] || R3 = [P1++P4];	// (F - G) + (C - B)
R5 = R6 + R7 (S)  || R4 = [I2++M3] || R7 = [P3++P4];	// (A - D) + (H - E) + ((F - G) + (C - B))
R6 = R6 << 1;
R5 = R6 + R5 (S)  || R6 = LOAD_WEIGHT;	// (A - D) + (H - E) + 3*((F - G) + (C - B))
R1 = ABS R5 || R2 = LOAD_NORM_EXP;
R5 *= R6;
R5 *= R1;
R2 = MAX(R1, R2);

LOOP_END INNER_WIDTH_LOOP;
P2 = P2 + P4;
P0 = P0 + P4;
P4 = LOAD_INPUT_WIDTH_STEP;
LOAD_NORM_EXP = R2;

P2 = P2 + P4;
P0 = P0 + P4;
P1 = P1 + P4;
P3 = P3 + P4;

P4 = LOAD_OUTPUT_WIDTH_STEP;
I3 += M0;
I2 += M0;
I0 += M0;
[P5 ++ P4] = R5;
P4 = LOAD_SKIP;
I1 += M0;
I3 += M3;
I1 += M3;


LOOP_END OUTER_HEIGHT_LOOP;

R0 = LOAD_NORM_EXP;
(R7:1,P5:0) = [SP ++];

UNLINK;
RTS;

_compute_haar_feature_with_skip_type_3.END:


#endif //__BFIN__
