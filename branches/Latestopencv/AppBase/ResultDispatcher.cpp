/*
 * ResultDispatcher.cpp
 *
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include "ResultDispatcher.h"
#include "Configurable.h"
#include <iostream>
#include "logging.h"

using namespace std;
const char logger[30] = "ResultDispatch";

ResultDispatcher::ResultDispatcher(Configuration& conf):m_sendIter(m_inQueue) {
// base class
	setConf(&conf);
	m_result = new MatchResult;
	m_queueFullBehaviour = DROP;
}

void ResultDispatcher::init(){
	Configuration *pConf = getConf();
	int inQSize = getQueueSize(pConf);
	m_inQueue(inQSize);
	for (int i = 0; i < inQSize; i++) {
		m_inQueue[i] = Safe<MatchResult *> (new MatchResult());
	}
}

ResultDispatcher::~ResultDispatcher() {
	for (int i = 0; i < m_inQueue.getSize(); i++) {
		delete m_inQueue[i].get();
	}
	if (m_result)
		delete m_result;

}


bool ResultDispatcher::enqueMsg(Copyable& msg) {
	Safe<MatchResult *> & currMsg = m_inQueue.getCurr();
	bool overwrite = false;
	bool wrote=false;
	currMsg.lock();
	if (currMsg.isUpdated()) {
		if (m_queueFullBehaviour == OVERWRIE_OLD) {
			EyelockLog(logger,  ERROR, "ResultDispatcher::input queue full, over-writing");
			currMsg.get()->CopyFrom(msg);
			wrote=true;
			overwrite = true;
		} else if (m_queueFullBehaviour == DROP) {
			EyelockLog(logger, ERROR, "ResultDispatcher::input queue full, dropping");
		} else
			throw "ResultDispatcher::queueFullBehaviour:OVERWRIE_NEW not implemented";
	}
	else
	{
		// just normally write the new message
		currMsg.get()->CopyFrom(msg);
		wrote=true;
	}

	if(wrote){
		currMsg.setUpdated(true);
		m_inQueue++;
	}

	currMsg.unlock();

	// if we wrote something lets inform the others who may be interested
	if(wrote){
		if(!overwrite)m_inQueue.incrCounter();
		dataAvailable();
	}
	return true;
}


bool ResultDispatcher::shouldWait(){
	return m_inQueue.isEmpty();
}

// blocks till it gets a new message
MatchResult *ResultDispatcher::getNextMsgToProcess() {

	bool bFound = false;
	while (!bFound && !ShouldIQuit()) {
		Safe<MatchResult *> & currMsg = m_sendIter.curr();
		currMsg.lock();
		if (currMsg.isUpdated()) {
			m_result->CopyFrom(*currMsg.get()); // make a copy
			currMsg.setUpdated(false);//empty
			bFound = true;
		}
		currMsg.unlock();
		if (bFound) {
			m_inQueue.decrCounter();
		}
		else{
			ProcessOnEmptyQueue();
			waitForDataAndTimeOut();
		}
		Frequency();
		m_sendIter.next();
	}
	if(bFound) {
		return m_result;
	}
	return 0;
}

void ResultDispatcher::process(MatchResult *msg) {
	try{
			//do something here

	}
	catch(const char *msg){
		cout <<msg <<endl;
	}
	catch(exception ex){
		cout <<ex.what()<<endl;
	}
}
unsigned int ResultDispatcher::MainLoop() {

	std::string name = "ResultDispatcher";
	try {
		while (!ShouldIQuit()) {
			MatchResult *msg = getNextMsgToProcess();
			if(msg) process(msg);
			Frequency();
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}
	return 0;
}
