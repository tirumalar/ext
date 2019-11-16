#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <string.h>
#include <tut/tut.hpp>

#include "stdio.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "FileRW.h"
#include "stdlib.h"
#include "iostream"
#include "BiOmega.h"

#include "DBAdapter.h"

#include "IrisDBHeader.h"
#include "AesRW.h"
#include "PermRW.h"
#include <unistd.h>

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}
void GenerateGuid(char *guidStr)
{
	char *pGuidStr = guidStr;
	int i;
	timeval mt;
	gettimeofday(&mt,0);
	srand(mt.tv_usec);  /*Randomize based on time.*/

	/*Data1 - 8 characters.*/
	for(i = 0; i < 8; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;

	/*Data2 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;

	/*Data3 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;

	/*Data4 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;

	/*Data5 - 12 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 12; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
}

int CopyFile (char* Src, char* Dst){
	int read_fd;
	int write_fd;
	struct stat stat_buf;
	off_t offset = 0;
	/* Open the input file. */
	read_fd = open (Src, O_RDONLY);
	/* Stat the input file to obtain its size. */
	fstat (read_fd, &stat_buf);
	/* Open the output file for writing, with the same permissions as the
	source file. */
	write_fd = open (Dst, O_WRONLY | O_CREAT, stat_buf.st_mode);
	/* Blast the bytes from one file to the other. */
	sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
	/* Close up. */
	close (read_fd);
	close (write_fd);
	return 0;
}


namespace tut {
struct EyelockDBData {
	EyelockDBData(){
		int	rem = remove("db.db3");
		printf("remove database = %d\n",rem);
		CopyFile("./data/UpdateSqlite.db3","db.db3");
		sleep(1);
	}


	~EyelockDBData() {
		int	rem = remove("db.db3");
	}

};
typedef test_group<EyelockDBData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("EyelockDBData Test");
}
namespace tut {
	template<>
	template<>
	void testobject::test<1>()
	{
		DBAdapter* db = new DBAdapter();
		db->OpenFile();
		int ret = db->GetUserCount();
		ensure("User count should be 0",ret == 0);
		string perid("123456");
		string username("JUNK"),usr("Maddy");
		string leftiris("madhav Shanbhag left iris");
		string rightiris("madhav Shanbhag right iris");
		string acd("ACD DATA");
		string acdnop;
		ret = db->UpdateUser(perid,username,leftiris,rightiris,acd,22,acdnop);
		ensure("Inserted in db successfully",ret == 0);
		ret = db->UpdateUser(perid,usr,leftiris,rightiris,acd,5,acdnop);
		ensure("Inserted in db as already present",ret == 1);

		//string perid("123456");
		string rusername,rleftiris,rrightiris,racd;
		int len;
		ret = db->GetUserData(perid,rusername,rleftiris,rrightiris,racd,len,acdnop);
//		printf("Got %s %s %s %s %s %d \n",perid.c_str(),rusername.c_str(),rleftiris.c_str(),rrightiris.c_str(),racd.c_str(),len);
		ensure("acdlen should match",len == 5);

		ret = db->GetUserACDData(perid,racd,len,acdnop);
		ensure("retval should be 0",ret == 0);
		ensure("acdlen should match",len == 5);
		ensure("acd content should match",0 == memcmp(racd.c_str(),acd.c_str(),acd.length()));

		ret = db->GetUserCount();
		ensure("User count should be 1",ret == 1);

		ret = db->DeleteUser(perid);
		ensure("Deleted in db as already present",ret == 0);

		ret = db->GetUserCount();
		ensure("User count should be 0",ret == 0);

		delete db;
	}
	template<>
	template<>
	void testobject::test<2>()
	{
		DBAdapter* db = new DBAdapter();
		db->OpenFile();
		int ret = db->GetUserCount();
		ensure("User count should be 0",ret == 0);
		string perid;
		string username;
		string leftiris;
		string rightiris;
		string acd, acdnop;
		perid.resize(GUID_SIZE);
		username.resize(GUID_SIZE);
		leftiris.resize(IRIS_SIZE_INCLUDING_MASK);
		rightiris.resize(IRIS_SIZE_INCLUDING_MASK);
		acd.resize(GUID_SIZE);
		int k=0;
		char abc[]={'a','b','c','d','e','f','g','h','i','j'};
		for(int i=0;i<10;i++){
			memset((void*)perid.c_str(),abc[i],perid.length());
			memset((void*)username.c_str(),k,username.length());k++;
			memset((void*)leftiris.c_str(),k,leftiris.length());k++;
			memset((void*)rightiris.c_str(),k,rightiris.length());k++;
			memset((void*)acd.c_str(),k,acd.length());k++;
			ret = db->UpdateUser(perid,username,leftiris,rightiris,acd,acd.length(),acdnop);
		}
		ret = db->GetUserCount();
		ensure("User count should be 10",ret == 10);

		string matchBuffer1;
		int req = 10*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE);
		matchBuffer1.resize(req);
		memset((void*)matchBuffer1.c_str(),0,req);
		string matchBuffer2;
		matchBuffer2.resize(req);
		memset((void*)matchBuffer2.c_str(),0,req);

		ret = db->MakeMatchBuffer((char*)matchBuffer1.c_str(),matchBuffer1.length(),5,0);
		ensure("ret should be 0",ret == 0);
		int val = 1;
		for(int i=0;i<5;i++){
			char *ptr = (char*)(matchBuffer1.c_str())+i*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE)+IRIS_SIZE_INCLUDING_MASK_PER_PERSON;
			for(int k=0;k<GUID_SIZE;k++){
				ensure("perid match1 ",abc[i]==*ptr++);
			}
			ptr = (char*)(matchBuffer1.c_str())+i*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE);
			for(int k=0;k<IRIS_SIZE_INCLUDING_MASK;k++){
				ensure("left iris ",val==*ptr++);
			}
			val +=1;
			for(int k=0;k<IRIS_SIZE_INCLUDING_MASK;k++){
				ensure("right iris ",val==*ptr++);
			}
			val +=3;
		}
		delete db;

		db = new DBAdapter();
		db->OpenFile();
		ret = db->MakeMatchBuffer((char*)matchBuffer2.c_str(),matchBuffer2.length(),5,5);
		ensure("ret should be 0",ret == 0);
		for(int i=0;i<5;i++){
			char *ptr = (char*)(matchBuffer2.c_str())+i*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE)+IRIS_SIZE_INCLUDING_MASK_PER_PERSON;
			for(int k=0;k<GUID_SIZE;k++){
				ensure("perid match2 ",abc[i+5]==*ptr++);
			}
		}
		delete db;
	}


	template<>
	template<>
	void testobject::test<3>()
	{
		FILE *fnamepwd = fopen("../Eyelock/data/Eyes/usr_pwd.txt","r");
		if(fnamepwd == NULL){
			printf("User Password File Missing %s\n","../Eyelock/data/Eyes/usr_pwd.txt");
			return ;
		}
		int dataBaseCnt=10;
		BiOmega *bioInstance = new BiOmega;
		bioInstance->SetPupilRadiusSearchRange(20, 90);
	    bioInstance->SetIrisRadiusSearchRange(80, 140);
	    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);
		int irisCodePos=0;

		remove("TestDB.db3");
		rename("db.db3","TestDB.db3");
		DBAdapter* db = new DBAdapter();
		db->OpenFile("TestDB.db3");
		int ret = db->GetUserCount();
		ensure("User count should be 0",ret == 0);
		string perid;
		string leftiris;
		string rightiris;
		perid.resize(GUID_SIZE);
		leftiris.resize(IRIS_SIZE_INCLUDING_MASK);
		rightiris.resize(IRIS_SIZE_INCLUDING_MASK);


		ret = db->GetUserCount();
		ensure("User count should be 10",ret == 0);
		char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";

		unsigned char irisCode[7000];
		unsigned char data[640000];
		for(int i=0; i<dataBaseCnt; i+=2){
			for(int j=0;j<2;j++){
				int w, h;
				char imgName[100];
				sprintf(imgName,inputpath,i+j);
//				printf("\n--------%s------------\n", imgName);
				int status = ReadPGM5(imgName, data, &w, &h,640000);
				if(w == 640 && h == 480 && status >=0){
					bool retVal=false;
					XTIME_OP("GetIrisCode",
							retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(irisCode+irisCodePos))
					);
					irisCodePos+=bioInstance->GetFeatureLength();
					if(retVal){
//						printf("SUCCESS ");
					}
					else{
						printf("FAIL ");
					}
				}else{
					printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
				}
			}
			irisCodePos=0;
