#ifdef __BFIN__
.text;

.align 4;

.global _getfeatureshift;
.type _getfeatureshift, STT_FUNC;

_getfeatureshift:


LINK 0;
[ -- SP] = (R7:1,P5:0);

R3 = 0;
R2 = 0;
R0 *= R0;
R3.L = SIGNBITS R0;
CC = R3 < 7 (IU);
IF !CC JUMP FINISHED_PROCESSING;
R1.L = 0x8B0F;
R1.H = 0x0194; 	// R1 = 0x7fffffff/81
CC = R0 < R1;
IF CC JUMP FINISHED_PROCESSING;
R0 = 7 (Z);
R2 = R0 - R3;
FINISHED_PROCESSING:
R0 = R2;

(R7:1,P5:0) = [SP ++];
UNLINK;
RTS;

_getfeatureshift.END:

#endif //__BFIN__
