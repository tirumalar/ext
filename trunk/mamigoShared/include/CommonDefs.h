/*
 * CommonDefs.h
 *
 *  Created on: 17-Sep-2009
 *      Author: mamigo
 */

#ifndef COMMONDEFS_H_
#define COMMONDEFS_H_

#include "MessageExt.h"
#include <list>
#include <assert.h>
#include <algorithm>
#include <string.h>

//#define LED_FLASH
//#define OTO_MATCH
//#define CMX_C1

enum QueueFullBehaviour{DROP=0,OVERWRIE_OLD=1,OVERWRIE_NEW=1};
enum MatchResultState{INITIAL=-1,NOMATCH=-2,DETECT=0,PASSED=1,FAILED=2,MOTION=3,LOITERING=5,CONFUSION=6,DBRELOAD=7,HEALTH=8,DUAL_AUTHN_CARD=9,TAMPER=12};
enum LedState{LED_INITIAL=-1,LED_DETECT=0,LED_PASSED=1,LED_FAILED=2,LED_NWSET=4,LED_LOITERING=5,LED_CONFUSION=6,LED_DBRELOAD=7,LED_DUAL_AUTHN_CARD=9,LED_CONFUSION_FLASH=11,LED_TAMPER=12,LED_PASSED_FLASH=13};
enum SoftwareType{ePC=0,eNANO,ePICO,eNTSGLM,eNTSGLS,eNTSGFM,eNTSGFS};
enum LEDMessageGeneration{eLOCALGEN=0,eREMOTEGEN=1};
enum DBMsgType{eREPLACEDB=0,eUPDATEDB,eDELETEDB,eSQLITEDB,eRELOADDB};

#define F2FKEY_MAX_SIZE 256
#define MAX_USERS		20000
using std::min;
using namespace std;

class LEDResult : public Copyable{
public:
	LEDResult(){
		reset();
	}
	virtual ~LEDResult(){
	}
	void init(bool bPassed=true){
		m_state=(bPassed)?LED_PASSED:LED_FAILED;
	}
	void setTimeStamp(uint64_t t){m_timeStamp =t;}
	uint64_t getTimeStamp(){return m_timeStamp;}
	void reset(){
		m_state=LED_INITIAL;
		m_NwValue = 0;
		m_NwSleepmsec = 0;
		m_generatedlocal=eLOCALGEN;
	}
	void setState(LedState state){ m_state=state;}
	void setGeneratedState(LEDMessageGeneration val){ m_generatedlocal = val;}
	LEDMessageGeneration getGeneratedState(){ return m_generatedlocal;}
	virtual void CopyFrom(const Copyable& other) {CopyFrom((const LEDResult&) other);}
	void CopyFrom(const LEDResult& other){
		m_state=other.m_state;
		m_NwValue = other.m_NwValue;
		m_NwSleepmsec = other.m_NwSleepmsec;
		m_timeStamp = other.m_timeStamp;
		m_generatedlocal = other.m_generatedlocal;
	}
	LEDResult(const LEDResult& data){
		reset();
		CopyFrom(data);
	}
	LEDResult& operator = (const LEDResult& data) // a=b;
	{
		CopyFrom(data);
		return *this;
	}
	LedState getState(){return m_state;}
	void setNwValandSleep(int val, int sleeptime){
		m_NwValue=val;
		m_NwSleepmsec = sleeptime;
	}
	void getNwValandSleep(int& val, int& sleeptime){
		val = m_NwValue;
		sleeptime = m_NwSleepmsec;
	}
protected:
#ifdef UNITTEST
public:
#endif
	LEDMessageGeneration m_generatedlocal;
	int m_NwValue;
	int m_NwSleepmsec;
	LedState m_state;
	uint64_t m_timeStamp;
};

class MatchResult : public Copyable{
public:
	MatchResult():m_keyOwn(0){
		m_keyOwn=m_key=(char *)malloc(F2FKEY_MAX_SIZE);
		memset(m_key,0,F2FKEY_MAX_SIZE);
		reset();
	}
	MatchResult(char *keyPtr):m_key(keyPtr),m_keyOwn(0){
		reset();
	}
	virtual ~MatchResult(){
		if(m_keyOwn)
			free(m_keyOwn);
	}
	void init(int index=-1, float score=1.0f,int keylength=2,int idbytelen=0, bool bPassed=true){
		m_name.clear();
		m_guid.clear();
		m_index=index;
		m_Score=score;
		m_keylength = keylength;
		assert(m_keylength<=F2FKEY_MAX_SIZE);
		m_IDlength = idbytelen;
		m_state=(bPassed)?PASSED:FAILED;
		m_PersonIrisInfo = 0;
	}
	void setKey(char* key,int keyLen){
		m_keylength=keyLen;
		memcpy(m_key,key,m_keylength);
	}
	void setF2F(char* key){
		setKey(key,2+getF2FByteLen(key));
	}
	void printF2FData(){
		printf("F2FData -> ");
		for(int i=0;i< m_keylength;i++){
			printf("%02x ",(unsigned char )m_key[i]);
		}
		printf("\n");
	}
	void setVar(float val ){ m_VarienceScore = val;}
	void setTimeStamp(uint64_t t){m_timeStamp =t;}
	uint64_t getTimeStamp(){return m_timeStamp;}

