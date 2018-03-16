/*
 * ReadDBHeader.cpp
 *
 *  Created on: 03-May-2010
 *      Author: madhav.shanbhag@mamigo.us
 */
#include "sqlite3.h"
#include "IrisDBHeader.h"
#include "FileRW.h"
#include "MemRW.h"
#include "AESClass.h"
#include "AesRW.h"
#include <string.h>
#include "PermuteServer.h"
extern "C" {
#include "file_manip.h"
}

IrisDBHeader::IrisDBHeader():m_CompactDB(NONCOMPACTDB),m_OffsetForFine(0) {
	m_KeyFilename=0;

}

IrisDBHeader::IrisDBHeader(char *fname):m_CompactDB(NONCOMPACTDB),m_OffsetForFine(0) {
	m_KeyFilename=fname;
}

void IrisDBHeader::SetDefaults()
{
	m_Magic[0] =  0;
	m_Magic[1] =  0;
	m_FileVersion = 0;
	m_IrisSize = 1280;
	m_NumRecords = 0;
	m_NumEyes = 0;
	m_IdSize  = 0;
	m_MetadataSize = 0;
	m_PasswordIndex = 0;
	m_PermutationKey = 0;
	m_RecEncSize = 0;
    m_MaskSize = 0;
    m_OffsetForFine = 0;

	// Lets Validate header Now.
//	int EncSz = ((2+2+m_IrisSize*4 + m_IdSize+m_MetadataSize*2 +15)>>4)<<4;
//	printf("Range ( %d %d)\n",EncSz - 100,EncSz + 100);
//	if(!(((EncSz + 50) > m_RecEncSize) &&((EncSz - 50) < m_RecEncSize))){
//		printf("ReadHeader::EncSize is not proper \n");
//		throw("ReadHeader::EncSize is not proper");
//	}
}

IrisDBHeader::~IrisDBHeader() {
}

void IrisDBHeader::ReadKeys(char* fname,int memio){
    // check if <fname>.keys exists.. if it does over write m_Key
	int ret = 0;
	ReaderWriter *reader;
    if(memio){
    	reader = new MemRW(fname);
    }else{
    	reader = new FileRW(fname);
    }

    ret = reader->Read((unsigned char*)(m_Key), 256 * 32, 0);

    if(ret < 1){
        printf("Using Default Keys\n");
        unsigned char key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};
        for(int j = 0;j < 256;j++){
            memcpy(m_Key + j * 32, key, 32);
        }
    }
    delete reader;
}

void IrisDBHeader::ReadAndParseHeader(ReaderWriter * rw){
	unsigned char cptr[100];
	int numbytes= 4;
	if(rw->Read(cptr,numbytes,0)!= numbytes){
		printf("ReadHeader::Using defaultValues\n");
		SetDefaults();
		return;
	}
	int i =0;
	m_Magic[0] =  cptr[i];
	m_Magic[1] =  cptr[i+1];
	i = 2;
	m_FileVersion = (cptr[i+1]<<8) +cptr[i];

	numbytes=GetHeaderSize()+GetIVSize()+4;
	if(rw->Read(cptr,numbytes,0)!= numbytes){
		printf("ReadHeader::Unable to read the Reqd bytes \n");
		throw("ReadHeader::Unable to read the Reqd bytes");
	}
	//now Try to set all the variables properly
	i = 4;
	m_IrisSize = (cptr[i+1]<<8) +cptr[i];
	i = 6;
	m_NumRecords = (cptr[i+1]<<8) +cptr[i];
	i = 8;
	m_NumEyes = (cptr[i+1]<<8) +cptr[i];
	i = 10;
	m_IdSize  = cptr[i];
	m_MetadataSize = cptr[i+1];
	i = 12;
	m_PasswordIndex = ((unsigned short)cptr[i+1]<<8) +cptr[i];
	i = 14;
	m_PermutationKey = ((unsigned int)cptr[i+3]<<24) +((unsigned int)cptr[i+2]<<16) +((unsigned int)cptr[i+1]<<8) +cptr[i];
	i = 18;
	i = GetHeaderSize() + GetIVSize();
	m_RecEncSize = ((unsigned int)cptr[i+3]<<24) +((unsigned int)cptr[i+2]<<16) +((unsigned int)cptr[i+1]<<8) +cptr[i];
	// Lets Validate header Now.

	m_MaskSize = m_IrisSize;

	int EncSz = ((2+2+ m_IrisSize*2 + m_MaskSize*2 + m_IdSize+m_MetadataSize*2 +15)>>4)<<4;
	printf("Range ( %d %d) %d\n",EncSz - 100,EncSz + 100, m_RecEncSize);
	if(!(((EncSz + 50) > m_RecEncSize) &&((EncSz - 50) < m_RecEncSize))){
		printf("ReadHeader::EncSize is not proper \n");
		throw("ReadHeader::EncSize is not proper");
	}

	m_CompactDB = NONCOMPACTDB;
	m_OffsetForFine = 0;
	if((m_FileVersion >= 0x3)&&(m_IrisSize == 640)){
		m_CompactDB = COMPACTDB;
	}
	else if((m_FileVersion >= 0x4)&&(m_IrisSize == 960)){
		m_CompactDB = SEMICOMPACTDB;
		m_OffsetForFine = (m_IrisSize)/3;
	}
}

