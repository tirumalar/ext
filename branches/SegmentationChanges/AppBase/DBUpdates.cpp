/*
 * DBUpdates.cpp
 *
 *  Created on: Apr 12, 2013
 *      Author: mamigo
 */
#include <stdio.h>
#include <algorithm>
#include "DBUpdates.h"
#include "FileRW.h"
#include "stdlib.h"
#include "iostream"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "IrisDBHeader.h"
#include "DBReceive.h"
#include "PermuteServer.h"
#include "PermRW.h"
#include "CommonDefs.h"
extern "C" {
	#include "file_manip.h"
}

DBUpdates::DBUpdates(Configuration& conf):m_temp1DbFileName(NULL),m_tempDbFileName(NULL),m_tempBuffer(NULL) {
	m_Debug = conf.getValue("Eyelock.DBUpdateDebug",false);

	m_dbFileName = conf.getValue("GRI.irisCodeDatabaseFile", "./data/sqlite.db3");
	DBRead((char*)m_dbFileName);

	m_tempDbFileName= (char *)malloc(strlen(m_dbFileName)+10);
	sprintf(m_tempDbFileName,"%s.tmp",m_dbFileName);

	m_temp1DbFileName= (char *)malloc(strlen(m_dbFileName)+20);
	sprintf(m_temp1DbFileName,"%s.tmp1",m_dbFileName);
	m_tempBuffer = (unsigned char*)malloc(65536);

}

DBUpdates::~DBUpdates() {
	if(m_temp1DbFileName)
		free(m_temp1DbFileName);
	if(m_tempDbFileName)
		free(m_tempDbFileName);
}

//read m_dbFileName

bool DBUpdates::Compare2DB(){
	IrisDBHeader irisDBHeader,irisDBHeader1;
	irisDBHeader.SetDefaults();
	int fsz = FileSize(m_dbFileName);
	if(!fsz){
		printf("No DB %s Available in SD card\n",m_dbFileName);
		return false;
	}
	printf("Reading received DB file %s\n", m_dbFileName);
	irisDBHeader.ReadHeader((char*)m_dbFileName);
	irisDBHeader.PrintAll();

	fsz = FileSize(m_tempDbFileName);
	if(!fsz){
		printf("No DB %s Available in SD card\n",m_tempDbFileName);
		return false;
	}
	printf("Reading received DB file %s\n", m_tempDbFileName);
	irisDBHeader1.ReadHeader((char*)m_tempDbFileName);
	irisDBHeader1.PrintAll();

	printf("Magic Val    %0x %0x ",irisDBHeader.GetMagic(),irisDBHeader1.GetMagic());
	printf("File Version %0x %0x ",irisDBHeader.GetFileVersion(),irisDBHeader1.GetFileVersion());
	printf("IrisSize     %0x %0x ",irisDBHeader.GetIrisSize(),irisDBHeader1.GetIrisSize());
	printf("Meta Size    %0x %0x ",irisDBHeader.GetMetaDataSize(),irisDBHeader1.GetMetaDataSize());
	printf("ID Size      %0x %0x ",irisDBHeader.GetF2FSize(),irisDBHeader1.GetF2FSize());
	printf("Pwd Index    %0x %0x ",irisDBHeader.GetPasswordIndex(),irisDBHeader1.GetPasswordIndex());
	printf("Enc Size     %0x %0x ",irisDBHeader.GetRecEncSize(),irisDBHeader1.GetRecEncSize());

	if((irisDBHeader.GetMagic() == irisDBHeader1.GetMagic())&&
	   (irisDBHeader.GetFileVersion() == irisDBHeader1.GetFileVersion())&&
	   (irisDBHeader.GetIrisSize() == irisDBHeader1.GetIrisSize())&&
	   (irisDBHeader.GetF2FSize() == irisDBHeader1.GetF2FSize())&&
	   (irisDBHeader.GetMetaDataSize() == irisDBHeader1.GetMetaDataSize())&&
	   (irisDBHeader.GetPasswordIndex() == irisDBHeader1.GetPasswordIndex())&&
	   (irisDBHeader.GetRecEncSize() == irisDBHeader1.GetRecEncSize())&&
	   (irisDBHeader1.GetNumRecord()!= 0)
 	){
		printf("Incremental DB and DB on Board are of same type we can continue For updates");
		return true;
	}
	return false;
}


