/*
 * MatchManagerFactory.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */
#include "MatchManagerInterface.h"
#include "MatchManagerFactory.h"
#include "MatchManager.h"
#include "MatchManagerSimple.h"
#include "Configuration.h"
#include "BiOmega.h"
#include "IrisDBHeader.h"


MatchManagerFactory::MatchManagerFactory(Configuration& conf) {
	setConf(&conf);
}

MatchManagerFactory::~MatchManagerFactory() {

}

MatchManagerInterface* MatchManagerFactory::Create(Configuration& conf,BiOmega *bio,int pcmatcher,int master){
	MatchManagerInterface *mi = NULL;

	if(pcmatcher){
		mi = (MatchManagerInterface *)new MatchManagerSimple(conf,bio);
	}else{
		mi = (MatchManagerInterface *)new MatchManager(conf,bio);
	}

	if(!master){
		mi->CreateMatchers(conf);
		mi->AssignDB();
	}

	return mi;
}
