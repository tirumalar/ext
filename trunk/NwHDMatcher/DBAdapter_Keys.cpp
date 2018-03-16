#include "DBAdapter_Keys.h"
#include <stdio.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <unistd.h>

using namespace soci;
using namespace std;

DBAdapter_Keys::DBAdapter_Keys():m_sqlConn(NULL){
}

DBAdapter_Keys::~DBAdapter_Keys(){
	CloseConnection();
}
void DBAdapter_Keys::CloseConnection(){
	if(m_sqlConn){
		m_sqlConn->close();
		delete m_sqlConn;
		m_sqlConn=NULL;
	}
}

string DBAdapter_Keys::GetDBFileName(){
	return m_filename;
}
bool DBAdapter_Keys::ReOpenConnection(){
	return OpenFile((char*)m_filename.c_str());
}
bool DBAdapter_Keys::OpenFile(char* filename){
	CloseConnection();
	try {
		m_sqlConn = new session(sqlite3, filename);
		m_filename.assign(filename);
		return true;
	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s",e.what());
	}
	return false;
}

int DBAdapter_Keys::GetCertCount(){
	if(!m_sqlConn) return -1;
	int count = -1;
	try {
		*(m_sqlConn) << "SELECT COUNT(*) FROM Keys", into(count);
	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s",e.what());
	}
	return count;
}

int DBAdapter_Keys::UpdateData(string host,int indx ,int64_t validity,bool isDevice,string& cert,string& key){
	if(!m_sqlConn) return -1;
	try {
		int count = 0;
		blob cert_blob(*(m_sqlConn));
		cert_blob.write(0, cert.c_str(),cert.size());

		blob key_blob(*(m_sqlConn));
		//key_blob.write(0, (char*)key,keylen);
		key_blob.write(0, (char*)key.c_str(),key.length());

//		FILE *fp  =  fopen("key_ud.bin","wb");
//		fwrite(key ,sizeof(char), keylen, fp );
//		fclose(fp);

		*(m_sqlConn) << "SELECT COUNT(*) FROM Keys where id = :p", into(count), use(indx);
		if (count) {
			printf("Host_Already_Present\n");
			*(m_sqlConn) << "update Keys set host = :host, validity = :validity, isDevice = :isDevice, cert = :cert, key = :key  where id = :p",
					use(host), use(validity), use(isDevice?1:0), use(cert_blob), use(key_blob), use(indx);
			return 1;
		}

		transaction t(*(m_sqlConn));
		*(m_sqlConn)<< "INSERT INTO Keys (host,validity,isDevice,cert,key) VALUES (:host, :validity,:isDevice, :cert, :key)", use(host),use(validity),use(isDevice?1:0),use(cert_blob), use(key_blob);

		t.commit();
		sync();
		return 0;

	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s\n",e.what());
	}
	return -1;
}

int DBAdapter_Keys::GetData(string host,int& indx ,int64_t& validity,bool& isDevice,string& cert,string& key){
	if(!m_sqlConn) return -1;
	try {
		blob cert_blob(*(m_sqlConn));
		blob key_blob(*(m_sqlConn));
		int device=0;
		*(m_sqlConn)<< "SELECT host,validity,isDevice,cert,key From Keys WHERE host = :host", into(
				host), into(validity),into(device), into(cert_blob), into(key_blob),use(host);
		isDevice = device!=0?true:false;
		cert.resize(cert_blob.get_len());
		cert_blob.read(0, (char*)cert.c_str(),cert_blob.get_len());

		key.resize(key_blob.get_len());
		key_blob.read(0, (char*)key.c_str(), key_blob.get_len());

		return 0;

	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s",e.what());
	}
	return -1;
}


int DBAdapter_Keys::DeleteData(int idx){
	if(!m_sqlConn) return -1;
	try {
		transaction t(*(m_sqlConn));
		int count = 0;
		*(m_sqlConn) << "SELECT COUNT(*) FROM Keys where id = :p", into(count), use(idx);
		if (count) {
			*(m_sqlConn) << "DELETE from Keys WHERE id = :p", use(idx);
			t.commit();
			sync();
			return 0;
		}else{
			return -1;
		}
	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s\n",e.what());
	}
	return -2;
}

int DBAdapter_Keys::ReadDB(vector<pair<string,int64_t> > & vec){
	if(!m_sqlConn) return -1;
	try {
		blob cert_blob(*(m_sqlConn));
		blob key_blob(*(m_sqlConn));
		int device=0;
		int64_t validity;
		string host;
		statement st = (m_sqlConn->prepare << "SELECT host,validity,isDevice From Keys", into(host), into(validity),into(device));
		st.execute();
		while (st.fetch()){
			if(!device){
				pair<string,int64_t> val;
				val.first= host;
				val.second = validity;
				vec.insert(vec.begin(),val);
			}
		}
		return 0;
	} catch (soci::soci_error const &err) {
		printf("SOCI ERRR :: %s\n",err.what());
	} catch (exception const &e) {
		printf("SOCI EXC :: %s",e.what());
	}
	return -1;
}






