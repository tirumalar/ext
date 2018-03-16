#pragma once
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
using namespace std;

namespace soci{
	class session;
}
#ifdef HBOX_PG
#define DATABASE_FILE "keys.db3"
#else
#define DATABASE_FILE "keys.db"
#endif
class DBAdapter_Keys{
private:
	soci::session *m_sqlConn;
	string m_filename;
public:
	DBAdapter_Keys();
	~DBAdapter_Keys();
	void CloseConnection(void);
	bool OpenFile(char* filename=DATABASE_FILE);
	bool ReOpenConnection();
	string GetDBFileName();
	int GetCertCount();
	int UpdateData(string host,int indx ,int64_t validity,bool isDevice,string& cert,string& key);
	//int UpdateData(string host,int indx ,int64_t validity,bool isDevice,string& cert,char* key, int keylen);
	int GetData(string host,int &indx ,int64_t& validity,bool& isDevice,string& cert,string& key);
	//int GetData(string host,int& indx ,int64_t& validity,bool& isDevice,string& cert,char* key, int& keylen);
	int DeleteData(int indx);
	int ReadDB(vector<pair<string,int64_t> >& vec);
};
