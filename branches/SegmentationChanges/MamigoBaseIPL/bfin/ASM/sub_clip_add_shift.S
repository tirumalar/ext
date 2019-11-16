#ifdef __BFIN__
/*****************************************************************************
 * sub_clip_add_shift.asm
 *****************************************************************************/

.text;

.align 4;

.global _sub_clip;
.type _sub_clip, STT_FUNC;

_sub_clip:

/*
R0 -> address of input image0
R1 -> address of input image1
R2 -> address of a structure containing output image and dims (2 bytes packed W,H)
FP+0x14-> cliphi cliplo packed in 32 bits
*/

LINK 0;
[ -- SP] = (R7:0,P5:0);
P0=R2;
I0=R0;		/* input image1*/
I1=R1;		/* input image2*/
R7=W[FP+0x14]; // read from stack
R3=W[FP+0x16]; // read from stack
R0=0;

P1=[P0++];  /*P1 is now output writer */
R2=[P0];	/*R2.L has Width, R2.H has Height*/
R1=R2.L(Z);	// R1.L is w
R2=R2>>16;	// h
R2*=R1;
LC0=R2;

R3.H=R3.L+R0.L(NS);	//cliplo,cliplo	to support parallel ops
R7.H=R7.L+R0.L(NS); //cliphi,cliphi to support parallel ops


R0=[I0++]||R2=[I1++];
LOOP CALC LC0;
LOOP_BEGIN CALC;

(R0,R1)=BYTEOP16M(R3:2,R1:0);

R0=MIN(R0,R7)(V)||R2=[I1++];
R0=MAX(R0,R3)(V);
R1=MIN(R1,R7)(V);
R1=MAX(R1,R3)(V);
R1=BYTEPACK(R1,R0);
[P1++]=R1||R0=[I0++];
LOOP_END CALC;


(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;
_sub_clip.END:

/******************************************sub_add_clip_shift********************/

.global _sub_clip_add_shift;
.type _sub_clip_add_shift, STT_FUNC;

_sub_clip_add_shift:

/*
R0 -> address of input image0
R1 -> address of input image1
R2 -> address of a structure containing output image and dims (2 bytes packed W,H)
FP+0x14 -> B where 2^B is the number of bins
FP+0x16 -> S where S is the shift

*/

LINK 0;
[ -- SP] = (R7:0,P5:0);
P0=R2;

P1=[P0++];  /*P1 is now output writer */
R2=[P0];	/*R2.L has Width, R2.H has Height*/
R3=R2.L(Z);	// R3.L is w
R2=R2>>16;	// h
R2*=R3;
LC0=R2;

I0=R0;		/* input image1*/
I1=R1;		/* input image2*/
R7=B[FP+0x14](Z); // read from stack, numbits
R3=B[FP+0x16](Z); // read from stack, shifts
R0=8;
R4=R7-R0;	// scale down the add/clippsed difference by this many bits
R7=R7+R3;
R7=R0-R7;	//(8-B+S)
R3.L=0x8000;	// adder is always 0x80 and we need it it z3 and z1
R3.H=0x8000;

R0=[I0++]||R2=[I1++];
(R0,R1)=BYTEOP16M(R3:2,R1:0); //substract

LOOP CALC1 LC0;
LOOP_BEGIN CALC1;

R2=R3;
R1 = LSHIFT R1 BY R7.L (V); // scale up
R0 = LSHIFT R0 BY R7.L (V); // scale up

R5=BYTEOP3P(R1:0,R3:2)(LO,R); //add and clip R1+R3
R6=BYTEOP3P(R1:0,R3:2)(LO);	  //add and clip R0+R2

R5 = LSHIFT R5 BY R4.L (V) ;
R6 = LSHIFT R6 BY R4.L (V) ;

R1=BYTEPACK(R5,R6)||R0=[I0++]||R2=[I1++];

[P1++]=R1||(R0,R1)=BYTEOP16M(R3:2,R1:0); //substract

LOOP_END CALC1;
NOP;

(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

_sub_clip_add_shift.END:

#endif //__BFIN__
