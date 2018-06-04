/*****************************************************************************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************************************************************************/
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

#ifdef ARUCO
//#include <aruco.h>
//#include <dictionary.h>
#endif

#include "eyelock_com.h"
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"

#include "log.h"

using namespace cv;
using namespace std::chrono;
using namespace std;

const char logger[30] = "test";

VideoStream *vs;



Mat outImg, r_outImg;
Mat smallImg;
Mat frame_gray;
Mat smallImgBeforeRotate;


double scaling = 8.0;


extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern int portcom_start();
extern void port_com_send(char *cmd);
extern void  *init_tunnel(void *arg);
extern void *init_ec(void * arg);
extern int port_com_send_return(char *cmd, char *buffer, int min_len);
int face_init(  );
float read_angle(void);



int z=0;		//AGC wait key

#define block 0
#define WIDTH 1200
#define HEIGHT 960

#define DISP // To show the image display


int CENTER_POS;		//low fx_abs val is 0 and max is 328. So the center falls in 164
#define CENTER_POSITION_ANGLE 95
#define MIN_POS 25
#define MAX_POS 328			//previous val was 200
#define WAIT 800



#define MODE_CHANGE_FREEZE 10
int cur_pos=CENTER_POS;

#define MIN_IRIS_FRAMES 10
int fileNum=0;
int move_counts=0;
#define FRAME_DELAY 40

// this defines how many frames without a face will cause a switch back to face mode ie look for faces
#define NO_FACE_SWITCH_FACE 10

string fileName = "output.csv";
string a_calibFile = "/home/root/data/calibration/auxRect.csv";
int noeyesframe = 0;
#define ANGLE_TO_STEPS 5



void MainIrisSettings();
void SetExp(int cam, int val);
void MoveToAngle(float a);
void MoveTo(int v);
void accelStatus();
void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
void SelLedDistance(int val);
char* GetTimeStamp();
void RecoverModeDrop();
void SwitchIrisCameras(bool mode);
void SetFaceMode();



void writeStringData(string fileName, string v);
void writeFloatData(string fileName, float v);
void writeStartNewLine(string fileName);
void clearData(string fileName);


// Temperature test purpose
void motorMove();
double parsingIntfromHex(string str1);
int calTemp(int i);




#define PIXEL_TOTAL 900



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





#define smallMoveTo 2		//limiting motor movement
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

	writeFloatData(fileName, current_a); 	//current angle
	writeStringData(fileName,",");
	writeFloatData(fileName, a); 	//move to angle
	writeStringData(fileName,",");

	float t_start=clock();
	port_com_send(buff);
	float t_result = (float)(clock()-t_start)/CLOCKS_PER_SEC;
	EyelockLog(logger, DEBUG, "Time required to move to angle:%3.3f", t_result);
	writeFloatData(fileName, t_result); //required time to move
	writeStringData(fileName,",");

}







void MoveTo(int v)
{
	EyelockLog(logger, TRACE, "MoveTo");
	/*
	char buff[100];
	if (v<MIN_POS) v= MIN_POS;
	if (v>MAX_POS) v=MAX_POS;
	//cvWaitKey(100);
	sprintf(buff,"fx_abs(%d)",v);
	printf("Sending move: %s\n",buff);
	port_com_send(buff);
	move_counts++;
	//cvWaitKey(100);
	 */

	EyelockLog(logger, DEBUG,"Move to command %d ",v);

	writeFloatData(fileName, v); 	//move to command
	writeStringData(fileName,",");


	v=v-CENTER_POS;
	v=v/ANGLE_TO_STEPS+CENTER_POSITION_ANGLE;
	EyelockLog(logger, DEBUG,"Value to MoveToAngle:angle = %d",v);
	MoveToAngle((float) v);
}


void accelStatus(){
#if 0
	char buff[100];
	sprintf(buff,"accel()");
	printf("Sending accel command\n");
	port_com_send(buff);

/*	for(int i = 0; i <30; i++){
		printf("%c \n",buff[i]);

	}*/

#endif
}




/*
#define IO_FILENAME "/home/root/"
void HandleEyelockIO(void)

{
	FILE *in;
	in = fopen(IO_FILENAME,"rt");
	if (in)
	{
		char temp[512];
		int len = fread(temp,sizeof(char),sizeof(temp),in);
		temp[len]=0;
		port_com_send(temp);
	}
}
*/





int IrisFrameCtr = 0;


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






#define RUN_STATE_FACE 0
#define RUN_STATE_EYES 1


#define FACE_GAIN_DEFAULT   (PIXEL_TOTAL*10)
#define FACE_GAIN_MAX       (PIXEL_TOTAL*80)
#define FACE_GAIN_MIN       (PIXEL_TOTAL*8)
#define FACE_GAIN_PER_GOAL   10
#define FACE_GAIN_HIST_GOAL  .1
#define FACE_CONTROL_GAIN   500.0
//#define FACE_CONTROL_GAIN   1000.0

int agc_val= FACE_GAIN_DEFAULT;
int agc_set_gain =0;

int run_state=RUN_STATE_FACE;




#define CAL_BRIGHT       85

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




std::chrono:: time_point<std::chrono::system_clock> start_mode_change;


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

#define MODE_FACE 0
#define MODE_EYES_MAIN 1
#define MODE_EYES_AUX 2
int currnet_mode = 0;

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
	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((faceAnalogGain&0x3)<<4) | 0X80);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Face camera Analog gain:%d",(((faceAnalogGain&0x3)<<4) | 0X80));
	agc_val= FACE_GAIN_DEFAULT;
	EyelockLog(logger, DEBUG, "Face camera gain default:%d", FACE_GAIN_DEFAULT);
	start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_FACE;
	//port_com_send("fixed_set_rgb(100,100,100)");
}

#define IRIS_FRAME_TIME 180
#define switchThreshold 37		// previous val was 40
#define errSwitchThreshold 6
int previousEye_distance = 0;

