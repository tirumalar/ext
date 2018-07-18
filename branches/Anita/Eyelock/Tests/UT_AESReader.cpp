/*
 * UT_AESReader.cpp
 *
 *  Created on: 07-Apr-2010
 *      Author: akhil
 */
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "stdio.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "FileRW.h"
#include "stdlib.h"
#include "iostream"
#include "BiOmega.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "IrisDBHeader.h"
#include "DBReceive.h"
#include "PermuteServer.h"
#include "PermRW.h"
#include <unistd.h>

extern int argc;
extern char **argv;

#define FILEVERSION 2
#if(FILEVERSION == 1)
#define IVSIZE 32
#else
#define IVSIZE 16
#endif
#define KEYSIZE 32

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}
extern int seqindx;

namespace tut {
void WriteHeader(char*fname,short numrec, short numeyes, bool Noperm=false,int irissz=1280,int filever=2,int idsz= 14 ){
	FILE *fp = fopen(fname,"wb");
	unsigned char magicd = 'd';
	unsigned char magic1 = '1';
	fwrite(&magicd,1,1,fp); //Magic
	fwrite(&magic1,1,1,fp); //Magic
	unsigned short filver = filever;
	fwrite(&filver,1,2,fp); // file ver
	unsigned short iriscodesize = irissz;
	fwrite(&iriscodesize,1,2,fp); //iris code size
	fwrite(&numrec,1,2,fp); // num record
	fwrite(&numeyes,1,2,fp); // rec size
	unsigned char idsize=idsz;
	fwrite(&idsize,1,1,fp); // id size
	unsigned char metadata= 0x2;
	fwrite(&metadata,1,1,fp); // metadata size
	unsigned short pwdindx = 0x0;
	fwrite(&pwdindx,1,2,fp); // pwd indx
	unsigned int key = 0x12345678;
	if(Noperm) key=0x0;
	fwrite(&key,1,4,fp); // permutation key
	fclose(fp);
}

void WriteIris(char * fname,unsigned char *iris,int sz)
{
	static int i=0;
	i++;
	int idsize = 14;
	unsigned int key = 0x12345678;
	int metadata = 2;
	int iriscodesize = 1280;
	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
	FILE *fp = fopen(fname,"a");
	fwrite(&Key,1,IVSIZE,fp); // 32 byte key
	unsigned char irisCode[7000];
	int irisCodePos =0;
	irisCode[irisCodePos++] = 2;
	irisCode[irisCodePos++] = 0;
	irisCode[irisCodePos++] = 0;
	irisCode[irisCodePos++] = 0;

	memcpy(irisCode+4,iris,sz);
	irisCodePos += sz;

	for(int l=0;l < 2;l++){
		irisCode[irisCodePos++]= 96;
	}
	if(idsize>2){
		irisCode[irisCodePos-1]= 0;
	}
	for(int l=2;l < idsize;l++){
		irisCode[irisCodePos++]= i;
	}
	//Meta1
	for(int l=0;l<metadata;l++){
		irisCode[irisCodePos++]= 0;
	}
	//Meta2
	for(int l=0;l<metadata;l++){
		irisCode[irisCodePos++]= 0;
	}

	int EncSz = ((2+2+sz +idsize+metadata*2  +15)>>4)<<4;

	int garbagesz= EncSz - irisCodePos;
	for(int j = 0;j< garbagesz;j++){
		irisCode[irisCodePos++]= 0;
	}

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);

	unsigned char Prem[8000],Enc[8000];
	memcpy(Prem,irisCode,4);
	m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
	m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
	int frmf2f = EncSz - iriscodesize*4+4;
	if (frmf2f<0) frmf2f = 0;
	memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);
	AES Aes;
	Aes.SetIV(Key,IVSIZE);
	Aes.SetKey(Key,KEYSIZE);
	Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

	unsigned int recsize = EncSz;
	fwrite(&recsize,1,4,fp); // Enc rec size
	fwrite(&Enc,1,EncSz,fp);
	fclose(fp);
}

unsigned char databuff [640000];

void WriteDb(const char * fname,BiOmega *bioInstance,const char *Key,const char *inputpath,int dataBaseCnt, int* order=0, bool debug = false);
void WriteDb(const char * fname,BiOmega *bioInstance,const char *Key,const char *inputpath,int dataBaseCnt, int* order,bool debug)
{
	unsigned char *data = databuff;

	FILE *fp = fopen(fname,"a");
	unsigned char Enc[6000];
	int iriscodesize = 1280;
	int metadata = 2;
	unsigned int key = 0x12345678;
	int idsize = 14;
	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);

	int irisCodePos=0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;

		for(int j=0;j<2;j++){

			int w, h;
			char imgName[100];
			int k = i+j;
			if(order){
				k = order[i+j];
			}
			sprintf(imgName,inputpath,k);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName,data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(irisCode+irisCodePos))
				);
				irisCodePos+=bioInstance->GetFeatureLength();
				if(retVal){
//					printf("SUCCESS\n");
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("F2F:");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 48;
		}
		if(idsize>2){
			irisCode[irisCodePos-1]= 0;
		}

		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= i;
//			printf("%d ",i);
		}
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}

		int EncSz = ((2+2+bioInstance->GetFeatureLength()*2 + idsize+metadata*2  +15)>>4)<<4; //to get 5168

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4);
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

		unsigned char buff0[2560],buff1[2560];
		{
			m_PermServer->Recover(Prem+4,buff0,buff1);
			ensure_equals("REcovery1",0,memcmp(buff0,irisCode+4,iriscodesize));
			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
		}

		{
			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
		}

		AES Aes;
		Aes.SetIV((const unsigned char *)Key,IVSIZE);
		Aes.SetKey((const unsigned char *)Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		unsigned int recsize = EncSz;
//		printf("Rec Sz %d\n",EncSz);
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		if(debug){
			FILE *filew = fopen("mnt/mmc/Result.txt","a+");
			int val =i;
			if(order) val = order[i];
			fprintf(filew,"%d Enc Size %d \n",val,EncSz);
			for(int cnt=0;cnt<EncSz;cnt++){
				if((cnt%64) == 0) fprintf(filew,"\n");
				fprintf(filew,"%#02x ",Enc[cnt]);
			}
			fprintf(filew,"\n");
			fclose(filew);
		}
	}
	fclose(fp);
	return;
}


}
namespace tut {
struct AESTestData {
	AesRW *AesRdr;
	AESTestData():AesRdr(0){
	}
	~AESTestData(){
		if(AesRdr)
		{
			delete AesRdr;
			AesRdr=0;
		}

	}
	TestConfiguration cfg;//empty configuration
};
typedef test_group<AESTestData> tg;
typedef tg::object testobject;
}

