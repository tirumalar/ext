#ifdef __BFIN__
/*****************************************************************************
 * GausPyr.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_reduce_4x_b;
.type _gaus5_reduce_4x_b, STT_FUNC;
/*
R0 -> address of input (image of size W, H)
R1 -> address of the output image
R2 -> address of args structure int args[]={W,H,Wstep_in,Wstep_out,scr};
assumtions:
1. scr has at least 5*W/4 shorts (5*W/2 bytes)
3. R1 (output image has at least W*H/4 bytes allocated

IMPORTANT: M0:3 has no relevance to I0:I3
*/

_gaus5_reduce_4x_b:

#define width	     	[FP -12]
#define height	     	[FP -16]
#define inRowEndGap	 	[FP -20]
#define outRowEndGap 	[FP -24]
#define temp1     		[FP -28]
#define temp2     		[FP -32]

#define scratch  		[I0++]
#define in		 		[P1++]
#define out		   	    [P0++]

#define scr1			[I1++]
#define scr2			[P2++]
#define scr3			[P5++]
#define scr4			[I2++]
#define scr5			[I3++]

LINK 40;
[ -- SP] = (R7:4, P5:3);
A0=0;
A1=0;
R3=0;
temp1=R3;
P5=R2; // args structure having width etc.
R5=3;
LC0=R5;
R5=5;			//constt
P0=R1;			//output
P1=R0;			//input
R0=[P5++];
width	=R0;
R1=R0>>1;		//size of each scratch row in bytes
M1=R1;			//wb
R5*=R1;			//5wb;
L0=R5;			//scratch rotates after 5wb
M2=R0;			//2wb
R7=R1>>2;
R7+=-1;
P4=R7;			// to intialize the inner loop to W/2 - 2
R1=[P5++];
height	=R1;
R2=[P5++];
R2=R2<<1;
R2=R2-R0;
inRowEndGap	=R2;
R3=[P5++]||R4=R0>>2;
R3=R3-R4;
outRowEndGap=R3;	//bytes that we need to skip in out row end
R4=R0>>4;			// w/16
M3=R4;				// used a loop counter for VFilter
R4=[P5++]; 		//scratch
I0=R4;
B0=R4;
R4.L=4;
R4.H=6;
I0+=M2; //initially place I0 at beginning of 3rd scratch row

LOOP FIRST_3_ROW_HCALC LC0;
LOOP_BEGIN FIRST_3_ROW_HCALC;
CALL _one_row_HFILTER;
LOOP_END FIRST_3_ROW_HCALC;

//Replicate 2 Rows from scratch
R0=width;
R0=R0>>3;
P5=R0;
I1=I0;			//I0:scr_row_1
I1+=M1;			//I1:scr_row_2
I2=I1;
I2+=M2;			//I2:scr_row_4
I3=I2;
I3+=M1;			//I3:scr_row_5
P5+=-1;			//w-2

R2=[I2++];
R3=[I3++];
LOOP EXTND_TOP_2ROWS LC1=P5;
LOOP_BEGIN EXTND_TOP_2ROWS;
[I0++]=R3||R3=[I3++];
[I1++]=R2||R2=[I2++];
LOOP_END EXTND_TOP_2ROWS;
[I0++]=R3;
[I1++]=R2;

//setup various pointers for the next phase

I0=B0;			// lets keep I0 for writing to scratch
I1=I0;			//I1:scr_row_1
B1=B0;
L1=L0;

I2=B0;			//I2:scr_row_4
B2=B0;
L2=L0;

I3=B0;			//I3:scr_row_5
B3=B0;
L3=L0;

//setup LC0 for main loop
R3=height;
R3=R3>>2;
R3+=-2;			//H/2-2
LC0=R3;
CALL _one_row_VFILTER;
//Main Loop

LOOP MAIN_HEIGHT_LOOP LC0;
LOOP_BEGIN  MAIN_HEIGHT_LOOP;

CALL _one_row_HFILTER;
CALL _one_row_HFILTER;

CALL _one_row_VFILTER;

LOOP_END MAIN_HEIGHT_LOOP;

//last row calc
CALL _one_row_HFILTER;
//bottom replicate 1 row: write at I0 read from P4=I0+M2+M1
R0=width;
R0=R0>>3;
P1=R0;
I0+=M2;
I0+=M1;
P4=I0;
P1+=-1;	//LOOP COUNTER
I0+=M2;

R2=[P4++];
LOOP EXTND_BOTTOM_1ROW LC1=P1;
LOOP_BEGIN EXTND_BOTTOM_1ROW;
[I0++]=R2||R2=[P4++];
LOOP_END EXTND_BOTTOM_1ROW;
[I0++]=R2;

