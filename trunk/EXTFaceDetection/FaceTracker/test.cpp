#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo/photo.hpp>
#include "portcom.h"

#define CAMERACALIBERATION_ARUCO


#ifdef CAMERACALIBERATION_ARUCO
#include <aruco.h>
#include <dictionary.h>
#endif

#include "eyelock_com.h"
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#if 0
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#endif
#include "log.h"

#include "eyelock_com.h"
#include "extFocus.h"

using namespace cv;
using namespace std::chrono;
using namespace std;

#define DEBUG_SESSION
#ifdef DEBUG_SESSION
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#define DEBUG_SESSION_DIR "DebugSessions/Session"
//#define DEBUG_SESSION_INFO "DebugSessions/Session/Info.txt"
FileConfiguration eyelockConf("");
std::string m_sessionDir;
std::string m_sessionInfo;
bool switchedToIrisMode = false;
void *DoTamper(void *arg);
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

//WIDTH and HEIGHT of input image
#define WIDTH 1200
#define HEIGHT 960

#define CENTER_POSITION_ANGLE 95

#define SCALE 3

#define MODE_CHANGE_FREEZE 10

#define MIN_IRIS_FRAMES 10
#define FRAME_DELAY 100

// this defines how many frames without a face will cause a switch back to face mode ie look for faces
#define NO_FACE_SWITCH_FACE 10

#define ANGLE_TO_STEPS 5

#define smallMoveTo 2		//limiting motor movement


//Controlling states between face and Eyes
#define RUN_STATE_FACE 0
#define RUN_STATE_EYES 1

//USed to switch cameras and keep it running while changes dont need
#define MODE_FACE 0
#define MODE_EYES_MAIN 1
#define MODE_EYES_AUX 2



//USed in DoImageCal function
#define NUM_AVG_CAL 10
#define  MAX_CAL_CURENT 20


int currnet_mode = 0;
int previousEye_distance = 0;

const char logger[30] = "test";
const char stateMachineLogger[30] = "StateMachine";


VideoStream *vs;
VideoStream *vsExp1, *vsExp2;
Mat outImgI1,outImgI2;
Mat imgIS1, imgIS2;

Mat outImg, smallImg, smallImgBeforeRotate;

double scaling = 8.0;		//Used for resizing the images in py lev 3


//Reading from faceConfig.ini
int CENTER_POS;
int MIN_POS;
int MAX_POS;


//Switching threshold and Hysteresis
int switchThreshold;		// 37
int errSwitchThreshold;		//6


// detect_area used for finding face in a certain rect area of entire image
int rectX, rectY, rectW, rectH;
float magOffMainl, magOffMainR, magOffAuxl, magOffAuxR;
cv::Point2f constantMainl, constantMainR, constantAuxl, constantAuxR;
int targetOffset;
Rect detect_area(15/SCALE,15/SCALE,(960/(SCALE*SCALE))+15/SCALE,(1200/(SCALE*SCALE))+15/SCALE); //15 was 30
Rect no_move_area, no_move_areaX;		//Face target area
Rect projFace;


int IrisFrameCtr = 0;		//used for counting Iris Frame

int cur_pos=CENTER_POS;

int fileNum=0;
int move_counts=0;


string fileName = "output.csv";			//Save temp data
string a_calibFile = "/home/root/data/calibration/auxRect.csv";		//Read target rect
int noeyesframe = 0;


//AGC parameters
int PIXEL_TOTAL;
int FACE_GAIN_DEFAULT;
int FACE_GAIN_MAX;
int FACE_GAIN_MIN;
int FACE_GAIN_PER_GOAL;
float FACE_GAIN_HIST_GOAL;
float FACE_CONTROL_GAIN;
//#define FACE_CONTROL_GAIN   1000.0

int agc_val= FACE_GAIN_DEFAULT;
int agc_set_gain =0;

float ERROR_LOOP_GAIN;	// used for error cal during face tracking and motor movement

int run_state=RUN_STATE_FACE;

//Face size tracking from 90 to 30cm range if the images are bright enough
int MIN_FACE_SIZE;
int MAX_FACE_SIZE;

Point eyes;	// hold eye info from each frame
char temp[512], tempI1[512], tempI2[512], tempI3[512], tempI4[512], tempI5[512];
int AGC_Counter = 0;
int noFaceCounter =0;

Mat outImgLast, outImg1, outImg1s;		//Used in MeasureSnr function

//Used as increasing exposure time in brightnessAdjust function,
//mainly used in Camera to Camera Calibration
int agc_val_cal=3;
int step;
int startPoint;
int thresholdVal = 30;
bool calDebug, calTwoPoint, projDebug, projPtr = false;;
float projOffset_m, projOffset_a;
bool useOffest_m, useOffest_a;
bool showProjection;

//variable used for tempering
//CvFont font;


struct coordinateProject{
	int camIDL;
	int frmaeL;
	Rect camLRect;
	int camIDR;
	int frmaeR;
	Rect camRRect;
};

std::chrono:: time_point<std::chrono::system_clock> start_mode_change;



extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern void  *init_tunnel(void *arg);
extern void *init_ec(void * arg);
extern int IrisFramesHaveEyes();
int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size, Rect &face);

int face_init(  );
float read_angle(void);
void SetExp(int cam, int val);
void MoveToAngle(float a);
void MoveTo(int v);
void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
void SelLedDistance(int val);
char* GetTimeStamp();
void RecoverModeDrop();



void MainIrisSettings();
void SwitchIrisCameras(bool mode);
void SetFaceMode();

void MoveRelAngle(float a);
void SetIrisMode(float CurrentEye_distance);
void DoStartCmd();
void DoStartCmd_CamCal();
float AGC(int width, int height,unsigned char *dsty, int limit);
Mat rotate(Mat src, double angle);
Mat rotation90(Mat src);
int IrisFramesHaveEyes();
void DoRunMode(bool bShowFaceTracking);
void DoRunMode_test(bool bShowFaceTracking, bool bDebugSessions);

void MeasureSnr();	//Measuring noise in images

//image optimization
void DoImageCal(int cam_id_ignore);
void CalAll();

#ifdef CAMERACALIBERATION_ARUCO
//Camera to Camera Calibration
float projectPoints(float y, float c, float m);
std::vector<aruco::Marker> gridBooardMarker(Mat img, int cam, bool calDebug);
vector<float> calibratedRect(std::vector<aruco::Marker> markerIris, std::vector<aruco::Marker> markerFace);
void brightnessAdjust(Mat outImg, int cam, bool calDebug);
bool CalCam(bool calDebug);
void runCalCam(bool calDebug);
#endif

// Temperature test purpose
void motorMove();
double parsingIntfromHex(string str1);
int calTemp(int i);
float AGC_average(int width, int height,unsigned char *dsty, int limit);

//Temparing
double StandardDeviation(std::vector<double> samples);
double Variance(std::vector<double> samples);


extFocus *fs;


void SetExp(int cam, int val)
{
	EyelockLog(logger, TRACE, "SetExp");
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	int fine = val - coarse*PIXEL_TOTAL;

	//sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	EyelockLog(logger, DEBUG, "Setting Gain %d\n",coarse);
	//port_com_send(buff);
}






void MoveToAngle(float a)
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



void MoveTo(int v)
{
	EyelockLog(logger, TRACE, "MoveTo");
	EyelockLog(logger, DEBUG,"Move to command %d ",v);

	v=v-CENTER_POS;
	v=v/ANGLE_TO_STEPS+CENTER_POSITION_ANGLE;

	EyelockLog(logger, DEBUG,"Value to MoveToAngle:angle = %d",v);

	MoveToAngle((float) v);
}



void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask)
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



void SelLedDistance(int val) // val between 0 and 100
{
	EyelockLog(logger, TRACE, "SelLedDistance");
	int set_val;
	set_val = MIN(val,33);
	setRGBled(set_val*0,set_val*0,set_val*3,10,0,0x11);
	val = val - set_val;

	set_val = MIN(val,33);
	setRGBled(set_val*0,set_val*0,set_val*3,10,0,0xa);
	val = val - set_val;

	if (val > 33)
	{
		setRGBled(set_val*3,0,0,10,0,0x4);
	}
	else
	{
		set_val = MIN(val,3);
		setRGBled(set_val*0,set_val*0,set_val*3,10,0,0x4);
	}
	EyelockLog(logger, DEBUG, "Value for LedDistance: set_val %d", set_val);
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



void RecoverModeDrop()
{
	EyelockLog(logger, TRACE, "RecoverModeDrop");
	//If no mode change happens for a set amount of time then set face mode to recover from
	//from possible condition of losing the mode change message

	static std::chrono:: time_point<std::chrono::system_clock> end;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start_mode_change;
	//printf("Elapsed Time %f\n",elapsed_seconds);
	if(elapsed_seconds.count()>=MODE_CHANGE_FREEZE)
	{
		run_state = RUN_STATE_FACE;
		agc_val=FACE_GAIN_DEFAULT;
		//SetFaceMode();

	}
}




void MainIrisSettings()
{
	EyelockLog(logger, TRACE, "MainIrisSettings");
	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");
	char cmd[512];

	int IrisLEDVolt = fconfig.getValue("FTracker.IrisLEDVolt",30);
	int IrisLEDcurrentSet = fconfig.getValue("FTracker.IrisLEDcurrentSet",40);
	int IrisLEDtrigger = fconfig.getValue("FTracker.IrisLEDtrigger",3);
	int IrisLEDEnable = fconfig.getValue("FTracker.IrisLEDEnable",49);
	int IrisLEDmaxTime = fconfig.getValue("FTracker.IrisLEDmaxTime",4);

	//return;
	EyelockLog(logger, DEBUG, "configuring Main Iris settings");
	// printf("configuring Main Iris settings\n");
	//Iris configuration of LED
	sprintf(cmd,"psoc_write(3,%i)| psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(6,%i)\n", IrisLEDEnable, IrisLEDVolt, IrisLEDcurrentSet, IrisLEDtrigger, IrisLEDmaxTime);
	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("psoc_write(3,0x31)| psoc_write(2,30) | psoc_write(5,40) | psoc_write(4,3) | psoc_write(6,4)");
	//Face cameras configuration
	//port_com_send("wcr(0x83,0x3012,12) | wcr(0x83,0x301e,0) | wcr(0x83,0x305e,128)");

	EyelockLog(logger, DEBUG, "Values in MainIrisSettings IrisLEDEnable:%d, IrisLEDVolt:%d, IrisLEDcurrentSet:%d, IrisLEDtrigger:%d, IrisLEDmaxTime:%d ", IrisLEDEnable, IrisLEDVolt, IrisLEDcurrentSet, IrisLEDtrigger, IrisLEDmaxTime);
}




void SwitchIrisCameras(bool mode)
{
	EyelockLog(logger, TRACE, "SwitchIrisCameras");

	char cmd[100];
	if (mode)
		sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	else
		sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);

	port_com_send(cmd);

	printf("mode....%d\n", mode);

/*	int w,h;
	vsExp1 = new VideoStream(8192);
	vsExp1->flush();
	vsExp1->get(&w,&h,(char *)imgIS1.data);


	vsExp2 = new VideoStream(8193);
	vsExp2->flush();
	vsExp2->get(&w,&h,(char *)imgIS2.data);*/



}





void SetFaceMode()
{
	EyelockLog(logger, TRACE, "SetFaceMode");
	if (currnet_mode==MODE_FACE)
	{
		EyelockLog(logger, DEBUG, "Don't need to change Face camera");
		return;
	}

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	int minPos = fconfig.getValue("FTracker.minPos",0);
	CENTER_POS = fconfig.getValue("FTracker.centerPos",164);
	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);
	int faceLEDVolt = fconfig.getValue("FTracker.faceLEDVolt",30);
	int faceLEDcurrentSet = fconfig.getValue("FTracker.faceLEDcurrentSet",20);
	int faceLEDtrigger = fconfig.getValue("FTracker.faceLEDtrigger",4);
	int faceLEDEnable = fconfig.getValue("FTracker.faceLEDEnable",4);
	int faceLEDmaxTime = fconfig.getValue("FTracker.faceLEDmaxTime",4);
	int faceCamExposureTime = fconfig.getValue("FTracker.faceCamExposureTime",12);
	int faceCamDigitalGain = fconfig.getValue("FTracker.faceCamDigitalGain",240);
	int faceAnalogGain = fconfig.getValue("FTracker.faceAnalogGain",128);
	char cmd[512];
	EyelockLog(logger, DEBUG, "Setting Face Mode\n");
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)\n",faceLEDVolt, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable);
	EyelockLog(logger, DEBUG, "Face camera settings-faceLEDVolt:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d",faceLEDVolt, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable);
	//printf(cmd);
	port_com_send(cmd);
//	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
//	port_com_send("psoc_write(2,30) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)");
	sprintf(cmd, "wcr(4,0x3012,%i)  | wcr(4,0x305e,%i)", faceCamExposureTime, faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "Face camera exposure and gain settings -faceCamExposureTime:%d, faceCamDigitalGain:%d",faceCamExposureTime, faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(4,0x3012,7)  | wcr(4,0x305e,0xfe)");
	sprintf(cmd,"wcr(0x04,0x30b0,%i)",((faceAnalogGain&0x3)<<4) | 0X80);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Face camera Analog gain:%d",(((faceAnalogGain&0x3)<<4) | 0X80));
	agc_val= FACE_GAIN_DEFAULT;
	EyelockLog(logger, DEBUG, "Face camera gain default:%d", FACE_GAIN_DEFAULT);
	start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_FACE;
	//port_com_send("fixed_set_rgb(100,100,100)");

/*	delete(vsExp1);
	delete(vsExp2);*/


	sprintf(cmd,"set_cam_mode(0x04,%d)",FRAME_DELAY);
	port_com_send(cmd);

	if(projDebug){
		port_com_send("set_cam_mode(0x87,100)");
	}


}



void MoveRelAngle(float a)
{
	EyelockLog(logger, TRACE, "MoveRelAngle");
	// add a limit check to make sure we are not out of bounds
	char buff[100];
	float move;
	float current_a = read_angle();
	EyelockLog(logger, DEBUG, "Current_a=%f ; next_a=%f\n",current_a,a);

	move=-1*a*ANGLE_TO_STEPS;

	EyelockLog(logger, DEBUG, "limiting small movements based on relative changes and face size changes:diffEyedistance %f", move);
	//limiting small movements based on relative changes and face size changes
	if (abs(move) > smallMoveTo)	//&& diffEyedistance >= errSwitchThreshold
	{
		sprintf(buff,"fx_rel(%d)",(int)move);
		EyelockLog(logger, DEBUG, "Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
		port_com_send(buff);
	}
}


void DimmFaceForIris()
{
	char cmd[512];
	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");
	int DimmingfaceAnalogGain = fconfig.getValue("FTracker.DimmingfaceAnalogGain",0);
	int DimmingfaceExposureTime = fconfig.getValue("FTracker.DimmingfaceExposureTime",7);
	int DimmingfaceDigitalGain = fconfig.getValue("FTracker.DimmingfaceDigitalGain",32);

	// printf("Dimming face cameras!!!");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x305e,%i)", DimmingfaceExposureTime, DimmingfaceDigitalGain);
	EyelockLog(logger, DEBUG, "Dimming face cameras -DimmingfaceExposureTime:%d, DimmingfaceDigitalGain:%d",DimmingfaceExposureTime, DimmingfaceDigitalGain);
	port_com_send(cmd);

	//Need to change this anlog gain setting
	{
		char cmd[512];
		sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((DimmingfaceAnalogGain&0x3)<<4) | 0X80);
		EyelockLog(logger, DEBUG, "FACE_GAIN_MIN:%d DimmingfaceAnalogGain:%d", FACE_GAIN_MIN, (((DimmingfaceAnalogGain&0x3)<<4) | 0X80));
		port_com_send(cmd);
	}
	agc_val = FACE_GAIN_MIN;

}




//void SetIrisMode(int CurrentEye_distance, int diffEyedistance)
void SetIrisMode(float CurrentEye_distance)
{

	EyelockLog(logger, TRACE, "SetIrisMode");
	char cmd[512];

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	int DimmingfaceAnalogGain = fconfig.getValue("FTracker.DimmingfaceAnalogGain",0);
	int DimmingfaceExposureTime = fconfig.getValue("FTracker.DimmingfaceExposureTime",7);
	int DimmingfaceDigitalGain = fconfig.getValue("FTracker.DimmingfaceDigitalGain",32);

	// printf("Dimming face cameras!!!");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x305e,%i)", DimmingfaceExposureTime, DimmingfaceDigitalGain);
	EyelockLog(logger, DEBUG, "Dimming face cameras -DimmingfaceExposureTime:%d, DimmingfaceDigitalGain:%d",DimmingfaceExposureTime, DimmingfaceDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x305e,0x20)");
	{
		char cmd[512];
		sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((DimmingfaceAnalogGain&0x3)<<4) | 0X80);
		EyelockLog(logger, DEBUG, "FACE_GAIN_MIN:%d DimmingfaceAnalogGain:%d", FACE_GAIN_MIN, (((DimmingfaceAnalogGain&0x3)<<4) | 0X80));
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
	start_mode_change = std::chrono::system_clock::now();


}


