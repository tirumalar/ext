#include "DBAdapter.h"
#include <stdio.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include "UtilityFunctions.h"
#include "logging.h"
#include "DBMap.h"
#include <unistd.h>
#define HBOX_PG
using namespace soci;
using namespace std;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

const char logger[30] = "DBAdapter";
DBAdapter::DBAdapter():m_sqlConn(NULL){
}

DBAdapter::~DBAdapter(){
	CloseConnection();
}
void DBAdapter::CloseConnection(){
	if(m_sqlConn){
		m_sqlConn->close();
		delete m_sqlConn;
		m_sqlConn=NULL;
	}
}

string DBAdapter::GetDBFileName(){
	return m_filename;
}
bool DBAdapter::ReOpenConnection(){
	return OpenFile((char*)m_filename.c_str());
}
bool DBAdapter::OpenFile(char* filename){
	CloseConnection();
	try {
#ifdef HBOX_PG
		m_sqlConn = new session(sqlite3, "test.db3");
		m_filename.assign("test.db3");
#else
		m_sqlConn = new session(sqlite3, filename);
		m_filename.assign(filename);
#endif
		return true;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s",e.what());
	}
	return false;
}

int DBAdapter::CheckIntegrity(){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	int result = -1;
	string trOut;
	try {
		EyelockLog(logger, DEBUG, "integrity check for %s", m_filename.c_str());
		*(m_sqlConn) << "PRAGMA integrity_check;", into(trOut);
		EyelockLog(logger, DEBUG, "integrity check for %s result: %s", m_filename.c_str(), trOut.c_str());
		result = (trOut.compare("ok") == 0) ? 0 : 1;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s",e.what());
	}
	return result;
}

int DBAdapter::GetUserCount(bool excludeDummies){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	int count = -1;
	try {
		if (excludeDummies)
		{
			*(m_sqlConn) << "SELECT COUNT(*) FROM person WHERE name NOT LIKE '%emptyxxx%'", into(count);
		}
		else
		{
			*(m_sqlConn) << "SELECT COUNT(*) FROM person", into(count);
		}
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s",e.what());
	}
	return count;
}

