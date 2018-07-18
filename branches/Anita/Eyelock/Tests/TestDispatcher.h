/*
 * TestDispatcher.h
 *
 *  Created on: 10-Oct-2009
 *      Author: mamigo
 */

#ifndef TESTDISPATCHER_H_
#define TESTDISPATCHER_H_

#include <tut/tut.hpp>
#include "LEDDispatcher.h"
using namespace tut;
class TestDispatcher: public LEDDispatcher {
public:
	TestDispatcher(Configuration& conf):LEDDispatcher(conf) {}
	virtual ~TestDispatcher() {	}
	void SetCurrState(bool inp){
		calls=0;
	}
	void SetExpected(bool inp){ expected = inp;}
	void Test(const char * msg, int expected_calls) { ensure_equals(msg,calls,expected_calls);}

protected:
	virtual void process(MatchResult *msg){}
	virtual bool enqueMsg(Copyable& msg){
		ensure("Expectation met",expected);
		LEDDispatcher::enqueMsg(msg);
		calls++;
	}
	int getCalls(){ return calls;}
	bool expected;
	int calls;
};

#endif /* TESTDISPATCHER_H_ */
