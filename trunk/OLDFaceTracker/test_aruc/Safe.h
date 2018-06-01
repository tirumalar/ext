/*
 * Safe.h
 *
 *	Safe holder for a type.
 *  Created on: 05-Sep-2009
 *      Author: mamigo
 */

#ifndef SAFE_H_
#define SAFE_H_

#include <pthread.h>

template <typename T>
class Safe {
public:
	Safe(T val):m_val(val) {
		pthread_mutex_init(&myMutex,NULL);
		bUpdated=false;
	}
	Safe(){
		pthread_mutex_init(&myMutex,NULL);
		bUpdated=false;
	}
//	Safe( const Safe& other):m_val(other.m_val), bUpdated(other.bUpdated){
//		pthread_mutex_init(&myMutex,NULL);
//	}
	virtual ~Safe() {
		pthread_mutex_destroy(&myMutex);
	}
	void waitFor(pthread_cond_t *condition){
		pthread_cond_wait(condition,&myMutex);
	}

	bool isUpdated(){return bUpdated;}

	void setUpdated(bool isUpdated){bUpdated=isUpdated;}

	void lock(){
		pthread_mutex_lock(&myMutex);
	}

	void unlock(){
		pthread_mutex_unlock(&myMutex);
	}
	T &get(){ return m_val;}
	void set(T val){ m_val=val;}
protected:

	pthread_mutex_t myMutex;
	T m_val;
	bool bUpdated;
};

#endif /* SAFE_H_ */
