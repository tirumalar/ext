
#include "FaceTracker.h"
#include "file_manip.h"

using namespace cv;
// using namespace std::chrono;
using namespace std;

#define DEBUG_SESSION

Point eyes;	// hold eye info from each frame

double scaling = 8.0;		//Used for resizing the images in py lev 3
bool mb_tempAlreadyLogged=false;
// detect_area used for finding face in a certain rect area of entire image
bool faceConfigInit;
int targetOffset;
// Rect detect_area(15/SCALE,15/SCALE,(960/(SCALE*SCALE))+15/SCALE,(1200/(SCALE*SCALE))+15/SCALE); //15 was 30
Rect no_move_area, no_move_areaX;		//Face target area
Rect search_eye_area;					//Narrow down the eye search range in face
Rect projFace;

VideoStream *vs;
Mat outImg;
Mat smallImg;
Mat smallImgBeforeRotate;
Mat dst;
Mat RotatedfaceImg;

// std::chrono:: time_point<std::chrono::system_clock> start_mode_change;

int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size, Rect &face, int min_face_size, int max_face_size);
int face_init();
float read_angle(void);

// Temperature monitoring parameters
int tempTarget; //5 mins = 60*5 = 300s
float tempLowThreshold;
float tempHighThreshold;
const char Mlogger[30] = "Motor";

#ifdef DEBUG_SESSION
//#define DEBUG_SESSION_DIR "DebugSessions/Session"
//#define DEBUG_SESSION_INFO "DebugSessions/Session/Info.txt"
FileConfiguration eyelockConf("");
std::string m_sessionDir;
std::string m_sessionInfo;

void LogSessionEvent(struct tm* tm1, struct timespec* ts, const char* msg)
{
	FILE *file = fopen(m_sessionInfo.c_str(), "a");
	if (file){
		char time_str[100];
		strftime(time_str, 100, "%Y %m %d %H:%M:%S", tm1);
		fprintf(file, "%s %09lu:%09lu %s\n", time_str, ts->tv_sec, ts->tv_nsec, msg);
		fclose(file);
	}
}

void LogSessionEvent(const char* msg)
{
	time_t timer;
	struct tm* tm1;
	time(&timer);
	tm1 = localtime(&timer);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	FILE *file = fopen(m_sessionInfo.c_str(), "a");
	if (file){
		char time_str[100];
		strftime(time_str, 100, "%Y %m %d %H:%M:%S", tm1);
		fprintf(file, "%s %09lu:%09lu %s\n", time_str, ts.tv_sec, ts.tv_nsec, msg);
		fclose(file);
	}
}
#endif /* DEBUG_SESSION */


void DoTemperatureLog()
{
	int len;
	float tempData;
	char temperatureBuf[512];

	if ((len = port_com_send_return("accel_temp()", temperatureBuf, 20)) > 0)
	{
		sscanf(temperatureBuf, "%f", &tempData);
		if (tempData > tempHighThreshold)
		{
			if(!mb_tempAlreadyLogged)
			{
				EyelockEvent("INFO:OIM Temperature is too high: %3.3f!", tempData);
				EyelockLog(logger, INFO, "OIM temperature is too high: %3.3f (threshold %3.3f)", tempData, tempHighThreshold);
				mb_tempAlreadyLogged = true;
			}
		}
		else if (tempData < tempLowThreshold)
		{
			if(!mb_tempAlreadyLogged)
			{
				EyelockEvent("INFO:OIM Temperature is too low: %3.3f!", tempData);
				EyelockLog(logger, INFO, "OIM temperature is too low: %3.3f (threshold %3.3f)", tempData, tempLowThreshold);
				mb_tempAlreadyLogged = true;
			}
		}
		else
		{
			if(mb_tempAlreadyLogged)
			{
				EyelockEvent("INFO:OIM Temperature restored to normal");
				EyelockLog(logger, INFO, "OIM temperature restored to normal: %3.3f (thresholds are [%3.3f,%3.3f])", tempData, tempLowThreshold, tempHighThreshold);
				mb_tempAlreadyLogged = false;
			}
		}
	}
}