namespace {
tut::tg test_group("AES Test CASES");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {

	set_test_name("FILE Reader Test");
	char *fname= "../Eyelock/data/Eyes/Reader.txt";
	unsigned char *buff0 = (unsigned char*)calloc(1024,1);
	unsigned char *buff1 = (unsigned char*)calloc(1024,1);
	unsigned char *buff = (unsigned char*)calloc(1024,1);
	for(int i =0; i<1024;i++){
		buff0[i] = rand();
	}
//Create File
	FILE *fp= fopen(fname,"wb");
	ensure("Unable to Create file ",(fp!=NULL));
	int retval = fwrite(buff0,1,1024,fp);
//	printf("retval %d\n",retval);
	ensure("Unable to Write to file ",(retval==1024));
	fclose(fp);
//Now Read it back
	fp= fopen(fname,"rb");
	ensure("Unable to Open file for reading ",(fp!=NULL));
	retval=fread(buff1,1,1024,fp);
//	printf("retval %d\n",retval);
	ensure("Unable to read from file ",(retval==1024));
	ensure("Read File not the same ",(0 == memcmp(buff0,buff1,1024)));
	fclose(fp);

	FileRW FileRdr(fname);
	retval= FileRdr.Read(buff,1024,0);
//	printf("retval %d\n",retval);
	ensure("Unable to read from fileReader ",(retval==1024));
	ensure("Read ALL will FileReader not same ",(0 == memcmp(buff,buff1,1024)));

	retval = FileRdr.Read(buff,1,1000);
//	printf("buff[0] %d buff0[1000] %d\n",buff[0],buff0[1000]);
//	printf("retval %d\n",retval);
	ensure("Unable to read from fileReader ",(retval==1));
	ensure("Read 1000th loc will FileReader not same ",buff0[1000] == buff[0]);

	retval = FileRdr.Read(buff,20,67);

//	printf("retval %d\n",retval);
	ensure("Unable to read from fileReader ",(retval==20));
	ensure("Read 67th loc will FileReader not same ",(0 == memcmp(buff,&buff0[67],20)));


	free(buff0);
	free(buff1);
	free(buff);
	remove(fname);
}

template<>
template<>
void testobject::test<2>() {
	set_test_name("Read DB Header Test");
	char *fname = "../Eyelock/data/Eyes/dbheader.txt";
	WriteHeader(fname,10,10);

	FILE *fp = fopen(fname,"a");

	char Buff[32];
	int sz = 5168;
	fwrite(&Buff,1,IVSIZE,fp); // 32 byte key
	fwrite(&sz,1,4,fp); // permutation key
	fclose(fp);
	IrisDBHeader rptr;
	rptr.ReadHeader(fname);
	rptr.PrintAll();

	ensure_equals("Magic",rptr.GetMagic(),0x6431);
	ensure_equals("FilVer",rptr.GetFileVersion(),FILEVERSION);
	ensure_equals("IrisSize",rptr.GetIrisSize(),1280);
	ensure_equals("NumRec",rptr.GetNumRecord(),10);
	ensure_equals("IdSize",rptr.GetF2FSize(),14);
	ensure_equals("MetaSize",rptr.GetMetaDataSize(),2);
	ensure_equals("RecEncSize",rptr.GetOneRecSizeinDBFile(),sz+IVSIZE+4);
	ensure_equals("PWDIndx",rptr.GetPasswordIndex(),0);
	ensure_equals("PermKey",rptr.GetPermutationKey(),0x12345678);
}

template<>
template<>
void testobject::test<3>() {
	set_test_name("AES Test");
	char *key="../Eyelock/data/Eyes/Test.bin";
	char *salt ="12345 54321";

	AES Aes;
	Aes.GenerateKey((unsigned char *)salt,(unsigned char *)key,strlen(key));

	char *Str = "I don't even know if it will work or not, Lets try";
	unsigned char Enc[1024]={0};
	unsigned char Dec[1024]={0};
	Aes.Encrypt((const unsigned char *)Str,Enc,16);
	Aes.Decrypt(Enc,Dec,16);

	ensure("Encryption and Decryption works",(0==memcmp(Str,Dec,16)));
	remove(key);
	}

template<>
template<>
void testobject::test<4>() {
	set_test_name("AES Test");
	char *key="USETHISFORENCRYPTION";
	char *salt ="12345 54321";

	AES Aes;
	Aes.GenerateKey((unsigned char *)salt,(unsigned char *)key,strlen(key));

	AES Aes1;
	Aes1.SetKey(Aes.m_Key32,32);
	Aes1.SetIV(Aes.m_IV32,32);

	char *Str = "I don't even know if it will work or not, Lets try";
	unsigned char Enc[1024]={0};
	unsigned char Dec[1024]={0};
	//int len = ((strlen(Str)+15)>>4)<<4;
	Aes.Encrypt((const unsigned char *)Str,Enc,48);
	Aes.Decrypt(Enc,Dec,48);
	ensure("Encryption and Decryption works",(0 == memcmp(Str,Dec,48)));

//	FILE *fp = fopen("/mnt/mmc/Enc.txt","wb");
//	int retval = fwrite(Enc,1,1024,fp);
//	printf("retval %d\n",retval);
//	ensure("Unable to Write to file ",(retval==1024));
//	fclose(fp);


	unsigned char Dec2[1024]={0};
	Aes1.Decrypt(Enc,Dec2,48);

	ensure("Encryption and Decryption works with file rd",(0 == memcmp(Dec,Dec2,48)));

}


template<>
template<>
void testobject::test<5>() {
	set_test_name("AESReader/Writer");
	cfg.setValue("GRI.EncryptionDebug", "0");

	char *fname = "../Eyelock/data/Eyes/dbheader.txt";
	WriteHeader(fname,2,4);
	unsigned char Enc[5152+32];
	unsigned char Dec[5152+32];
	int recsize = 5152;
	for(int i=0;i<recsize;i++)	{
		Dec[i]= i+48;
		Enc[i]= 0;
	}
	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
	FILE *fp = fopen(fname,"a");
	fwrite(&Key,1,IVSIZE,fp); // 32 byte key
	fwrite(&recsize,1,4,fp);
	Dec[0]=2;Dec[1]=0;Dec[2]=0;Dec[3]=0;

	AES Aes;
	Aes.SetIV(Key,IVSIZE);
	Aes.SetKey(Key,KEYSIZE);

	Aes.Encrypt((const unsigned char *)Dec,Enc,recsize);
	fwrite(&Enc,1,recsize,fp); //rec size
	const unsigned char Key1[] ={0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1};
	fwrite(&Key1,1,IVSIZE,fp); // 32 byte key
	fwrite(&recsize,1,4,fp);
	Aes.SetIV(Key1,IVSIZE);
	Aes.SetKey(Key,KEYSIZE);

	unsigned char Enc1[5152];
	unsigned char Dec1[5152];

	for(int i=0;i<recsize;i++)	{
		Dec1[i]= rand();
		Enc1[i] =0;
	}
	Dec1[0]=2;Dec1[1]=0;Dec1[2]=0;Dec1[3]=0;
	Aes.Encrypt((const unsigned char *)Dec1,Enc1,recsize);
	fwrite(&Enc1,1,recsize,fp);
	fclose(fp);

	// Now the testcase begins
	IrisDBHeader rptr;
	rptr.ReadHeader(fname);

	AesRdr = new AesRW(new FileRW(fname,18));
	AesRdr->Init(&rptr);
	unsigned char Buff[12000];

	for(int i=0;i<rptr.GetNumRecord();i++){
		AesRdr->Read(Buff+i*rptr.GetOneRecSizeinDB(),rptr.GetOneRecSizeinDBFile(),i*rptr.GetOneRecSizeinDBFile());
	}
	ensure("Encryption and Decryption works1", (0==memcmp(Buff,Dec,rptr.GetOneRecSizeinDB())));
	//ensure("Encryption and Decryption works2", (0==memcmp(Buff+rptr.GetOneRecSizeinDB(),Dec1,rptr.GetOneRecSizeinDB())));

}

template<>
template<>
void testobject::test<6>() {
	set_test_name("AESReader/Writer speed");
	cfg.setValue("GRI.EncryptionKey","USETHISFORENCRYPTION");
	cfg.setValue("GRI.EncryptionDebug", "0");
	AES Aes;
	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,0xF,0xE};

