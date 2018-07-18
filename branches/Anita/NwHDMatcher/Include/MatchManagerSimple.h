/*
 * MatchManagerRemote.h
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */

#ifndef MATCHMANAGERREMOTE_H_
#define MATCHMANAGERREMOTE_H_

#include "MatchManagerInterface.h"

class MatchManagerSimple: public MatchManagerInterface {
public:
	MatchManagerSimple(Configuration& conf,BiOmega *bio);
	virtual void AssignDB();
	virtual void CreateMatchers(Configuration & conf);
	virtual ~MatchManagerSimple();
	virtual int getKeyLength(){ return m_KeyLength;}
	virtual void RecoverFromBadState();
	virtual void InitResult();
	virtual bool HDMDiagnostics();
	virtual void RegisterResult(int id, int taskid, std::pair<int,float> inp, unsigned char *key, int keysz = -1);

protected:
	int m_KeyLength;


};

#endif /* MATCHMANAGERREMOTE_H_ */
