// This ASSUMES that the output is within 16 bits

#define LOAD_SPECULARITY_DIFFERENCE_WORDS   [FP-12]
#define Y_COUNT 						 	[FP-16]
#define X_COUNT 							[FP-20]
#define INPBASE_POINTER 					[FP-24]
#define TOPOFFSET							[FP-28]
#define CURRENT_BASE 						[FP-32]
#define STEP_SIZE   						[FP-36]
#define OUTBASE_POINTER  					[FP-40]
#define MAX_POINT							[FP-44]
.text;
.align 4;
.global _detect_single_specularity;
.type _detect_single_specularity, STT_FUNC;

_detect_single_specularity:
LINK 52;
[--SP] = (R7:4,P5:0);
R4 = 4;
Y_COUNT = R4;
X_COUNT = R4;
P0 = R2;
INPBASE_POINTER = R0;
CURRENT_BASE = R0;
OUTBASE_POINTER = R1;

R2 = [P0++];//GET WIDTH
R2 +=-8;
R2 = R2>>2; // GET WIDTH COUNT = (W-8)/4
P5 = R2;

R2 = [P0++];//GET HEIGHT
R3 = R2;
R2 +=-8; // GET HEIGHT COUNT = (H-8)
P4 = R2;

R2 = [P0++];// GET STEP SIZE
STEP_SIZE = R2;
R5 = [P0++]; //GET THRESHOLD
LOAD_SPECULARITY_DIFFERENCE_WORDS = R5;
R5 = [P0++];
MAX_POINT = R5;
R4 = 4;
R4 = R2.L*R4.L(FU);
TOPOFFSET = R4;// TOPOFFSET

P0 = R1;//OUTPUT POINTER


LOOP OUTER_HEIGHT_LOOP LC0 = P4;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

LOOP INNER_WIDTH_LOOP LC1 = P5;
LOOP_BEGIN INNER_WIDTH_LOOP;


//a0 b0 c0 d0 e0 f0 g0 h0 i0
//a1 b1 c1 d1 e1 f1 g1 h1 i1
//a2 b2 c2 d2 e2 f2 g2 h2 i2
//a3 b3 c3 d3 e3 f3 g3 h3 i3
//a4 b4 c4 d4 e4 f4 g4 h4 i4
//a5 b5 c5 d5 e5 f5 g5 h5 i5
//a6 b6 c6 d6 e6 f6 g6 h6 i6
//a7 b7 c7 d7 e7 f7 g7 h7 i7
//a8 b8 c8 d8 e8 f8 g8 h8 i8

R0 = CURRENT_BASE;
I1 = R0;
R1 = TOPOFFSET;
R1 = R0 - R1(NS);
I0 = R1;// TOP POINTER

R1 = TOPOFFSET;
R1 = R1 + R0(NS);
I2 = R1;// BOTTOM POINTER

// I1 POINTS TO BASE POINTER
// R2 = Current 4 bytes e4 f4 g4 h4
R2 = [I1--]; //NOW I1 POINTS TO LEFT
// left
R0 = [I1++]; //NOW IT POINTS TO BASE
// R0 = Left 4 bytes a4 b4 c4 d4
(R7, R6) = BYTEOP16M(R3:2, R1:0) || R0 = [I0++] || R3 = LOAD_SPECULARITY_DIFFERENCE_WORDS;
R7 = MIN(R7, R3) (V)||R1 = [I1++];//NOW I1 POINTS TO RIGHT
R6 = MIN(R6, R3) (V);

// top
// R0 = Top 4 bytes e0 f0 g0 h0
(R5, R4) = BYTEOP16M(R3:2, R1:0) || R0 = [I2++];//LOAD BOTTOM DATA
R7 = MIN(R7, R5) (V);
R6 = MIN(R6, R4) (V);

// bottom
// R0 = Bottom 4 bytes e8 f8 g8 h8
(R5, R4) = BYTEOP16M(R3:2, R1:0) || R0 = [I1]; //LOAD RIGHT DATA//NOW I1 POINTS TO RIGHT
R7 = MIN(R7, R5) (V);
R6 = MIN(R6, R4) (V);