	int sz = 1024*2;

	unsigned char *Enc = (unsigned char *) calloc(1024*sz+32,1);
	unsigned char *Dec = (unsigned char *) calloc(1024*sz+32,1);
	unsigned char *Dec1 = (unsigned char *) calloc(1024*sz+32,1);
	if((Enc == NULL)||(Dec == NULL)||(Dec1 == NULL)){
		printf("Unable to allocate\n");
	}

	for(int k=0;k<=sz;k+=256)
	{
		int step = 1024;
		int tot = step*k;
//		printf("Mem %d KBytes or ",tot/1024);
//		printf("%#6x Bytes takes \n",tot);

		for(int i=0;i<tot;i++)	{
			Dec[i]= i+48;
		}

		XTIME_OP("ONLYENCRYPT",
		{
				for(int i=0;i<k;i++){
					Aes.SetIV(Key,IVSIZE);
					Aes.SetKey(Key,KEYSIZE);
					Aes.Encrypt(&Dec[i*step],&Enc[i*step],step);
				}
		}\
		);

		XTIME_OP("ONLYDECRYPT",
		{
				for(int i=0;i<k;i++){
					Aes.SetIV(Key,IVSIZE);
					Aes.SetKey(Key,KEYSIZE);
					Aes.Decrypt(&Enc[i*step],&Dec1[i*step],step);
				}
		}\
		);

		ensure("Encryption and Decryption works",(0== memcmp(Dec1,Dec,tot)));

	}

	free(Enc);
	free(Dec);
	free(Dec1);
}

template<>
template<>
void testobject::test<7>() {
	set_test_name("Create DB with 10 Images");
	char *fname = "../Eyelock/data/Eyes/DB10.bin";
	int dataBaseCnt =10;
	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";

	unsigned char Enc[6000];
	int metadata = 2;
	int idsize = 14;

	WriteHeader(fname,5,dataBaseCnt);

	FILE *fp = fopen(fname,"a");

	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);

	char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key

		irisCode[0] = 2;
		irisCode[1] = 0;
		irisCode[2] = 0;
		irisCode[3] = 0;


		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,i+j);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;
				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, irisCode+4+j*bioInstance->GetFeatureLength())
				);

				if(retVal){
//					printf("SUCCESS\n");
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}

		}
		//Now Try to fill the other stuff
//		printf("F2F:");
		int k = bioInstance->GetFeatureLength()*2;
		for(int l=0;l<idsize;l++){
			irisCode[k++]= i;
//			printf("%d ",i);
		}
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[k++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[k++]= 0;
		}

		int l = ((2+2+bioInstance->GetFeatureLength()*2 + idsize+metadata*2  +15)>>4)<<4;

		for(int j = 0;j< l-k;j++){
			irisCode[k++]= 0;
		}
		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)irisCode,Enc,l);

		unsigned int recsize = l;
		fwrite(&recsize,1,4,fp); // Enc rec size

		fwrite(&Enc,1,l,fp);
//		printf("Rec Size = %d %d %d %d \n",l,bioInstance->GetFeatureLength(),idsize,metadata);

	}
	fclose(fp);
//	printf("Done");

}

template<>
template<>
void testobject::test<8>() {
	set_test_name("AESDB Confirm ");

	char *fname = "../Eyelock/data/Eyes/DB10.bin";

	// Now the testcase begins
	IrisDBHeader rptr;
	rptr.ReadHeader(fname);

	AesRdr = new AesRW(new FileRW(fname,rptr.GetHeaderSize()));
	AesRdr->Init(&rptr);
	unsigned char Buff[50000];

	for(int i=0;i<rptr.GetNumRecord();i++){
		AesRdr->Read(Buff+i*rptr.GetOneRecSizeinDB(),rptr.GetOneRecSizeinDBFile(),i*rptr.GetOneRecSizeinDBFile());

		int *pNumEyes = (int*)(Buff+i*rptr.GetOneRecSizeinDB());
//		printf("NumEYES %d %d\n",pNumEyes[0],2);
//		printf("F2F Bytes:");
		unsigned char *ptr = Buff+i*rptr.GetOneRecSizeinDB() +rptr.GetIrisSize()*4;
		for(int j=0;j<rptr.GetF2FSize();j++){
//			printf("%d %d\n",ptr[j],(i<<1));
			ensure_equals("F2F",ptr[j],(i<<1));
		}
		printf("\n");
	}

}
void SaveIrisCode(char*iris,int id){
	char *outpath = "../Eyelock/data/Eyes/Match%d.bin";
	char fname[100];
	sprintf(fname,outpath,id);
	printf("Saving Match file %s\n",fname);
	FILE *fp = fopen(fname,"wb");
	char* ptr= "MATCH;0;0;2560;";
	fwrite(ptr,1,strlen(ptr),fp);
	fwrite(iris,1,2560,fp);
	fclose(fp);
}

template<>
template<>
void testobject::test<9>() {
	set_test_name("Create permute DB with 10 Images");
	char *buff = "../Eyelock/data/Eyes/PDB%d.bin";

	seqindx = 2;//number of eyes
	if(argc >= 4)
		seqindx = atoi(argv[3]);

	int dataBaseCnt = seqindx;
	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	char fname[100];
	sprintf(fname,buff,seqindx);
	WriteHeader(fname,dataBaseCnt/2,dataBaseCnt);

	unsigned char Buff[50000];
	int buffcnt=0;
	FILE *fp = fopen(fname,"a");

	unsigned char Enc[6000];
	int iriscodesize = 1280;
	int metadata = 2;
	unsigned int key = 0x12345678;
	int idsize = 14;

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);

	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);
	bioInstance->SetPupilRadiusSearchRange(20, 90);
    bioInstance->SetIrisRadiusSearchRange(80, 140);
    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);
	int irisCodePos=0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
		memcpy(&Buff[buffcnt],&Key,IVSIZE);
		buffcnt+=IVSIZE;
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;

		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,i+j);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(irisCode+irisCodePos))
				);
				SaveIrisCode((char*)irisCode+irisCodePos,i+j);

				irisCodePos+=bioInstance->GetFeatureLength();
				if(retVal){
//					printf("SUCCESS\n");
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("F2F:");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 48;
		}
		if(idsize>2){
			irisCode[irisCodePos-1]= 0;
		}

		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= i;