void MoveRelAngle(float a, int diffEyedistance);
void SetIrisMode(int CurrentEye_distance, int diffEyedistance);
void DoStartCmd();

void MoveRelAngle(float a, int diffEyedistance)
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

void SetIrisMode(int CurrentEye_distance, int diffEyedistance)
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
	EyelockLog(logger, DEBUG, "previousEye_distance: %i; CurrentEye_distance: %i; diffEyedistance: %i\n", previousEye_distance, CurrentEye_distance, diffEyedistance);
	if (CurrentEye_distance >= switchThreshold && diffEyedistance <= errSwitchThreshold)
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
	else if (CurrentEye_distance < switchThreshold && diffEyedistance <= errSwitchThreshold)
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
	else if (CurrentEye_distance >= switchThreshold && diffEyedistance > errSwitchThreshold)
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
	}
	IrisFrameCtr=0;
	start_mode_change = std::chrono::system_clock::now();
}

#if 0
//DoStartCmd ISCwest Demo
void m2_DoStartCmd()
{
	//setRGBled(BRIGHTNESS_MIN,BRIGHTNESS_MIN,BRIGHTNESS_MIN,10,0,0x1F);


	char cmd[100];
	//port_com_send("set_audio(1)");
	//port_com_send("/home/root/tones/auth.wav");
	//start_mode_change = std::chrono::system_clock::now();

	//Mohammad
	//setting up face cameras and motor pos
	printf("Re Homing\n");
	port_com_send("fx_home()\n");
	//port_com_send("wcr(0x1f,0x301a,0x1199)");
	usleep(100000);
	//cvWaitKey(6000);
	port_com_send("fx_abs(25)\n");
	//cvWaitKey(6000);

	printf("Moving to Center\n");
	MoveTo(CENTER_POS);
	read_angle();







	//printf("configuring face settings\n");
	//Face configuration of LED
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,1)");
	//Ilya drive the face LEDs with 4amp and we guess it fried the board
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,40) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");



	// flip
	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping...recommended by ilya
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	// reset colum of all imagers
#if 0
	port_com_send("set_cam_mode(0,10)");
	cvWaitKey(500);
	port_com_send("wcr(0x1f,0x30b0,0x80) | wcr(0x1f,0x305e,0x20) | wcr(0x1f,0x30d4,0xE007)");
	port_com_send("set_cam_mode(0x07,10)");
	cvWaitKey(500);
	port_com_send("set_cam_mode(0x87,10)");
	cvWaitKey(500);
#endif

	//cvWaitKey(6000);
	//Face cameras configuration
//	port_com_send("wcr(0x04,0x3012,16) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,160)");
	port_com_send("wcr(0x04,0x3012,12) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xF0)");
	//port_com_send("wcr(4,0x30b0,0x4090)");// enable analog gain x2
	//cvWaitKey(6000);


	//this is before ilya mucked with it
	//printf("configuring Main Iris settings\n");
	//Main Iris cameras configuration
	port_com_send("wcr(0x18,0x3012,12) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,64)");
	//port_com_send("wcr(0x18,0x3040,0xC000)"); //Flipping of iris images

	//Aux Iris Cameras Configuration
	//port_com_send("wcr(0x03,0x3012,12) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,128)");
	port_com_send("wcr(0x03,0x3012,12) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,160)"); // was 128 Anita chnaged my Mo


	// setup up all pll values
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);
	// port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6))");
	port_com_send("wcr(0x04,0x302e,2) | wcr(0x04,0x3030,44) | wcr(0x04,0x302c,2) | wcr(0x04,0x302a,6))");
	cvWaitKey(10);

	port_com_send("wcr(0x04,0x30b0,0x90");	//sarvesh

	// port_com_send("wcr(0x1f,0x30b0,0x4090");
	// port_com_send("wcr(0x1B,0x30b0,0x80");

	//changes that ilya make
	//printf("configuring Main Iris settings\n");
	//Main Iris cameras configuration
	//port_com_send("wcr(0x18,0x3012,12) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,40)");


	//Aux Iris Cameras Configuration
	//port_com_send("wcr(0x03,0x3012,12) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,128)");
	//port_com_send("wcr(0x1f,0x3040,0xC000)"); //Flipping of iris images

	//streaming Main and face cameras
	//port_com_send("set_cam_mode(0x07,40)");

	//Always start with Face and Auxillary Iris Cameras
	//sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);






	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	//cvWaitKey(6000);
	port_com_send(cmd);

/*	//This code is for playing sound
	if(1)
	{
		port_com_send("set_audio(1)");
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
		sleep(2);
	}*/

}
#endif




//Old
#define SCALE 3
Rect detect_area(15/SCALE,15/SCALE,(960/(SCALE*SCALE))+15/SCALE,(1200/(SCALE*SCALE))+15/SCALE); //15 was 30

/*
//follwoing command is for test demo 1 no_move_area
Rect no_move_area(0,52,120,14);
*/

//follwoing command is for test demo 2 no_move_area
//Rect no_move_area(0,64,120,14);

//follwoing command is for test demo 3 no_move_area
//Rect no_move_area(0,68,120,12);

//Test rect after camera to camera calibration
//Rect no_move_area(0,460/scaling,960/scaling,231/scaling);
Rect no_move_area;


//Rect detect_area_center(120/SCALE,120/SCALE,(960-120*2)/(SCALE*SCALE),1200/SCALE)
//Rect no_move_area_aux(0,660/scaling,960/scaling,160/scaling); //Mohammad rect at the center 580




