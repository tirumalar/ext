/*
 * F2FSignal.h
 *
 *  Created on: 16-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef F2FSIGNAL_H_
#define F2FSIGNAL_H_

#include "MatchedSignal.h"
class Configuration;

class F2FSignal: public MatchedSignal {
public:

	enum BoardType { EyeswipeMini, EyeswipeNano, Eyelock };

	F2FSignal(Configuration& conf);
	virtual ~F2FSignal();
    virtual int Config();
    virtual void RestartSignal();
    virtual void CloseSignal();

    virtual void Send(const char *data);
    int m_Timep;
    char *m_flag;
    BoardType m_BoardType;
};

#endif /* F2FSIGNAL_H_ */
