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

#include <string.h>
#include "SSLWrap.h"
#include "AESFileEncDec.h"
#include "KeyMgr.h"
#include <unistd.h>

namespace tut {

struct FileEncDecData {
	FileEncDecData() {

	}
	~FileEncDecData() {

	}
};

typedef test_group<FileEncDecData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group1("FileEncDecData TESTS");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		AESFileEncDec t("LetEncryptKey012345678912LetEncryptKey012345678912LetEncryptKey012345678912");
		string inp("v3.01.xxxxxxxxx");
		string out,decout;
		t.EncryptString(inp,out);

		AESFileEncDec t1("LetEncryptKey012345678912LetEncryptKey012345678912LetEncryptKey012345678912");
		t1.DecryptString(out,decout);
//		printf("%d  %s -> out %d %s -> %d %s \n",inp.length(),inp.c_str(),out.length(),out.c_str(),decout.length(),decout.c_str());
		ensure("enc dev key should match",0 == memcmp(decout.c_str(),inp.c_str(),inp.length()));
	}
	template<>
	template<>
	void testobject::test<2>() {
		FILE *fo=fopen("test.bin","wb");
		string inp;
		inp.resize(10);
		memset((void*)inp.c_str(),0xAA,10);
		int outlen = fwrite(inp.c_str(),1, inp.length(),fo);
		fclose(fo);

		int ret1 = AESFileEncDec::EncryptNxt("test.bin","out.bin","v3.01.1234567");
		ensure("enc file size is",(48+16) == ret1);
		int ret = AESFileEncDec::DecryptNxt("out.bin","outdec.bin");

		ensure("enc dev key should match",10 == ret);
		remove("test.bin");
		remove("out.bin");
		remove("outdec.bin");
	}
	template<>
	template<>
	void testobject::test<3>() {
		KeyMgr km;
		FILE *fo=fopen("inp.bin","wb");
		string inp;
		inp.resize(1024);
		memset((void*)inp.c_str(),0x12,1024);
		int outlen = fwrite(inp.c_str(),1, inp.length(),fo);
		fclose(fo);

		int enc = km.EncryptFile("inp.bin","enc.bin","12345678901234567890123456789012");
		ensure("enc file size is",(48+1024+16) == enc);
		int dec = km.DecryptFile("enc.bin","dec.bin");
		ensure("enc dev key should match",1024 == dec);

		remove("inp.bin");
		remove("enc.bin");
		remove("dec.bin");
	}
	template<>
	template<>
	void testobject::test<4>() {
		KeyMgr km;
		FILE *fo=fopen("inp.bin","wb");
		string inp;
		inp.resize(1024);
		memset((void*)inp.c_str(),0x12,1024);
		inp[0] = 0x1f;
		inp[1] = 0x8b;
		int outlen = fwrite(inp.c_str(),1, inp.length(),fo);
		fclose(fo);

		int dec = km.DecryptFile("inp.bin","dec.bin");
		ensure("It was a non encrypted input so made a copy to output",1024 == dec);

		remove("inp.bin");
		remove("dec.bin");
	}
}