FaceTracker::FaceTracker(char* filename)
:FaceConfig(filename)
,previousEye_distance(0)
,cur_pos(0)
,fileNum(0)
,noeyesframe(0)
,agc_val(0)
,agc_set_gain(0)
,agc_val_cal(3)
,thresholdVal(10)
,AGC_Counter(0)
,noFaceCounter(0)
,last_angle_move(0)
,switchedToIrisMode(false)
,bDebugSessions(false)
,bShowFaceTracking(false)
,high(false)
,low(false)
,maxAngle(136)
,minAngle(72)
,m_ProjPtr(false)
,FRAME_DELAY(60)
,bFaceMapDebug(false)
,bActiveCenterPos(false)
,bIrisToFaceMapDebug(false)
,m_nCalculatedBrightness(0)
,m_ImageWidth(1200)
,m_ImageHeight(960)
,m_AdaptiveGain(false)
,m_AdaptiveGainFactor(20000)
,m_AdaptiveGainAuxAdjust(1.25)
,m_CalibrationImageSize(1200)
,m_bCalibImageSizeIs1280(false)
{

	FRAME_DELAY = FaceConfig.getValue("FTracker.FRAMEDELAY",60);
	CENTER_POS = FaceConfig.getValue("FTracker.centerPos",164);

	cur_pos = CENTER_POS;
	tempTarget = FaceConfig.getValue("FTracker.tempReadingTimeInMinutes",5);
	tempTarget = tempTarget * 60;	//converting into sec
	tempHighThreshold = FaceConfig.getValue("FTracker.OimTemperatureHighThreshold", 80.0f);
	tempLowThreshold = FaceConfig.getValue("FTracker.OimTemperatureLowThreshold", 0.0f);
	
	//For motor acceleration
	initialMotion = FaceConfig.getValue("FTracker.IntialSpeedMotion",150);
	finalMotion = FaceConfig.getValue("FTracker.FinalSpeedMotion",150);
	MotorAcceleration = FaceConfig.getValue("FTracker.MotorAcceleration",150);

	switchThreshold = FaceConfig.getValue("FTracker.switchThreshold",37);
	errSwitchThreshold = FaceConfig.getValue("FTracker.errSwitchThreshold",2);
	
	m_IrisLEDVolt = FaceConfig.getValue("FTracker.IrisLEDVolt",30);
	m_IrisLEDcurrentSet = FaceConfig.getValue("FTracker.IrisLEDcurrentSet",40);
	m_IrisLEDtrigger = FaceConfig.getValue("FTracker.IrisLEDtrigger",3);
	m_IrisLEDEnable = FaceConfig.getValue("FTracker.IrisLEDEnable",49);
	m_IrisLEDmaxTime = FaceConfig.getValue("FTracker.IrisLEDmaxTime",4);
	m_minPos = FaceConfig.getValue("FTracker.minPos",0);
	
	m_allLEDhighVoltEnable = FaceConfig.getValue("FTracker.allLEDhighVoltEnable",1);
	m_faceLEDVolt = FaceConfig.getValue("FTracker.faceLEDVolt",30);
	m_faceLEDcurrentSet = FaceConfig.getValue("FTracker.faceLEDcurrentSet",20);
	m_faceLEDtrigger = FaceConfig.getValue("FTracker.faceLEDtrigger",4);
	m_faceLEDEnable = FaceConfig.getValue("FTracker.faceLEDEnable",4);
	m_faceLEDmaxTime = FaceConfig.getValue("FTracker.faceLEDmaxTime",4);
	
	m_faceCamExposureTime = FaceConfig.getValue("FTracker.faceCamExposureTime",12);
	m_faceCamDigitalGain = FaceConfig.getValue("FTracker.faceCamDigitalGain",240);
	m_faceAnalogGain = FaceConfig.getValue("FTracker.faceAnalogGain",128);
	m_faceCamDataPedestal = FaceConfig.getValue("FTracker.faceCamDataPedestal",0);
	
	m_DimmingfaceAnalogGain = FaceConfig.getValue("FTracker.DimmingfaceAnalogGain",0);
	m_DimmingfaceExposureTime = FaceConfig.getValue("FTracker.DimmingfaceExposureTime",7);
	m_DimmingfaceDigitalGain = FaceConfig.getValue("FTracker.DimmingfaceDigitalGain",32);
	
	m_AuxIrisCamDataPedestal = FaceConfig.getValue("FTracker.AuxIrisCamDataPedestal",0);
	m_MainIrisCamDataPedestal = FaceConfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	// Left Aux Camera
	m_LeftAuxIrisCamExposureTime = FaceConfig.getValue("FTracker.LeftAuxIrisCamExposureTime",8);
	m_LeftAuxIrisCamDigitalGain = FaceConfig.getValue("FTracker.LeftAuxIrisCamDigitalGain",80);

	// Right Aux Camera
	m_RightAuxIrisCamExposureTime = FaceConfig.getValue("FTracker.RightAuxIrisCamExposureTime",8);
	m_RightAuxIrisCamDigitalGain = FaceConfig.getValue("FTracker.RightAuxIrisCamDigitalGain",80);

	// Left Main Camera
	m_LeftMainIrisCamExposureTime = FaceConfig.getValue("FTracker.LeftMainIrisCamExposureTime",8);
	m_LeftMainIrisCamDigitalGain = FaceConfig.getValue("FTracker.LeftMainIrisCamDigitalGain",128);

	// Right Main Camera
	m_RightMainIrisCamExposureTime = FaceConfig.getValue("FTracker.RightMainIrisCamExposureTime",8);
	m_RightMainIrisCamDigitalGain = FaceConfig.getValue("FTracker.RightMainIrisCamDigitalGain",128);

	b_EnableFaceAGC = FaceConfig.getValue("FTracker.EnableFaceAGC", true);

	m_irisAnalogGain = FaceConfig.getValue("FTracker.irisAnalogGain",144);
	m_faceAnalogGain = FaceConfig.getValue("FTracker.faceAnalogGain",128);
	m_capacitorChargeCurrent = FaceConfig.getValue("FTracker.capacitorChargeCurrent",60);


	//Reading AGC parameters
	PIXEL_TOTAL = FaceConfig.getValue("FTracker.PIXEL_TOTAL",900);

	FACE_GAIN_DEFAULT = FaceConfig.getValue("FTracker.FACE_GAIN_DEFAULT",10);
	FACE_GAIN_DEFAULT = FACE_GAIN_DEFAULT * PIXEL_TOTAL;

	agc_val = FACE_GAIN_DEFAULT;

	FACE_GAIN_STEP = FaceConfig.getValue("FTracker.FACE_GAIN_STEP", 20);
	FACE_GAIN_STEP = FACE_GAIN_STEP * PIXEL_TOTAL;
	FACE_GAIN_MAX = FaceConfig.getValue("FTracker.FACE_GAIN_MAX",80);
	FACE_GAIN_MAX = FACE_GAIN_MAX * PIXEL_TOTAL;
	FACE_GAIN_MIN = FaceConfig.getValue("FTracker.FACE_GAIN_MIN",8);
	FACE_GAIN_MIN = FACE_GAIN_MIN * PIXEL_TOTAL;
	FACE_GAIN_PER_GOAL = FaceConfig.getValue("FTracker.FACE_GAIN_PER_GOAL",10);
	FACE_GAIN_HIST_GOAL = FaceConfig.getValue("FTracker.FACE_GAIN_HIST_GOAL",float(0.1));
	FACE_CONTROL_GAIN = FaceConfig.getValue("FTracker.FACE_CONTROL_GAIN",float(500.0));
	ERROR_LOOP_GAIN = FaceConfig.getValue("FTracker.ERROR_LOOP_GAIN",float(0.08));
	ERROR_CHECK_EYES = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES",float(0.06));
	ERROR_CHECK_EYES_HT = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_HT",float(0.0));
	ERROR_CHECK_EYES_HB = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_HB",float(0.06));
	ERROR_CHECK_EYES_WL = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_WL",float(0.0));
	ERROR_CHECK_EYES_WR = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_WR",float(0.0));


	MIN_FACE_SIZE = FaceConfig.getValue("FTracker.MIN_FACE_SIZE",10);
	MAX_FACE_SIZE = FaceConfig.getValue("FTracker.MAX_FACE_SIZE",70);


	bShowFaceTracking = FaceConfig.getValue("FTracker.ShowFaceTracking", false);

	bFaceMapDebug = FaceConfig.getValue("FTracker.FaceMapDebug", false);

	bActiveCenterPos = FaceConfig.getValue("FTracker.ActivateCenterPosTest", false);

	// Flag to enable/disable AES Encrption in port com
	bDoAESEncryption = FaceConfig.getValue("FTracker.AESEncrypt", false);

	m_Motor_Bottom_Offset = FaceConfig.getValue("FTracker.MotorBottomOffset",5);
	m_Motor_Range = FaceConfig.getValue("FTracker.MotorRange",55);


	// Eyelock.ini Parameters	
	FileConfiguration EyelockConfig("/home/root/Eyelock.ini");
	// Get the width and height of Image from Eyelock.ini
	m_ImageWidth = EyelockConfig.getValue("FrameSize.width", 1200);
	m_ImageHeight = EyelockConfig.getValue("FrameSize.height", 960);

	detect_area.x = 15/SCALE;
	detect_area.y = 15/SCALE;
	detect_area.width = (m_ImageHeight/(SCALE*SCALE))+15/SCALE;
	detect_area.height = (m_ImageWidth/(SCALE*SCALE))+15/SCALE; //15 was 30
	m_ToneVolume = EyelockConfig.getValue("GRI.AuthorizationToneVolume", 40);
	m_FixedAudSetVal = EyelockConfig.getValue("Eyelock.FixedAudSetValue", 5);
	m_ImageAuthentication = EyelockConfig.getValue("Eyelock.ImageAuthentication", true);

	m_OIMFTPEnabled = EyelockConfig.getValue("Eyelock.OIMFTPEnable", true);

	bIrisToFaceMapDebug = EyelockConfig.getValue("Eyelock.IrisToFaceMapDebug", false);

	m_EyelockIrisMode = EyelockConfig.getValue("Eyelock.IrisMode",1);

	int m_ImageSize = m_ImageWidth * m_ImageHeight;
	m_LeftCameraFaceInfo.faceImagePtr = new unsigned char[m_ImageSize];
	m_RightCameraFaceInfo.faceImagePtr = new unsigned char[m_ImageSize];

	// Adaptive Gain
	m_AdaptiveGain = EyelockConfig.getValue("FTracker.AdaptiveGain", false);
	m_AdaptiveGainFactor = EyelockConfig.getValue("FTracker.AdaptiveGainFactor",20000);
	m_AdaptiveGainAuxAdjust = EyelockConfig.getValue("FTracker.AdaptiveGainAuxAdjust",float(1.25));
	
	if (m_OIMFTPEnabled) {
		// Calibration Parameters from CalRectFromOIM.ini
		FileConfiguration CalRectConfig("/home/root/CalRect.ini");
		rectX = CalRectConfig.getValue("FTracker.targetRectX", 0);
		rectY = CalRectConfig.getValue("FTracker.targetRectY", 497);
		rectW = CalRectConfig.getValue("FTracker.targetRectWidth", 960);
		rectH = CalRectConfig.getValue("FTracker.targetRectHeight", 121);
		// ImageSize used for calibration
		m_CalibrationImageSize = CalRectConfig.getValue("FTracker.ImageWidth", 1200);
		m_bCalibImageSizeIs1280 = CalRectConfig.getValue("FTracker.CalibImageSizeIs1280", false);

	} else {
		// To support old devices which don't have cal rect file stored on OIM
		FileConfiguration CalibDefaultConfig("/home/root/data/calibration/CalRect.ini");
		rectX = CalibDefaultConfig.getValue("FTracker.targetRectX", 0);
		rectY = CalibDefaultConfig.getValue("FTracker.targetRectY", 497);
		rectW = CalibDefaultConfig.getValue("FTracker.targetRectWidth", 960);
		rectH = CalibDefaultConfig.getValue("FTracker.targetRectHeight", 121);

		// ImageSize used for calibration
		m_CalibrationImageSize = CalibDefaultConfig.getValue("FTracker.ImageWidth", 1200);
		m_bCalibImageSizeIs1280 = CalibDefaultConfig.getValue("FTracker.CalibImageSizeIs1280", false);

	}

	// Correction factor for 1200 image calibration for 1280x960 new firmware
	mb1200ImageCalibration = EyelockConfig.getValue("Eyelock.1200ImageCalibration", false);
	m1280shiftval_y = EyelockConfig.getValue("Eyelock.1280shiftval_y", 80);

	// Calibration correction for 1200 image calibration but input image stream size is 1280
	if(m_CalibrationImageSize == 1200 && m_bCalibImageSizeIs1280 == false){
		rectY = rectY + m1280shiftval_y;
	}
#ifdef DEBUG_SESSION
	bDebugSessions = FaceConfig.getValue("FTracker.DebugSessions",false);
	m_sessionDir = string(EyelockConfig.getValue("Eyelock.DebugSessionDir","DebugSessions/Session"));
	m_sessionInfo = m_sessionDir + "/Info.txt";
#endif
}

FaceTracker::~FaceTracker()
{
	if(m_LeftCameraFaceInfo.faceImagePtr)
		delete [] m_LeftCameraFaceInfo.faceImagePtr;
	if(m_RightCameraFaceInfo.faceImagePtr)
		delete [] m_RightCameraFaceInfo.faceImagePtr;
}

void FaceTracker::SetExp(int cam, int val)
{
	EyelockLog(logger, TRACE, "SetExp");
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	// int fine = val - coarse*PIXEL_TOTAL;
	//sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	EyelockLog(logger, TRACE, "Setting Gain %d\n",coarse);
	//port_com_send(buff);
}

void FaceTracker::SetFaceExposure(int val)
{
	EyelockLog(logger, TRACE, "SetFaceExposure");
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	// int fine = val - coarse*PIXEL_TOTAL;
	//sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	// sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	sprintf(buff,"wcr(4,0x305e,%d)", coarse);
	// printf("%s\n", buff);
	EyelockLog(logger, TRACE, "Setting Exposure %d\n",coarse);
	port_com_send(buff);
}


void FaceTracker::MoveToAngle(float a)
{
	EyelockLog(logger, TRACE, "Motor: MoveToAngle");
	char buff[100];
	float current_a = read_angle();
	if (current_a == 0)
		return;
	float move;
	move = current_a-a;
	EyelockLog(logger, DEBUG,"Move angle diff %3.3f\n",move);
	move=-1*move*ANGLE_TO_STEPS;

	sprintf(buff,"fx_rel(%d)",(int)(move+0.5));
	EyelockLog(logger, DEBUG, "Sending by current angle %3.3f dest %3.3f: %s\n",current_a,a,buff);

	float t_start=clock();
	port_com_send(buff);
	float t_result = (float)(clock()-t_start)/CLOCKS_PER_SEC;
	EyelockLog(logger, DEBUG, "Time required to move to angle:%3.3f", t_result);
}

void FaceTracker::MoveTo(int v)
{
	EyelockLog(logger, TRACE, "MoveTo");
	EyelockLog(logger, DEBUG,"Move to command %d ",v);

	v=v-CENTER_POS;
	//v=v/ANGLE_TO_STEPS+CENTER_POSITION_ANGLE;
	v=v/ANGLE_TO_STEPS+motorCenter;

	EyelockLog(logger, DEBUG,"Value to MoveToAngle:angle = %d",v);

	MoveToAngle((float) v);
}
void FaceTracker::MoveToAbs(int v)
{
	char cmd[128];
	EyelockLog(logger, TRACE, "MoveToAbs");
	EyelockLog(logger, DEBUG,"Move to Absolute command %d ",v);

	port_com_send("fx_home");

	sprintf(cmd, "fx_abs(%i)",v);
	//printf(cmd);
	port_com_send(cmd);
	//EyelockLog(logger, DEBUG, "Moving to center position");
	EyelockLog(logger, DEBUG, "Move to ABS CENTER Position after Match/nonMatch:%d", v);
}


void FaceTracker::setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask)
{
	EyelockLog(logger, TRACE, "setRGBled");
	static int free = 1;
	static int setTime = 0;

	static std::chrono:: time_point<std::chrono::system_clock> start, end;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	if(elapsed_seconds.count()>=setTime || VIPcall==1)
	{
		free=1;

	}
	EyelockLog(logger, DEBUG, "Current time : %u",start);
	if(free)
	{
		char temp[40];
		//sprintf(temp,"fixed_set_rgbm(%d,%d,%d,%d)",R,G,B,mask);
		EyelockLog(logger, DEBUG, "fixed_set_rgbm:%d,%d,%d,%d",R,G,B,mask);
		sprintf(temp,"fixed_set_rgb(%d,%d,%d)",R,G,B);
		port_com_send(temp);
		free = 0;
		start = std::chrono::system_clock::now();
		setTime = (double)mtime/1000;
		EyelockLog(logger, DEBUG, "time set for setREGLed: settime : %u",setTime);
	}
}

char* GetTimeStamp()
{
	EyelockLog(logger, TRACE, "GetTimeStamp");
	static char timeVar[100];
	//sprintf(timeVar,"%3.3f",(float)clock()/CLOCKS_PER_SEC);
    static long int oldms;

	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	sprintf(timeVar,"%ld %ld",ms,ms-oldms);
	oldms = ms;
	return(timeVar);
}

void FaceTracker::MainIrisSettings(int FaceWidth, int CameraState)
{
	EyelockLog(logger, TRACE, "MainIrisSettings");

	char cmd[512];

	//return;
	EyelockLog(logger, DEBUG, "configuring Main Iris settings");
	// printf("configuring Main Iris settings\n");
	//Iris configuration of LED
	sprintf(cmd,"psoc_write(3,%i)| psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(6,%i)", m_IrisLEDEnable, m_IrisLEDVolt, m_IrisLEDcurrentSet, m_IrisLEDtrigger, m_IrisLEDmaxTime);
	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("psoc_write(3,0x31)| psoc_write(2,30) | psoc_write(5,40) | psoc_write(4,3) | psoc_write(6,4)");
	//Face cameras configuration
	//port_com_send("wcr(0x83,0x3012,12) | wcr(0x83,0x301e,0) | wcr(0x83,0x305e,128)");

	EyelockLog(logger, DEBUG, "Values in MainIrisSettings IrisLEDEnable:%d, IrisLEDVolt:%d, IrisLEDcurrentSet:%d, IrisLEDtrigger:%d, IrisLEDmaxTime:%d ", m_IrisLEDEnable, m_IrisLEDVolt, m_IrisLEDcurrentSet, m_IrisLEDtrigger, m_IrisLEDmaxTime);
	if(m_AdaptiveGain){
		AdaptiveGain(FaceWidth, CameraState);
	}
}

void FaceTracker::CreateFaceWidthGainMap()
{
#if 0
	FaceWidthGainMap.insert ( std::pair<int,int>(20,255));
	FaceWidthGainMap.insert ( std::pair<int,int>(21,255));
	FaceWidthGainMap.insert ( std::pair<int,int>(22,255));
	FaceWidthGainMap.insert ( std::pair<int,int>(23,240));
	FaceWidthGainMap.insert ( std::pair<int,int>(24,221));
	FaceWidthGainMap.insert ( std::pair<int,int>(25,205));
	FaceWidthGainMap.insert ( std::pair<int,int>(26,190));
	FaceWidthGainMap.insert ( std::pair<int,int>(27,177));
	FaceWidthGainMap.insert ( std::pair<int,int>(28,165));
	FaceWidthGainMap.insert ( std::pair<int,int>(29,154));
	FaceWidthGainMap.insert ( std::pair<int,int>(30,145));
	FaceWidthGainMap.insert ( std::pair<int,int>(31,136));
	FaceWidthGainMap.insert ( std::pair<int,int>(32,128));
	FaceWidthGainMap.insert ( std::pair<int,int>(33,121));
	FaceWidthGainMap.insert ( std::pair<int,int>(34,115));
	FaceWidthGainMap.insert ( std::pair<int,int>(35,109));
	FaceWidthGainMap.insert ( std::pair<int,int>(36,103));
	FaceWidthGainMap.insert ( std::pair<int,int>(37,98));
	FaceWidthGainMap.insert ( std::pair<int,int>(38,93));
	FaceWidthGainMap.insert ( std::pair<int,int>(39,89));
	FaceWidthGainMap.insert ( std::pair<int,int>(40,85));
	FaceWidthGainMap.insert ( std::pair<int,int>(41,81));
	FaceWidthGainMap.insert ( std::pair<int,int>(42,77));
	FaceWidthGainMap.insert ( std::pair<int,int>(43,74));
	FaceWidthGainMap.insert ( std::pair<int,int>(44,71));
	FaceWidthGainMap.insert ( std::pair<int,int>(45,68));
	FaceWidthGainMap.insert ( std::pair<int,int>(46,65));
	FaceWidthGainMap.insert ( std::pair<int,int>(47,63));
	FaceWidthGainMap.insert ( std::pair<int,int>(48,61));
	FaceWidthGainMap.insert ( std::pair<int,int>(49,58));
	FaceWidthGainMap.insert ( std::pair<int,int>(50,56));
	FaceWidthGainMap.insert ( std::pair<int,int>(51,54));
	FaceWidthGainMap.insert ( std::pair<int,int>(52,52));
	FaceWidthGainMap.insert ( std::pair<int,int>(53,51));
	FaceWidthGainMap.insert ( std::pair<int,int>(54,49));
	FaceWidthGainMap.insert ( std::pair<int,int>(55,47));
	FaceWidthGainMap.insert ( std::pair<int,int>(56,46));
	FaceWidthGainMap.insert ( std::pair<int,int>(57,44));
	FaceWidthGainMap.insert ( std::pair<int,int>(58,43));
	FaceWidthGainMap.insert ( std::pair<int,int>(59,42));
	FaceWidthGainMap.insert ( std::pair<int,int>(60,40));
	FaceWidthGainMap.insert ( std::pair<int,int>(61,39));
#else
	FaceWidthGainMap.insert ( std::pair<int,int>(20,255));
	FaceWidthGainMap.insert ( std::pair<int,int>(21,241));
	FaceWidthGainMap.insert ( std::pair<int,int>(22,220));
	FaceWidthGainMap.insert ( std::pair<int,int>(23,202));
	FaceWidthGainMap.insert ( std::pair<int,int>(24,186));
	FaceWidthGainMap.insert ( std::pair<int,int>(25,172));
	FaceWidthGainMap.insert ( std::pair<int,int>(26,160));
	FaceWidthGainMap.insert ( std::pair<int,int>(27,149));
	FaceWidthGainMap.insert ( std::pair<int,int>(28,139));
	FaceWidthGainMap.insert ( std::pair<int,int>(29,130));
	FaceWidthGainMap.insert ( std::pair<int,int>(30,122));
	FaceWidthGainMap.insert ( std::pair<int,int>(31,115));
	FaceWidthGainMap.insert ( std::pair<int,int>(32,108));
	FaceWidthGainMap.insert ( std::pair<int,int>(33,102));
	FaceWidthGainMap.insert ( std::pair<int,int>(34,96));
	FaceWidthGainMap.insert ( std::pair<int,int>(35,91));
	FaceWidthGainMap.insert ( std::pair<int,int>(36,87));
	FaceWidthGainMap.insert ( std::pair<int,int>(37,82));
	FaceWidthGainMap.insert ( std::pair<int,int>(38,78));
	FaceWidthGainMap.insert ( std::pair<int,int>(39,75));
	FaceWidthGainMap.insert ( std::pair<int,int>(40,71));
	FaceWidthGainMap.insert ( std::pair<int,int>(41,68));
	FaceWidthGainMap.insert ( std::pair<int,int>(42,65));
	FaceWidthGainMap.insert ( std::pair<int,int>(43,62));
	FaceWidthGainMap.insert ( std::pair<int,int>(44,60));
	FaceWidthGainMap.insert ( std::pair<int,int>(45,57));
	FaceWidthGainMap.insert ( std::pair<int,int>(46,55));
	FaceWidthGainMap.insert ( std::pair<int,int>(47,53));
	FaceWidthGainMap.insert ( std::pair<int,int>(48,51));
	FaceWidthGainMap.insert ( std::pair<int,int>(49,49));
	FaceWidthGainMap.insert ( std::pair<int,int>(50,47));
	FaceWidthGainMap.insert ( std::pair<int,int>(51,46));
	FaceWidthGainMap.insert ( std::pair<int,int>(52,44));
	FaceWidthGainMap.insert ( std::pair<int,int>(53,43));
	FaceWidthGainMap.insert ( std::pair<int,int>(54,41));
	FaceWidthGainMap.insert ( std::pair<int,int>(55,40));
	FaceWidthGainMap.insert ( std::pair<int,int>(56,39));
	FaceWidthGainMap.insert ( std::pair<int,int>(57,37));
	FaceWidthGainMap.insert ( std::pair<int,int>(58,36));
	FaceWidthGainMap.insert ( std::pair<int,int>(59,35));
	FaceWidthGainMap.insert ( std::pair<int,int>(60,34));
	FaceWidthGainMap.insert ( std::pair<int,int>(61,33));
#endif
}

int FaceTracker::CalculateGain(int facewidth)
{
	int gain = 12200 / pow(facewidth, 2) ;

	if(gain >= 255)
		gain = 255;
	if(gain <= 32)
		gain = 32;
	return gain;
}

int FaceTracker::CalculateGainWithKH(int facewidth, int CameraState)
{
	int KF = m_AdaptiveGainFactor;
	int gain;

// 	printf("m_AdaptiveGainFactor...%d  Adjust----%f\n", m_AdaptiveGainFactor, m_AdaptiveGainAuxAdjust);
	if(CameraState == STATE_AUX_IRIS){
		KF = m_AdaptiveGainAuxAdjust * m_AdaptiveGainFactor;
		gain = m_LeftAuxIrisCamDigitalGain;
	}else{
		gain = m_LeftMainIrisCamDigitalGain;
	}

	float KG = 0.000173;
	int KH = 8;

	// gain = KG * (KF/facewidth + KH)2
	if(facewidth)
		gain = KG * pow(((KF/facewidth) + KH), 2);

	if(gain >= 255)
		gain = 255;
	if(gain <= 32)
		gain = 32;

	return gain;
}

void FaceTracker::AdaptiveGain(int faceWidth, int CameraState)
{
	EyelockLog(logger, TRACE, "AdaptiveGain");

	char buff[512];
	// Set the gain based on distance
	// int Gain = FaceWidthGainMap.find(faceWidth)->second;
	int Gain = CalculateGainWithKH(faceWidth, CameraState);
	// printf("FaceTracker:: FaceWidth %d Gain %d\n", faceWidth, Gain);
	sprintf(buff,"wcr(0x1f,0x305e,%d)", Gain);
	port_com_send(buff);
}

void FaceTracker::SwitchIrisCameras(bool mode)
{
	EyelockLog(logger, TRACE, "SwitchIrisCameras");

	char cmd[100];
	if (mode)
		sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY); // --- Main
	else
		sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY); // --- AUX

	port_com_send(cmd);
}