//			printf("%d ",i);
		}
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}

		int EncSz = ((2+2+bioInstance->GetFeatureLength()*2 + idsize+metadata*2  +15)>>4)<<4;

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4);
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

		unsigned char buff0[2560],buff1[2560];
		{
			m_PermServer->Recover(Prem+4,buff0,buff1);
			ensure_equals("REcovery1",0,memcmp(buff0,irisCode+4,iriscodesize));
			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
		}

		{
			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
		}

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		unsigned int recsize = EncSz;
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		memcpy(&Buff[buffcnt],&recsize,4);
		buffcnt+=4;
		memcpy(&Buff[buffcnt],&Enc,EncSz);
		buffcnt+=EncSz;

//		printf("Rec:%d",i);
//		printf("Rec Size = %d",EncSz);
	}
	fclose(fp);
//	printf("done\n");

	{
		int cntr = 1000;
		seqindx = cntr*seqindx;
		char fname1[100];
		sprintf(fname1,buff,seqindx);
//		printf("Creating DB %s \n",fname1);
		WriteHeader(fname1,seqindx/2,seqindx);

		FILE *fp = fopen(fname1,"a");
//		printf("BuffCnt = %d\n",buffcnt);
		for(int i=0;i<cntr;i++){
			fwrite(Buff,1,buffcnt,fp); // Enc rec size
		}
		fclose(fp);
//		printf("FILE size %d \n",FileSize(fname1));

	}


	fp = fopen(fname,"rb");
	fseek(fp,18,SEEK_SET);
	unsigned char buf[32];
	int recsz=5152;
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV1",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz1",recsz,5152);


	fseek(fp,18+IVSIZE+4+5152,SEEK_SET);
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV2",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz2",recsz,5152);
	fclose(fp);
}


template<>
template<>
void testobject::test<10>() {
	set_test_name("AES perm DB Confirm ");

	char fname[100];
	char *buff = "../Eyelock/data/Eyes/PDB%d.bin";

	sprintf(fname,buff,seqindx);
	// Now the testcase begins
	IrisDBHeader rptr;
	rptr.ReadHeader(fname);

	ReaderWriter *perRdr = (ReaderWriter *) new PermRW(new AesRW(new FileRW(fname,rptr.GetHeaderSize())));
	perRdr->Init(&rptr);
	unsigned char Buff[50000];

	for(int i=0;i<rptr.GetNumRecord();i++){
		perRdr->Read(Buff,rptr.GetOneRecSizeinDBFile(),i*rptr.GetOneRecSizeinDBFile());

		int *pNumEyes = (int*)(Buff);
//		printf("NumEYES %d %d\n",pNumEyes[0],2);
//		printf("F2F Bytes:");
		unsigned char *ptr = Buff+rptr.GetIrisSize()*4+4;
		for(int j=2;j<rptr.GetF2FSize()-2;j++){
			//printf("%d %c %d\n",ptr[j],ptr[j],(i<<1));
//			printf("%c",ptr[j]);
//			ensure_equals("F2F",ptr[j],(i<<1));
		}
//		printf("\n");
	}
}


template<>
template<>
void testobject::test<11>() {
	set_test_name("Create permute DB with input3");
	char *buff = "../Eyelock/data/Eyes/PDB%d.bin";

	int dataBaseCnt = seqindx&(~1);
	dataBaseCnt = MAX(dataBaseCnt,10);
	const char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	char fname[100];
	sprintf(fname,buff,seqindx);
	WriteHeader(fname,dataBaseCnt/2,dataBaseCnt);

	char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
	BiOmega *bioInstance = new BiOmega;
	//WriteJunkDb(fname,Key,dataBaseCnt-10);

	unsigned char iris[1280*4];
	for(int i=0;i<(dataBaseCnt-10)/2;i++){
		memset(iris,2*i,1280*4);
		WriteIris(fname,iris,1280*4);
	}

	WriteDb(fname,bioInstance,Key,inputpath,10);
	delete bioInstance;
}

template<>
template<>
void testobject::test<12>() {
	set_test_name("Create 2 DB Asc and Dsc");
	char *bufftemp = "../Eyelock/data/Eyes/PD.bin";
	FILE *fin= fopen(bufftemp,"wb");
	fclose(fin);

	int dataBaseCnt = 10;
	dataBaseCnt = MAX(dataBaseCnt,10);
	const char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	{
		char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
		BiOmega *bioInstance = new BiOmega;
		WriteDb(bufftemp,bioInstance,Key,inputpath,10);
		delete bioInstance;
	}

	for(int k=2;k<=dataBaseCnt;k+=2){
		char *buff = "../Eyelock/data/Eyes/RPDB%d.bin";
		char fname[100];
		sprintf(fname,buff,k);
//		printf("Creating %s \n",fname);
		WriteHeader(fname,k/2,k);

		fin= fopen(bufftemp,"rb");
		char Key[200],Enc[10000]={0};
		int recsize=0;
		FILE *fp;
		fp = fopen(fname,"a");
		int sz = 0;
		for(int i=0;i<k;i+=2){
			fread(&Key,1,IVSIZE,fin);// 32 byte key
			fread(&recsize,1,4,fin);// Enc rec size
			fread(&Enc,1,recsize,fin);
			fwrite(Key,1,IVSIZE,fp);// 32 byte key
			fwrite(&recsize,1,4,fp);// Enc rec size
			fwrite(Enc,1,recsize,fp);
		}
		fclose(fin);
		fclose(fp);
	}
}
template<>
template<>
void testobject::test<13>() {
	set_test_name("Create permute Compact DB with 10 Images");
	char *buff = "../Eyelock/data/Eyes/CPDB%d.bin";

	seqindx = 2;//number of eyes
	if(argc >= 4)
		seqindx = atoi(argv[3]);

	int dataBaseCnt = seqindx;
	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	char fname[100];
	sprintf(fname,buff,seqindx);
	WriteHeader(fname,dataBaseCnt/2,dataBaseCnt,false,640,3);

	unsigned char Buff[50000],tempbuff[10000];
	int buffcnt=0;
	FILE *fp = fopen(fname,"a");

	unsigned char Enc[6000];
	int iriscodesize = 640;
	int metadata = 2;
	unsigned int key = 0x12345678;
	int idsize = 14;

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);

	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);
	bioInstance->SetPupilRadiusSearchRange(20, 90);
    bioInstance->SetIrisRadiusSearchRange(80, 140);
    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);

	int irisCodePos=0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
		memcpy(&Buff[buffcnt],&Key,IVSIZE);
		buffcnt+=IVSIZE;
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;

		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,i+j);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(tempbuff))
				);
				unsigned char *Out = irisCode + irisCodePos;

				for(int k=0;k<(bioInstance->GetFeatureLength()>>1);k++){
					unsigned char a = tempbuff[2*k]&0x0F;
					a = (a)|((tempbuff[2*k+1]&0xF)<<4);
					Out[k]= a;
				}

				irisCodePos+=(bioInstance->GetFeatureLength()>>1);
				if(retVal){
//					printf("SUCCESS\n");
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("F2F:");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 48;
		}
		if(idsize>2){
			irisCode[irisCodePos-1]= 0;
		}

		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= i;
