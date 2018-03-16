
#ifdef __BFIN__
/*****************************************************************************
 * gaus5_expand_reflect.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_expand_reflect;
.type _gaus5_expand_reflect, STT_FUNC;
/*
R0 -> address of input image (small)
R1 -> address of the scratch image
R2 -> address of a structure containing output image (large) and dims (2 bytes packed W,H)
assumtions:
1. the filter is 3x3.
2. R1 has at least 3*4*W bytes of space
3. output image has at least W*H*4 bytes allocated

IMPORTANT: M0:3 has no relevance to I0:I3
*/

#define temp0 [FP-12]
#define temp1 [FP-16]

_gaus5_expand_reflect:

LINK 20;
[ -- SP] = (R7:0, P5:0);

P4=R2;
I3=R1;  	/* scratch pixel circular buffer*/
I2=R1;		/* scrtach row circular buffer*/
I0=0;		/* byte alignment for the byte unpack operation */
I1=0;
P0=R0;		/* input image*/
R6=2;
LC0=R6;
R7=3;
P5=[P4++];  /*P5 is now output writer */
R2=[P4];	/*R2.L has Width, R2.H has Height*/

R3=R2.L(Z);	// R3.L is w

R5=R3;
R5<<=1;
M0=R5;		//2*w
R5<<=1;		//v=4*w
M1=R5;		// size of one scratch row;
P4=P5;

R5 *= R7;	//3*v
L3=R5;		//size of circular index (12*w)
L2=R5;		//size of circular index (12*w)

R2=R2>>16;	// height
M2=R2;
B3=I3; 		/* scratch pixel circular buffer is initialized */
B2=I2; 		  /* scrtach row circular buffer is ready*/

P4=M0;
P4=P4+P5;	//writer2

R3>>=1;		//w/2
M3=R3;
R3>>=1;		//w/4
R6=R3;
LC1=R6;

R7.L=6; // constant
R7.H=8;
/************************************** First 2 rows calc *************/
/* Move I2 to middle scratch row I3 to last row */
I2+=M1;

R1=[P0++];
(R1,R0)=BYTEUNPACK R1:0 (R);

LOOP TOP2ROWS LC0;
LOOP_BEGIN TOP2ROWS;
R2.L=R0.H<<1;			// V Border Extn
A0=R2;
LOOP EACH_ROW_HCALC LC1;
LOOP_BEGIN EACH_ROW_HCALC;

R2=(A0+=R7.L*R0.L)(IS);
R2.H=R0.L+R0.H(NS);
R2.H=R2.H<<2;
[I2++]=R2||R3=[P0++];	// FOR NEXT LOOP

R2=R0+R1;
A0=R2;

R2=(A0+=R7.L*R0.H)(IS);
R2.H=R0.H+R1.L(NS);
R2.H=R2.H<<2;
[I2++]=R2||A0=A0>>16;

R2=R1; 					// FOR NEXT LOOP

R4=(A0+=R7.L*R1.L)(IS);
R4.H=R1.L+R1.H(NS);
R4.H=R4.H<<2;

[I2++]=R4||(R1,R0)=BYTEUNPACK R3:2 (R);	// FOR NEXT LOOP

R4=R2+R0;
A0=R4;

R4=(A0+=R7.L*R2.H)(IS);
R4.H=R2.H+R0.L(NS);
R4.H=R4.H<<2;
[I2++]=R4||A0=A0>>16;
LOOP_END EACH_ROW_HCALC;

LC1=R6;	// next row
I2 -=4;
//Use R2 to do the last calc
R4=R2+R2;
A0=R4;
R4=(A0+=R7.L*R2.H)(IS);
R4.H=R2.L+R2.H(NS);
R4.H=R4.H<<2;
[I2++]=R4;

LOOP_END TOP2ROWS;

// R0 and R1 already have the 4 bytes from the next row unpacked
// so lets remember them on stack till main loop
temp1 = R1;
temp0 = R0;

R6=M2;
R6+=-2; //h-2
LC0=R6;	// for main loop

/*******************First Row V Calc******************/
R0=M3;
LC1=R0;

I3+=M1;
P1=I3;		//reader1
I3+=M1;
P2=I3;		//reader2