	float getVar(){ return m_VarienceScore;}
	void reset(){
		m_name.clear();
		m_guid.clear();
		m_index=-1;
		m_Score=1.0;
		m_state=DETECT;
		m_keylength = 2;	// min two bytes
		m_IDlength = 0;
		m_NwValue = 0;
		m_NwSleepmsec = 0;
		m_VarienceScore=0;
		m_frameIndex = -1;
		m_cameraIndex=-1;
		m_eyeIdx = -1;
		m_cam.assign("NONE");
		m_PersonIrisInfo = 0;
		}
	//bool isSet(){ return m_index>=0;}

	void setState(MatchResultState state){ m_state=state;}

	virtual void CopyFrom(const Copyable& other) {CopyFrom((const MatchResult&) other);}
	void CopyFrom(const MatchResult& other){
		m_index=other.m_index;
		m_Score=other.m_Score;
		m_state=other.m_state;

		m_keylength=other.m_keylength;
		m_IDlength=other.m_IDlength;

		m_NwValue = other.m_NwValue;
		m_NwSleepmsec = other.m_NwSleepmsec;
		m_VarienceScore = other.m_VarienceScore;
		memset(m_key,0,F2FKEY_MAX_SIZE);
		memcpy(m_key,other.m_key,m_keylength);
		m_timeStamp = other.m_timeStamp;
		m_name.assign(other.m_name);
		m_guid.assign(other.m_guid);
		m_frameIndex = other.m_frameIndex;
		m_cameraIndex=other.m_cameraIndex;
		m_eyeIdx = other.m_eyeIdx;
		m_PersonIrisInfo = other.m_PersonIrisInfo;
		m_cam.assign(other.m_cam);
	}
	MatchResult(const MatchResult& data)
	{
		m_keyOwn=m_key=(char *)malloc(F2FKEY_MAX_SIZE);
		reset();
		CopyFrom(data);
	}
	MatchResult& operator = (const MatchResult& data) // a=b;
	{
		memcpy(m_keyOwn,data.m_keyOwn,m_keylength);
		CopyFrom(data);
		return *this;
	}
	void CopyShallowFrom(const MatchResult& other){
		m_index=other.m_index;
		m_Score=other.m_Score;
		m_state=other.m_state;
		m_PersonIrisInfo = other.m_PersonIrisInfo;
		m_keylength=other.m_keylength;
		m_IDlength=other.m_IDlength;
		m_key=	other.m_key;
		if(m_keyOwn)
			free(m_keyOwn);
		m_keyOwn=0;
		m_timeStamp = other.m_timeStamp;
		m_name.assign(other.m_name);
		m_guid.assign(other.m_guid);
		m_frameIndex = other.m_frameIndex;
		m_cameraIndex=other.m_cameraIndex;
		m_eyeIdx = other.m_eyeIdx;
		m_cam.assign(other.m_cam);
	}
	char *getKey(){
		return m_key;
	}
	void setName(string name){
		m_name.assign(name);
	}
	void setGUID(string guid){
		m_guid.assign(guid);
	}
	string getName(){
		return m_name;
	}
	string getGUID(){
		return m_guid;
	}
	char *getUID(int& len)
	{	int f2flen=getF2FByteLen();
		len=m_keylength-(2+f2flen);
		len=min(len,m_IDlength);
		return m_key+2+f2flen;
	}
	MatchResultState getState()
	{
		return m_state;
	}
	int getEyeIndex(){ return m_index;}
	float getScore() { return m_Score;}
	char *getF2F(int& len, int& bits)
	{
		len=getF2FByteLen();
		bits=getF2FBitLen();
		return m_key+2;
	}
	void setNwValandSleep(int val, int sleeptime){
		m_NwValue=val;
		m_NwSleepmsec = sleeptime;
	}

	void getNwValandSleep(int& val, int& sleeptime){
		val = m_NwValue;
		sleeptime = m_NwSleepmsec;
	}
	void setFrameInfo(int fno,int eno,char* cam, int EXTCameraIdx){
		m_frameIndex = fno;
		m_eyeIdx = eno;
		m_cameraIndex=EXTCameraIdx;
		m_cam.assign(cam);
	}
	void getFrameInfo(int& fno,int& eno,string& cam, int& EXTCameraIdx){
		fno =m_frameIndex;
		eno = m_eyeIdx ;
		EXTCameraIdx=m_cameraIndex;
		cam.assign(m_cam);
	}
	void SetPersonIrisInfo(int val){
		m_PersonIrisInfo = val;
	}
	int GetPersonIrisInfo(){
		return m_PersonIrisInfo;
	}

protected:
	static int getF2FBitLen(char *key){
		return (int)((((unsigned int) key[1])<<8) + (unsigned char) key[0]);
	}
	int getF2FBitLen(){
		return getF2FBitLen(m_key);
	}
	static int getF2FByteLen(char *key){
			return (getF2FBitLen(key)+7)>>3;
		}
	int getF2FByteLen(){
		return getF2FByteLen(m_key);
	}
#ifdef UNITTEST
public:
#endif
	int m_IDlength;
	int m_index;
	float m_Score;
	char *m_keyOwn;
	char *m_key;
	int m_keylength;
	int m_NwValue;
	int m_NwSleepmsec;
	float m_VarienceScore;
	int m_PersonIrisInfo;
	string m_guid;
	string m_name;
	MatchResultState m_state;
	uint64_t m_timeStamp;
	int m_frameIndex,m_eyeIdx;
	int m_cameraIndex;
	string m_cam;
};

#endif /* COMMONDEFS_H_ */
