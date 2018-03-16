#ifdef __BFIN__

#define temp1			[FP-32]



#define scr0			[I1++]
#define scr1			[P4++]
#define scr2			[P5++]
#define scr3			[I2++]
#define scr4			[I3++]
#define out             [I0++]


#define VERT_GAUSS(RESULT,P0,LOOP_NAME) R0 = OUTPUT;\
I0=R0;\
R4.L=4;\
R4.H=6;\
R0=1;\
R3.L=128;\
R3.H=128;\
R1=scr0||R5=scr4;\
R5=R1+|+R5||R2=scr1;\
LOOP LOOP_NAME LC1=P0;\
LOOP_BEGIN LOOP_NAME;\
A0=R5.L*R0.L,A1=R5.H*R0.L(FU)||R1=scr3;\
R5=R1+|+R2||R1=scr2;\
A0+=R5.L*R4.L,A1+=R5.H*R4.L(FU)||R5=scr4;\
R6.L=(A0+=R1.L*R4.H),R6.H=(A1+=R1.H*R4.H)(IU)||R1=scr0;\
R6=R6+|+R3;\
R6=R6>>8(V)||R2=scr1;\
temp1=R6||R5=R1+|+R5;\
A0=R5.L*R0.L,A1=R5.H*R0.L(FU)||R1=scr3;\
R5=R1+|+R2||R1=scr2;\
A0+=R5.L*R4.L,A1+=R5.H*R4.L(FU)||R5=scr4;\
R6.L=(A0+=R1.L*R4.H),R6.H=(A1+=R1.H*R4.H)(IU)||R1=scr0;\
R6=R6+|+R3||R7=temp1;\
R6=R6>>8(V)||R2=scr1;\
R7=BYTEPACK(R7,R6);\
out=R7||R5=R1+|+R5;\
LOOP_END LOOP_NAME;


#define COPYROW(DST,SRC,CNT,LOOP_NAME) R0 = DST;\
I1 = R0;\
R1 = SRC;\
I2 = R1;\
R0 = CNT;\
R2 = R0>>3;\
P5 = R2;\
R0 = [I2++];\
LOOP LOOP_NAME LC1 = P5;\
LOOP_BEGIN LOOP_NAME;\
R0=[I2++]||[I1++]=R0;\
LOOP_END LOOP_NAME;


#define TEMP4			[FP-56]

#define ROW4			[FP-52]
#define ROW3			[FP-48]
#define ROW2			[FP-44]
#define ROW1			[FP-40]
#define ROW0			[FP-36]

#define SCRATCH			[FP-28]
#define INPUTHEIGHT		[FP-24]
#define INPUTWIDTH		[FP-20]
#define OUTPUT			[FP-16]
#define INPUT			[FP-12]



#define INCREMENTINPUT() R0 = INPUT;\
R1 = INPUTWIDTH;\
R0 = R0+R1;\
INPUT=R0;

#define INCREMENTOUTPUT() R0 = OUTPUT;\
R1 = INPUTWIDTH;\
R1 = R1>>2;\
R0 = R0+R1;\
OUTPUT=R0;


#define SETUPVERT() R0 = ROW0;\
I1=R0;\
R0 = ROW1;\
P4=R0;\
R0 = ROW2;\
P5=R0;\
R0 = ROW3;\
I2=R0;\
R0 = ROW4;\
I3=R0;\
R0 = INPUTWIDTH;\
R2 = R0>>4;\
P0=R2;

#define scratch  		[I2++]
#define in		 		[I3++]

