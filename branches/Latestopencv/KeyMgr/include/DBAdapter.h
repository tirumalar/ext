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

#define DATABASE_FILE "keys.db"

class DBAdapter{
private:
	soci::session *m_sqlConn;
	string m_filename;
public:
	DBAdapter();
	~DBAdapter();
	void CloseConnection(void);
	bool OpenFile(char* filename=DATABASE_FILE);
	bool ReOpenConnection();
	string GetDBFileName();
	int GetCertCount();
	int UpdateData(string host,int indx ,int64_t validity,bool isDevice,string& cert,string& key);
	//int UpdateData(string host,int indx ,int64_t validity,bool isDevice,string& cert,char* key, int keylen);
	int GetData(string& host,int indx ,int64_t& validity,bool& isDevice,string& cert,string& key);
	//int GetData(string host,int& indx ,int64_t& validity,bool& isDevice,string& cert,char* key, int& keylen);
	int DeleteData(int indx);
	int DeleteAllKeys();
};
