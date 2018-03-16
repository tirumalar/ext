/*
 * EyeDispatcher.h
 *
 *  Created on: Jan 17, 2011
 *      Author: developer1
 */

#ifndef EYEDISPATCHER_H_
#define EYEDISPATCHER_H_

#include "GenericProcessor.h"
#include "EyeDispatcher.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include "SafeFrameMsg.h"
#include "CircularAccess.h"
#include "Synchronization.h"
#include <vector>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
class SocketFactory;

class EyeDispatcher: public GenericProcessor {
public:
	EyeDispatcher(Configuration& conf, const char *address);
	EyeDispatcher(Configuration& conf);
	virtual ~EyeDispatcher();
	void process(Copyable *msg){}
	virtual void init(){};
	virtual unsigned int MainLoop();
	const char *getName(){ return "EyeDispatcher"; }
	void FlushAll();
	bool IsEmpty(){ return shouldWait();}
	void SetAddress(const char *address);
	void AddAddress(const char *address);
	void SetSecureComm(bool secure);
	void ResetSecureComm ();
	void ClearAddressList();
	bool IsActive(); // Do we have any destination address?
	virtual bool enqueMsg(Copyable& msg);

	bool FindAddress(const char *address, int port);
protected :
	void GrabSendMsg(void);
    int getQueueSize(Configuration *conf){return 0;}
    Copyable *createNewQueueItem(){return NULL;}
    virtual Copyable *getNextMsgToProcess(){return NULL;}
	void Init(Configuration& conf, const char *address);
	void SendMessage(HTTPPOSTMsg* out_msg,bool bblock=true);
	void SendMessage(HTTPPOSTMsg* out_msg, HostAddress &haddr, bool bblock=true, bool retry=false);
	bool GetFreeMsg(int cam,int& index);
	void PushMsg(int cam,HTTPPOSTMsg& msg);
	HTTPPOSTMsg* GetMsgToSend();
	bool  m_bSecure, m_bOrigSecure;
	Mutex m_Lock; // Needed for dynamic changes to host address list (enrollment requests, etc)
	Mutex m_SecureCommLock; // Needed for dynamic changes to host address list (enrollment requests, etc)
	BinarySemaphore m_Sem;
	int m_queueSz,m_cw,m_ch;
	int m_block;
	std::string m_destAddr;
	HostAddress *m_haddr;
	std::vector< HostAddress* > m_haddrList;
    struct timeval m_timeOutConn,m_timeOutSend;
    SocketFactory *m_socketFactory;
    int m_numbits;
    Mutex m_vectorLock;
    int m_prevcam;
    std::vector<std::pair<HTTPPOSTMsg*,bool> > m_CList[2];
    HTTPPOSTMsg* m_tempMsg;
    bool m_debug;
    int m_eyeDispatcherSleepUsec;
};

#endif /* EYEDISPATCHER_H_ */
