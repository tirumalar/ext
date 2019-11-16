/*
 * UT_Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <string.h>
#include "DBMap.h"

namespace tut {

struct DBMapData {
	DBMapData() {

	}
	~DBMapData() {

	}
};

typedef test_group<DBMapData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("DBMap TESTS");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
		DBMap db;
		string guid("04963E38-6DD7-56E3-69AE-A42DE62F6666");
		string name("testing name");
		string acd("abcdefghijklmnopqrstuvwxyz");
		string iris("Same iris in both");
		PERSON_INFO p=DUAL;
		int acdlen = 96;
		bool ret = db.Insert(guid,iris,iris,name,acd,acdlen);
		ensure("Inserted to map",ret);

		string resname,resacd;
		PERSON_INFO p1=SINGLE;
		int len=-1;
		bool ret1=db.GetAcdData(guid,resname,resacd,len,p1);
		ensure("Inserted to map",ret1);
		ensure("Name should match",0==memcmp((void*)resname.c_str(),(void*)name.c_str(),resname.length()));
		ensure("acd data should match",0==memcmp((void*)(resacd.c_str()+2),(void*)acd.c_str(),12));
		ensure("acd length should match",len == acdlen);
		ensure("INFO", p1 == SINGLE);
	}
template<>
template<>
void testobject::test<2>() {
	DBMap db;
	string guid("04963E38-6DD7-56E3-69AE-A42DE62F6666");
	string name("testing name");
	string acd("abcdefghijklmnopqrstuvwxyz");
	string iris("Same iris in both");
	int acdlen = 96;
	PERSON_INFO p=DUAL;
	bool ret = db.Insert(guid,iris,iris,name,acd,acdlen);
	ensure("Inserted to map",ret);

	string name1("overwrite testing name");
	string acd1("123456789012abcdefghijklmnopqrstuvwxyz");
	acdlen = 104;
	ret = db.Insert(guid,iris,name1,name1,acd1,acdlen);
	ensure("Inserted to map",ret);

	string resname,resacd;
	int len= -1;
	bool ret1=db.GetAcdData(guid,resname,resacd,len,p);
	ensure("Inserted to map",ret1);
	ensure("Name should match",0==memcmp((void*)resname.c_str(),(void*)name1.c_str(),resname.length()));
	ensure("acd data should match",0==memcmp((void*)(resacd.c_str()+2),(void*)acd1.c_str(),13));
	ensure("acd length should match",len == acdlen);
	ensure("INFO", p == DUAL);
	}

template<>
template<>
void testobject::test<3>() {
		DBMap db;
		string guid("04963E38-6DD7-56E3-69AE-A42DE62F6666");
		string name("testing name");
		string acd("abcdefghijklmnopqrstuvwxyz");
		string iris("Same iris in both");
		PERSON_INFO p=DUAL;
		int acdlen = 96;
		bool ret1 = db.Insert(guid,iris,iris,name,acd,acdlen);
		ensure("Inserted to map",ret1);

		int ret = db.GetSize();
		ensure("Size of map",ret==1);
		ret = db.Erase(guid);
		ensure("Erase element from map",ret==1);
		ret = db.GetSize();
		ensure("Size of map should be 0",ret==0);
		ret1 = db.Insert(guid,iris,iris,name,acd,acdlen);
		ensure("Inserted to map",ret1);
		db.Clear();
		ret = db.GetSize();
		ensure("Erase all element from map",ret==0);
	}
}
