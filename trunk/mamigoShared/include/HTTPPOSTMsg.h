#pragma once
#include "MessageExt.h"
#include "base64.h"
#include <string.h>
#include <stdlib.h>

#if (defined(__BFIN__)||defined(__linux__))
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

enum NWMESSAGETYPE {
	UNKNOWN_MSG = 0,
	IMG_MSG,
	HB_MSG,
	LED_MSG,
	RELOADDB_MSG,
	RECVDB_MSG,
	DOENROLL_MSG,
	FACEGRAB_MSG,
	MOTION_MSG,
	F2F_MSG,
	SETPIN_MSG,
	MATCH_MSG = 11,
	PING_MSG,
	RESET_EYELOCK_MSG,
	ENABLETHREADPROPERTIES,
	DOWNLOADDB,
	EYELOCK_MODE,
	HALO_DC,
	UPDATEUSR,
	DELETEUSR,
	RECEIVEUSR,
	RECEIVESQLITE = 21,
	GET_VERSION_MSG,
	GETUSERCOUNT_MSG,
	TESTACS,
	GETACS,
	CALIBRATION_MSG,
	TESTMATCH,
	TIMESYNC_MSG,
	LOCATEDEVICE_MSG
};

class HTTPPOSTMsg :
	public BinMessage
{
public:
	HTTPPOSTMsg(unsigned int Bytes):BinMessage(Bytes){}
	HTTPPOSTMsg(const char *Msg, unsigned int Len):BinMessage(Msg,Len){}
#if 1
	virtual void CopyFrom(const Copyable& _other)	{

		const HTTPPOSTMsg& other=(const HTTPPOSTMsg&)_other;
		Timestamp=other.Timestamp;
		char *image=other.getImage();
		long long int alignement= ((long long int)image) & 0x3;
		if(alignement == 0){
			SetData(other.Buffer,other.Size);
			return;
		}
		// we are here means the image is not 4 bytes aligned
		// first copy a few bytes from begining
		int size=(image-other.Buffer-28);
		SetData(other.Buffer,size);
//commit comment
		char *filler="    ";
		Append(filler,4-alignement);

		size=other.Size-size;
		Append(image-28,size);
	}
#endif

	virtual bool  IsDone(){
		if(isLED()) return true;
		if(isHeartBeat()) return true;
		if(isReloadDB()) return true;

		if(isDoEnroll()) return true;

		if(isFaceGrab()) return true;
		if(isMotion()) return true;

		if(isRecvDB()) return true;

		if(isF2F()) return true;
		if(isSetPin()) return true;
		if(isPing()) return true;
		if(isResetEyelock()) return true;
		if(isThreadProp()) return true;
		if(isDownloadDB()) return true;
		if(isUpdateUsr()) return true;
		if(isEyelockMode()) return true;
		if(isHaloDC()) return true;
		if(isSqlite()) return true;
		if(isVersion()) return true;
		if(isTestACS()) return true;
		if(isGetACS()) return true;
		if(isCalibration())return true;
		// if starts with POST it is IMAGE message
		char *temp=Buffer;
		if(0==strnicmp("MATCH;",temp,6))
			return IsDoneMatch();
		else if(0==strnicmp("POST",temp,4))
			return IsDoneHTTP();
		//else{
			//printf("Ignoring Unknown message: %*.s\n",20,Buffer);
		//}

		return true;
	}

	char *getImage() const{
		// if starts with POST it is HTTP message
		int length=0;
		char *temp=Buffer;
		if(0==strnicmp("POST",temp,4))
			return getFrameHTTP(&length);
		else
			return 0;
	}

	//WARNING does not support XML messages
	NWMESSAGETYPE getMsgType(){
//		printf("Buffer %s\n", Buffer);
		if(isHeartBeat()) return HB_MSG;
		if(isLED()) return LED_MSG;
		if(isReloadDB()) return RELOADDB_MSG;
		if(isRecvDB() || isRecvEncDB()) return RECVDB_MSG;
		if(isDoEnroll()) return DOENROLL_MSG;
		if(isFaceGrab()) return FACEGRAB_MSG;
		if(isMotion()) return MOTION_MSG;
		if(isF2F()) return F2F_MSG;
		if(isSetPin()) return SETPIN_MSG;
		if(isMatch()) return MATCH_MSG;
		if(isPing()) return PING_MSG;
		if(isResetEyelock()) return RESET_EYELOCK_MSG;
		if(isThreadProp()) return ENABLETHREADPROPERTIES;
		if(isDownloadDB()) return DOWNLOADDB;
		if(isEyelockMode()) return EYELOCK_MODE;
		if(isHaloDC()) return HALO_DC;
		if(isUpdateUsr() || isEncUpdateUsr()) return UPDATEUSR;
		if(isReceiveUsr()) return RECEIVEUSR;
		if(isDeleteUsr()) return DELETEUSR;
		if(isSqlite()) return RECEIVESQLITE;
		if(isVersion()) return GET_VERSION_MSG;
		if(isGetCount()) return GETUSERCOUNT_MSG;
		if(isTestACS()) return TESTACS;
		if(isGetACS()) return GETACS;
		if(isTestMATCH()) return TESTMATCH;
		if(isCalibration()) return CALIBRATION_MSG;
		if(isHwTimeSync()) return TIMESYNC_MSG;
		if(isLocateDevice()) return LOCATEDEVICE_MSG;
		int fr_Len=0;
		char *frame=getFrameHTTP(&fr_Len);
		if(fr_Len>0 && frame!=0) return IMG_MSG;

		return UNKNOWN_MSG;
	}

bool isHeartBeat(){
	bool isHB = (0==strncmp(Buffer,hb,hblen));
	return isHB;
}
bool isTestACS(){
	return (0 == strncmp(Buffer, "TESTACS", 7));
}
bool isGetACS(){
	return (0 == strncmp(Buffer, "GETACS", 6));
}
bool isTestMATCH(){
	return (0 == strncmp(Buffer, "TESTMATCH", 9));
}
bool isCalibration(){
	return (0 == strncmp(Buffer, "CALIBRATION", 11));
}
bool isLED(){
	bool isLED = (0==strncmp(Buffer,ledMsg,ledMsglen));
	return isLED;
}

void makeReloadMsg(){
	SetData(reloadMsg,reloadMsglen);
}

bool isReloadDB(){
	bool isReloadDB = (0==strncmp(Buffer,reloadMsg,reloadMsglen));
	return isReloadDB;
}

bool isRecvDB(){
	bool isRecvDB = (0==strncmp(Buffer,recvdbMsg,recvdbMsglen));
	return isRecvDB;
}

bool isDoEnroll(){
	bool isDoEnroll = (0==strncmp(Buffer,doenrollMsg,doenrollMsglen));
	return isDoEnroll;
}

bool isFaceGrab(){
	bool isFaceGrab = (0==strncmp(Buffer,faceGrabMsg,faceGrabMsglen));
	return isFaceGrab;
}

bool isMotion(){
	bool isMotion = (0==strncmp(Buffer,motionMsg,motionMsglen));
	return isMotion;
}

bool isF2F(){
	bool isF2F = (0==strncmp(Buffer,f2fMsg,f2fMsglen));
	return isF2F;
}

bool isSetPin(){
	bool isSetPin = (0==strncmp(Buffer,setpinMsg,setpinMsglen));
	return isSetPin;
}

bool isMatch(){
	bool isMatch = (0==strncmp(Buffer,matchMsg,matchMsglen));
	return isMatch;
}

bool isPing(){
	bool isping = (0==strncmp(Buffer,pingMsg,pingMsglen));
	return isping;
}

bool isResetEyelock(){
	bool isreset = (0==strncmp(Buffer,resetMsg,resetMsglen));
	return isreset;
}

bool isThreadProp(){
	bool isthreadprop = (0==strncmp(Buffer,threadPropMsg,threadPropMsglen));
	return isthreadprop;
}

bool isDownloadDB(){
	bool isdownloadDBReq = (0==strncmp(Buffer,downloadDBMsg,downloadDBMsglen));
	return isdownloadDBReq;
}

bool isEyelockMode(){
	bool ismode = (0==strncmp(Buffer,"EYELOCK_MODE",12));
	return ismode;
}

bool isUpdateUsr(){
	bool ismode = (0==strncmp(Buffer,"UPDATEUSR",9));
	return ismode;
}
bool isReceiveUsr(){
	bool ismode = (0==strncmp(Buffer,"RECEIVEUSR",10));
	return ismode;
}
bool isDeleteUsr(){
	bool ismode = (0==strncmp(Buffer,"DELETEUSR",9));
	return ismode;
}

bool isHaloDC(){
	bool ishalo	 = (0==strncmp(Buffer,"Halo-DC",7));
	return ishalo;
}

bool isSqlite(){
	bool isReceiveSqlite = (0==strncmp(Buffer,sqliteMsg,sqliteMsglen));
	return isReceiveSqlite;
}

bool isVersion(){
	bool ismode = (0==strncmp(Buffer,"GET_VERSION_MSG",15));
	return ismode;
}
bool isEncUpdateUsr(){
	bool ismode = (0==strncmp(Buffer,"UPDATEUSRENCRYPTED",18));
	return ismode;
}
bool isRecvEncDB(){
	bool ismode = (0==strncmp(Buffer,"RECEIVEENCRYPTEDDB",18));
	return ismode;
}
bool isGetCount(){
	bool ismode = (0==strncmp(Buffer,"GET_USER_COUNT",14));
	return ismode;
}
bool isHwTimeSync(){
	bool ismode = (0==strncmp(Buffer,"TIMESYNC_MSG",12));
	return ismode;
}
bool isLocateDevice(){
	bool ismode = (0==strncmp(Buffer,"LOCATEDEVICE_MSG",16));
	return ismode;
}


bool getThreadMessageType(){
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	int test = atoi(temp);
	printf("ThreadMessageType %d\n",test);
	if(test)
		return true;
	return false;
}

bool getCalibrationData(int& nwset, int& flashtime,int& triggertime,int& led){
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	nwset = atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	flashtime = atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	triggertime = atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	led = atoi(temp);
	return true;
}

bool getHaloAndDc(int& dc, int& shift,int& maxspecval,float& halothres){
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	halothres = atof(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	dc = atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	shift = atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){
		return false;
	}
	temp+=1;
	maxspecval = atoi(temp);
	return true;
}

int getLEDMessageType(){
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(temp==0){
		// legacy Message Type
		return 1;
	}
	temp+=1;
	return atoi(temp);
}

int getLEDMessageData(int& value,int& sleeptime){
	char *temp=Buffer;
	value =0;
	sleeptime = 0;
	temp=strstr(temp,";");
	if(temp==0){return 1;}
	temp+=1;
	temp=strstr(temp,";");
	if(temp==0){ return 1;}
	temp+=1;
	value=atoi(temp);
	temp=strstr(temp,";");
	if(temp==0){ return 1;}
	temp+=1;
	sleeptime=atoi(temp)*1000;//microsec
	return 0;
}


int getClientID(){
		char *temp=Buffer;
		temp=strstr(temp,"id=\"");
		if(0==temp) return -1;
		temp+=4;
		return atoi(temp);
	}

bool getCameraNo(int& camID){
		char *stPos=Buffer;
		stPos=strstr(stPos,"cameraId=\"");
		if(0==stPos) return false;
		stPos+=10;
		camID = atoi(stPos);
		return true;
	}


bool getCameraID(char *CamID){
		char *stPos=Buffer;
		stPos=strstr(stPos,"cameraId=\"");
		if(0==stPos) return false;
		stPos+=10;

		char *endPos=strstr(stPos,"\"");

		if(0==endPos) return false;

		strncpy(CamID,stPos,endPos-stPos);

		return true;
	}
bool getImageDims(int& width, int& height){
	char *temp=Buffer;
	temp=strstr(temp,"width=");
	if(0==temp) return false;
	temp+=7;
	int w=atoi(temp);

	temp=strstr(temp,"height=");
	if(0==temp) return false;
	temp+=8;
	height=atoi(temp);
	width=w;
	return true;
}
bool getScore(int& score){
	char *temp=Buffer;
	temp=strstr(temp,"score=");
	if(0==temp) return false;
	temp+=7;
	int s=atoi(temp);
	score = s;
	return true;
}
bool getfScore(float& score){
	char *temp=Buffer;
	temp=strstr(temp,"fscore=");
	if(0==temp) return false;
	temp+=8;
	double s=atof(temp);
	score = s;
	return true;
}

bool getaScore(float& score){
	char *temp=Buffer;
	temp=strstr(temp,"ascore=");
	if(0==temp) return false;
	temp+=8;
	double s=atof(temp);
	score = s;
	return true;
}

bool getBLC(float& blc){
	char *temp=Buffer;
	temp=strstr(temp,"blc=");
	if(0==temp) return false;
	temp+=5;
	double s=atof(temp);
	blc = s;
	return true;
}

bool getHalo(float& halo){
	char *temp=Buffer;
	temp=strstr(temp,"halo=");
	if(0==temp) return false;
	temp+=6;
	double s=atof(temp);
	halo = s;
	return true;
}

bool getTimeStamp(uint64_t& ts){

	char *temp=Buffer;
	temp=strstr(temp,"timeStamp=");
	if(0==temp) return false;
	temp+=11;
	uint64_t s=atoll(temp);
	ts = s;
	return true;

}

int getEXTCameraIndex(){
		char *temp=Buffer;
		temp=strstr(temp,"CameraId=\"");
		if(0==temp) return -1;
		temp+=9;
		return atoi(temp);
	}

int getFrameIndex(){
		char *temp=Buffer;
		temp=strstr(temp,"frameId=\"");
		if(0==temp) return -1;
		temp+=9;
		return atoi(temp);
	}

int getEyeIndex(){
		char *temp=Buffer;
		temp=strstr(temp,"imageId=\"");
		if(0==temp) return -1;
		temp+=9;
		return atoi(temp);
	}

int getIlluminator(){
		char *temp=Buffer;
		temp=strstr(temp,"Il0=\"");
		if(0==temp) return -1;
		temp+=5;
		return atoi(temp);
	}

bool getPrevIndex(int& prev){
		char *temp=Buffer;
		temp=strstr(temp,"prev=\"");
		if(0==temp) return false;
		temp+=6;
		prev = atoi(temp);
		return true;
	}

bool getIrx(float& irx){
	char *temp=Buffer;
	temp=strstr(temp,"irx=\"");
	if(0==temp) return false;
	temp+=5;
	double s=atof(temp);
	irx = s;
	return true;
}
bool getIry(float& iry){
	char *temp=Buffer;
	temp=strstr(temp,"iry=\"");
	if(0==temp) return false;
	temp+=5;
	double s=atof(temp);
	iry = s;
	return true;
}

bool getXY(int& x, int& y){
	char *temp=Buffer;
	temp=strstr(temp,"x=");
	if(0==temp) return false;
	temp+=3;
	x=atoi(temp);

	temp=strstr(temp,"y=");
	if(0==temp) return false;
	temp+=3;
	y=atoi(temp);
	return true;
}
bool getSDKDBMsg(int& msgtype,std::string& fname){
//"RECEIVEUSR;%d;fname;"
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	msgtype=atoi(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	char* temp1=strstr(temp,";");
	if(0==temp1) return false;
	fname.assign(temp,temp1-temp);
	return true;
}

bool getReloadDBParsedMsg(int& msgtype,int& filenumber, int& sd,unsigned long& securetrait, int& isEncrypt){
//"RELOADDB;%d;%d;%d;%d;",eREPLACEDB,m_rxfilenumber-1,msg->m_SD,(void*)msg->m_SecureTrait;msg->isEncrypt;);
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	msgtype=atoi(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	filenumber=atoi(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	sd=atoi(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	securetrait=atol(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	isEncrypt=atoi(temp);
	return true;
}

char *getDBPrelude(int& preludeSize, int& dbFileSize)
{
	char *db_prelude = getDB(dbFileSize);
	preludeSize=GetSize()-(db_prelude-GetBuffer());
	return db_prelude;
}
char* getIpaddr(){
 	char *temp=Buffer;
 	temp=strstr(temp,";");
 	if(0==temp) return 0;
 	temp+=1;
 	return temp;
 }

char *getServerIPAndDataAndPreludeSize(char* ipaddr,int& preludeSize, int& datasize){
	char *temp = getDBServerIpaddr(ipaddr);
	if(0==temp) return 0;
 	int cLen=atoi(temp);
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;
   	datasize = cLen;
	preludeSize = GetSize()-(temp-Buffer);
	return temp;
}

char *getSqliteServerIPAndDataAndPreludeSize(char* ipaddr,int& preludeSize, int& datasize){
	char *temp = getSqliteDBServerIpaddr(ipaddr);
	if(0==temp) return 0;
 	int cLen=atoi(temp);
    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;
   	datasize = cLen;
	preludeSize = GetSize()-(temp-Buffer);
	return temp;
}
char* getDBServerIpaddr(char *buff)
{
	char *temp = getIpaddr();
	char *t1=NULL;
	t1 = strstr(temp,";");
	if(t1){
		//temp[t1-temp] = '\0';
		memcpy(buff,temp,t1-temp);
		if(0==t1)
			return 0;
		t1+=1;
	}
	return t1;
}

char* getSqliteDBServerIpaddr(char *buff)
{
	char *temp=Buffer;
	char *t1=NULL;
	t1 = strstr(temp,";");
	if(t1){
		//temp[t1-temp] = '\0';
		memcpy(buff,temp,t1-temp);
		if(0==t1)
			return 0;
		t1+=1;
	}
	return t1;
}

char* getNwMatchIpaddr()
{
	char *temp=Buffer;
	char *t1=NULL;
	t1 = strstr(temp,";");
	if(0==t1) return 0;
	return t1+1;
}

char* getF2F(int& type){
 	char *temp=Buffer;
 	temp=strstr(temp,";");
 	if(0==temp) return 0;
 	temp+=1;
 	type=*temp;
 	temp=strstr(temp,";");
 	if(0==temp) return NULL;
 	temp+=1;
 	return temp;
}

bool getMatchIris(char **ptr,int& id, int& taskid, int& size){
	char *temp=Buffer;
	temp=strstr(temp,"MATCH;");
	if(0==temp) return false;
	temp+=6;
	id=atoi(temp);

	temp=strstr(temp,";");
    if(0==temp) return false;
    temp+=1;
    taskid=atoi(temp);

    temp=strstr(temp,";");
    if(0==temp) return false;
    temp+=1;
    int fealen=atoi(temp);
	temp=strstr(temp,";");
	temp+=1;
	*ptr = temp;
	size = fealen;
	return true;
}

int getSpoof(){
   	char *temp=Buffer;
   	temp=strstr(temp,"Spoof=");
    if(0==temp) return 0;
    temp+=7;

    int cLen=atoi(temp);
  	return cLen;
}

static int calcMessageSize(int width, int height){ return width*height+2048;}

protected:

char *getDB(int& size){
   	char *temp=Buffer;

   	temp=strstr(temp,";");
    if(0==temp) return 0;
    temp+=1;

    int cLen=atoi(temp);

    temp=strstr(temp,";");
   	if(0==temp) return 0;
   	temp+=1;

   	size=cLen;
   	return temp;
}

    bool isDoneRecvDB(){
    	// look for content length and make sure we have rcvd that many bytes
    	char *temp=Buffer;
    	temp=strstr(temp,";");
    	if(0==temp) return false;
    	temp+=1;
    	int cLen=atoi(temp);
    	temp=strstr(temp,";");
    	if(0==temp) return false;
    	int expectedSize=temp-Buffer+cLen+1;
    	if(Size<expectedSize) return false;
    		return true;
    }

	bool  IsDoneHTTP(){
		// look for content length and make sure we have rcvd that many bytes
		char *temp=Buffer;
		temp=strstr(temp,"-Length: ");
		if(0==temp) return false;
		temp+=9;
		int cLen=atoi(temp);
		temp=10+strstr(temp,"----------------------------");
		if(0==temp) return false;
		temp=strstr(temp,"----------------------------");
		if(0==temp) return false;
		int expectedSize=temp-Buffer+cLen;
		if(Size<expectedSize) return false;
		return true;
	}
bool  IsDoneXML(){
		// look for closing tag in last few bytes
		char *temp=Buffer;
		if(Size<16) return false;
		temp+=(Size-16);	//scroll to the end
		temp=strstr(temp,"</images>");
		if(0==temp) return false;
		return true;
	}

bool  IsDoneMatch(){
	// look for content length and make sure we have rcvd that many bytes
	char *temp=Buffer;
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;
	int fealen=atoi(temp);
	temp=strstr(temp,";");
	if(0==temp) return false;
	temp+=1;

	int expectedSize=temp-Buffer+fealen;
	//printf("Size %d Exp %d \n",Size,expectedSize);
	if(Size<expectedSize) return false;
	return true;
}

char* getFrameHTTP(int *length) const{
		char *temp=Buffer;
		std::string ret;
		temp=strstr(temp,"octet-stream");
		if(0==temp) return 0;
		temp=strstr(temp+13,"octet-stream");
		if(0==temp) return 0;
		temp+=16;
		*length=Size-(temp-Buffer);
		return temp;
	}
char *getFrameXML(int *length){
		char *temp=Buffer;
		std::string ret;
		temp=strstr(temp,"maxValue=");
		if(0==temp) return 0;
		temp=strstr(temp,">");
		if(0==temp) return 0;
		temp+=1;
		char *end=strstr(temp,"</image>");
		if(0==end) return 0;
		std::string encoded(temp,end-temp);
		tempStr=base64_decode(encoded);
		*length=tempStr.length();
		return (char *)tempStr.c_str();
	}
std::string tempStr;
static const char *hb;
static int hblen;

static const char *ledMsg;
static int ledMsglen;

static const char *reloadMsg;
static int reloadMsglen;

static const char *recvdbMsg;
static int recvdbMsglen;

static const char *sqliteMsg;
static int sqliteMsglen;

static const char *doenrollMsg;
static int doenrollMsglen;

static const char *faceGrabMsg;
static int faceGrabMsglen;

static const char *motionMsg;
static int motionMsglen;

static const char *f2fMsg;
static int f2fMsglen;

static const char *setpinMsg;
static int setpinMsglen;

static const char *matchMsg;
static int matchMsglen;

static const char *pingMsg;
static int pingMsglen;

static const char *resetMsg;
static int resetMsglen;

static const char *threadPropMsg;
static int threadPropMsglen;

static const char *downloadDBMsg;
static int downloadDBMsglen;

public:
static void init(const char *hbStr, const char *ledStr, const char *reloadStr, const char *recvdbStr, const char *doenrollStr,
		const char *faceGrabStr,const char *motionStr,const char *f2fStr,const char* setpinStr,const char *matchStr,const char *pingStr,const char *resetStr,const char *threadPropStr, const char *sqliteStr, const char* downloadDBStr);



};