// right
// R0 = Right 4 bytes i4 j4 k4 l4
(R5, R4) = BYTEOP16M(R3:2, R1:0) || R1 = LOAD_SPECULARITY_DIFFERENCE_WORDS;
R7 = MIN(R7, R5) (V);
R6 = MIN(R6, R4) (V);

// if the four values are greater than neighbouring 4 by a threshold then
// we need to look closely.
// Now find the Max between these four and check if it is threshold then definitely
// one or more among the four register is > its neighbour

R4 = MAX(R6,R7)(V);
R5 = PACK(R4.L,R4.H);
R5 = MAX(R4,R5)(V);
R0 = I1;
CURRENT_BASE = R0;

CC = R5 == R1; //T T
IF CC JUMP INVESTIGATE_LIKELY_SPECULARITY;

RETURN_LOCATION_FOR_INVESTIGATE_LIKELY_SPECULARITY:
//INCREMENT X COUNT
R4 = X_COUNT;
R4+=4;
X_COUNT = R4;
// INCREMENT THE POINTER OF BASE
LOOP_END INNER_WIDTH_LOOP;

R0 = INPBASE_POINTER;//CURRENT_BASE;
R1 = STEP_SIZE;
R0 = R0+R1(NS);
CURRENT_BASE = R0;
INPBASE_POINTER = R0;

R4 = 4;
X_COUNT = R4;
R4 = Y_COUNT;
R4+=1;
Y_COUNT = R4;

LOOP_END OUTER_HEIGHT_LOOP;

R0 = P0;
R1 = OUTBASE_POINTER;
R0 = R0 - R1;
R0 = R0>>2;
RETURN_ERROR:
(R7:4,P5:0) = [SP ++];
UNLINK;
RTS;

INVESTIGATE_LIKELY_SPECULARITY:

//Now where are the pointers pointing
//i0 points to  i j k l
//i1 points to  i4 j4 k4 l4
//i2 points to  i8 j8 k8 l8
//i3 points to  not inititalized
//r6 and r7 have the respective minimas.
//r2 has e4 f4 g4 h4.
R0 = TOPOFFSET;
R0 +=8;
R1 = -R0;
M0 = R1;
M1 = 1;
I1 += M0; // I1 POINTS TO A0 B0 C0 D0


I3 = I1; //SAVE THE ABCD POINTER

//TOP ROW CHECK
R0 = [I1++]; // A0B0C0D0
R1 = [I1++]; // E0F0G0H0

// E4-A0 F4-B0 G4-C0 H4-D0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-B0 F4-C0 G4-D0 H4-E0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-C0 F4-D0 G4-E0 H4-F0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-D0 F4-E0 G4-F0 H4-G0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V) || I1 += M1;


R0 = R1; //EFGH
R1 = [I0]; //IJKL

// E4-F0 F4-G0 G4-H0 H4-I0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-G0 F4-H0 G4-I0 H4-J0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-H0 F4-I0 G4-J0 H4-K0
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-I0 F4-J0 G4-K0 H4-L0
//BUG FIX
R0 = R1;
// E4-I F4-J G4-K H4-L
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V);
R6 = MIN(R6, R4) (V);

//I3 HAS THE POINTER OF ABCD


R0 = STEP_SIZE;
M2 = R0;
M3 = 8;
I3+=M2;
//I3 POINTS TO A1B1C1D1
I2 = I3;
I2+= M3||R0 = [I3++M2];
//I2 POINTS TO I1JIKILI

//R0W 1
// E4-A1 F4-B1 G4-C1 H4-D1
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];
R6 = MIN(R6, R4) (V);
// E4-I1 F4-J1 G4-K1 H4-L1
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I3++M2];
R6 = MIN(R6, R4) (V);
//ROW 2
// E4-A2 F4-B2 G4-C2 H4-D2
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];
R6 = MIN(R6, R4) (V);
// E4-I2 F4-J2 G4-K2 H4-L2
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I3++M2];
R6 = MIN(R6, R4) (V);

//ROW 3
// E4-A3 F4-B3 G4-C3 H4-D3
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];//I3J3K3L3
R6 = MIN(R6, R4) (V);
// E4-I3 F4-J3 G4-K3 H4-L3
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I3++M2]; //A4B4C4D4
R6 = MIN(R6, R4) (V) || R0 = [I3++M2]; //A5B5C5D5

