/*
 * FrameGrabber.cpp
 *
 *  Created on: Jan 8, 2011
 *      Author: developer1
 */

#include "FrameGrabber.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


char xtod(char c){
	if( c >= '0' && c <= '9')
		return c - '0';
	if( c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if( c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return c = 0;
}

int HexToDec(const char *hex, int l){
	if(*hex == 0)
		return(l);
	return HexToDec(hex+1,l*16 + xtod(*hex));
}

int xstrtoi(const char *hex){
	return HexToDec(hex,0);
}

unsigned char hexStringToChar(const char *str){
	return (unsigned char) xstrtoi(str);
}

unsigned short hexStringToShort(const char *str){
	return (unsigned short) xstrtoi(str);
}

void FrameGrabber::update_extra_regs(char *str)
{
    if(str){
		printf("Configuring User Input registers \n");
		unsigned short value;
		unsigned char regis;
		char* ptr = strtok(str," ,.;");
		while(ptr != NULL){
			if(strncmp("-d",ptr,2)==0){
				ptr = strtok(NULL," ,.;");
				if(ptr == NULL){
					printf(" Error in MT9P031.extra_registers\n");
					break;
				}
				int del = atoi(ptr);
				printf("Sleeping %d uSec\n",del);
				usleep(del);
				ptr = strtok(NULL," ,.;");
				continue;
			}
			regis = hexStringToChar(ptr);
			ptr = strtok(NULL," ,.;");
			if(ptr == NULL){
				printf(" Error in MT9P031.extra_registers\n");
				break;
			}
			value = hexStringToShort(ptr);
			//printf("%d :: %#x\n",regis,value);
			update(regis, value);
			ptr = strtok(NULL," ,.;");
		}
	}
}