void DBUpdates::DBRead(char* dbFileName){
	IrisDBHeader irisDBHeader;
	irisDBHeader.SetDefaults();
	m_GUID.clear();
	int fsz = FileSize(dbFileName);
	if(!fsz){
		printf("No DB Available in SD card\n");
		return;
	}
	printf("Reading received DB file %s\n", dbFileName);
	irisDBHeader.ReadHeader((char*)dbFileName);
	irisDBHeader.PrintAll();
	ReaderWriter *perRdr = (ReaderWriter *) new PermRW(new AesRW(new FileRW((char*)dbFileName,irisDBHeader.GetHeaderSize())));
	perRdr->Init(&irisDBHeader);

	for(int i=0;i<irisDBHeader.GetNumRecord();i++){
		unsigned char Buff[8000];
		perRdr->Read(Buff,irisDBHeader.GetOneRecSizeinDBFile(),i*irisDBHeader.GetOneRecSizeinDBFile());
		unsigned char *ptr = Buff+4+irisDBHeader.GetIrisSize()*4;
/*		{
			printf("For %d Person : ",i);
			unsigned char *abc = (unsigned char *)ptr;
			for(int j=0;j<irisDBHeader.GetF2FSize();j++){
				printf("%02x",*abc++);
			}
			printf("\n");
			abc = (unsigned char *)ptr;
			for(int j=0;j<irisDBHeader.GetF2FSize();j++){
				printf("%c",*abc++);
			}
			printf("\n");
		}
*/
		int numbits = (int)((((unsigned int) ptr[1])<<8) + (unsigned char) ptr[0]);
		int f2fOffset = ((numbits+7)>>3);
		std::string test((const char*)ptr,f2fOffset+2);
		m_GUID.push_back(test);
		if(m_Debug){
			printf("%d -> ",i);
			unsigned char *ptr2 = (unsigned char *)m_GUID[i].c_str();
			for(int j=0;j<m_GUID[i].length();j++){
				printf("%c",*ptr2++);
			}
			printf("\n");
			ptr2 = (unsigned char *)m_GUID[i].c_str();
			for(int j=0;j<m_GUID[i].length();j++){
				printf("%02x",*ptr2++);
			}
			printf("\n");
		}
	}
}

bool DBUpdates::ReadDecrementalRecords(char *filename){
	bool error = false;
	m_addOrRemoveGUID.clear();
	int fsz = FileSize(filename);
	if(!fsz){
		printf("No GUID in SD card\n");
		return false;
	}
	printf("Reading received GUID file %s %d\n", filename,fsz);

	FILE *fi = fopen(filename,"rb+");
	if(fi == NULL){
		printf("ReadDecrementalRecords failed, unable to read file \n");
	}
	int cntr=0;

	while((!error)&&(cntr<fsz)){
		int retbytes = fread(m_tempBuffer,1,2,fi);
		cntr+=retbytes;
		if(retbytes!=2){
			printf("ReadDecrementalRecords failed, unable to get F2F Number of bits \n");
			error = true;
			continue;
		}
		int numbits = (int)((((unsigned int) m_tempBuffer[1])<<8) + (unsigned char) m_tempBuffer[0]);
		int f2fbytes = ((numbits+7)>>3);
		if(m_Debug){
			printf("%d %d\n",numbits,f2fbytes);
		}
		retbytes = fread(m_tempBuffer+2,1,f2fbytes,fi);
		cntr+=retbytes;
		if(retbytes!=f2fbytes){
			printf("ReadDecrementalRecords failed, unable to get F2F data bytes \n");
			error = true;
			continue;
		}
		retbytes = fread(m_tempBuffer+2+f2fbytes,1,1,fi);
		cntr+=retbytes;
		if((retbytes!=1)||(*(m_tempBuffer+2+f2fbytes) != ';')){
			printf("ReadDecrementalRecords failed, unable to get ; %d %c \n",retbytes,*(m_tempBuffer+2+f2fbytes));
			error = true;
			continue;
		}
		std::pair<int, std::string> value;
		value.second.assign((const char*)m_tempBuffer,f2fbytes+2);
		value.first=-1;
		m_addOrRemoveGUID.push_back(value);
		if(m_Debug){
			for(int j=0;j<f2fbytes+2;j++){
				printf("%02x",m_tempBuffer[j]);
			}
			printf("\n");
		}
	}
	fclose(fi);
	return !error;
}

