/*
 * DBReceive.h
 *
 *  Created on: 13-Feb-2010
 *      Author: akhil
 */

#ifndef DBRECEIVE_H_
#define DBRECEIVE_H_


#include "GenericProcessor.h"
#include "HTTPPOSTMsg.h"
#include "socket.h"
#include "IrisDBHeader.h"
class BinMessage;
class HDMatcherMsg;
class LEDDispatcher;
class TriggerDispatcher;
class F2FDispatcher;
class SocketFactory;
class DBRecieveMsg;


class DBReceive: public GenericProcessor {
public:
	DBReceive(Configuration& conf);
	virtual ~DBReceive();
	virtual void process(Copyable *msg);
	virtual Copyable *createNewQueueItem();
	bool instateNewFile(void);
	void EnableDispatchers(bool success=true);
	const char *getName(){
		return "DBReceive";
	}
	void SetF2FDispatcher(F2FDispatcher *ptr){m_f2fDispatcher = ptr;}
	void SendMessage(HTTPPOSTMsg& msg);
	bool SearchAndUpdate(char *guid,char *data,int length);
	unsigned char *GetGuidFromIncrement(int index);
	bool ReadIncrementalRecords(char *filename);
	bool CheckIncrementalRecordsWithDB(void);
	void RemoveTheRecords();
	void AddUpdateTheRecords();
	unsigned char* GetIrisFromIncrement(int index);
	FileMsg *m_DBFileMsg;
	HTTPPOSTMsg *m_ReloadMsg;
	LEDDispatcher *m_ledDispatcher;
	TriggerDispatcher *m_trigDispatcher;
	F2FDispatcher *m_f2fDispatcher;

protected:

	void GetCompleteData(char* buffer, char*, int, int, Socket&);
	void HandleSqliteDB(DBRecieveMsg *msg);
	static int callback(void *NotUsed, int argc, char **argv, char **azColName);
	HTTPPOSTMsg m_F2FDbRxMsg,m_F2FDbDoneMsg;
	MatchResult m_f2fResult;
	const char *m_dbFileName;
	char *m_tempDbFileName,*m_temp1DbFileName;
//	BinMessage *m_DBack;
//	BinMessage *m_DBNack;
    HostAddress *m_resultDestAddr;
    SocketFactory *m_socketFactory;
	struct timeval m_timeOut,m_timeOutSend;
	bool m_Debug;
	unsigned char *m_TempRecord;
	int m_numRecords,m_TempRecordSize;
	int *m_TempRecordState;
	IrisDBHeader m_TempirisDBHeader;
	HDMatcherMsg *m_DeleteUpdateMsg;
	char *m_tempBuffer;
	HTTPPOSTMsg *m_ReceiveMsg;
	int m_rxfilenumber;
};

class DBRecieveMsg: public Copyable{
public:
	DBRecieveMsg(bool shallow=false);
	~DBRecieveMsg();
	bool CheckDB(char *fname);
	int m_SD;			//the socket descriptor for the incoming request
	char *m_prelude; 	// initial few bytes if they were recvd before handoff
	int m_preludeSize;  //size of prelude
	int m_FileSize;
	void *m_SecureTrait;
	int m_isDownloadUsr;
	DBMsgType m_msgType;
	string m_IpPort;
	int m_isEncrypt;
	void CopyFrom(const Copyable& other);
protected:
	bool m_shallow;
};
#endif /* DBRECEIVE_H_ */
