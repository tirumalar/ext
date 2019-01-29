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
#include <math.h>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "opencv2/objdetect/objdetect.hpp"

#define CAMERACALIBERATION_ARUCO

#ifdef CAMERACALIBERATION_ARUCO
#include <aruco.h>
#include <dictionary.h>
#endif

#include "portcom.h"
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "logging.h"

#include "extFocus.h"

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

const char logger[30] = "CameraTool";


VideoStream *vs;
Mat outImgI1;

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

int MIN_FACE_SIZE_INTER;
int MAX_FACE_SIZE_INTER;

Point eyes;	// hold eye info from each frame
char temp[512], tempI1[512], tempI2[512], tempI3[512], tempI4[512], tempI5[512];
int AGC_Counter = 0;
int noFaceCounter =0;

Mat outImgLast, outImg1, outImg1s;		//Used in MeasureSnr function

//Used as increasing exposure time in brightnessAdjust function,
//mainly used in Camera to Camera Calibration
int agc_val_cal= 1;
int step;
int startPoint;
int thresholdVal = 30;
bool calDebug, calTwoPoint, projDebug, projPtr = false;;
float projOffset_m, projOffset_a;
bool useOffest_m, useOffest_a;
bool showProjection;

struct coordinateProject{
	int camIDL;
	int frmaeL;
	Rect camLRect;
	int camIDR;
	int frmaeR;
	Rect camRRect;
};

extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern void  *init_tunnel(void *arg);
extern int IrisFramesHaveEyes();

int face_init(  );
float read_angle(void);
void SetExp(int cam, int val);
void MoveToAngle(float a);
void MoveTo(int v);
void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
void SelLedDistance(int val);
char* GetTimeStamp();


void MainIrisSettings();
void SwitchIrisCameras(bool mode);
void SetFaceMode();

void MoveRelAngle(float a);
void SetIrisMode(float CurrentEye_distance);

void DoStartCmd_CamCal();
float AGC(int width, int height,unsigned char *dsty, int limit);
Mat rotation90(Mat src);
int IrisFramesHaveEyes();
void MeasureSnr();	//Measuring noise in images


#ifdef CAMERACALIBERATION_ARUCO
//Camera to Camera Calibration
float projectPoints(float y, float c, float m);
std::vector<aruco::Marker> gridBooardMarker(Mat img, int cam, bool calDebug);
vector<float> calibratedRect(std::vector<aruco::Marker> markerIris, std::vector<aruco::Marker> markerFace);
void brightnessAdjust(Mat outImg, int cam, bool calDebug);
bool CalCam(bool calDebug);
void runCalCam(bool calDebug);
#endif



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



}


int tempTarget; 		//5 mins = 60*5 = 300s
int mask1, mask2, mask3;
int outerFaceRange1,outerFaceRange2, innerFaceRange1, innerFaceRange2;

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


/*	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	port_com_send("fx_home\n");*/

#ifdef NOOPTIMIZE
	usleep(100000);
#endif


	//Reset the lower motion
	//sprintf(cmd, "fx_abs(%i)",MIN_POS);
	//EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", MIN_POS);
	//port_com_send(cmd);
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
	port_com_send("psoc_write(9,80)");	// charge cap for max current 60 < range < 95
	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	// faceCamDataPedestal, faceCamDigitalGain);
	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,2) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0x30)");
	//wcr(0x04,0x3012,6) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0x50)

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

/*
	//Turn on analog gain
	port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	port_com_send("wcr(0x4,0x30b0,0x80");		//Only face camera gain is x90

	port_com_send("wcr(0x1f,0x301a,0x1998)"); // ilya added to leave the pll on always
*/





	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((1&0x3)<<4) | 0X80);
	//EyelockLog(logger, DEBUG, "Iris analog gain: %d", (((1&0x3)<<4) | 0X80));
	port_com_send(cmd);

	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((1&0x3)<<4) | 0X80);
	//EyelockLog(logger, DEBUG, "Face analog gain: %d", (((1&0x3)<<4) | 0X80));
	port_com_send(cmd);
	//port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	//port_com_send("wcr(0x4,0x30b0,0x90");		//Only face camera gain is x90


	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");
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


float eye_size;
float p; //AGC

