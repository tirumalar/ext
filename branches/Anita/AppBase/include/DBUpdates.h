/*
 * DBUpdates.h
 *
 *  Created on: Apr 12, 2013
 *      Author: mamigo
 */

#ifndef DBUPDATES_H_
#define DBUPDATES_H_
#include <vector>
#include "Configurable.h"
#include <utility>
#include <algorithm>
#include <string>
class Configuration;


class DBUpdates {
public:
	DBUpdates(Configuration& conf);
	virtual ~DBUpdates();
	void DBRead(char* dbFileName);
	bool ReadIncrementalRecords(char *filename);
	unsigned char *GetGuidFromIncrement(int index);
	bool CheckIncrementalRecordsWithDB();
	void AddUpdateTheRecords();
	bool Compare2DB(void);
	bool ReadDecrementalRecords(char *filename);
	bool CheckDecrementalRecordsWithDB();
	void WriteDBHeader(char *fname,short numrec, short numeyes, int perm,unsigned short irissz,unsigned char metadata,unsigned short filever,unsigned char idsz,unsigned short pwdindx);
	bool DeleteTheRecords();
	bool DeleteInDB(void);
	bool UpdateInDB(void);
	void CreateDummyIris(unsigned char *Output,IrisDBHeader& irisDBHeader);
	void ExtractRecords();
private:
	std::vector< std::string > m_GUID;
	std::vector< std::pair< int, std::string > > m_addOrRemoveGUID;

	const char *m_dbFileName;
	char *m_temp1DbFileName,*m_tempDbFileName;
	unsigned char *m_tempBuffer;
	bool m_Debug;
};

#endif /* DBUPDATES_H_ */
