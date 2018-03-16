#ifdef __BFIN__
.text;

.align 4;

.global _HaarDecisionPositiveThreshold;
.type _HaarDecisionPositiveThreshold, STT_FUNC;

#define LOAD_SCORE [P5]
#define SAVE_SCORE [P5++]
#define LOAD_ZERO [FP-12]
#define LOAD_BASE_SCORE [FP-16]
#define LOAD_GOOD_SCORE [FP-20]
#define LOAD_TS_MINUS_32_EXPONENT [FP-24]
#define LOAD_MAX_SCORE [FP-28]
#define LOAD_FEATURE_SHIFT [I2]
#define LOAD_FEATURE_SCALE [I3]

_HaarDecisionPositiveThreshold:

LINK 28;
[ -- SP] = (R7:1,P5:0);

L0 = 0;
L1 = 0;
L2 = 0;
L3 = 0;

I0 = R0;	// feature_vector
I1 = R1;	// variance vector
P0 = R2;	// parameter structure

P4 = [P0++];	// width
R0 = [P0++];	// height
LC0 = R0;

P5 = [P0++];	// output score pointer
R1 = [P0++];	// threshold
R6 = [P0++];	// threshold exponent
R6 += -32;
LOAD_TS_MINUS_32_EXPONENT = R6;

R0 = [P0++];	// base score
LOAD_BASE_SCORE = R0;
R0 = [P0++];	// good score
LOAD_GOOD_SCORE = R0;

I2 = P0;
I3 = P0;
I3 += 4;

R0 = 0;
LOAD_ZERO = R0;

R2 = 0;

// R1 is the threshold
// R7 is always the loaded feature
// R6 is the threshold's exponent  (Ts - 32)

LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R6 = LOAD_BASE_SCORE;
R4 = LOAD_FEATURE_SHIFT;

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

R7 = [I0++] || R0 = [I1++];	// loading feature value and variance values

CC = R7 <= 0;
IF CC JUMP COMPARISION_FINISHED;

R7 = ASHIFT R7 BY R4.L || R5 = LOAD_FEATURE_SCALE;
R7 *= R5;

R5.L = SIGNBITS R0 || LOAD_MAX_SCORE = R2;
R0 = ASHIFT R0 BY R5.L;

R4 = R0.H * R1.H (FU);
R2 = R0.H * R1.L, R3 = R0.L * R1.H (FU);
R2 = R2 + R3 (NS) || R3 = LOAD_TS_MINUS_32_EXPONENT;
R5 = R5 + R3 (S) ;	// Ts + Vs - 32
R2 >>= 16;
R4 = R4 + R2 (NS) || R2 = LOAD_ZERO;
R2.L = SIGNBITS R4;
R5 = R5 + R2 (S);	// R5.L = Ts + Vs + Rs - 32 - total scaling of the left part
R4 = ASHIFT R4 BY R2.L || R3 = LOAD_ZERO;

R3.L = SIGNBITS R7;	// exponent of the feature values
R7 = ASHIFT R7 BY R3.L || R2 = LOAD_MAX_SCORE;

// R3 is the extra scaling of feature values
// R5 is the extra scaling of variance * threshold values
// whichever have a larger extra, must be smaller
// so if R5 < R3 => feature value must be smaller than than the variance*threshold 
CC = R5 < R3;	// comparing the exponents, implies feature value is smaller than threshold
IF CC JUMP COMPARISION_FINISHED;
R6 = LOAD_GOOD_SCORE;
// else either the exponents are equal or exponent of R5 is larger
CC = R3 < R5;	// LHS has extra scaling than RHS, implies feature value is bigger
IF CC JUMP COMPARISION_FINISHED; // clearly RHS is bigger than threshold
// else R3 == R5, only thing left is to compare the decimals
CC = R4 < R7;	// comparing the decimals, implies RHS is bigger than threshold
IF CC JUMP COMPARISION_FINISHED;
R6 = LOAD_BASE_SCORE;

COMPARISION_FINISHED:
R3 = LOAD_SCORE;
R3 = R3 + R6 (S) || R6 = LOAD_BASE_SCORE;
R2 = MAX(R2, R3);
R4 = LOAD_FEATURE_SHIFT || SAVE_SCORE = R3;

LOOP_END INNER_WIDTH_LOOP;

LOOP_END OUTER_HEIGHT_LOOP;

R0 = R2;
(R7:1,P5:0) = [SP++];

UNLINK;
RTS;
_HaarDecisionPositiveThreshold.END:


#endif //__BFIN__
