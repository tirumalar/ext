/*
 * HDMRemote.h
 *
 *  Created on: 04-Mar-2010
 *      Author: akhil
 */

#ifndef HDMREMOTE_H_
#define HDMREMOTE_H_

#include "HDMatcher.h"
#include <time.h>
#include <cstdlib>
#include <utility>
class HDMatcher;
class HostAddress;
class SocketClient;
class Socket;
class TextMessage;
class FixedMsg;
class BinMessage;
class SocketFactory;
class DBAdapter;
typedef  enum{PING,ASSIGNDB,MATCH,EXITIT,UPDATEIRIS,DELETEIRIS,VALIDATECARD}NWMSGTYPE;

class HDMRemote: public HDMatcher {
public:
	HDMRemote(int size,int id,int featureMask,const char *add=NULL);
	virtual ~HDMRemote();
	virtual void InitSSL();
	virtual bool StartMatch(unsigned char *iriscode, int taskid);
	virtual void AssignDB(char *fname,int memio=0);
	virtual void AssignDB(DBAdapter *dbAdapter);
	virtual bool GetPong(int& id,int& start,int& numeyes);
	//pessimistic calc
//	virtual int  GetNumEyesPossibleInBuffer();
	int GetType(){return 1;}///REMOTE
	virtual bool CheckIfDone(bool check=true);
	void SetTimeOut(struct timeval timeOut){m_timeOut=timeOut;}
	virtual bool SendPing();
	virtual bool isReboot(bool);
	virtual bool UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris);
	virtual bool DeleteSingleUserOnly(unsigned char *guid);
	virtual void PrintGUIDs();
protected:
	SocketClient* m_sock;
	HostAddress *m_pAddress;
	TextMessage* m_genMsg;
	FixedMsg* m_outDataMsg;
	BinMessage* m_dbMsg;
	BinMessage* m_matchMsg;
	SocketFactory *m_socketFactory;
	bool GetAck(Socket* sock, NWMSGTYPE ack);
	bool GetResult();
	struct timeval m_timeOut;
};

#endif /* HDMREMOTE_H_ */
