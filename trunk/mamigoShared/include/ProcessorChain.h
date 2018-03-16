/*
 * ProcessorChain.h
 *
 *  Created on: 18-Sep-2009
 *      Author: mamigo
 */

#ifndef PROCESSORCHAIN_H_
#define PROCESSORCHAIN_H_

#include <list>
#include "CommonDefs.h"

using namespace std;
class ProcessorChain {
public:
	ProcessorChain();
	virtual ~ProcessorChain();

	void addProcessor(ProcessorChain* next){m_nextProcessors.push_back(next);}

	// by default does nothing
	virtual bool enqueMsg(Copyable& msg){return true;}

	virtual void callNext(Copyable& msg);
protected:

	list<ProcessorChain *> m_nextProcessors;
};

#endif /* PROCESSORCHAIN_H_ */
