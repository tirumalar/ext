/*
 * NwLEDDispatcher.h
 *
 *  Created on: Jan 28, 2013
 *      Author: mamigo
 */

#ifndef NWLEDDISPATCHER_H_
#define NWLEDDISPATCHER_H_

#include "GenericProcessor.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include "CommonDefs.h"
class SocketFactory;

class NwLEDDispatcher: public GenericProcessor {
public:
	NwLEDDispatcher(Configuration& conf);
	virtual ~NwLEDDispatcher();
	virtual const char *getName(){ return "NwLEDDispatcher";}
protected:
	virtual int getQueueSize(Configuration* conf);
	virtual Copyable *createNewQueueItem();
	virtual void process(Copyable *msg);
	struct timeval m_timeOut;
	HostAddress *m_resultDestAddr[2];
	BinMessage m_outMsg;
	SocketFactory *m_socketFactory;
	bool m_debug,m_tsDestAddrpresent;
	char *m_ledstr;

};

#endif /* NWLEDDISPATCHER_H_ */
