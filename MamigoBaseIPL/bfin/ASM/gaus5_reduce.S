#ifdef __BFIN__
/*****************************************************************************
 * GausPyr.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_reduce_reflect;
.type _gaus5_reduce_reflect, STT_FUNC;
/*
R0 -> address of input image
R1 -> address of the scratch image
R2 -> address of the dimenstion strcuture (2 bytes packed W,H) followed by output
assumtions:
1. the filter is 5x5.
2. R1 has at least 5*W bytes
3. R3 (output image has at least W*H/4 bytes allocated

IMPORTANT: M0:3 has no relevance to I0:I3
*/

_gaus5_reduce_reflect:

#define temp1 [FP -12]
#define temp2 [FP -16]
#define width [FP -20]

LINK 40;
[ -- SP] = (R7:0, P5:0);
P4=R2;
I3=R1;  	/* scratch pixel circular buffer*/
I2=R1;		/* scrtach row circular buffer*/
I0=0;
R4.L=0x5;
R4.H=0x4;
I1=R0;		/* input image*/

A0=0||P5=[P4++];  /*P5 is now output writer */
A1=0||R2=[P4];	/*R2.L has Width, R2.H has Height*/

R3=R2.L(Z);	// R3.L is v (same as w)

M2=2;	// just a useful constant
width=R3;
R5=0;
R5.L = R3.L*R4.L (IU);	/*R5.L has 5v */

R2=R2>>16;
M3=R2;	  	/* M3 has h, we are not using it as a increment for M3*/

L3=R5;		/* back to starting point after 5 jumps*/
L2=R5;		/* back to starting point after 5 jumps*/

M1=R3;		/* jumps a whole row */
B3=I3; 		/* scratch pixel circular buffer is initialized */
R4=0x3;
LC0=R4;
R3=R3>>1;	  /* R3=v/2 we need v/2 at many places hence this*/
R5=R3>>1;
R5+= -1;
M0=R5;		  // M0=v/4-1

R7=R3;
R7+= -1;
P0=R7;

B2=I2; 		  /* scrtach row circular buffer is ready*/

R4=0x6;
R4.H=128;

/* Move I3 to middle scratch row */
I3+=M1;
I3+=M1;


LOOP FIRST_3_ROW_HCALC LC0;
LOOP_BEGIN FIRST_3_ROW_HCALC;

R1=[I1++ M2];
(R6,R5)=BYTEUNPACK R1:0 (R);
R6=PACK(R5.H,R6.L); //<----- vertical border ext by reflection

LOOP EACH_ROW_HCALC LC1=P0;
LOOP_BEGIN EACH_ROW_HCALC;

A0.L=R6.L||R2.L=W[I1++]; // done
R0=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
temp1=R5||R0.L=R0.L+R6.H(NS);		 //(b)
(R2,R5)=BYTEUNPACK R3:2;
R0=R0+R5;				 			// (a)
R6=temp1||W[I3++]=R0.L;			 	// (a)
LOOP_END EACH_ROW_HCALC;

// last calc
A0.L=R6.L;
R0=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
R0.L=R0.L+R6.H(NS);		//(b)
R0=R0+R5; 				// <----- vertical border ext by reflection
W[I3++]=R0.L;			// (a)

LOOP_END FIRST_3_ROW_HCALC;

/* outer loop runs (H-3)/2 times it comes much later*/
R5=M3; /* R5 has height*/
R5+= -0x3; /* R5 now has H-3*/
R5=R5>>1;  /* R5 now has (H-3)/2*/
LC0=R5;


/* extend top border */
R5=M0; /* v/4-1 */
P0=R5;

P1=I3; 	/* P1 first writer */

I2=I3;
I2+=M1;
P2=I2; /* P2 is second writer */

I3-=M1; /* I3 is first reader */

I2=I3;
I2-=M1; /* I2 is second reader */


R6=[I3++];
LOOP EXTND_TOP_2ROWS LC1=P0;
LOOP_BEGIN EXTND_TOP_2ROWS;
[P1++]=R6||R7=[I2++];
[P2++]=R7||R6=[I3++];
LOOP_END EXTND_TOP_2ROWS;
[P1++]=R6||R7=[I2++];
[P2++]=R7;

/* done extend top border  I3 is back to the top of scratch*/


