/*
 * DBWriter.h
 *
 *  Created on: May 13, 2011
 *      Author: developer1
 */

#ifndef DBWRITER_H_
#define DBWRITER_H_

class IrisDBHeader;
class DBWriter {
public:
	DBWriter();
	virtual ~DBWriter();
	int AppendDB(char* eye1,char* eye2,char* name,char* dbfile);
	int WriteHeader(IrisDBHeader *irisDBHeader,unsigned char* ptr,int inc);
private:
	int WriteHeader(char*fname,short numrec, short numeyes, bool Noperm=false,int irissz=1280,int filever=2);
    void ExtractDB(char *& dbfile, IrisDBHeader *& irisDBHeader, unsigned char* DB);
    int FindInDB(IrisDBHeader *& irisDBHeader, unsigned char *DB, char *& name);
};

#endif /* DBWRITER_H_ */
