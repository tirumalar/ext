
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "HTTPPOSTMsg.h"
#include "MatchProcessor.h"
#include "LEDDispatcher.h"
#include "TestDispatcher.h"
#include "ImageMsgWriter.h"
#include "BiOmega.h"
#include <unistd.h>

extern "C" {
#include "file_manip.h"
}

namespace tut {
struct TestData3 {
	TestData3() {

	}
	~TestData3() {
	}
};
typedef test_group<TestData3, 20> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("File Msg Tests");
}

namespace tut {

	template<>
	template<>
	void testobject::test<1>() {
		set_test_name("Simple flow Test");
		const char fname[] = "./Testout.bin";
		FileMsg fmsg(fname,0);
		fmsg.SetFileSize(1024);
		int buff[300],buff1[300];
		for(int i= 0;i<1024>>3;i++)
		{
			buff[i] = i;
			buff1[i]=0;
		}
		fmsg.Unwrap((char* )buff,1024,1);
		FILE *fp=0;
		fp = fopen(fname,"rb");
		ensure("FIle NULL ",fp);
		ensure_equals("number of read bytes",fread(buff1, 1,1024, fp),1024);
		for(int i= 0;i<1024>>3;i++)
		{
			ensure_equals("read bytes",buff1[i],buff[i]);
		}
		fclose(fp);
		int FileLen = FileSize(fname);
		ensure_equals("File SIZE",FileLen,1024);
	}
	template<>
	template<>
	void testobject::test<2>() {
		set_test_name("Append flow Test");

		const char fname[] = "./Testout1.bin";
		FileMsg fmsg(fname,0);
		fmsg.SetFileSize(1000*4);
		int buff[1000],buff1[1000];
		for(int i= 0;i<1000;i++)
		{
			buff[i] = i;
			buff1[i]= 0;
		}
		fmsg.Unwrap((char* )buff,200*4,1);
		fmsg.Unwrap((char* )&buff[200],200*4,1);
		fmsg.Unwrap((char* )&buff[400],200*4,1);
		fmsg.Unwrap((char* )&buff[600],200*4,1);
		fmsg.Unwrap((char* )&buff[800],200*4,1);
		FILE *fp=0;
		fp = fopen(fname,"rb");
		ensure("File NULL ",fp);
		ensure_equals("number of read bytes",fread(buff1, 1,1000*4, fp),1000*4);
		for(int i= 0;i<1000;i++)
		{
			ensure_equals("read bytes",buff1[i],buff[i]);
		}
		fclose(fp);
		int FileLen = FileSize(fname);
		ensure_equals("File SIZE",FileLen,4000);

	}

	template<>
	template<>
	void testobject::test<3>() {
		set_test_name("Wrap Test");
		printf("Creating Input\n");
		const char fname[] = "./abc.txt";
		FILE *fp = fopen(fname,"w");
		char *Buffer = (char *)malloc(1024*1024);
		char k = 'A';
		for(int i=0;i<1024*1024;i++){
			Buffer[i] = k;
			if (k =='Z'){
				k = '\n';
			}else if (k =='\n'){
				k ='A';
			}else
				k++;
		}
		fwrite(Buffer,1,1024*1024,fp);
		fclose(fp);
		free(Buffer);
		FileMsg fmsg(fname,100);
		int recbytes =0;

		while(recbytes<1024*1024){
			int bytes = 0xC000;
			char *ptr = fmsg.Wrap(bytes);
			//printf("%d :: %s\n",bytes,ptr);
			if(bytes == 0) break;
			recbytes +=bytes;
		}
		int FileLen = FileSize(fname);
		ensure_equals("Transfered size",FileLen+20,recbytes);

	}
}