int IrisDBHeader::GetIrisSizeForMatcher(){
	int irissz = m_IrisSize;
	if(m_CompactDB == COMPACTDB)
		irissz = irissz*2;
	else if(m_CompactDB == SEMICOMPACTDB)
		irissz = (irissz*4)/3;
	return irissz;
}

void IrisDBHeader::WriteHeader(ReaderWriter * rw){
	int i =0;
	rw->Write(&m_Magic[0],1,i);
	i+=1;
	rw->Write(&m_Magic[1],1,i);
	i+=1;
	rw->Write((unsigned char *)&m_FileVersion,2,i);
	i+=2;
	rw->Write((unsigned char *)&m_IrisSize,2,i);
	i+=2;
	rw->Write((unsigned char *)&m_NumRecords,2,i);
	i+=2;
	rw->Write((unsigned char *)&m_NumEyes,2,i);
	i+=2;
	rw->Write((unsigned char *)&m_IdSize,1,i);
	i+=1;
	rw->Write((unsigned char *)&m_MetadataSize,1,i);
	i+=1;
	rw->Write((unsigned char *)&m_PasswordIndex,2,i);
	i+=2;
	rw->Write((unsigned char *)&m_PermutationKey,4,i);
	i+=4;
}

void IrisDBHeader::CopyHeader(IrisDBHeader *ptr){
	ptr->m_Magic[0] = m_Magic[0];
	ptr->m_Magic[1] = m_Magic[1];
	ptr->m_FileVersion = m_FileVersion;
	ptr->m_IrisSize = m_IrisSize;
	ptr->m_NumRecords = m_NumRecords;
	ptr->m_NumEyes = m_NumEyes;
	ptr->m_IdSize = m_IdSize;
	ptr->m_MetadataSize = m_MetadataSize;
	ptr->m_PasswordIndex = m_PasswordIndex;
	ptr->m_PermutationKey = m_PermutationKey;
	ptr->m_RecEncSize = m_RecEncSize;
	ptr->m_CompactDB = m_CompactDB;
	ptr->m_MaskSize = m_MaskSize;
	ptr->m_OffsetForFine = m_MaskSize;
}

void IrisDBHeader::WriteIV(ReaderWriter * rw,unsigned char *IV,int pos){
	rw->Write(IV,(int)GetIVSize(),pos);
}

void IrisDBHeader::WriteRecSz(ReaderWriter * rw,int recsz,int pos){
	rw->Write((unsigned char*)&recsz,4,pos);
}

void IrisDBHeader::WriteEncRecord(ReaderWriter * rw,unsigned char *rec,int pos){
	rw->Write(rec,m_RecEncSize,pos);
}

bool IrisDBHeader::ReadHeader(ReaderWriter *RW){
	ReadAndParseHeader(RW);
	ReadKeys(m_KeyFilename);
	return true;
}

bool IrisDBHeader::ReadHeader(char *fname){
	ReaderWriter *RW = (ReaderWriter*)new FileRW(fname,0);
	ReadAndParseHeader(RW);
	char Buff[400];
    sprintf(Buff, "%s.keys", fname);
	ReadKeys(Buff);
	delete RW;
	return true;
}

bool IrisDBHeader::ReadHeader(char *dbptr,char *keyptr){
	ReaderWriter *RW = (ReaderWriter*)new MemRW(dbptr);
	ReadAndParseHeader(RW);
	ReadKeys(keyptr,1);
	delete RW;
	return true;
}