#define STATE_LOOK_FOR_FACE 1
#define STATE_MOVE_MOTOR    2
#define STATE_MAIN_IRIS     3
#define STATE_AUX_IRIS      4

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
				sprintf(buffer, "%i have un-focus Images and only %i markers detected", portNum, markers.size());
				//cv::imshow("Binary No or only 1 marker detected", imgCopy);
				cv::imshow(buffer, imgCopy1);
				cv::moveWindow(buffer, 20, 20);
				c=cvWaitKey();
				if (c=='q'){
					printf("Continue!\n");
					destroyWindow(buffer);
				}

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
			char cmd[500], cmd1[500];
			//printf("Cam ::: %i\n", cam);
			if (cam == 4){
				sprintf(cmd1, "%i Markers detected in Face camera", markers.size());
				imshow(cmd1, imgCopy1);
				cv::moveWindow(cmd1,20,20);

				//cv::imshow("<<< Detecting Markers >>> ", imgCopy1);
				sprintf(cmd,"/home/root/data/calibration/detectedMarkerCam%i.bmp", vs->cam_id);
				cv::imwrite(cmd,imgCopy1);
				printf(cmd);
				c=cvWaitKey();
				if (c=='q'){
					printf("Continue!\n");
					destroyWindow(cmd1);
				}
			}
			else{
				sprintf(cmd1, "%i Markers detected in %s %s camera", markers.size(), cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
				imshow(cmd1, imgCopy1);
				cv::moveWindow(cmd1,20,20);

				//cv::imshow("<<< Detecting Markers >>> ", imgCopy1);
				sprintf(cmd,"/home/root/data/calibration/detectedMarkerCam%i.bmp", vs->cam_id);
				cv::imwrite(cmd,imgCopy1);
				printf(cmd);
				c=cvWaitKey();
				if (c=='q'){
					printf("Continue!\n");
					destroyWindow(cmd1);
				}
			}
/*			sprintf(cmd1, "%i number of Markers detected in %s %s camera", markers.size(), cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
			imshow(cmd1, imgCopy1);
			cv::moveWindow(cmd1,20,20);

			//cv::imshow("<<< Detecting Markers >>> ", imgCopy1);
			sprintf(cmd,"/home/root/data/calibration/detectedMarkerCam%i.bmp", vs->cam_id);
			cv::imwrite(cmd,imgCopy1);
			printf(cmd);
			c=cvWaitKey();
			if (c=='q'){
				printf("Continue!\n");
				destroyWindow(cmd1);
			}*/

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
	char buff[512], buffX[512], v;
	int exposure_camID;


/*	vs->cam_id face is 4; main is 1 and 2; Aux is 129 and 130
	for changing exposure cam setting is
	face 4; main 24 and aux 4.*/

	float bThreshold;
	if (cam == 4){
		agc_val_cal = 1;
		bThreshold = 9.00;
		exposure_camID = 4;
	}
	else if (cam == 129 || cam == 130){
		agc_val_cal = 1;
		bThreshold = 25.00;
		exposure_camID = 3;
	}
	else if(cam == 1 || cam == 2){
		agc_val_cal = 1;
		//bThreshold = 15.00;
		bThreshold = 12.00;
		exposure_camID = 24;
	}

	if (calDebug){
		if (cam == 4){
			sprintf(buffX, "Adjusting Brightness of Face camera");
			imshow(buffX, outImg);
			cv::moveWindow(buffX, 20, 20);
		}
		else{
			sprintf(buffX, "Adjusting Brightness of %s %s camera", cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
			imshow(buffX, outImg);
			cv::moveWindow(buffX, 20, 20);
		}
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
			//sprintf(buffX, "Adjusting Brightness of %s %s camera", cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
			imshow(buffX, outImg);
			//cv::moveWindow(buffX, 20, 20);
			//imshow("Adjusting Brightness", outImg);
			v = cvWaitKey();
			if(v == 'q')
				continue;

		}

		if(agc_val_cal > 26)
		{
			sprintf(buff,"wcr(%d,0x3012,%d)",exposure_camID,agc_val_cal + 1);
			//sprintf(buff,"wcr(%d,0x3012,%d)",cam,agc_val_cal + 4);
			port_com_send(buff);
			//printf("Increase brightness");
			break;
		}


	}

	if (calDebug){
		destroyWindow(buffX);
	}
	//destroyWindow("<<< Adjusting Brightness >>>");
	printf("Brightness adjustment is completed!\n");

}

int RunSystemCmdCal(const char *ptr){
	int status = system(ptr);
	return status;
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
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id, calDebug);
	printf("Detecting marker of aux 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxRight = gridBooardMarker(outImg,vs->cam_id, calDebug);

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
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of aux 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxleft = gridBooardMarker(outImg,vs->cam_id, calDebug);
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
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of main 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainRight = gridBooardMarker(outImg,vs->cam_id, calDebug);
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
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of main 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainleft = gridBooardMarker(outImg,vs->cam_id, calDebug);
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
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	//adjusting brightness
	brightnessAdjust(outImg,vs->cam_id,calDebug);
	printf("Detecting marker of Face 8194 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerFace = gridBooardMarker(outImg,vs->cam_id, calDebug);
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

	char c;



	if (calDebug){
		cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, pt3, pt4, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, pt5, pt6, cv::Scalar(0, 255, 0), 3);
		cv::rectangle(smallImg, pt7, pt8, cv::Scalar(0, 255, 0), 3);
		imwrite("/home/root/data/calibration/MarkerRucoDetectofLeftRightAUX_Main.bmp", smallImg);
		sprintf(buff, "Each IrisCam projected in Face cam");
		imshow(buff,smallImg);
		cv::moveWindow(buff,20,20);
		cvWaitKey();
		destroyWindow(buff);
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



	// Creating CalRect.ini and upload to OIM ftp
	char buf[200];
	std::ofstream outfile("/home/root/CalRect.ini");

	EyelockLog(logger, DEBUG, "Create CalRect.ini file in /home/root folder");
	outfile << "FTracker.targetRectX=" << x << std::endl;
	outfile << "FTracker.targetRectY=" << y << std::endl;
	outfile << "FTracker.targetRectWidth=" << width << std::endl;
	outfile << "FTracker.targetRectHeight=" << height << std::endl;
	outfile.close();

	EyelockLog(logger, DEBUG, "Finished writing Calibration Rect data to CalRect.ini file");


	// Format the ftp drive
	EyelockLog(logger, DEBUG, "Format the ftp drive");
	port_com_send("f_formt(0)");


	sprintf(buf, "%s", "wput -B -t 2 CalRect.ini ftp://guest:guest@192.168.4.172");
	EyelockLog(logger, DEBUG, "Create ftp upload system command");
	fflush(stdout);
	RunSystemCmdCal(buf);
	EyelockLog(logger, DEBUG, "Upload of CalRect File to ftp 192.168.4.172 Successful");

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
		imwrite("/home/root/data/calibration/MarkerRucoDetectofLeftRightAUX_Main_testRect.bmp", smallImgX);
		sprintf(buff, "Projected Target in Face cam");
		imshow(buff, smallImgX);
		cv::moveWindow(buff, 20, 20);
		cvWaitKey();
		destroyWindow(buff);

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

/*	//Active this while loop if the calibration needs to run by moving motors
	while(check){
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>Motor is moving to %i and conducting calibration\n", newPos);

		sprintf(cmd, "fx_home()\n");
		port_com_send(cmd);
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
	}*/

	int numOfAttempt = 0;

	while(check){

		check = CalCam(calDebug);	// Rum calibration, return false if fail
		numOfAttempt++;

		printf("Center The motor so That it is facing the Target\n");
		//If Motor reach to max position and failt to calibrate
		if (numOfAttempt > 5){

			printf("Fail Camera to Camera geometric calibration process due to unfocus Images!!! \n");

			check = false;

		}
	}

}

#endif




void RunCamFocus(){

	int w,h;
	char key;

	bool quit = false;

	extFocus *fs;
	fs = new extFocus();

	char cmd[512];

	//Set AUX cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(100);

	//Fetching images from Face camera
	int leftCam = 8192;
	int rightCam = 8193;
	int faceCam = 8194;

	VideoStream *vs;
	vs = new VideoStream(leftCam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	printf("Start Left Iris Aux Camera \n");

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);
		//printf("quit:::::: %i\n", quit);

		if(fs->quit)
			break;
	}
	delete(vs);
	delete(fs);


	fs = new extFocus();

	vs = new VideoStream(rightCam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	printf("Start Right Iris Aux Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);
		//printf("quit:::::: %i\n", quit);

		if(fs->quit)
			break;
	}
	delete(vs);
	delete(fs);

	//Set Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(100);


	fs = new extFocus();
	vs = new VideoStream(leftCam);
	vs->flush();
	usleep(10000);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	printf("Start left Iris Main Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);
		//printf("quit:::::: %i\n", quit);

		if(fs->quit)
			break;
	}
	delete(vs);
	delete(fs);


	fs = new extFocus();
	vs = new VideoStream(rightCam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	printf("Start right Iris Main Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, vs->cam_id);
		//printf("quit:::::: %i\n", quit);

		if(fs->quit)
			break;
	}
	delete(vs);
	delete(fs);



	fs = new extFocus();
	vs = new VideoStream(8194);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	quit = false;

	printf("Start face Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		quit = fs->camControlFocus(outImg, faceCam);
		//printf("quit:::::: %i\n", quit);

		if(fs->quit)
			break;
	}
	delete(vs);
	delete(fs);



	return 0;
}





int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");
	bool bShowFaceTracking = fconfig.getValue("FTracker.ShowFaceTracking", false);
	pthread_t threadId;
    pthread_t thredEcId;

    int w,h;
    char key;


    int run_mode;
    int cal_mode=0;				//initialize image optimization
    int cal_cam_mode = 0;		//initializing camera to camera geometric calibration
    int temp_mode = 0;
    int focusMode = 0;


	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);


	run_mode=1;  //DMO forced


	//Camera to camera geometric calibration
	if (strcmp(argv[1],"calcam")==0)
	{
		EyelockLog(logger, DEBUG, "calcam mode is running");
		//run_mode =1;
		cal_cam_mode=1;
	}

	if (strcmp(argv[1],"focus")==0)
	{
		focusMode =1;

	}
	//pThread for face tracker active
	if (run_mode)
		pthread_create(&threadId,NULL,init_tunnel,NULL);
	/*if (run_mode)
		pthread_create(&threadId,NULL,init_ec,NULL);*/

#if 0 // Duplication
	if(focusMode){
		portcom_start();

		fs->DoStartCmd_focus();
		delete(fs);
		RunCamFocus();

		return 0;

	}

#endif

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

}




