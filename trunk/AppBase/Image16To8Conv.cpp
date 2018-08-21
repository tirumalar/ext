/*
 * Image16To8Conv.cpp
 *
 *  Created on: Nov 9, 2012
 *      Author: mamigo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <iostream>
#include <exception>
#include "socket.h"

#ifdef __aarch64__
#define __ARM__
#endif

#ifdef __ARM__
#include <arm_neon.h>
//static unsigned int ConvertU16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const unsigned short dc)
//{
//	unsigned char *p8 = (unsigned char*)pDest;
//	unsigned short *p16 = (unsigned short *)pSrc;
//	const int chunks = count >> 4; // process 256 bit chunks at a time (256/16)=16
//	uint8x8_t a8, b8;
//	uint8x16_t out;
//	const uint16x8_t dc8 = vdupq_n_u16(dc); // DC term to subtract
//	for(int i = 0; i < chunks; i++, p16 += 16, p8 += 16)
//	{
//		a8 = vqmovn_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 0), dc8));
//		b8 = vqmovn_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 8), dc8));
//		out = vcombine_u8 (a8, b8);
//		vst1q_u8 (p8, out );
//	}
//	return 0;
//}

// Need template specialization for const int to make neon happy, doesn't like "const int" as shift parameter
// since it could be out of the valid range.
template <int N>
class Convert
{
public:
	static unsigned int U16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const unsigned short dc)
	{
		//printf ("KSHIFT %d, DC %d\n", kShift, dc);
		unsigned char *p8 = (unsigned char*)pDest;
		unsigned short *p16 = (unsigned short *)pSrc;
		const int chunks = count >> 4; // process 256 bit chunks at a time (256/16)=16
		uint8x8_t a8, b8;
		uint8x16_t out;
		const uint16x8_t dc8 = vdupq_n_u16(dc); // DC term to subtract
		for(int i = 0; i < chunks; i++, p16 += 16, p8 += 16)
		{
			a8 = vqmovn_u16( vshrq_n_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 0), dc8), kShift));
			b8 = vqmovn_u16( vshrq_n_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 8), dc8), kShift));
			out = vcombine_u8 (a8, b8);
			vst1q_u8 (p8, out );
		}
		return 0;
	}
private:
	static const int kShift = N;
};

// Specialize 0 to avoid shift instruction
template <> unsigned int Convert<0>::U16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const unsigned short dc)
{
	unsigned char *p8 = (unsigned char*)pDest;
	unsigned short *p16 = (unsigned short *)pSrc;
	const int chunks = count >> 4; // process 256 bit chunks at a time (256/16)=16
	uint8x8_t a8, b8;
	uint8x16_t out;
	const uint16x8_t dc8 = vdupq_n_u16(dc); // DC term to subtract
	for(int i = 0; i < chunks; i++, p16 += 16, p8 += 16)
	{
		a8 = vqmovn_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 0), dc8));
		b8 = vqmovn_u16( vqsubq_u16( vld1q_u16 ((const uint16_t *)p16 + 8), dc8));
		out = vcombine_u8 (a8, b8);
		vst1q_u8 (p8, out );
	}
	return 0;
}
unsigned int ConvertU16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const int shift, const unsigned short dc)
{
	switch(shift)
	{
		case 0:
			Convert<0>::U16ToU8(pSrc, pDest, count, dc);
			break;
		case 1:
			Convert<1>::U16ToU8(pSrc, pDest, count, dc);
			break;
		case 2:
			Convert<2>::U16ToU8(pSrc, pDest, count, dc);
			break;
		case 3:
			Convert<3>::U16ToU8(pSrc, pDest, count, dc);
			break;
		case 4:
			Convert<4>::U16ToU8(pSrc, pDest, count, dc);
			break;
		default:
			throw Exception("shift parameter out of range");
			break;
	}
}
#else
unsigned int ConvertU16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const int shift, const unsigned short dc)
{
	unsigned char *p8 = pDest;
	unsigned short *p16 = (unsigned short *)pSrc;
	for(int i = 0; i < count; i++, p8++, p16++)
	{
		unsigned short tmp = ((*p16 - dc) >> shift);
		*p8 = (tmp & 0xff00) ? ((unsigned char)0xff) : ((unsigned char) (tmp & 0x00ff));
	}
	return 0;
}
#endif