//			printf("%d ",i);
		}
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}

		int EncSz = ((2+2+bioInstance->GetFeatureLength() + idsize+metadata*2  +15)>>4)<<4;

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4);
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

		unsigned char buff0[2560],buff1[2560];
		{
			m_PermServer->Recover(Prem+4,buff0,buff1);
			ensure_equals("REcovery1",0,memcmp(buff0,irisCode+4,iriscodesize));
			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
		}

		{
			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
		}

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		unsigned int recsize = EncSz;
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		memcpy(&Buff[buffcnt],&recsize,4);
		buffcnt+=4;
		memcpy(&Buff[buffcnt],&Enc,EncSz);
		buffcnt+=EncSz;

//		printf("Rec:%d",i);
//		printf("Rec Size = %d",EncSz);
	}
	fclose(fp);
//	printf("done");

	fp = fopen(fname,"rb");
	fseek(fp,18,SEEK_SET);
	unsigned char buf[32];
	int recsz=2592;
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV1",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz1",recsz,2592);


	fseek(fp,18+IVSIZE+4+2592,SEEK_SET);
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV2",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz2",recsz,2592);
	fclose(fp);
}


template<>
template<>
void testobject::test<14>() {
	set_test_name("Create permute Compact DB with 10 Images");
	char *buff = "../Eyelock/data/Eyes/SCPDB%d.bin";
	int EncSz;

	seqindx = 2;//number of eyes
	if(argc >= 4)
		seqindx = atoi(argv[3]);

	int dataBaseCnt = seqindx;
	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	char fname[100];
	sprintf(fname,buff,seqindx);
	char a = 'A';

	FILE *fp = fopen(fname,"wb");
	unsigned char magicd = 'd';
	unsigned char magic1 = '1';
	fwrite(&magicd,1,1,fp); //Magic
	fwrite(&magic1,1,1,fp); //Magic
	unsigned short filver = 4;
	fwrite(&filver,1,2,fp); // file ver
	unsigned short iriscodesize = 960;
	fwrite(&iriscodesize,1,2,fp); //iris code size
	short int numrec = dataBaseCnt/2;
	short int numeyes = dataBaseCnt;
	fwrite(&numrec,1,2,fp); // num record
	fwrite(&numeyes,1,2,fp); // rec size
	unsigned char idsize= 0x14;
	fwrite(&idsize,1,1,fp); // id size
	unsigned char metadata= 0x1;
	fwrite(&metadata,1,1,fp); // metadata size
	unsigned short pwdindx = 0;
	fwrite(&pwdindx,1,2,fp); // pwd indx
	unsigned int key = 0x12345678;
	fwrite(&key,1,4,fp); // permutation key
	fclose(fp);


	unsigned char Buff[50000],tempbuff[10000];
	int buffcnt=0;
	fp = fopen(fname,"a");
	unsigned char Enc[6000];

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);
	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);
	bioInstance->SetPupilRadiusSearchRange(20, 90);
    bioInstance->SetIrisRadiusSearchRange(80, 140);
    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);

	int irisCodePos=0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
		memcpy(&Buff[buffcnt],&Key,IVSIZE);
		buffcnt+=IVSIZE;
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
//		printf("Pos %d\n",irisCodePos);
		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,i+j);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(tempbuff))
				);
				unsigned char *Out = irisCode + irisCodePos;

				//Write Code
				{
					unsigned char *mask = &tempbuff[0];
					//Write Mask top 2 bits clubbed together
//					printf("COARSE CODE :: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>3);k++){
						unsigned char a = mask[4*k]&0xC0;
						unsigned char b = mask[4*k+1]&0xC0;
						unsigned char c = mask[4*k+2]&0xC0;
						unsigned char d = mask[4*k+3]&0xC0;
						a = a|(b>>2)|(c>>4)|(d>>6);
						Out[k]= a;
	//					if(k<10)printf("%d ",Out[k]);
					}
	//				printf("\n");
					Out += (bioInstance->GetFeatureLength()>>3);
					irisCodePos += (bioInstance->GetFeatureLength()>>3);
//					printf("Pos %d\n",irisCodePos);
					//Write the Bottom 4 bits now
//					printf("FINE CODE:: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>2);k++){
						unsigned char a = mask[2*k]&0x0F;
						a = a|((mask[2*k+1]&0x0F)<<4);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					Out += (bioInstance->GetFeatureLength()>>2);
					irisCodePos+=(bioInstance->GetFeatureLength()>>2);
//					printf("Pos %d\n",irisCodePos);
				}
				{
					unsigned char *mask = &tempbuff[bioInstance->GetFeatureLength()>>1];
					//Write Mask top 2 bits clubbed together
//					printf("COARSE MASK :: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>3);k++){
						unsigned char a = mask[4*k]&0xC0;
						unsigned char b = mask[4*k+1]&0xC0;
						unsigned char c = mask[4*k+2]&0xC0;
						unsigned char d = mask[4*k+3]&0xC0;
						a = a|(b>>2)|(c>>4)|(d>>6);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					Out += (bioInstance->GetFeatureLength()>>3);
					irisCodePos += (bioInstance->GetFeatureLength()>>3);
//					printf("Pos %d\n",irisCodePos);
//					printf("FINE MASK :: ");
					//Write the Bottom 4 bits now
					for(int k=0;k<(bioInstance->GetFeatureLength()>>2);k++){
						unsigned char a = mask[2*k]&0x0F;
						a = a|((mask[2*k+1]&0x0F)<<4);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					irisCodePos+=(bioInstance->GetFeatureLength()>>2);
//					printf("Pos %d\n",irisCodePos);
				}
				if(retVal){
//					printf("SUCCESS %d \n",irisCodePos);
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("Pos %d\n",irisCodePos);

//		printf("F2F:");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 0;
		}
		if(idsize>2){
			irisCode[irisCodePos-1]= 0;
		}

		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= a;
//			printf("%c ",a);
		}
		a+=1;
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
//		printf("Pos %d\n",irisCodePos);
		int totalfeaturelength = iriscodesize*4;

		EncSz = ((2+2+ totalfeaturelength + idsize+metadata*2  +15)>>4)<<4;
//		printf("Enc Size %d \n",EncSz);

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4); // copy 02 00 00 00
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

		unsigned char buff0[2560],buff1[2560];
		{
			m_PermServer->Recover(Prem+4,buff0,buff1);
//			printf("Coarse CODE :: ");
			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k]);
			}
//			printf("\n");
//			printf("Fine CODE :: ");
			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k+320]);
			}