// Main DoStartCmd configuration for Eyelock matching
#if 1
void DoStartCmd(){

	EyelockLog(logger, TRACE, "DoStartCmd");
	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	int minPos = fconfig.getValue("FTracker.minPos",0);
	CENTER_POS = fconfig.getValue("FTracker.centerPos",164);
	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);

	int faceLEDVolt = fconfig.getValue("FTracker.faceLEDVolt",30);
	int faceLEDcurrentSet = fconfig.getValue("FTracker.faceLEDcurrentSet",20);
	int faceLEDtrigger = fconfig.getValue("FTracker.faceLEDtrigger",4);
	int faceLEDEnable = fconfig.getValue("FTracker.faceLEDEnable",4);
	int faceLEDmaxTime = fconfig.getValue("FTracker.faceLEDmaxTime",4);


/*	int IrisLEDVolt = fconfig.getValue("FTracker.IrisLEDVolt",30);
	int IrisLEDcurrentSet = fconfig.getValue("FTracker.IrisLEDcurrentSet",40);
	int IrisLEDtrigger = fconfig.getValue("FTracker.IrisLEDtrigger",3);
	int IrisLEDEnable = fconfig.getValue("FTracker.IrisLEDEnable",49);
	int IrisLEDmaxTime = fconfig.getValue("FTracker.IrisLEDmaxTime",4);*/


	int faceCamExposureTime = fconfig.getValue("FTracker.faceCamExposureTime",12);
	int faceCamDigitalGain = fconfig.getValue("FTracker.faceCamDigitalGain",240);
	int faceCamDataPedestal = fconfig.getValue("FTracker.faceCamDataPedestal",0);

	int AuxIrisCamExposureTime = fconfig.getValue("FTracker.AuxIrisCamExposureTime",8);
	int AuxIrisCamDigitalGain = fconfig.getValue("FTracker.AuxIrisCamDigitalGain",80);

	//printf("aux gain :::: %i\n", AuxIrisCamDigitalGain);
	//printf("aux exp :::: %i\n", AuxIrisCamExposureTime);

	int AuxIrisCamDataPedestal = fconfig.getValue("FTracker.AuxIrisCamDataPedestal",0);

	int MainIrisCamExposureTime = fconfig.getValue("FTracker.MainIrisCamExposureTime",8);
	int MainIrisCamDigitalGain = fconfig.getValue("FTracker.MainIrisCamDigitalGain",128);
	int MainIrisCamDataPedestal = fconfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	int irisAnalogGain = fconfig.getValue("FTracker.irisAnalogGain",144);
	int faceAnalogGain = fconfig.getValue("FTracker.faceAnalogGain",128);

	int capacitorChargeCurrent = fconfig.getValue("FTracker.capacitorChargeCurrent",60);

  //printf("AuxIrisCamDigitalGain = %d\n",AuxIrisCamDigitalGain);

	char cmd[512];

	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	printf("Re Homing\n");

	EyelockLog(logger, DEBUG, "port_com_send fx_home command is issued");
#ifdef NOOPTIMIZE
	usleep(100000);
