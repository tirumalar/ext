/*
 * ResultDispatcher.h
 *
 *  abstract class which reads an input queue and processes the resuls
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef RESULTDISPATCHER_H_
#define RESULTDISPATCHER_H_

#include "HThread.h"
#include "CircularAccess.h"
#include "Safe.h"
#include "ProcessorChain.h"
#include "Configurable.h"

class Configuration;


typedef CircularAccess< Safe< MatchResult *> > ResultQueue;

typedef citerator<ResultQueue, Safe< MatchResult *>& > ResultQueueIterator;

class ResultDispatcher: public HThread, public ProcessorChain ,public Configurable {
public:
	ResultDispatcher(Configuration& conf);
	virtual ~ResultDispatcher();
	virtual unsigned int MainLoop();
	virtual bool enqueMsg(Copyable& msg);
	virtual const char *getName()=0;
	virtual void init();
	virtual void ProcessOnEmptyQueue(){}
protected:
	virtual int getQueueSize(Configuration* conf){ return 1;}
	virtual MatchResult *getNextMsgToProcess();
	virtual bool shouldWait();
	virtual void process(MatchResult *msg);
	QueueFullBehaviour m_queueFullBehaviour;
	MatchResult *m_result; // temp buffer;

	ResultQueue m_inQueue;
	ResultQueueIterator m_sendIter;
};

#endif /* RESULTDISPATCHER_H_ */
