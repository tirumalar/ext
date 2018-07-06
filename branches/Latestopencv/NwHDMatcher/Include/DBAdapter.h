#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include <Synchronization.h>
#include "IrisMacros.h"

using namespace std;
//using namespace soci;
class DBMap;
namespace soci{
	class session;
}

#define DATABASE_FILE "db.db3"

class DBAdapter{
private:
	soci::session *m_sqlConn;
	Mutex m_lock;
	string m_filename;
public:
	DBAdapter();
	~DBAdapter();
	void CloseConnection(void);
	bool OpenFile(char* filename=DATABASE_FILE);
	bool ReOpenConnection();
	string GetDBFileName();
	int GetUserCount(bool excludeDummies = false);
	int UpdateUser(string perid,string username,string leftiris,string rightiris,string acd,int acdlen,string acdnop);
	int UpdateUser(string perid,string username,string leftiris,string rightiris,string acd,int acdlen,string acdnop,string pin);
	int GetUserData(string perid,string& username,string& leftiris,string& rightiris,string& acd,int& acdlen,string& acdnop);
	int GetUserData(string perid,string& username,string& leftiris,string& rightiris,string& acd,int& acdlen,string& acdnop,string& pin);
	int DeleteUser(string perid);
	int MakeMatchBuffer(char *matchBuffer,int alloclen,int count,int startindex=0,bool compressed=false,char *coarseBuffer=NULL);
	int GetUserACDData(string perid,string& acd,int& acdlen, string& acdnop);
	int GetUserACDData(string perid,string& acd,int& acdlen, string& acdnop, string& pin);
	int GetUsername(string perid,string& username);
	vector<pair<int,string> > GetIncrementalData();
	int SetUpdateUser(string perid,int uptype);
	int getAllUserIDs(vector<string>& outIDs);
	int UpdateACS(string display,string acd,int acdlen);
	int UpdateACS(string display,string acd,int acdlen,string parityMask,string dataMask);
	int AddACSMaskColumn();
	bool CheckACSMaskColumn();
	int AddPINColumn();
	bool CheckPINColumn();
	int GetACSData(string& display,string& acd,int& acdlen);
	int GetACSData(string& display,string& acd,int& acdlen,string& parityMask,string& dataMask);
	int ReadDBForCheckingIris(DBMap *dbmap);
	int getAllACD(char **data, int len, int type);
	int getAllACD(char **data, int acdByteLength, int pinByteLength, int nameMaxLength, int type);
	int getACDCount(string card, int type);
	int GetUserDataFromACD(string& perid,string& username,string& leftiris,string& rightiris,string acd,int acdlen, int type);
	int CheckIntegrity();

protected:
	bool CheckColumn(string tableName, string columnName);
	int AddColumn(string tableName, string columnName, int columnType); // columnType: INTEGER - 1, FLOAT - 2, TEXT - 3, BLOB - 4 (refer to sqlite3.h)
};
