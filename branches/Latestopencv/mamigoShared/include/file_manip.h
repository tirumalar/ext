/*
 * file_manip.h
 *
 *  Created on: 23 Aug, 2008
 *      Author: akhil
 */

#ifndef FILE_MANIP_H_
#define FILE_MANIP_H_
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>


#define TV_AS_USEC(t,u) uint64_t u = t.tv_sec; u = u*1000000; u += t.tv_usec
#define TV_AS_MSEC(t,u) uint64_t u = t.tv_sec; u = u*1000; u += (t.tv_usec/1000)

#define CURR_TV_AS_USEC(k) uint64_t k; {struct timeval _timerc;gettimeofday(&_timerc,0); TV_AS_USEC(_timerc,u); k=u;};
#define CURR_TV_AS_MSEC(k) uint64_t k; {struct timeval _timerc;gettimeofday(&_timerc,0); TV_AS_MSEC(_timerc,u); k=u;};
#define CURR_TV_AS_SEC(k) uint32_t k; {struct timeval _timerc;gettimeofday(&_timerc,0); k=_timerc.tv_sec;};


#define DEF_TIMER(m) struct timeval _timer_1,_timer_2; const char* _timer_msg=m
#define START_TIMER gettimeofday(&_timer_1,0)
#define END_TIMER gettimeofday(&_timer_2,0);printf("%s:%llu\n",_timer_msg,tvdelta(&_timer_2,&_timer_1))
#define TIME_OP(m, op) { \
	DEF_TIMER(m); \
	START_TIMER; \
	op;	\
	END_TIMER; \
	}

#define XTIME_OP(m, op) { \
	op;\
	}

#define XTIME_ADD(m, op) { \
	op;\
	}
#define XPRINTTIME(val)

#define END_TIMER_ADD(val) gettimeofday(&_timer_2,0); val+=tvdelta(&_timer_2,&_timer_1)
#define TIME_ADD(val,op) { \
	DEF_TIMER(NULL); \
	START_TIMER; \
	op;	\
	END_TIMER_ADD(val); \
	}
#define PRINTTIME(val) printf("%s:%llu\n",#val,val);

#define SAVE_NAME_INDX(img,name,index) savefile_OfSize_asPGM_index((unsigned char *)(img)->imageData,(img)->width, (img)->height,name,index)

uint64_t tvdelta(struct timeval *t1, struct timeval *t2);

int ReadPGM5(const char *fname, unsigned char *data, int *w, int *h, int bufferSize);
int ReadPGM5WHandBits(const char *fname, int *w, int *h, int *bits);
unsigned char * file_OfSize_readFromOffset(const char *fileName, int imagesize,int offset);
unsigned char * file_OfSize_readFromOffset_into(const char *fileName, int imagesize,int offset, unsigned char* buffer);
void savefile_OfSize_as(unsigned char* image, int imagesize, const char *fileName);
void savefile_OfSize_asPGM(unsigned char* image, short width, short height,const char *fileName);
void savefile_OfSize_asPGM_index(unsigned char* image, short width, short height, const char* prefix, int index);
void savefile_OfSize_asPGM16(unsigned char* image, short width, short height,const char *fileName);
void savefile_OfSize_asPGM16_index(unsigned char* image, short width, short height, const char* prefix, int index);
int FileSize( const char * szFileName );
//__int64 FileSize64( const char * szFileName );

#endif /* FILE_MANIP_H_ */
