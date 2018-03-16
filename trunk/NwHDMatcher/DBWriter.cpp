#include "DBWriter.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "IrisDBHeader.h"
#include "FileRW.h"
#include "PermRW.h"
#include "PermuteServer.h"
#include <string.h>
#include <fstream>

extern "C" {
#include "file_manip.h"
}

#ifdef __ANDRIOD__
	#include "logging.h"
	#define printf LOGI
#endif

DBWriter::DBWriter(){

}

DBWriter::~DBWriter(){

}


int DBWriter::WriteHeader(IrisDBHeader *irisDBHeader,unsigned char* ptr, int inc){

	ptr[0] = 'd';
	ptr[1] = '1';
	ptr[2] = irisDBHeader->GetFileVersion();
	ptr[3] = 0;
	ptr[4] = irisDBHeader->GetIrisSize()&0xFF;
	ptr[5] = (irisDBHeader->GetIrisSize()&0xFF00)>>8;

	int numrec = irisDBHeader->GetNumRecord()+inc;
	ptr[6] = numrec&0xFF;
	ptr[7] = (numrec&0xFF00)>>8;

	int numeyes = irisDBHeader->GetNumEyes()+2*inc;
	ptr[8] = numeyes&0xFF;
	ptr[9] = (numeyes&0xFF00)>>8;

	ptr[10] = irisDBHeader->GetF2FSize()&0xFF;

	ptr[11] = irisDBHeader->GetMetaDataSize()&0xFF;

	ptr[12] = irisDBHeader->GetPasswordIndex()&0xFF;
	ptr[13] = (irisDBHeader->GetPasswordIndex()&0xFF00)>>8;

	ptr[14] = irisDBHeader->GetPermutationKey()&0xFF;
	ptr[15] = (irisDBHeader->GetPermutationKey()&0xFF00)>>8;
	ptr[16] = (irisDBHeader->GetPermutationKey()&0xFF0000)>>16;
	ptr[17] = (irisDBHeader->GetPermutationKey()&0xFF000000)>>24;
	return 0;
}


void DBWriter::ExtractDB(char *& dbfile, IrisDBHeader *& irisDBHeader,unsigned char *DB)
{
    ReaderWriter *reader = new FileRW((char*)(dbfile), irisDBHeader->GetHeaderSize());
    PermRW dbRdr(new AesRW(reader));
    dbRdr.Init(irisDBHeader);
    int bytes = 0;
    for(int i = 0;i < irisDBHeader->GetNumRecord();i++){
        bytes += dbRdr.Read(DB + i * irisDBHeader->GetOneRecSizeinDB(), irisDBHeader->GetOneRecSizeinDBFile(), i * irisDBHeader->GetOneRecSizeinDBFile());
//        printf("Extracted DB ");
//        unsigned char *key = DB + 4 + i * (irisDBHeader->GetOneRecSizeinDB());
//        for(int k=0;k<4;k++){
//        	printf(" %d ",key[k*irisDBHeader->GetIrisSize()]);
//        }
//        printf("\n");
    }
}

int  DBWriter::FindInDB(IrisDBHeader *& irisDBHeader, unsigned char *DB, char *& name)
{
	int foundIndx = -1;
    //Try To find a match in the DB looking at the Names.
    for(int i = 0;i < irisDBHeader->GetNumRecord();i++){
        unsigned char *key = DB + 4 + irisDBHeader->GetIrisSize() * 4 + i * (irisDBHeader->GetOneRecSizeinDB());

        char buff[100]={0};
        memcpy(buff,key+2,irisDBHeader->GetF2FSize()-2);
        //printf("STRCMP %s %s \n",name,buff);
        //if(0 == memcmp(name,(const char *)key + 2,irisDBHeader->GetF2FSize()-2))
        if(0 == strcmp(name,(const char *)key + 2)){
            foundIndx = i;
            printf("Found an Index Matching @ %d \n", i);
        }
    }
    return foundIndx;
}


int DBWriter::WriteHeader(char*fname,short numrec, short numeyes, bool Noperm,int irissz,int filever){
	FILE *fp = fopen(fname,"wb");
	if(fp == NULL){
		printf("Unable to create file %s\n",fname);
		return -2;
	}

	unsigned char magicd = 'd';
	unsigned char magic1 = '1';
	fwrite(&magicd,1,1,fp); //Magic
	fwrite(&magic1,1,1,fp); //Magic
	unsigned short filver = filever;
	fwrite(&filver,1,2,fp); // file ver
	unsigned short iriscodesize = irissz;
	fwrite(&iriscodesize,1,2,fp); //iris code size
	fwrite(&numrec,1,2,fp); // num record
	fwrite(&numeyes,1,2,fp); // num eyes
	unsigned char idsize= 20;
	fwrite(&idsize,1,1,fp); // id size
	unsigned char metadata= 0x1;
	fwrite(&metadata,1,1,fp); // metadata size
	unsigned short pwdindx = 0x0;
	fwrite(&pwdindx,1,2,fp); // pwd indx
	unsigned int key = 0x12345678;
	if(Noperm) key=0x0;
	fwrite(&key,1,4,fp); // permutation key
	fclose(fp);
	return 1;
}