bool addremoveSortFunction(std::pair< int, std::string > i, std::pair< int, std::string > j){
	return (i.first < j.first);
}

bool DBUpdates::CheckDecrementalRecordsWithDB(){
	for(int i=0;i< m_GUID.size();i++){
		unsigned char *ptr = (unsigned char *)m_GUID[i].c_str(),*ptr2;
		int numbits = (int)((((unsigned int) ptr[1])<<8) + (unsigned char) ptr[0]);
		int f2fbytes = ((numbits+7)>>3);
		if(m_Debug){
			printf("%d -> ",i);
			ptr2 = ptr;
			for(int j=0;j<m_GUID[i].length();j++){
				printf("%c",*ptr2++);
			}
			printf("\n");
			ptr2 = ptr;
			for(int j=0;j<m_GUID[i].length();j++){
				printf("%02x",*ptr2++);
			}
			printf("\n");
		}
		for(int k= 0;k<m_addOrRemoveGUID.size();k++){
			unsigned char *guid = (unsigned char*)m_addOrRemoveGUID[k].second.c_str();
			int numbits = (int)((((unsigned int) ptr[1])<<8) + (unsigned char) ptr[0]);
			int f2fbytes = ((numbits+7)>>3);
			if(m_Debug){
				printf("Comparing \n");
				for(int j=0;j<f2fbytes+2;j++){
					printf("%02x",ptr[j]);
				}
				printf("\n");
				printf("With \n");
				for(int j=0;j<f2fbytes+2;j++){
					printf("%02x",guid[j]);
				}
				printf("\n");
			}
			if(0 == memcmp(ptr,guid,f2fbytes+2)){
				m_addOrRemoveGUID[k].first = i;
				printf("To Be deleted GUID %d %s \n",i,guid);
			}
		}
	}
	// Try to Sort it
	std::sort(m_addOrRemoveGUID.begin(), m_addOrRemoveGUID.end(), addremoveSortFunction);

	return true;
}

//write header
void DBUpdates::WriteDBHeader(char *fname,short numrec, short numeyes, int perm,unsigned short irissz,unsigned char metadata,unsigned short filever,unsigned char idsz,unsigned short pwdindx){
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
	fwrite(&metadata,1,1,fp); // metadata size
	fwrite(&pwdindx,1,2,fp); // pwd indx
	fwrite(&perm,1,4,fp); // permutation key
	fclose(fp);
}

