/*
 * IOClass.h
 *
 *  Created on: 16-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef MATCHEDSIGNAL_H_
#define MATCHEDSIGNAL_H_
#include <stdio.h>

class MatchedSignal {
public:
	MatchedSignal();
	virtual ~MatchedSignal();
	int virtual Config()=0;
	void virtual Send(const char *data)=0;
	void PrintForDebug(const char *data){
	    printf("\nMatch Key Content: ");
	    unsigned char *ptr = (unsigned char*)(data);
	    for(int i = 0;i < ((m_bitcount + 7) >> 3);i++)
	        printf("%02x ", *ptr++);
	    printf("\n");
	}
	virtual void RestartSignal(){}

	virtual void CloseSignal(){}

	int m_bitcount;
	int m_endstate;
	bool m_debug,m_bdebug;
	int m_sleeptime;

};

#endif /* MATCHEDSIGNAL_H_ */
