#ifdef __BFIN__

/*****************************************************************************
 * GausPyr.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_expand_reflect_2s;
.type _gaus5_expand_reflect_2s, STT_FUNC;
/*
R0 -> address of input (2 short images of size W, H interlaced)
R1 -> address of the output image
R2 -> address of args structure int args[]={W,H,Wstep_in,Wstep_out,scr,scaleShift};
assumtions:
1. R1 has at least 12*W integers (48*W bytes)
3. R3 (output image has at least 4*W*H shorts allocated

IMPORTANT: M0:3 has no relevance to I0:I3
*/

_gaus5_expand_reflect_2s:

#define width	     	[FP -12]
#define height	     	[FP -16]
#define inRowEndGap	 	[FP -20]
#define wStepOut   		[FP -24]
#define temp1     		[FP -28]
#define shift_A     	[FP -32]
#define shift_B         [FP -36]

#define scratch  		[I0++]
#define in		 		[P1++]
#define out1		   	[P0++]
#define out2		   	[P5++]

#define scr1			[I1++]
#define scr2			[I2++]
#define scr3			[I3++]


LINK 40;
[ -- SP] = (R7:4, P5:3);
P5=R2; 				// args structure having width etc.
R5=3;				//constt
P0=R1;				//output
P1=R0;				//input
R0=[P5++];
width	=R0;
R1=R0<<4;			// w in bytes wb
M1=R1;				//wb
R5*=R1;				//3wb;
L0=R5;				//scratch rotates after 3wb
R1=R1<<1;
M2=R1;				//2wb
R7=R0;
R7+=-1;
P4=R7;				// to intialize the inner loop to W-1
R1=[P5++];
height	=R1;
R2=[P5++]||R0=R0<<2;
R2=R2-R0;
R2+=-4;
inRowEndGap	=R2;
P2=R2;
R3=[P5++];
wStepOut=R3;		//bytes that we need to skip to move between out rows
R4=[P5++]; 			//scratch
I0=R4;
B0=R4;
R6=6;
R0=[P5++];			//scaleshift
R1=10;
R1=R1+R0;
shift_A=R1||R1=R6<<1;	//R1=12;
R1=R1+R0;
shift_B=R1;			//shift by this amount during vfilter

I0+=M1; //initially place I0 at beginning of 2nd scratch row


// perform top 2 rows h-calc
CALL _one_row_HFILTER;
CALL _one_row_HFILTER;

//Replicate 1 row from scratch
P5=width;
I0=B0;			// lets keep I0 for writing to scratch

I1=I0;			//I1:scr_row_1
B1=B0;
L1=L0;

I2=I1;
I2+=M1; 		//I2:scr_row_2
B2=B0;
L2=L0;

I3=I2;
I3+=M1;			//I3:scr_row_3
B3=B0;
L3=L0;

P5=P5<<2;
P3=P5;
P5+=-1;			//w-1

R3=[I3++];
LOOP EXTND_TOP_2ROWS LC1=P5;
LOOP_BEGIN EXTND_TOP_2ROWS;
[I1++]=R3||R3=[I3++];
LOOP_END EXTND_TOP_2ROWS;
[I1++]=R3;
I1=I0;

//setup LC0 for main loop
R3=height;
R3+=-2;			//H-2 (all except first and last)
LC0=R3;
CALL _one_row_VFILTER;
//Main Loop

LOOP MAIN_HEIGHT_LOOP LC0;
LOOP_BEGIN  MAIN_HEIGHT_LOOP;

CALL _one_row_HFILTER;

CALL _one_row_VFILTER;

LOOP_END MAIN_HEIGHT_LOOP;

//last row calc
//bottom replicate 1 row: write at I0 read from P4=I0+M1
P3+=-1;
I0+=M1;
P4=I0;
I0+=M2;	// bring I0 back where it started

R2=[P4++];
LOOP EXTND_BOTTOM_1ROW LC1=P3;
LOOP_BEGIN EXTND_BOTTOM_1ROW;
[I0++]=R2||R2=[P4++];
LOOP_END EXTND_BOTTOM_1ROW;
[I0++]=R2;
P3+=1;

CALL _one_row_VFILTER;

L0=0;
L1=0;
L2=0;
L3=0;

(R7:4, P5:3) = [SP ++];
UNLINK;
RTS;
_gaus5_expand_reflect_2s.END:

/***** Sub routines below this ****/

// a sub routine for 1 row horizontal filter

_one_row_HFILTER:
R6.H=6;
R4.L=4;
R4.H=4;
// handle first column
R5=in||R1=R4>>2;
A0=R5.L*R6.L,A1=R5.H*R6.H(IS)||R7=in;
A0+=R7.L*R1.L,A1+=R7.H*R1.H(IS);
R0=R7;
LOOP EACH_ROW_HCALC LC1=P4;
LOOP_BEGIN EACH_ROW_HCALC;
//typical column
R2=(A0+=R0.L*R1.L),R3=(A1+=R0.H*R1.H)(IS)||temp1=R5;
scratch=R2||A0=R0.L*R4.L,A1=R0.H*R4.H(IS);
scratch=R3||R2=(A0+=R5.L*R4.L),R3=(A1+=R5.H*R4.H)(IS)||R7=temp1;
R5=R0>>0||R0=in;
//help with next iteration
scratch=R2||A0=R7.L*R1.L,A1=R7.H*R1.H(IS);
scratch=R3||A0+=R5.L*R6.L,A1+=R5.H*R6.H(IS);
LOOP_END EACH_ROW_HCALC;
// handle last column
R2=(A0+=R7.L*R1.L),R3=(A1+=R7.H*R1.H)(IS)||P2=inRowEndGap;
scratch=R2||A0=R5.L*R4.L,A1=R5.H*R4.H(IS);
scratch=R3||R2=(A0+=R7.L*R4.L),R3=(A1+=R7.H*R4.H)(IS);
scratch=R2;
scratch=R3;
P1=P1+P2;		//jump the input pointer over row end gap
RTS;
_one_row_HFILTER.END:

// a sub routine for vertical filtering of 1 row of output
_one_row_VFILTER:
// assumes I1 is at the correct spot
P2=wStepOut||R6.H=R6.L>>3;  //R6.H=0 => R6=6 now.
I2=I1;
I2+=M1;
I3=I2;
I3+=M1;
P5=P0+P2;		// second out put writer
P2=P5+P2;		// remember it as the new position for P0
R4=shift_A;
// head start for the first iteration
R1=scr1||R3=scr3;
LOOP EACH_ROW_VCALC LC1=P3>>1;
LOOP_BEGIN EACH_ROW_VCALC;
R7=R1+R3(NS)||R2=scr2;
R5=R2+R3(NS)||R1=scr1;
R2*=R6;
R7=R2+R7(NS)||R2=scr2;
R7=ASHIFT R7 BY R4.L||R3=scr3;
R0.L=R7(RND)||R4=shift_B;
R5=ASHIFT R5 BY R4.L||temp1=R0;
R0.L=R5(RND);
R5=R2+R3(NS);
R5=ASHIFT R5 BY R4.L;
R0.H=R5(RND)||R4=shift_A;
out2=R0||R7=R1+R3(NS);
R2*=R6;
R7=R2+R7(NS)||R0=temp1;
R7=ASHIFT R7 BY R4.L;
R0.H=R7(RND)||R1=scr1;
out1=R0||R3=scr3;
LOOP_END EACH_ROW_VCALC;
P0=P2;
R0=[I1--];	// I1 over read
RTS;
_one_row_VFILTER.END:
#endif //__BFIN__
