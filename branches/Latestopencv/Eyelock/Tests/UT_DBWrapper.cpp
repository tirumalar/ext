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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <string.h>
#include "DBAdapter_Keys.h"

extern int CopyFile (char* Src, char* Dst);

namespace tut {

struct DBWrapperTestData {
	DBWrapperTestData() {
		CopyFile("./data/keys.db","keys.db");
	}
	~DBWrapperTestData() {
		remove("keys.db");
	}
};

typedef test_group<DBWrapperTestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("DBWrapperTestData TESTS");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		DBAdapter_Keys db;
		db.OpenFile("keys.db");
		int cnt = db.GetCertCount();
		ensure("cnt should be two",cnt==2);

		string host("Madhav");
		int indx = -1;
		int64_t validity = 1234567890;
		bool isDevice = true;
		string cert("Certinsert");
		string key("Keyinsert");
		//push data
		int ret = db.UpdateData(host,indx,validity,isDevice,cert,key);
		ensure("inserted",ret==0);

		int indx1 = 3;
		int64_t validity1 = -1;
		bool isDevice1 = false;
		string cert1("123");
		string key1("456");
		string host1("Madhav");
		//read data
		indx = 3;
		ret = db.GetData(host1,indx1,validity1,isDevice1,cert1,key1);
		ensure("read",ret==0);
		ensure("indx",indx1==indx);
		ensure("isDevice",isDevice==isDevice1);
		ensure("validity",validity1==validity);
		ensure("cert",0==memcmp(cert.c_str(),cert1.c_str(),cert.length()));
		ensure("key",0==memcmp(key.c_str(),key1.c_str(),key.length()));
		ensure("host",0==memcmp(host.c_str(),host1.c_str(),host.length()));

		cnt = db.GetCertCount();
		ensure("cnt",cnt==3);
		validity = 0x12345678;
		ret = db.UpdateData(host,indx,validity,isDevice,cert,key);
		ensure("inserted1",ret==1);
		ret = db.GetData(host,indx1,validity1,isDevice1,cert1,key1);
		ensure("read again ",ret==0);
		ensure("validity",validity1==validity);

		cnt = db.GetCertCount();
		ensure("cnt",cnt==3);

		ret = db.DeleteData(indx);
		ensure("deleted",ret==0);
		cnt = db.GetCertCount();
		ensure("cnt",cnt==2);
	 }

}