#endif

	//Reset the lower motion
	sprintf(cmd, "fx_abs(%i)",minPos);
	EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", minPos);
	port_com_send(cmd);

	//move to center position
	// printf("Moving to Center\n");
	EyelockLog(logger, DEBUG, "Moving to center position");
	MoveTo(CENTER_POS);
	read_angle();		//read current angle


	//printf("configuring face LEDs\n");
	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n",faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);

	EyelockLog(logger, DEBUG, "Configuring face LEDs faceLEDVolt:%d, allLEDhighVoltEnable:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d, faceLEDmaxTime:%d", faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);

	// printf(cmd);
	port_com_send(cmd);
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");


	//port_com_send("psoc_write(9,90)");	// charge cap for max current 60 < range < 95
	sprintf(cmd, "psoc_write(9,%i)\n", capacitorChargeCurrent);
	EyelockLog(logger, DEBUG, "capacitorChargeCurrent:%d", capacitorChargeCurrent);
	//printf(cmd);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95
	//port_com_send("psoc_write(9,80)");	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	//Face cameras configuration
	//printf("configuring face Cameras\n");
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", faceCamExposureTime, faceCamDataPedestal, faceCamDigitalGain);
	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xFE)");
	//port_com_send("wcr(0x04,0x3012,12) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xF0)");	//Demo Config



	//Main Iris cameras configuration
	//printf("configuring Aux Iris Cameras\n");
	EyelockLog(logger, DEBUG, "Configuring AUX Iris Cameras");
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);

	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("wcr(0x18,0x3012,8) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,0x80)");
	//port_com_send("wcr(0x18,0x3012,8) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,80)"); //DEMO Config


	//Aux Iris Cameras Configuration
	//printf("configuring Main Iris Cameras\n");
	EyelockLog(logger, DEBUG, "Configuring Main Iris Cameras");
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", faceCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("wcr(0x03,0x3012,8) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,0xB0)"); // was 128 Anita chnaged my Mo
	//port_com_send("wcr(0x03,0x3012,8) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,128)");	//Demo Config


	// setup up all pll values
	//printf("setting up PLL\n");
	EyelockLog(logger, DEBUG, "Setting up PLL");
	//following process will activate PLL for all cameras

	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	//printf("Analog gain is %d\n",analogGain);
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((irisAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Iris analog gain: %d", (((irisAnalogGain&0x3)<<4) | 0X80));
	// printf(cmd);
	port_com_send(cmd);
	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((faceAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Face analog gain: %d", (((faceAnalogGain&0x3)<<4) | 0X80));
	// printf(cmd);
	port_com_send(cmd);
	//port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	//port_com_send("wcr(0x4,0x30b0,0x90");		//Only face camera gain is x90


	// printf("Turning on Alternate cameras\n");
	EyelockLog(logger, DEBUG, "Turning on Alternate cameras");
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	//sprintf(cmd,"set_cam_mode(0x4,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);

	port_com_send("wcr(0x1f,0x301a,0x1998)"); // ilya added to leave the pll on always

	//This code is for playing sound
	if(1)
	{
		EyelockLog(logger, DEBUG, "playing audio -set_audio(1)");
		port_com_send("set_audio(1)");
		sleep(1);
		port_com_send("data_store_set(0)");
		sleep(1);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
		sleep(1);
		port_com_send("data_store_set(1)");
		sleep(1);
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/rej.raw");
		sleep(1);
	}


	//Reading the calibrated Rect
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
	no_move_area.height = (a_rect[3])/scaling -targetOffset*2;

	currnet_mode = -1;
	SetFaceMode();		// just for now

}
#endif

void DoStartCmd_CamCal(){

	EyelockLog(logger, TRACE, "DoStartCmd_CamCal");
	char cmd[100];

	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	port_com_send("fx_home()\n");
#ifdef NOOPTIMIZE
	usleep(100000);
#endif
	port_com_send("fx_abs(25)\n");

	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	port_com_send("psoc_write(3,1)| psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,1) | psoc_write(6,4)");


	//port_com_send("psoc_write(9,90)");	// charge cap for max current 60 < range < 95
	port_com_send("psoc_write(9,60)");	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images



	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	port_com_send("wcr(0x04,0x3012,2) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0x30)");



	//Main Iris cameras configuration
	EyelockLog(logger, DEBUG, "Configuring Main Iris Cameras");
	port_com_send("wcr(0x18,0x3012,3) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,0x40)");


	//Aux Iris Cameras Configuration
	EyelockLog(logger, DEBUG, "Configuring Alternate Iris Cameras");
	port_com_send("wcr(0x03,0x3012,3) | wcr(0x03,0x301e,0) | wcr(0x03,0x305e,0x40)"); // was 128 Anita chnaged my Mo


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


void detectAndDisplay( Mat frame );
//std::printf("Detect face = %i\n",1);

int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size);
Point eyes;

//at 40inch or 106cm distance
#define MIN_FACE_SIZE 10		// previous val 40
#define MAX_FACE_SIZE 70		//// previous val 60



extern int IrisFramesHaveEyes();
char temp[100];

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

int AGC_Counter = 0;

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

int noFaceCounter =0;

void DoRunMode()
{
	EyelockLog(logger, TRACE, "DoRunMode");
	float eye_size;

	// if (run_state == RUN_STATE_FACE)
	{
		float p;
		char temp_str[40];

		int64 startTime = cv::getTickCount();
		EyelockLog(logger, DEBUG, "image resize");
		cv::resize(outImg, smallImgBeforeRotate, cv::Size(), (1 / scaling),
				(1 / scaling), INTER_NEAREST);	//Mohammad

		smallImg = rotation90(smallImgBeforeRotate);

//FACE_GAIN_DEFAULT   0x80		128
//       0xe0		224
//FACE_GAIN_MIN       0x10		16
//FACE_GAIN_PER_GOAL   1
//FACE_GAIN_HIST_GOAL  0
//FACE_CONTROL_GAIN   20.0

//int agc_val= FACE_GAIN_DEFAULT;		128
//int agc_set_gain =0;
		EyelockLog(logger, DEBUG, "AGC Calculation");
		p = AGC(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),
				180);

		if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
			agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
		if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
			agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
		agc_val = MAX(FACE_GAIN_MIN,agc_val);
		agc_val = MIN(agc_val,FACE_GAIN_MAX);
		//Too close
//			if(agc_val==FACE_GAIN_MIN)
//			{
//				//SelLedDistance(150);
//				setRGBled(120,0,0,500,0,0x4);
//			}
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

		EyelockLog(logger, DEBUG, "FindEyeLocation");
		if (FindEyeLocation(smallImg, eyes, eye_size)) {
			noeyesframe = 0;

			if (detect_area.contains(eyes)) {
				EyelockLog(logger, DEBUG, "eyes found");

				noFaceCounter = 0;

#define ERROR_LOOP_GAIN 0.08
				int CurrentEye_distance = eye_size;
				int diffEyedistance = abs(
						CurrentEye_distance - previousEye_distance);

				//if (!no_move_area.contains(eyes))
				if (!no_move_area.contains(eyes)) {
					if ((eye_size >= MIN_FACE_SIZE)
							&& (eye_size <= MAX_FACE_SIZE)) {
						float err;
						int constant = 10;
						int MoveToLimitBound = 1;
						err = (no_move_area.y + no_move_area.height / 2)
								- eyes.y;
						EyelockLog(logger, DEBUG, "abs err----------------------------------->  %d\n", abs(err));
						err = (float) err * (float) SCALE
								* (float) ERROR_LOOP_GAIN;

						// if we need to move
						if (abs(err) > MoveToLimitBound) {
							int x, w, h;

							//Experiment
							/////////////////////////////////////////////////
							EyelockLog(logger, DEBUG, "Switching ON IRIS LEDs!!!!\n");
							SetIrisMode(eye_size, diffEyedistance);
							run_state = RUN_STATE_EYES;
							////////////////////////////////////////////////

							MoveRelAngle(-1 * err, diffEyedistance);
							//	while (waitKey(10) != 'z');
							//	printf("fx_abs val: %i\n", -1*err);
							//	printf("Face is IN RANGE!!!!\n");

							//flush the video buffer to get rid of frames from motion
							{
								//int vid_stream_flush(void);
								//vid_stream_flush();
								vs->flush();
							}
							// might need this vid_stream_get(&w,&h,(char *)outImg.data);
						}

					} else
						EyelockLog(logger, DEBUG, "Face out of range\n");
				} else {

					cv::rectangle(smallImg,
							Rect(eyes.x - 5, eyes.y - 5, 10, 10),
							Scalar(255, 0, 0), 2, 0);
					/*						printf("Switching ON IRIS LEDs!!!!\n");
					 SetIrisMode(eye_size, diffEyedistance);
					 run_state = RUN_STATE_EYES;*/

					//port_com_send("fixed_set_rgb(0,0,50)");
				}

			}

		}
		{
//				noeyesframe++;
			//Sarvesh
			if (noFaceCounter < (NO_FACE_SWITCH_FACE + 2))
				noFaceCounter++;

			if (noFaceCounter == NO_FACE_SWITCH_FACE) {
				MoveTo(CENTER_POS);
				run_state = RUN_STATE_FACE;
				SetFaceMode();

			}

			//MOhammad
			/*				if (IrisFrameCtr==MIN_IRIS_FRAMES)
			 {
			 printf("Iris number reach to %i\n and it will go to home now", IrisFrameCtr);
			 run_state = RUN_STATE_FACE;
			 SetFaceMode();
			 MoveTo(CENTER_POS);
			 }*/

		}

		/*if (noeyesframe == 10)
		 {
		 SetFaceMode();
		 MoveTo(CENTER_POS);
		 }
		 */
		/*				if (move_counts> MOVE_COUNTS_REHOME)
		 {
		 printf("Re Homing\n");
		 setRGBled(0,0,0,100,0,0x1F);
		 port_com_send("fx_home()\n");
		 cvWaitKey(6000);
		 MoveTo(CENTER_POS);
		 setRGBled(20,20,20,100,0,0x1F);
		 move_counts=0;
		 }*/

#ifdef DISP
		EyelockLog(logger, DEBUG, "Imshow");
		cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
		imshow(temp, smallImg);
#endif
	}

	/*
	 if (run_state == RUN_STATE_EYES)
	 {
	 if (IrisFramesHaveEyes()==0)
	 {
	 run_state = RUN_STATE_FACE;
	 SetFaceMode();
	 MoveTo(CENTER_POS);
	 }
	 }
	 */

}

Mat outImgLast, outImg1, outImg1s;
void MeasureSnr() {
	EyelockLog(logger, TRACE, "MeasureSnr");
	Mat s1;
	static int once = 0;
	if (once == 10) {
	Scalar s2 = sum(outImg);
	float pixels = outImg.cols * outImg.rows;
	double avg = s2.val[0] / pixels;
	EyelockLog(logger, DEBUG, "a = %3.3f\n", avg);
	/*
		printf("id ===%x %x %x %x\n",outImg.at <char> (0,0),outImg.at <char> (0,1),outImg.at <char> (0,2),outImg.at <char> (0,3));
		absdiff(outImg, outImgLast, s1);       // |I1 - I2|


		imshow("Signal",outImg-outImg1s);
		// imshow("other",outImg-outImg1);  //Time debug
	 	s1=s1-(Scalar(sum(s1).val[0])/pixels); // remove exposure noise
		imshow("Noise",s1*10);
	 	 s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
		 s1 = s1.mul(s1);
		 Scalar s = sum(s1);         // sum elements per channel

		 Scalar s2 = sum(outImg);
		 double rms_noise = sqrt(s.val[0]/pixels);
		 double sse = s.val[0];
		 double mid= s1.cols*s1.rows*128*128;
		 double avg = s2.val[0]/pixels;
		 float psnr = 20.0*log10((128.0)/rms_noise);
		 float psnr_avg = 20.0*log10((avg)/rms_noise);
	     printf("Snr = %3.3f avg = %3.3f snravg %3.3f\n", psnr,avg,psnr_avg);*/
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

#define NUM_AVG_CAL 10
#define  MAX_CAL_CURENT 20
void DoImageCal(int cam_id_ignore)
{
	EyelockLog(logger, TRACE, "Inside DoImageCal");
    int w,h;
	char temp[100];
   // vid_stream_get(&w,&h,(char *)outImg.data);
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
    	if (a.val[0]>40)
    		break;
    	}
    if (a.val[0]<40)
    {
    	EyelockLog(logger, DEBUG, "Error Not enough light");
    	exit(0);
    }
    if (a.val[0]>220)
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


#if 0
int arucoMarkerTracking(Mat InImage){

	 //cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	//cv::Ptr<cv::aruco::Dictionary> dictionary;// = aruco::getPredefinedDictionary(aruco::DICT_6X6_250);;

	//dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

//	cv::Ptr<aruco::DetectorParameters> parameters;
	//cv::Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_100);

		//for (int i = 0; i < 50; i++){
		//aruco::drawMarker(markerDict, i , 500, oMarker, 1);
	aruco::MarkerDetector MDetector;

//	 MDetector.setDictionary("ARUCO_MIP_36h12");
	 MDetector.setDictionary("ARUCO");
	       auto Markers=MDetector.detect(InImage);
	       //for each marker, draw info and its boundaries in the image
	       for(auto m:MDetector.detect(InImage)){
	           std::cout<<m<<std::endl;
	           m.draw(InImage);
		       cv::imshow("in",InImage);
	       }
	       //cv::waitKey(0);//wait for key to be pressed

//	}
	       return 1;
}

std::vector<aruco::Marker> gridBooardMarker(Mat img){
	//VideoCapture inputVideo;
	//inputVideo.open(0);
	int imgCount = 0;

	printf("Inside gridBooardMarker\n");
    int w,h;

	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	int portNum = vs->m_port;
	Mat imgCopy;

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

#if 1
	if(!img.empty()){
		markers = mDetector.detect(imgCopy);
		//cv::imshow("streaming without marker", imgCopy);

		if (markers.size() < 2){
			printf("%i camera detected %i markers!\n", portNum, markers.size());
			cv::imshow("marker Detects", imgCopy);
			//sprintf(buffer, "No_marker_detect%i.png", portNum);
			//imwrite(buffer, imgCopy);
			return markers;
			//exit(EXIT_FAILURE);
		}

		for(size_t i = 0; i < markers.size(); i++){
			//cout << markers[i] << endl;
			//cout << "IDs ::: " << markers[i].id << "    center  ::: " << markers[i].getCenter() << endl;
			markers[i].draw(imgCopy);

		}
		//namedWindow("marker Detects", WINDOW_NORMAL);
		//sprintf(buffer, "imgAruco%i.png", portNum);
		cv::imshow("marker Detects", imgCopy);
		//imwrite(buffer, imgCopy);

	}
	else{
		printf("There is no Image to detect Aruco-markers!!!\n");
		exit(EXIT_FAILURE);

	}



	//comment the following lines it you want continue streaming and finish calibration!
	while (1)
	{
		char c=cvWaitKey(200);
		if (c=='q')
			return markers;
		if (c == 's'){
			char fName[50];
			sprintf(fName,"%d_%d.pgm",portNum,imgCount++);
			cv::imwrite(fName,imgCopy);
			printf("savedAruco %s\n",fName);

		}
	}



#endif
	//return markers;

}

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
		int id = targetID[i];
		for (int j = 0; j < markerIris.size(); j++){
			int idIris = markerIris[j].id;
			if (id == idIris){
				pointsIris.push_back(markerIris[j].getCenter());
			}
		}
	}


	//Search target ID's center in Face camera
	for (int i = 0; i < targetID.size(); i++){
		int id = targetID[i];
		for (int j = 0; j < markerFace.size(); j++){
			int idFace = markerFace[j].id;
			if (id == idFace){
				pointsFace.push_back(markerFace[j].getCenter());
			}
		}
	}



	//check whether it was successfully detected atleast two target points from both camera
	if (pointsIris.size() <= 1){
		printf("Fail to detect two aruco markers in horizental direction!\n");
		return rectResult;
		//exit(EXIT_FAILURE);
	}

	//cout << pointsIris[0].x << "-------------" << pointsIris[0].y << endl;
	//cout << pointsIris[1].x << "-------------" << pointsIris[1].y << endl;
	//cout << pointsFace[0].x << "-------------" << pointsFace[0].y << endl;
	//cout << pointsFace[1].x << "-------------" << pointsFace[1].y << endl;
	//cout << endl;

	//calculate the zoom offset or slope
	float mx = (pointsFace[1].x - pointsFace[0].x) / (pointsIris[1].x - pointsIris[0].x);
	float my = (pointsFace[1].y - pointsFace[0].y) / (pointsIris[1].y - pointsIris[0].y);

	//cout << "row::::: " << row << "   col:::::" << col << endl;

	//measure the x , x_offset and y, y_offset
	float x_offset = pointsFace[0].x - (mx * pointsIris[0].x);
	float xMax_offset = (mx * col) + x_offset;
	float y_offset = pointsFace[0].y - (my * pointsIris[0].y);
	float yMax_offset = (my * row) + y_offset;


	//cout << x_offset << "*********************" << xMax_offset << endl;
	//cout << y_offset << "  **********************   " << yMax_offset << endl;



	cout << "successfully calculated co-orinates! \n" << endl;

	rectResult.push_back(x_offset);
	rectResult.push_back(y_offset);
	rectResult.push_back(xMax_offset);
	rectResult.push_back(yMax_offset);

	return rectResult;

	//draw a rect to verify the rect
	cv::Point pt1(x_offset, y_offset);
	cv::Point pt2(xMax_offset, yMax_offset);
	smallImg = rotation90(outImg);
	cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255));
	imwrite("imgArucoRect.png", smallImg);

}