//			printf("\n");

//			printf("Coarse MASK :: ");
			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k]);
			}
//			printf("\n");
//			printf("Fine MASK :: ");
			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k+320]);
			}
//			printf("\n");


			ensure_equals("REcovery1",0,memcmp
					(buff0,irisCode+4,iriscodesize));
			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
		}

		{
			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
//			printf("Coarse CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k]);
//			}
//			printf("\n");
//			printf("Fine CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k+320]);
//			}
//			printf("\n");
//
//			printf("Coarse MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k]);
//			}
//			printf("\n");
//			printf("Fine MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k+320]);
//			}
//			printf("\n");

			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
		}

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		unsigned int recsize = EncSz;
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		memcpy(&Buff[buffcnt],&recsize,4);
		buffcnt+=4;
		memcpy(&Buff[buffcnt],&Enc,EncSz);
		buffcnt+=EncSz;

//		printf("Rec:%d",i);
//		printf("Rec Size = %d",EncSz);
	}
	fclose(fp);
//	printf("done");

	fp = fopen(fname,"rb");
	fseek(fp,18,SEEK_SET);
	unsigned char buf[32];
	int recsz=EncSz;
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV1",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz1",recsz,EncSz);


	fseek(fp,18+IVSIZE+4+EncSz,SEEK_SET);
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV2",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz2",recsz,EncSz);
	fclose(fp);
}


template<>
template<>
void testobject::test<15>() {
	set_test_name("Create permute Compact DB with 10 Images");
	char *buff = "../Eyelock/data/Eyes/SCPDB%d.bin";
	int EncSz;

	seqindx = 2;//number of eyes
	if(argc >= 4)
		seqindx = atoi(argv[3]);


	int dataBaseCnt = seqindx;
	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	char fname[100];
	sprintf(fname,buff,seqindx);
	char a = 'A';

	FILE *fp = fopen(fname,"wb");
	unsigned char magicd = 'd';
	unsigned char magic1 = '1';
	fwrite(&magicd,1,1,fp); //Magic
	fwrite(&magic1,1,1,fp); //Magic
	unsigned short filver = 4;
	fwrite(&filver,1,2,fp); // file ver
	unsigned short iriscodesize = 960;
	fwrite(&iriscodesize,1,2,fp); //iris code size
	short int numrec = dataBaseCnt/2;
	short int numeyes = dataBaseCnt;
	fwrite(&numrec,1,2,fp); // num record
	fwrite(&numeyes,1,2,fp); // rec size
	unsigned char idsize= 0x14;
	fwrite(&idsize,1,1,fp); // id size
	unsigned char metadata= 0x1;
	fwrite(&metadata,1,1,fp); // metadata size
	unsigned short pwdindx = 0;
	fwrite(&pwdindx,1,2,fp); // pwd indx
	unsigned int key = 0x12345678;
	fwrite(&key,1,4,fp); // permutation key
	fclose(fp);


	unsigned char Buff[50000],tempbuff[10000];
	int buffcnt=0;
	fp = fopen(fname,"a");
	unsigned char Enc[6000];

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);
	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);
	bioInstance->SetPupilRadiusSearchRange(20, 90);
    bioInstance->SetIrisRadiusSearchRange(80, 140);
    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);

	int irisCodePos=0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
//		printf("I %d \n",i);
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
//		memcpy(&Buff[buffcnt],&Key,IVSIZE);
//		buffcnt+=IVSIZE;
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
//		printf("Pos %d\n",irisCodePos);
		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,(i+j)%8);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(tempbuff))
				);
				unsigned char *Out = irisCode + irisCodePos;

				//Write Code
				{
					unsigned char *mask = &tempbuff[0];
					//Write Mask top 2 bits clubbed together
//					printf("COARSE CODE :: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>3);k++){
						unsigned char a = mask[4*k]&0xC0;
						unsigned char b = mask[4*k+1]&0xC0;
						unsigned char c = mask[4*k+2]&0xC0;
						unsigned char d = mask[4*k+3]&0xC0;
						a = a|(b>>2)|(c>>4)|(d>>6);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					Out += (bioInstance->GetFeatureLength()>>3);
					irisCodePos += (bioInstance->GetFeatureLength()>>3);
//					printf("Pos %d\n",irisCodePos);
					//Write the Bottom 4 bits now
//					printf("FINE CODE:: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>2);k++){
						unsigned char a = mask[2*k]&0x0F;
						a = a|((mask[2*k+1]&0x0F)<<4);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					Out += (bioInstance->GetFeatureLength()>>2);
					irisCodePos+=(bioInstance->GetFeatureLength()>>2);
//					printf("Pos %d\n",irisCodePos);
				}
				{
					unsigned char *mask = &tempbuff[bioInstance->GetFeatureLength()>>1];
					//Write Mask top 2 bits clubbed together
//					printf("COARSE MASK :: ");
					for(int k=0;k<(bioInstance->GetFeatureLength()>>3);k++){
						unsigned char a = mask[4*k]&0xC0;
						unsigned char b = mask[4*k+1]&0xC0;
						unsigned char c = mask[4*k+2]&0xC0;
						unsigned char d = mask[4*k+3]&0xC0;
						a = a|(b>>2)|(c>>4)|(d>>6);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					Out += (bioInstance->GetFeatureLength()>>3);
					irisCodePos += (bioInstance->GetFeatureLength()>>3);
//					printf("Pos %d\n",irisCodePos);
//					printf("FINE MASK :: ");
					//Write the Bottom 4 bits now
					for(int k=0;k<(bioInstance->GetFeatureLength()>>2);k++){
						unsigned char a = mask[2*k]&0x0F;
						a = a|((mask[2*k+1]&0x0F)<<4);
						Out[k]= a;
//						if(k<10)printf("%d ",Out[k]);
					}
//					printf("\n");
					irisCodePos+=(bioInstance->GetFeatureLength()>>2);
//					printf("Pos %d\n",irisCodePos);
				}
				if(retVal){
//					printf("SUCCESS %d \n",irisCodePos);
				}
				else
					printf("FAIL\n");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("Pos %d\n",irisCodePos);

//		printf("F2F:");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 0;
		}
		if(idsize>2){
			irisCode[irisCodePos-1]= 0;
		}

		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= a;
//			printf("%c ",a);
		}
		a+=1;
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
//		printf("Pos %d\n",irisCodePos);
		int totalfeaturelength = iriscodesize*4;

		EncSz = ((2+2+ totalfeaturelength + idsize+metadata*2  +15)>>4)<<4;
//		printf("Enc Size %d \n",EncSz);

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4); // copy 02 00 00 00
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

//		unsigned char buff0[2560],buff1[2560];
//		{
//			m_PermServer->Recover(Prem+4,buff0,buff1);
//			printf("Coarse CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k]);
//			}
//			printf("\n");
//			printf("Fine CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k+320]);
//			}
//			printf("\n");
//
//			printf("Coarse MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k]);
//			}
//			printf("\n");
//			printf("Fine MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k+320]);
//			}
//			printf("\n");
//
//
//			ensure_equals("REcovery1",0,memcmp
//					(buff0,irisCode+4,iriscodesize));
//			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
//		}

