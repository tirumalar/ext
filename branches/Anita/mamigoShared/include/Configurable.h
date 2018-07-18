/*
 * Configurable.h
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 *
 *    Any object which needs a configuration should inherit this interface
 */

#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_

#include "Configuration.h"

class Configurable {
public:
	Configurable(){}
	virtual ~Configurable(){}
	virtual void setConf(Configuration *conf){_conf=conf;}
protected:
	Configuration * getConf(){ return _conf;}
private:
	Configuration *_conf;
};

#endif /* CONFIGURABLE_H_ */