int DBAdapter::UpdateUser(string perid,string username,string leftiris,string rightiris,string acd,int acdlen, string acdnop){

	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		int count = 0;
		blob leftiris_blob(*(m_sqlConn));
		leftiris_blob.write(0, leftiris.c_str(),leftiris.size());

		blob rightiris_blob(*(m_sqlConn));
		rightiris_blob.write(0, rightiris.c_str(),rightiris.size());

		blob acd_blob(*(m_sqlConn));
		acd_blob.write(0, acd.c_str(),acd.length());

		blob acdnop_blob(*(m_sqlConn));
		acdnop_blob.write(0, acdnop.c_str(),acd.length());

		*(m_sqlConn) << "SELECT COUNT(*) FROM person where pers_id = :p", into(count), use(perid);
		if (count) {
			EyelockLog(logger, WARN, "User_Already_Present\n");
			*(m_sqlConn) << "update person set name = :user, leftIris = :left, rightIris = :right, acd = :a, acdlen = :len, acdnop = :b where pers_id = :p",
					use(username), use(leftiris_blob), use(rightiris_blob), use(acd_blob), use(acdlen), use(acdnop_blob), use(perid);
			return 1;
		}

		transaction t(*(m_sqlConn));
		*(m_sqlConn)<< "INSERT INTO person (pers_id,name,leftIris,rightIris,acd,acdlen,acdnop) VALUES (:per_id, :name, :leftIris, :rightIris, :acd, :acdlen, :acdnop)", use(perid), use(username), use(leftiris_blob), use(rightiris_blob), use(acd_blob), use(acdlen), use(acdnop_blob);

		t.commit();
		sync();
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::UpdateUser(string perid,string username,string leftiris,string rightiris,string acd,int acdlen,string acdnop, string pin) {

	EyelockLog(logger, TRACE, "Updating user %s, ID: %s, card data: %s (length: %d), PIN: %s", username.c_str(), perid.c_str(), acd.c_str(), acdlen, pin.c_str());
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		int count = 0;
		blob leftiris_blob(*(m_sqlConn));
		leftiris_blob.write(0, leftiris.c_str(),leftiris.size());

		blob rightiris_blob(*(m_sqlConn));
		rightiris_blob.write(0, rightiris.c_str(),rightiris.size());

		blob acd_blob(*(m_sqlConn));
		acd_blob.write(0, acd.c_str(),acd.length());

		blob acdnop_blob(*(m_sqlConn));
		acdnop_blob.write(0, acdnop.c_str(),acd.length());

		*(m_sqlConn) << "SELECT COUNT(*) FROM person where pers_id = :p", into(count), use(perid);
		if (count) {
			EyelockLog(logger, WARN, "User_Already_Present\n");
			*(m_sqlConn) << "update person set name = :user, leftIris = :left, rightIris = :right, acd = :a, acdlen = :len, acdnop = :b, pindata = :pin where pers_id = :p",
					use(username), use(leftiris_blob), use(rightiris_blob), use(acd_blob), use(acdlen), use(acdnop_blob), use(pin), use(perid);
			return 1;
		}

		transaction t(*(m_sqlConn));
		*(m_sqlConn)<< "INSERT INTO person (pers_id,name,leftIris,rightIris,acd,acdlen,acdnop,pindata) VALUES (:per_id, :name, :leftIris, :rightIris, :acd, :acdlen, :acdnop, :pin)", use(perid), use(username), use(leftiris_blob), use(rightiris_blob), use(acd_blob), use(acdlen), use(acdnop_blob), use(pin);

		t.commit();
		sync();
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::GetUserData(string perid,string& username,string& leftiris,string& rightiris,string& acd,int& acdlen,string& acdnop){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	int ret = 0;
	try {
		blob acd_blob(*(m_sqlConn));
		blob acdnop_blob(*(m_sqlConn));
		blob left_blob(*(m_sqlConn));
		blob right_blob(*(m_sqlConn));

		*(m_sqlConn)<< "SELECT name,leftIris,rightIris,acd,acdlen,acdnop From person WHERE pers_id = :userID", into(
				username), into(left_blob), into(right_blob), into(acd_blob), into(acdlen), into(acdnop_blob),use(perid);

		acd.resize(acd_blob.get_len());
		acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());

		acdnop.resize(acdnop_blob.get_len());
		acdnop_blob.read(0, (char*)acdnop.c_str(),acdnop_blob.get_len());

		leftiris.resize(left_blob.get_len());
		left_blob.read(0, (char*)leftiris.c_str(),left_blob.get_len());

		rightiris.resize(right_blob.get_len());
		right_blob.read(0, (char*)rightiris.c_str(),right_blob.get_len());
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::GetUserData(string perid,string& username,string& leftiris,string& rightiris,string& acd,int& acdlen,string& acdnop, string& pin) {

	EyelockLog(logger, TRACE, "Retrieving data of user with ID: %s", perid.c_str());
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	int ret = 0;
	try {
		blob acd_blob(*(m_sqlConn));
		blob acdnop_blob(*(m_sqlConn));
		blob left_blob(*(m_sqlConn));
		blob right_blob(*(m_sqlConn));
		indicator pinInd;

		*(m_sqlConn)<< "SELECT name,leftIris,rightIris,acd,acdlen,acdnop,pindata From person WHERE pers_id = :userID", into(
				username), into(left_blob), into(right_blob), into(acd_blob), into(acdlen), into(acdnop_blob), into(pin,pinInd), use(perid);

		acd.resize(acd_blob.get_len());
		acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());

		acdnop.resize(acdnop_blob.get_len());
		acdnop_blob.read(0, (char*)acdnop.c_str(),acdnop_blob.get_len());

		leftiris.resize(left_blob.get_len());
		left_blob.read(0, (char*)leftiris.c_str(),left_blob.get_len());

		rightiris.resize(right_blob.get_len());
		right_blob.read(0, (char*)rightiris.c_str(),right_blob.get_len());
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::GetUsername(string perid,string& username){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		int count = 0;
		*(m_sqlConn) << "SELECT COUNT(*) FROM person where pers_id = :userID", into(count), use(perid);
		if (count == 0) {
			EyelockLog(logger, DEBUG, "DBAdapter::GetUsername :: Invalid user ID %s", perid.c_str());
			return -1;
		}
		*(m_sqlConn)<< "SELECT name From person WHERE pers_id = :userID", into(username),use(perid);
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, DEBUG, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::GetUserACDData(string perid,string& acd,int& acdlen, string& acdnop){

	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	int ret = 0;
	try {
		blob acd_blob(*(m_sqlConn));
		blob acdnop_blob(*(m_sqlConn));
		*(m_sqlConn)<< "SELECT acd,acdlen,acdnop From person WHERE pers_id = :userID",into(acd_blob), into(acdlen), into(acdnop_blob), use(perid);
		acd.resize(acd_blob.get_len());
		acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());
		acdnop.resize(acdnop_blob.get_len());
		acdnop_blob.read(0, (char*)acdnop.c_str(),acdnop_blob.get_len());
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, DEBUG, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::GetUserACDData(string perid,string& acd,int& acdlen, string& acdnop, string& pin) {

	EyelockLog(logger, TRACE, "Retrieving card data and PIN of user with ID: %s", perid.c_str());
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		blob acd_blob(*(m_sqlConn));
		blob acdnop_blob(*(m_sqlConn));
		indicator pinInd;
		*(m_sqlConn)<< "SELECT acd,acdlen,acdnop,pindata From person WHERE pers_id = :userID",into(acd_blob), into(acdlen), into(acdnop_blob), into(pin,pinInd), use(perid);
		acd.resize(acd_blob.get_len());
		acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());
		acdnop.resize(acdnop_blob.get_len());
		acdnop_blob.read(0, (char*)acdnop.c_str(),acdnop_blob.get_len());
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, DEBUG, "SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::DeleteUser(string perid){
//	EyelockLog(logger, DEBUG, "DBAdapter::DeleteUser() start\n");
	if(!m_sqlConn) return -1;
	ScopeLock lock(m_lock);
	try {
		transaction t(*(m_sqlConn));
		int count = 0;
		*(m_sqlConn) << "SELECT COUNT(*) FROM person where pers_id = :p", into(count), use(perid);
		if (count) {
			*(m_sqlConn) << "DELETE from person WHERE pers_id = :p", use(perid);
			t.commit();
			sync();
			return 0;
		}else{
			return -1;
		}
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -2;
}

int DBAdapter::MakeMatchBuffer(char* matchBuffer,int len ,int count,int startindex,bool compressed,char* coarseBuffer){
	//printf("MakeMatchBuffer buffer ptr %d, len %d, count %d, startindex %d\n", matchBuffer, len, count);
	if(!m_sqlConn) return -1;
	ScopeLock lock(m_lock);
	unsigned char temp[IRIS_SIZE_INCLUDING_MASK_PER_PERSON];
	try {
		if(count*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE) > len){
			EyelockLog(logger, WARN, " Buffer not Big enough\n");
			return -1;
		}
		char *buff = matchBuffer;
		unsigned char *coarse = (unsigned char*)coarseBuffer;
		blob left_blob(*(m_sqlConn));
		blob right_blob(*(m_sqlConn));
		string perid;
		statement st = (m_sqlConn->prepare << "SELECT leftIris,rightIris,pers_id from person LIMIT :cnt OFFSET :skip", into(left_blob),into(right_blob),into(perid),use(count),use(startindex));
		st.execute();
		while (st.fetch()){
		//	EyelockLog(logger, DEBUG, "Got Blob Len left %d ,right %d GUID %d \n",left_blob.get_len(),right_blob.get_len(),perid.length());
			if(compressed){
				left_blob.read(0,(char*)temp,left_blob.get_len());
				CompressIris(temp,(unsigned char*)buff,IRIS_SIZE_INCLUDING_MASK);buff+=COMPRESS_IRIS_SIZE_INCLUDING_MASK;
				if(coarseBuffer){
					CreateCoarse(temp,coarse,IRIS_SIZE_INCLUDING_MASK);coarse +=COARSE_IRIS_SIZE_INCLUDING_MASK;
				}
				right_blob.read(0, (char*)temp,right_blob.get_len());
				CompressIris(temp,(unsigned char*)buff,IRIS_SIZE_INCLUDING_MASK);buff+=COMPRESS_IRIS_SIZE_INCLUDING_MASK;
				if(coarseBuffer){
					CreateCoarse(temp,coarse,IRIS_SIZE_INCLUDING_MASK);coarse +=COARSE_IRIS_SIZE_INCLUDING_MASK;
				}
				memcpy(buff,perid.c_str(),GUID_SIZE);buff+=GUID_SIZE;
			}else{
				left_blob.read(0,buff,left_blob.get_len());buff+=IRIS_SIZE_INCLUDING_MASK;
				right_blob.read(0, buff,right_blob.get_len());buff+=IRIS_SIZE_INCLUDING_MASK;
				memcpy(buff,perid.c_str(),GUID_SIZE);buff+=GUID_SIZE;
			}
		}
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}


vector<pair<int,string> > DBAdapter::GetIncrementalData(){
	vector<pair<int,string> > ret;
	if(!m_sqlConn) return ret;
	ScopeLock lock(m_lock);
	try {
		rowset<row> rSet = (m_sqlConn->prepare << "SELECT pers_id,updatetype from person");
		rowset<row>::const_iterator itr;
		for (itr = rSet.begin(); itr != rSet.end(); ++itr) {
			row const& r = *itr;
			pair<int,string> temp;
			temp.second = r.get<string>(0);
			temp.first = r.get<int>(1);
			ret.push_back(temp);
		}
	} catch (soci::soci_error const &err) {
		ret.clear();
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		ret.clear();
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return ret;
}



int DBAdapter::SetUpdateUser(string perid,int uptype){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		int count = 0;
		*(m_sqlConn) << "SELECT COUNT(*) FROM person where pers_id = :p", into(count), use(perid);
		if (count) {
			*(m_sqlConn) << "update person set updatetype = :update where pers_id = :p",use(uptype), use(perid);
			return 0;
		}
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::getAllUserIDs(vector<string>& outIDs){
		if(!m_sqlConn) return -1;
		ScopeLock lock(m_lock);
		try {
			rowset<row> rSet = (m_sqlConn->prepare << "SELECT pers_id from person");
			rowset<row>::const_iterator itr;
			for (itr = rSet.begin(); itr != rSet.end(); ++itr) {
				row const& r = *itr;
				string temp = r.get<string>(0);
				outIDs.push_back(temp);
			}
		return 0;
		} catch (soci::soci_error const &err) {
			outIDs.clear();
			EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
		} catch (exception const &e) {
			outIDs.clear();
			EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
		}
		return -1;
}

int DBAdapter::UpdateACS(string display,string acd,int acdlen){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {

		blob acd_blob(*(m_sqlConn));
		acd_blob.write(0, acd.c_str(),acd.length());

		*(m_sqlConn) << "update acsTestData set display = :display, data = :a, length = :len",
					use(display), use(acd_blob), use(acdlen);
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::UpdateACS(string display,string acd,int acdlen,string parityMask,string dataMask){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {

		blob acd_blob(*(m_sqlConn));
		acd_blob.write(0, acd.c_str(),acd.length());
		blob parityMask_blob(*(m_sqlConn));
		parityMask_blob.write(0, parityMask.c_str(),parityMask.length());
		blob dataMask_blob(*(m_sqlConn));
		dataMask_blob.write(0, dataMask.c_str(),dataMask.length());

		*(m_sqlConn) << "update acsTestData set display = :display, data = :a, length = :len, parityMask = :p, dataMask = :d",
					use(display), use(acd_blob), use(acdlen), use(parityMask_blob), use(dataMask_blob);
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::AddACSMaskColumn(){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		*(m_sqlConn)<< "ALTER table acsTestData ADD column parityMask blob";
		*(m_sqlConn)<< "ALTER table acsTestData ADD column dataMask blob";
		//*(m_sqlConn)<< "ALTER table acsTestData ADD column dataMask blob";
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

bool DBAdapter::CheckACSMaskColumn(){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;

	blob dataMask_blob(*(m_sqlConn));
	int count = 0;
	try {
		*(m_sqlConn)<< "SELECT COUNT(*), dataMask from acsTestData", into(count), into(dataMask_blob);
		if (count){
			return true;
		}
		else
			return false;
	} catch (soci::soci_error const &err) {
		return false;
	}
}

bool DBAdapter::CheckPINColumn() {

	return (CheckColumn("person", "pindata"));
}

int DBAdapter::AddPINColumn() {

	int pindataAddResult = AddColumn("person", "pindata", SQLITE_TEXT);

	return pindataAddResult;
}

bool DBAdapter::CheckColumn(string tableName, string columnName) {

	EyelockLog(logger, TRACE, "Checking if column %s exists in table %s", columnName.c_str(), tableName.c_str());

	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;

	int cid, notnull, pk;
	string name, type, dflt_value;
	indicator ind;
	bool result = false;
	try {
		statement st = (m_sqlConn->prepare << "PRAGMA table_info(" + tableName + ")", into(cid), into(name), into(type), into(notnull), into(dflt_value, ind), into(pk));
		st.execute();

		while (st.fetch()) {
			result = (name == columnName);
		}

		return result;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI error in CheckColumn: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "Exception in CheckColumn: %s\n",e.what());
	} catch (...) {
		EyelockLog(logger, ERROR, "Unknown exception in CheckColumn");
	}
	return false;
}

string ConvertTypeToStr(int type) {

	string ret = "UNKNOWN";
	switch (type) {
		case SQLITE_INTEGER:
			ret = "INTEGER";
			break;
		case SQLITE_FLOAT:
			ret = "REAL";
			break;
		case SQLITE_TEXT:
			ret = "TEXT";
			break;
		case SQLITE_BLOB:
			ret = "BLOB";
			break;
		default:
			break;
	}

	return ret;
}

int DBAdapter::AddColumn(string tableName, string columnName, int columnType){

	EyelockLog(logger, TRACE, "Adding column %s of type %d into table %s", columnName.c_str(), (int)columnType, tableName.c_str());

	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		*(m_sqlConn)<< "ALTER table " + tableName + " ADD column " + columnName + " " + ConvertTypeToStr(columnType);
		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI error in AddColumn: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "Exception in AddColumn: %s\n",e.what());
	} catch (...) {
		EyelockLog(logger, ERROR, "Unknown exception in AddColumn");
	}
	return -1;
}

int DBAdapter::GetACSData(string& display,string& acd,int& acdlen){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		blob acd_blob(*(m_sqlConn));
		int count = 0;

		*(m_sqlConn)<< "SELECT COUNT(*), display, data, length from acsTestData", into(count), into(display), into(acd_blob), into(acdlen);
		if (count){
			acd.resize(acd_blob.get_len());
			acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());
			return 0;
		}
		else
			return -1;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::GetACSData(string& display,string& acd,int& acdlen, string& parityMask, string& dataMask) {
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	try {
		blob acd_blob(*(m_sqlConn));
		blob parityMask_blob(*(m_sqlConn));
		blob dataMask_blob(*(m_sqlConn));
		int count = 0;

		*(m_sqlConn)<< "SELECT COUNT(*), display, data, length, parityMask, dataMask from acsTestData", into(count), into(display), into(acd_blob), into(acdlen), into(parityMask_blob), into(dataMask_blob);
		if (count){
			acd.resize(acd_blob.get_len());
			acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());
			parityMask.resize(parityMask_blob.get_len());
			parityMask_blob.read(0, (char*)parityMask.c_str(),parityMask_blob.get_len());
			dataMask.resize(dataMask_blob.get_len());
			dataMask_blob.read(0, (char*)dataMask.c_str(),dataMask_blob.get_len());
			return 0;
		}
		else
			return -1;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}

int DBAdapter::ReadDBForCheckingIris(DBMap *dbmap){
	if(!m_sqlConn) return -1;
	ScopeLock lock(m_lock);
	if(!dbmap) return -1;
	dbmap->Clear();
	unsigned char temp[IRIS_SIZE_INCLUDING_MASK_PER_PERSON];
	try {
		blob acd_blob(*(m_sqlConn));
		blob left_blob(*(m_sqlConn));
		blob right_blob(*(m_sqlConn));
		string perid,name,acd,pin;
		string left,right;
		int acdlen;
		indicator pinInd;
		//statement st = (m_sqlConn->prepare << "SELECT leftIris,rightIris,acd,acdlen,pindata,name,pers_id from person", into(left_blob),into(right_blob),into(acd_blob),into(acdlen),into(pin,pinInd),into(name),into(perid));
		statement st = (m_sqlConn->prepare << "SELECT leftIris,rightIris,acd,acdlen,name,pers_id from person", into(left_blob),into(right_blob),into(acd_blob),into(acdlen),into(name),into(perid));
		st.execute();
		while (st.fetch()){
			left.resize(left_blob.get_len());
			left_blob.read(0,(char*)left.c_str(),left_blob.get_len());
			right.resize(right_blob.get_len());
			right_blob.read(0,(char*)right.c_str(),right_blob.get_len());
			acd.resize(acd_blob.get_len());
			acd_blob.read(0, (char*)acd.c_str(),acd_blob.get_len());
			//dbmap->Insert(perid,left,right,name,acd,acdlen,pin);
			dbmap->Insert(perid,left,right,name,acd,acdlen);
		}
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter::getAllACD(char **data, int len, int type){
		if(!m_sqlConn) return -1;
		ScopeLock lock(m_lock);
		blob acd_blob(*(m_sqlConn));
		blob acdnop_blob(*(m_sqlConn));
		string name;
		try {
			statement st = (m_sqlConn->prepare << "SELECT acd, acdnop, name from person", into(acd_blob), into(acdnop_blob), into(name));
			st.execute();

			string acd, acdnop;
			int i = 0;
			while (st.fetch() && i < MAX_USERS) {
				acd.resize(acd_blob.get_len());
				acd_blob.read(0, (char*)acd.c_str(), acd_blob.get_len());
				const char *temp;
				char *ptr = data[i];
				if (type == 2) {
					acdnop.resize(acd_blob.get_len());
					acdnop_blob.read(0, (char*)acdnop.c_str(), acdnop_blob.get_len());
					temp = acdnop.c_str();
					memcpy(ptr, temp, len);
					temp = acd.c_str();
					memcpy(ptr+len, temp, len);
				}
				else {
					temp = acd.c_str();
					memcpy(ptr, temp, len);
					strcpy(ptr+len, name.c_str());
				}
				i++;
			}
			return 0;
		} catch (soci::soci_error const &err) {
			data = NULL;
			EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
		} catch (exception const &e) {
			data = NULL;
			EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
		}
		return -1;
}

int DBAdapter::getAllACD(char **data, int acdByteLength, int pinByteLength, int nameMaxLength, int type){

	EyelockLog(logger, TRACE, "Retrieving card data and PINs from the database and storing in memory");
	EyelockLog(logger, TRACE, "Card data byte length: %d, PIN length: %d, name max length: %d. Function call type: %d", acdByteLength, pinByteLength, nameMaxLength, type);

	if(!m_sqlConn) return -1;
	if(data == NULL) {
		EyelockLog(logger, ERROR, "Memory for saving card data and PINs is not allocated");
		return -1;
	}
	ScopeLock lock(m_lock);
	blob acd_blob(*(m_sqlConn));
	blob acdnop_blob(*(m_sqlConn));
	string name,pin;
	indicator acdInd, acdnopInd, pinInd;
	try {
		string acd, acdnop;
		int i = 0;

		if (pinByteLength) {
			statement st = (m_sqlConn->prepare << "SELECT acd, acdnop, pindata, name from person", into(acd_blob, acdInd), into(acdnop_blob, acdnopInd), into(pin, pinInd), into(name));
			st.execute();
			while (st.fetch() && i < MAX_USERS) {
				acd.resize(acd_blob.get_len());
				acd_blob.read(0, (char*)acd.c_str(), acd_blob.get_len());
				char *ptr = data[i];
				memcpy(ptr, acd.c_str(), MIN((size_t)acdByteLength, acd.length()));
				strncpy(ptr+acdByteLength, pin.c_str(), (size_t)pinByteLength);
				strncpy(ptr+acdByteLength+pinByteLength, name.c_str(), nameMaxLength);
				i++;
			}
		}
		else {
			statement st = (m_sqlConn->prepare << "SELECT acd, acdnop, name from person", into(acd_blob, acdInd), into(acdnop_blob, acdnopInd), into(name));
			st.execute();

			while (st.fetch() && i < MAX_USERS) {
				acd.resize(acd_blob.get_len());
				acd_blob.read(0, (char*)acd.c_str(), acd_blob.get_len());
				char *ptr = data[i];
				if (type == 2) {
					acdnop.resize(acd_blob.get_len());
					acdnop_blob.read(0, (char*)acdnop.c_str(), acdnop_blob.get_len());
					const char *temp = acdnop.c_str();
					memcpy(ptr, temp, acdByteLength);
					temp = acd.c_str();
					memcpy(ptr+acdByteLength, temp, acdByteLength);
				}
				else {
					memcpy(ptr, acd.c_str(), MIN((size_t)acdByteLength, acd.length()));
					if (pinByteLength)
						strncpy(ptr+acdByteLength, pin.c_str(), (size_t)pinByteLength);
					strncpy(ptr+acdByteLength+pinByteLength, name.c_str(), nameMaxLength);
				}
				i++;
			}
		}
		return 0;
	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI error in getAllACD: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, ERROR, "Exception in getAllACD: %s\n",e.what());
	} catch (...) {
		EyelockLog(logger, ERROR, "Unknown exception in getAllACD");
	}
	return -1;
}

int DBAdapter::getACDCount(string card, int type){
        printf("getACDCount(): card %s, type %d, len %d\n", card.c_str(), type, card.length());
                if(!m_sqlConn) return -1;
                ScopeLock lock(m_lock);
                int count = 0;
                blob acd_blob(*(m_sqlConn));
                acd_blob.write(0, card.c_str(),4);
                try {
                        if (type == 1)
                                *(m_sqlConn) << "SELECT COUNT(*) FROM person where acd = :c", into(count), use(acd_blob);
                        else if (type == 2)
                                *(m_sqlConn) << "SELECT COUNT(*) FROM person where acdnop = :c", into(count), use(acd_blob);
                } catch (soci::soci_error const &err) {
                        EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
                } catch (exception const &e) {
                        EyelockLog(logger, ERROR, "SOCI EXC :: %s\n",e.what());
                }
                printf("count %d\n", count);
                return count;
}

int DBAdapter::GetUserDataFromACD(string& perid,string& username,string& leftiris,string& rightiris,string acd,int acdlen, int type){
	ScopeLock lock(m_lock);
	if(!m_sqlConn) return -1;
	const char *temp = acd.c_str();
	//printf("%x %x %x %x\n", temp[0], temp[1], temp[2], temp[3]);
	int ret = 0;
	try {
		blob acd_blob(*(m_sqlConn));
		acd_blob.write(0, acd.c_str(),(acdlen+7)/8);
		blob left_blob(*(m_sqlConn));
		blob right_blob(*(m_sqlConn));

		if (type == 1)
			*(m_sqlConn)<< "SELECT pers_id,name,leftIris,rightIris From person WHERE acd = :a", into(perid),
				into(username), into(left_blob), into(right_blob), use(acd_blob);
		else if (type == 2)
			*(m_sqlConn)<< "SELECT pers_id,name,leftIris,rightIris From person WHERE acdnop = :c", into(perid),
							into(username), into(left_blob), into(right_blob), use(acd_blob);

		leftiris.resize(left_blob.get_len());
		left_blob.read(0, (char*)leftiris.c_str(),left_blob.get_len());

		rightiris.resize(right_blob.get_len());
		right_blob.read(0, (char*)rightiris.c_str(),right_blob.get_len());

		return 0;

	} catch (soci::soci_error const &err) {
		EyelockLog(logger, ERROR, "SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		EyelockLog(logger, DEBUG, "SOCI EXC :: %s",e.what());
	}
	return -1;
}
