/*
 * NWHDMatcher.h
 *
 *  Created on: 03-Mar-2010
 *      Author: akhil
 *      runs in a different process space
 */

#ifndef NWHDMATCHER_H_
#define NWHDMATCHER_H_
#include "socket.h"

class Configuration;
class HDMatcherMsg;
class BinMessage;
class HDMatcher;
class IrisDBHeader;
class ReaderWriter;
class IrisData;
class FileMsgSocketFactory;
class FileMsg;
class SocketFactory;
class DBAdapter;

class NWHDMatcher {
public:
	NWHDMatcher(Configuration& conf);
	virtual ~NWHDMatcher();
	void run();
	static void OnConnect(Socket& client, void *arg);
	void SaveFile(char *DBptr ,int size);
	bool GetBindedStatus(){ return m_portBinded;}
protected :
	bool do_serv_task(Socket& client);
	int CheckMsg(char* inp);
	char* getUpdateIris(char* Buffer,int size,int& msgsize,int& preludeSize);
	char* getDBPrelude(char* Buffer, int size,int& preludeSize, int& dbFileSize,int& start,int& numeyes);
	char* getIris(char* Buffer,int size,int& id,int& taskId,int& irissize,int& preludesize);
	void UpdateIris(char *ptr, int preludeSize, Socket & client);
	void DeleteIris(char *ptr, int preludeSize, Socket & client);

	void Init(char *ptr);
	HDMatcher *m_HDMatcher;
	int m_port;
	unsigned char *m_DB;
	unsigned char *m_CoarseDB;
	unsigned char *m_IrisDB;
	HDMatcherMsg *m_HDMsg;
	BinMessage *m_BinMsg,*m_DBack,*m_Pongack,*m_Matchack,*m_Exitack,*m_Headerack;
	FileMsg *m_DBFileMsg;

private:
	bool m_ShouldIQuit;
	void GetMatch(char *ptr, int preludeSize, Socket & client);
	void GetDB(char *& ptr, int & preludeSize, Socket & client);
	void GetCompleteData(char *db,char *srcptr, int preludeSize, int size, Socket & client);
	void GetCompleteDBData(char *ptr,int preludeSize,int dbFileSize, Socket &client);
	void GeneratePongMsg(void);
	char *m_KeyFilename;
	int m_ID;
	int m_BufferSize;
	bool m_Debug;
	ReaderWriter *m_dbRdr;
	char *m_MatchMsgRxBuffer;
	IrisData *m_IrisData;
	SocketFactory *m_socketFactory;
	char *m_tempDbFileName,*m_dbFileName;
	DBAdapter *m_dbAdapter;
	bool m_portBinded;
};

#endif /* NWHDMATCHER_H_ */