LOOP TOP_ROW_VCALC LC1;
LOOP_BEGIN TOP_ROW_VCALC;

R2=[P2++];
R1=[P1++]||R0=R2<<1;	// doubling due to v border
R3=R2+R1;

A0=R0||R2=[P2++];		//Load next integer

R4=(A0+=R7.L*R1.L)(IS);
R4+=32;
R4.L=R4.L>>6;

A0=A0>>16;
R6=(A0+=R7.L*R1.H)(IS);
R6+=32;
R4.H=R6.L>>6;

R3+=8;
R3.H=R3.H+R7.H(NS);
R3=R3>>4 (V);
// R3 R4 can not be touched now on

R1=[P1++]||R0=R2<<1;  //Load next integer
A0=R0;
R0=(A0+=R7.L*R1.L)(IS);
R0+=32;
R0.L=R0.L>>6;

A0=A0>>16;
R6=(A0+=R7.L*R1.H)(IS);
R6+=32;
R0.H=R6.L>>6;

R5=R2+R1;
R5+=8;
R5.H=R5.H+R7.H(NS);
R5=R5>>4(V);

R0=BYTEPACK(R4,R0);
[P5++]=R0;

R1=BYTEPACK(R3,R5);
[P4++]=R1;

LOOP_END TOP_ROW_VCALC;

/**************************Main loop********************************/
// I2 for writing to scracth, I3, P2 for reading from scracth
// P0, P4,P5 for output

R0 = temp0;
R1 = temp1;

LOOP MAIN_ROW_LOOP LC0;
LOOP_BEGIN MAIN_ROW_LOOP;
R3=M3;
R3>>=1;		//w/4
R3+=-1;		//w/4-1 (the last 4 bytes need special treatment)
LC1=R3;

P5=P4;		//writer1
P4=M0;
P4=P4+P5;	//writer2
I2+=M1;
P2=I2;
I2-=M1;
R2.L=R0.H<<1;		// doubling due to v border extn

A0=R2;
LOOP MAIN_COL_LOOP LC1;
LOOP_BEGIN MAIN_COL_LOOP;
R2=(A0+=R7.L*R0.L)(IS)||R3=[P2++];
R2.H=R0.L+R0.H(NS)||R4=[I3++];
R2.H=R2.H<<2;
[I2++]=R2;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R5+=8;
R5.H=R5.H+R7.H(NS);
R5=R5>>4(V);
B[P4++]=R5;
R5>>=16;
B[P4++]=R5;

//------------------
R2=R1+R0;

A0=R2||R4=[I3++];
R2=(A0+=R7.L*R0.H)(IS)||R3=[P2++];
R2.H=R0.H+R1.L(NS);
R2.H=R2.H<<2;
[I2++]=R2||A0=A0>>16;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R5+=8;
R5.H=R5.H+R7.H(NS)||R4=[I3++];
R5=R5>>4 (V);
B[P4++]=R5;
R5>>=16;
B[P4++]=R5;

//------

R2=(A0+=R7.L*R1.L)(IS)||R3=[P2++];
R2.H=R1.L+R1.H(NS);
R2.H=R2.H<<2;
[I2++]=R2;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS)||R3=[P0++]; // FOR NEXT LOOP
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R6=R1;	// FOR NEXT LOOP
R5+=8;
R5.H=R5.H+R7.H(NS);
R5=R5>>4 (V);
B[P4++]=R5||(R1,R0)=BYTEUNPACK R3:2 (R); // FOR NEXT LOOP
R5>>=16;
B[P4++]=R5||R4=[I3++];

//------

R2=R6+R0;
A0=R2;
R2=(A0+=R7.L*R6.H)(IS)||R3=[P2++];
R2.H=R6.H+R0.L(NS);
R2.H=R2.H<<2;
[I2++]=R2||A0=A0>>16;  //FOR NEXT LOOP

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R5+=8;
R5.H=R5.H+R7.H(NS);
R5=R5>>4 (V);
B[P4++]=R5;
R5>>=16;
B[P4++]=R5;
LOOP_END MAIN_COL_LOOP;

