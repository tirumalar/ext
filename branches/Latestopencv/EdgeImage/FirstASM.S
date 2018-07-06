#ifdef __ARMX__
.text

.global ExtractEvenBytes;
ExtractEvenBytes:

      # r0: Ptr to source data
      # r1: Ptr to destination data
      # r2: Ptr to destination data
      # r3: Iteration count:

      push   	  {lr}
      lsr         r3, r3, #3
  .loop:
      # load 8 pixels:
      vld4.8      {d0-d3}, [r0]!
      vst1.8      {d0}, [r1]!
      vst1.8      {d2}, [r2]!

      subs        r3, r3, #1
      bne         .loop

      pop         {pc}


.global GetExtractedLineASM;
GetExtractedLineASM:

      # r0: Ptr to source data
      # r1: Ptr to destination data
      # r2: Ptr to destination data
      # r3: Iteration count:

      push   	  {lr}
      lsr         r3, r3, #2
  .loop2:
      # load 8 pixels:
      vld4.16     {d0-d3}, [r0]!
      vst1.16      {d0}, [r1]!
      vst1.16      {d2}, [r2]!

      subs        r3, r3, #1
      bne         .loop2

      pop         {pc}




.global convert_asm_neon;
convert_asm_neon:
      # r0: Ptr to input data
      # r1: Ptr to out data
      # r2: Iteration count:
      push   	  {r4-r6,lr}

      # build the three constants:
      mov         r3, #1
      vdup.8      d0, r3
      mov         r3, #4
      vdup.8      d1, r3
      mov         r3, #6
      vdup.8      d2, r3

#		minus4 = (unsigned char *)Gauss4xBuffer;
#		minus2 = minus4 + 2*outputwidth;
#		zero = minus4 +1;
#		plus2 = minus2 + 1;
#		plus4 = minus4 + 2;

	  # minus4 in r0
	  # minus2 in r3
  	  add        r3, r0, r2
  	  add        r3, r3, r2
  	  # zero in r4
	  add        r4, r0, #1
	  # plus4 in r5
	  add        r5, r0, #2
	  # plus2 in r6
	  add        r6, r3, #1

	  lsr        r2, r2, #3
  .loop1:

#for(int i=0;i<outputwidth;i+=8,oxp+=8){
#	res = vmull_u8 (vld1_u8(minus4+=8), one8);
#	res = vmlal_u8 (res,vld1_u8(plus4+=8), one8);
#	res = vmlal_u8 (res,vld1_u8(minus2+=8), four8);
#	res = vmlal_u8 (res,vld1_u8(plus2+=8), four8);
#	res = vmlal_u8 (res,vld1_u8(zero+=8), six8);
#	vst1q_u16(oxp,res);
#}

      vld1.8      d3, [r0]!
      vmull.u8    q2, d3, d0

      vld1.8      d3, [r5]!
      vmlal.u8    q2, d3, d0

      vld1.8      d3, [r3]!
      vmlal.u8    q2, d3, d1

      vld1.8      d3, [r6]!
      vmlal.u8    q2, d3, d1

      vld1.8      d3, [r4]!
      vmlal.u8    q2, d3, d2

      # store:
      vst1.16      {d4,d5}, [r1]!

      subs        r2, r2, #1
      bne         .loop1

      pop         { r4-r6, pc }


#endif
