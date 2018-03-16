/*
 * HDMPCRemote.h
 *
 *  Created on: Apr 23, 2012
 *      Author: developer1
 */

#ifndef HDMPCREMOTE_H_
#define HDMPCREMOTE_H_

#include "HDMRemote.h"

class HDMPCRemote: public HDMRemote {
public:
	HDMPCRemote(int size, int id,const char* addr);
	virtual ~HDMPCRemote();

	virtual bool StartMatch(unsigned char *iriscode, int taskid);
	virtual void AssignDB(char *fname,int memio=0);

	virtual bool GetPong(int& id,int& start,int& numeyes);
	virtual int  GetNumEyesPossibleInBuffer();
	virtual bool CheckIfDone(bool check=true);
	virtual bool SendPing(){ return true;}
	virtual bool CheckIfAvailable(){return m_Status==AVAILABLE;}
	bool GetResult();
	bool GetCardResult();
	bool GetAck(Socket* sock,NWMSGTYPE ackRx);
	virtual int GetType(){return 3;}///PCMATCHER
	virtual void AssignDB(DBAdapter *dbAdapter);
	virtual bool UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris);
	virtual bool DeleteSingleUserOnly(unsigned char *guid);

	bool SendPCMatchMsg(char *msg, int len);

private:
	int m_taskId;
};

#endif /* HDMPCREMOTE_H_ */