//		{
//			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
//			printf("Coarse CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k]);
//			}
//			printf("\n");
//			printf("Fine CODE :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff0[k+320]);
//			}
//			printf("\n");
//
//			printf("Coarse MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k]);
//			}
//			printf("\n");
//			printf("Fine MASK :: ");
//			for(int k=0;k<10;k++){
//				printf("%d ",buff1[k+320]);
//			}
//			printf("\n");
//
//			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
//			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
//		}

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		unsigned int recsize = EncSz;
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
//		memcpy(&Buff[buffcnt],&recsize,4);
//		buffcnt+=4;
//		memcpy(&Buff[buffcnt],&Enc,EncSz);
//		buffcnt+=EncSz;

//		printf("Rec:%d",i);
//		printf("Rec Size = %d\n",EncSz);
	}
	fclose(fp);
//	printf("done");

	fp = fopen(fname,"rb");
	fseek(fp,18,SEEK_SET);
	unsigned char buf[32];
	int recsz=EncSz;
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV1",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz1",recsz,EncSz);


	fseek(fp,18+IVSIZE+4+EncSz,SEEK_SET);
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV2",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz2",recsz,EncSz);
	fclose(fp);
}
#if 0
template<>
template<>
void testobject::test<16>() {
	skip();
	set_test_name("Check DB ");
	{
		char *pdb8 = "../Eyelock/data/Eyes/PDB8.bin";
		DBRecieveMsg dbrx;
		bool ret = dbrx.CheckDB(pdb8);
		ensure_equals("CheckPDB8",ret,true);
	}

	{
		char *cpdb8 = "../Eyelock/data/Eyes/CPDB8.bin";
		DBRecieveMsg dbrx;
		bool ret = dbrx.CheckDB(cpdb8);
		ensure_equals("CheckCPDB8",ret,true);
	}

	{
		char *scpdb8 = "../Eyelock/data/Eyes/SCPDB8.bin";
		DBRecieveMsg dbrx;
		bool ret = dbrx.CheckDB(scpdb8);
		ensure_equals("CheckSCDB",ret,true);
	}
}
#endif

template<>
template<>
void testobject::test<17>() {
	set_test_name("Create DB with N Images");
	char *buff = "../Eyelock/data/Eyes/PDB%d.bin";

	int dataBaseCnt = seqindx = 2;//number of eyes

	if(argc >= 4)
	{
		dataBaseCnt = seqindx = atoi(argv[3]);
	}

	char *inputpath = "../Eyelock/data/Eyes/Image%d.pgm";
	FILE *fnamepwd = fopen("../Eyelock/data/Eyes/usr_pwd.txt","r");
	if(fnamepwd == NULL){
		printf("User Password File Missing %s\n","../Eyelock/data/Eyes/usr_pwd.txt");
		return ;
	}

	int idsize =  125;//length of the password+user cred
	if(argc >= 5)
	{
		idsize =  atoi(argv[4]);
	}

	char fname[100];
	sprintf(fname,buff,seqindx);
	WriteHeader(fname,dataBaseCnt/2,dataBaseCnt,false,1280,2,idsize);

	unsigned char Buff[50000];
	int buffcnt=0;
	FILE *fp = fopen(fname,"a");

	unsigned char Enc[6000];
	int iriscodesize = 1280;
	int metadata = 2;
	unsigned int key = 0x12345678;


	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, key);

	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};

	BiOmega *bioInstance = new BiOmega;
//	printf("--------%s Iris Recognition TESTS----------\n", bioInstance->GetVersion());
//	printf("DB file = %s\n",fname);
	bioInstance->SetPupilRadiusSearchRange(20, 90);
    bioInstance->SetIrisRadiusSearchRange(80, 140);
    bioInstance->SetEyeLocationSearchArea(230,195, 180, 90);
	int irisCodePos=0;
	int EncSz =0;
	unsigned char irisCode[7000];
	for(int i=0; i<dataBaseCnt; i+=2)
	{
		fwrite(&Key,1,IVSIZE,fp); // 32 byte key
		memcpy(&Buff[buffcnt],&Key,IVSIZE);
		buffcnt+=IVSIZE;
		irisCodePos=0;
		irisCode[irisCodePos++] = 2;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;
		irisCode[irisCodePos++] = 0;

		for(int j=0;j<2;j++){

			int w, h;
			unsigned char *data = databuff;
			char imgName[100];
			sprintf(imgName,inputpath,i+j);
//			printf("\n--------%s------------\n", imgName);
			int status = ReadPGM5(imgName, data, &w, &h,640000);
			if(w == 640 && h == 480 && status >=0)
			{
				bool retVal=false;

				XTIME_OP("GetIrisCode",
				retVal = bioInstance->GetIrisCode(data, w, h, w, (char*)(irisCode+irisCodePos))
				);
					irisCodePos+=bioInstance->GetFeatureLength();
				if(retVal){
//					printf("SUCCESS ");
				}
				else
					printf("FAIL ");
			}
			else
			{
				printf("Error: Incorrect Format or Unable to Read Image %s\n", imgName);
			}
			//if(data) free(data);
		}
	// Call the permute here
		//Now Try to fill the other stuff
//		printf("\nUSER PWD :");
		for(int l=0;l < 2;l++){
			irisCode[irisCodePos++]= 0;
		}
		irisCode[irisCodePos-2]= 35; //35 bit weigand
		char buff[128];
		memset(buff+32,32,128-32);
		fgets (buff,128,fnamepwd);
		for(int l=2;l < idsize;l++){
			irisCode[irisCodePos++]= buff[l-2];
//			printf("%c ",buff[l-2]);
		}
//		printf("\n");

		//Meta1
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}
		//Meta2
		for(int l=0;l<metadata;l++){
			irisCode[irisCodePos++]= 0;
		}

		EncSz = ((2+2+bioInstance->GetFeatureLength()*2 + idsize+metadata*2  +15)>>4)<<4;

		int garbagesz= EncSz - irisCodePos;
		for(int j = 0;j< garbagesz;j++){
			irisCode[irisCodePos++]= 0;
		}


		unsigned char Prem[8000];
		memcpy(Prem,irisCode,4);
		m_PermServer->Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		m_PermServer->Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);

		unsigned char buff0[2560],buff1[2560];
		{
			m_PermServer->Recover(Prem+4,buff0,buff1);
			ensure_equals("REcovery1",0,memcmp(buff0,irisCode+4,iriscodesize));
			ensure_equals("REcovery2",0,memcmp(buff1,irisCode+iriscodesize*1+4,iriscodesize));
		}

		{
			m_PermServer->Recover(Prem+4+iriscodesize*2,buff0,buff1);
			ensure_equals("REcovery3",0,memcmp(buff0,irisCode+iriscodesize*2+4,iriscodesize));
			ensure_equals("REcovery4",0,memcmp(buff1,irisCode+iriscodesize*3+4,iriscodesize));
		}

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		int bytes = Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);
		EncSz = ((bytes+15)>>4)<<4;


		unsigned int recsize = EncSz;
		fwrite(&recsize,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		memcpy(&Buff[buffcnt],&recsize,4);
		buffcnt+=4;
		memcpy(&Buff[buffcnt],&Enc,EncSz);
		buffcnt+=EncSz;
//		printf("Rec %d Size = %d",i,EncSz);
	}
	fclose(fp);
