/*
 * DBMap.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: developer
 */

#include "DBMap.h"
#include <string.h>

DBMapElement::DBMapElement():m_personInfo(DUAL){
}
DBMapElement::~DBMapElement(){
}

void DBMapElement::Update(string& guid,string& name,string& acd,int& acdlen,PERSON_INFO& per){
	m_guid = guid;
	m_name = name;
	m_acddata.resize(acd.length());
	m_acdlength = acdlen;
	memcpy((void*)m_acddata.c_str(),(void*)acd.c_str(),acd.length());
	m_personInfo = per;
}

void DBMapElement::Update(string& guid,string& name,string& acd,int& acdlen,const string& pin,PERSON_INFO& per){
	Update(guid, name, acd, acdlen, per);
	m_pin = pin;
}

void DBMapElement::GetAcdData(string &guid,string& name,string& acddata,int& acdlen,PERSON_INFO& per){
	guid = m_guid;
	name = m_name;
	acdlen = m_acdlength;
	acddata.resize(m_acddata.length());
	memcpy((void*)acddata.c_str(),(void*)m_acddata.c_str(),m_acddata.length());
	per = m_personInfo;
}

void DBMapElement::GetAcdData(string &guid,string& name,string& acddata,int& acdlen,string& pin,PERSON_INFO& per){
	GetAcdData(guid, name, acddata, acdlen, per);
	pin = m_pin;
}

DBMap::DBMap() {
	// TODO Auto-generated constructor stub
}

DBMap::~DBMap() {
	// TODO Auto-generated destructor stub
}

PERSON_INFO DBMap::GetPersonInfo(string& leftiris,string& rightiris){
	PERSON_INFO ret = DUAL;
	if(0 == memcmp(leftiris.c_str(),rightiris.c_str(),(leftiris.length()+rightiris.length())>>1)){
		ret=SINGLE;
	}
	return ret;
}

bool DBMap::Insert(string& guid,string& leftiris,string& rightiris,string& name,string& acd, int& acdlen){
	PERSON_INFO ret = GetPersonInfo(leftiris,rightiris);
	return Insert(guid,name,acd,acdlen,ret);
}

bool DBMap::Insert(string& guid,string& leftiris,string& rightiris,string& name,string& acd, int& acdlen, const string& pin){
	PERSON_INFO ret = GetPersonInfo(leftiris,rightiris);
	return Insert(guid,name,acd,acdlen,pin,ret);
}

bool DBMap::Insert(string& guid,string& name,string& acd, int& acdlen,PERSON_INFO& per){
	string pin;
	return Insert(guid, name, acd, acdlen, pin, per);
}

bool DBMap::Insert(string& guid,string& name,string& acd, int& acdlen, const string& pin, PERSON_INFO& per){
	DBMapElement local;
	string name_acd;
	int bytes = (acdlen+7)>>3;
	name_acd.resize(bytes+2);
	memset((void*)name_acd.c_str(),0,bytes+2);
	memcpy((void*)(name_acd.c_str()),&acdlen,2);
	memcpy((void*)(name_acd.c_str()+2),acd.c_str(),bytes);
	local.Update(guid,name,name_acd,acdlen,pin,per);
	m_map[guid] = local;
	return true;
}

bool DBMap::GetAcdData(string &guid,string& name,string& acddata,int& acdlen,string& pin,PERSON_INFO& per){
	bool ret = false;
	map<string,DBMapElement >::iterator it;
	it=m_map.find(guid);
	if(it != m_map.end()){
		DBMapElement local = (*it).second;
		local.GetAcdData(guid,name,acddata,acdlen,pin,per);
		ret = true;
	}
	return ret;
}

bool DBMap::GetAcdData(string &guid,string& name,string& acddata,int& acdlen,PERSON_INFO& per){
	string pin;
	return GetAcdData(guid,name,acddata,acdlen,pin,per);
}

void DBMap::Clear(){
	m_map.clear();
}

int DBMap::GetSize(){
	return m_map.size();
}

int DBMap::Erase(string &guid){
	return m_map.erase(guid);
}
