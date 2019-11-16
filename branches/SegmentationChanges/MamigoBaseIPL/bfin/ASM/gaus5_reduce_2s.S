#ifdef __BFIN__

/*****************************************************************************
 * GausPyr.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_reduce_reflect_2s;
.type _gaus5_reduce_reflect_2s, STT_FUNC;
/*
R0 -> address of input (2 short images of size W, H interlaced)
R1 -> address of the output image
R2 -> address of args structure int args[]={W,H,Wstep_in,Wstep_out,scr};
assumtions:
1. R1 has at least 5*W integers (4*5*W bytes)
3. R3 (output image has at least W*H bytes allocated

IMPORTANT: M0:3 has no relevance to I0:I3
*/

_gaus5_reduce_reflect_2s:

#define width	     	[FP -12]
#define height	     	[FP -16]
#define inRowEndGap	 	[FP -20]
#define outRowEndGap 	[FP -24]
#define temp1     		[FP -28]
#define temp2     		[FP -32]

#define scratch  		[I0++]
#define in		 		[P1++]
#define out		   	   W[P0++]

#define scr1			[I1++]
#define scr2			[P2++]
#define scr3			[P5++]
#define scr4			[I2++]
#define scr5			[I3++]

LINK 40;
[ -- SP] = (R7:4, P5:3);
A0=0;
A1=0;
P5=R2; // args structure having width etc.
R5=3;
LC0=R5;
R5=5;			//constt
P0=R1;			//output
P1=R0;			//input
R0=[P5++];
width	=R0;
R1=R0<<2;		// w in bytes wb
M1=R1;			//wb
R5*=R1;			//5wb;
L0=R5;			//scratch rotates after 5wb
R1=R1<<1;
M2=R1;			//2wb
R7=R0>>1;
R7+=-2;
P4=R7;			// to intialize the inner loop to W/2 - 2
R1=[P5++];
height	=R1;
R2=[P5++]||R3=R0<<2;
R2=R2-R3;
inRowEndGap	=R2;
P2=R2;
R3=[P5++]||R4=R0<<1;
R3=R3-R4;
outRowEndGap=R3;	//bytes that we need to skip in out row end
R4=[P5++]; 		//scratch
I0=R4;
B0=R4;
R6.L=6;
R6.H=6;
R4.L=4;
R4.H=4;
I0+=M2; //initially place I0 at beginning of 3rd scratch row

LOOP FIRST_3_ROW_HCALC LC0;
LOOP_BEGIN FIRST_3_ROW_HCALC;
CALL _one_row_HFILTER;
LOOP_END FIRST_3_ROW_HCALC;

//Replicate 2 Rows from scratch
P5=width;
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

P3=width;
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
R3=R3>>1;
R3+=-2;			//H/2-1
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
P1=width;
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
_gaus5_reduce_reflect_2s.END:

/***** Sub routines below this ****/

// a sub routine for 1 row horizontal filter

_one_row_HFILTER:
// handle first column
R7=in;
R1=R4>>1||R5=in;
R2=R5.L*R4.L,R3=R5.H*R4.H(IS)||R5=in;
temp1=R2||A0=R7.L*R6.L,A1=R7.H*R6.H(IS);
temp2=R3||R0=(A0+=R5.L*R1.L),R1=(A1+=R5.H*R1.H)(IS);
R2=R2<<1;
R2=R0+R2;
scratch=R2||R3=R3<<1;
R3=R1+R3;
R2=temp1||scratch=R3||R1=R4>>2;	//R1=1,1 constt
R3=temp2||A0=R2;

LOOP EACH_ROW_HCALC LC1=P4;
LOOP_BEGIN EACH_ROW_HCALC;
// handle a typical column
//R7 now has p2_; temp1,2 have 4*p1_; R5 has p0
A1=R3||R2=in;
R2=R2.L*R4.L,R3=R2.H*R4.H(IS);
A0+=R7.L*R1.L,A1+=R7.H*R1.H(IS);
A0+=R5.L*R6.L,A1+=R5.H*R6.H(IS);
R7=R5>>0||R5=in;
R0=(A0+=R5.L*R1.L),R1=(A1+=R5.H*R1.H)(IS);
R0=R0+R2;
R1=R1+R3;
scratch=R0||A0=R2;
scratch=R1||R1=R4>>2;

LOOP_END EACH_ROW_HCALC;
// handle last column
A1=R3||R2=in;
A0+=R7.L*R1.L,A1+=R7.H*R1.H(IS)||P2=inRowEndGap;
A0+=R5.L*R6.L,A1+=R5.H*R6.H(IS);
A0+=R2.L*R4.L,A1+=R2.H*R4.H(IS);
R2=(A0+=R5.L*R1.L),R3=(A1+=R5.H*R1.H)(IS);
scratch=R2;
scratch=R3;
P1=P1+P2;		//jump the input pointer over row end gap
RTS;
_one_row_HFILTER.END:

// a sub routine for vertical filtering of 1 row of output
_one_row_VFILTER:
//use I1 to place P2,P5,I2,I3
I1+=M1;
P2=I1;
I1+=M1;
P5=I1;
I1+=M1;
I2=I1;
I1+=M1;
I3=I1;
I1+=M1;	//I1 is back home
// head start for the first iteration
R3=scr3||R1=scr4;
R5=R1+R3(NS)||R2=scr2;
LOOP EACH_ROW_VCALC LC1=P3;
LOOP_BEGIN EACH_ROW_VCALC;

R3=R3<<1||R7=scr1;
R5=(R5+R2)<<2;			//R0:4*p1_+4*p0+4*p1
R0=R5+R3(NS)||R2=scr5;  //R0:4*p1_+6*p0+4*p1
R0=R0+R7(NS)||R3=scr3;	//R0:p2_+4*p1_+6*p0+4*p1
R0=R0+R2(NS)||R1=scr4;	//R0:p2_+4*p1_+6*p0+4*p1+p2
R0=R0<<8||R2=scr2;
R0.L=R0(RND);
out=R0||R5=R1+R3(NS);

LOOP_END EACH_ROW_VCALC;
R0=outRowEndGap;
R1=P0;
R0=R0+R1;	//skip the gap bytes at the end
P0=R0;
I1+=M1;		//skip one row becuase we are reducing

RTS;
_one_row_VFILTER.END:

#endif //__BFIN__
