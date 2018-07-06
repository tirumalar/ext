/*
 * ProcessorChain.cpp
 *
 *  Created on: 18-Sep-2009
 *      Author: mamigo
 */

#include "ProcessorChain.h"

ProcessorChain::ProcessorChain(){

}

ProcessorChain::~ProcessorChain() {

}


void ProcessorChain::callNext(Copyable& msg){
	list<ProcessorChain *>::iterator it;
	for(it=m_nextProcessors.begin(); it!=m_nextProcessors.end(); it++)
	{
		(*it)->enqueMsg(msg);
	}
}