int agc_val_cal=3;

void brightnessAdjust(Mat outImg, int cam){
	float p, p_old;
	int w,h;

	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	//vs = new VideoStream(8193);
	p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),128);
	//imwrite("b_ad_a8193Img.png",outImg);
	//printf("percentile::: %f\n", p);
	//outImg.release();
	//delete (vs);

	//cout << "Cam ID:::::" << vs->cam_id << endl;
	//int agc_val_cal=5;
	char buff[512];

	float bThreshold;
	if (cam == 4){
		agc_val_cal = 3;
		bThreshold = 20.00;
	}
	else
		bThreshold = 65.00;


	while(!(p >= bThreshold)){
		agc_val_cal++;
		sprintf(buff,"wcr(%d,0x3012,%d)",cam,agc_val_cal);
		//printf(buff);
		//printf("Setting Gain %d\n",coarse);
		port_com_send(buff);
		p_old = p;

		vs->get(&w,&h,(char *)outImg.data);
		vs->get(&w,&h,(char *)outImg.data);
		vs->get(&w,&h,(char *)outImg.data);

		p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),128);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Percentile::: %3.3f Agc value = %d\n",p,agc_val_cal);
		imshow("AGC change", outImg);
		cvWaitKey();

		if(!(abs(p - p_old) > 1)){
			sprintf(buff,"wcr(%d,0x3012,%d)",cam,agc_val_cal + 4);
			port_com_send(buff);
			break;
		}


	}

	printf("Brightness adjustment is completed!\n");

}