P0=R3; /* for the next loop */
/* read words with I2,P1,P2,P3,P4*/
I2=I3;
I2+=M1;
P1=I2;
I2+=M1;
P2=I2;
I2+=M1;
P3=I2;
I2+=M1;
P4=I2;
I2+=M1; /* bring I2 home*/

/* perform one row G calc here */
R5=0; /*FLUSH to make sure we have no left over*/
LOOP EACH_ROW_VCALC LC1=P0;
LOOP_BEGIN EACH_ROW_VCALC;

R5.L=W[I2++]; 			/*b0 */
R6=W[P1++](Z); 			/*b1 */
R7=W[P3++](Z); 			/*b3 */
R7=(R7+R6)<<2;			/*4(b1+b3)*/
R6=W[P4++](Z)||R7=R7+R5(NS);	/*b4*/		/*b0+4b1  +4b3*/
R7=R7+R6(NS);			/*b0+4b1  +4b3+b4*/
A0=R7||R5=W[P2++](Z); 			/*b2 */
R6=(A0+= R5.L*R4.L) (IS); 	/*R4.L should be 6*/
R6.L=R6.L+R4.H (NS);		/* add 128 for rounding  R4.H should be 128*/
R6=R6>>8;					/* get back to an unsigned byte */
B[P5++]=R6;

LOOP_END EACH_ROW_VCALC;
NOP;

R3 += -1; 	//v/2-1

/* Ready for the larger loop now */
P0=R3;    	//inner loop for v/2-1
LOOP H_3ROWSLOOP LC0;
LOOP_BEGIN H_3ROWSLOOP;

/* we will write two rows of H calc togather:
a) read from I1, write to I3
b) read from P1, write to I2
*/
R0=I1;
R1=width;
R0=R0+R1;
P1=R0;

R1=[I1++ M2];
(R6,R5)=BYTEUNPACK R1:0 (R);
R6=PACK(R5.H,R6.L);			// <----- vertical border ext by reflection
I2=I3;
I2+=M1;
I2+=M1;
P4=I2;
I2+=M1;
P3=I2;
I2+=M1;
P2=I2;
I2+=M1; /* back to I3 */
I2+=M1;

R1=[P1];
(R7,R1)=BYTEUNPACK R1:0 (R);
R7=PACK(R1.H,R7.L);  // <----- vertical border ext by reflection
P1+=2;

LOOP COLS_LOOP LC1=P0;
LOOP_BEGIN COLS_LOOP;
/* H Calc 1 */
A0.L=R6.L||R2.L=W[I1++]; 	// done
R0.L=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  				// NO Parallel
temp1=R5||R0.L=R0.L+R6.H(NS);	// temp1 for next loop
(R2,R5)=BYTEUNPACK R3:2;		// R5 will be used in next iteration
R0.L=R0.L+R5.L (NS);				 	// (a)
//remember this in R0
/* H Cal2 */
W[I3++]=R0.L||A1.L=R7.L||R2=W[P1++](Z);
R0.H=(A1+=R4.L*R1.L) (IS);
R7=(R7+R1)<<2;	  			// NO Parallel
temp2=R1||R0.H=R0.H+R7.H(NS);		 	//(b)
(R2,R1)=BYTEUNPACK R3:2;	// R1 will be used in next iteration
R0.H=R0.H+R1.L (NS)||R6=W[P4++](Z);	// (a)||b0 for next
// remeber this value in R0 for future
/* V Calc */
/* use P4,P3,P2,R0 to perform V Calc and write the result to P5*/
W[I2++]=R0.H||R2=W[P3++](Z);			//b1
R2=PACK(R6.L,R2.L);
R0=R0+|+R2;					//b0+b4 b1+b3
R0.L=R0.L<<2;					//4(b1+b3)
R0.L=R0.L+R0.H (NS);
A0.L=R0.L||R2=W[P2++](Z);
R7=temp2||R0=(A0+=R4.L*R2.L)(IS);
R6=temp1||R0.L=R0.L+R4.H(NS);		/* add 128 for rounding  R4.H should be 128*/
R0=R0>>8;				/* get back to an unsigned byte */
B[P5++]=R0;
LOOP_END COLS_LOOP;

