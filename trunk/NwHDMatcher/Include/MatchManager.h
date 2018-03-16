/*
 * MatchManager.h
 *
 *  Created on: 26-Feb-2010
 *      Author: akhil
 */

#ifndef MATCHMANAGER_H_
#define MATCHMANAGER_H_

#include "MatchManagerInterface.h"


class MatchManager: public MatchManagerInterface  {
public:
	MatchManager(Configuration& conf,BiOmega *bio);
	virtual ~MatchManager();

	virtual void AssignDB();
	virtual void CreateMatchers(Configuration & conf);
	virtual void AssignDB(int id,char *filename);
	virtual void InitResult();
	virtual bool HDMDiagnostics();
	bool SpecialCaseOfOneUser();
};



#endif /* MATCHMANAGER_H_ */
