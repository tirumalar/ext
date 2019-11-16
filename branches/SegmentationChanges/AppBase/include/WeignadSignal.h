/*
 * WeignadSignal.h
 *
 *  Created on: 16-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef WEIGNADSIGNAL_H_
#define WEIGNADSIGNAL_H_

#include "MatchedSignal.h"
class Configuration;

class WeignadSignal: public MatchedSignal {
public:

	enum BoardType { EyeswipeMini, EyeswipeNano, Eyelock };

	WeignadSignal(Configuration& conf);
	virtual ~WeignadSignal();
    int virtual Config();
    virtual void RestartSignal();
    virtual void CloseSignal();

	void virtual Send(const char *data);
	int m_lowtime;
	int m_hightime;
	char *m_clk1,*m_clk2;
	BoardType m_BoardType;
	bool m_oldWeigandSupport;
	bool m_pac;
	bool m_wighid;
	int m_WaitBobTime;
	bool m_wiegandLedIn;
	bool m_transTOC;
};

#endif /* WEIGNADSIGNAL_H_ */
