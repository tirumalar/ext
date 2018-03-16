/*
 * ReadDBHeader.h
 *
 *  Created on: 03-May-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#ifndef READDBHEADER_H_
#define READDBHEADER_H_

#include "stdio.h"
#ifdef __ANDROID__
#include "logging.h"
#define printf LOGI
#endif

class ReaderWriter;
enum {NONCOMPACTDB=0,COMPACTDB,SEMICOMPACTDB};
class IrisDBHeader {
public:
	IrisDBHeader();
	IrisDBHeader(char *fname);
	virtual ~IrisDBHeader();
	bool ReadHeader(ReaderWriter *rw);
	bool ReadHeader(char *fname);
	bool ReadHeader(char *dbptr,char *keyptr);
	bool ReadHeaderRemote(char *BuffPtr,char *keyfilename);
	int GetHeaderSize() { return 18;} // later make it depend on file version
	void CopyHeader(IrisDBHeader *ptr);
	void WriteHeader(ReaderWriter * rw);
	void WriteIV(ReaderWriter * rw,unsigned char *IV,int pos);
	void WriteRecSz(ReaderWriter * rw,int recsz,int pos);
	void WriteEncRecord(ReaderWriter * rw,unsigned char *rec,int pos);

	unsigned short GetMagic(){return (m_Magic[0]<<8)+(m_Magic[1]);}
	unsigned short GetFileVersion(){return m_FileVersion;}
	unsigned short GetIrisSize(){return m_IrisSize;}
	unsigned short GetMaskSize(){return m_MaskSize;}
	unsigned int GetFeatureLength(){return m_IrisSize+m_MaskSize;}
	unsigned short GetNumRecord(){return m_NumRecords;}
	unsigned char GetF2FSize(){return m_IdSize;}
	//unsigned char GetIDSize(){return 0;}
	unsigned char GetMetaDataSize(){return m_MetadataSize;}
	unsigned short GetPasswordIndex(){return m_PasswordIndex;}
	unsigned int GetPermutationKey(){return m_PermutationKey;}
	unsigned int GetRecEncSize(){return m_RecEncSize;}
	unsigned char* GetKey(){return m_Key +(m_PasswordIndex*32);}
	int GetOneRecSizeinDB(){return 2+2+((((m_IrisSize*2 + m_MaskSize*2 +m_IdSize+2*m_MetadataSize)+3)>>2)<<2);}
	int GetOneRecSizeinDBFile(){return GetIVSize()+4+m_RecEncSize;}
	unsigned short GetNumEyes(){ return m_NumEyes;}
	int GetIVSize(){
		if(m_FileVersion == 1)return 32;
		return 16;
	}
	int GetKeySize(){ return 32;}
	int IsDBCompact(){ return m_CompactDB;}
	void SetNumRecord(unsigned short inp){m_NumRecords= inp;}
	void SetNumEyes(unsigned short inp){m_NumEyes= inp;}
	void SetRecEncSize(unsigned int inp){m_RecEncSize= inp;}
	int GetIrisSizeForMatcher();
	int GetOffsetForFine(){
		return m_OffsetForFine;
	}
	void PrintAll(){
		printf("Magic %c %c ",m_Magic[0],m_Magic[1]);
		printf("FileVer %#x ",m_FileVersion);
		printf("IrisSize %#x ",m_IrisSize);
		printf("MaskSize %#x ",m_MaskSize);
		printf("NumRec %#x ",m_NumRecords);
		printf("NumEyes %#x ",m_NumEyes);
		printf("IdSize %#x ",m_IdSize);
		printf("MetaDataSz %#x ",m_MetadataSize);
		printf("PWDIndx %#x ",m_PasswordIndex);
		printf("PermKey %#x ",m_PermutationKey);
		printf("RecEncSize %#x ",m_RecEncSize);
		printf("CompactDB %#x ",m_CompactDB);
		printf("FineOffset %#x \n",m_OffsetForFine);
	}
	void SetDefaults();
	bool IsDBCorrupt(const char* fname);
private:
    unsigned char m_Magic[2];
    unsigned short m_FileVersion;
    unsigned short m_IrisSize;
    unsigned short m_MaskSize;
    unsigned short m_OffsetForFine;
    unsigned short m_NumEyes;
    unsigned short m_NumRecords;
    unsigned char m_IdSize;
    unsigned char m_MetadataSize;
    unsigned short m_PasswordIndex;
    unsigned short m_CompactDB;
    unsigned int m_PermutationKey;
    unsigned int m_RecEncSize;
    unsigned char m_Key[8192];
    char *m_KeyFilename;

protected:
    void ReadKeys(char *fname,int memio=0);
    void ReadAndParseHeader(ReaderWriter * rw);

};
char * getNameFromID(const char* id);
char * getACSTestData(int & bytes, int & bitlen);
char * getACSDisplayTestData();
char * sqlitePackDB(int & length, int & xcount, bool hdrOnly);
int getNumUsers();
#endif /* READDBHEADER_H_ */
