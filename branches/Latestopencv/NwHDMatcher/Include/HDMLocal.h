/*
 * HDMLocal
 *
 *  Created on: 26-Feb-2010
 *      Author: akhil
 */

#ifndef HDMLOCAL_H_
#define HDMLOCAL_H_
#include <stdio.h>

class HDMatcher;
class IrisDBHeader;
class IrisData;
class HDMLocal: public HDMatcher {
public:
	HDMLocal(int size,int id,bool useCoarseFine,int irissz = 1280);
	virtual ~HDMLocal();
	virtual bool StartMatch(unsigned char *iriscode, int taskid);
	virtual void StartMatch(unsigned char *iriscode, int taskid, int *numdenbuf);
	virtual void SetResultMatch(std::pair<int,float> result,int taskid);

	virtual void AssignDB(char *fname,int memio=0);
	virtual void AssignDB(DBAdapter *dbAdapter);
	virtual void StartMatchInterface(int shift,bool greedyMatch,float threshold, float coarseThreshold,int irissz=1280);
	virtual bool UpdateSingleUserOnly(unsigned char * perid,unsigned char * leftiris,unsigned char * rightiris);
	virtual bool AddSingleUserOnly(string perid,string leftiris,string rightiris);
	virtual bool DeleteSingleUserOnly(unsigned char *guid);
	virtual bool FindSingleUserOnly(unsigned char *guid);
	virtual void PrintGUIDs();
#ifndef UNITTEST
protected:
#endif
	void DoMatch(unsigned char *iriscode, int taskid);
	unsigned char* m_data; // initialize it
	unsigned char* m_CoarseDb;
	int m_type;
};

#endif /* HDMLOCAL_H_*/
