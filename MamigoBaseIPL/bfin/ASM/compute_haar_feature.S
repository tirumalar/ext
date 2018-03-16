#ifdef __BFIN__

// This ASSUMES that the output is within 16 bits

.text;

.align 4;

.global _compute_haar_feature;
.type _compute_haar_feature, STT_FUNC;

_compute_haar_feature:


LINK 0;
[ -- SP] = (R7:0,P5:0);

L0 = 0;
L1 = 0;
L2 = 0;
L3 = 0;

P0 = R0;
P1 = R1;

/*
	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = extra_output_width_step;	// extra widthstep
	dstParam[3] = weight;
	dstParam[4] = p[0];
	dstParam[5] = p[1];
	dstParam[6] = p[2];
	dstParam[7] = p[3];
	dstParam[8] = extra_input_width_step;
*/	
	
P4 = [P0++];	// width of the image
P4 += -1;		// one less due to  pipeliing

R0 = [P0++];	// height
LC0 = R0;

P3 = [P0++];	// extra output widthstep;
P3 += 4;	

R7 = [P0++];	// weight -

R0 = [P0++];	I0 = R0;
R1 = [P0++];	I1 = R1;
R2 = [P0++];	I2 = R2;
R3 = [P0++];	I3 = R3;

P2 = [P0++];	// extra input width step;
M0 = 8;
M1 = P2;


LOOP OUTER_HEIGHT_LOOP LC0;
LOOP_BEGIN OUTER_HEIGHT_LOOP;

R0 = [I0++M0] || R3 = [I3++M0];
R2 = [I2++M0] || R1 = [I1++M0];

R5 = R3 + R0 (NS) || R6 = [P1]; 
R5 = R5 - R1 (NS) || R0=[I0++M0] || R3 = [I3++M0];
R5 = R5 - R2 (NS) || R1=[I1++M0] || R2 = [I2++M0];
R5 *= R7;
R5 = R5 + R6 (S);

LOOP INNER_WIDTH_LOOP LC1 = P4;
LOOP_BEGIN INNER_WIDTH_LOOP;

// 0 | 1
// 2 | 3  // 4 being estimated

R5 = R3 + R0 (NS) || [P1++] = R5; 
R5 = R5 - R1 (NS) || R0=[I0++M0] || R3 = [I3++M0];
R5 = R5 - R2 (NS) || R1=[I1++M0] || R2 = [I2++M0];
R5 *= R7;
R5 = R5 + R6 (S);


LOOP_END INNER_WIDTH_LOOP;

[P1++P3] = R5;

I0 -= M0;
I1 -= M0;
I2 -= M0;
I3 -= M0;

I0 += M1;
I1 += M1;
I2 += M1;
I3 += M1;

LOOP_END OUTER_HEIGHT_LOOP;


(R7:0,P5:0) = [SP ++];
UNLINK;
RTS;

_compute_haar_feature.END:


#endif //__BFIN__