bool IrisDBHeader::IsDBCorrupt(const char* fname)
{
	bool bCorrupt = false;
	int fsz = FileSize(fname);
	if(fsz<=18){
		printf("Header Size too low to use\n");
		char szNewCorruptDB[256] = {0};
		strcpy(szNewCorruptDB,fname);
		strcat(szNewCorruptDB,".corrupt");
		rename(fname,szNewCorruptDB);
	}
	printf("Check: if db is corrupt : (%d*%d + %d)!= %d \n",this->GetOneRecSizeinDBFile(),this->GetNumRecord(),this->GetHeaderSize(),fsz);
	if(this->GetOneRecSizeinDBFile()*this->GetNumRecord()+this->GetHeaderSize() != fsz){
		printf("Check: File size does not match to the header\n");
		bCorrupt=true;
	}

	if(this->GetMagic() != 0x6431){
		printf("Check: Magic in header does not match\n");
		bCorrupt=true;
	}

	return bCorrupt;
}
char * getNameFromID(const char * id)
{
	return 0;
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	int count = 0;
 //open the DB
#ifndef HBOX_PG
	rc = sqlite3_open("test.db", &db);
#else
	rc = sqlite3_open("test.db3", &db);
#endif
	if( rc ){
	  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  sqlite3_close(db);
	  return(0);
	}

	sqlite3_stmt * xstmt;
	char * sql1 = "select name from person where pers_id='%s';";
	char * sql2 = "select name from person where acd = x%s;";
	char theSQL[256];
	sprintf(theSQL, sql1, id);

	rc = sqlite3_prepare_v2(db, theSQL, strlen(theSQL), &xstmt, 0);
	if(rc)
	{
		fprintf(stderr, "statement didnt take: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}
	rc = sqlite3_reset(xstmt);
	rc = sqlite3_step(xstmt);
	char * retdata = 0;

	if(rc == SQLITE_ROW)
	{
		//0 = data,
		//1 = bitlen

		const char * preret = (const char *)sqlite3_column_text(xstmt, 0);

		retdata = new char[strlen(preret)];
		strcpy(retdata, preret);

		//printf("count is %d\n", count);
	}
	if(retdata ==0)
	{
		//try to see if it's a pac id
		sprintf(theSQL, sql2, id);
		sqlite3_finalize(xstmt);

		rc = sqlite3_prepare_v2(db, theSQL, strlen(theSQL), &xstmt, 0);
		rc = sqlite3_reset(xstmt);
		rc = sqlite3_step(xstmt);
		if(rc == SQLITE_ROW)
			{
				//0 = data,
				//1 = bitlen

				const char * preret = (const char *)sqlite3_column_text(xstmt, 0);

				retdata = new char[strlen(preret)];
				strcpy(retdata, preret);

				//printf("count is %d\n", count);
			}

	}

	printf("getting name %s\n", retdata);
	rc = sqlite3_finalize(xstmt);
	sqlite3_close(db);
	return retdata;
	//endpoint must delete this data
}
char * getACSTestData(int & bytes, int & bitlen)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	int count = 0;
 //open the DB
#ifndef HBOX_PG
	rc = sqlite3_open("./test.db", &db);
#else	
	rc = sqlite3_open("test.db3", &db);
#endif	
	if( rc ){
	  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  sqlite3_close(db);
	  return(0);
	}

	sqlite3_stmt * xstmt;
	char * sql1 = "select data, length from acsTestData";

	rc = sqlite3_prepare_v2(db, sql1, strlen(sql1), &xstmt, 0);
	if(rc)
	{
		fprintf(stderr, "statement didnt take: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}
	rc = sqlite3_reset(xstmt);
	rc = sqlite3_step(xstmt);
	char * retdata = 0;

	if(rc == SQLITE_ROW)
	{
		//0 = data,
		//1 = bitlen

		bytes = sqlite3_column_bytes(xstmt, 0);
		bitlen = sqlite3_column_int(xstmt, 1);
		retdata = new char[bytes+2];
		const void * acsbytes = sqlite3_column_blob(xstmt, 0);
		memcpy(retdata+2, acsbytes, bytes);
		short sbitlen = (short)sbitlen;
		//memcpy(retdata, &sbitlen,2);
		*((short*)retdata) = bitlen;
		//printf("count is %d\n", count);
	}
	printf("verify data>");
	for(int i = 0; i < bytes + 2; i++)
		printf("%x ", retdata[i]);
	printf(" Len %d bitlen %d", *((short*)retdata), bitlen);
	rc = sqlite3_finalize(xstmt);
	sqlite3_close(db);
	return retdata;
	//endpoint must delete this data
}

char * getACSDisplayTestData()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	int count = 0;
	char* str = NULL;


 //open the DB
#ifndef HBOX_PG 
	rc = sqlite3_open("test.db", &db);
#else
	rc = sqlite3_open("test.db3", &db);
#endif
	if( rc ){
	  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  sqlite3_close(db);
	  return(0);
	}

	sqlite3_stmt * xstmt;
	char * sql1 = "select display from acsTestData";

	rc = sqlite3_prepare_v2(db, sql1, strlen(sql1), &xstmt, 0);
	if(rc)
	{
		fprintf(stderr, "statement didnt take: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}
	rc = sqlite3_reset(xstmt);
	rc = sqlite3_step(xstmt);
	char * retdata = 0;

	if(rc == SQLITE_ROW)
	{
		str = new char[100];
		const char *acs = (const char *)sqlite3_column_text(xstmt, 0);
		strcpy(str, acs);
	}
	if(str)printf("test data string ; %s\n", str);
	rc = sqlite3_finalize(xstmt);
	sqlite3_close(db);
	return str;
}



int getNumUsers()
{
	 sqlite3 *db;
			char *zErrMsg = 0;
			int rc;
			int count = 0;
		 //open the DB
#ifndef HBOX_PG		 
			rc = sqlite3_open("test.db", &db);
#else			
			rc = sqlite3_open("test.db3", &db);
#endif
			if( rc ){
			  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			  sqlite3_close(db);
			  return(0);
			}

			sqlite3_stmt * xstmt;
			//char * sql1 = "select count(pers_id) from person;"; // select count(pers_id) from person
			//char * sql2 = "select name, leftIris, rightIris, acd from person;";
			char * sql2 = "select name from person;";
			rc = sqlite3_prepare_v2(db, sql2, strlen(sql2), &xstmt, 0);
			if(rc)
			{
				fprintf(stderr, "statement didnt take: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return 0;
			}
			//rc = sqlite3_reset(xstmt);
			rc = sqlite3_step(xstmt);
			while(rc == SQLITE_ROW)
			{
				const unsigned char *name = sqlite3_column_text(xstmt, 0);

				if (strstr((const char *)name, "emptyxxx") == NULL) {
					count++;
					//printf("db name 2 %s, %d\n", name, count);
				}
				rc = sqlite3_step(xstmt);
			}
			rc = sqlite3_finalize(xstmt);
			sqlite3_close(db);
			return count;
}


char * sqlitePackDB(int & length, int & xcount, bool headerOnly)
  {
	 sqlite3 *db;
		char *zErrMsg = 0;
		int rc;
		int count = 0;
	 //open the DB
#ifndef HBOX_PG	
		rc = sqlite3_open("test.db", &db);
#else	
		rc = sqlite3_open("test.db3", &db);
#endif
		if( rc ){
		  fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		  sqlite3_close(db);
		  return(0);
		}

		sqlite3_stmt * xstmt;
		char * sql1 = "select count(leftIris) from person;";
		char * sql2 = "select name, leftIris, rightIris, acd, acdlen from person;";
		rc = sqlite3_prepare_v2(db, sql1, strlen(sql1), &xstmt, 0);
		if(rc)
		{
			fprintf(stderr, "statement didnt take: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return 0;
		}
		rc = sqlite3_reset(xstmt);
		rc = sqlite3_step(xstmt);
		if(rc == SQLITE_ROW)
		{
			count = sqlite3_column_int(xstmt, 0);
			//printf("count is %d\n", count);
		}
		rc = sqlite3_finalize(xstmt);

		rc = sqlite3_prepare_v2(db, sql2,strlen(sql2), &xstmt, 0);
		if(rc)
		{
			fprintf(stderr, "statement didn't take %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return 0;
		}

		rc = sqlite3_reset(xstmt);
//fail C++:  (a>b)?(a):(b)*2*2560...   it took everything after the colon as part B, not just the 2... lol fail
		int retlen = ((count > 2)?(count):(2)) * 2 * (2560+500);
		//printf("alloc len %d\n", retlen);
		char * ret = new char[retlen];
		rc = sqlite3_step(xstmt);
		int maxNameLen = 0;
		int maxIDLen = 0;
				while(rc == SQLITE_ROW)
				{
					//name, left iris, right iris, acd, acd bitlen
					const unsigned char * name = sqlite3_column_text(xstmt, 0);
					int namelen = strlen((const char *)name);
					if(namelen > maxNameLen)
						maxNameLen = namelen;
					int len = sqlite3_column_bytes(xstmt, 3);
					if(len > maxIDLen)
						maxIDLen = len;
					rc = sqlite3_step(xstmt);
				}
				//printf("max name len %d\n", maxNameLen);
		char * pos = ret;
		//2 bytes magic  (0)
		memcpy(pos, "d1", 2);//0
		pos += 2;
		//2 bytes version (2)
		unsigned short dbver = 2;
		memcpy(pos, (void*)&dbver, 2); //2
		pos += 2;
		//2 bytes iris code size (4)
		unsigned short irisCodeSize = 1280;
		memcpy(pos, (void *)&irisCodeSize, 2);//4
		pos+=2;
		//2 bytes record count (6)
		unsigned short recordCount = (unsigned short)(count > 2)?(count):(2) ;
		memcpy(pos,(void *)&recordCount, 2); //no wonder we can't support 100k  6
		pos +=2;
		//2 bytes total iris code count (8)
		unsigned short totalIrisCodeCount = 2*recordCount;
		memcpy(pos,(void *)&totalIrisCodeCount,2); //8
        pos += 2;
        //1 byte name + idlength + 1 byte null + 2 bytes bit (10)
		//nameLength + idLength + \0 for name + 2 bytes id bit count
		unsigned char idlen = maxNameLen + maxIDLen + 1 + 2;
		memcpy(pos, (void*)&idlen, 1); //10
		pos += 1;
		//1 byte metadata length (11)
		*pos = 1;  //11
		pos++;

		//2 bytes password index(12)
		unsigned short pwindex = 0;

		memcpy(pos, (void*)&pwindex, 2); //14
		 pos+=2;
		//4 bytes permutation key(14)
		unsigned int permKey = 1;
		memcpy(pos,(void *)&permKey,4); //14-18
		pos += 4;

		//header verify
		unsigned short magic = *((unsigned short*)(ret));
		dbver = *((unsigned short*)(ret+2));
		irisCodeSize = *((unsigned short*)(ret+4));
		recordCount = *((unsigned short*)(ret+6));
		totalIrisCodeCount = *((unsigned short*)(ret+8));
		idlen = *((unsigned char *)(ret+10));
		char metalen = *((unsigned char *)(ret+11));
		pwindex = *((unsigned short*)(ret+12));
		permKey = *((unsigned int*)(ret+14));

		//printf("Header verify magic %d dbver %d iriscodesize %d recordcount %d iriscount %d idlen %d metalen %d pwindex %d permkey %d",
		//		magic,
		//		dbver,
		//		irisCodeSize,
		//		recordCount,
		//		totalIrisCodeCount,
		//		idlen,
		//		metalen,
		//		pwindex,
		//		permKey);

		//verify start write pos (18)
		//pos should point to spot 18 at this point
		//printf("pos - startpos %d\n", pos- ret);
		rc = sqlite3_reset(xstmt);
		rc = sqlite3_step(xstmt);
		//pos = ret + 18 + 16;  //34

int encoded = 0;
int datalen = 0;
	unsigned char Prem[8000];
	PermuteServer *m_PermServer = new PermuteServer(1280, permKey);
	unsigned char m_Key[256*32];
	 const unsigned char Key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};
	 for(int j = 0;j < 256;j++){
		 memcpy(m_Key + j * 32, Key, 32);
	 }

	 AES Aes;
			Aes.SetIV(Key, 16);
			Aes.SetKey(m_Key, 32);
			while(rc == SQLITE_ROW)
			{

				//xencpos: start of whole record including IV and recordLength
				char * xEncPos = pos;
				//printf("this record encode pos %d\n", pos - ret);  //make sure this says 18 for the first record
				//memset(pos, 0, 16);
				//expected 16 bytes IV

				 memcpy(pos, &Key, 16);
					pos += 16;



				//encpos: location of record length in the record
				char * encpos = pos;
						pos += 4;

				//startpos: start of actual record.  This record will eventually be encrypted.
				char * startpos = pos; //0

				//2 bytes iris count (as if we'd have 65,000 irises for someone)
				//printf("pos - startpos %d\n", pos- startpos);
				unsigned short irisCount = 2;
				memcpy(pos, (void*)&irisCount, 2);
				pos += 2; //2
				//2 bytes reserved
				//printf("pos - startpos %d\n", pos- startpos);
				memset(pos,0,2);
				pos += 2;  //4
				//printf("pos - startpos %d\n", pos- startpos);

				//n bytes iris 1
				int len0 = sqlite3_column_bytes(xstmt, 1);
				const void * byteptr = sqlite3_column_blob(xstmt,1);



				//permute the code
				m_PermServer->Permute((unsigned char*)byteptr, (unsigned char*)byteptr + 1280 , Prem);



				memcpy(pos, Prem, sizeof(char)*len0);
				pos += len0; //2564
				//printf("pos - startpos %d\n", pos- startpos);
				//printf("irisRecLen %d\n", len0);
				//n bytes iris 2
				len0 = sqlite3_column_bytes(xstmt,2);
				byteptr = sqlite3_column_blob(xstmt,2);

				//permute the code
				m_PermServer->Permute((unsigned char*)byteptr, (unsigned char*)byteptr + 1280 , Prem);

				memcpy(pos, Prem, sizeof(char) * len0);
				pos += len0; //5124
				//printf("pos - startpos %d\n", pos- startpos);
				//bitlength rounded up (ACD length * 8) 2 bytes
				//ACD
				//0 = name, 1 = left iris, 2 = right iris, 3 = acd, 4 = acdlen
				len0 = sqlite3_column_bytes(xstmt,3);
				byteptr = sqlite3_column_blob(xstmt, 3);
				int ibitlen = sqlite3_column_int(xstmt, 4);
				//2 bytes access control data bitlength:  len*8
				unsigned short bitlength = ibitlen;
				memcpy(pos, (void*)&bitlength, 2);
				pos += 2;  //5126
				//printf("pos - startpos %d\n", pos- startpos);
				//n bytes ACD (probably 4 most of the time)
				memcpy(pos, byteptr, len0);
				pos += len0;//5126+bitlen (4) 5130
				//printf("pos - startpos %d\n", pos- startpos);
				//n bytes name
				const unsigned char * nameptr = sqlite3_column_text(xstmt, 0);
				memcpy(pos, nameptr, strlen((const char *)nameptr));
				pos += strlen((const char *)nameptr);
				//printf("pos - startpos %d\n", pos- startpos);
				int pad = maxNameLen - strlen((const char *)nameptr);
				// nameLength - n bytes padding
				if(pad > 0)
				{
					memset(pos, 0, pad);
					pos += pad;
				}
				//printf("pos - startpos %d\n", pos- startpos);

				//1 bytes null terminator for name (I padded with 0s so that should be fine)
				*pos = 0;
				pos ++;
				//printf("pos - startpos %d\n", pos- startpos);

				//2 bytes metadata: all metadata is 1 byte with 0 in it
				memset(pos, 0, 2);
				pos += 2;
				//n bytes padding

				 int iRawRecordSize = pos - startpos;
				int iRawRecordPadding = iRawRecordSize % 16;
				//printf("recordpad = %d\n", iRawRecordPadding);
				if (iRawRecordPadding == 0)
					iRawRecordPadding = 16;
				else iRawRecordPadding = 16 - iRawRecordPadding;
				//printf("recordpad = %d\n", iRawRecordPadding);
				memset(pos,0, iRawRecordPadding);
				pos += iRawRecordPadding;

				//calclate the record length
				//printf("pos - startpos %d\n", pos- startpos);
				int enclen = pos - startpos;
				//printf("enclen is %d\n", enclen);
				//write the record length

				//encrypt ALL of that
				int cryptLength = Aes.Encrypt((const unsigned char*)(startpos),Prem,enclen);
				//write the encrypted data to position after startlen

				memcpy(startpos, Prem, cryptLength);
				//write the cryptlength to the record length
				memcpy(encpos, (void*)&cryptLength, 4);
				//adjust POS location to point to the spot right after the crypto is entered
				pos = startpos + cryptLength;
				if(count == 1 && encoded == 0)
				{
					rc = sqlite3_reset(xstmt);  //replay the same one only if it's 0
					//datalen = enclen;
				}
					rc = sqlite3_step(xstmt);
					//printf("Record total encoded %d\n", pos - xEncPos);
				encoded++;
			}

		printf("verify encsize %d\n", *((int*)(ret + 18+16)));
		sqlite3_finalize(xstmt);
	//start executing
		//total length of file stored
		length = pos - ret;
		sqlite3_close(db);
		//number of records in the file
		//printf("permserver datasize: %d\n", sizeof(m_PermServer)); //won't be accurate but will show what I probably missed.
		delete m_PermServer;
		printf("length is %d count is %d encoded is %d\n", length,count, encoded);
		return ret;
  }
