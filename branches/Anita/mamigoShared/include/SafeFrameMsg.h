/*
 * SafeFrameMsg.h
 *
 *  Created on: 7 Jan, 2009
 *      Author: akhil
 */

#ifndef SAFEFRAMEMSG_H_
#define SAFEFRAMEMSG_H_
#include <pthread.h>
#include "MessageExt.h"
/*
 * Mutex protected BinMessage. It allows lock/unlock operations by its client
 */

class SafeFrameMsg: public BinMessage {
private:
	pthread_mutex_t myMutex;
	bool bUpdated;
public:
	SafeFrameMsg(unsigned int Bytes):BinMessage(Bytes){
		pthread_mutex_init(&myMutex,NULL);
		bUpdated=false;
	}
	SafeFrameMsg(const char *Msg, unsigned int Len):BinMessage(Msg,Len){
		pthread_mutex_init(&myMutex,NULL);
		bUpdated=false;
	}
	virtual ~SafeFrameMsg(){
		pthread_mutex_destroy(&myMutex);
	}
	void lock(){
		pthread_mutex_lock(&myMutex);
	}

	void unlock(){
			pthread_mutex_unlock(&myMutex);
	}
	void waitFor(pthread_cond_t *condition){
		pthread_cond_wait(condition,&myMutex);
	}
	bool isUpdated(){return bUpdated;}
	void setUpdated(bool isUpdated){bUpdated=isUpdated;}
};

#endif /* SAFEFRAMEMSG_H_ */