/************************* Last 4 bytes need special handling ******/
R2=(A0+=R7.L*R0.L)(IS)||R3=[P2++];
R2.H=R0.L+R0.H(NS)||R4=[I3++];
R2.H=R2.H<<2;
[I2++]=R2;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R5+=8;
R5.H=R5.H+R7.H(NS);
R5=R5>>4 (V);
B[P4++]=R5;
R5>>=16;
B[P4++]=R5;

//------------------
R2=R1+R0;

A0=R2||R4=[I3++];
R2=(A0+=R7.L*R0.H)(IS)||R3=[P2++];
R2.H=R0.H+R1.L(NS);
R2.H=R2.H<<2;
[I2++]=R2||A0=A0>>16;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5>>=6;
B[P5++]=R5|| A1=A1>>16;
R5=(A1+=R7.L*R4.H)(IS);
R5+=32;
R5=R5>>6;
B[P5++]=R5;

R5=R2+R4;
R5+=8;
R5.H=R5.H+R7.H(NS)||R4=[I3++];
R5=R5>>4 (V);
B[P4++]=R5;
R5>>=16;
B[P4++]=R5;

//------

R2=(A0+=R7.L*R1.L)(IS)||R3=[P2++];
R2.H=R1.L+R1.H(NS);
R2.H=R2.H<<2;
[I2++]=R2;

R5=R2+R3;
A1=R5;
R5=(A1+=R7.L*R4.L)(IS);
R5+=32;
R5.L=R5.L>>6;
A1=A1>>16;
R6=R1;	// FOR NEXT LOOP
R1=(A1+=R7.L*R4.H)(IS)||R3=[P2++];
R1+=32;
R5.H=R1.L>>6; // do not write R5 yet

R2=R2+R4;
R2+=8;
R2.H=R2.H+R7.H(NS);
R2=R2>>4 (V);

//------

R0=R6<<1; // doubling due to V Border extn
A0=R0;
R0=(A0+=R7.L*R6.H)(IS)||R4=[I3++];
R0.H=R6.H+R6.L(NS);
R0.H=R0.H<<2;
[I2++]=R0;

R6=R0+R3;
A0=R6;
R6=(A0+=R7.L*R4.L)(IS)||R3=[P0++]; // FOR NEXT LOOP
R6+=32;
R6.L=R6.L>>6;
R6=PACK(R5.H,R6.L);
R5=BYTEPACK(R5,R6);
R6=R0+R4;
R6+=8;
R6>>=4;
[P5++]=R5||(R1,R0)=BYTEUNPACK R3:2 (R); // FOR NEXT LOOP
R6=PACK(R2.H,R6.L);
R5=BYTEPACK(R2,R6);
[P4++]=R5;

LOOP_END MAIN_ROW_LOOP;
/**************************End Main loop********************************/


/**************************Start End game: Last Row VCalc***************/
//I3 points to the start of last written scratch row
// P4 is already pointing to the next writing location (where P5 used to write)
// P5 is pointing to a reading location which we can just replicate using P2
R0=M3; //w/2
LC1=R0;
P2=M0;
P2=P2+P4;	//P2 is now writer 2
I2+=M1;


LOOP BTM_ROW_VCALC LC1;
LOOP_BEGIN BTM_ROW_VCALC;

R2=[I2++];
R1=[I3++]||R0=R2<<1;	// doubling due to v border
R3=R2+R1;

A0=R0||R2=[I2++];		//Load next integer

R4=(A0+=R7.L*R1.L)(IS);
R4+=32;
R4.L=R4.L>>6;

A0=A0>>16;
R6=(A0+=R7.L*R1.H)(IS);
R6+=32;
R4.H=R6.L>>6;

R1=[I3++]||R0=R2<<1;  //Load next integer
A0=R0;
R0=(A0+=R7.L*R1.L)(IS)||R5=[P5++];
R0+=32;
R0.L=R0.L>>6;

A0=A0>>16;
R6=(A0+=R7.L*R1.H)(IS);
R6+=32;
R0.H=R6.L>>6;

[P2++]=R5||R0=BYTEPACK(R4,R0);
[P4++]=R0;

LOOP_END BTM_ROW_VCALC;
L3=0;
L2=0;

(R7:0, P5:0) = [SP ++];
UNLINK;
RTS;
_gaus5_expand_reflect.END:

#endif //__BFIN__