static void SaveCode(char *code) {
	char filename[512];
	static int index=0;
	sprintf(filename,"/mnt/sdcard/Eyelock/RegEye_%03d.bin",index++);
    std::ofstream output(filename, std::ios::binary);
        if (output.good()) {
                output.write(code, 2560);
        }
}


int DBWriter::AppendDB(char* eye1,char* eye2,char* name,char* dbfile){
	printf("Append Db in file %s\n",dbfile);

//	SaveCode(eye1);
//	SaveCode(eye2);
	int ret = 0;
	//ERROR 0 If unable to open FILE
	FILE* f1 = fopen(dbfile,"rb");
	if(f1 == NULL){
		printf("Unable to open file %s\n",dbfile);
		return 0;
	}

	int fileSZ = FileSize(dbfile);
	printf("File Sz %d\n",fileSZ);
	if(fileSZ == 0){
		ret = -2;
//		ret = WriteHeader(dbfile,0,0);
		if(ret !=1)
			return ret; // -2 means unable to create file in the specified path
	}


	IrisDBHeader *irisDBHeader,header;
	irisDBHeader = &header;
	irisDBHeader->ReadHeader(dbfile);
	irisDBHeader->PrintAll();

	if (irisDBHeader->GetNumEyes() != 2*irisDBHeader->GetNumRecord()){
		printf("Num Iris != Num Record*2\n");
		return -1;
	}

	int EncSz;
	unsigned char *DB = (unsigned char*)((malloc(fileSZ)));

    ExtractDB(dbfile, irisDBHeader,DB);
    int foundIndx = FindInDB(irisDBHeader, DB, name);

    unsigned char Buff[50000];
    int buffcnt = 0;
    unsigned char Enc[6000];

    int iriscodesize = irisDBHeader->GetIrisSize();
    const unsigned char Key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};
    memcpy(&Buff[buffcnt], &Key, irisDBHeader->GetIVSize());
    buffcnt += irisDBHeader->GetIVSize();

    int irisCodePos = 0;
    unsigned char irisCode[7000];
    irisCodePos = 0;
    irisCode[irisCodePos++] = 2;
    irisCode[irisCodePos++] = 0;
    irisCode[irisCodePos++] = 0;
    irisCode[irisCodePos++] = 0;
    memcpy((unsigned char*)((&irisCode[irisCodePos])), (unsigned char*)((eye1)), irisDBHeader->GetIrisSize() + irisDBHeader->GetMaskSize());
    irisCodePos += (irisDBHeader->GetIrisSize() + irisDBHeader->GetMaskSize());
    memcpy((unsigned char*)((&irisCode[irisCodePos])), (unsigned char*)((eye2)), irisDBHeader->GetIrisSize() + irisDBHeader->GetMaskSize());
    irisCodePos += (irisDBHeader->GetIrisSize() + irisDBHeader->GetMaskSize());
    for(int l = 0;l < 2;l++){
        irisCode[irisCodePos++] = 0;
    }

    memset(&irisCode[irisCodePos],0,irisDBHeader->GetF2FSize() + irisDBHeader->GetMetaDataSize());

    int width = strlen(name);
    if(width > (irisDBHeader->GetF2FSize()-2)){
    	width = irisDBHeader->GetF2FSize()-4;
    }
    for(int l = 2;l < (width+2);l++){
        irisCode[irisCodePos++] = name[l - 2];
    }

    int totalfeaturelength = (irisDBHeader->GetIrisSize() + irisDBHeader->GetMaskSize()) * 2;
    EncSz = ((2 + 2 + totalfeaturelength + irisDBHeader->GetF2FSize() + irisDBHeader->GetMetaDataSize() * 2 + 15) >> 4) << 4;
    printf("Enc Size %d \n", EncSz);

    int garbagesz = EncSz - irisCodePos;
    for(int j = 0;j < garbagesz;j++){
        irisCode[irisCodePos++] = 0;
    }
    printf("check is Permute to be done\n");
    //Permute from here..
    unsigned char Prem[8000];
    if(!(irisDBHeader->GetPermutationKey()== 0)){
        PermuteServer *m_PermServer = new PermuteServer(iriscodesize, irisDBHeader->GetPermutationKey());
    	memcpy(Prem, irisCode, 4); // copy 02 00 00 00
		m_PermServer->Permute(irisCode + 4, irisCode + iriscodesize + 4, Prem + 4);
		m_PermServer->Permute(irisCode + iriscodesize * 2 + 4, irisCode + iriscodesize * 3 + 4, Prem + iriscodesize * 2 + 4);
		int frmf2f = EncSz - iriscodesize * 4 + 4;
		if(frmf2f < 0)
			frmf2f = 0;

		memcpy(Prem + iriscodesize * 4 + 4, irisCode + iriscodesize * 4 + 4, frmf2f);

		//test it to verify permute
		unsigned char buff0[2560], buff1[2560];
		m_PermServer->Recover(Prem + 4, buff0, buff1);
		if(0 == memcmp(buff0, irisCode + 4, iriscodesize)){
			printf("Perm Verified1\n");
		}
		if(0 == memcmp(buff1, irisCode + iriscodesize * 1 + 4, iriscodesize)){
			printf("Perm Verified2\n");
		}
		m_PermServer->Recover(Prem + 4 + iriscodesize * 2, buff0, buff1);
		if(0 == memcmp(buff0, irisCode + iriscodesize * 2 + 4, iriscodesize)){
			printf("Perm Verified3\n");
		}
		if(0 == memcmp(buff1, irisCode + iriscodesize * 3 + 4, iriscodesize)){
			printf("Perm Verified4\n");
		}
		delete m_PermServer;
	}
    AES Aes;
    Aes.SetIV(Key, irisDBHeader->GetIVSize());
    Aes.SetKey(irisDBHeader->GetKey(), 32);

    unsigned char *EncInput = NULL;
    if(irisDBHeader->GetPermutationKey()==0){
    	printf("NO Permutation\n");
    	EncInput = irisCode;
    }else{
    	EncInput = Prem;
    	printf("With Permutation\n");
    }
    Aes.Encrypt((const unsigned char*)(EncInput),Enc,EncSz);

	unsigned int recsize = EncSz;
	memcpy(&Buff[buffcnt],&recsize,4);
	buffcnt+=4;
	memcpy(&Buff[buffcnt],&Enc,EncSz);
	buffcnt+=EncSz;

