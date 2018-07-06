/*
 * MatchManagerFactory.h
 *
 *  Created on: Jun 8, 2011
 *      Author: developer1
 */

#ifndef MATCHMANAGERFACTORY_H_
#define MATCHMANAGERFACTORY_H_
#include "Configurable.h"

class MatchManagerInterface;
class Configuration;
class BiOmega;

class MatchManagerFactory: public Configurable {
public:
	MatchManagerFactory(Configuration& conf);
	virtual ~MatchManagerFactory();
	MatchManagerInterface *Create(Configuration& conf,BiOmega *bio,int pcmatcher,int master=0);
};

#endif /* MATCHMANAGERFACTORY_H_ */
