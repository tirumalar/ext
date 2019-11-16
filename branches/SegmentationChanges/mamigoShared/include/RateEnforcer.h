/*
 * RateEnforcer.h
 *
 *  Created on: 7 Jan, 2009
 *      Author: akhil
 */

#ifndef RATEENFORCER_H_
#define RATEENFORCER_H_

#if (defined(__WIN32__) || defined(_WIN32))
	#include "GroupSockHelper.hh"	// it has the windows version of gettimeofday
	#include <apr_time.h>		// it has a microsecond sleep function
	#define usleep  apr_sleep
#else
	#include <unistd.h>
	#include <sys/time.h>
#endif

#include <time.h>
#include <iostream>
#include <assert.h>
using std::cout;
/*
Two ways to use this class:
start(), tick(), tick().... as frameServer does it
or
start() end(); start() end(); start end()... as frameClient does
*/
class RateEnforcer {
public:
	RateEnforcer(float fps):count(0), freeTime(0){
		assert(fps>0);
		duration_uSec=(long)(1000000.0f/fps);
	}
	virtual ~RateEnforcer(){}
	inline void start(){
		pStart=&startTime;
		pEnd=&endTime;
		gettimeofday(pStart,0);
	}
	inline void end(){
		gettimeofday(&endTime,0);
		long diff=duration_uSec-tvdelta(&endTime,&startTime);
		if(diff>0) usleep(diff);
	}
	inline void tick(){
		gettimeofday(pEnd,0);
		long diff=duration_uSec-tvdelta(pEnd,pStart);
		//swap thepointers
		pStart=pEnd;
		pEnd=(pEnd==&startTime)?(&endTime):(&startTime);

		//update counters
		freeTime+=diff;
		count++;

		if(diff>0) usleep(diff);
		if(count%1024==0) report();
	}
	inline static unsigned long tvdelta(struct timeval *t1, struct timeval *t2){
			unsigned long delta=(t1->tv_sec-t2->tv_sec)*1000000;
			delta +=(t1->tv_usec-t2->tv_usec);
			return delta;
	}
	inline timeval *getStartTime(){ return pStart;}
private:

	long duration_uSec;
	struct timeval startTime;
	struct timeval endTime;
	struct timeval *pStart;
	struct timeval *pEnd;
	long count;
	long freeTime;

	void report(){
		long actualTime=(duration_uSec<<10)-freeTime;
		actualTime>>=10;	//uSec_per_frame
		float FPS=1000000.0f/actualTime;
		cout<<"\nFPS:\t"<<FPS<<"\n";
		count=0;
		freeTime=0;
	}
};

#endif /* RATEENFORCER_H_ */
