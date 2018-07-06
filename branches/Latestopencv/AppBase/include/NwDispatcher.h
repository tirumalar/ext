/*
 * NwDispatcher.h
 *
 *  Created on: 17-Sep-2009
 *      Author: mamigo
 */

#ifndef NWDISPATCHER_H_
#define NWDISPATCHER_H_

#include "ResultDispatcher.h"
#include "socket.h"
#include "MessageExt.h"
class SocketFactory;
#define NW_MATCHMSG_MAXLEN 256
class NwDispatcher: public ResultDispatcher {
public:
	NwDispatcher(Configuration& conf);
	virtual ~NwDispatcher();
	const char *getName(){
			return "NwDispatcher";
	}
#ifndef UNITTEST
protected:
#endif
	void AppendFile(BinMessage* out);
	virtual int getQueueSize(Configuration* conf);
	virtual void process(MatchResult *msg);
	virtual void ProcessOnEmptyQueue();
#ifndef UNITTEST
private:
#endif
	bool m_debug;
	enum NwMsgType{ UID=0,F2F,RAW, VERBOSE};
	NwMsgType m_type;
	struct timeval m_timeOut;
	HostAddress *m_resultDestAddr;
	const char* m_msgFormat;
	const char* m_nwFormat;
	BinMessage m_outMsg;
	const char* m_fileName;
	FileMsg *m_DLFileMsg;
	long int m_timeCount;
	SocketFactory *m_socketFactory;
};

#endif /* NWDISPATCHER_H_ */
