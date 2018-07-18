/*
 * DBMap.h
 *
 *  Created on: Feb 14, 2015
 *      Author: developer
 */

#ifndef DBMAP_H_
#define DBMAP_H_
#include <map>
#include <utility>
#include <string>
using namespace std;

enum PERSON_INFO{DUAL=0,SINGLE};

class DBMapElement {
public:
	DBMapElement();
	virtual ~DBMapElement();
	void Update(string& guid,string& name,string& acd,int& acdlen,PERSON_INFO& per);
	void Update(string& guid,string& name,string& acd,int& acdlen,const string& pin,PERSON_INFO& per);
	void GetAcdData(string &guid,string& name,string& acddata,int& acdlen,PERSON_INFO& per);
	void GetAcdData(string &guid,string& name,string& acddata,int& acdlen,string& pin,PERSON_INFO& per);
private:
	string m_guid;
	string m_name;
	string m_acddata;
	int m_acdlength;
	string m_pin;
	PERSON_INFO m_personInfo;
};

class DBMap {
public:
	DBMap();
	virtual ~DBMap();
	PERSON_INFO GetPersonInfo(string& leftiris,string& rightiris);
	bool Insert(string& guid,string& leftiris,string& rightiris,string& name,string& acd, int& acdlen);
	bool Insert(string& guid,string& leftiris,string& rightiris,string& name,string& acd, int& acdlen, const string& pin);
	bool GetAcdData(string &guid,string& name,string& acddata, int& acdlen,PERSON_INFO& per);
	bool GetAcdData(string &guid,string& name,string& acddata, int& acdlen,string& pin,PERSON_INFO& per);
	bool GetPin(string &guid,string& name,string& acddata, int& acdlen,PERSON_INFO& per);
	void Clear();
	int GetSize();
	int Erase(string &guid);
private:
	bool Insert(string& guid,string& name,string& acd, int& acdlen,PERSON_INFO& per);
	bool Insert(string& guid,string& name,string& acd, int& acdlen,const string& pin, PERSON_INFO& per);
	map<string,DBMapElement> m_map;
};


#endif /* DBMAP_H_ */
