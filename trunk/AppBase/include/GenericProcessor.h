/*
 * GenericProcessor.h
 *
 *  Created on: 11-Dec-2009
 *      Author: akhil
 */

#ifndef GENERICPROCESSOR_H_
#define GENERICPROCESSOR_H_

#include "HThread.h"
#include "CircularAccess.h"
#include "Safe.h"
#include "ProcessorChain.h"
#include "Configurable.h"

class Configuration;

class GenericProcessor: public HThread, public ProcessorChain, public Configurable {
public:
	GenericProcessor(Configuration& conf);
	virtual ~GenericProcessor();
	virtual void init();
	virtual unsigned int MainLoop();
	virtual bool enqueMsg(Copyable& msg);
	virtual const char *getName()=0;
protected:
	virtual int getQueueSize(Configuration* conf){ return 1;}
	virtual Copyable *getNextMsgToProcess();
	virtual bool shouldWait();

	// these need to be overridden by derived classes
	virtual void process(Copyable *msg)=0;
	virtual void postProcess() {} // always call this regardless of whether queue is full
	virtual Copyable *createNewQueueItem()=0;
	virtual void recoverFromBadState(){}
	virtual void runDiagnostics(){}
	virtual void afterEnque(Safe<Copyable*> & currMsg);
	QueueFullBehaviour m_queueFullBehaviour;

	Copyable *m_result; // temp buffer;

	typedef CircularAccess< Safe< Copyable *> > ResultQueue;
	typedef citerator<ResultQueue, Safe< Copyable *>& > ResultQueueIterator;

	ResultQueue m_inQueue;
	ResultQueueIterator m_sendIter;

};

#endif /* GENERICPROCESSOR_H_ */
