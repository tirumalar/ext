#include "GenericProcessor.h"
#include "Configuration.h"
#include "socket.h"
#include <iostream>

using namespace std;

GenericProcessor::GenericProcessor(Configuration& conf) :
	m_sendIter(m_inQueue) {
	// base class
	setConf(&conf);
	m_queueFullBehaviour = DROP;
	m_counter=0;
	m_timeelapsed=0;
	m_result = NULL;
}
void GenericProcessor::init() {

	Configuration *pConf = getConf();
	int inQSize = getQueueSize(pConf);
	m_inQueue(inQSize);

	for (int i = 0; i < inQSize; i++) {
		m_inQueue[i] = Safe<Copyable *> (createNewQueueItem());
	}
	m_result = createNewQueueItem();
}

GenericProcessor::~GenericProcessor() {
	for (int i = 0; i < m_inQueue.getSize(); i++) {
		delete m_inQueue[i].get();
	}
	if (m_result)
		delete m_result;
}

void GenericProcessor::afterEnque(Safe<Copyable*> & currMsg){
	currMsg.setUpdated(true);
	m_inQueue++;
}

bool GenericProcessor::enqueMsg(Copyable& msg) {
	Safe<Copyable *> & currMsg = m_inQueue.getCurr();

	bool wrote = false;
	bool overwrite = false;
	currMsg.lock();
	if (currMsg.isUpdated()) {
		if (m_queueFullBehaviour == OVERWRIE_OLD) {
			//printf("GenericProcessor::input queue full, over-writing\n");
			printf(".");
			currMsg.get()->CopyFrom(msg);
			wrote = true;
			overwrite = true;
		} else if (m_queueFullBehaviour == DROP) {
			printf("%s :: GenericProcessor::input queue full, dropping\n",getName());
		} else
			throw "GenericProcessor::queueFullBehaviour:OVERWRIE_NEW not implemented";
	} else {
		// just normally write the new message
		currMsg.get()->CopyFrom(msg);
		wrote = true;
	}

	if(wrote)
		afterEnque(currMsg);

	currMsg.unlock();

	// if we wrote something lets inform the others who may be interested
	if (wrote) {
		if(!overwrite)m_inQueue.incrCounter();
		dataAvailable();
		return true;
	}
	return false;
}

bool GenericProcessor::shouldWait() {
	return m_inQueue.isEmpty();
}

// blocks till it gets a new message
Copyable *GenericProcessor::getNextMsgToProcess() {

	bool bFound = false;
	while (!bFound && !ShouldIQuit()) {
		recoverFromBadState();
		Safe<Copyable *> & currMsg = m_sendIter.curr();
		currMsg.lock();
		if (currMsg.isUpdated()) {
			m_result->CopyFrom(*currMsg.get()); // make a copy
			currMsg.setUpdated(false);//empty
			bFound = true;
		}
		currMsg.unlock();
		if (bFound) {
			m_inQueue.decrCounter();
		} else {
			runDiagnostics();
			waitForDataAndTimeOut();
			//printf("done waiting\n");
		}
		m_sendIter.next();
	}
	if (bFound) {
		return m_result;
	}
	return 0;
}


unsigned int GenericProcessor::MainLoop() {
	std::string name = "GenericProcessor::";
	try {

		while (!ShouldIQuit()) {
			Copyable *msg = getNextMsgToProcess();

			if (msg){
				process(msg);
				Frequency();
			}
			postProcess();
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}

	return 0;
}