int tempTarget; 		//5 mins = 60*5 = 300s


// Main DoStartCmd configuration for Eyelock matching
void DoStartCmd(){

	EyelockLog(logger, TRACE, "DoStartCmd");
	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

/*	double id = fconfig.getValue("FTracker.uintID",0);
	double idm;
	cout << "Input the device ID::::: ";
	cvWaitKey(1);
	cin >> idm;
	cout << "Device ID IS ::::::::::::::::::::::: " << idm << endl;

	//check device ID
	if(id == idm){
		printf("-------------->>>>>>>>>>>>>>>>>>> Device ID didn't match with calibration file!\n");
		exit(EXIT_FAILURE);
	}*/

	tempTarget = fconfig.getValue("FTracker.tempReadingTimeInMinutes",5);
	tempTarget = tempTarget * 60;	//converting into sec


	rectX = fconfig.getValue("FTracker.targetRectX",0);
	rectY = fconfig.getValue("FTracker.targetRectY",497);
	rectW = fconfig.getValue("FTracker.targetRectWidth",960);
	rectH = fconfig.getValue("FTracker.targetRectHeight",121);

	projDebug = fconfig.getValue("FTracker.faceIrisProjection",false);


	//magOffMainl, magOffMainR, magOffAuxl, magOffAuxR;

	magOffMainl = fconfig.getValue("FTracker.magOffsetMainLeftCam",float(0.15));
	magOffMainR = fconfig.getValue("FTracker.magOffsetMainRightCam",float(0.15));
	magOffAuxl = fconfig.getValue("FTracker.magOffsetAuxLeftCam",float(0.22));
	magOffAuxR = fconfig.getValue("FTracker.magOffsetAuxRightCam",float(0.22));
/*	printf("magOffMainl::: %3.3f, magOffMainR::: %3.3f, magOffAuxl::: %3.3f, magOffAuxR::: %3.3f\n"
			, magOffMainl, magOffMainR, magOffAuxl, magOffAuxR);*/

	constantMainl.x = fconfig.getValue("FTracker.constantMainLeftCam_x",float(0.15));
	constantMainl.y = fconfig.getValue("FTracker.constantMainLeftCam_y",float(0.15));
	constantMainR.x = fconfig.getValue("FTracker.constantMainRightCam_x",float(0.15));
	constantMainR.y = fconfig.getValue("FTracker.constantMainRightCam_y",float(0.15));
	constantAuxl.x = fconfig.getValue("FTracker.constantAuxLeftCam_x",float(0.22));
	constantAuxl.y = fconfig.getValue("FTracker.constantAuxLeftCam_y",float(0.22));
	constantAuxR.x = fconfig.getValue("FTracker.constantAuxRightCam_x",float(0.22));
	constantAuxR.y = fconfig.getValue("FTracker.constantAuxRightCam_y",float(0.22));

/*
	printf("constantMainl.x ::: %3.3f, constantMainl.y ::: %3.3f, constantMainR.x ::: %3.3f, constantMainR.y ::: %3.3f, constantAuxl.x ::: %3.3f, constantAuxl.y ::: %3.3f, constantAuxR.x ::: %3.3f, constantAuxR.y ::: %3.3f",
			constantMainl.x, constantMainl.y, constantMainR.x, constantMainR.y, constantAuxl.x, constantAuxl.y, constantAuxR.x, constantAuxR.y);
*/

/*	float projOffset_m = 100.00, projOffset_a = 200.00;
	bool useOffest_m= false, useOffest_a = true;*/


	useOffest_m = fconfig.getValue("FTracker.projectionOffsetMain",false);
	useOffest_a = fconfig.getValue("FTracker.projectionOffsetAux",true);
	projOffset_m = fconfig.getValue("FTracker.projectionOffsetValMain",float(50.00));
	projOffset_a = fconfig.getValue("FTracker.projectionOffsetValAux",float(200.00));

	//printf("useOffest_m ::: %s, useOffest_a ::: %s, projOffset_m ::: %3.3f, projOffset_a ::: %3.3f\n", useOffest_m ? "true":"false", useOffest_a ? "true":"false",projOffset_m,projOffset_a);

	showProjection = fconfig.getValue("FTracker.showProjection",false);

	MIN_POS = fconfig.getValue("FTracker.minPos",0);

	startPoint = fconfig.getValue("FTracker.startPoint",100);
	MAX_POS = fconfig.getValue("FTracker.maxPos",350);
	CENTER_POS = fconfig.getValue("FTracker.centerPos",164);

	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);

	int faceLEDVolt = fconfig.getValue("FTracker.faceLEDVolt",30);
	int faceLEDcurrentSet = fconfig.getValue("FTracker.faceLEDcurrentSet",20);
	int faceLEDtrigger = fconfig.getValue("FTracker.faceLEDtrigger",4);
	int faceLEDEnable = fconfig.getValue("FTracker.faceLEDEnable",4);
	int faceLEDmaxTime = fconfig.getValue("FTracker.faceLEDmaxTime",4);

	//Reading AGC parameters
	PIXEL_TOTAL = fconfig.getValue("FTracker.PIXEL_TOTAL",900);
	FACE_GAIN_DEFAULT = fconfig.getValue("FTracker.FACE_GAIN_DEFAULT",10);
	FACE_GAIN_DEFAULT = FACE_GAIN_DEFAULT * PIXEL_TOTAL;
	FACE_GAIN_MAX = fconfig.getValue("FTracker.FACE_GAIN_MAX",80);
	FACE_GAIN_MAX = FACE_GAIN_MAX * PIXEL_TOTAL;
	FACE_GAIN_MIN = fconfig.getValue("FTracker.FACE_GAIN_MIN",8);
	FACE_GAIN_MIN = FACE_GAIN_MIN * PIXEL_TOTAL;
	FACE_GAIN_PER_GOAL = fconfig.getValue("FTracker.FACE_GAIN_PER_GOAL",10);
	FACE_GAIN_HIST_GOAL = fconfig.getValue("FTracker.FACE_GAIN_HIST_GOAL",float(0.1));
	FACE_CONTROL_GAIN = fconfig.getValue("FTracker.FACE_CONTROL_GAIN",float(500.0));
	ERROR_LOOP_GAIN = fconfig.getValue("FTracker.ERROR_LOOP_GAIN",float(0.08));

	MIN_FACE_SIZE = fconfig.getValue("FTracker.MIN_FACE_SIZE",10);
	MAX_FACE_SIZE = fconfig.getValue("FTracker.MAX_FACE_SIZE",70);

	int faceCamExposureTime = fconfig.getValue("FTracker.faceCamExposureTime",12);
	int faceCamDigitalGain = fconfig.getValue("FTracker.faceCamDigitalGain",240);
	int faceCamDataPedestal = fconfig.getValue("FTracker.faceCamDataPedestal",0);

	int AuxIrisCamExposureTime = fconfig.getValue("FTracker.AuxIrisCamExposureTime",8);
	int AuxIrisCamDigitalGain = fconfig.getValue("FTracker.AuxIrisCamDigitalGain",80);
	int AuxIrisCamDataPedestal = fconfig.getValue("FTracker.AuxIrisCamDataPedestal",0);

	int MainIrisCamExposureTime = fconfig.getValue("FTracker.MainIrisCamExposureTime",8);
	int MainIrisCamDigitalGain = fconfig.getValue("FTracker.MainIrisCamDigitalGain",128);
	int MainIrisCamDataPedestal = fconfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	int irisAnalogGain = fconfig.getValue("FTracker.irisAnalogGain",144);
	int faceAnalogGain = fconfig.getValue("FTracker.faceAnalogGain",128);

	int capacitorChargeCurrent = fconfig.getValue("FTracker.capacitorChargeCurrent",60);


	switchThreshold = fconfig.getValue("FTracker.switchThreshold",37);
	errSwitchThreshold = fconfig.getValue("FTracker.errSwitchThreshold",2);

	char cmd[512];


	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);


	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	printf("Re Homing\n");

	EyelockLog(logger, DEBUG, "port_com_send fx_home command is issued");
#ifdef NOOPTIMIZE
	usleep(100000);
#endif

	//Reset the lower motion
	sprintf(cmd, "fx_abs(%i)",MIN_POS);
	EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", MIN_POS);
	port_com_send(cmd);

	//move to center position
	EyelockLog(logger, DEBUG, "Moving to center position");
	MoveTo(CENTER_POS);
	//printf("enter pos val %d\n", CENTER_POS);
	read_angle();		//read current angle


	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n",faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);

	EyelockLog(logger, DEBUG, "Configuring face LEDs faceLEDVolt:%d, allLEDhighVoltEnable:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d, faceLEDmaxTime:%d", faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);
	port_com_send(cmd);
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");


	//port_com_send("psoc_write(9,90)");	// charge cap for max current 60 < range < 95
	sprintf(cmd, "psoc_write(9,%i)\n", capacitorChargeCurrent);
	EyelockLog(logger, DEBUG, "capacitorChargeCurrent:%d", capacitorChargeCurrent);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95


	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xFE)");
	//port_com_send("wcr(0x04,0x3012,12) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xF0)");	//Demo Config



	//AUX cameras configuration
	EyelockLog(logger, DEBUG, "Configuring AUX Iris Cameras");
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x18,0x3012,8) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,0x80)");
	//port_com_send("wcr(0x18,0x3012,8) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,80)"); //DEMO Config


	//Main Iris Cameras Configuration
	EyelockLog(logger, DEBUG, "Configuring Main Iris Cameras");
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", faceCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x03,0x3012,8) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,0xB0)");
	//port_com_send("wcr(0x03,0x3012,8) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,128)");	//Demo Config


	EyelockLog(logger, DEBUG, "Setting up PLL");

	//following process will activate PLL for all cameras
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((irisAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Iris analog gain: %d", (((irisAnalogGain&0x3)<<4) | 0X80));

	port_com_send(cmd);
	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((faceAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Face analog gain: %d", (((faceAnalogGain&0x3)<<4) | 0X80));
	port_com_send(cmd);
	//port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	//port_com_send("wcr(0x4,0x30b0,0x90");		//Only face camera gain is x90


	//Temp Change
	//MainIrisSettings();
///////////////////ilya///////////////////
	//This code is for playing sound
	if(1)
	{
		EyelockLog(logger, DEBUG, "playing audio -set_audio(1)");
		port_com_send("set_audio(1)");
		usleep(100000);
		port_com_send("data_store_set(0)");
		usleep(100000);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
		sleep(1);
		port_com_send("data_store_set(1)");
		usleep(100000);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/rej.raw");
		sleep(1);
	}



/*	port_com_send("play_snd(0)");
	printf("Playing sound--------------------------------------------> \n");*/


///////////////////////////////////////////////




	EyelockLog(logger, DEBUG, "Turning on Alternate cameras");
	//sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Need Just faced camera to be ON (changed by Mo)
	port_com_send(cmd);

	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");




/*	//Reading the calibrated Rect
	ifstream m_AuxRect(a_calibFile);
	vector<int> a_rect;
	string line;
	unsigned int value;
	int targetOffset = 3;

	if (m_AuxRect.is_open()){
		while(getline(m_AuxRect, line)){
			value = atoi(line.c_str());
			a_rect.push_back(value);
		}
	}
	else{
		printf("Can't find auxRect.csv File! Possibly the device is not calibrated!\n");
		EyelockLog(logger, DEBUG, "Can't find auxRect.csv File! Possibly the device is not calibrated!");
	}


	no_move_area.x = a_rect[0]/scaling;
	no_move_area.y = a_rect[1]/scaling + targetOffset;
	no_move_area.width = a_rect[2]/scaling;
	no_move_area.height = (a_rect[3])/scaling -targetOffset*2;*/

	//Reading the calibrated Rect
	targetOffset = 3;		//Adding offset

	no_move_area.x = rectX/scaling;
	no_move_area.y = rectY/scaling + targetOffset;
	no_move_area.width = rectW/scaling;
	no_move_area.height = (rectH)/scaling -targetOffset*2;

	system("touch /home/root/Eyelock.run");
}


//This settings only need for camera to camera calibration
void DoStartCmd_CamCal(){

	EyelockLog(logger, TRACE, "DoStartCmd_CamCal");
	char cmd[512];

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	MIN_POS = fconfig.getValue("FTracker.minPos",0);
	startPoint = fconfig.getValue("FTracker.startPoint",100);
	MAX_POS = fconfig.getValue("FTracker.maxPos",350);
	CENTER_POS = fconfig.getValue("FTracker.centerPos",164);
	step = fconfig.getValue("FTracker.CalStep",15);
	calDebug = fconfig.getValue("FTracker.calDebug",false);
	calTwoPoint = fconfig.getValue("FTracker.twoPointCalibration",true);

	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);

	int calibVolt = fconfig.getValue("FTracker.calibVolt",30);
	int calibCurrent = fconfig.getValue("FTracker.calibCurrent",30);
	int calibTrigger = fconfig.getValue("FTracker.calibTrigger",1);
	int calibLEDEnable = fconfig.getValue("FTracker.calibLEDEnable",1);
	int calibLEDMaxTime = fconfig.getValue("FTracker.calibLEDMaxTime",4);

	int faceCamExposureTime = fconfig.getValue("FTracker.calibFaceCamExposureTime",2);
	int faceCamDigitalGain = fconfig.getValue("FTracker.calibFaceCamDigitalGain",48);
	int faceCamDataPedestal = fconfig.getValue("FTracker.calibFaceCamDataPedestal",0);

	int AuxIrisCamExposureTime = fconfig.getValue("FTracker.calibAuxIrisCamExposureTime",3);
	int AuxIrisCamDigitalGain = fconfig.getValue("FTracker.calibAuxIrisCamDigitalGain",64);
	int AuxIrisCamDataPedestal = fconfig.getValue("FTracker.calibAuxIrisCamDataPedestal",0);

	int MainIrisCamExposureTime = fconfig.getValue("FTracker.calibMainIrisCamExposureTime",3);
	int MainIrisCamDigitalGain = fconfig.getValue("FTracker.calibMainIrisCamDigitalGain",64);
	int MainIrisCamDataPedestal = fconfig.getValue("FTracker.calibMainIrisCamDataPedestal",0);


	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	port_com_send("fx_home()\n");

#ifdef NOOPTIMIZE
	usleep(100000);
#endif


	//Reset the lower motion
	sprintf(cmd, "fx_abs(%i)",MIN_POS);
	EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", MIN_POS);
	port_com_send(cmd);
	//printf(cmd);
	//printf("------------------------------------------------------------------>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n", calibVolt, allLEDhighVoltEnable, calibCurrent, calibTrigger, calibLEDEnable, calibLEDMaxTime);

	//printf("Before EyelockLOG\n");
	EyelockLog(logger, DEBUG, "Configuring face LEDs faceLEDVolt:%d, allLEDhighVoltEnable:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d, faceLEDmaxTime:%d", calibVolt, allLEDhighVoltEnable, calibCurrent, calibTrigger, calibLEDEnable, calibLEDMaxTime);
	port_com_send(cmd);

	//printf("faceCamExposureTime:: %d", faceCamExposureTime);
	//printf("faceCamDataPedestal:: %d", faceCamDataPedestal);
	//printf("faceCamDigitalGain:: %d", faceCamDigitalGain);


	//Setting up cap current
	port_com_send("psoc_write(9,60)");	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	// faceCamDataPedestal, faceCamDigitalGain);
	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,2) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0x30)");


	//AUX cameras configuration
	EyelockLog(logger, DEBUG, "Configuring AUX Iris Cameras");
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x18,0x3012,3) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,0x40)");

	//Main Iris Cameras Configuration
	EyelockLog(logger, DEBUG, "Configuring Main Iris Cameras");
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", faceCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x03,0x3012,3) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,0x40)");


	// setup up all pll values
	EyelockLog(logger, DEBUG, "setting up PLL");
	//following process will activate PLL for all cameras

	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	port_com_send("wcr(0x4,0x30b0,0x80");		//Only face camera gain is x90

	port_com_send("wcr(0x1f,0x301a,0x1998)"); // ilya added to leave the pll on always


}





float AGC(int width, int height,unsigned char *dsty, int limit)
{
	EyelockLog(logger, TRACE, "AGC");
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

	EyelockLog(logger, DEBUG, "average : %3.1f percentile : %3.1f\n",average,percentile);
	return (float)percentile;
}



float AGC_average(int width, int height,unsigned char *dsty, int limit)
{
	EyelockLog(logger, TRACE, "AGC");
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

	EyelockLog(logger, DEBUG, "average : %3.1f percentile : %3.1f\n",average,percentile);
	return (float)average;
}



Mat rotate(Mat src, double angle)
{
	EyelockLog(logger, TRACE, "rotate");
	Mat dst;
    Point2f pt(src.cols*0.5, src.rows*0.5);
    Mat M = cv::getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(src, dst, M, src.size());
    //cv::warpAffine(src, dst, M, smallImgBeforeRotate.size(),cv::INTER_CUBIC);
    M.release();
    return dst;
}



Mat rotation90(Mat src){
	EyelockLog(logger, TRACE, "rotation90");
	Mat dst;
	transpose(src, dst);
	flip(dst,dst,0);
	return dst;

}


int IrisFramesHaveEyes()
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



double StandardDeviation(std::vector<double> samples)
{
     return sqrt(Variance(samples));
}

double Variance(std::vector<double> samples)
{
     int size = samples.size();

     double variance = 0;
     double t = samples[0];
     for (int i = 1; i < size; i++)
     {
    	 printf("Data:::: %3.4f\n", samples[i]);
          t += samples[i];
          double diff = ((i + 1) * samples[i]) - t;
          variance += (diff * diff) / ((i + 1.0) *i);
     }
     printf("variance:::: %4.4f\n",variance / (size - 1));
     return variance / (size - 1);
}

float StandardDeviation_m1(vector<float> vec){

	float sum  = 0.0;
	int n = 0;
	for(int i = 0; i < vec.size(); i++){
		sum = sum + vec[i];
		printf("Data:::: %3.4f\n", vec[i]);
		n++;
	}

	float mean = sum/n;
	printf("n:::::%d     MEan:::: %4.4f\n",n,mean);

	float sumPrd = 0.0;
	for(int i = 0; i < vec.size(); i++){
		sumPrd = sumPrd + pow(vec[i] - mean,2);
	}
	 float std = sqrt(sumPrd/n - 1);

	return std;

}


//#define LED_brightness_control
//#define Tempering



Mat preProcessingImg(Mat outImg){
	float p;

	EyelockLog(logger, DEBUG, "preProcessing");
	EyelockLog(logger, DEBUG, "resize");
	cv::resize(outImg, smallImgBeforeRotate, cv::Size(), (1 / scaling),
			(1 / scaling), INTER_NEAREST);	//py level 3

	//std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	smallImg = rotation90(smallImgBeforeRotate);	//90 deg rotation
/*	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << time_span.count() << endl;;

	// can be used for saving temp data
	fstream infile(fileName);
	if(infile.good()){
		// cout << "File exist and keep writing in it!" << endl;
		EyelockLog(logger, DEBUG, "File exist and keep writing in it");
	}
	else{
		EyelockLog(logger, DEBUG, "Create Log file");
	}

	ofstream writeFile(fileName, std::ios_base::app);
	writeFile <<  time_span.count() << '\n';*/


	//AGC control to block wash out images
	EyelockLog(logger, DEBUG, "AGC Calculation");
	p = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);

	if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	agc_val = MAX(FACE_GAIN_MIN,agc_val);
	agc_val = MIN(agc_val,FACE_GAIN_MAX);

	AGC_Counter++;



	static int agc_val_old = 0;
	if (abs(agc_val - agc_val_old) > 300) {
		// printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
		SetExp(4, agc_val);
		agc_val_old = agc_val;
	}

	agc_set_gain = agc_val;


	return smallImg;
}


void LEDbrightnessControl(Mat smallImg){
	float agcAvg, agcPer;
	int limit = 0;
	agcAvg = AGC_average(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),limit);
	agcPer = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),limit);

	// can be used for saving temp data
	fstream infile(fileName);
	if(infile.good()){
		// cout << "File exist and keep writing in it!" << endl;
		EyelockLog(logger, DEBUG, "File exist and keep writing in it");
	}
	else{
		EyelockLog(logger, DEBUG, "Create Log file");
		// cout << "Create log file!" << endl;
		ofstream file(fileName);
		file << "Time" << ',' << "image Average" << ',' << "Image percentile" << ',' << "Cutoff Limit" << '\n';
	}

	ofstream writeFile(fileName, std::ios_base::app);


	using std::chrono::system_clock;

	std::chrono::duration<int,std::ratio<60*60*24> > one_day (1);

	system_clock::time_point today = system_clock::now();

	std::time_t tt;

	tt = system_clock::to_time_t ( today );

	writeFile <<  ctime(&tt) << ',';
	writeFile <<  agcAvg << ',';
	writeFile <<  agcPer << ',';
	writeFile <<  limit << '\n';

	if (agcAvg > 100){
		// SET LED AS high

	}
	else
	{
		//Set default LED
	}
}

float last_angle_move=0;
void moveMotorToFaceTarget(float eye_size, bool bShowFaceTracking, bool bDebugSessions){
	if ((eye_size >= MIN_FACE_SIZE) && (eye_size <= MAX_FACE_SIZE)) {	// check face size

		float err;
		int MoveToLimitBound = 1;
		err = (no_move_area.y + no_move_area.height / 2) - eyes.y;
		EyelockLog(logger, DEBUG, "abs err----------------------------------->  %d\n", abs(err));
		err = (float) err * (float) SCALE * (float) ERROR_LOOP_GAIN;

		// if we need to move
		if (abs(err) > MoveToLimitBound) {
			int x, w, h;

			EyelockLog(logger, DEBUG, "Switching ON IRIS LEDs!!!!\n");

#ifdef DEBUG_SESSION
			if(bDebugSessions){
				struct stat st = {0};
				if (stat(m_sessionDir.c_str(), &st) == -1) {
					mkdir(m_sessionDir.c_str(), 0777);
				}
				// boost::filesystem::path temp_session_dir(DEBUG_SESSION_DIR);
				// boost::filesystem::create_directory(temp_session_dir);

				if (switchedToIrisMode == false)
				{
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

	}
	else{
		EyelockLog(logger, DEBUG, "Face out of range\n");
	}

}


void faceModeState(bool bDebugSessions){
	MoveTo(CENTER_POS);
	run_state = RUN_STATE_FACE;
	SetFaceMode();
#ifdef DEBUG_SESSION
	if(bDebugSessions){
		switchedToIrisMode = false;
		char logmsg[] = "Switched_to_face_mode";
		LogSessionEvent(logmsg);
	}
#endif /* DEBUG_SESSION */

}

void switchStaes(int states, float eye_size, bool bShowFaceTracking, bool bDebugSessions){
	switch (states){
		case 1: printf("set Iris mode\n");
				SetIrisMode(eye_size);
				break;
		case 2: printf("Move Motor to face target\n");
				moveMotorToFaceTarget(eye_size, bShowFaceTracking, bDebugSessions);
				break;
		case 3: printf("set Face mode\n");
				faceModeState(bDebugSessions);
				break;
		default:printf("Default---------------------------------\n");
				break;
	}
}


float eye_size;
float p; //AGC


void DoRunMode(bool bShowFaceTracking, bool bDebugSessions)
{
	EyelockLog(logger, TRACE, "DoRunMode");
	Rect face;

	// if (run_state == RUN_STATE_FACE)
	{

		EyelockLog(logger, DEBUG, "image resize");
		cv::resize(outImg, smallImgBeforeRotate, cv::Size(), (1 / scaling),
				(1 / scaling), INTER_NEAREST);	//py level 3

		smallImg = rotation90(smallImgBeforeRotate);	//90 deg rotation


		//AGC control to block wash out images
		EyelockLog(logger, DEBUG, "AGC Calculation");
		p = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);

		if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
			agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
		if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
			agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
		agc_val = MAX(FACE_GAIN_MIN,agc_val);
		agc_val = MIN(agc_val,FACE_GAIN_MAX);

		AGC_Counter++;
		if (agc_set_gain != agc_val)
			;	// && AGC_Counter%2==0)
		{
			//	while (waitKey(10) != 'z');
			{
				static int agc_val_old = 0;
				if (abs(agc_val - agc_val_old) > 300) {
					// printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
					SetExp(4, agc_val);
					agc_val_old = agc_val;
				}
			}

			agc_set_gain = agc_val;
		}


#ifdef LED_brightness_control


		float agcAvg, agcPer;
		int limit = 0;
		agcAvg = AGC_average(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),limit);
		agcPer = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),limit);

		// can be used for saving temp data
		fstream infile(fileName);eye_size
		if(infile.good()){
			// cout << "File exist and keep writing in it!" << endl;
			EyelockLog(logger, DEBUG, "File exist and keep writing in it");
		}
		else{
			EyelockLog(logger, DEBUG, "Create Log file");
			// cout << "Create log file!" << endl;
			ofstream file(fileName);
			file << "Time" << ',' << "image Average" << ',' << "Image percentile" << ',' << "Cutoff Limit" << '\n';
		}

		ofstream writeFile(fileName, std::ios_base::app);


		using std::chrono::system_clock;

		std::chrono::duration<int,std::ratio<60*60*24> > one_day (1);

		system_clock::time_point today = system_clock::now();

		std::time_t tt;

		tt = system_clock::to_time_t ( today );

		writeFile <<  ctime(&tt) << ',';
		writeFile <<  agcAvg << ',';
		writeFile <<  agcPer << ',';
		writeFile <<  limit << '\n';

		if (agcAvg > 100){
			// SET LED AS high

		}
		else
		{
			//Set default LED
		}

#endif



		EyelockLog(logger, DEBUG, "FindEyeLocation");
		if (FindEyeLocation(smallImg, eyes, eye_size, face)) {	//Find face
			if (detect_area.contains(eyes)) {		//Find face/eye into the rect first, Future use of detect face at center
				EyelockLog(logger, DEBUG, "eyes found");

				noFaceCounter = 0;


				int CurrentEye_distance = eye_size;
				int diffEyedistance = abs(CurrentEye_distance - previousEye_distance);
				//int diffEyedistance = abs(CurrentEye_distance - previousEye_distance);

			//	printf("Switching ON IRIS LEDs!!!!\n");
				SetIrisMode(eye_size);
				printf("set iris mode\n");

				run_state = RUN_STATE_EYES;

				if (!no_move_area.contains(eyes)) {		//if target area doesn't have face then move
					if ((eye_size >= MIN_FACE_SIZE)
							&& (eye_size <= MAX_FACE_SIZE)) {	// check face size

						float err;

						int MoveToLimitBound = 1;
						err = (no_move_area.y + no_move_area.height / 2)
								- eyes.y;
						EyelockLog(logger, DEBUG, "abs err----------------------------------->  %d\n", abs(err));
						err = (float) err * (float) SCALE * (float) ERROR_LOOP_GAIN;

						// if we need to move
						if (abs(err) > MoveToLimitBound) {
							int x, w, h;

							//Experiment ----> Turn ON iris cameras before the motor take action
							/////////////////////////////////////////////////
							EyelockLog(logger, DEBUG, "Switching ON IRIS LEDs!!!!\n");

#ifdef DEBUG_SESSION
							if(bDebugSessions){
								struct stat st = {0};
								if (stat(m_sessionDir.c_str(), &st) == -1) {
									mkdir(m_sessionDir.c_str(), 0777);
								}
								// boost::filesystem::path temp_session_dir(DEBUG_SESSION_DIR);
								// boost::filesystem::create_directory(temp_session_dir);

								if (switchedToIrisMode == false)
								{
									switchedToIrisMode = true;
									char logmsg[] = "Switching to iris mode";
									LogSessionEvent(logmsg);
								}
							}
#endif


							////////////////////////////////////////////////

							MoveRelAngle(-1 * err);


							//flush the video buffer to get rid of frames from motion
							{
								vs->flush();
							}

						}

					} else{
						EyelockLog(logger, DEBUG, "Face out of range\n");
					}
				} else {

					cv::rectangle(smallImg,
							Rect(eyes.x - 5, eyes.y - 5, 10, 10),
							Scalar(255, 0, 0), 2, 0);

					//IF we want to turn on the iris cameras after motor take action

				//							printf("Switching ON IRIS LEDs!!!!\n");
				//	 SetIrisMode(eye_size, diffEyedistance);
				//	 run_state = RUN_STATE_EYES;

					//port_com_send("fixed_set_rgb(0,0,50)");
				}

			}



		}
		{

			if (noFaceCounter < (NO_FACE_SWITCH_FACE + 2))
				noFaceCounter++;

			if (noFaceCounter == NO_FACE_SWITCH_FACE) {
				MoveTo(CENTER_POS);
				run_state = RUN_STATE_FACE;
				SetFaceMode();
				printf("Set Face mode\n");
#ifdef DEBUG_SESSION
				if(bDebugSessions){
					switchedToIrisMode = false;
					char logmsg[] = "Switched_to_face_mode";
					LogSessionEvent(logmsg);

#if 0
					boost::filesystem::path temp_session_dir(DEBUG_SESSION_DIR);
					if (boost::filesystem::is_directory(temp_session_dir))
					{
						time_t timer;
						struct tm* tm1;
						time(&timer);
						tm1 = localtime(&timer);
						char time_str[100];

						strftime(time_str, 100, "_%Y_%m_%d_%H-%M-%S", tm1);
						boost::filesystem::path session_dir(DEBUG_SESSION_DIR+std::string(time_str));

						FILE *file = fopen(DEBUG_SESSION_INFO, "a");
						if (file){
							strftime(time_str, 100, "%Y %m %d %H:%M:%S", tm1);
							fprintf(file, "[%s] Session closed\n", time_str);
							fclose(file);
						}

						rename(temp_session_dir.c_str(), session_dir.c_str());
					}
#endif
				}
#endif /* DEBUG_SESSION */
			}

		}


		//EyelockLog(logger, TRACE, "Just before Tempering ------------------------>>>>>>>>>>>>>>> \n");
#ifdef Tempering
		int t = clock();
		if ((len = port_com_send_return("accel", temp, 20)) > 0) {
			//printf("Inside tempering ------------------------------>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			EyelockLog(logger, TRACE, "got data %d", len);
			temp[len] = 0;
			sscanf(temp, "%f %f %f %f", &x, &y, &z, &a);
			EyelockLog(logger, TRACE, "Buffer =>%s\n", temp);
			EyelockLog(logger, TRACE, "%3.3f %3.3f %3.3f %3.3f  readTime=%2.4f\n",x, y, z, a, (float) (clock() - t) / CLOCKS_PER_SEC );
			EyelockLog(logger, TRACE, "ACCEL reading ------------------------------------------------>>>>>>>>>>>>>>>>\n");
			EyelockLog(logger, TRACE, "x::: %3.3f y::: %3.3f z::: %3.3f a::: %3.3f\n", x,y,z,a);

			ofstream writeFile(fileName, std::ios_base::app);


			using std::chrono::system_clock;

			std::chrono::duration<int,std::ratio<60*60*24> > one_day (1);

			system_clock::time_point today = system_clock::now();

			std::time_t tt;

			tt = system_clock::to_time_t ( today );

			//writeFile <<  ctime(&tt) << ',';
			writeFile <<  x << ',';
			writeFile <<  y << ',';
			writeFile <<  z << ',';



			difX = fabs(x - xp);
			difY = fabs(y - yp);
			difZ = fabs(z - zp);
			xp = x; yp = y; zp = z;

			writeFile <<  difX << ',';
			writeFile <<  difY << ',';
			writeFile <<  difZ << ',';
			writeFile <<  a << '\n';

			int numData = 10;
			if(tempDataX.size()<=numData)
			{
				printf("monitoring temparing-------------->>>>>>>>>! \n");
				tempDataX.push_back(difX);
				tempDataY.push_back(difY);
				tempDataZ.push_back(difZ);
			}
			//tempDataX.erase(0,1);


			if (tempDataX.size() > numData){

				tempDataX.erase(tempDataX.begin(),tempDataX.begin()+2);
				tempDataY.erase(tempDataY.begin(),tempDataY.begin()+2);
				tempDataZ.erase(tempDataZ.begin(),tempDataZ.begin()+2);

				float xAvg = average(tempDataX);
				float yAvg = average(tempDataY);
				float zAvg = average(tempDataZ);
				printf("AVG val :::::: %3.3f, %3.3f, %3.3f -------------->>>>>>>>>! \n", xAvg,yAvg,zAvg);
				EyelockLog(logger, TRACE, "AVG val :::::: %3.3f, %3.3f, %3.3f -------------->>>>>>>>>! \n", xAvg,yAvg,zAvg);
				if (xAvg > 0.01 & yAvg > 0.01 & zAvg > 0.08){
					printf("TEMPER DETECTED!!!!!!!!!!!!!!!!!!!!!!------------------>>>>>>>>>>> \n");
					system("touch /home/root/OIMtamper");
					//exit(EXIT_FAILURE);;
				}
				char s = cv::waitKey(1);
/*				if (s == 'q')
					break;
				else{}*/


				tempDataX.clear();
				tempDataY.clear();
				tempDataZ.clear();
			}


		}
#endif
		//EyelockLog(logger, TRACE, "Just after Tempering ------------------------>>>>>>>>>>>>>>> \n");



		if(bShowFaceTracking){
			sprintf(temp, "Debug facetracker Window\n");
			EyelockLog(logger, DEBUG, "Imshow");
			cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
			imshow(temp, smallImg);
		}
#ifdef DEBUG_SESSION
		if (bDebugSessions)
		{
			if (switchedToIrisMode)
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
					sprintf(filename, "%s/FaceImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, FrameNo, CamId);

					imwrite(filename, smallImg);

					char logmsg[300];
					sprintf(logmsg, "Saved-FaceImage-FrNum%d-CamID%d-%s", FrameNo, CamId, filename);
					LogSessionEvent(logmsg);
				}
			}
		}
#endif
	}

}


#define STATE_LOOK_FOR_FACE 1
#define STATE_MOVE_MOTOR    2
#define STATE_MAIN_IRIS     3
#define STATE_AUX_IRIS      4
#define Tempering
#define tempRecord


void *DoTemperatureLog(void * arg)
{
	static time_t start = time(0);
	double seconds_since_start = difftime( time(0), start);
	int len;

	//printf ("%3.3f seconds ", seconds_since_start);

	//printf("cumilative time in sec :::::::::::: %d \n", seconds_since_start);
	if (tempTarget < int(seconds_since_start)){

		//std::string temperatureLogFileName("temperature.log");
		//ofstream temperatureLogStream(temperatureLogFileName, std::ios_base::app);

		float tempData;
		char temperatureBuf[512];
		//float sumST = 0.0;

		if ((len = port_com_send_return("accel_temp()", temperatureBuf, 20)) > 0) {
			sscanf(temperatureBuf, "%f", &tempData);
			EyelockLog(logger, TRACE, "temp reading =>%3.3f\n", tempData);

			//printf("cumilative time ::: %3.3f and Temp ::::: %3.3f\n", sumST, tempData);
			//temperatureLogStream <<  tempData << '\n';

			time_t timer;
			struct tm* tm1;
			time(&timer);
			tm1 = localtime(&timer);

			FILE *file = fopen("temperature.log", "a");
			if (file){
				char time_str[100];
				strftime(time_str, 100, "%Y %m %d %H:%M:%S", tm1);
				fprintf(file, "[%s] %3.3f\n", time_str, tempData);
				fclose(file);
			}

			//sumST = 0.0;
			start = time(0);
		}
	}
}


int system_state = STATE_LOOK_FOR_FACE;
int last_system_state = STATE_LOOK_FOR_FACE;

int SelectWhichIrisCam(float eye_size, int cur_state)
{
	if ((cur_state!=STATE_MAIN_IRIS) &&(cur_state!=STATE_AUX_IRIS))
		{
		// this is we are just getting into irises so hard decision not hysterises
		if (eye_size >= (switchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;

		}
	if (cur_state==STATE_MAIN_IRIS)
	   if (eye_size<(switchThreshold-errSwitchThreshold))
		   return STATE_AUX_IRIS;
	   else
		   return STATE_MAIN_IRIS;

	if (cur_state==STATE_AUX_IRIS)
		if (eye_size >= (switchThreshold+errSwitchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;
}

char * StateText(int state)
{
	switch (state)
	{
	    case STATE_LOOK_FOR_FACE: return("FACE");
		case STATE_MAIN_IRIS: return ("MAIN");
		case STATE_AUX_IRIS:  return ("AUX");
		case STATE_MOVE_MOTOR:return ("MOVE");
	}
	return "none";
}

void DoAgc(void)
{
	p = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);

	if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
		agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	agc_val = MAX(FACE_GAIN_MIN,agc_val);
	agc_val = MIN(agc_val,FACE_GAIN_MAX);

	AGC_Counter++;
	if (agc_set_gain != agc_val)
		;	// && AGC_Counter%2==0)
	{
		//	while (waitKey(10) != 'z');
		{
			static int agc_val_old = 0;
			if (abs(agc_val - agc_val_old) > 300) {
				// printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
				SetExp(4, agc_val);		//comment out if O2 led is connected
				agc_val_old = agc_val;
			}
		}

		agc_set_gain = agc_val;
	}
}



/*float projectPoints(float y, float c, float m){
	//y = mx + c
	//y is Face points and x is Iris points
	//x = (y-c)/m
	printf("y::: %3.3f, c::: %3.3f, m::: %3.3f, x::: %3.3f\n", y,c,m, float(abs(y-c)/m));
	return (abs(y-c)/m);

}*/

float projectPoints(float y, float c, float m){
	//y = mx + c
	//y is Face points and x is Iris points
	//x = (y-c)/m
	//printf("y::: %3.3f, c::: %3.3f, m::: %3.3f, x::: %3.3f\n", y,c,m, float((y-c)/m));
	return ((y-c)/m);

}



coordinateProject projectRect(Rect &face, int stateofIrisCameras){

	printf("scaling %d targetOffset %d\n", scaling, targetOffset);

	printf("constantMainl.x  %f constantMainl.y %f magOffMainl %f\n", constantMainl.x ,constantMainl.y,  magOffMainl);

	printf("Rect %d %d %d %d\n", rectX, rectY, rectW, rectH);

	float scale = 1.0;
	//showProjection = true;
	Rect ret1(0,0,0,0);
	Rect ret2(0,0,0,0);
	Rect retd1(0,0,0,0);
	Rect retd2(0,0,0,0);

/*	cam_idd = frame->imageData[2]&0xff;
	frame_number = frame->imageData[3]&0xff;*/


	// printf("targetOffset %d\n", targetOffset);


	Mat imgCopy;
	smallImg.copyTo(imgCopy);



	Rect faceP = face;
	faceP.y = (rectY/scaling) + targetOffset;
	faceP.height = (rectH)/scaling -(targetOffset*2);

	if (projPtr){
		sprintf(temp, "Debug facetracker WindowX\n");
		EyelockLog(logger, DEBUG, "ImshowX");


		if (projDebug){

			/*
			Mat outImgM = rotation90(outImg);

			printf("face x = %d  face y = %d face width = %d  face height = %d\n",face.x, face.y, face.width, face.height);
			face.x =face.x * scaling;
			face.y =face.y * scaling;
			face.height =face.height * scaling;
			face.width =face.width * scaling;
			printf("face x = %d  face y = %d face width = %d  face height = %d\n",face.x, face.y, face.width, face.height);

			projFace.x = face.x;				//column
			projFace.y = rectY + targetOffset;	//row
			projFace.width = face.width;
			projFace.height = rectH -(targetOffset*2);*/

			//printf("After scaling %d targetOffset %d\n", scaling, targetOffset);

			//printf("magOffMainl %d magOffAuxl %d\n", magOffMainl, magOffAuxl);


			projFace.x = face.x * scaling;				//column
			projFace.y = rectY + targetOffset;	//row
			projFace.width = face.width * scaling;
			projFace.height = rectH -(targetOffset*2);
			//printf("projFace x = %d  projFace y = %d projFace width = %d  projFace height = %d\n",projFace.x, projFace.y, projFace.width, projFace.height);

			//printf("face x = %i  face y = %i face width = %i  face height = %i\n",face.x, face.y, face.width, face.height);
			//cv::rectangle(outImgM, no_move_areaX, Scalar(255, 0, 0), 1, 0);
			//cv::rectangle(outImgM, projFace, Scalar(255, 0, 0), 1, 0);
			//cv::rectangle(outImgM, face, Scalar(255,0,0),1,0);



			//vs->cam_id face is 4; main is 1 left and 2 right; Aux is 129 left and 130 right
			//int projectPoints(float y, float c, float m)
			// y-> is face point, c is contant and m is the offset
			//ptr.x-> column, ptr.y-> row
			//magOffMainl, magOffMainR, magOffAuxl, magOffAuxR;
			//constantMainl.x, constantMainl.y, constantMainR.x, constantMainR.y, constantAuxl.x, constantAuxl.y, constantAuxR.x, constantAuxR.y;
			cv::Point2i ptr1, ptr2, ptr3, ptr4;

			if(stateofIrisCameras == STATE_MAIN_IRIS){
			// if(vsExp1->cam_id == 1){
				//project two coordinates diagonally a part in face frame into Iris
				ptr1.x = projectPoints(projFace.x, constantMainl.x, magOffMainl);
				ptr1.y = projectPoints(projFace.y, constantMainl.y, magOffMainl);

				ptr2.x = projectPoints((face.x+face.width), constantMainl.x, magOffMainl);
				ptr2.y = projectPoints((projFace.y+projFace.height), constantMainl.y, magOffMainl);


				//check extreme conditions
				if (ptr1.x > 1200){
					ptr1.x = 1200;
				}
				if (ptr2.y > 960){
					ptr2.y = 960;
				}

				if (ptr1.x < 0){
					ptr1.x = 0;
				}
				if (ptr2.y < 0){
					ptr2.y = 0;
				}

				//create RECT
				ret1.x = ptr1.x;
				ret1.y = ptr1.y;
				ret1.width = abs(ptr1.x - ptr2.x);
				ret1.height = abs(ptr1.y - ptr2.y);

				//check extreme conditions
				if(ret1.width > 1200)
					ret1.width = 1200;
				if(ret1.height > 960)
					ret1.height = 960;

				//Offset correction if useOffset is true
				if(useOffest_m){
					ret1.height = ret1.height - int(projOffset_m);
				}

				if (showProjection){
					retd1.x = ptr1.x/scale;
					retd1.y = ptr1.y/scale;
					retd1.width = abs(ptr1.x - ptr2.x)/scale;

					if (useOffest_m)
						retd1.height = (abs(ptr1.y - ptr2.y)-projOffset_m)/scale;
					else
						retd1.height = abs(ptr1.y - ptr2.y)/scale;

				}

				printf("Main Left::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",STATE_MAIN_IRIS, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
				printf("Main Left::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",STATE_MAIN_IRIS, ret2.x,ret2.y,ret2.width,ret2.height);


				//printf("Camera ID::: %d, ptr1.x:: %d, ptr1.y:: %d, ptr2.x:: %d, ptr2.y:: %d\n",vsExp1->cam_id, ptr1.x,ptr1.y,ptr2.x,ptr2.y);
				//printf("Camera ID::: %d, ret1.x:: %d, ret1.y:: %d, ret1.width:: %d, ret1.height:: %d\n",vsExp1->cam_id, ret1.x,ret1.y,ret1.width,ret1.height);

			}
			else if(stateofIrisCameras == STATE_AUX_IRIS){
			// else if(vsExp1->cam_id == 129){
				//project two coordinates diagonally a part in face frame into Iris
				ptr1.x = projectPoints(projFace.x, constantAuxl.x, magOffAuxl);
				ptr1.y = projectPoints(projFace.y, constantAuxl.y, magOffAuxl);

				ptr2.x = projectPoints((face.x+face.width), constantAuxl.x, magOffAuxl);
				ptr2.y = projectPoints((projFace.y+projFace.height), constantAuxl.y, magOffAuxl);


				//check extreme conditions
				if (ptr1.x > 1200){
					ptr1.x = 1200;
				}
				if (ptr2.y > 960){
					ptr2.y = 960;
				}

				if (ptr1.x < 0){
					ptr1.x = 0;
				}
				if (ptr2.y < 0){
					ptr2.y = 0;
				}

				//create RECT
				ret1.x = ptr1.x;
				ret1.y = ptr1.y;
				ret1.width = abs(ptr1.x - ptr2.x);
				ret1.height = abs(ptr1.y - ptr2.y);

				//check extreme conditions
				if(ret1.width > 1200)
					ret1.width = 1200;
				if(ret1.height > 960)
					ret1.height = 960;

				//Offset correction if useOffset is true
				if(useOffest_a){
					ret1.height = ret1.height - int(projOffset_a);
				}

				if (showProjection){
					retd1.x = ptr1.x/scale;
					retd1.y = ptr1.y/scale;
					retd1.width = abs(ptr1.x - ptr2.x)/scale;
					if (useOffest_a)
						retd1.height = (abs(ptr1.y - ptr2.y)-projOffset_a)/scale;
					else
						retd1.height = abs(ptr1.y - ptr2.y)/scale;
				}

				printf("Aux Left::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",STATE_AUX_IRIS, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
				printf("Aux Left::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",STATE_AUX_IRIS, ret2.x,ret2.y,ret2.width,ret2.height);

				//printf("Camera ID::: %d, ptr1.x:: %d, ptr1.y:: %d, ptr2.x:: %d, ptr2.y:: %d\n",vsExp1->cam_id, ptr1.x,ptr1.y,ptr2.x,ptr2.y);
				//printf("Camera ID::: %d, ret1.x:: %d, ret1.y:: %d, ret1.width:: %d, ret1.height:: %d\n",vsExp1->cam_id, ret1.x,ret1.y,ret1.width,ret1.height);

			}

			if(stateofIrisCameras == STATE_MAIN_IRIS){
			// if(vsExp2->cam_id == 2){
				//project two coordinates diagonally a part in face frame into Iris
				ptr3.x = projectPoints(projFace.x, constantMainR.x, magOffMainR);
				ptr3.y = projectPoints(projFace.y, constantMainR.y, magOffMainR);

				ptr4.x = projectPoints((face.x+face.width), constantMainR.x, magOffMainR);
				ptr4.y = projectPoints((projFace.y+projFace.height), constantMainR.y, magOffMainR);


				//check extreme conditions
				if (ptr3.x > 1200){
					ptr3.x = 1200;
				}
				if (ptr4.y > 960){
					ptr4.y = 960;
				}

				if (ptr3.x < 0){
					ptr3.x = 0;
				}
				if (ptr4.y < 0){
					ptr4.y = 0;
				}

				//create RECT
				ret2.x = ptr3.x;
				ret2.y = ptr3.y;
				ret2.width = abs(ptr3.x - ptr4.x);
				ret2.height = abs(ptr3.y - ptr4.y);

				//check extreme conditions
				if(ret2.width > 1200)
					ret2.width = 1200;
				if(ret2.height > 960)
					ret2.height = 960;

				//Offset correction if useOffset is true
				if(useOffest_m){
					ret2.height = ret2.height - int(projOffset_m);
				}

				if (showProjection){
					retd2.x = ptr3.x/scale;
					retd2.y = ptr3.y/scale;
					retd2.width = abs(ptr3.x - ptr4.x)/scale;


					if (useOffest_m)
						retd2.height = (abs(ptr3.y - ptr4.y)-projOffset_m)/scale;
					else
						retd2.height = abs(ptr3.y - ptr4.y)/scale;

				}

				printf("Main Right::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",STATE_MAIN_IRIS, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
				printf("Main Right::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",STATE_MAIN_IRIS, ret2.x,ret2.y,ret2.width,ret2.height);

				}
			else if(stateofIrisCameras == STATE_AUX_IRIS){
			// else if(vsExp2->cam_id == 130){
				//project two coordinates diagonally a part in face frame into Iris
				ptr3.x = projectPoints(projFace.x, constantAuxR.x, magOffAuxR);
				ptr3.y = projectPoints(projFace.y, constantAuxR.y, magOffAuxR);

				ptr4.x = projectPoints((face.x+face.width), constantAuxR.x, magOffAuxR);
				ptr4.y = projectPoints((projFace.y+projFace.height), constantAuxR.y, magOffAuxR);

				//check extreme conditions
				if (ptr3.x > 1200){
					ptr3.x = 1200;
				}
				if (ptr4.y > 960){
					ptr4.y = 960;
				}

				if (ptr3.x < 0){
					ptr3.x = 0;
				}
				if (ptr4.y < 0){
					ptr4.y = 0;
				}

				//create RECT
				ret2.x = ptr3.x;
				ret2.y = ptr3.y;
				ret2.width = abs(ptr3.x - ptr4.x);
				ret2.height = abs(ptr3.y - ptr4.y);

				//check extreme conditions
				if(ret2.width > 1200)
					ret2.width = 1200;
				if(ret2.height > 960)
					ret2.height = 960;

				//Offset correction if useOffset is true
				if(useOffest_a){
					ret2.height = ret2.height - int(projOffset_a);
				}

				if (showProjection){
					retd2.x = ptr3.x/scale;
					retd2.y = ptr3.y/scale;
					retd2.width = abs(ptr3.x - ptr4.x)/scale;
					retd2.height = abs(ptr3.y - ptr4.y)/scale;
				}

				if (useOffest_a)
					retd2.height = (abs(ptr3.y - ptr4.y)-projOffset_a)/scale;
				else
					retd2.height = abs(ptr3.y - ptr4.y)/scale;

				printf("Aux Right::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",STATE_AUX_IRIS, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
				printf("Aux Right::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",STATE_AUX_IRIS, ret2.x,ret2.y,ret2.width,ret2.height);



			}



/*			CvFont font;
			double hScale=.5;
			double vScale=.5;
			int    lineWidth=1;*/

			//int s_camId = vsExp1->cam_id;
			//sprintf(tempI1, "Iris left cam");
			//sprintf(tempI1, "CAM %2x %s",s_camId,s_camId&0x80 ?  "AUX":"MAIN");
			//cvPutText (outImage1,drawComm,cvPoint(20,50), &font, cvScalar(255,255,255));
			//cvPutText ((unsigned char*)imgIS1,tempI1,cvPoint(25,100), &font, cvScalar(0,0,0));

			//int s_camIdd = vsExp2->cam_id;
			//sprintf(tempI2, "Iris Right cam");
			//sprintf(tempI1, "CAM %2x %s",s_camId,s_camId&0x80 ?  "AUX":"MAIN");
			//sprintf(tempI2, "CAM %2x %s",s_camIdd,s_camIdd&0x80 ?  "AUX":"MAIN");
			//cvPutText ((unsigned char*)imgIS2,tempI1,cvPoint(25,100), &font, cvScalar(0,0,0));



			if (showProjection){

				cv::rectangle(imgCopy, no_move_area, Scalar(255, 0, 0), 1, 0);
				cv::rectangle(imgCopy, faceP, Scalar(255, 0, 0), 1, 0);

				//scaling in a smaller image

				Mat imgl, imgR;
				cv::resize(imgIS1, imgl, cv::Size(), (1 / scale),(1 / scale), INTER_NEAREST);
				cv::resize(imgIS2, imgR, cv::Size(), (1 / scale),(1 / scale), INTER_NEAREST);

				cv::rectangle(imgl, retd1, Scalar(255, 0, 0), 2, 0);
				cv::rectangle(imgR, retd2, Scalar(255, 0, 0), 2, 0);

				imshow(temp, imgCopy);
				imshow(tempI1, imgl);
				imshow(tempI2, imgR);


			}
		}

	}
	else{
/*		vsExp1->cam_id = 0;
		vsExp2->cam_id = 0;*/
		return {0,0,ret1,0,0,ret2};
	}

	//printf("number ::::::::::::::: %i, ID::::::::::::%i\n", (int)imgIS1.at<uchar>(0,2), int(vsExp1->cam_id));

	return {int(0),(int)imgIS1.at<uchar>(0,3),ret1, int(0),(int)imgIS2.at<uchar>(0,3),ret2};
	// return {int(vsExp1->cam_id),(int)imgIS1.at<uchar>(0,3),ret1, int(vsExp2->cam_id),(int)imgIS2.at<uchar>(0,3),ret2};
}

cv::Rect ImgFaceData;

cv::Rect getFaceData()
{
	return ImgFaceData;
}

void DoRunMode_test(bool bShowFaceTracking, bool bDebugSessions){
	pthread_t threadIdtamper;
	pthread_t threadIdtemp;
	cv::Rect face;
	coordinateProject result;

	EyelockLog(logger, TRACE, "DoRunMode_test");

	int start_process_time = clock();

	smallImg = preProcessingImg(outImg);

	bool foundEyes = FindEyeLocation(smallImg, eyes, eye_size, face);
/*	float projOffset_m = 100.00, projOffset_a = 200.00;
	bool useOffest_m= false, useOffest_a = true;*/


	float process_time = (float) (clock() - start_process_time) / CLOCKS_PER_SEC;


	bool eyesInDetect = foundEyes? detect_area.contains(eyes):false;
	bool eyesInViewOfIriscam = eyesInDetect ? no_move_area.contains(eyes):false;
	//bool projPtr = false;


	if (foundEyes==false)
		noFaceCounter++;
	noFaceCounter = min(noFaceCounter,NO_FACE_SWITCH_FACE);

	if (foundEyes)
		noFaceCounter=0;
	last_system_state = system_state;

	// figure out our next state
	switch(system_state)
	{
	case STATE_LOOK_FOR_FACE:
							// we see eyes but need to move to them
							if (eyesInDetect && !eyesInViewOfIriscam)
								{
								system_state = STATE_MOVE_MOTOR;
								break;
								}
							//if (eyesInDetect && eyesInViewOfIriscam)			//changed by Ilya
							if (eyesInDetect)			//changed by Mo
									system_state = SelectWhichIrisCam(eye_size,system_state);
							DoAgc();
							//if (eyesInViewOfIriscam)
							break;

	case STATE_MAIN_IRIS:
	case STATE_AUX_IRIS:
						system_state = SelectWhichIrisCam(eye_size,system_state);
						if (noFaceCounter >= NO_FACE_SWITCH_FACE)
							{
							system_state=STATE_LOOK_FOR_FACE;
							break;
							}
						if (eyesInDetect &&  !eyesInViewOfIriscam)
							moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
						break;
	case STATE_MOVE_MOTOR:
						//if (eyesInViewOfIriscam)		//by ilya
						if (eyesInDetect)			//changed by Mo
							{
							system_state = SelectWhichIrisCam(eye_size,system_state);
							break;
							}
						if (!foundEyes)
							{
							system_state = STATE_LOOK_FOR_FACE;
							break;
							}
						DoAgc();
						moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
	}


	currnet_mode = -1;
	// handle switching state
	//if (last_system_state != system_state)
	EyelockLog(stateMachineLogger, TRACE, "STATE:%8s  NFC:%2d %c%c%c  I_SIZE:%03.1f  I_POS(%3d,%3d) MV:%3.3f TIME:%3.3f AGC:%5d MS:%d \n",StateText(system_state),
					noFaceCounter,
				foundEyes?'E':'.',
				eyesInDetect?'D':'.',
				eyesInViewOfIriscam?'V':'.',
						eye_size,
						eyes.x,
						eyes.y,
						last_angle_move,
						process_time,
						agc_set_gain,
						g_MatchState
						);
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
						// above states switches no action has to be taken
						moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
						// flush after moving to get more accurate motion on next loop
						vs->flush();
						break;
					case STATE_MAIN_IRIS:
						// enable main camera and set led currnet
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						projPtr = true;
						stateofIrisCameras = STATE_MAIN_IRIS;
						//result = projectRect(face);
						break;
					case STATE_AUX_IRIS:
						// enable aux camera and set led currnet
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(false);									//switch cameras
						projPtr = true;
						stateofIrisCameras = STATE_AUX_IRIS;
						//result = projectRect(face);
						break;
					}
					break;
	case STATE_AUX_IRIS:
					switch (system_state)
					{
					case STATE_MOVE_MOTOR: // cannot happen
					case STATE_LOOK_FOR_FACE:
						// disable iris camera set current for face camera
						MoveTo(CENTER_POS);
						SetFaceMode();
						break;
					case STATE_MAIN_IRIS:
						//if the switch happen from AUX to MAIN then we
						//dont need to dim down the face cam settings because it is already
						//dimmed down
						//DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						projPtr = true;
						stateofIrisCameras = STATE_MAIN_IRIS;
						//result = projectRect(face);
						break;
					}
					break;
	case STATE_MAIN_IRIS:
						switch (system_state)
						{
						case STATE_MOVE_MOTOR:
							break;
						case STATE_LOOK_FOR_FACE:
							// disable iris camera set current for face camera
							MoveTo(CENTER_POS);
							SetFaceMode();
							break;
						case STATE_AUX_IRIS:
							//if the switch happen from AUX to MAIN then we
							//dont need to dim down the face cam settings because it is already
							//dimmed down
							//DimmFaceForIris();
							MainIrisSettings();											//change to Iris settings
							SwitchIrisCameras(false);									//switch cameras
							projPtr = true;
							stateofIrisCameras = STATE_AUX_IRIS;
							//result = projectRect(face);
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
						break;
					case STATE_AUX_IRIS:
						// switch only the expusure and camera enables
						// no need to change voltage or current
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(false);									//switch cameras
						projPtr = true;
						stateofIrisCameras = STATE_AUX_IRIS;
						//result = projectRect(face);
						break;
					case STATE_MAIN_IRIS:
						// enable main cameras and set
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						projPtr = true;
						stateofIrisCameras = STATE_MAIN_IRIS;
						//result = projectRect(face);
						break;
					}
					break;
	}

	//For dispaying face tracker
	CvFont font;
		double hScale=0.7;
		double vScale=0.7;
		int lineWidth=1;
		bool static firstTime = true;
		if(firstTime){
						cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
						firstTime = false;
					}

		//
		ImgFaceData = face;
		//printf("stateofIrisCameras.....%d\n", stateofIrisCameras);
		result = projectRect(face, stateofIrisCameras);
		// printf("cameID1:: %d, Frame1:: %i, Rect1.x :: %d,Rect1.y :: %d,Rect1.width :: %d,Rect1.height :: %d\n", result.camIDL, result.frmaeL, result.camLRect.x,result.camLRect.y,result.camLRect.width,result.camLRect.height);
		// printf("cameID2:: %d, Frame2:: %i, Rect2.x :: %d,Rect2.y :: %d,Rect2.width :: %d,Rect2.height :: %d\n", result.camIDR, result.frmaeR, result.camRRect.x,result.camRRect.y,result.camRRect.width,result.camRRect.height);

/*
		float scale = 1.0;
		//showProjection = true;
		Rect ret1,ret2,retd1,retd2;

		Mat imgCopy;
		smallImg.copyTo(imgCopy);

		Rect faceP = face;
		faceP.y = (rectY/scaling) + targetOffset;
		faceP.height = (rectH)/scaling -(targetOffset*2);


		if (projPtr){
			sprintf(temp, "Debug facetracker WindowX\n");
			EyelockLog(logger, DEBUG, "ImshowX");


			if (projDebug){


				Mat outImgM = rotation90(outImg);

				printf("face x = %d  face y = %d face width = %d  face height = %d\n",face.x, face.y, face.width, face.height);
				face.x =face.x * scaling;
				face.y =face.y * scaling;
				face.height =face.height * scaling;
				face.width =face.width * scaling;
				printf("face x = %d  face y = %d face width = %d  face height = %d\n",face.x, face.y, face.width, face.height);

				projFace.x = face.x;				//column
				projFace.y = rectY + targetOffset;	//row
				projFace.width = face.width;
				projFace.height = rectH -(targetOffset*2);

				projFace.x = face.x * scaling;				//column
				projFace.y = rectY + targetOffset;	//row
				projFace.width = face.width * scaling;
				projFace.height = rectH -(targetOffset*2);
				printf("projFace x = %d  projFace y = %d projFace width = %d  projFace height = %d\n",projFace.x, projFace.y, projFace.width, projFace.height);

				//printf("face x = %i  face y = %i face width = %i  face height = %i\n",face.x, face.y, face.width, face.height);
				//cv::rectangle(outImgM, no_move_areaX, Scalar(255, 0, 0), 1, 0);
				//cv::rectangle(outImgM, projFace, Scalar(255, 0, 0), 1, 0);
				//cv::rectangle(outImgM, face, Scalar(255,0,0),1,0);



				//vs->cam_id face is 4; main is 1 left and 2 right; Aux is 129 left and 130 right
				//int projectPoints(float y, float c, float m)
				// y-> is face point, c is contant and m is the offset
				//ptr.x-> column, ptr.y-> row
				//magOffMainl, magOffMainR, magOffAuxl, magOffAuxR;
				//constantMainl.x, constantMainl.y, constantMainR.x, constantMainR.y, constantAuxl.x, constantAuxl.y, constantAuxR.x, constantAuxR.y;
				cv::Point2i ptr1, ptr2, ptr3, ptr4;
				Rect ret1,ret2,retd1,retd2;
				if(vsExp1->cam_id == 1){
					//project two coordinates diagonally a part in face frame into Iris
					ptr1.x = projectPoints(projFace.x, constantMainl.x, magOffMainl);
					ptr1.y = projectPoints(projFace.y, constantMainl.y, magOffMainl);

					ptr2.x = projectPoints((face.x+face.width), constantMainl.x, magOffMainl);
					ptr2.y = projectPoints((projFace.y+projFace.height), constantMainl.y, magOffMainl);


					//check extreme conditions
					if (ptr1.x > 1200){
						ptr1.x = 1200;
					}
					if (ptr2.y > 960){
						ptr2.y = 960;
					}

					if (ptr1.x < 0){
						ptr1.x = 0;
					}
					if (ptr2.y < 0){
						ptr2.y = 0;
					}

					//create RECT
					ret1.x = ptr1.x;
					ret1.y = ptr1.y;
					ret1.width = abs(ptr1.x - ptr2.x);
					ret1.height = abs(ptr1.y - ptr2.y);

					//check extreme conditions
					if(ret1.width > 1200)
						ret1.width = 1200;
					if(ret1.height > 960)
						ret1.height = 960;

					//Offset correction if useOffset is true
					if(useOffest_m){
						ret1.height = ret1.height - int(projOffset_m);
					}

					if (showProjection){
						retd1.x = ptr1.x/scale;
						retd1.y = ptr1.y/scale;
						retd1.width = abs(ptr1.x - ptr2.x)/scale;

						if (useOffest_m)
							retd1.height = (abs(ptr1.y - ptr2.y)-projOffset_m)/scale;
						else
							retd1.height = abs(ptr1.y - ptr2.y)/scale;

					}

					printf("Camera ID::: %d, ptr1.x:: %d, ptr1.y:: %d, ptr2.x:: %d, ptr2.y:: %d\n",vsExp1->cam_id, ptr1.x,ptr1.y,ptr2.x,ptr2.y);
					printf("Camera ID::: %d, ret1.x:: %d, ret1.y:: %d, ret1.width:: %d, ret1.height:: %d\n",vsExp1->cam_id, ret1.x,ret1.y,ret1.width,ret1.height);

				}
				else if(vsExp1->cam_id == 129){
					//project two coordinates diagonally a part in face frame into Iris
					ptr1.x = projectPoints(projFace.x, constantAuxl.x, magOffAuxl);
					ptr1.y = projectPoints(projFace.y, constantAuxl.y, magOffAuxl);

					ptr2.x = projectPoints((face.x+face.width), constantAuxl.x, magOffAuxl);
					ptr2.y = projectPoints((projFace.y+projFace.height), constantAuxl.y, magOffAuxl);


					//check extreme conditions
					if (ptr1.x > 1200){
						ptr1.x = 1200;
					}
					if (ptr2.y > 960){
						ptr2.y = 960;
					}

					if (ptr1.x < 0){
						ptr1.x = 0;
					}
					if (ptr2.y < 0){
						ptr2.y = 0;
					}

					//create RECT
					ret1.x = ptr1.x;
					ret1.y = ptr1.y;
					ret1.width = abs(ptr1.x - ptr2.x);
					ret1.height = abs(ptr1.y - ptr2.y);

					//check extreme conditions
					if(ret1.width > 1200)
						ret1.width = 1200;
					if(ret1.height > 960)
						ret1.height = 960;

					//Offset correction if useOffset is true
					if(useOffest_a){
						ret1.height = ret1.height - int(projOffset_a);
					}

					if (showProjection){
						retd1.x = ptr1.x/scale;
						retd1.y = ptr1.y/scale;
						retd1.width = abs(ptr1.x - ptr2.x)/scale;
						if (useOffest_a)
							retd1.height = (abs(ptr1.y - ptr2.y)-projOffset_a)/scale;
						else
							retd1.height = abs(ptr1.y - ptr2.y)/scale;
					}

					printf("Camera ID::: %d, ptr1.x:: %d, ptr1.y:: %d, ptr2.x:: %d, ptr2.y:: %d\n",vsExp1->cam_id, ptr1.x,ptr1.y,ptr2.x,ptr2.y);
					printf("Camera ID::: %d, ret1.x:: %d, ret1.y:: %d, ret1.width:: %d, ret1.height:: %d\n",vsExp1->cam_id, ret1.x,ret1.y,ret1.width,ret1.height);

				}


				if(vsExp2->cam_id == 2){
					//project two coordinates diagonally a part in face frame into Iris
					ptr3.x = projectPoints(projFace.x, constantMainR.x, magOffMainR);
					ptr3.y = projectPoints(projFace.y, constantMainR.y, magOffMainR);

					ptr4.x = projectPoints((face.x+face.width), constantMainR.x, magOffMainR);
					ptr4.y = projectPoints((projFace.y+projFace.height), constantMainR.y, magOffMainR);


					//check extreme conditions
					if (ptr3.x > 1200){
						ptr3.x = 1200;
					}
					if (ptr4.y > 960){
						ptr4.y = 960;
					}

					if (ptr3.x < 0){
						ptr3.x = 0;
					}
					if (ptr4.y < 0){
						ptr4.y = 0;
					}

					//create RECT
					ret2.x = ptr3.x;
					ret2.y = ptr3.y;
					ret2.width = abs(ptr3.x - ptr4.x);
					ret2.height = abs(ptr3.y - ptr4.y);

					//check extreme conditions
					if(ret2.width > 1200)
						ret2.width = 1200;
					if(ret2.height > 960)
						ret2.height = 960;

					//Offset correction if useOffset is true
					if(useOffest_m){
						ret2.height = ret2.height - int(projOffset_m);
					}

					if (showProjection){
						retd2.x = ptr3.x/scale;
						retd2.y = ptr3.y/scale;
						retd2.width = abs(ptr3.x - ptr4.x)/scale;


						if (useOffest_m)
							retd2.height = (abs(ptr3.y - ptr4.y)-projOffset_m)/scale;
						else
							retd2.height = abs(ptr3.y - ptr4.y)/scale;

					}

					printf("Camera ID::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",vsExp2->cam_id, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
					printf("Camera ID::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",vsExp2->cam_id, ret2.x,ret2.y,ret2.width,ret2.height);

					}
				else if(vsExp2->cam_id == 130){
					//project two coordinates diagonally a part in face frame into Iris
					ptr3.x = projectPoints(projFace.x, constantAuxR.x, magOffAuxR);
					ptr3.y = projectPoints(projFace.y, constantAuxR.y, magOffAuxR);

					ptr4.x = projectPoints((face.x+face.width), constantAuxR.x, magOffAuxR);
					ptr4.y = projectPoints((projFace.y+projFace.height), constantAuxR.y, magOffAuxR);

					//check extreme conditions
					if (ptr3.x > 1200){
						ptr3.x = 1200;
					}
					if (ptr4.y > 960){
						ptr4.y = 960;
					}

					if (ptr3.x < 0){
						ptr3.x = 0;
					}
					if (ptr4.y < 0){
						ptr4.y = 0;
					}

					//create RECT
					ret2.x = ptr3.x;
					ret2.y = ptr3.y;
					ret2.width = abs(ptr3.x - ptr4.x);
					ret2.height = abs(ptr3.y - ptr4.y);

					//check extreme conditions
					if(ret2.width > 1200)
						ret2.width = 1200;
					if(ret2.height > 960)
						ret2.height = 960;

					//Offset correction if useOffset is true
					if(useOffest_a){
						ret2.height = ret2.height - int(projOffset_a);
					}

					if (showProjection){
						retd2.x = ptr3.x/scale;
						retd2.y = ptr3.y/scale;
						retd2.width = abs(ptr3.x - ptr4.x)/scale;
						retd2.height = abs(ptr3.y - ptr4.y)/scale;
					}

					if (useOffest_a)
						retd2.height = (abs(ptr3.y - ptr4.y)-projOffset_a)/scale;
					else
						retd2.height = abs(ptr3.y - ptr4.y)/scale;

					printf("Camera ID::: %d, ptr3.x:: %d, ptr3.y:: %d, ptr4.x:: %d, ptr4.y:: %d\n",vsExp2->cam_id, ptr3.x,ptr3.y,ptr4.x,ptr4.y);
					printf("Camera ID::: %d, ret2.x:: %d, ret2.y:: %d, ret2.width:: %d, ret2.height:: %d\n",vsExp2->cam_id, ret2.x,ret2.y,ret2.width,ret2.height);

				}



				CvFont font;
				double hScale=.5;
				double vScale=.5;
				int    lineWidth=1;

				int s_camId = vsExp1->cam_id;
				sprintf(tempI1, "Iris left cam");
				//sprintf(tempI1, "CAM %2x %s",s_camId,s_camId&0x80 ?  "AUX":"MAIN");
				//cvPutText (outImage1,drawComm,cvPoint(20,50), &font, cvScalar(255,255,255));
				//cvPutText ((unsigned char*)imgIS1,tempI1,cvPoint(25,100), &font, cvScalar(0,0,0));

				int s_camIdd = vsExp2->cam_id;
				sprintf(tempI2, "Iris Right cam");
				//sprintf(tempI1, "CAM %2x %s",s_camId,s_camId&0x80 ?  "AUX":"MAIN");
				//sprintf(tempI2, "CAM %2x %s",s_camIdd,s_camIdd&0x80 ?  "AUX":"MAIN");
				//cvPutText ((unsigned char*)imgIS2,tempI1,cvPoint(25,100), &font, cvScalar(0,0,0));



				if (showProjection){

					cv::rectangle(imgCopy, no_move_area, Scalar(255, 0, 0), 1, 0);
					cv::rectangle(imgCopy, faceP, Scalar(255, 0, 0), 1, 0);

					//scaling in a smaller image

					Mat imgl, imgR;
					cv::resize(imgIS1, imgl, cv::Size(), (1 / scale),(1 / scale), INTER_NEAREST);
					cv::resize(imgIS2, imgR, cv::Size(), (1 / scale),(1 / scale), INTER_NEAREST);

					cv::rectangle(imgl, retd1, Scalar(255, 0, 0), 2, 0);
					cv::rectangle(imgR, retd2, Scalar(255, 0, 0), 2, 0);

					imshow(temp, imgCopy);
					imshow(tempI1, imgl);
					imshow(tempI2, imgR);


				}
			}

		}
*/


		if(bShowFaceTracking){
			//printf("face x = %i  face y = %i face width = %i  face height = %i\n",face.x, face.y, face.width, face.height);
			sprintf(temp, "Debug facetracker Window\n");
			EyelockLog(logger, DEBUG, "Imshow");
			cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
			cv::rectangle(smallImg, face, Scalar(255,0,0),1,0);
			imshow(temp, smallImg);
		}



//For debug session
#ifdef DEBUG_SESSION
	if (bDebugSessions)
	{
		if (switchedToIrisMode)
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
				sprintf(filename, "%s/FaceImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, FrameNo, CamId);

				imwrite(filename, smallImg);

				char logmsg[300];
				sprintf(logmsg, "Saved-FaceImage-FrNum%d-CamID%d-%s", FrameNo, CamId, filename);
				LogSessionEvent(logmsg);
			}
		}
	}
#endif

#ifdef tempRecord
	pthread_create(&threadIdtemp,NULL,DoTemperatureLog,NULL);
	// DoTemperatureLog();
#endif



#ifdef Tempering
	{
		pthread_create(&threadIdtamper,NULL,DoTamper,NULL);
		// DoTamper();
	}
#endif

}




//Measure noise from images
void MeasureSnr() {
	EyelockLog(logger, TRACE, "MeasureSnr");
	Mat s1;
	static int once = 0;
	if (once == 10) {
	Scalar s2 = sum(outImg);
	float pixels = outImg.cols * outImg.rows;
	double avg = s2.val[0] / pixels;
	EyelockLog(logger, DEBUG, "a = %3.3f\n", avg);

	} else {
		if (once == 0) {
			outImg.convertTo(outImg1, CV_32FC1);
			//outImg.copyTo(outImg1);
		} else {
				Mat cc;
				outImg.convertTo(cc, CV_32FC1);
				outImg1 = outImg1 + cc;
		}
			once++;
			if (once == 10) {
				outImg1 = outImg1 / 10;
				outImg1.convertTo(outImg1s, CV_8UC1);
				Scalar av = mean(outImg1s);
				outImg1s = outImg1s + 10 - av.val[0];
				imshow("offs", outImg1s * 10);
		}
	}

	outImg.copyTo(outImgLast);

}


#define minCalBrightness 40		//orignal 40
#define maxCalBrightness 220

//Individual camera's offset correction
void DoImageCal(int cam_id_ignore)
{
	EyelockLog(logger, TRACE, "Inside DoImageCal");
    int w,h;
	char temp[100];
   // vid_stream_get(&w,&h,(char *)outImg.data);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	Scalar a;
    int  cam_id =(int)outImg.at<char>(0,2)&0xff;
    EyelockLog(logger, DEBUG, "Calibrating camera %x\n",cam_id);
    for (int current=1;current<MAX_CAL_CURENT;current++)
    	{
    	sprintf(temp,"psoc_write(5,%d)",current);
    	EyelockLog(logger, DEBUG, "Sending %s>\n",temp);
    	port_com_send(temp);
    	vs->get(&w,&h,(char *)outImg.data);
    	vs->get(&w,&h,(char *)outImg.data);

    	a=mean(outImg);
    	EyelockLog(logger, DEBUG, "Mean is %f\n",a.val[0]);
    	if (a.val[0]>minCalBrightness)
    		break;
    	}
    if (a.val[0]<minCalBrightness)
    {
    	EyelockLog(logger, DEBUG, "Error Not enough light");
    	exit(0);
    }
    if (a.val[0]>maxCalBrightness)
       {
    	EyelockLog(logger, DEBUG, "Too much light");
       	exit(0);
       }
	outImg.convertTo(outImg1,CV_32FC1 );
    for (int x=1; x< NUM_AVG_CAL;x++)
    {
		 Mat cc;
		 	vs->get(&w,&h,(char *)outImg.data);
		 	//vid_stream_get(&w,&h,(char *)outImg.data);
		outImg.convertTo(cc,CV_32FC1 );
		outImg1=outImg1+cc;
		EyelockLog(logger, DEBUG, "collecting image %d / %d\n",x,NUM_AVG_CAL);

    }
	outImg1=outImg1/NUM_AVG_CAL;
	outImg1.convertTo(outImg1s,CV_8UC1);
	Scalar av = mean(outImg1s);
	outImg1s=outImg1s+10-av.val[0];
	imshow("Imagex10 ",outImg1s*10);

	// save the image
	sprintf(temp,"cal%02x.pgm",cam_id);
	imwrite(temp,outImg1s);
	sprintf(temp,"cal%02xx10.pgm",cam_id);
	//imwrite(temp,outImg1s*10);

	sprintf(temp,"cal%02x.bin",cam_id);
	FILE *f = fopen(temp, "wb");
	if (f)
	{
	int length = outImg1s.cols*outImg1s.rows;
	fwrite(outImg1s.data, length, 1, f);
	fclose(f);
	}
	else
		EyelockLog(logger, DEBUG, "cant write output file %s\n",temp);
	printf("All done press q to quit\n");
	while (1)
		{
		char c=cvWaitKey(200);
		if (c=='q')
			return;
		}
}

//Run offset correction for all cameras
void CalAll()
{
	EyelockLog(logger, TRACE, "CalAll");
	DoStartCmd();
	port_com_send("set_cam_mode(0x83,100");
	port_com_send("psoc_write(3,3)");

	EyelockLog(logger, DEBUG, "CalAll AUX Cameras");
	vs = new VideoStream(8192);
	DoImageCal(0);
	delete (vs);

	vs = new VideoStream(8193);
	DoImageCal(0);
	delete (vs);

	EyelockLog(logger, DEBUG, "CalAll Main Cameras");
	port_com_send("set_cam_mode(0x3,100");
	vs = new VideoStream(8192);
	DoImageCal(0);
	delete (vs);

	vs = new VideoStream(8193);
	DoImageCal(0);
	delete (vs);

}

//Functions require for camera to camera geomatric calibration

#ifdef CAMERACALIBERATION_ARUCO

//Detect ARUCO markers
std::vector<aruco::Marker> gridBooardMarker(Mat img, int cam, bool calDebug){
	//VideoCapture inputVideo;
	//inputVideo.open(0);

/*	//for saving values
	ofstream calScore;
	string path = "/home/eyelock/calVal.csv";
	calScore.open(path, ios::out | ios::app);*/


	int imgCount = 0;
	char c;


	//printf("Inside gridBooardMarker\n");
    int w,h;

	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	int portNum = vs->m_port;
	Mat imgCopy, imgCopy1;

	//float scale = 1.0;
	if (portNum == 8194){
		//cv::resize(outImg, smallImgBeforeRotate, cv::Size(),(1.0/scale),(1.0/scale), INTER_NEAREST);	//Mohammad
		smallImg = rotation90(outImg);

		smallImg.copyTo(imgCopy);
	}
	else{
		outImg.copyTo(imgCopy);
	}

	aruco::MarkerDetector mDetector;
	aruco::MarkerDetector::MarkerCandidate mCandidate;
	mDetector.setDictionary("ARUCO_MIP_36h12");
	std::vector<aruco::Marker> markers;

	char buffer[512];
	imgCopy.copyTo(imgCopy1);

#if 1
	if(!img.empty()){
		if (vs->cam_id == 4){
			//equalizeHist( imgCopy, imgCopy );
			//threshold( imgCopy, imgCopy, 10, 255,THRESH_BINARY);
		}
		else{
			threshold( imgCopy, imgCopy, thresholdVal, 255,THRESH_BINARY);
		}
		markers = mDetector.detect(imgCopy);
		//cv::imshow("streaming without marker", imgCopy);

		if (markers.size() < 2){
			printf("%i camera detected %i markers!\n", portNum, markers.size());

			if(calDebug){
				cv::imshow("No or only 1 marker detected", imgCopy);
				c=cvWaitKey(200);
				if (c=='q')
					printf("Continue!\n");
				//sprintf(buffer, "No_marker_detect%i.png", portNum);
				//imwrite(buffer, imgCopy);
			}
			return markers;
			//exit(EXIT_FAILURE);
		}

		for(size_t i = 0; i < markers.size(); i++){
			//cout << markers[i] << endl;

/*			//for saving values
			cout << "IDs ::: " << markers[i].id << "    center  ::: " << markers[i].getCenter() << endl;
			calScore << markers[i].id << ","<< markers[i].getCenter().x << "," << markers[i].getCenter().y << endl;
*/

			//markers[i].draw(imgCopy);		//uncomment to check binary image
			markers[i].draw(imgCopy1);

		}

		//calScore << cam << endl;
		//namedWindow("marker Detects", WINDOW_NORMAL);
		//sprintf(buffer, "imgAruco%i.png", portNum);

		//cv::imshow("marker Detects", imgCopy);		//uncomment to check binary image

		//cout << "Inside marker detect calDebug :::: " << calDebug << endl;
		if(calDebug){
			char cmd[500];
			cv::imshow("<<< Detecting Markers >>> ", imgCopy1);
			sprintf(cmd,"detectedMarkerCam%i.bmp", vs->cam_id);
			cv::imwrite(cmd,imgCopy1);
			printf(cmd);
			c=cvWaitKey();
/*			if (c=='q')
				printf("Continue!\n");*/
			//imwrite(buffer, imgCopy);
		}

	}
	else{
		printf("There is no Image to detect Aruco-markers!!!\n");
		exit(EXIT_FAILURE);

	}


/*

	//comment the following lines it you want continue streaming and finish calibration!
	while (1)
	{
		c=cvWaitKey(200);
		if (c=='q')
			return markers;
		if (c == 's'){
			char fName[50];
			sprintf(fName,"%d_%d.pgm",portNum,imgCount++);
			cv::imwrite(fName,imgCopy);
			printf("savedAruco %s\n",fName);

		}
	}

*/


#endif

}


//Find rect points from detected ARUCO markers
vector<float> calibratedRect(std::vector<aruco::Marker> markerIris, std::vector<aruco::Marker> markerFace){
	std::vector<cv::Point2f> pointsIris, pointsFace;
	vector<float> rectResult;
	int row = outImg.rows;
	int col = outImg.cols;
	int count = 0;


	//Search max distance between points and collect marker Id which has max distact between them
	cv::Point2f ptr1, ptr2;
	std::vector<int> targetID;
	float maxDis = 0.0;
	int id;
	float m_offset, constantX, constantY; //magnification offset



	//Sorting out IDs that support two point condition
	// two points need to have max Euclidean distance and diagonally a part from each other
	for(int i = 0; i < markerIris.size(); i++){

		ptr1 = markerIris[i].getCenter();

		for(int j = i + 1; j < markerIris.size(); j++){

			ptr2 = markerIris[j].getCenter();
			//cout << markerIris[i].id << " VS " << markerIris[j].id << endl;

			if (std::abs(float(ptr1.y - ptr2.y)) < 20 || std::abs(float(ptr1.x - ptr2.x)) < 20){
				continue;
			}
			else{
				float dis = sqrt(pow((ptr1.x - ptr2.x),2) + pow((ptr1.y - ptr2.y),2));

				if (dis > maxDis){
					maxDis = dis;
					if(targetID.empty())
					{
						targetID.push_back(markerIris[i].id);
						targetID.push_back(markerIris[j].id);
					}
					else{
						targetID.pop_back();
						targetID.pop_back();
						targetID.push_back(markerIris[i].id);
						targetID.push_back(markerIris[j].id);
					}
				}
				//cout << "Dist ::::::::: " << dis << endl;
			}
		}
	}



	//Search target ID's center in iris camera
	for (int i = 0; i < targetID.size(); i++){
		id = targetID[i];
		for (int j = 0; j < markerIris.size(); j++){
			int idIris = markerIris[j].id;
			//pointsExp.push_back(markerIris[j].getCenter());
			if (id == idIris){
				//pointsExp.push_back(markerIris[j].getCenter());
				pointsIris.push_back(markerIris[j].getCenter());
			}
		}
	}


	//Search target ID's center in Face camera
	for (int i = 0; i < targetID.size(); i++){
		int id = targetID[i];
		for (int j = 0; j < markerFace.size(); j++){
			int idFace = markerFace[j].id;
			//pointsExp.push_back(markerFace[j].getCenter());
			if (id == idFace){
				//pointsExp.push_back(markerFace[j].getCenter());
				pointsFace.push_back(markerFace[j].getCenter());
			}
		}
	}



	//check whether it was successfully detected atleast two target points from both camera
	if (pointsIris.size() <= 1 || pointsFace.size() <= 1){
		printf("Fail to detect two common aruco markers with maximum Diagonal "
				"Distance in Both iris and face camera!\n");
		printf("Running the calibration again----->>>>>\n\n\n\n\n");
		return rectResult;
		exit(EXIT_FAILURE);
	}

	//pointsExp.push_back();

	if (calDebug){
	cout << pointsIris.size() << "-------------" << pointsIris[0].x << "-------------" << pointsIris[0].y << endl;
	cout << pointsIris.size() << "-------------" << pointsIris[1].x << "-------------" << pointsIris[1].y << endl;
	cout << pointsFace.size() << "-------------" << pointsFace[0].x << "-------------" << pointsFace[0].y << endl;
	cout << pointsFace.size() << "-------------" << pointsFace[1].x << "-------------" << pointsFace[1].y << endl;
	cout << endl;
	}



	if (calTwoPoint){
		//calculate the zoom offset or slope
		float mx = abs((pointsFace[1].x - pointsFace[0].x) / (pointsIris[1].x - pointsIris[0].x));
		float my = abs((pointsFace[1].y - pointsFace[0].y) / (pointsIris[1].y - pointsIris[0].y));

		m_offset = (mx + my)/2.0; 		//average Magnification offset

		float constantX = pointsFace[1].x - (mx * pointsIris[1].x);
		float constantY = pointsFace[1].y - (my * pointsIris[1].y);

		//constant = (cx + cy)/2.0;

		if (calDebug){
			cout << "number of face point ::: " << pointsFace.size() << " Number of Iris points :::: " << pointsIris.size() << endl;
			cout << "mx::::: " << mx << "   my:::::" << my << "		m_offset::: "<< m_offset <<endl;
			cout << "cx::::: " << constantX << "   cy:::::" << constantY <<endl;
		}



	}
	else{

		// Here for calculating magnification offset--- It will use all the common co-ordinates between
		int cc = 0;
		float sumIx1=0, sumFx1=0,sumIx2=0, sumFx2=0,multIx1Fx1=0,multIx2Fx2=0,powIx1=0,powFx1=0,powIx2=0,powFx2=0;
		float slopeIFx, slopeIFy;
		Vec4f lineX, lineY;

		for(int i = 0; i < markerIris.size(); i++){
			for(int j = 0; j < markerFace.size(); j++){
				if (markerIris[i].id == markerFace[j].id){
					sumIx1 += markerIris[i].getCenter().x;
					sumFx1 += markerFace[j].getCenter().x;
					multIx1Fx1 += markerIris[i].getCenter().x * markerFace[j].getCenter().x;
					powIx1 += markerIris[i].getCenter().x * markerIris[i].getCenter().x;
					powFx1 += markerFace[j].getCenter().x * markerFace[j].getCenter().x;

					sumIx2 += markerIris[i].getCenter().y;
					sumFx2 += markerFace[j].getCenter().y;
					multIx2Fx2 += markerIris[i].getCenter().y * markerFace[j].getCenter().y;
					powIx2 += markerIris[i].getCenter().y * markerIris[i].getCenter().y;
					powFx2 += markerFace[j].getCenter().y * markerFace[j].getCenter().y;

					cc++;
					break;
				}
			}
		}


		slopeIFx = ( (cc*multIx1Fx1) - (sumIx1 * sumFx1) ) / ( (cc*powIx1) - (sumIx1*sumIx1) );
		slopeIFy = ( (cc*multIx2Fx2) - (sumIx2 * sumFx2) ) / ( (cc*powIx2) - (sumIx2*sumIx2) );


		constantX = ((sumFx1 * powIx1) - (sumIx1*multIx1Fx1)) / ((cc*powIx1) - (sumIx1*sumIx1));
		constantY = ((sumFx2 * powIx2) - (sumIx2*multIx2Fx2)) / ((cc*powIx2) - (sumIx2*sumIx2));

		m_offset = (slopeIFx + slopeIFy)/2.0; 		//average Magnification offset
		//constant = (constantIFx + constantIFy)/2.0;

		if (calDebug){
			cout << "slopeIFx::::: " << slopeIFx << "   slopeIFy:::::" << slopeIFy << "m_offset :::::::: "<< m_offset << endl;
			cout << "constantIFx::::: " << constantX << "   constantIFy:::::" << constantY << endl;
		}

	}




	//use average magnification offset for projecting co-ordinates
	float x1_offset = cvRound(pointsFace[0].x - (m_offset * pointsIris[0].x));
	float y1_offset = cvRound(pointsFace[0].y - (m_offset * pointsIris[0].y));

	float x2_offset = cvRound((m_offset * col) + x1_offset);
	float y2_offset = cvRound((m_offset * row) + y1_offset);


	if (calDebug){
	cout << x1_offset << "*********************" << x2_offset << endl;
	cout << y1_offset << "  **********************   " << y2_offset << endl;
	}


	cout << "successfully calculated co-orinates! \n" << endl;

	rectResult.push_back(x1_offset);
	rectResult.push_back(y1_offset);
	rectResult.push_back(x2_offset);
	rectResult.push_back(y2_offset);
	rectResult.push_back(m_offset);		//common slope for two euqation
	rectResult.push_back(constantX);	//constant for calculating x coordinates
	rectResult.push_back(constantY);	//constant for calculating y coordinates



/*
	//draw a rect to verify the rect
	cv::Point pt1(x_offset, y_offset);
	cv::Point pt2(xMax_offset, yMax_offset);
	smallImg = rotation90(outImg);
	cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255));
	imwrite("imgArucoRect.png", smallImg);*/

	return rectResult;

/*	//draw a rect to verify the rect
	cv::Point pt1(x_offset, y_offset);
	cv::Point pt2(xMax_offset, yMax_offset);
	smallImg = rotation90(outImg);
	cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255));
	imwrite("imgArucoRect.png", smallImg);*/

}


//ADjust brightness during calibration
void brightnessAdjust(Mat outImg, int cam, bool calDebug){
	float p, p_old;
	int w,h;

	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//vs = new VideoStream(8193);
	p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),50);
	//imwrite("b_ad_a8193Img.png",outImg);
	//printf("percentile::: %f\n", p);
	//outImg.release();

	//delete (vs);

	//cout << "Cam ID:::::" << vs->cam_id << endl;
	//int agc_val_cal=5;
	char buff[512];
	int exposure_camID;


/*	vs->cam_id face is 4; main is 1 and 2; Aux is 129 and 130
	for changing exposure cam setting is
	face 4; main 24 and aux 4.*/

	float bThreshold;
	if (cam == 4){
		agc_val_cal = 3;
		bThreshold = 10.00;
		exposure_camID = 4;
	}
	else if (cam == 129 || cam == 130){
		bThreshold = 25.00;
		exposure_camID = 3;
	}
	else if(cam == 1 || cam == 2){
		bThreshold = 15.00;
		exposure_camID = 24;
	}


	while(!(p >= bThreshold)){
		agc_val_cal++;
		sprintf(buff,"wcr(%d,0x3012,%d)",exposure_camID,agc_val_cal);
		//printf(buff);
		//printf("bThreshold:::  %3.3f\n",bThreshold);
		port_com_send(buff);
		p_old = p;

		vs->get(&w,&h,(char *)outImg.data);
		vs->get(&w,&h,(char *)outImg.data);


		p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),50);
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Percentile::: %3.3f Agc value = %d\n",p,agc_val_cal);

		if (calDebug){
			imshow("<<< Adjusting Brightness >>>", outImg);
			cvWaitKey();
		}

		if(agc_val_cal > 26)
		{
			sprintf(buff,"wcr(%d,0x3012,%d)",cam,agc_val_cal + 4);
			port_com_send(buff);
			//printf("Increase brightness");
			break;
		}


	}

	//destroyWindow("<<< Adjusting Brightness >>>");
	printf("Brightness adjustment is completed!\n");

}



//Calculate Final Rect from all three cameras
bool CalCam(bool calDebug){

	char cmd[512];
	char buff[512];
	int w,h;

	//Turn on Alternate cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);


	//Fetching images from Aux right
	vs = new VideoStream(8193);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);


	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id, calDebug);
	printf("Detecting marker of aux 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxRight = gridBooardMarker(outImg,0, calDebug);

	//If detected markers less then 2
	if (markerIrisAuxRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",3,3);
		port_com_send(buff);

		return true;
	}
	delete (vs);



	//Fetching images from Aux left
	vs = new VideoStream(8192);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of aux 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxleft = gridBooardMarker(outImg,0, calDebug);
	if (markerIrisAuxleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",3,3);
		port_com_send(buff);
		return true;
	}
	delete (vs);



	//Turn on Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);

	//Fetching images from Main Right
	vs = new VideoStream(8193);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of main 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainRight = gridBooardMarker(outImg,0, calDebug);
	if (markerIrisMainRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",24,3);
		port_com_send(buff);
		return true;
	}
	delete (vs);

	//Fetching images from Main Left
	vs = new VideoStream(8192);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of main 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainleft = gridBooardMarker(outImg,0, calDebug);
	if (markerIrisMainleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",24,3);
		port_com_send(buff);
		return true;
	}
	delete (vs);


	//Fetching images from Face camera
	vs = new VideoStream(8194);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of Face 8194 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerFace = gridBooardMarker(outImg,0, calDebug);
	if (markerFace.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",4,3);
		port_com_send(buff);
		return true;
	}


	//initializing vectors for co-ordinate calc
	vector<float> rectLeftAux, rectRightAux, rectLeftMain, rectRightMain;

	printf("Processing Right aux camera rect-----------------> \n");
	rectRightAux = calibratedRect(markerIrisAuxRight, markerFace);		//calculating xy cordinate
	if (rectRightAux.empty()){
		delete (vs);
		return true;
	}

	printf("Processing Left aux camera rect-----------------> \n");
	rectLeftAux = calibratedRect(markerIrisAuxleft, markerFace);	//calculating xy cordinate
	if (rectLeftAux.empty()){
		delete (vs);
		return true;
	}

	printf("Processing Right main camera rect-----------------> \n");
	rectRightMain = calibratedRect(markerIrisMainRight, markerFace);	//calculating xy cordinate
	if (rectRightMain.empty()){
		delete (vs);
		return true;
	}

	printf("Processing Left main camera rect-----------------> \n");
	rectLeftMain = calibratedRect(markerIrisMainleft, markerFace);	//calculating xy cordinate
	if (rectLeftMain.empty()){
		delete (vs);
		return true;
	}

	//Aux Left sorting
	float x_offset, y_offset, xMax_offset, yMax_offset;
	x_offset = rectLeftAux[0];
	y_offset = rectLeftAux[1];
	xMax_offset = rectLeftAux[2];
	yMax_offset = rectLeftAux[3];


	cv::Point pt1(x_offset, y_offset);
	cv::Point pt2(xMax_offset, yMax_offset);
	smallImg = rotation90(outImg);
	//For exp purpose
	Mat smallImgX;
	smallImg.copyTo(smallImgX);



	//Aux Right sorting
	x_offset = rectRightAux[0];
	y_offset = rectRightAux[1];
	xMax_offset = rectRightAux[2];
	yMax_offset = rectRightAux[3];

	cv::Point pt3(x_offset, y_offset);
	cv::Point pt4(xMax_offset, yMax_offset);





	//Main Left sorting
	x_offset = rectLeftMain[0];
	y_offset = rectLeftMain[1];
	xMax_offset = rectLeftMain[2];
	yMax_offset = rectLeftMain[3];


	cv::Point pt5(x_offset, y_offset);
	cv::Point pt6(xMax_offset, yMax_offset);


	//Main Right sorting
	x_offset = rectRightMain[0];
	y_offset = rectRightMain[1];
	xMax_offset = rectRightMain[2];
	yMax_offset = rectRightMain[3];

	cv::Point pt7(x_offset, y_offset);
	cv::Point pt8(xMax_offset, yMax_offset);





	if (calDebug){
		cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, pt3, pt4, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, pt5, pt6, cv::Scalar(0, 255, 0), 3);
		cv::rectangle(smallImg, pt7, pt8, cv::Scalar(0, 255, 0), 3);
		imwrite("MarkerRucoDetectofLeftRightAUX_Main.png", smallImg);
		imshow("Each IrisCam projected in Face cam",smallImg);
		cvWaitKey();
	}

	delete (vs);



	//Data saved for Aux Rect
	int x, y, width, height;
	float m_offset_mr, m_offset_ml, m_offset_ar, m_offset_al;
	x = 0; y = pt1.y; width = int(smallImg.cols); height = int(abs(pt1.y - pt4.y));


	//check for negative value
	if (y < 0 || height < 0)
		return true;


	Rect auxRect(x, y,width, height);
	//cout << x << "    " << y << "    " << width << "    " << height << "    " <<endl;

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");


	//Writing Calibrated Rect
	stringstream ssI;
	string ssO;

	ssI << x;
	ssI >> ssO;
	fconfig.setValue("FTracker.targetRectX",ssO.c_str());
	if(calDebug){
		printf("x:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << y;
	ssI >> ssO;
	fconfig.setValue("FTracker.targetRectY",ssO.c_str());
	if(calDebug){
		printf("y:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << width;
	ssI >> ssO;
	fconfig.setValue("FTracker.targetRectWidth",ssO.c_str());
	if(calDebug){
		printf("width:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << height;
	ssI >> ssO;
	fconfig.setValue("FTracker.targetRectHeight",ssO.c_str());
	if(calDebug){
		printf("height:: %s\n", ssO.c_str());
	}


	//Writing magnification offset
	ssI.clear();
	ssI << rectRightAux[4];
	ssI >> ssO;
	fconfig.setValue("FTracker.magOffsetAuxRightCam",ssO.c_str());

	ssI.clear();
	ssI << rectLeftAux[4];
	ssI >> ssO;
	fconfig.setValue("FTracker.magOffsetAuxLeftCam",ssO.c_str());

	ssI.clear();
	ssI << rectRightMain[4];
	ssI >> ssO;
	fconfig.setValue("FTracker.magOffsetMainRightCam",ssO.c_str());

	ssI.clear();
	ssI << rectLeftMain[4];
	ssI >> ssO;
	fconfig.setValue("FTracker.magOffsetMainLeftCam",ssO.c_str());


	//Writing reference Marker points
	ssI.clear();
	ssI << float(rectRightAux[5]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantAuxRightCam_x",ssO.c_str());
	//printf("constantAuxRightCam_x:: %s		%3.3f\n", ssO.c_str(), rectRightAux[5]);

	ssI.clear();
	ssI << float(rectRightAux[6]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantAuxRightCam_y",ssO.c_str());
	//printf("constantAuxRightCam_y:: %s		%3.3f\n", ssO.c_str(), rectRightAux[6]);


	ssI.clear();
	ssI << float(rectLeftAux[5]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantAuxLeftCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftAux[6]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantAuxLeftCam_y",ssO.c_str());

	ssI.clear();
	ssI << float(rectRightMain[5]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantMainRightCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectRightMain[6]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantMainRightCam_y",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftMain[5]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantMainLeftCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftMain[6]);
	ssI >> ssO;
	fconfig.setValue("FTracker.constantMainLeftCam_y",ssO.c_str());


	fconfig.writeIni("/home/root/data/calibration/faceConfig.ini");

	printf("Finished calibration process------------------->\n");
	printf("Finish writing all the values in faceConfig.ini\n");

/*	//saving Aux Rect info ---> This is the Rect info we will use for face Tracking
	ofstream auxfile("auxRect.csv");
	auxfile << x << endl;
	auxfile << y << endl;
	auxfile << width << endl;
	auxfile << height << endl;
	auxfile.close();*/



	//Data saved for Main Rect
	x = 0; y = pt5.y; width = int(smallImg.cols); height = int(abs(pt5.y - pt8.y));
	Rect mainRect(x, y,width, height);

/*	//Saving Main Rect Info
	ofstream mainfile("mainRect.csv");
	mainfile << 0 << endl;
	mainfile << int(pt7.y) << endl;
	mainfile << int(smallImg.cols) << endl;
	mainfile << int(abs(pt7.y - pt6.y)) << endl;
	mainfile.close();*/


	//cv::rectangle(smallImgX, mainRect,cv::Scalar( 0, 255, 0 ), 4);
	if (calDebug){
		cv::rectangle(smallImgX, auxRect,cv::Scalar( 255, 255, 255 ), 4);
		imwrite("MarkerRucoDetectofLeftRightAUX_Main_testRect.png", smallImgX);
		imshow("Projected Target in Face cam", smallImgX);
		cvWaitKey();

	}

	return false;
}



//Run calibration until meet all the condition based on brightness changes and motor movement
void runCalCam(bool calDebug){
	bool check = true;
	//step = 5;		//initialize increment step

	int newPos = MIN_POS + startPoint;
	//printf(">>>>>>>>>>>>>>>>>>>>>>New Pos ::: %i, MIN_POS ::: %i, startPoint ::: %i   \n",newPos, MIN_POS, startPoint);
	char cmd[512];

	//cout << "Caibration Debug mode::::" << calDebug << endl;


/*	double id;
	cout << "Input the device ID::::: ";
	cin >> id;

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	stringstream idss;
	string ids;

	idss << id;
	idss >> ids;


	printf("Device ID is:: %s\n", ids.c_str());
	fconfig.setValue("FTracker.uintID",ids.c_str());

	fconfig.writeIni("/home/root/data/calibration/faceConfig.ini");*/


	while(check){
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>Motor is moving to %i and conducting calibration\n", newPos);
		sprintf(cmd, "fx_home()\n");
		usleep(10000);
		sprintf(cmd, "fx_abs(%i)\n", newPos);
		port_com_send(cmd);
		//MoveTo(newPos);
		usleep(10000);
		//printf(">>>>>>>>>>>>>>>>>>>>>>New Pos ::: %i, step ::: %i, Max Pos ::: %i   \n",newPos, step, MAX_POS);
		//printf(cmd);
		//printf("------------------------------------------------------------------>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		check = CalCam(calDebug);	// Rum calibration, return false if fail
		newPos = newPos + step;

		//If Motor reach to max position and failt to calibrate
		if (newPos > MAX_POS){
			printf("Fail Camera to Camera geometric calibration process due to no good images!!! \n");
			check = false;
		}
	}

}

#endif

//Parsing int and convertng into Hex
double parsingIntfromHex(string str1){
	EyelockLog(logger, TRACE, "parsingIntfromHex");

    int loc1 = str1.find_first_of('=');

    string str2 = str1.substr(loc1+1,str1.length());

    int loc2 = str2.find_first_of('(');
    string str3 = str2.substr(0,loc2);
    //cout << str3 << endl;

    unsigned int intVal;
    std::stringstream ss;
    ss << std::hex << str3;
    ss >> intVal;
    //cout << intVal << endl;

    return intVal;
}

//Read data from temp sensor---> Needs to be updated based on new firmware
int calTemp(int i)
{
	EyelockLog(logger, TRACE, "calTemp");
    int len;
    char buffer[512];
    char cmd[512];
	float t_start=clock();

    //Temp reading commands from camera sensor
	sprintf(cmd, "wcr(0x0%i,0x30b4,0x01)", i);
	port_com_send(cmd);
    //port_com_send("wcr(0x1f,0x30b4,0x01)");
    //cvWaitKey(3000);
	sprintf(cmd, "wcr(0x0%i,0x30b4,0x31)", i);
	port_com_send(cmd);
    //port_com_send("wcr(0x1f,0x30b4,0x31)");
    //cvWaitKey(3000);
	sprintf(cmd, "wcr(0x0%i,0x30b4,0x11)", i);
	port_com_send(cmd);
    //port_com_send("wcr(0x1f,0x30b4,0x11)");
    //cvWaitKey(300);
	sprintf(cmd, "rcr(0x0%i,0x30b2)", i);
    if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
    {
    	EyelockLog(logger, DEBUG, "failed to read temp register!\n");
    }
    //cvWaitKey(3000);
    //printf("Temp val: %s\n", buffer);
    string str1(buffer);

    double tempRead = parsingIntfromHex(str1);

	sprintf(cmd, "rcr(0x0%i,0x30cc)", i);
    //70C calibration data read
    if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
    {
    	EyelockLog(logger, DEBUG, "failed to read temp register!\n");
    }
    //printf("Temp val 120: %s\n", buffer);
    string str2(buffer);
    double cal120 = parsingIntfromHex(str2);

    sprintf(cmd, "rcr(0x0%i,0x30c8)", i);
    //55C calibration data read
    if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
    {
    	EyelockLog(logger, DEBUG, "failed to read temp register!\n");
    }
    //printf("Temp val 50: %s\n", buffer);
    string str3(buffer);
    double cal55 = parsingIntfromHex(str3);

    double slope = (120.0 - 55.0)/(cal120 - cal55);
    double constant = 55.0 - (slope * double(cal55));

    double tempInC = (slope * tempRead)  + constant;

	float t_result = (float)(clock()-t_start)/CLOCKS_PER_SEC;
	//cout << tempRead << " " << cal120 << " " << cal55 << endl;
	//cout << "Slope::: " << slope << "Constant:::" << constant << endl;
    cout << "TempData of " << i << " Cam:::" << tempInC << endl;


	sprintf(cmd, "wcr(0x0%i,0x30b4,0x21)", i);
    port_com_send(cmd);


    return tempInC;

}

//Motor Move for each temp measure
void motorMove(){
	EyelockLog(logger, TRACE, "motorMove");
	int temp;
	int WAIT = 800;

/*    MoveTo(CENTER_POS);
    temp = calTemp();*/

	EyelockLog(logger, DEBUG, "\nStart temp expermiment process-----------------> \n");
	char cmd[512];
    while(1){
        MoveTo(MAX_POS);
        cvWaitKey(WAIT);
        //sprinf(cmd, "")
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        //writeStartNewLine(fileName);

        MoveTo(CENTER_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        //writeStartNewLine(fileName);

        MoveTo(MIN_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        //writeStartNewLine(fileName);

        MoveTo(CENTER_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        //writeStartNewLine(fileName);

    }
}




void RunCamFocus(){

	int w,h;
	char key;

	bool quit = false;

	fs = new extFocus();

	char cmd[512];

	//Set AUX cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);


	//Fetching images from Face camera
	int leftCam = 8192;
	int rightCam = 8193;

	VideoStream *vs;
	vs = new VideoStream(leftCam);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);

		if(quit)
			break;
	}
	delete(vs);
	delete(fs);


	fs = new extFocus();

	vs = new VideoStream(rightCam);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);

		if(quit)
			break;
	}
	delete(vs);
	delete(fs);

	//Set Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(1000);


	fs = new extFocus();
	vs = new VideoStream(leftCam);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);

		if(quit)
			break;
	}
	delete(vs);
	delete(fs);


	fs = new extFocus();
	vs = new VideoStream(rightCam);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);

		if(quit)
			break;
	}
	delete(vs);
	delete(fs);

}





int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");
	bool bShowFaceTracking = fconfig.getValue("FTracker.ShowFaceTracking", false);
#ifdef DEBUG_SESSION
	bool bDebugSessions = fconfig.getValue("FTracker.DebugSessions",false);

	eyelockConf = FileConfiguration("/home/root/Eyelock.ini");
	m_sessionDir = string(eyelockConf.getValue("Eyelock.DebugSessionDir","DebugSessions/Session"));
	m_sessionInfo = m_sessionDir + "/Info.txt";
#endif

/*	// can be used for saving temp data
	fstream infile(fileName);
	if(infile.good()){
		// cout << "File exist and keep writing in it!" << endl;
		EyelockLog(logger, DEBUG, "File exist and keep writing in it");
	}
	else{
		EyelockLog(logger, DEBUG, "Create Log file");
		// cout << "Create log file!" << endl;
		ofstream file(fileName);
	}*/


    int w,h;
    char key;


    int run_mode;
    int cal_mode=0;				//initialize image optimization
    int cal_cam_mode = 0;		//initializing camera to camera geometric calibration
    int temp_mode = 0;
    int focusMode = 0;
    pthread_t threadId;
    pthread_t thredEcId;



	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);

	//outImg2 = Mat(Size(WIDTH,HEIGHT), CV_8U);
#if 0
	if (argc<2)
	{
		EyelockLog(logger, DEBUG, "error params");
		// printf("error params\n");
		exit (0);
	}
#endif
	//check device ID with user in
	float id = 0.0;
/*	cout << "Give device ID:::: " << endl;
	cin >> id;

	float idF = fconfig.getValue("FTracker.uintID",0.0f);

	//printf("Device ID:: %3.3f,    Calibration ID :: %3.3f \n", id, idF);

	if (id != idF)
	{
		printf("Device ID:: %3.3f,    Calibration ID :: %3.3f \n", id, idF);
		EyelockLog(logger, DEBUG, "Calibration settings with The device ID doesn't match");
		printf("Calibration settings with The device ID doesn't match\n");
		exit (0);
	}*/

	//Check argc for run face tracker
#if 0
	if (argc== 3)
		run_mode=1;
	else
		run_mode =0;
#endif

	run_mode=1;  //DMO forced

#if 0 //DMOOUT - No more cmd line stuff always just run in FaceTracking only mode...

	//Image optimization
	if (strcmp(argv[1],"cal")==0)
	{
		run_mode =1;
		cal_mode=1;
	}

	//Send raw face/Eyecrop images
	if (strcmp(argv[1],"send")==0)
	{
		void SendUdpImage(int port, char *image, int bytes_left);
		Mat imageIn, image;
		int x;
		Mat sendImg;

		sendImg = Mat(Size(WIDTH,HEIGHT), CV_8U);


		image=imread(argv[2],0);
		image.convertTo(image,CV_8UC1);

		EyelockLog(logger, DEBUG, "sending udp  %d  %dbytes \n",image.cols*image.rows,image.dims);

		image.copyTo(sendImg(cv::Rect(0,0,image.cols, image.rows)));

		for (x=0; x<atoi(argv[3]);x++)
				{
					EyelockLog(logger, DEBUG, "send bad\n");
					SendUdpImage(8192, (char *)sendImg.data, sendImg.cols*sendImg.rows);
					usleep(40000);
				}
        exit(1);
		image=imread("send.pgm",0);
		image.convertTo(image,CV_8UC1);
		EyelockLog(logger, DEBUG, "sending udp  %d  %dbytes \n",image.cols*image.rows,image.dims);

		for (x=0; x< atoi(argv[3]);x++)
		{
			EyelockLog(logger, DEBUG, "send good\n");
			SendUdpImage(8192, (char *)image.data, image.cols*image.rows);
			usleep(40000);
		}
		return 0;

	}
	if (strcmp(argv[1],"play")==0)
	{
		// char *dir = argv[2];
		//int imageid = atoi(argv[3]);

		//char filename[100];

		printf("argc: %d\n", argc);
		printf("argv 0: %s\n", argv[0]);
		printf("argv 1: %s\n", argv[1]);
		printf("argv 2: %s\n", argv[2]);

		  for(int i = 2; i < argc-2; i++){
			// sprintf(filename, "InputImage_%d.pgm", imageid++);
			void SendUdpImage(int port, char *image, int bytes_left);
			// Mat imageIn, image;
			// int x;
			printf("filename...%s\n", argv[i]);
			IplImage *image = cvLoadImage(argv[i], CV_LOAD_IMAGE_GRAYSCALE);
			// image=imread(filename,0);
			if(image){
				//int camId = image->imageData[2]&0xff;
				//char camIdText[50];
				//sprintf(camIdText, "CamID: %d\n", camId);
				// // void cvPutText(CvArr* img, const char* text, CvPoint org, const CvFont* font, CvScalar color)
				// // example: putText(result, "Differencing the two images.", cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
				//putText(image, camIdText, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, cvScalar(200,200,250));

				// cvShowImage("test11", image);
				// cvWaitKey(1000);
				// image.convertTo(image,CV_8UC1);
				// EyelockLog(logger, DEBUG, "sending udp  %d  %dbytes \n",image.cols*image.rows,image.dims);
				SendUdpImage(8192, (char *)image->imageData, image->height * image->width);

				usleep(100000);

			}else{
				printf("No more images to read in the session directory\n");
				break;
			}
		}
#if 0
		image=imread(filename,0);
		image.convertTo(image,CV_8UC1);
		EyelockLog(logger, DEBUG, "sending udp  %d  %dbytes \n",image.cols*image.rows,image.dims);

		for (x=0; x< atoi(argv[3]);x++)
		{
			EyelockLog(logger, DEBUG, "send good\n");
			SendUdpImage(8193, (char *)image.data, image.cols*image.rows);
			usleep(40000);
		}
#endif
		return 0;

	}
	if (strcmp(argv[1], "show") == 0) {
		for (int i = 2; i < argc - 2; i++) {
			void SendUdpImage(int port, char *image, int bytes_left);
			IplImage *image = cvLoadImage(argv[i], CV_LOAD_IMAGE_GRAYSCALE);
			if (image) {
				cvShowImage("Display", image);
				cvWaitKey(1000);
			} else {
				printf("No more images to read in the session directory\n");
				break;
			}
		}
		return 0;

	}

	//Camera to camera geometric calibration
	if (strcmp(argv[1],"calcam")==0)
	{
		EyelockLog(logger, DEBUG, "calcam mode is running");
		run_mode =1;
		cal_cam_mode=1;
	}

	//Run temp test
	if (strcmp(argv[1],"temp")==0)
	{
		EyelockLog(logger, DEBUG, "temperature test mode is running");
		run_mode =1;
		temp_mode=1;
	}
#endif
	if (strcmp(argv[1],"focus")==0)
	{
		focusMode =1;

	}
	//pThread for face tracker active
	if (run_mode)
		pthread_create(&threadId,NULL,init_tunnel,NULL);
	if (run_mode)
		pthread_create(&threadId,NULL,init_ec,NULL);


	//Set environment for Image optimization
	if (cal_mode)
	{
		portcom_start();
		CalAll();
		return 0;
	}


	if(focusMode){
		portcom_start();

		fs->DoStartCmd_focus();
		delete(fs);
		RunCamFocus();

		return 0;

	}


	//Set environment for camera to camera calibration
	if (cal_cam_mode){
		portcom_start();
		DoStartCmd_CamCal();

#ifdef CAMERACALIBERATION_ARUCO
		runCalCam(calDebug);
#endif
		return 0;

	}


	fs = new extFocus();
	if(focusMode){
		portcom_start();

		fs->DoStartCmd_focus();
		delete(fs);
		RunCamFocus();

		return 0;

	}


/*	//Set environment for camera to camera calibration
	if (cal_cam_mode){
		portcom_start();
		DoStartCmd_CamCal();

#ifdef CAMERACALIBERATION_ARUCO
		runCalCam(calDebug);
#endif
		return 0;

	}*/

	//Set environment for temp Test
	if (temp_mode){
		portcom_start();
		//DoStartCmd();
		motorMove();
		return 0;

	}


	//vid_stream_start
#if 0
	vs= new VideoStream(atoi(argv[1]));
	sprintf(temp,"Disp %d",atoi (argv[1]) );
#else
	vs= new VideoStream(8194); //Facecam is 8194...
	sprintf(temp,"Disp %d", 8194);
#endif
	//Setting up Run mode
	if (run_mode)
	{
		EyelockLog(logger, DEBUG, "run_mode");
/*		FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

		double id = fconfig.getValue("FTracker.uintID",0);
		double idm;
		cout << "Input the device ID::::: ";
		cvWaitKey(1);
		cin >> idm;
		cout << "Device ID IS ::::::::::::::::::::::: " << idm << endl;

		//check device ID
		if(id == idm){
			printf("-------------->>>>>>>>>>>>>>>>>>> Device ID didn't match with calibration file!\n");
			exit(EXIT_FAILURE);
		}*/

		face_init();
		portcom_start();
		DoStartCmd();
/*		double hScale=.5;
		double vScale=.5;
		int    lineWidth=1;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);*/

		imgIS1 = Mat(Size(WIDTH,HEIGHT), CV_8U);
		imgIS2 = Mat(Size(WIDTH,HEIGHT), CV_8U);
#if 0
		vsExp1 = new VideoStream(8192);
		vsExp1->flush();


		vsExp2 = new VideoStream(8193);
		vsExp2->flush();
#endif
	}

	//imshow() face tracking streaming
#if 0 // Anita
	if(bShowFaceTracking)
	{
		cv::namedWindow(temp);
	}
#endif

	//Controlling Offset correction while normal streaming (used for testing offset correction)
	if (vs->m_port==8192)
		vs->offset_sub_enable=0;
	if (vs->m_port==8193)
			vs->offset_sub_enable=0;

	int s_canId;
	while (1)
	{
		{
			vs->get(&w,&h,(char *)outImg.data);
			s_canId=vs->cam_id;
		}

		//Main Face tracking operation
		if (run_mode)
			{
			 //DoRunMode(bShowFaceTracking, bDebugSessions);
			 DoRunMode_test(bShowFaceTracking, bDebugSessions);
#if 0
			vsExp1->get(&w,&h,(char *)imgIS1.data);

			vsExp2->get(&w,&h,(char *)imgIS2.data);
#endif


			}
		else
			{
			//portcom_start();
			//DoStartCmd();

			//For testing image optimization (OFFset correction)
			Mat DiffImage = imread("white.pgm",CV_LOAD_IMAGE_GRAYSCALE);
			Mat dstImage;
			if (DiffImage.cols!=0)
			{
				dstImage=outImg-DiffImage;
				cv::resize(dstImage, smallImg, cv::Size(), 1, 1, INTER_NEAREST); //Time debug
				EyelockLog(logger, DEBUG, "sub\n");
			}
			else
				cv::resize(outImg, smallImg, cv::Size(), 1, 1, INTER_NEAREST); //Time debug
			//MeasureSnr();
			{
				char text[10];
				//sprintf(text,"CAM %2x %s %d",s_canId,s_canId&0x80 ?  "AUX":"MAIN",vs->frameId );
				//putText(smallImg,text,Point(10,60), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(255,255,255),2);
				//putText(smallImg,text,Point(10+1,60+1), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(0,0,0),2);

			}
		//	cv::resize(outImg, smallImg, cv::Size(), 0.25, 0.25, INTER_NEAREST); //Time debug
				if(bShowFaceTracking){
					imshow("FaceTracker",smallImg);  //Time debug
					cvWaitKey(1);
				}
			}



	    key = cv::waitKey(1);
	    //printf("Key pressed : %u\n",key);

#if 0 //DMOOUT no more key'd exits...
	    //For quit streaming
		if (key=='q')
			break;


		//For saving images while streaming individual cameras
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%d_%d.pgm",atoi(argv[1]),fileNum++);
			cv::imwrite(fName,outImg);
			printf("saved %s\n",fName);

		}
#endif
	}


}




