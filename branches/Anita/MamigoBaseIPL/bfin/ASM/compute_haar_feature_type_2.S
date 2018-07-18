#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits

.text;

.align 4;

.global _compute_haar_feature_type_2;
.type _compute_haar_feature_type_2, STT_FUNC;

_compute_haar_feature_type_2:

#define LOAD_WEIGHT [FP-12]
#define LOAD_INPUT_WIDTH_STEP [FP-16]
#define LOAD_OUTPUT_WIDTH_STEP [FP-20]
#define LOAD_INNER_LOOP_COUNT [FP-24]
#define LOAD_NORM_EXP [FP-28]

LINK 28;
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
P0 = R0; // x1

R1 = [P4++];	
I1 = R1;	
P1 = R1; // x2

R2 = [P4++];	
I2 = R2;	
P2 = R2; // x3

R3 = [P4++];
I3 = R3;	
P3 = R3; // x4

[-- SP] = P5;

R4 = [P4++];	// y1 must be 0
R4 = [P4++];	//skip
P5 = [P4++];  	// y3
R4 = [P4++];	//skip

P0 = P0 + (P5 << 2);
P1 = P1 + (P5 << 2);
P2 = P2 + (P5 << 2);
P3 = P3 + (P5 << 2);

P5 = [SP ++];

R0 = [P4++];	// extra input width step;
M0 = R0;
LOAD_INPUT_WIDTH_STEP = R0;


R0 = 0 (Z);
LOAD_NORM_EXP = R0;
		/*
		A ---------------- E 4
		|                  | 
		B ---------------- F 5
		|                  | 
		C ---------------- G 6
		|                  | 
		D ---------------- H 7
		
		answer = A+H-D-E+3(C+F-B-G)
		
		*/
		
LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++] || R4 = [P0++];

R6 = R0 - R4 (S) || R3 = [I3++] || R7 = [P3++];		// A - E
R7 = R7 - R3 (S) || P4 = LOAD_INNER_LOOP_COUNT;		// H - D
R7 = R6 + R7 (NS) || R1 = [I1++] || R5 = [P1++];	// (A - E) + (H - D)

R0 = R5 - R1 (S)  || R2 = [I2++] || R6 = [P2++];	// F - B
R4 = R2 - R6 (S);								// C - G
R6 = R0 + R4 (S)  || R0 = [I0++] || R4 = [P0++];	// (F - B) + (C - G)
R2 = R6 + R7 (S)  || R3 = [I3++] || R7 = [P3++];
R6 = R6 << 1	  ;
R2 = R6 + R2 (S)  || R6 = LOAD_WEIGHT;
R1 = ABS R2 || R5 = LOAD_NORM_EXP;
R2 *= R6;
R2 *= R1;
R5 = MAX(R1, R5);

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

R6 = R0 - R4 (S) || [P5++] = R2;					// A - E
R7 = R7 - R3 (S) || LOAD_NORM_EXP = R5;									// H - D
R7 = R6 + R7 (NS) || R1 = [I1++] || R5 = [P1++];	// (A - E) + (H - D)

R0 = R5 - R1 (S)  || R2 = [I2++] || R6 = [P2++];									// F - B
R4 = R2 - R6 (S)  || P4 = LOAD_INPUT_WIDTH_STEP;	// C - G
R6 = R0 + R4 (S)  || R0 = [I0++] || R4 = [P0++];	// (F - B) + (C - G)
R2 = R6 + R7 (S)  || R3 = [I3++] || R7 = [P3++];	// (A - E) + (H - D) + ((F - B) + (C - G))
R6 = R6 << 1;
R2 = R6 + R2 (S)  || R6 = LOAD_WEIGHT;	// (A - E) + (H - D) + 3*((F - B) + (C - G))
R1 = ABS R2 || R5 = LOAD_NORM_EXP;
R2 *= R6;
R2 *= R1;
R5 = MAX(R1, R5);

LOOP_END INNER_WIDTH_LOOP;

LOAD_NORM_EXP = R5;

P2 = P2 + P4;
P0 = P0 + P4;
P1 = P1 + P4;
P3 = P3 + P4;
P2 += 4;
P1 += 4;

P4 = LOAD_OUTPUT_WIDTH_STEP;
I2 += M0;
I0 += M0;
I1 += M0;
I3 += M0;
I2 += 4;
I1 += 4;

[P5 ++ P4] = R2;

LOOP_END OUTER_HEIGHT_LOOP;

R0 = LOAD_NORM_EXP;
(R7:1,P5:0) = [SP ++];

UNLINK;
RTS;

_compute_haar_feature_type_2.END:


#endif //__BFIN__