void DBUpdates::CreateDummyIris(unsigned char *Output,IrisDBHeader& irisDBHeader)
{
//	unsigned int key = 0x12345678;
	int metadata = 2;
	int iriscodesize = 1280;
	int cnt = 0;
//	const unsigned char Key[] ={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2};
	unsigned int randBuff[8]={0};
	for(int i=0;i<4;i++){
		randBuff[i] = rand();
	}
	memcpy(&Output[cnt],randBuff,irisDBHeader.GetIVSize()); // 16 byte key
	cnt += irisDBHeader.GetIVSize();
	unsigned char irisCode[7000]={0};
	int irisCodePos =0;
	irisCode[irisCodePos++] = 2;
	irisCode[irisCodePos++] = 0;
	irisCode[irisCodePos++] = 0;
	irisCode[irisCodePos++] = 0;

	//memcpy(irisCode+4,iris,sz);
	memset(&irisCode[irisCodePos],0x00,irisDBHeader.GetOneRecSizeinDBFile());

	memset(&irisCode[irisCodePos],0x00,irisDBHeader.GetIrisSize());
	irisCodePos +=irisDBHeader.GetIrisSize();
	memset(&irisCode[irisCodePos],0x00,irisDBHeader.GetIrisSize());
	irisCodePos +=irisDBHeader.GetIrisSize();
	memset(&irisCode[irisCodePos],0x00,irisDBHeader.GetIrisSize());
	irisCodePos +=irisDBHeader.GetIrisSize();
	memset(&irisCode[irisCodePos],0x00,irisDBHeader.GetIrisSize());
	irisCodePos +=irisDBHeader.GetIrisSize();

	unsigned char Prem[8000]={0},Enc[8000]={0};
	int EncSz = irisDBHeader.GetRecEncSize();
	if(irisDBHeader.GetPermutationKey()!=0){
		PermuteServer permServer = PermuteServer(iriscodesize, irisDBHeader.GetPermutationKey());
		memcpy(Prem,irisCode,4);
		permServer.Permute(irisCode+4, irisCode+iriscodesize+4, Prem+4);
		permServer.Permute(irisCode+iriscodesize*2+4,  irisCode+iriscodesize*3+4, Prem+iriscodesize*2+4);
		int frmf2f = EncSz - iriscodesize*4+4;
		if (frmf2f<0) frmf2f = 0;
		memcpy(Prem+iriscodesize*4+4,irisCode+iriscodesize*4+4,frmf2f);
	}else{

		memcpy(Prem,irisCode,EncSz);
	}

	AES Aes;

	Aes.SetIV((unsigned char *)randBuff,irisDBHeader.GetIVSize());
	Aes.SetKey(irisDBHeader.GetKey(),irisDBHeader.GetKeySize());
	Aes.Encrypt((const unsigned char *)Prem,Enc,EncSz);

	unsigned int recsize = EncSz;
	memcpy(&Output[cnt],&recsize,4); // 4 byte size
	cnt += 4;
	memcpy(&Output[cnt],&Enc,EncSz); // 4 byte size
	unsigned char res[8000]={0};
	Aes.Decrypt(Enc,res,EncSz);
}


