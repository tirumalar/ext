/*
 * SDKDispatcher.h
 *
 *  Created on: Jan 1, 2015
 *      Author: developer
 */

#ifndef SDKDISPATCHER_H_
#define SDKDISPATCHER_H_
#include <set>
#include "GenericProcessor.h"
#include "socket.h"
#include "MessageExt.h"
#include "Synchronization.h"

class SocketFactory;
enum ServerType{MATCHED,TAMPERED};
enum SDKCallbackType{TAMPERSET,TAMPERCLEAR,MATCH};

class SDKCallbackMsg : public Copyable
{
public:
	SDKCallbackMsg(): m_eventType(TAMPERSET), m_msg("") {};
	SDKCallbackMsg(SDKCallbackType eventType, string info);

	SDKCallbackType GetEventType() { return m_eventType; };
	string GetMsg() { return m_msg; };
	BinMessage* GetBinMsg() { return new BinMessage(m_msg.c_str(), (unsigned int)m_msg.length()); };

	virtual void CopyFrom(const Copyable& other) { CopyFrom((SDKCallbackMsg&) other); };
	virtual void CopyFrom(SDKCallbackMsg& other) { m_eventType = other.GetEventType(); m_msg = other.GetMsg(); };

	static string GetDeviceTypeAndId();

protected:
	SDKCallbackType m_eventType;
	string m_msg;
};

class SDKDispatcher: public GenericProcessor {
public:
	SDKDispatcher(Configuration* conf);
	virtual ~SDKDispatcher();
	virtual const char *getName(){ return "SDKDispatcher";}
	void AppendTamperServer(string address);
	void AppendMatchServer(string address);
	void ClearMatchServer();
	void ClearTamperServer();
	set<string> GetTampreAddressList();
	set<string> GeMatchAddressList();
	bool DeleteMatchServer(string server);
	bool DeleteTamperServer(string server);
	void SendTamperMsg(SDKCallbackMsg& msg);
	void SendMatchMsg(SDKCallbackMsg& msg);
	void SendMatchMsg(string Matchinfo);
	void init();
	int32_t Save();
	int32_t Load();
	bool enqueMsg(SDKCallbackMsg& msg);

private:
	virtual void process(Copyable *msg);
	virtual Copyable *createNewQueueItem();
	virtual int getQueueSize(Configuration* conf);
	virtual void runDiagnostics() {};
	bool SendMsg(HostAddress &add,timeval &sendtimeOut,timeval &conntimeOut,BinMessage& out_msg,bool bblock=true);
	const char* m_tamperFilePath;
	long int m_tamperRepeatTimeSec;
	long int m_lastTamperSent;
	SocketFactory *m_socketFactory;
	Configuration *m_conf;
	bool m_debug;
	struct timeval m_timeOut;
	set<string> m_tamperDestAddr;
	set<string> m_matchDestAddr;
	Mutex m_tamperLock,m_matchLock;

};

#endif /* SDKDISPATCHER_H_ */
