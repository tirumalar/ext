#ifdef __BFIN__

.text;

.align 4;

.global _compute_integral_image;
.type _compute_integral_image, STT_FUNC;

.global _compute_integral_with_square_image;
.type _compute_integral_with_square_image, STT_FUNC;

_compute_integral_image:

/*
R0 -> input value (could be 32 bits)

*/


LINK 0;
[ -- SP] = (R7:0,P5:0);

L2 = 0;
L3 = 0;

P0 = R0;
I2 = R1;
P1 = R2;


// I0 and I1 are pointers to a circular scratch
// scratch need to (width + 1)*4 bytes
// I2 is the output pointer
// P0 is input poiter
// P5 is the offset

P2 = 1;
P4 = [P1++];	// width of the image
R0 = [P1++];	// height
LC0 = R0;
P5 = [P1++];	// extra widthstep for the input
R0 = [P1++];	// 4 byte aligned scratch pointer
B0 = R0;
B1 = R0;
I0 = R0;
I1 = R0;
R0 = P4;
R0 += 1;
R0 <<=2;
L0 = R0;
L1 = R0;

R0 = 0;


LOOP INNER_WIDTH_INIT_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_INIT_LOOP;
[I2++] = R0;
[I0++] = R0;
LOOP_END INNER_WIDTH_INIT_LOOP;

[I0++] = R0;	// making scratch 0 and copying it to output image as well
[I2++] = R0;


LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R3 = 0;
R0 = [I1++];
R1 = [I1++];
R7 = B [P0++] (Z);

LOOP INNER_WIDTH_LOOP LC1 = P4 >> 1;
LOOP_BEGIN INNER_WIDTH_LOOP;

// 0 | 1
// 3 | 4  // 4 being estimated

R4 = R3 - R0 (S) || R0=[I1++]; 
R4 = R4 + R7 (S) || R7 = B [P0++] (Z) || [I2++] = R3;
R4 = R4 + R1 (S) || [I0++] = R3 ;


// 1 | 0
// 4 | 3	3 being estimated

R3 = R4 - R1 (S) || R1=[I1++];
R3 = R3 + R7 (S) || R7 = B [P0++] (Z) || [I2++] = R4;
R3 = R3 + R0 (S) || [I0++] = R4 ;

LOOP_END INNER_WIDTH_LOOP;

P0 = P0 + P5;	// P5 is the extra widthstep

I1 -=4 || [I0++] = R3;
[I2++] = R3;
P0 -= P2;

LOOP_END OUTER_HEIGHT_LOOP;

L0 = 0;	// resetting these back
L1 = 0;

(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_integral_image.END:



_compute_integral_with_square_image:

/*
R0 -> input value (could be 32 bits)

*/


LINK 0;
[ -- SP] = (R7:0,P5:0);

L2 = 0;
L3 = 0;

P0 = R0;
I2 = R1;	// output pointer for integral
P1 = R2;


// I0 and I1 are pointers to a circular scratch
// scratch need to (width + 1)*8 bytes
// I2 is the output pointer
// P0 is input poiter
// P5 is the offset

P2 = 1;
P4 = [P1++];	// width of the image
R0 = [P1++];	// height
LC0 = R0;
P5 = [P1++];	// extra widthstep for the input
R0 = [P1++];	// 4 byte aligned scratch pointer
B0 = R0;
B1 = R0;
I0 = R0;
I1 = R0;
R0 = [P1++];	//output pointer for integral sq
I3 = R0;

R0 = P4;
R0 += 1;
R0 <<=3;
L0 = R0;
L1 = R0;

R0 = 0;


LOOP INNER_WIDTH_INIT_SQR_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_INIT_SQR_LOOP;
[I2++] = R0; 
[I0++] = R0;
[I3++] = R0; 
[I0++] = R0;
LOOP_END INNER_WIDTH_INIT_SQR_LOOP;

[I0++] = R0;	// making scratch 0 and copying it to output image as well
[I2++] = R0;
[I0++] = R0;	// making scratch 0 and copying it to output image as well
[I3++] = R0;


LOOP OUTER_HEIGHT_SQR_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_SQR_LOOP;

R3 = 0;
R5 = 0;
R0 = [I1++];
R2 = [I1++];
R1 = [I1++];
R6 = [I1++];
R7 = B [P0++] (Z);

LOOP INNER_WIDTH_SQR_LOOP LC1 = P4 >> 1;
LOOP_BEGIN INNER_WIDTH_SQR_LOOP;

// 0 | 1
// 3 | 4  // 4 being estimated
R4 = R3 - R0 (S) || R0=[I1++]; 
R4 = R4 + R7 (S) || [I2++] = R3;		// saving into scratch
R4 = R4 + R1 (S) || [I0++] = R3 ;		// saving into output

// 2 | 6
// 5 | 3
R7 = R7.L * R7.L (IS) || [I0++] = R5;		// saving to output sqr
R3 = R5 - R2 (NS) || [I3++] = R5 || R2 = [I1++];	// saving to scratch sqr
R3 = R3 + R7 (NS) || R7 = B [P0++] (Z) || [I0++] = R4;		// saving to output 
R3 = R3 + R6 (NS) || [I2++] = R4;		// saving to scratch

// 1 | 0
// 4 | 3	3 being estimated

R4 = R4 - R1 (S) || R1=[I1++] || [I0] = R3;
R3 = R4 + R7 (S) || [I3++] = R3;
R3 = R3 + R0 (S);

// R3 retained for the next iteration

// 6 | 2
// 3 | 5	5 sqr being estimated
R7 = R7.L * R7.L (IS);
R5 = R2 - R6 (NS) || R6 = [I0++];	// R6 masquerading at R3 for sqr
R5 = R5 + R7 (NS) || R7 = B [P0++] (Z);
R5 = R5 + R6 (NS) || R6=[I1++];

// R5 retained for next iteration

LOOP_END INNER_WIDTH_SQR_LOOP;

P0 = P0 + P5;	// P5 is the extra widthstep

I1 -=4 || [I0++] = R3;
I1 -=4 || [I2++] = R3;
[I0++] = R5;
[I3++] = R5;

P0 -= P2;

LOOP_END OUTER_HEIGHT_SQR_LOOP;

L0 = 0;	// resetting these back
L1 = 0;

(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_integral_with_square_image.END:
#endif //__BFIN__