bool DBUpdates::DeleteTheRecords(){
	unsigned char Buff[8000];
	IrisDBHeader irisDBHeader;
	irisDBHeader.SetDefaults();
	int fsz = FileSize(m_dbFileName);
	if(!fsz){
		printf("No DB Available in SD card\n");
		return false;
	}
	irisDBHeader.ReadHeader((char*)m_dbFileName);
	irisDBHeader.PrintAll();

	int tobedeleted = 0;

	for(int i=0;i<m_addOrRemoveGUID.size();i++){
		if(m_addOrRemoveGUID[i].first != -1){
			tobedeleted++;
		}
	}

	printf("Delete the Records %d \n",tobedeleted);
	if(tobedeleted == 0)
		return false;

	FileRW rwOut= FileRW(m_temp1DbFileName);
	irisDBHeader.WriteHeader(&rwOut);
	FileRW rwInp= FileRW((char*)m_dbFileName);

	int inpoffset=irisDBHeader.GetHeaderSize();
	int outoffset=irisDBHeader.GetHeaderSize();

	int prev_indx = 0;
	tobedeleted=0;
	for(int k=0;k<m_addOrRemoveGUID.size();k++){
		if(m_addOrRemoveGUID[k].first != -1){
			for(int i=prev_indx;i<m_addOrRemoveGUID[k].first;i++){
				int rd = rwInp.Read(Buff,irisDBHeader.GetOneRecSizeinDBFile(),inpoffset);
				if(rd != irisDBHeader.GetOneRecSizeinDBFile()) return false;
				int wr = rwOut.Write(Buff,irisDBHeader.GetOneRecSizeinDBFile(),outoffset);
				if(wr != irisDBHeader.GetOneRecSizeinDBFile()) return false;
				inpoffset+=irisDBHeader.GetOneRecSizeinDBFile();
				outoffset+=irisDBHeader.GetOneRecSizeinDBFile();
			}
			prev_indx = m_addOrRemoveGUID[k].first+1;
			inpoffset+=irisDBHeader.GetOneRecSizeinDBFile();
			tobedeleted++;
		}
	}

	for(int i=prev_indx;i<irisDBHeader.GetNumRecord();i++){
		int rd = rwInp.Read(Buff,irisDBHeader.GetOneRecSizeinDBFile(),inpoffset);
		if(rd != irisDBHeader.GetOneRecSizeinDBFile()) return false;
		int wr = rwOut.Write(Buff,irisDBHeader.GetOneRecSizeinDBFile(),outoffset);
		if(wr != irisDBHeader.GetOneRecSizeinDBFile()) return false;
		inpoffset+=irisDBHeader.GetOneRecSizeinDBFile();
		outoffset+=irisDBHeader.GetOneRecSizeinDBFile();
	}

	if(tobedeleted){
		FILE *fp = fopen(m_temp1DbFileName,"rb+");
		fseek(fp,6,SEEK_SET);
		short numrec = MAX(irisDBHeader.GetNumRecord()-tobedeleted,0);
		if(numrec == 0){
			short num=1;
			fwrite(&num,1,2,fp); // num record
			short numeyes = num*2;
			fwrite(&numeyes,1,2,fp); // rec size
			fclose(fp);
			printf("Writing Dummy Records\n");
			CreateDummyIris(Buff,irisDBHeader);
			FILE *fo = fopen(m_temp1DbFileName,"rb+");
			fseek(fo,irisDBHeader.GetHeaderSize(),SEEK_SET);
			int writebytes = fwrite(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fo);
			fclose(fo);
		}else{
			fwrite(&numrec,1,2,fp); // num record
			short numeyes = numrec*2;
			fwrite(&numeyes,1,2,fp); // rec size
			fclose(fp);
		}
	}

#ifdef _OLD_WORKING
	//Make a copy of the given DB
	fsz = FileSize(m_dbFileName);
	int written =0;
	FILE *fi = fopen(m_dbFileName,"rb+");
	FILE *fo = fopen(m_temp1DbFileName,"wb+");
	while(written < fsz){
		int retbytes = fread(Buff,1,1024,fi);
		int writebytes = fwrite(Buff,1,retbytes,fo); // num record
		written = writebytes+written;
	}
	fclose(fi);
	fclose(fo);
	CreateDummyIris(Buff,irisDBHeader);
	//To be UPDATED
	for(int i=0;i<irisDBHeader.GetNumRecord();i++){
		bool found = false;
		int index = -1;
		for(int k=0;k<m_addOrRemoveGUID.size();k++){
			if(m_addOrRemoveGUID[k].first == i){
				found = true;
				index = k;
				break;
			}
		}
		if(found){
			printf("Overwriting index %d for GUID %s \n",m_addOrRemoveGUID[index].first,m_GUID[m_addOrRemoveGUID[index].first].c_str());
			// point to the input DB file at index
			FILE *fo = fopen(m_temp1DbFileName,"rb+");
			fseek(fo,irisDBHeader.GetHeaderSize()+irisDBHeader.GetOneRecSizeinDBFile()*m_addOrRemoveGUID[index].first,SEEK_SET);
			int writebytes = fwrite(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fo);
			fclose(fo);
			int len = m_GUID[m_addOrRemoveGUID[index].first].length();
			memset(m_tempBuffer,0,len);
			m_GUID[m_addOrRemoveGUID[index].first].assign((char*)m_tempBuffer,len);
		}
	}
#endif
	printf("Deleted the Records %d \n",tobedeleted);
	return true;
}

bool DBUpdates::DeleteInDB(){
	m_addOrRemoveGUID.clear();
	printf("ReadDecrementalRecords \n");
	if(!ReadDecrementalRecords(m_tempDbFileName)){
		return false;
	}
	printf("CheckDecrementalRecordsWithDB \n");
	if(!CheckDecrementalRecordsWithDB()){
		return false;
	}
	printf("DeleteTheRecords \n");
	if(!DeleteTheRecords()){
		return false;
	}
	printf("Rename the files \n");
	if(0 ==rename(m_temp1DbFileName,m_tempDbFileName)){
		return true;
	}
	else
		return false;
}

bool DBUpdates::UpdateInDB(){
	m_addOrRemoveGUID.clear();
	printf("ReadIncrementalRecords \n");
	if(!ReadIncrementalRecords(m_tempDbFileName)){
		return false;
	}
	printf("CheckIncrementalRecordsWithDB \n");
	if(!CheckIncrementalRecordsWithDB()){
		return false;
	}
	printf("AddUpdateTheRecords \n");
	AddUpdateTheRecords();
	printf("Rename the files \n");
	if(0 ==rename(m_temp1DbFileName,m_tempDbFileName)){
		return true;
	}
	else
		return false;
}