//			printf("\nUSER PWD :");
			char buff[128] ={0};
			fgets (buff,128,fnamepwd);
			string username(buff);
			string acd(username),acdnop;
			GenerateGuid((char*)perid.c_str());
			memcpy((void*)leftiris.c_str(),(void*)irisCode,leftiris.length());
			memcpy((void*)rightiris.c_str(),(void*)(irisCode+IRIS_SIZE_INCLUDING_MASK),rightiris.length());
//			printf("inserting %s \n",perid.c_str());
			ret = db->UpdateUser(perid,username,leftiris,rightiris,acd,acd.length()*8,acdnop);
			ensure("Insert person success",0 == ret);
			ret = db->GetUserCount();
			ensure("User count should match",1+(i/2) == ret);
		}
		db->CloseConnection();
		remove("TestDB.db3");
	}
	template<>
	template<>
	void testobject::test<4>()
	{
		unsigned char m_dbbuff[50*6*1024];
		char *buff = "./data/Mamigo.bin";
		IrisDBHeader irishdr;
		irishdr.ReadHeader(buff);
		irishdr.PrintAll();
		ReaderWriter *perRdr = (ReaderWriter *) new PermRW(new AesRW(new FileRW(buff,irishdr.GetHeaderSize())));
		perRdr->Init(&irishdr);
		for(int l=0;l<irishdr.GetNumRecord();l++){
			perRdr->Read(m_dbbuff+l*irishdr.GetOneRecSizeinDB(),irishdr.GetOneRecSizeinDBFile(),l*irishdr.GetOneRecSizeinDBFile());
			unsigned char *ptr = m_dbbuff+l*irishdr.GetOneRecSizeinDB() +IRIS_SIZE_INCLUDING_MASK*2+4;
//			printf("F2F for %d \n",l);
//			for(int j=0;j<irishdr.GetF2FSize();j++){
//				printf("%02x",*ptr++);
//			}
//			printf("\n");
			ptr = m_dbbuff+l*irishdr.GetOneRecSizeinDB()+IRIS_SIZE_INCLUDING_MASK*2+4;
//			printf("F2F for %d \n",l);
//			for(int j=0;j<irishdr.GetF2FSize();j++){
//				printf("%c",(char)(*ptr++));
//			}
//			printf("\n");
		}
		delete perRdr;
		remove("Mamigo.db3");
		rename("db.db3","Mamigo.db3");
		DBAdapter* db = new DBAdapter();
		db->OpenFile("Mamigo.db3");
		int ret = db->GetUserCount();
		ensure("User count should be 0",ret == 0);
		string perid;
		string leftiris;
		string rightiris;
		perid.resize(GUID_SIZE);
		leftiris.resize(IRIS_SIZE_INCLUDING_MASK);
		rightiris.resize(IRIS_SIZE_INCLUDING_MASK);
		ret = db->GetUserCount();
		ensure("User count should be 10",ret == 0);
		for(int i=0;i<irishdr.GetNumRecord();i++){

			GenerateGuid((char*)perid.c_str());
			unsigned char *ptr = m_dbbuff+i*irishdr.GetOneRecSizeinDB();
			memcpy((void*)leftiris.c_str(),(void*)ptr,leftiris.length());
			memcpy((void*)rightiris.c_str(),(void*)(ptr+IRIS_SIZE_INCLUDING_MASK),rightiris.length());

			ptr = m_dbbuff+i*irishdr.GetOneRecSizeinDB()+4+IRIS_SIZE_INCLUDING_MASK*2+2;
			string username((char*)ptr);
			string acd(username), acdnop;
//			printf("inserting %s %s \n",username.c_str(),perid.c_str());
			ret = db->UpdateUser(perid,username,leftiris,rightiris,acd,acd.length()*8,acdnop);
			ensure("Insert person success",0 == ret);
			ret = db->GetUserCount();
			ensure("User count should match",(1+i) == ret);
		}
		db->CloseConnection();
		delete db;
		remove("Mamigo.db3");
	}

	template<>
	template<>
	void testobject::test<5>()
	{
		system("cp --preserve=all ./data/sqlite.db3 ./data/testing.db3");
		system("cp --preserve=all ./data/UpdateSqlite.db3 ./data/update.db3");
		DBAdapter* db = new DBAdapter();
		db->OpenFile("./data/testing.db3");
		int cnt = db->GetUserCount();
		char * guid[]={
			"04963E38-6DD7-56E3-69AE-A42DE62F6666",
			"3239C49A-7C25-C40C-6503-CA943377E5A1",
			"77A5A664-401F-AE27-9430-53B3F7A586AF",
			"87F98B02-2F40-BA6C-942B-4C4E43A6618E",
			"188C3E54-11E5-5CFC-4BC9-93E59E04CAED",
			"70A341C2-DC76-3C3D-BC37-7DE53C405A5C",
			"F3C29A9B-C3D0-662F-F708-19C38C868FE8",
		};
		string username,leftiris,rightiris,acd,acdnop;
		int acdlen;
		DBAdapter inc;
		inc.OpenFile("./data/update.db3");
		for(int i=0;i<7;i++){
			string gui(guid[i]);
			int ret = db->GetUserData(gui,username,leftiris,rightiris,acd,acdlen,acdnop);
			ensure("Read data ",0 == ret);
			ret = inc.UpdateUser(gui,username,leftiris,rightiris,acd,acdlen,acdnop);
			ensure("written data ",0 == ret);
			ret = inc.SetUpdateUser(gui,1);
			ensure("Updated Type ",0 == ret);
		}
		vector<pair<int,string> > retd = inc.GetIncrementalData();
		ensure("The Count should remain same",retd.size() == 7);
		for(int i=0;i<7;i++){
			ensure("Update type should match",retd[i].first == 1);
		}

		db->CloseConnection();
		delete db;
		remove("./data/update.db3");
	}

	template<>
	template<>
	void testobject::test<6>()
	{
		DBAdapter* db = new DBAdapter();
		db->OpenFile("./data/sqlite.db3");
		int cnt = db->GetUserCount();

		vector<string> retIDs;
		int ret = db->getAllUserIDs(retIDs);
		ensure("The Count should remain same",retIDs.size() == cnt);
		ensure("Update type should match",retIDs[0].compare("972E6E6C-D9DA-335E-1601-C3424E98362C") == 0);

		db->CloseConnection();
		delete db;
	}



}
