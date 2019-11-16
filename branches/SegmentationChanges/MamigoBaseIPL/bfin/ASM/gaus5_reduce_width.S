#ifdef __BFIN__
/*****************************************************************************
 * GausPyr.asm
 *****************************************************************************/

.text;

.align 4;

.global _gaus5_reduce_width_reflect;
.type _gaus5_reduce_width_reflect, STT_FUNC;
/*
R0 -> address of input image
R1 -> address of out which has at least W*H/2 bytes allocated
R2 -> (2 shorts packed W,H)

IMPORTANT: M0:3 has no relevance to I0:I3
*/

_gaus5_reduce_width_reflect:

#define temp1 [FP -12]

LINK 40;
[ -- SP] = (R7:4, P5:3);
I1=R0;		// input image
P5=R1; 		//P5 is now output writer
R3=R2>>17;	//W/2
R3+=-1;		//W/2-1
R2.H=0; 	
P4=R2;		//H
P3=R3;		//W/2
I0=0;		
L1=0;
R4=6;
R4.H=8;
R7=7;
A0=0;  
A1=0;
M2=2;	// just a useful constant

LOOP HEIGHT_LOOP LC0=P4;
LOOP_BEGIN HEIGHT_LOOP;

R1=[I1++ M2];
(R6,R5)=BYTEUNPACK R1:0 (R);
R6=PACK(R5.H,R6.L); //<----- vertical border ext by reflection

LOOP WIDTH_LOOP LC1=P3;
LOOP_BEGIN WIDTH_LOOP;

A0.L=R6.L||R2.L=W[I1++]; // done
R0=(A0+=R4.L*R5.L) (IS);
R6=(R6+R5)<<2;	  		// NO Parallel
temp1=R5||R0.L=R0.L+R6.H(NS);		 //(b)
(R2,R5)=BYTEUNPACK R3:2;
R0=R0+R5;				 			// (a)
R0.L=R0.L+R4.H(NS);					// add 8
R0.L=R0.L>>4||R6=temp1;				// devide by 16
B[P5++]=R0;			 	// (a)
LOOP_END WIDTH_LOOP;

// last calc
A0.L=R6.L;
R0=(A0+=R7.L*R5.L) (IS);	//<-----V. BORDER exten
R6=(R6+R5)<<2;	  					// NO Parallel
R0.L=R0.L+R6.H(NS);		 //(b)
R0.L=R0.L+R4.H(NS);					// add 8
R0.L=R0.L>>4;						// devide by 16
B[P5++]=R0;			 	
LOOP_END HEIGHT_LOOP;
I1=0;
(R7:4, P5:3) = [SP ++];
UNLINK;
RTS;
_gaus5_reduce_width_reflect.END:

#endif //__BFIN__
