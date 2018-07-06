/*
 * Image16To8Conv.h
 *
 *  Created on: Nov 9, 2012
 *      Author: mamigo
 */

#ifndef IMAGE16TO8CONV_H_
#define IMAGE16TO8CONV_H_

unsigned int ConvertU16ToU8(unsigned short *pSrc, unsigned char *pDest, int count, const int shift, const unsigned short dc);

#endif /* IMAGE16TO8CONV_H_ */