#define HORZ_GAUSS(INP,OUT,P2,LOOP_NAME) R0 = INP;\
I3 = R0;\
R0 = OUT;\
I2 = R0;\
R4.L=4;\
R4.H=6;\
P3=FP;\
P3+=-56;\
R0=in;\
R2=in||R5=R0<<8(V);\
R6=R2.B(Z);\
R5=R5>>8(V);\
A0=R6;\
R7.L=(A0+=R5.H*R4.L),R7.H=(A1=R5.L*R4.H)(IU);\
R7.L=R7.L<<1;\
LOOP LOOP_NAME LC1=P2;\
LOOP_BEGIN LOOP_NAME;\
R5.L=R7.L+R7.H(NS);\
B[P3]=R2||R1=R2>>0;\
R2=in||(R7,R6) = BYTEOP16P (R1:0, R3:2);\
temp1=R5||(R6,R5) = BYTEOP16P (R1:0, R3:2);\
R7.H=W[P3]||A0.L=R5.L;\
R6.L=(A0+=R7.L*R4.L),R6.H=(A1=R7.H*R4.H)(IU)||R3=in;\
R7=temp1;\
R7.H=R6.H+R6.L(NS)||B[P3]=R2;\
scratch=R7||(R5,R7) = BYTEOP16P (R1:0, R3:2)(R);\
R0=R1>>0;\
(R6,R5)= BYTEOP16P (R1:0, R3:2);\
R6.H=W[P3]||A0.L=R7.L;\
R7.L=(A0+=R6.L*R4.L),R7.H=(A1=R6.H*R4.H)(IU);\
R0=R2>>0;\
R2=R3>>0;\
LOOP_END LOOP_NAME;\
R5.L=R7.L+R7.H(NS);\
B[P3]=R2||R1=R2>>0;\
R2=PACK(R2.H,R2.H);\
(R7,R6) = BYTEOP16P (R1:0, R3:2);\
R7.H=W[P3]||A0.L=R6.L;\
R6.L=(A0+=R7.L*R4.L),R6.H=(A1=R7.H*R4.H)(IU);\
R5.H=R6.L+R6.H(NS);\
scratch=R5;



.text;
.align 4;
.global _compute_2592x972To648x486;
.type _compute_2592x972To648x486, STT_FUNC;
_compute_2592x972To648x486:

LINK 80;
[--SP] = (R7:4,P5:0);
I0 = R2;
INPUT = R0;
OUTPUT = R1;

R2 = [I0++];
INPUTWIDTH = R2;
R3 = [I0++];
INPUTHEIGHT = R3;
R4 = [I0++];
SCRATCH = R4;

R5 = R2>>1; //W/4 SHORTS


ROW0 = R4;
R4 = R4+ R5;
ROW1 = R4;
R4 = R4+ R5;
ROW2 = R4;
R4 = R4+ R5;
ROW3 = R4;
R4 = R4+ R5;
ROW4 = R4;
R4 = R4+ R5;


R0=0;
TEMP4=R0;

R3 = R3>>1; // HEIGHT LOOP
R3 += -2;
P1 = R3;


//W/8-1
R6 = R2>>3;
R6 +=-1;
P2 = R6;

R0 = 2;
M2 = R0;

I0 = 0;
HORZ_GAUSS(INPUT,ROW2,P2,FIRSTROW)
INCREMENTINPUT()
HORZ_GAUSS(INPUT,ROW3,P2,SECONDROW)
INCREMENTINPUT()
HORZ_GAUSS(INPUT,ROW4,P2,THIRDROW)
INCREMENTINPUT()

COPYROW(ROW0,ROW4,INPUTWIDTH,COPY0)
COPYROW(ROW1,ROW3,INPUTWIDTH,COPY1)


LOOP LOOP_NAME_VERT LC0 = P1;\
LOOP_BEGIN LOOP_NAME_VERT;

SETUPVERT()

VERT_GAUSS(OUTPUT,P0,VERT_KERNEL)
INCREMENTOUTPUT()

R1= ROW0;
R2= ROW1;
R3= ROW2;
R4= ROW3;
R5= ROW4;

ROW0 = R3;
ROW1 = R4;
ROW2 = R5;
ROW3 = R1;
ROW4 = R2;

HORZ_GAUSS(INPUT,ROW3,P2,ROWONE)
INCREMENTINPUT()
HORZ_GAUSS(INPUT,ROW4,P2,ROWTWO)
INCREMENTINPUT()

LOOP_END LOOP_NAME_VERT;


SETUPVERT()

VERT_GAUSS(OUTPUT,P0,VERT_PROLOG_SECONDLAST)
INCREMENTOUTPUT()

R1= ROW0;
R2= ROW1;
R3= ROW2;
R4= ROW3;
R5= ROW4;

ROW0 = R3;
ROW1 = R4;
ROW2 = R5;
ROW3 = R1;
ROW4 = R2;

HORZ_GAUSS(INPUT,ROW3,P2,HORZ_PROLOG_SECONDLAST)
INCREMENTINPUT()

R1= ROW0;
R2= ROW1;
R3= ROW2;
R4= ROW3;

ROW0= R1;
ROW1= R2;
ROW2= R3;
ROW3= R4;
ROW4= R3;

SETUPVERT()
VERT_GAUSS(OUTPUT,P0,VERT_PROLOG)

(R7:4,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_2592x972To648x486.END:


#endif //__BFIN__