//ROW 5
// E4-A5 F4-B5 G4-C5 H4-D5
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];//I4J4K4L4
R6 = MIN(R6, R4) (V) || R0 = [I2++M2];//I5J5K5L5;
// E4-I5 F4-J5 G4-K5 H4-L5
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I3++M2];//A6B6C6D6
R6 = MIN(R6, R4) (V);

//ROW 6
// E4-A6 F4-B6 G4-C6 H4-D6
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];//I6J6K6L6
R6 = MIN(R6, R4) (V);
// E4-I6 F4-J6 G4-K6 H4-L6
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I3++M2];//A7B7C7D7
R6 = MIN(R6, R4) (V);


//ROW 7
// E4-A7 F4-B7 G4-C7 H4-D7
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || R0 = [I2++M2];//I7J7K7L7
R6 = MIN(R6, R4) (V);
// E4-I7 F4-J7 G4-K7 H4-L7
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V)|| R0 = [I3++];//A8B8C8D8
R6 = MIN(R6, R4) (V)|| R1 = [I3++];//E8F8G8H8;

//BOTTOM ROW CHECK

// E4-A8 F4-B8 G4-C8 H4-D8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-B8 F4-C8 G4-D8 H4-E8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-C8 F4-D8 G4-E8 H4-F8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-D8 F4-E8 G4-F8 H4-G8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V) || I1 += M1;


R0 = R1; //E8F8G8H8
R1 = [I3]; //I8J8K8L8

// E4-F8 F4-G8 G4-H8 H4-I8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-G8 F4-H8 G4-I8 H4-J8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);
// E4-H8 F4-I8 G4-J8 H4-K8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);

//BUG FIX
R0 = R1;  //I8J8K8L8

// E4-I8 F4-J8 G4-K8 H4-L8
(R5, R4) = BYTEOP16M(R3:2, R1:0);
R7 = MIN(R7, R5) (V) || I1 += M1;
R6 = MIN(R6, R4) (V);

R4 = MAX(R6,R7)(V);
R5 = PACK(R4.L,R4.H);
R5 = MAX(R4,R5)(V)|| R1 = LOAD_SPECULARITY_DIFFERENCE_WORDS;
CC = R5 == R1; //T T

IF !CC JUMP RETURN_LOCATION_FOR_INVESTIGATE_LIKELY_SPECULARITY(BP);


R0 = P0;
R3 = OUTBASE_POINTER;
R0 = R0 - R3(NS)||R2 = MAX_POINT;
R3 = R0>>2;
R3 +=-4;
R0 = -1;
CC = R2 < R3;
IF CC JUMP RETURN_ERROR;


R4 = X_COUNT;
R5 = Y_COUNT;
R3 = PACK(R6.L,R6.L); //FIRST VALUE
CC = R3 == R1; //T T
IF !CC JUMP CHECK_SECOND;
//ELSE NOW WRITE IT IN BUFFER
R2 = PACK(R5.L,R4.L);//Y,X
[P0++] = R2;//STORE OUTPUT
CHECK_SECOND:
R4+=1;
R3 = PACK(R6.H,R6.H); //SECOND VALUE
CC = R3 == R1; //T T
IF !CC JUMP CHECK_THIRD;
//ELSE NOW WRITE IT IN BUFFER
R2 = PACK(R5.L,R4.L);//Y,X
[P0++] = R2;//STORE OUTPUT
CHECK_THIRD:
R4+=1;
R3 = PACK(R7.L,R7.L); //THIRD VALUE
CC = R3 == R1; //T T
IF !CC JUMP CHECK_FOURTH;
//ELSE NOW WRITE IT IN BUFFER
R2 = PACK(R5.L,R4.L);
[P0++] = R2;//STORE OUTPUT

CHECK_FOURTH:
R4+=1;
R3 = PACK(R7.H,R7.H); //FOURTH VALUE
CC = R3 == R1; //T T
IF !CC JUMP RETURN_LOCATION_FOR_INVESTIGATE_LIKELY_SPECULARITY;
//ELSE NOW WRITE IT IN BUFFER
R2 = PACK(R5.L,R4.L);//Y,X
[P0++] = R2;//STORE OUTPUT

JUMP RETURN_LOCATION_FOR_INVESTIGATE_LIKELY_SPECULARITY;


_detect_single_specularity.END: