
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
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "logging.h"


#define CYCLE_TEST

using namespace cv;
using namespace std::chrono;
using namespace std;

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
int targetOffset;
Rect detect_area(15/SCALE,15/SCALE,(960/(SCALE*SCALE))+15/SCALE,(1200/(SCALE*SCALE))+15/SCALE); //15 was 30
Rect no_move_area;		//Face target area

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
char temp[512], tempI1[512], tempI2[512];
int AGC_Counter = 0;
int noFaceCounter =0;

Mat outImgLast, outImg1, outImg1s;		//Used in MeasureSnr function

//Used as increasing exposure time in brightnessAdjust function,
//mainly used in Camera to Camera Calibration
int agc_val_cal=3;
int step;
int startPoint;
int thresholdVal = 30;
bool calDebug, calTwoPoint;
//variable used for tempering

std::chrono:: time_point<std::chrono::system_clock> start_mode_change;
extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern void  *init_tunnel(void *arg);
extern void *init_ec(void * arg);
extern int IrisFramesHaveEyes();
int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size);

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


double parsingIntfromHex(string str1);

float AGC_average(int width, int height,unsigned char *dsty, int limit);

double StandardDeviation(std::vector<double> samples);
double Variance(std::vector<double> samples);



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

	sprintf(cmd,"set_cam_mode(0x04,%d)",FRAME_DELAY);
	port_com_send(cmd);
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
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Turn on Alternate cameras
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

Mat preProcessingImg(Mat outImg){
	float p;

	EyelockLog(logger, DEBUG, "preProcessing");
	EyelockLog(logger, DEBUG, "resize");
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





#define STATE_LOOK_FOR_FACE 1
#define STATE_MOVE_MOTOR    2
#define STATE_MAIN_IRIS     3
#define STATE_AUX_IRIS      4
#define Tempering
#define tempRecord





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


void DoRunMode_test(bool bShowFaceTracking, bool bDebugSessions){
	pthread_t threadIdtamper;
	pthread_t threadIdtemp;

	EyelockLog(logger, TRACE, "DoRunMode_test");

	int start_process_time = clock();

	smallImg = preProcessingImg(outImg);
	bool foundEyes = FindEyeLocation(smallImg, eyes, eye_size);

	float process_time = (float) (clock() - start_process_time) / CLOCKS_PER_SEC;


	bool eyesInDetect = foundEyes? detect_area.contains(eyes):false;
	bool eyesInViewOfIriscam = eyesInDetect ? no_move_area.contains(eyes):false;


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
	EyelockLog(stateMachineLogger, TRACE, "STATE:%8s  NFC:%2d %c%c%c  I_SIZE:%03.1f  I_POS(%3d,%3d) MV:%3.3f TIME:%3.3f AGC:%5d \n",StateText(system_state),
					noFaceCounter,
				foundEyes?'E':'.',
				eyesInDetect?'D':'.',
				eyesInViewOfIriscam?'V':'.',
						eye_size,
						eyes.x,
						eyes.y,
						last_angle_move,
						process_time,
						agc_set_gain
						);
		/*if (g_MatchState)
			g_MatchState=0;*/
		last_angle_move=0;


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
						break;
					case STATE_AUX_IRIS:
						// enable aux camera and set led currnet
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(false);									//switch cameras
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
						break;
					case STATE_MAIN_IRIS:
						// enable main cameras and set
						DimmFaceForIris();											//Dim face settings
						MainIrisSettings();											//change to Iris settings
						SwitchIrisCameras(true);									//switch cameras
						break;
					}
					break;
	}
#ifndef CYCLE_TEST
	//For dispaying face tracker
	if(bShowFaceTracking){
		sprintf(temp, "Debug facetracker Window\n");
		EyelockLog(logger, DEBUG, "Imshow");
		cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
		imshow(temp, smallImg);
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




int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");
	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");
	bool bShowFaceTracking = fconfig.getValue("FTracker.ShowFaceTracking", false);
	bool bDebugSessions = false;

	int w,h;
    char key;
    int run_mode;

    pthread_t threadId;
    pthread_t thredEcId;

	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);
	if (argc<2)
	{
		EyelockLog(logger, DEBUG, "error params");
		exit (0);
	}

	//Check argc for run face tracker
	if (argc== 3)
		run_mode = 1;
	else
		run_mode =0;

	//pThread for face tracker active
	if (run_mode)
		pthread_create(&threadId,NULL,init_tunnel,NULL);
	/*
	if (run_mode)
		pthread_create(&threadId,NULL,init_ec,NULL);*/

	//vid_stream_start
	vs= new VideoStream(atoi(argv[1]));
	sprintf(temp,"Disp %d",atoi (argv[1]) );


	//Setting up Run mode
	if (run_mode)
	{
		EyelockLog(logger, DEBUG, "run_mode");
		face_init();
		portcom_start();
		DoStartCmd();
	}
	unsigned int count = 0;
	int s_canId;
	while (1)
	{
		{
			vs->get(&w,&h,(char *)outImg.data);
			s_canId=vs->cam_id;
		}
#ifdef CYCLE_TEST
		if(count % 4*5 == 0){ // Every 5 seconds
			char fileName[50];
			sprintf(fileName,"%d.pgm",atoi(argv[1]));
			cv::imwrite(fileName,outImg);
		}
#endif
		//Main Face tracking operation
		if (run_mode)
			{

			 DoRunMode_test(bShowFaceTracking, bDebugSessions);
			}
		else
			{

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
				sprintf(text,"CAM %2x %s",s_canId,s_canId&0x80 ?  "AUX":"MAIN" );
				putText(smallImg,text,Point(10,60), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(255,255,255),2);
				putText(smallImg,text,Point(10+1,60+1), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(0,0,0),2);

			}
		//	cv::resize(outImg, smallImg, cv::Size(), 0.25, 0.25, INTER_NEAREST); //Time debug
#ifndef CYCLE_TEST
		if(bShowFaceTracking)
			imshow("FaceTracker", smallImg);  //Time debug
#endif
		}

	    key = cv::waitKey(1);
	    //printf("Key pressed : %u\n",key);

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
		count++;
	}


}