void FaceTracker::readFaceAnalogGainReg(uint32_t Value)
{
	int count = 0;
	char cmd[512];
	int len;
	char buffer[512];
	while(count < 2)
	{
		sprintf(cmd,"rcr(0x04,0x30b0)");

		if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
		{
			PortComLog(logger, ERROR, "Failed to read register 0x30b0!!");
		}
		if (strstr(buffer,"0x90"))
		{
			// printf("readFaceAnalogGainReg %d %s\n", Value, buffer);
			PortComLog(logger, DEBUG, "Face camera Analog gain write successful attempts..%d", count);
			break;
		}
		else
		{
			sprintf(cmd,"wcr(0x04,0x30b0,%i)",Value);
			port_com_send(cmd);
			PortComLog(logger, ERROR, "Face camera Analog gain write again attempts..%d", count);
		}
		count++;
	}
}

void FaceTracker::readDimFaceGainregVal(uint32_t Value)
{
	int count = 0;
	char cmd[512];
	int len;
	char buffer[512];

	while(count < 2)
	{
		sprintf(cmd,"rcr(0x04,0x30b0)");
		if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
		{
			PortComLog(logger, ERROR, "Failed to read register 0x30b0!!");
		}

		if (strstr(buffer,"0x80"))
		{
			// printf("readDimFaceGainregVal %d %s\n", Value, buffer);
			PortComLog(logger, DEBUG, "Face camera Analog dimming gain write successful attempts..%d", count);
			break;
		}
		else
		{
			sprintf(cmd,"wcr(0x04,0x30b0,%i)",Value);
			port_com_send(cmd);
			PortComLog(logger, ERROR, "Face camera Analog dimming gain write again attempts..%d", count);
		}
		count++;
	}
}


void FaceTracker::SetFaceMode()
{
	// printf("Inside setFaceMode\n");
	EyelockLog(logger, TRACE, "SetFaceMode");
	if (currnet_mode==MODE_FACE)
	{
		EyelockLog(logger, DEBUG, "Don't need to change Face camera");
		return;
	}

	char cmd[512];
	EyelockLog(logger, DEBUG, "Setting Face Mode\n");
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)", m_faceLEDVolt, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable);
	EyelockLog(logger, DEBUG, "Face camera settings-faceLEDVolt:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d", m_faceLEDVolt, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable);
	//printf(cmd);
	port_com_send(cmd);
//	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
//	port_com_send("psoc_write(2,30) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)");
	sprintf(cmd, "wcr(4,0x3012,%i)  | wcr(4,0x305e,%i)", m_faceCamExposureTime, m_faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "Face camera exposure and gain settings -faceCamExposureTime:%d, faceCamDigitalGain:%d",m_faceCamExposureTime, m_faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(4,0x3012,7)  | wcr(4,0x305e,0xfe)");
	sprintf(cmd,"wcr(0x04,0x30b0,%i)",((m_faceAnalogGain&0x3)<<4) | 0X80);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Face camera Analog gain:%d",(((m_faceAnalogGain&0x3)<<4) | 0X80));

	// Read the register 0x30b0
	unsigned int Value = ((m_faceAnalogGain&0x3)<<4) | 0X80;
	readFaceAnalogGainReg(Value);

	agc_val= FACE_GAIN_DEFAULT;
	EyelockLog(logger, DEBUG, "Face camera gain default:%d", FACE_GAIN_DEFAULT);
	// start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_FACE;
	//port_com_send("fixed_set_rgb(100,100,100)");

	sprintf(cmd,"set_cam_mode(0x04,%d)",FRAME_DELAY);
	port_com_send(cmd);
}