bool CalCam(){

	char cmd[512];

	//Turn on Alternate cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);


	//Fetching images from Aux right
	vs = new VideoStream(8193);

	//adjusting brightness
	brightnessAdjust(outImg,3);
	printf("Detecting marker of aux 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxRight = gridBooardMarker(outImg);

	//If detected markers less then 2
	if (markerIrisAuxRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		return true;
	}
	delete (vs);



	//Fetching images from Aux left
	vs = new VideoStream(8192);

	//adjusting brightness
	brightnessAdjust(outImg,3);
	printf("Detecting marker of aux 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxleft = gridBooardMarker(outImg);
	if (markerIrisAuxleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		return true;
	}
	delete (vs);



	//Turn on Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);

	//Fetching images from Main Right
	vs = new VideoStream(8193);

	//adjusting brightness
	brightnessAdjust(outImg,24);
	printf("Detecting marker of main 8193 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainRight = gridBooardMarker(outImg);
	if (markerIrisMainRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		return true;
	}
	delete (vs);

	//Fetching images from Main Left
	vs = new VideoStream(8192);

	//adjusting brightness
	brightnessAdjust(outImg,24);
	printf("Detecting marker of main 8192 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainleft = gridBooardMarker(outImg);
	if (markerIrisMainleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		return true;
	}
	delete (vs);


	//Fetching images from Face camera
	vs = new VideoStream(8194);

	//adjusting brightness
	brightnessAdjust(outImg,4);
	printf("Detecting marker of Face 8194 cam\n");

	//Detecting markers
	std::vector<aruco::Marker> markerFace = gridBooardMarker(outImg);
	if (markerFace.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
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

	cv::rectangle(smallImg, pt1, pt2, cv::Scalar(255, 255, 255), 3);

	//Aux Right sorting
	x_offset = rectRightAux[0];
	y_offset = rectRightAux[1];
	xMax_offset = rectRightAux[2];
	yMax_offset = rectRightAux[3];

	cv::Point pt3(x_offset, y_offset);
	cv::Point pt4(xMax_offset, yMax_offset);
	cv::rectangle(smallImg, pt3, pt4, cv::Scalar(255, 255, 255), 3);




	//Main Left sorting
	x_offset = rectLeftMain[0];
	y_offset = rectLeftMain[1];
	xMax_offset = rectLeftMain[2];
	yMax_offset = rectLeftMain[3];


	cv::Point pt5(x_offset, y_offset);
	cv::Point pt6(xMax_offset, yMax_offset);
	cv::rectangle(smallImg, pt5, pt6, cv::Scalar(0, 255, 0), 3);

	//Main Right sorting
	x_offset = rectRightMain[0];
	y_offset = rectRightMain[1];
	xMax_offset = rectRightMain[2];
	yMax_offset = rectRightMain[3];

	cv::Point pt7(x_offset, y_offset);
	cv::Point pt8(xMax_offset, yMax_offset);
	cv::rectangle(smallImg, pt7, pt8, cv::Scalar(0, 255, 0), 3);


	printf("Finished calibration process------------------->\n");
	imwrite("MarkerRucoDetectofLeftRightAUX_Main.png", smallImg);
	delete (vs);


	//Data saved for Aux Rect
	int x, y, width, height;
	x = 0; y = pt1.y; width = int(smallImg.cols); height = int(abs(pt1.y - pt4.y));
	Rect auxRect(x, y,width, height);

	ofstream auxfile("auxRect.csv");
	auxfile << x << endl;
	auxfile << y << endl;
	auxfile << width << endl;
	auxfile << height << endl;
	auxfile.close();



	//Data saved for Main Rect
	x = 0; y = pt5.y; width = int(smallImg.cols); height = int(abs(pt5.y - pt8.y));
	Rect mainRect(x, y,width, height);

	ofstream mainfile("mainRect.csv");
	mainfile << 0 << endl;
	mainfile << int(pt7.y) << endl;
	mainfile << int(smallImg.cols) << endl;
	mainfile << int(abs(pt7.y - pt6.y)) << endl;
	mainfile.close();

	cv::rectangle(smallImgX, auxRect,cv::Scalar( 255, 255, 255 ), 4);
	cv::rectangle(smallImgX, mainRect,cv::Scalar( 0, 255, 0 ), 4);
	imwrite("MarkerRucoDetectofLeftRightAUX_Main_testRect.png", smallImgX);



	return false;
}




void runCalCam(){
	bool check = true;
	int step = 10;		//initialize increment step
	int newPos = MIN_POS + step;

	while(check){
		printf("Motor is moving to %i and conducting calibration\n", newPos);
		MoveTo(newPos);
		usleep(1000);

		check = CalCam();	// Rum calibration, return false if fail
		newPos = newPos + step;

		//If Motor reach to max position and failt to calibrate
		if (newPos > MAX_POS){
			printf("Fail Camera to Camera geometric calibration process due to no good images!!! \n");
			check = false;
		}
	}

}

#endif


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

	writeFloatData(fileName, tempInC); //curremt temparature reading
	writeStringData(fileName,",");


	writeFloatData(fileName, t_result); //curremt temparature reading
	writeStringData(fileName,",");

	sprintf(cmd, "wcr(0x0%i,0x30b4,0x21)", i);
    port_com_send(cmd);

    //writeStartNewLine(fileName);


    return tempInC;

}

void motorMove(){
	EyelockLog(logger, TRACE, "motorMove");
	int temp;
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
        writeStartNewLine(fileName);

        MoveTo(CENTER_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        writeStartNewLine(fileName);

        MoveTo(MIN_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        writeStartNewLine(fileName);

        MoveTo(CENTER_POS);
        cvWaitKey(WAIT);
        temp = calTemp(1);
        temp = calTemp(2);
        temp = calTemp(4);
        temp = calTemp(8);
        temp = calTemp(10);
        writeStartNewLine(fileName);

    }
}


void writeStringData(string fileName, string v){
	ofstream file(fileName, std::ios_base::out | std::ios_base::app);
	file << v;

	file.close();
}



void writeFloatData(string fileName, float v){
	ofstream file(fileName, std::ios_base::out | std::ios_base::app);
	file << v;

	file.close();
}


void writeStartNewLine(string fileName){
	ofstream file(fileName, std::ios_base::out | std::ios_base::app);
	file << std::endl;

	file.close();
}



void clearData(string fileName){
	ofstream file(fileName);
	file.clear();
	file.close();
}



int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");
/*	temp_log = fopen("tempLog", "a");
	//temp_log << "a " << ";" << "b" << endl;
	//temp_log << "c " << ";" << "d" << endl;
	fclose(temp_log);*/

	fstream infile(fileName);
	if(infile.good()){
		// cout << "File exist and keep writing in it!" << endl;
		EyelockLog(logger, DEBUG, "File exist and keep writing in it");
	}
	else{
		EyelockLog(logger, DEBUG, "Create Log file");
		// cout << "Create log file!" << endl;
		ofstream file(fileName);
	}

	clearData(fileName);
	writeStringData(fileName,"Move to command"); writeStringData(fileName,",");
	writeStringData(fileName,"Current angle"); writeStringData(fileName,",");
	writeStringData(fileName,"Move to angle "); writeStringData(fileName,",");
	writeStringData(fileName,"required time to move"); writeStringData(fileName,",");
	writeStringData(fileName,"Measure temp of Cam1"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Measure temp of Cam2"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Measure temp of Cam3"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Measure temp of Cam4"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Measure temp of CamFace"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");
	writeStringData(fileName,"Required time to measure temp"); writeStringData(fileName,",");

	writeStartNewLine(fileName);
	//writeData("std::endl");

	//file.close();

    int w,h;
    char key;

    int face_mode;
    int run_mode;
    int cal_mode=0;
    int cal_cam_mode = 0;		//initializing camera to camera calibration
    int temp_mode = 0;
    pthread_t threadId;
    pthread_t thredEcId;


	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);

	if (argc<2)
	{
		EyelockLog(logger, DEBUG, "error params");
		// printf("error params\n");
		exit (0);
	}


	if (argc== 3)
		run_mode=1;
	else
		run_mode =0;


	if (strcmp(argv[1],"cal")==0)
	{
		run_mode =1;
		cal_mode=1;
	}

	if (strcmp(argv[1],"send")==0)
	{
		void SendUdpImage(int port, char *image, int bytes_left);
		Mat imageIn, image;
		int x;


		image=imread("send_bad.pgm",0);
				image.convertTo(image,CV_8UC1);
				EyelockLog(logger, DEBUG, "sending udp  %d  %dbytes \n",image.cols*image.rows,image.dims);

				for (x=0; x<atoi(argv[2]);x++)
				{
					EyelockLog(logger, DEBUG, "send bad\n");
					SendUdpImage(8192, (char *)image.data, image.cols*image.rows);
					usleep(40000);
				}

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

	if (strcmp(argv[1],"calcam")==0)
	{
		EyelockLog(logger, DEBUG, "calcam mode is running");
		run_mode =1;
		cal_cam_mode=1;
	}


	if (strcmp(argv[1],"temp")==0)
	{
		EyelockLog(logger, DEBUG, "temperature test mode is running");
		run_mode =1;
		temp_mode=1;
	}

	if (run_mode)
		pthread_create(&threadId,NULL,init_tunnel,NULL);
	if (run_mode)
		pthread_create(&threadId,NULL,init_ec,NULL);


	if (cal_mode)
	{
		portcom_start();
		CalAll();
		return 0;
	}

	//for camera to camera calibration
	if (cal_cam_mode){
		portcom_start();
		DoStartCmd_CamCal();
		//runCalCam();
		return 0;

	}


	if (temp_mode){
		portcom_start();
		//DoStartCmd();
		motorMove();
		return 0;

	}


	//vid_stream_start(atoi(argv[1]));
	vs= new VideoStream(atoi(argv[1]));
	sprintf(temp,"Disp %d",atoi (argv[1]) );

	if (run_mode)
	{
		EyelockLog(logger, DEBUG, "run_mode");
		face_init();
		portcom_start();
		DoStartCmd();
	}

	cv::namedWindow(temp);

	if (vs->m_port==8192)
		vs->offset_sub_enable=0;
	if (vs->m_port==8193)
			vs->offset_sub_enable=0;
	while (1)
	{
		//printf("******before Recover***********\n");
		//  Ilya removed we dont know what its for
		//RecoverModeDrop();
		//printf("******after Recover***********\n");
		//if ((run_mode==0) || (run_state == RUN_STATE_FACE))
		{
		//	printf("******calling vid_stream_get***********\n");
			//vid_stream_get(&w,&h,(char *)outImg.data);
			vs->get(&w,&h,(char *)outImg.data);
		}
		//else
		{
			//printf("waiting for iris frame time\n");
			//cv::waitKey(IRIS_FRAME_TIME);
		}



		if (run_mode)
			{
			 DoRunMode();
			}
		else
			{
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
			imshow(temp,smallImg);  //Time debug
			}
	    key = cv::waitKey(1);
	    //printf("Key pressed : %u\n",key);
		if (key=='q')
			break;
		if (key=='c')
		{
			portcom_start();
			DoImageCal(1);
		}
			if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%d_%d.pgm",atoi(argv[1]),fileNum++);
			cv::imwrite(fName,outImg);
			printf("saved %s\n",fName);

		}
		static int aruc_on=0;



		if (key=='a')
			{
			aruc_on=~aruc_on;
			printf("set aruc %d\n",aruc_on);

			portcom_start();
			DoStartCmd_CamCal();


/*			//Homing
			printf("Re Homing\n");
			port_com_send("fx_home()\n");
			usleep(100000);

			port_com_send("fx_abs(168)\n");
			port_com_send("wcr(0x04,0x3012,12)");*/

			//r_outImg = rotation90(outImg);

			}
		if(aruc_on){
			//scaling = 2;
		    //cv::resize(outImg, smallImgBeforeRotate, cv::Size(),(1/scaling),(1/scaling), INTER_NEAREST);	//Mohammad
		    //smallImg = rotation90(smallImgBeforeRotate);
		    //cv::resize(smallImg, r_outImg, cv::Size(),(scaling),(scaling), INTER_NEAREST);
			//gridBooardMarker(smallImg);

			vs = new VideoStream(8194);
			//gridBooardMarker(outImg);
			delete (vs);
			//arucoMarkerTracking(outImg);
		}
		if(key=='b')
			{
				char fName[50];
				sprintf(fName,"%d_%d.bin",atoi(argv[1]),fileNum++);
				FILE *f = fopen("white.bin", "rb");
				int length = 1200*960;
				fwrite(outImg.data, length, 1, f);
				cv::imwrite(fName,outImg);
				printf("saved %s\n",fName);

			}

	}


}