//	printf("done\n");

	fp = fopen(fname,"rb");
	fseek(fp,18,SEEK_SET);
	unsigned char buf[32];
	int recsz;
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV1",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz1",recsz,EncSz);


	fseek(fp,18+IVSIZE+4+EncSz,SEEK_SET);
	fread(buf,IVSIZE,1,fp);
	ensure_equals("IV2",0,memcmp(buf,Key,IVSIZE));
	fread(&recsz,4,1,fp);
	ensure_equals("Recsz2",recsz,EncSz);
	fclose(fp);
}


template<>
template<>
void testobject::test<18>() {
	set_test_name("Read DB File and Print");

	char *buff = argv[3];
	// Now the testcase begins
	IrisDBHeader rptr;
	rptr.ReadHeader(buff);
	rptr.PrintAll();

	ReaderWriter *perRdr = (ReaderWriter *) new PermRW(new AesRW(new FileRW(buff,rptr.GetHeaderSize())));
	perRdr->Init(&rptr);
	unsigned char Buff[50000];

	for(int i=0;i<rptr.GetNumRecord();i++){
		perRdr->Read(Buff,rptr.GetOneRecSizeinDBFile(),i*rptr.GetOneRecSizeinDBFile());

		int *pNumEyes = (int*)(Buff);
//		printf("Person %d -> \n",i);
		for(int j=0;j<2;j++){
//			printf("IRIS %d :",j);
			unsigned char *ptr = Buff+4+rptr.GetIrisSize()*4*i + rptr.GetIrisSize()*2*j ;
			for(int j=0;j<25;j++){
//				printf("%02x " ,ptr[j]);
			}
//			printf("\n");
		}

		unsigned char *ptr = Buff+rptr.GetIrisSize()*4+4;
		for(int j=0;j<rptr.GetF2FSize();j++){
//			printf("%c" ,ptr[j]);
		}
//		printf("\n");
		for(int j=0;j<rptr.GetF2FSize();j++){
//			printf("%02x " ,ptr[j]);
		}
//		printf("\n");
	}
}

#if 0
template<>
template<>
void testobject::test<19>() {
	skip();
	set_test_name("Append ONE DB File Over to Another DB File and Print");
	int iriscodesize = 1280;
	char *buff = "../Eyelock/data/Eyes/ReadDB1.bin";
	IrisDBHeader rptr;
	rptr.ReadHeader(buff);
	rptr.PrintAll();
	ReaderWriter *perRdr = (ReaderWriter *) new PermRW(new AesRW(new FileRW(buff,rptr.GetHeaderSize())));
	perRdr->Init(&rptr);

	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
	char *buff1 = "../Eyelock/data/Eyes/ReadDB2.bin";
	IrisDBHeader wptr;
	wptr.ReadHeader(buff1);
	wptr.PrintAll();

	PermuteServer * m_PermServer = new PermuteServer(iriscodesize, wptr.GetPermutationKey());

	printf("Reading File\n");
	rptr.PrintAll();
	printf("Writing File\n");
	wptr.PrintAll();

	char *fname = "../Eyelock/data/Eyes/WriteDB.bin";
	WriteHeader(fname,rptr.GetNumRecord()+wptr.GetNumRecord(),rptr.GetNumEyes()+wptr.GetNumEyes(), wptr.GetPermutationKey()?false:true,1280,wptr.GetFileVersion(),wptr.GetF2FSize());

	int f1sz = FileSize(buff1);
	f1sz = f1sz -  wptr.GetHeaderSize();
	int a;
	FILE *fp1 = fopen(fname,"a");
	FILE *fp2 = fopen(buff1,"r");
	fseek(fp2,wptr.GetHeaderSize(),SEEK_SET);
	for(int i=0;i<f1sz;i++){
		fread(&a,1,1,fp2);
		fwrite(&a,1,1,fp1);
		if(i%10000 == 0)
			printf(".");fflush(stdout);
	}
	fclose(fp1);
	fclose(fp2);

	unsigned char Buff[8000];
	unsigned char Enc[8000];
	int EncSz = wptr.GetRecEncSize();
	unsigned char Prem[8000];
	for(int i=0;i<rptr.GetNumRecord();i++){
		printf("Reading Record %d \n",i);
		memset(Buff,0,8000);
		perRdr->Read(Buff,rptr.GetOneRecSizeinDBFile(),i*rptr.GetOneRecSizeinDBFile());

		if(wptr.GetPermutationKey() !=0){
			memcpy(Prem,Buff,4);
			m_PermServer->Permute(Buff+4, Buff+iriscodesize+4, Prem+4);
			m_PermServer->Permute(Buff+iriscodesize*2+4,  Buff+iriscodesize*3+4, Prem+iriscodesize*2+4);
			int frmf2f = EncSz - iriscodesize*4+4;
			if (frmf2f<0) frmf2f = 0;
			memcpy(Prem+iriscodesize*4+4,Buff+iriscodesize*4+4,frmf2f);
		}else{
			printf("No Perm\n");
			memcpy(Prem,Buff,EncSz);
		}

		for(int j=0;j<36;j++){
			printf("%c" ,*(Prem+iriscodesize*4+4+j));
		}
		printf("\n");

		AES Aes;
		Aes.SetIV(Key,IVSIZE);
		Aes.SetKey(Key,KEYSIZE);
		Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

		FILE *fp = fopen(fname,"a");
		fwrite(&Key,1,IVSIZE,fp);
		fwrite(&EncSz,1,4,fp); // Enc rec size
		fwrite(&Enc,1,EncSz,fp);
		fclose(fp);
	}
}
#endif

//template<>
//template<>
//void testobject::test<20>() {
//	set_test_name("DES Test");
//	unsigned char key[32]={"USETHISFORENCRYPTION"};
//	unsigned char salt[32] ={"1234554321hfkahdskhkdshfkhds"};
//
//	AES Aes1;
//	Aes1.SetDebug(1);
//	Aes1.SetKey(key,32);
//	Aes1.SetIV(salt,32);
//
//	char *Str = "I don't even know if it will work or not, Lets try";
//	unsigned char Enc[1024]={0};
//	unsigned char Dec[1024]={0};
//	int len = strlen(Str);
//	int ret = Aes1.EncryptDes((const unsigned char *)Str,Enc,len);
//	int len1 = Aes1.DecryptDes(Enc,Dec,ret);
//
//	ensure("Encryption and Decryption lengtf were same ",len1 == ret);
//	ensure("Encryption and Decryption works",(0 == memcmp(Str,Dec,len)));
//}
}