// Following is due to v border extension
// last H1 calc
A0.L=R6.L;
R0.L=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
R0.L=R0.L+R6.H(NS);		//(b)
R0.L=R0.L+R5.L(NS); 				// <----- vertical border ext by reflection
// last H2 calc
A1.L=R7.L||W[I3++]=R0.L;			// (a)
R0.H=(A1+=R4.L*R1.L) (IS);
R7=(R7+R1)<<2;	  			// NO Parallel
R0.H=R0.H+R7.H(NS);		 	//(b)
R0.H=R0.H+R1.L(NS)||R6=W[P4++];	//b0 for the next
//^----- vertical border ext by reflection
/* last V Calc */
W[I2++]=R0.H||R2=W[P3++](Z);	// (a)		//b1
R2=PACK(R6.L,R2.L);
R0=R0+|+R2;					//b0+b4 b1+b3
R0.L=R0.L<<2;					//4(b1+b3)
R0.L=R0.L+R0.H (NS);
A0.L=R0.L||R2=W[P2++](Z);
R0=(A0+=R4.L*R2.L)(IS);
R0.L=R0.L+R4.H(NS);		/* add 128 for rounding  R4.H should be 128*/
R0=R0>>8;				/* get back to an unsigned byte */
B[P5++]=R0;
I3+=M1;
I1=P1;
LOOP_END H_3ROWSLOOP;
/* Main loop is done */



/* end game start*/
P0=R3;    	//inner loop for v/2-1
R5=M0; 		/* v/4 -1*/
LC0=R5; 	/* for a future loop */

/* H is even => we have one row left for HCalc */

R1=[I1++ M2];
(R6,R5)=BYTEUNPACK R1:0 (R);
R6=PACK(R5.H,R6.L);   // <----- vertical border ext by reflection

LOOP BTM_ROW_LOOP LC1=P0;
LOOP_BEGIN BTM_ROW_LOOP;

A0.L=R6.L||R2.L=W[I1++]; // done
R0=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
R0.L=R0.L+R6.H(NS);		 //(b)
R6=R5;					 // for next loop
(R2,R5)=BYTEUNPACK R3:2;
R0=R0+R5;				 // (a)
W[I3++]=R0.L;			 // remeber this value in R0 for future
LOOP_END BTM_ROW_LOOP;
// last H1 calc
A0.L=R6.L;
R0.L=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
R0.L=R0.L+R6.H(NS);		//(b)
R0.L=R0.L+R5.L (NS); 				// <----- vertical border ext by reflection
W[I3++]=R0.L;			// (a)

/*Bottom row extension, for even H we need only one row*/
/* I3 is pointing to the writing location */
/* make I2 point to the reading location*/

I2=I3;

/* move back two rows for refelct, optionally move fron two rows for replicate*/
I2-=M1;
I2-=M1;

R6=[I2++];
LOOP EXTND_BTM_1ROW LC0;
LOOP_BEGIN EXTND_BTM_1ROW;
R6=[I2++]||[I3++]=R6;
LOOP_END EXTND_BTM_1ROW;
[I3++]=R6;

R3+=1;	//R3 back to v/2
P0=R3;
/* now I3 points to reading location for next VCalc*/
I2=I3;
I2+=M1;
P1=I2;
I2+=M1;
P2=I2;
I2+=M1;
P3=I2;
I2+=M1;
P4=I2;
I2+=M1;

R5=0; /* flush */
LOOP BTM_1ROW_VCALC LC1=P0;
LOOP_BEGIN BTM_1ROW_VCALC;

R5.L=W[I2++]; 			/*b0 */
R6=W[P1++](Z); 			/*b1 */
R7=W[P3++](Z); 			/*b3 */
R7=(R7+R6)<<2;			/*4(b1+b3)*/
R7=R7+R5(NS)||R6=W[P4++](Z);  		/*b4*/ /*b0+4b1  +4b3*/
R7=R7+R6(NS);			/*b0+4b1  +4b3+b4*/
A0=R7||R5=W[P2++](Z); 			/*b2 */
R6=(A0+= R5.L*R4.L) (IS); 	/*R4.L should be 6*/
R6.L=R6.L+R4.H (NS);		/* add 128 for rounding  R4.H should be 128*/
R6=R6>>8;					/* get back to an unsigned byte */
B[P5++]=R6;

LOOP_END BTM_1ROW_VCALC;

L3=0;
L2=0;

(R7:0, P5:0) = [SP ++];
UNLINK;
RTS;
_gaus5_reduce_reflect.END:

#endif //__BFIN__