// Now DB is Ready To Be Written To a file:

	unsigned char Header[20];

	char fname[200];
	sprintf(fname,"%s.tmp",dbfile);

	FileRW *fw = new FileRW(fname,0);
	FileRW *fr = new FileRW(dbfile,0);

	int offset = 0;
	int addrec = 0;
	if(foundIndx == -1) addrec = 1;
	WriteHeader(irisDBHeader,Header,addrec);
	//WriteHeader
	fw->Write(Header,irisDBHeader->GetHeaderSize(),0);
	fr->Read(Header,irisDBHeader->GetHeaderSize(),0);
	offset += irisDBHeader->GetHeaderSize();

	for(int i = 0;i < irisDBHeader->GetNumRecord();i++){
		fr->Read(irisCode,irisDBHeader->GetOneRecSizeinDBFile(),offset);
		if(foundIndx == i){
			fw->Write(Buff,irisDBHeader->GetOneRecSizeinDBFile(),offset);
		}else
			fw->Write(irisCode,irisDBHeader->GetOneRecSizeinDBFile(),offset);
		offset+=irisDBHeader->GetOneRecSizeinDBFile();
	}
	if(foundIndx == -1)
		fw->Write(Buff,irisDBHeader->GetOneRecSizeinDBFile(),offset);

	delete(fw);
	delete(fr);
	free(DB);

	rename(fname,dbfile);
//	FILE *fp = fopen(dbfile,"rb");
//	fseek(fp,18,SEEK_SET);
//	unsigned char buf[32];
//	int recsz=EncSz;
//	fread(buf,irisDBHeader->GetIVSize(),1,fp);
//	if(0 == memcmp(buf,Key,irisDBHeader->GetIVSize()))
//		printf("Enc1 Success\n");
//	fread(&recsz,4,1,fp);
//	if(recsz == EncSz){
//		printf("RecSz Success\n");
//	}
//	fseek(fp,18+irisDBHeader->GetIVSize()+4+EncSz,SEEK_SET);
//
//	fread(buf,irisDBHeader->GetIVSize(),1,fp);
//	if(0 == memcmp(buf,Key,irisDBHeader->GetIVSize())){
//		printf("Enc2 Success\n");
//	}
//	fread(&recsz,4,1,fp);
//	if(recsz == EncSz){
//		printf("RecSz Success\n");
//	}
//	fclose(fp);
	fileSZ = FileSize(dbfile);
	printf("File Sz %d\n",fileSZ);
	printf("Done::exiting Out of DB Writer\n");
	return 1;

}