bool DBUpdates::ReadIncrementalRecords(char *filename){
	m_TempirisDBHeader.SetDefaults();
	int fsz = FileSize(filename);
	if(!fsz){
		printf("No DB Available in SD card\n");
		return false;
	}
	printf("Reading received DB file %s\n", filename);
	m_TempirisDBHeader.ReadHeader((char*)filename);
	m_TempirisDBHeader.PrintAll();


	ReaderWriter *perRdr1 = (ReaderWriter *) new PermRW(new AesRW(new FileRW((char*)filename,m_TempirisDBHeader.GetHeaderSize())));
	perRdr1->Init(&m_TempirisDBHeader);
	int bytes =0;
	for(int i=0;i<m_TempirisDBHeader.GetNumRecord();i++){
		perRdr1->Read(m_tempBuffer,m_TempirisDBHeader.GetOneRecSizeinDBFile(),i*m_TempirisDBHeader.GetOneRecSizeinDBFile());
		unsigned char *key = m_tempBuffer + 4 + m_TempirisDBHeader.GetIrisSize()*4;
		int numbits = (int)((((unsigned int) key[1])<<8) + (unsigned char) key[0]);
		int f2fbytes = ((numbits+7)>>3);
		std::pair<int, std::string> value;
		value.second.assign((const char*)key,f2fbytes+2);
		value.first=-1;
		m_addOrRemoveGUID.push_back(value);
	}
	delete perRdr1;
	return true;
}

bool DBUpdates::CheckIncrementalRecordsWithDB(){
	for(int i=0;i< m_GUID.size();i++){
		unsigned char *ptr = (unsigned char *)m_GUID[i].c_str(),*ptr2;
		int numbits = (int)((((unsigned int) ptr[1])<<8) + (unsigned char) ptr[0]);
		int f2fbytes = ((numbits+7)>>3);
		if(m_Debug){
			printf("%d -> ",i);
			ptr2 = ptr+2;
			for(int j=0;j<f2fbytes;j++){
				printf("%c",*ptr2++);
			}
			printf("\n");
			ptr2 = ptr+2;
			for(int j=0;j<f2fbytes;j++){
				printf("%02x",*ptr2++);
			}
			printf("\n");
		}
		for(int k= 0;k<m_addOrRemoveGUID.size();k++){
			unsigned char *guid = (unsigned char*)m_addOrRemoveGUID[k].second.c_str();
			int numbits = (int)((((unsigned int) ptr[1])<<8) + (unsigned char) ptr[0]);
			int f2fbytes = ((numbits+7)>>3);
			if(0 == memcmp(ptr,guid,f2fbytes+2)){
				m_addOrRemoveGUID[k].first = i;
				printf("To Be updated GUID %d %s \n",i,guid);
			}
		}
	}
	return true;
}