CALL _one_row_VFILTER;

L0=0;
L1=0;
L2=0;
L3=0;

(R7:4, P5:3) = [SP ++];
UNLINK;
RTS;
_gaus5_reduce_4x_b.END:

/***** Sub routines below this ****/

// a sub routine for 1 row horizontal filter

_one_row_HFILTER:
// handle first column
P3=FP;
P3+=-28;
R0=in;
R2=in||R5=R0<<8(V);
R6=R2.B(Z);
R5=R5>>8(V);
A0=R6;
R7.L=(A0+=R5.H*R4.L),R7.H=(A1=R5.L*R4.H)(IU);
R7.L=R7.L<<1;
LOOP EACH_ROW_HCALC LC1=P4;
LOOP_BEGIN EACH_ROW_HCALC;
// handle a typical column
R5.L=R7.L+R7.H(NS);
B[P3]=R2||R1=R2>>0;
R2=in||(R7,R6) = BYTEOP16P (R1:0, R3:2);			//R7.L is useful
temp2=R5||(R6,R5) = BYTEOP16P (R1:0, R3:2);					//R5.L is useful
R7.H=W[P3]||A0.L=R5.L;
R6.L=(A0+=R7.L*R4.L),R6.H=(A1=R7.H*R4.H)(IU)||R3=in;
R7=temp2;
R7.H=R6.H+R6.L(NS)||B[P3]=R2;
scratch=R7||(R5,R7) = BYTEOP16P (R1:0, R3:2)(R);			//R7.L is useful
R0=R1>>0;
(R6,R5)= BYTEOP16P (R1:0, R3:2);
R6.H=W[P3]||A0.L=R7.L;
R7.L=(A0+=R6.L*R4.L),R7.H=(A1=R6.H*R4.H)(IU);
R0=R2>>0;
R2=R3>>0;

LOOP_END EACH_ROW_HCALC;
// handle last column
R5.L=R7.L+R7.H(NS);
B[P3]=R2||R1=R2>>0;
(R7,R6) = BYTEOP16P (R1:0, R3:2);			//R6.L and R7.L is useful
R7.H=W[P3]||A0.L=R6.L;
R6.L=(A0+=R7.L*R4.L),R6.H=(A1=R7.H*R4.H)(IU);
R5.H=R6.L+R6.H(NS);
scratch=R5;
P2=inRowEndGap;	// can be parallel
P1=P1+P2;		//jump the input pointer over row end gap
RTS;
_one_row_HFILTER.END:

// a sub routine for vertical filtering of 1 row of output
// we can process two pixels every time
_one_row_VFILTER:
//use I1 to place P2,P5,I2,I3
P3=M3;
I1+=M1;
P2=I1;
I1+=M1;
P5=I1;
I1+=M1;
I2=I1;
I1+=M1;
I3=I1;
I1+=M1;	//I1 is back home
R0=1;	// a useful constt;
R3.L=0xFF;
R3.H=0xFF;
R1=scr1||R5=scr5;
R5=R1+|+R5||R2=scr2;
LOOP EACH_ROW_VCALC LC1=P3;
LOOP_BEGIN EACH_ROW_VCALC;

A0=R5.L*R0.L,A1=R5.H*R0.L(FU)||R1=scr4;
R5=R1+|+R2||R1=scr3;
A0+=R5.L*R4.L,A1+=R5.H*R4.L(FU)||R5=scr5;	//4x
R6.L=(A0+=R1.L*R4.H),R6.H=(A1+=R1.H*R4.H)(IU)||R1=scr1;	//6x, 
R6=R6+|+R3;		
R6=R6>>8(V)||R2=scr2;
temp2=R6||R5=R1+|+R5;
A0=R5.L*R0.L,A1=R5.H*R0.L(FU)||R1=scr4;
R5=R1+|+R2||R1=scr3;
A0+=R5.L*R4.L,A1+=R5.H*R4.L(FU)||R5=scr5;
R6.L=(A0+=R1.L*R4.H),R6.H=(A1+=R1.H*R4.H)(IU)||R1=scr1;	//6x, 
R6=R6+|+R3||R7=temp2;	
R6=R6>>8(V)||R2=scr2;
R7=BYTEPACK(R7,R6);
out=R7||R5=R1+|+R5;
LOOP_END EACH_ROW_VCALC;
// scr1,scr2,scr5 has been over run, if we fix I1 thats sufficient
R1=[I1--];
R0=outRowEndGap;
R1=P0;
R0=R0+R1;	//skip the gap bytes at the end
P0=R0;
I1+=M1;		//skip one row becuase we are reducing

RTS;
_one_row_VFILTER.END:
#endif //__BFIN__
