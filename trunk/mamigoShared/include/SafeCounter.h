/*
 * SafeCounter.h
 *
 *  Created on: 15 Oct, 2009
 *      Author: akhil
 */

#ifndef SAFECOUNTER_H_
#define SAFECOUNTER_H_
#include <stdio.h>
#include "Safe.h"

class SafeCounter: public Safe<int> {
public:
	SafeCounter(int maxVal=1,int minVal=0){init(maxVal,minVal);}
	virtual ~SafeCounter(){};
	void init(int maxVal,int minVal=0){
		m_min=minVal;
		m_max=maxVal;
		m_val = minVal;
	}
	void Incr(){
		lock();
		m_val++;
//		printf("I %d \n",m_val);
		unlock();
	}
	void Decr(){
		lock();
		m_val--;
//		printf("D %d \n",m_val);
		unlock();
	}
	bool IncrLimit(int byVal=1){
			bool bSuccess=false;
			lock();
			if(m_val+byVal<=m_max) {
				m_val+=byVal;
				bSuccess=true;
			}
//			printf("I %d \n",m_val);
			unlock();
			return bSuccess;
	}
	bool DecrLimit(){
		bool bSuccess=false;
		lock();
		if(m_val>m_min) {
			m_val--;
			bSuccess=true;
		}
//		printf("D %d \n",m_val);
		unlock();
		return bSuccess;
	}
	bool isMin(){
//		printf("%d <= %d \n",m_val,m_min);
		return m_val<=m_min;
	}
	bool isMax(){
//		printf("%d >= %d \n",m_val,m_max);
		return m_val>=m_max;
	}

protected:
	int m_min;
	int m_max;
};

#endif /* SAFECOUNTER_H_ */