void DBUpdates::AddUpdateTheRecords(){
	unsigned char Buff[8000];
	IrisDBHeader irisDBHeader;
	irisDBHeader.SetDefaults();
	int fsz = FileSize(m_dbFileName);
	if(!fsz){
		printf("No DB Available in SD card\n");
		return ;
	}
	irisDBHeader.ReadHeader((char*)m_dbFileName);
	irisDBHeader.PrintAll();

	int tobeadded = 0;
	int tobeupdated = 0;

	for(int i=0;i<m_addOrRemoveGUID.size();i++){
		if(m_addOrRemoveGUID[i].first != -1){
			tobeupdated++;
		}else
			tobeadded++;
	}

	printf("Update/Added the Record %d %d \n",tobeupdated,tobeadded);

	//Make a copy of the given DB
	fsz = FileSize(m_dbFileName);
	int written =0;
	FILE *fi = fopen(m_dbFileName,"rb+");
	FILE *fo = fopen(m_temp1DbFileName,"wb+");
	while(written < fsz){
		int retbytes = fread(Buff,1,1024,fi);
		int writebytes = fwrite(Buff,1,retbytes,fo); // num record
		written = writebytes+written;
	}
	fclose(fi);
	fclose(fo);

	//To be UPDATED
	for(int i=0;i<irisDBHeader.GetNumRecord();i++){
		bool found = false;
		int index = -1;
		for(int k=0;k<m_addOrRemoveGUID.size();k++){
			if(m_addOrRemoveGUID[k].first == i){
				found = true;
				index = k;
				break;
			}
		}
		if(found){
			printf("Overwriting index %d for GUID %s \n",m_addOrRemoveGUID[index].first,m_GUID[m_addOrRemoveGUID[index].first].c_str());
			// point to the input DB file at index
			FILE *fi = fopen(m_tempDbFileName,"rb+");
			fseek(fi,m_TempirisDBHeader.GetHeaderSize()+m_TempirisDBHeader.GetOneRecSizeinDBFile()*index,SEEK_SET);
			int readbytes = fread(Buff,1,m_TempirisDBHeader.GetOneRecSizeinDBFile(),fi);
			fclose(fi);

			FILE *fo = fopen(m_temp1DbFileName,"rb+");
			fseek(fo,irisDBHeader.GetHeaderSize()+irisDBHeader.GetOneRecSizeinDBFile()*m_addOrRemoveGUID[index].first,SEEK_SET);
			int writebytes = fwrite(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fo);
			fclose(fo);
		}
	}

	if(tobeadded > 0){
		//To be ADDED
		FILE *fp = fopen(m_temp1DbFileName,"rb+");
		fseek(fp,6,SEEK_SET);
		short numrec = irisDBHeader.GetNumRecord()+tobeadded;
		fwrite(&numrec,1,2,fp); // num record
		short numeyes = (irisDBHeader.GetNumRecord()+tobeadded)<<1;
		fwrite(&numeyes,1,2,fp); // rec size
		fclose(fp);

		for(int k=0;k<m_TempirisDBHeader.GetNumRecord();k++){
			if(m_addOrRemoveGUID[k].first == -1){
				// point to the input DB file at index
				FILE *fi = fopen(m_tempDbFileName,"rb+");
				fseek(fi,m_TempirisDBHeader.GetHeaderSize()+m_TempirisDBHeader.GetOneRecSizeinDBFile()*k,SEEK_SET);
				int readbytes = fread(Buff,1,m_TempirisDBHeader.GetOneRecSizeinDBFile(),fi);
				fclose(fi);

				FILE *fo = fopen(m_temp1DbFileName,"rb+");
				fseek(fo,0,SEEK_END);
				int writebytes = fwrite(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fo);
				fclose(fo);
			}
		}
	}
	printf("Updated/Added the Record %d %d \n",tobeupdated,tobeadded);
}

void DBUpdates::ExtractRecords(){
	unsigned char Buff[8000];
	IrisDBHeader irisDBHeader;
	irisDBHeader.SetDefaults();
	int fsz = FileSize(m_dbFileName);
	if(!fsz){
		printf("No DB Available in SD card\n");
		return ;
	}
	irisDBHeader.ReadHeader((char*)m_dbFileName);
	irisDBHeader.PrintAll();

	for(int i=0;i<m_GUID.size();i++){
		char buff[1024];
		sprintf(buff,"%s_%d.bin",m_dbFileName,i);
		// point to the input DB file at index
		FILE *fi = fopen(m_dbFileName,"rb+");
		fseek(fi,irisDBHeader.GetHeaderSize()+irisDBHeader.GetOneRecSizeinDBFile()*i,SEEK_SET);
		int readbytes = fread(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fi);
		fclose(fi);


		WriteDBHeader(buff,1,2,irisDBHeader.GetPermutationKey(),1280,irisDBHeader.GetMetaDataSize(),irisDBHeader.GetFileVersion(),irisDBHeader.GetF2FSize(),irisDBHeader.GetPasswordIndex());
		FILE *fo = fopen(buff,"rb+");
		fseek(fo,0,SEEK_END);
		int writebytes = fwrite(Buff,1,irisDBHeader.GetOneRecSizeinDBFile(),fo);
		fclose(fo);
	}
}