void FaceTracker::MoveRelAngle(float a)
{
	FILE *file;
	float pr_time;
	EyelockLog(logger, TRACE, "MoveRelAngle");
	// add a limit check to make sure we are not out of bounds
	char buff[100];
	float move;
	float current_a = read_angle();
	EyelockLog(logger, DEBUG, "Current_a=%f ; next_a=%f\n",current_a,a);

	MotorLog(Mlogger, DEBUG, "Current angle = ,%03.3f, next Angle = ,%03.3f, ",current_a,a);
	 // make sure we are not asked to go bellow the bottom
	if ((a>0)&&((current_a -a)<motorBottom))
	{
		MotorLog(Mlogger, DEBUG, "Before correction %3.3f  bot %3.3f ",a,motorBottom);
		a = -1*(motorBottom-current_a);
		// dont move in opposite direction we are there so too bad
		if (a<0)
			a=0;
		MotorLog(Mlogger, DEBUG, "Bottom Hit---------- ---  %3.3f",a);
	}
	if((a < 0)&&(current_a-a)>motorTop)
	{
		MotorLog(Mlogger, DEBUG, "Before correction for Motor top %3.3f  top %3.3f ",a,motorTop);
		a = -1*(motorTop-current_a);

		MotorLog(Mlogger, DEBUG, "motorTop Hit---------- ---  %3.3f",a);

	}


	move=-1*a*ANGLE_TO_STEPS;



	EyelockLog(logger, DEBUG, "limiting small movements based on relative changes and face size changes:diffEyedistance %f", move);
	//limiting small movements based on relative changes and face size changes
	//printf("Current_a=%f ; next_a=%f\n",current_a,a);
	
	//low angle condition but the motor wants to move up
	low = ((int)(abs(current_a)) < minAngle && (a < 0)) ? true : false;

	//high angle conditions but the motor wats top move down
	high = ((int)(abs(current_a)) > maxAngle && (a > 0)) ? true : false;

	//move angle range
	angleRange = ((int)(abs(current_a)) <= maxAngle && (int)(abs(current_a)) >= minAngle) ? true : false;

	//printf("low and high::::::::::::::::::::: %i 	%i\n", low, high);
	

#if 0
	if (abs(move) > smallMoveTo && angleRange)
	{
		sprintf(buff,"fx_rel(%d)",(int)move);
		EyelockLog(logger, DEBUG, "In move angle range --- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
		port_com_send(buff);
		//printf("fx_rel(%d)\n",(int)(abs(move)));
	}
	else{

		if (low){
			sprintf(buff,"fx_rel(%d)",(int)move);
			EyelockLog(logger, DEBUG, "low angle condition but the motor wants to move up --- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
			port_com_send(buff);
			//printf("fx_rel(%d)\n",(int)(abs(move)));
		}

		if(high){
			sprintf(buff,"fx_rel(%d)",(int)move);
			EyelockLog(logger, DEBUG, "high angle conditions but the motor wats top move down -- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
			port_com_send(buff);
			//printf("fx_rel(%d)\n",(int)(abs(move)));
		}

	}

#else
	if (abs(move) > smallMoveTo)	//&& diffEyedistance >= errSwitchThreshold
	{
		sprintf(buff,"fx_rel(%d)",(int)move);
		EyelockLog(logger, DEBUG, "Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
		port_com_send(buff,&pr_time);
	}
#endif

	usleep(50000);
	current_a = read_angle();

	MotorLog(Mlogger, DEBUG, "After move angle = , %03.3f, Processing Time = ,%03.3f",current_a,pr_time);

}


void FaceTracker::DimmFaceForIris()
{
	EyelockLog(logger, TRACE, "DimmFaceForIris");
	char cmd[512];
	
	// printf("Dimming face cameras!!!");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x305e,%i)", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	EyelockLog(logger, DEBUG, "Dimming face cameras -DimmingfaceExposureTime:%d, DimmingfaceDigitalGain:%d", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	port_com_send(cmd);

	//Need to change this analog gain setting
	{
		char cmd[512];
		sprintf(cmd,"wcr(0x04,0x30b0,%i)",((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80);
		EyelockLog(logger, DEBUG, "FACE_GAIN_MIN:%d DimmingfaceAnalogGain:%d", FACE_GAIN_MIN, (((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80));
		port_com_send(cmd);
	}
	unsigned int Value = ((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80;
	readDimFaceGainregVal(Value);
	agc_val = FACE_GAIN_MIN;

}

#if 0 // Commented as the function is not used
void FaceTracker::SetIrisMode(float CurrentEye_distance)
{

	EyelockLog(logger, TRACE, "SetIrisMode");
	char cmd[512];
	
	// printf("Dimming face cameras!!!");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x305e,%i)", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	EyelockLog(logger, DEBUG, "Dimming face cameras -DimmingfaceExposureTime:%d, DimmingfaceDigitalGain:%d", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x305e,0x20)");
	{
		char cmd[512];
		sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80);
		EyelockLog(logger, DEBUG, "FACE_GAIN_MIN:%d DimmingfaceAnalogGain:%d", FACE_GAIN_MIN, (((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80));
		port_com_send(cmd);
	}
	agc_val = FACE_GAIN_MIN;

	//switching cameras
	//EyelockLog(logger, DEBUG, "previousEye_distance: %i; CurrentEye_distance: %i; diffEyedistance: %i\n", previousEye_distance, CurrentEye_distance, diffEyedistance);
	EyelockLog(logger, DEBUG, "previousEye_distance: %i; CurrentEye_distance: %i; \n", previousEye_distance, CurrentEye_distance);

	if (currnet_mode==MODE_FACE)
			errSwitchThreshold =0;

	if (CurrentEye_distance >= (switchThreshold+errSwitchThreshold)) //&& diffEyedistance <= errSwitchThreshold)
	{
		if(currnet_mode==MODE_EYES_MAIN)
		{
			printf("Dont need to change Main cam");
			EyelockLog(logger, DEBUG, "Don't need to change Main cameras");
			return;
		}

		MainIrisSettings();											//change to Iris settings
		SwitchIrisCameras(true);									//switch cameras
		currnet_mode = MODE_EYES_MAIN;								//set current mode
	 	previousEye_distance = CurrentEye_distance;					//save current eye distance for later use
		printf("Turning on Main Cameras\n");
		EyelockLog(logger, DEBUG, "Turning on Main Cameras");
		//port_com_send("fixed_set_rgb(100,0,0)");
	}
	else if (CurrentEye_distance < (switchThreshold-errSwitchThreshold))// && diffEyedistance <= errSwitchThreshold)
	{
		if(currnet_mode==MODE_EYES_AUX)
		{
			printf("Dont need to change Aux cam");
			EyelockLog(logger, DEBUG, "Dont need to change Aux camera");
			return;
		}

		MainIrisSettings();
		SwitchIrisCameras(false);
		currnet_mode = MODE_EYES_AUX;
		previousEye_distance = CurrentEye_distance;
		printf("Turning on AUX Cameras\n");
		EyelockLog(logger, DEBUG, "Turning on AUX Cameras");
		//port_com_send("fixed_set_rgb(100,100,0)");
	}
/*	else if (CurrentEye_distance >= switchThreshold && diffEyedistance > errSwitchThreshold)
	{
		if(currnet_mode==MODE_EYES_MAIN)
		{
			printf("Dont need to change Main cam");
			EyelockLog(logger, DEBUG, "Don't need to change Main cameras");
			return;
		}
		MainIrisSettings();
		SwitchIrisCameras(true);
		currnet_mode = MODE_EYES_MAIN;
		previousEye_distance = CurrentEye_distance;
		printf("Turning on Main Cameras\n");
		EyelockLog(logger, DEBUG, "Turning on Main cameras");
		//port_com_send("fixed_set_rgb(100,0,0)");
	}
	else if (CurrentEye_distance < switchThreshold && diffEyedistance > errSwitchThreshold)
	{
		if(currnet_mode==MODE_EYES_AUX)
		{
			printf("Dont need to change Aux cam");
			EyelockLog(logger, DEBUG, "Dont need to change Aux camera");
			return;
		}
		//AuxIrisSettings();			//use this function if we have different settings for aux and main cameras
		// AS we are using same LED setting for aux and main, Im calling this function at the very beginning
		MainIrisSettings();
		SwitchIrisCameras(false);
		currnet_mode = MODE_EYES_AUX;
		previousEye_distance = CurrentEye_distance;
		printf("Turning on AUX Cameras\n");
		EyelockLog(logger, DEBUG, "Turning on AUX cameras");
		//port_com_send("fixed_set_rgb(100,100,0)");
	}*/
	IrisFrameCtr=0;
	// start_mode_change = std::chrono::system_clock::now();


}
#endif
#if 0
cv::Rect FaceTracker::seacrhEyeArea(cv::Rect no_move_area){
	/// printf("no_move_area 	x: %d	y: %d	w: %d	h: %d\n", no_move_area.x, no_move_area.y, no_move_area.height, no_move_area.width);
	//printf("ERROR_CHECK_EYES %3.3f \n", ERROR_CHECK_EYES);

	float hclip = float(no_move_area.height - float(no_move_area.height * ERROR_CHECK_EYES));
	//printf("hclip::::: %3.3f\n", hclip);

	//float yclip = cvRound(hclip/2.0);
	float yclip = hclip/2.0;
	//printf("yclip::::: %3.3f\n", yclip);


	Rect modRect;
	modRect.x = no_move_area.x;
	modRect.width = no_move_area.width;

#if 0
	//Center the the search rect
	modRect.y = no_move_area.y + yclip;
	modRect.height = no_move_area.height - hclip;
#else
	//adjust the search REect to Top
	modRect.y = no_move_area.y;
	modRect.height = int(float(no_move_area.height * ERROR_CHECK_EYES));
#endif

	// printf("search_eye_area 	x: %d	y: %d	w: %d	h: %d\n", modRect.x, modRect.y, modRect.height, modRect.width);

#if 1 // To check if needed
	if(modRect.x < 0)
		modRect.x = 0;
	if(modRect.y < 0)
			modRect.y = 0;
	if(modRect.width > WIDTH)
			modRect.width = WIDTH;
	if(modRect.height > HEIGHT)
			modRect.height = HEIGHT;
#endif

	return modRect;
}
#else
cv::Rect FaceTracker::seacrhEyeArea(cv::Rect no_move_area){
	EyelockLog(logger, TRACE, "Entering > seacrhEyeArea()");
	if (float(ERROR_CHECK_EYES_HT +  ERROR_CHECK_EYES_HB) >= 0.9){
		ERROR_CHECK_EYES_HT = 0.0;
		ERROR_CHECK_EYES_HT = 0.4;
	}

	float hclipT = float(no_move_area.height * ERROR_CHECK_EYES_HT);
	float hclipB = float(no_move_area.height * ERROR_CHECK_EYES_HB);

	Rect modRect;
	modRect.x = no_move_area.x;
	modRect.width = no_move_area.width;


	//adjust the search REect to Top
	modRect.y = no_move_area.y + int(hclipT);
	modRect.height = no_move_area.height - int(hclipT + hclipB);

	// printf("search_eye_area 	x: %d	y: %d	w: %d	h: %d\n", modRect.x, modRect.y, modRect.height, modRect.width);

	if(modRect.x < 0)
		modRect.x = 0;
	if(modRect.y < 0)
			modRect.y = 0;
	if(modRect.width > m_ImageWidth)
			modRect.width = m_ImageWidth;
	if(modRect.height > m_ImageHeight)
			modRect.height = m_ImageHeight;


	return modRect;
}
#endif


cv::Rect FaceTracker::adjustWidthDuringFaceDetection(cv::Rect face){
	EyelockLog(logger, TRACE, "Entering > adjustWidthDuringFaceDetection()");
	cv::Rect modFace;
     
	if(bFaceMapDebug) 
		EyelockLog(logger, DEBUG, "Before adjustWidthDuringFaceDetection Function face.x	%d face.y	%d face.height	%d face.width	%d\n", face.x, face.y, face.height, face.width);

	if (float(ERROR_CHECK_EYES_WL +  ERROR_CHECK_EYES_WR) >= 0.8){
		ERROR_CHECK_EYES_WL = 0.1;
		ERROR_CHECK_EYES_WR = 0.1;
	}

	float wclipL = float(face.width * ERROR_CHECK_EYES_WL);
	float wclipR = float(face.width * ERROR_CHECK_EYES_WR);

	modFace.x = face.x + int(wclipL);
	modFace.width = face.width - int(wclipL + wclipR);

	modFace.y = face.y;
	modFace.height = face.height;

	if(bFaceMapDebug) 
		EyelockLog(logger, DEBUG, "After adjustWidthDuringFaceDetection Function face.x	%d face.y	%d face.height	%d face.width	%d\n", face.x, face.y, face.height, face.width);

	return modFace;
}

#define NUMAVG 10

void FaceTracker::motorInit(){
	EyelockLog(logger, TRACE, "Entering > motorInit()");
	char cmd[512];
	sprintf(cmd,"fx_home");
	port_com_send(cmd);

	float sum = 0;
	for(int i = 0; i < NUMAVG; i++){
		sum+= read_angle();

	}
  motorBottom=sum/NUMAVG + m_Motor_Bottom_Offset;
  motorTop= motorBottom + m_Motor_Range;
  MotorLog(Mlogger, DEBUG, "Motor Bottom   %3.3f ------------",motorBottom);

}

void FaceTracker::motorInitCenterPos( int v){
	EyelockLog(logger, TRACE, "Entering > motorInitCenterPos()");
	char cmd[512];
	sprintf(cmd,"fx_abs(%d)", v);
	port_com_send(cmd);

	float sum = 0;
	for(int i = 0; i < NUMAVG; i++){
		sum+= read_angle();

	}
	motorCenter=sum/NUMAVG;
	MotorLog(Mlogger, DEBUG, "Motor motorCenter   %3.3f ------------",motorCenter);
}

// Main DoStartCmd configuration for Eyelock matching
void FaceTracker::DoStartCmd()
{
	EyelockLog(logger, TRACE, "DoStartCmd");
	faceConfigInit = false;

	char cmd[512];

	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);


	//adjust Motor Motion
	sprintf(cmd,"fx_mot_set(%i,%i,%i)",initialMotion, finalMotion, MotorAcceleration);
	//printf(cmd);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Motor acceleration setting is issued");
	EyelockLog(logger, DEBUG, cmd);

	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	printf("Re Homing\n");
	sprintf(cmd,"fx_home");
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "port_com_send fx_home command is issued");

	printf("Motor Int\n");
	motorInit();

	/*//Reset the lower motion
	sprintf(cmd, "fx_abs(%i)",MIN_POS);
	EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", MIN_POS);
	port_com_send(cmd);*/

	motorInitCenterPos(CENTER_POS);

	//move to center position
	printf("Moving to Center\n");
	//MoveToAbs(CENTER_POS_TEST);
	MoveTo(CENTER_POS);
	EyelockLog(logger, DEBUG, "Move to center Position:%d", CENTER_POS);
/*
	sprintf(cmd, "fx_abs(%i)",CENTER_POS);
	EyelockLog(logger, DEBUG, "Moving to center position");
	port_com_send(cmd);
	//MoveTo(CENTER_POS);
*/
	read_angle();		//read current angle

	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n", m_faceLEDVolt, m_allLEDhighVoltEnable, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable, m_faceLEDmaxTime);

	EyelockLog(logger, DEBUG, "Configuring face LEDs faceLEDVolt:%d, allLEDhighVoltEnable:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d, faceLEDmaxTime:%d", m_faceLEDVolt, m_allLEDhighVoltEnable, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable, m_faceLEDmaxTime);
	port_com_send(cmd);
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");

	//port_com_send("psoc_write(9,90)");	// charge cap for max current 60 < range < 95
	sprintf(cmd, "psoc_write(9,%i)\n", m_capacitorChargeCurrent);
	EyelockLog(logger, DEBUG, "capacitorChargeCurrent:%d", m_capacitorChargeCurrent);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95

#if 1
	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images
	EyelockLog(logger, DEBUG, "FLIPPING OF IRIS CAMERAS");
#endif

	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",m_faceCamExposureTime, m_faceCamDataPedestal, m_faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", m_faceCamExposureTime, m_faceCamDataPedestal, m_faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xFE)");
	//port_com_send("wcr(0x04,0x3012,12) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xF0)");	//Demo Config

	// Aux Left Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring AUX Left Iris Cameras");
	sprintf(cmd, "wcr(0x01,0x3012,%i) | wcr(0x01,0x301e,%i) | wcr(0x01,0x305e,%i)\n",m_LeftAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_LeftAuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", m_LeftAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_LeftAuxIrisCamDigitalGain);
	port_com_send(cmd);

	// Aux Right Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring AUX Right Iris Cameras");
	sprintf(cmd, "wcr(0x02,0x3012,%i) | wcr(0x02,0x301e,%i) | wcr(0x02,0x305e,%i)\n",m_RightAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_RightAuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", m_RightAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_RightAuxIrisCamDigitalGain);
	port_com_send(cmd);

	//Main Left Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring Main Left Iris Cameras");
	sprintf(cmd, "wcr(0x08,0x3012,%i) | wcr(0x08,0x301e,%i) | wcr(0x08,0x305e,%i)\n",m_LeftMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_LeftMainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", m_LeftMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_LeftMainIrisCamDigitalGain);
	port_com_send(cmd);

	//Main Right Iris Cameras Configuration
	EyelockLog(logger, DEBUG, "Configuring Main Right Iris Cameras");
	sprintf(cmd, "wcr(0x10,0x3012,%i) | wcr(0x10,0x301e,%i) | wcr(0x10,0x305e,%i)\n",m_RightMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_RightMainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", m_RightMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_RightMainIrisCamDigitalGain);
	port_com_send(cmd);

	EyelockLog(logger, DEBUG, "Setting up PLL");

	//following process will activate PLL for all cameras
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((m_irisAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Iris analog gain: %d", (((m_irisAnalogGain&0x3)<<4) | 0X80));

	port_com_send(cmd);
	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((m_faceAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Face analog gain: %d", (((m_faceAnalogGain&0x3)<<4) | 0X80));
	port_com_send(cmd);

	readFaceAnalogGainReg((((m_faceAnalogGain&0x3)<<4) | 0X80));
	//port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	//port_com_send("wcr(0x4,0x30b0,0x90");		//Only face camera gain is x90

///////////////////ilya///////////////////

	if(m_OIMFTPEnabled){
		// No need to load snd files; read from ftp
		EyelockLog(logger, DEBUG, "playing audio -set_audio(1)");
		if(m_ToneVolume == 0)
			port_com_send("fixed_aud_set(0)");
		else if(m_ToneVolume > 0 && m_ToneVolume < 50)
			port_com_send("fixed_aud_set(1)");
		else{
			sprintf(cmd,"fixed_aud_set(%d)",m_FixedAudSetVal);
			port_com_send(cmd); // port_com_send("fixed_aud_set(7)");
		}
		// usleep(100000);

		// Auth.raw
		sprintf(cmd,"f_load_snd(0, \"%s\")","auth.raw");
		port_com_send(cmd);
		// usleep(100000); //sleep(1)

		// rej.raw
		sprintf(cmd,"f_load_snd(1, \"%s\")","rej.raw");
		port_com_send(cmd);
		// usleep(100000);

		// tamper1.raw
		sprintf(cmd,"f_load_snd(2, \"%s\")","tamper1.raw");
		port_com_send(cmd);
		// usleep(100000);
	}else{
		//This code is for playing sound
		EyelockLog(logger, DEBUG, "playing audio -set_audio(1)");
		// port_com_send("set_audio(1)");
		if(m_ToneVolume == 0)
			port_com_send("fixed_aud_set(0)");
		else if(m_ToneVolume > 0 && m_ToneVolume < 50)
			port_com_send("fixed_aud_set(1)");
		else{
			sprintf(cmd,"fixed_aud_set(%d)",m_FixedAudSetVal);
			port_com_send(cmd); // port_com_send("fixed_aud_set(7)");
		}
		usleep(100000);
		port_com_send("data_store_set(0)");
		usleep(100000);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
		sleep(1);
		port_com_send("data_store_set(1)");
		usleep(100000);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/rej.raw");
		sleep(1);
		port_com_send("data_store_set(2)");
		usleep(100000);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/tamper1.raw");
		sleep(1);
	}

///////////////////////////////////////////////

	EyelockLog(logger, DEBUG, "Turning on Alternate cameras");
	// sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	sprintf(cmd,"set_cam_mode(0x04,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);

	EyelockLog(logger, DEBUG, "set_cam_mode(0x04,%d)");

	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");

	EyelockLog(logger, DEBUG, "Leave the PLL always ON");

	//Reading the calibrated Rect
	int targetOffset = 0; // 3;		//Adding offset

	no_move_area.x = rectX/scaling;
	no_move_area.y = rectY/scaling + targetOffset;
	no_move_area.width = rectW/scaling;
	no_move_area.height = (rectH)/scaling -targetOffset*2;

	// printf("no_move_area 	x: %d	y: %d	w: %d	h: %d\n", no_move_area.x, no_move_area.y, no_move_area.height, no_move_area.width);
	search_eye_area = seacrhEyeArea(no_move_area);

	// system("touch /home/root/Eyelock.run");
	faceConfigInit = true;

	// Init the seed with 0 during startup
	char cmd1[100];
	sprintf(cmd1,"cam_set_seed(%i)", 0);		//Set the seed
	port_com_send(cmd1);
}

float FaceTracker::CalcExposureLevel(int width, int height,unsigned char *dsty, int limit)
{
	EyelockLog(logger, TRACE, "CalcExposureLevel");
	//Percentile and Average Calculation
	unsigned char *dy = (unsigned char *)dsty;
	double total = 0,Ptotal = 0,percentile = 0,hist[256]={0},average=0;
	int pix=0,i;
	int n = width * height;
	//int limit = 180;    // Lower limit for percentile calculation
	for (; n > 0; n--)
	{
		pix =(int) *dy;
		hist[pix]++;
		dy++;
		average=average+pix;
	}

	for(i=0;i<=255;i++)
	{
		float histValue = hist[i];
		total = total + (double)histValue;
		if(i>=limit)
			Ptotal = Ptotal + (double)histValue;
	}
	percentile = (Ptotal*100)/total;
	average=average/(width*height);

	EyelockLog(logger, TRACE, "average : %3.1f percentile : %3.1f\n",average,percentile);

	// imshow("AGC", smallImg);
	// cvWaitKey(1);
	return (float)percentile;
}

Mat FaceTracker::rotation90(Mat src)
{
	EyelockLog(logger, TRACE, "rotation90");
	transpose(src, dst);
	flip(dst,dst,0);
	return dst;
}

int FaceTracker::IrisFramesHaveEyes()
{
	EyelockLog(logger, TRACE, "IrisFramesHaveEyes");
	IrisFrameCtr++;
	//printf("Iris with eyes %d\n",IrisFrameCtr);
	//cvWaitKey(30);
	if (IrisFrameCtr>MIN_IRIS_FRAMES)
		return 0;
	else
		return 1;
}

Mat FaceTracker::preProcessingImg(Mat outImg)
{
	EyelockLog(logger, TRACE, "preProcessing");

	cv::resize(outImg, smallImgBeforeRotate, cv::Size(), (1 / scaling),
			(1 / scaling), INTER_NEAREST);	//py level 3

	EyelockLog(logger, TRACE, "After resize in preProcessingImg");
	if(bFaceMapDebug){
		imshow("OriginalFaceImage", outImg);
		cvWaitKey(1);
	}
	
	smallImg = rotation90(smallImgBeforeRotate);	//90 deg rotation

	//AGC control to block wash out images
	EyelockLog(logger, TRACE, "AGC Calculation");

#if 0
	float p;
	p = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);

	printf("[AGC] should not be here\n");
	if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	agc_val = MAX(FACE_GAIN_MIN,agc_val);
	agc_val = MIN(agc_val,FACE_GAIN_MAX);

	AGC_Counter++;

	static int agc_val_old = 0;
	if (abs(agc_val - agc_val_old) > 300) {
		SetExp(4, agc_val);
		agc_val_old = agc_val;
	}

	agc_set_gain = agc_val;
#endif

	return smallImg;
}


void FaceTracker::moveMotorToFaceTarget(float eye_size, bool bShowFaceTracking, bool bDebugSessions)
{
	EyelockLog(logger, TRACE, "Entering > moveMotorToFaceTarget");
	if ((eye_size >= MIN_FACE_SIZE) && (eye_size <= MAX_FACE_SIZE)) {// check face size

		float err;
		int MoveToLimitBound = 1;
		//err = (no_move_area.y + no_move_area.height / 2) - eyes.y;		//Following no_move_area
		//instead of following no_move_area we will use search_eye_area to make eyes at the center of no_move_area

		float denum;

		if(m_EyelockIrisMode == 2)
			denum = 1.25;
		else
			denum = 1.0/2.0;

		err = (search_eye_area.y + search_eye_area.height * denum) - eyes.y;

		EyelockLog(logger, DEBUG,
				"abs err----------------------------------->  %d\n", abs(err));
		err = (float) err * (float) SCALE * (float) ERROR_LOOP_GAIN;

		// if we need to move
		if (abs(err) > MoveToLimitBound) {
			EyelockLog(logger, DEBUG, "Switching ON IRIS LEDs!!!!\n");

#ifdef DEBUG_SESSION
			if (bDebugSessions) {
				struct stat st = { 0 };
				if (stat(m_sessionDir.c_str(), &st) == -1) {
					mkdir(m_sessionDir.c_str(), 0777);
				}

				if (switchedToIrisMode == false) {
					switchedToIrisMode = true;
					char logmsg[] = "Switching to iris mode";
					LogSessionEvent(logmsg);
				}
			}
#endif

			MoveRelAngle(-1 * err);
			last_angle_move = -1 * err;
			//Flash the streaming
			vs->flush();
		}

	} else {
		EyelockLog(logger, DEBUG, "Face out of range\n");
	}

}

void FaceTracker::faceModeState(bool bDebugSessions)
{
	EyelockLog(logger, TRACE, "Entering > faceModeState");
	MoveTo(CENTER_POS);
	//MoveToAbs(CENTER_POS_TEST);

	run_state = RUN_STATE_FACE;
	SetFaceMode();
#ifdef DEBUG_SESSION
	if (bDebugSessions) {
		switchedToIrisMode = false;
		char logmsg[] = "Switched_to_face_mode";
		LogSessionEvent(logmsg);
	}
#endif /* DEBUG_SESSION */

}

int FaceTracker::SelectWhichIrisCam(float eye_size, int cur_state)
{
	EyelockLog(logger, TRACE, "Entering > SelectWhichIrisCam");

	if ((cur_state != STATE_MAIN_IRIS) && (cur_state != STATE_AUX_IRIS)) {
		// this is we are just getting into irises so hard decision not hysteresis
		if (eye_size >= (switchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;

	}
	if (cur_state == STATE_MAIN_IRIS)
		if (eye_size < (switchThreshold - errSwitchThreshold))
			return STATE_AUX_IRIS;
		else
			return STATE_MAIN_IRIS;

	if (cur_state == STATE_AUX_IRIS)
		if (eye_size >= (switchThreshold + errSwitchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;
}

char* FaceTracker::StateText(int state)
{
	EyelockLog(logger, TRACE, "Entering > StateText");
	switch (state)
	{
	    case STATE_LOOK_FOR_FACE: return("FACE");
		case STATE_MAIN_IRIS: return ("MAIN");
		case STATE_AUX_IRIS:  return ("AUX");
		case STATE_MOVE_MOTOR:return ("MOVE");
	}
	return "none";
}


void FaceTracker::SweepFaceBrightness(void)
{
	EyelockLog(logger, TRACE, "Entering > SweepFaceBrightness");
	// Sweep over the range step by step...
	if (m_nCalculatedBrightness < FACE_GAIN_MIN)
		m_nCalculatedBrightness = FACE_GAIN_MIN;

	m_nCalculatedBrightness += FACE_GAIN_STEP;

	if (m_nCalculatedBrightness > FACE_GAIN_MAX)
		m_nCalculatedBrightness = FACE_GAIN_MIN;

	// printf("SweepFaceBrightnessm nCurrentBrightnessLevel.%d\n", m_nCalculatedBrightness);

	agc_val = m_nCalculatedBrightness;

	SetFaceExposure(m_nCalculatedBrightness);
}



void FaceTracker::DoAgc(void)
{
	EyelockLog(logger, TRACE, "Entering > DoAgc");

	if(!foundFace)
		p = CalcExposureLevel(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);
	else
		p = CalcExposureLevel(m_CroppedFaceImageForAGC.cols, m_CroppedFaceImageForAGC.rows, (unsigned char *) (m_CroppedFaceImageForAGC.data),180);


	//if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
	agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	//if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
	//	agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	agc_val = MAX(FACE_GAIN_MIN,agc_val);
	agc_val = MIN(agc_val,FACE_GAIN_MAX);

	//printf("Outside condition >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
	AGC_Counter++;
	// AgcCntr++;
	if (agc_set_gain != agc_val) 	;	// && AGC_Counter%2==0)
	{
		//	while (waitKey(10) != 'z');
		{
			static int agc_val_old = 0;
			if (abs(agc_val - agc_val_old) > 300) {
				// printf("[AGC]  %05.6f Agc value = %d\n",p,agc_val);
				SetFaceExposure(agc_val);		//comment out if O2 led is connected
				agc_val_old = agc_val;
			}
		}

		agc_set_gain = agc_val;
	}
}


void FaceTracker::DoRunMode_test(bool bShowFaceTracking, bool bDebugSessions){
	EyelockLog(logger, TRACE, "DoRunMode_test");

	cv::Rect face;
	int start_process_time = clock();

	unsigned char FaceCameraFrameNo = (int)outImg.at<uchar>(0,3);

//	printf("FaceTracking: FrameNo = %d\n", FaceCameraFrameNo);

	unsigned int FaceFrameIndex=0;
	static unsigned int FaceCtrIndex=0;

	if(FaceCtrIndex != 0)
		FaceFrameIndex = (255 * FaceCtrIndex) + FaceCameraFrameNo;
	else
		FaceFrameIndex = FaceCameraFrameNo;

	if(FaceCameraFrameNo == 255){
		FaceCtrIndex++;
	}


	// printf("FaceCameraFrameNo%d\n", FaceCameraFrameNo);

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);

	smallImg = preProcessingImg(outImg);

	foundFace = FindEyeLocation(smallImg, eyes, eye_size, face, MIN_FACE_SIZE, MAX_FACE_SIZE);

	if(foundFace){
		//  EyelockLog(logger, INFO, "FaceTracking found Face");
		cv::Rect myFaceROI(face.x, face.y, face.width, face.height);
		// EyelockLog(logger, TRACE, "Before cropped image AGC");
		m_CroppedFaceImageForAGC = smallImg(myFaceROI);
		// EyelockLog(logger, TRACE, "After cropped image AGC");
	}

	float process_time = (float) (clock() - start_process_time) / CLOCKS_PER_SEC;


	bool eyesInDetect = foundFace? detect_area.contains(eyes):false;		// Face in face camera field of view
	bool eyesInViewOfIriscam = eyesInDetect ? search_eye_area.contains(eyes):false;		// Face/eyes in narrow Rect calculated from calibration
	bool eyesInViewOfIriscamNoMove = eyesInDetect ? no_move_area.contains(eyes):false;	//Face/eyes in Bigger Rect calculated from calibration

	if (foundFace==false){
		noFaceCounter++;
		m_ProjPtr = false;	//projPtr is only false if FindEyeLocation() doesn't find any face
	}
	noFaceCounter = min(noFaceCounter,NO_FACE_SWITCH_FACE);

	if (foundFace)
		noFaceCounter=0;
	last_system_state = system_state;


	// EyelockLog(logger, INFO, "FaceTracking SystemState = %d", system_state);
	// figure out our next state
	switch(system_state)
	{
	case STATE_LOOK_FOR_FACE:
							// we see eyes but need to move to them
							if (eyesInDetect && !eyesInViewOfIriscam)
								{
								system_state = STATE_MOVE_MOTOR;		//Setting up this state cause one extra move during face tracking
								moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
								break;
								}
							//if (eyesInDetect && eyesInViewOfIriscam)	//Very strict condition and slowing down the performance
							//if (eyesInViewOfIriscamNoMove)			//Relax the the previous condition but still slowing down the performance
							if (eyesInDetect)	//No strict condition but there will be no face images in pipeline
									system_state = SelectWhichIrisCam(eye_size,system_state);
							if(b_EnableFaceAGC)
								SweepFaceBrightness();
							//if (eyesInViewOfIriscam)
							break;

	case STATE_MAIN_IRIS:
						system_state = SelectWhichIrisCam(eye_size,system_state);
						if (noFaceCounter >= NO_FACE_SWITCH_FACE)
							{
							system_state=STATE_LOOK_FOR_FACE;
							break;
							}
						if (eyesInDetect &&  !eyesInViewOfIriscam){
							moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
							system_state = STATE_MOVE_MOTOR;	//Setting up this state cause one extra move during face tracking
						}
						break;
	case STATE_AUX_IRIS:
						system_state = SelectWhichIrisCam(eye_size,system_state);
						if (noFaceCounter >= NO_FACE_SWITCH_FACE)
							{
							system_state=STATE_LOOK_FOR_FACE;
							break;
							}
						if (eyesInDetect &&  !eyesInViewOfIriscam){
							moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
							system_state = STATE_MOVE_MOTOR;	//Setting up this state cause one extra move during face tracking
						}
						break;
	case STATE_MOVE_MOTOR:
						//if (eyesInDetect && eyesInViewOfIriscam)	//Very strict condition and slowing down the performance
						//if (eyesInViewOfIriscamNoMove)			//Relax the the previous condition but still slowing down the performance
						if (eyesInDetect)	//No strict condition but there will be no face images in pipeline
							{
							system_state = SelectWhichIrisCam(eye_size,system_state);
							break;
							}
						if (!foundFace)
							{
							system_state = STATE_LOOK_FOR_FACE;
							break;
							}
						if(b_EnableFaceAGC)
							DoAgc();
						moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
	}


	currnet_mode = -1;		//This variable is needed for setFaceMode()
	// handle switching state
	//if (last_system_state != system_state)
	// if(foundEyes)

		if (g_MatchState)
			g_MatchState=0;
		last_angle_move=0;

	int stateofIrisCameras = 0;

	if (last_system_state != system_state)
	switch(last_system_state)
	{
	case STATE_LOOK_FOR_FACE:
					switch (system_state)
					{
					case STATE_MOVE_MOTOR:
						// Activate the following command line if system_state = STATE_MOVE_MOTOR is used in previous switch case
						//moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
						// flush after moving to get more accurate motion on next loop
						vs->flush();
						m_ProjPtr = false;
						break;
					case STATE_MAIN_IRIS:
						// enable main camera and set led currnet
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings(eye_size, STATE_MAIN_IRIS);											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						stateofIrisCameras = STATE_MAIN_IRIS;
						m_ProjPtr = true;		//Added by Mo
						break;
					case STATE_AUX_IRIS:
						// enable aux camera and set led currnet
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings(eye_size, STATE_AUX_IRIS);											//change to Iris settings
						SwitchIrisCameras(false);									//switch cameras
						stateofIrisCameras = STATE_AUX_IRIS;
						m_ProjPtr = true;		//Added by Mo
						break;
					}
				//DMOOUT	if(b_EnableFaceAGC)
				//DMOOUT		DoAgc();
					break;
	case STATE_AUX_IRIS:
					switch (system_state)
					{
					case STATE_MOVE_MOTOR: // cannot happen
						// Activate the following command line if system_state = STATE_MOVE_MOTOR is used in previous switch case
						//moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
						m_ProjPtr = false;
						break;
					case STATE_LOOK_FOR_FACE:
						// disable iris camera set current for face camera
						MoveTo(CENTER_POS);
						SetFaceMode();
						m_ProjPtr = false;
						break;
					case STATE_MAIN_IRIS:
						//if the switch happen from AUX to MAIN then we
						//dont need to dim down the face cam settings because it is already
						//dimmed down
						//DimmFaceForIris();											//Dim face settings
						MainIrisSettings(eye_size, STATE_MAIN_IRIS);											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						stateofIrisCameras = STATE_MAIN_IRIS;
						m_ProjPtr = true;		//Added by Mo
						break;
					}
					break;
	case STATE_MAIN_IRIS:
						switch (system_state)
						{
						case STATE_MOVE_MOTOR:
							// Activate the following command line if system_state = STATE_MOVE_MOTOR is used in previous switch case
							//moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
							m_ProjPtr = false;
							break;
						case STATE_LOOK_FOR_FACE:
							// disable iris camera set current for face camera
							MoveTo(CENTER_POS);
							SetFaceMode();
							m_ProjPtr = false;
							break;
						case STATE_AUX_IRIS:
							//if the switch happen from AUX to MAIN then we
							//dont need to dim down the face cam settings because it is already
							//dimmed down
							//DimmFaceForIris();
							MainIrisSettings(eye_size, STATE_AUX_IRIS);											//change to Iris settings
							SwitchIrisCameras(false);									//switch cameras
							stateofIrisCameras = STATE_AUX_IRIS;
							m_ProjPtr = true;		//Added by Mo
							break;
						}
						break;
	case STATE_MOVE_MOTOR:
					switch (system_state)
					{
					case STATE_LOOK_FOR_FACE:
						// disable iris camera set current for face camera
						MoveTo(CENTER_POS);
						SetFaceMode();
						m_ProjPtr = false;
						break;
					case STATE_AUX_IRIS:
						// switch only the expusure and camera enables
						// no need to change voltage or current
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings(eye_size, STATE_AUX_IRIS);											//change to Iris settings
						SwitchIrisCameras(false);									//switch cameras
						stateofIrisCameras = STATE_AUX_IRIS;
						m_ProjPtr = true;		//Added by Mo
						break;
					case STATE_MAIN_IRIS:
						// enable main cameras and set
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings(eye_size, STATE_MAIN_IRIS);											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						stateofIrisCameras = STATE_MAIN_IRIS;
						m_ProjPtr = true;		//Added by Mo
						break;
					}
					break;
	}

	// EyelockLog(logger, INFO, "End of last_system_state != system_state = %d", system_state);

/*	if(system_state  == STATE_MOVE_MOTOR  || last_system_state  == STATE_MOVE_MOTOR){
		m_ProjPtr = false;
	}*/

	/*no_move_area.x = rectX/scaling;
	no_move_area.y = rectY/scaling + targetOffset;
	no_move_area.width = rectW/scaling;
	no_move_area.height = (rectH)/scaling -targetOffset*2;*/

	face = adjustWidthDuringFaceDetection(face);
	if(bFaceMapDebug)
		EyelockLog(logger, DEBUG, "in DoRunMode_test face.x	%d face.y	%d face.height	%d face.width	%d\n", face.x, face.y, face.height, face.width);

	
	//Scaling out projFace coordinates
	cv::Rect FaceCoord;
	FaceCoord.x = face.x * scaling;		//column
	FaceCoord.y = face.y * scaling; // rectY - (targetOffset * scaling);
	FaceCoord.width = face.width * scaling;
	FaceCoord.height = face.height *scaling; // rectH + (targetOffset * scaling);

	cv::Mat RotatedfaceImg, saveRotatedImg;

	cv::Rect rightRect, leftRect;
	//Seperate Right eye Rect
	rightRect.x = FaceCoord.x; rightRect.y = FaceCoord.y;
	rightRect.height = FaceCoord.height; rightRect.width = FaceCoord.width/2.0;

	//Seperate left eye Rect
	leftRect.x = FaceCoord.x + (FaceCoord.width/2); leftRect.y = FaceCoord.y;
	leftRect.height = FaceCoord.height; leftRect.width = FaceCoord.width/2.0;

	int ImageSize = m_ImageWidth * m_ImageHeight;

	if(bFaceMapDebug){
		RotatedfaceImg = rotation90(outImg);
		RotatedfaceImg.copyTo(saveRotatedImg);
		//cv::rectangle(RotatedfaceImg, projFace, Scalar(255,0,0),1,0);
		cv::rectangle(RotatedfaceImg, rightRect, Scalar(255,0,0),1,0);
		cv::rectangle(RotatedfaceImg, leftRect, Scalar(255,0,0),1,0);
		imshow("RescaledFaceImage", RotatedfaceImg);
		cvWaitKey(1);
	}

	// printf("eye_size.......%d\n", eye_size);

	// printf("PushToQueue foundEyes %d FaceFrameNo %d face x = %d  face y = %d face width = %d  face height = %d \n", foundEyes, FaceCameraQFrameNo, face.x,  face.y,  face.width, face.height);
	 if (true) {//system_state == STATE_MAIN_IRIS || system_state == STATE_AUX_IRIS){ // Removed for odriod by Anita
//		if(m_ProjPtr && eyesInViewOfIriscamNoMove){ // Removed by sarvesh

		 	m_LeftCameraFaceInfo.FoundFace = foundFace;
			m_LeftCameraFaceInfo.ScaledFaceCoord = FaceCoord;
			m_LeftCameraFaceInfo.FaceFrameNo = FaceCameraFrameNo;
			m_LeftCameraFaceInfo.projPtr = m_ProjPtr;
			m_LeftCameraFaceInfo.FaceWidth = eye_size;

			m_RightCameraFaceInfo.FoundFace = foundFace;
			m_RightCameraFaceInfo.ScaledFaceCoord = FaceCoord;
			m_RightCameraFaceInfo.FaceFrameNo = FaceCameraFrameNo;
			m_RightCameraFaceInfo.projPtr = m_ProjPtr;
			m_LeftCameraFaceInfo.FaceWidth = eye_size;

			if(bIrisToFaceMapDebug){
				RotatedfaceImg = rotation90(outImg);
				memcpy(m_LeftCameraFaceInfo.faceImagePtr, RotatedfaceImg.data, ImageSize);
				memcpy(m_RightCameraFaceInfo.faceImagePtr, RotatedfaceImg.data, ImageSize);
			}

			EyelockLog(logger, TRACE, "FaceTracking:  Pushing FaceFrame = %d\n", FaceCameraFrameNo);
			// If we're full, the top is stale anyway, get rid of it, then add the current item
			// Increase the size of the queues if we are dropping too many unprocessed FaceFrames here...
   		    if (g_pLeftCameraFaceQueue->Full())
   		    	g_pLeftCameraFaceQueue->Pop();
			g_pLeftCameraFaceQueue->Push(m_LeftCameraFaceInfo);
			EyelockLog(logger, TRACE, "FaceTracking:  Pushed LeftQueue, Size() = %d\n", g_pLeftCameraFaceQueue->Size());
			// If we're full, the top is stale anyway, get rid of it, then add the current item
   		    if (g_pRightCameraFaceQueue->Full())
   		    	g_pRightCameraFaceQueue->Pop();
			g_pRightCameraFaceQueue->Push(m_RightCameraFaceInfo);
   		    EyelockLog(logger, TRACE, "FaceTracking:  Pushed Pushed RightQueue, Size() = %d\n", g_pRightCameraFaceQueue->Size());

			if(bFaceMapDebug){
				char filename[100];
				sprintf(filename,"FaceImage_%d_ProjPtr_%d.pgm", FaceCameraFrameNo, m_ProjPtr);
				cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
				cv::rectangle(smallImg, search_eye_area, Scalar(255, 0, 0), 1, 0);
				cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
				cv::rectangle(smallImg, face, Scalar(255,0,0),1,0);
				// imwrite(filename, smallImg);
				imwrite(filename, RotatedfaceImg);
			}
		}
	// }
	 RotatedfaceImg.release();
	 saveRotatedImg.release();
		// printf("DoRunMode_test face.x %d face.y %d face.width %d face.height %d\n",  face.x,face.y,face.width,face.height);


			EyelockLog(logger, TRACE, "FaceFrameNo:%d STATE:%8s LAST_STATE:%8s NFC:%2d %c%c%c  I_SIZE:%03.1f  I_POS(%3d,%3d) Proj:%d MV:%3.3f TIME:%3.3f AGC:%5d MS:%d \n",FaceCameraFrameNo, StateText(system_state),
					StateText(last_system_state),
							noFaceCounter,
							foundFace?'E':'.',
						eyesInDetect?'D':'.',
						eyesInViewOfIriscam?'V':'.',
								eye_size,
								eyes.x,
								eyes.y,
								m_ProjPtr,
								last_angle_move,
								process_time,
								agc_set_gain,
								g_MatchState
								);



		if(bShowFaceTracking){
			//printf("face x = %i  face y = %i face width = %i  face height = %i\n",face.x, face.y, face.width, face.height);
			EyelockLog(logger, TRACE, "Imshow");
			cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, search_eye_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, face, Scalar(255,0,0),1,0);
			imshow("FaceTracker", smallImg);
			cvWaitKey(1);
		}



//For debug session
#ifdef DEBUG_SESSION
	if (bDebugSessions)
	{
		if (foundFace)
		{
			struct stat st = {0};
			if (stat(m_sessionDir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
				time_t timer;
				struct tm* tm1;
				time(&timer);
				tm1 = localtime(&timer);
				char time_str[100];
				strftime(time_str, 100, "%Y_%m_%d_%H-%M-%S", tm1);

				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);

				int FrameNo = vs->frameId;
				int CamId = vs->cam_id;
				char filename[200];
				sprintf(filename, "%s/FaceImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, FaceCameraFrameNo, CamId);

				imwrite(filename, smallImg);

				// To save full face Images
				char filename1[200];
				sprintf(filename1, "%s/FaceFullImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, FaceCameraFrameNo, CamId);
				imwrite(filename1, rotation90(outImg));

				char logmsg[300];
				sprintf(logmsg, "Saved-FaceImage-FrNum%d-CamID%d-%s", FaceCameraFrameNo, CamId, filename);
				LogSessionEvent(logmsg);
			}
		}
	}
#endif

// Push face Coordinates to Queue
	// pthread_create(&threadIdFace,NULL,face_queue,faceInfo);

}

//DMO
void *init_facetracking(void *arg) {

	// printf("Inside init_facetracking\n");
	FaceTracker m_faceTracker("/home/root/data/calibration/Face.ini");

	EyelockLog(logger, TRACE, "init_facetracking Start FaceTracking Thread");

	// Flag to enable/disable AES Encrption in port com
	bool bDoAESEncryption = m_faceTracker.FaceConfig.getValue("FTracker.AESEncrypt", false);

	bool bDebugFrameBuffer = m_faceTracker.FaceConfig.getValue("FTracker.DebugFrameBuffer", false);

	// printf("bDoAESEncryption  %d bDebugFrameBuffer................%d\n", bDoAESEncryption, bDebugFrameBuffer);

	int w, h;

	pthread_t threadId;
	pthread_t thredEcId;

	// Intialize Portcom for device control
	portcom_start(bDoAESEncryption);

	//pThread for face tracker active
	// Run Mode
	// Create Tunnel Thread
	// Create Eyelock_Com thread

	//vid_stream_start
	vs = new VideoStream(8194, m_faceTracker.m_ImageAuthentication); //Facecam is 8194...

	pthread_create(&threadId, NULL, init_tunnel, NULL);
	pthread_setname_np(threadId, "init_tunnel");

	EyelockLog(logger, TRACE, "Start Tunnel Thread");

	// Allocate our ec messaging queue, then start the thread...
	pthread_create(&thredEcId, NULL, init_ec, vs);
	pthread_setname_np(thredEcId, "init_ec");

	EyelockLog(logger, TRACE, "Start  Eyelock Com Thread");

	//Setting up Run mode
	EyelockLog(logger, DEBUG, "run_mode");

	// Initialize Face Detection
	face_init();

	// Load FaceTracking config... Configure all hardware
	m_faceTracker.DoStartCmd();

	// Main Loop...
	outImg = Mat(Size(m_faceTracker.m_ImageWidth, m_faceTracker.m_ImageHeight), CV_8U);

	// static unsigned int count = 0;
	while (1)
	{
		// printf("Inside while of face tracking\n");

		clock_t begin = clock();

		bool status = vs->get(&w, &h, (char *) outImg.data, bDebugFrameBuffer);
		/*
		if(count % 4*5 == 0){ // Every 5 seconds
			cv::imwrite("FaceImage.pgm",outImg);
		}*/

		clock_t end = clock();

		if(status == false){
			// printf("No Face Image for 15 seconds....%d\n", status);
			EyelockLog(logger, ERROR, "No Face Image for 15 seconds");
			system("/home/root/forcereboot.sh &");
		}else{
			double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			EyelockLog(logger, TRACE, "time_spent in receiving a frame..%ld\n", time_spent);
		}

		//Main Face tracking operation
		m_faceTracker.DoRunMode_test(m_faceTracker.bShowFaceTracking, m_faceTracker.bDebugSessions);
		// count++;
	}
	outImg.release();
}












