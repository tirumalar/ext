/*
 * HDMatcherFactory.h
 *
 *  Created on: 02-Mar-2010
 *      Author: akhil
 */

#ifndef HDMATCHERFACTORY_H_
#define HDMATCHERFACTORY_H_
#include <time.h>
#include "Configurable.h"
enum MATCHER_TYPE{ LOCAL,REMOTEPROXY,NWMATCHER,PCMATCHER};

class HDMatcher;
class MatchManagerInterface;
class Configuration;
class IrisDBHeader;

class HDMatcherFactory: public Configurable {
public:
	HDMatcherFactory(Configuration& conf,MatchManagerInterface *ptr=0);
	virtual ~HDMatcherFactory();
	MatchManagerInterface *m_Mgr;
	HDMatcher *Create(int matchtype,int irissz, int size,int id,const char *add=NULL);
protected:
	struct timeval m_timeOut;

};

#endif /* HDMATCHERFACTORY_H_ */
