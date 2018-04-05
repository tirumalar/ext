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
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <time.h>



#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/photo/photo.hpp>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>
#include "eyelock_com.h"

#include <chrono>
#include <time.h>
#include <fstream>

using namespace cv;
using namespace std::chrono;
using namespace std;

VideoCapture TheVideoCapturer;

Mat TheInputImage, TheInputImageCopy, outImg, grImage, mask;
Mat smallImg;
Mat frame_gray;
Mat smallImgBeforeRotate;
int downScale = 4;

//experiment purposes
ofstream fileName("processedTimePyr16.csv");
string t = "Processing time: ";
double scaling = 8;		//8


extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern int portcom_start();
extern void port_com_send(char *cmd);


int z=0;

#define WIDTH 1200
#define HEIGHT 960

int face_init(  );

#define NO_EYE_CENTER_FRAMES 15
#define NO_EYE_HOME_FRAMES 400
#define CENTER_POS 164		//low fx_abs val is 0 and max is 328. So the center falls in 164
#define CENTER_POSITION_ANGLE 95
#define MIN_POS 0
#define MAX_POS 328			//previous val was 200
#define POS_threshold 28
//#define MOTOR_STEP 5  //For 8mm Lens
#define MOTOR_STEP 1 //For 3mm Lens, value was 3
#define MOTOR_STEP_MO 2
#define BRIGHTNESS_MAX 10
#define BRIGHTNESS_MIN 10
#define MODE_CHANGE_FREEZE 10
int cur_pos=CENTER_POS;
#define MOVE_COUNTS_REHOME 7
#define MIN_IRIS_FRAMES 100
int fileNum=0;
int move_counts=0;
#define FRAME_DELAY 40

// this defines how many frames without a face will cause a switch back to face mode ie look for faces
#define NO_FACE_SWITCH_FACE 10


float read_angle(void);

float home_angle=90.0;
int noeyesframe = 0;

#define ANGLE_TO_STEPS 5



void faceSettings();
void AuxIrisSettings();
void MainIrisSettings();


#define PIXEL_TOTAL 900

void SetExp(int cam, int val)
{
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	int fine = val - coarse*PIXEL_TOTAL;

	sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	port_com_send(buff);



}

void MoveToAngle(float a)
{
	char buff[100];
	float current_a = read_angle();
	if (current_a == 0)
		return;
	float move;
	move = current_a-a;
	printf("angle diff %3.3f\n",move);
	move=-1*move*ANGLE_TO_STEPS;

	sprintf(buff,"fx_rel(%d)",(int)(move+0.5));
	printf("Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
	port_com_send(buff);
}
void MoveRelAngle(float a)
{

	// add a limit check to make sure we are not out of bounds
	char buff[100];
	float move;
	float current_a = read_angle();
	printf("Current_a=%f; next_a=%f\n",current_a,a);

	/*if (a > 0){
		if ((current_a + (-a)) > 120){
			printf("Hitting Max!!\n");
			sprintf(buff,"fx_abs(%d)",MAX_POS);
			port_com_send(buff);
			return;
		}
	}
	else
	{
		if ((current_a-(-a)) < 69)
		{
			sprintf(buff,"fx_home()");
			port_com_send(buff);
			return;
		}
	}*/




	move=-1*a*ANGLE_TO_STEPS;


	sprintf(buff,"fx_rel(%d)",(int)move);
	//printf("Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
	port_com_send(buff);
}

void MoveTo(int v)
{
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

	printf("Move to %d ",v);
	v=v-CENTER_POS;
	v=v/ANGLE_TO_STEPS+CENTER_POSITION_ANGLE;
	printf("angle = %d",v);
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
void motorMoveTo(int v)
{
	char buff[100];
	char buffRe[100];
	if (v<MIN_POS) v= MIN_POS;
	if (v>MAX_POS) v=MAX_POS;
	//cvWaitKey(100);
	sprintf(buffRe,"fx_home()");
	port_com_send(buffRe);
	sprintf(buff,"fx_abs(%d)",v);
	printf("Sending move: %s\n",buff);
	port_com_send(buff);
	move_counts++;
	//cvWaitKey(100);
}

//cycle experiment. This code is not part of the main code
void motorCycleTest(int up){
	int center = up/2;	//center calc

	char bufferBottom[100], buffCent[100], bufferUp[100];

	//cvWaitKey(100);
	sprintf(bufferBottom,"fx_home()");
	sprintf(buffCent,"fx_abs(%d)",center);
	sprintf(bufferUp,"fx_abs(%d)",up);
	sprintf(buffCent,"fx_abs(%d)",center);

	port_com_send(bufferBottom);	//go to bottom or home
	port_com_send(buffCent);		// go to center
	port_com_send(bufferUp);		// go up

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


int no_eye_counter =0;

extern void  *init_tunnel(void *arg);
extern void *init_ec(void * arg);

int IrisFrameCtr=0;
#include <sys/time.h>

void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask)
{
	static int free = 1;
	static int setTime = 0;

	static std::chrono:: time_point<std::chrono::system_clock> start, end;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	if(elapsed_seconds.count()>=setTime || VIPcall==1)
	{
		free=1;

	}


	//printf("Current time : %u",start);
	if(free)
	{
		char temp[40];
		//sprintf(temp,"fixed_set_rgbm(%d,%d,%d,%d)",R,G,B,mask);
		sprintf(temp,"fixed_set_rgb(%d,%d,%d)",R,G,B);
		port_com_send(temp);
		free = 0;
		start = std::chrono::system_clock::now();
		setTime = (double)mtime/1000;
	}

}

void SelLedDistance(int val) // val between 0 and 100
{

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

}
char* GetTimeStamp()
{
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
#define FACE_GAIN_PER_GOAL   1
#define FACE_GAIN_HIST_GOAL  0
#define FACE_CONTROL_GAIN   100.0

int agc_val= FACE_GAIN_DEFAULT;
int agc_set_gain =0;

int run_state=RUN_STATE_FACE;




//Mohammad
void faceSettings(){

	//printf("configuring face settings\n");
//	return;
	//Face configuration of LED
	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,1)");
	//Face cameras configuration
	port_com_send("wcr(4,0x3012,12) | wcr(4,0x301e,0) | wcr(4,0x305e,160)");
}

void AuxIrisSettings(){

	//printf("configuring Aux Iris settings\n");
	//Iris configuration of LED
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,40) | psoc_write(4,3) | psoc_write(3,0x31)| psoc_write(6,4)");
	//Face cameras configuration
	/*port_com_send("wcr(3,0x3012,12) | wcr(3,0x301e,0) | wcr(3,0x305e,128)");
	port_com_send("wcr(3,0x3040,0xC000)"); //Flipping of iris images*/

}


void MainIrisSettings(){

	//return;
	printf("configuring Main Iris settings\n");
	//Iris configuration of LED
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,3) | psoc_write(3,0x31)| psoc_write(6,1)");
	//Face cameras configuration
	//port_com_send("wcr(0x83,0x3012,12) | wcr(0x83,0x301e,0) | wcr(0x83,0x305e,128)");
}

std::chrono:: time_point<std::chrono::system_clock> start_mode_change;
void RecoverModeDrop()
{
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
	printf("SWITCHING cameras------------------->");
	char cmd[100];
	if (mode)
		sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	else
		sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);

	port_com_send(cmd);
}

#define MODE_FACE 1
#define MODE_EYES 2
int currnet_mode =0;
void SetFaceMode()
{
	// enable face camera only
	// was 70
	//printf("switching to face mode : %s\n",GetTimeStamp());
	//port_com_send("grab_send(3)");
	//port_com_send("b_on_time(2,0,\"\")");
	//port_com_send("b_on_time(1,70,\"grab_send(4)\")");

	//port_com_send("fixed_set_rgb(10,10,10)");
	//setRGBled(20,20,20,1000,0,0x1F);
	if (currnet_mode==MODE_FACE)
		return;
//	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
	port_com_send("psoc_write(2,30) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)");
	port_com_send("wcr(4,0x3012,12) | wcr(4,0x305e,0xF0)"); //| wcr(4,0x301e,0)
	start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_FACE;
}

#define IRIS_FRAME_TIME 180

void SetIrisMode(int eye_distance)
{
	// later switch cameras

	if (currnet_mode==MODE_EYES)
		return;
	IrisFrameCtr=0;
	printf("Set Iris Mode is Active!!!");
	AuxIrisSettings();


/*	//switching cameras
	if (eye_distance > 40)
	{
		SwitchIrisCameras(true);
		MainIrisSettings();
		printf("Main Cameras\n");
	}
	else
	{
		SwitchIrisCameras(false);
		AuxIrisSettings();
		printf("AUX Cameras\n");
	}*/


/*
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,7) | psoc_write(3,0x31)| psoc_write(6,4)");
	port_com_send("wcr(4,0x3012,1) | wcr(4,0x301e,0) | wcr(4,0x305e,10)");
*/
//| wcr(4,0x301e,0)
	//printf("Dimming face cameras!!!");
	port_com_send("wcr(4,0x3012,1)  | wcr(4,0x305e,40)");

	//cvWaitKey(200);
	IrisFrameCtr=0;
	start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_EYES;
}



void SetIrisMode_a()
{
	//return;
	printf("Set Iris Mode is Active!!!");
	AuxIrisSettings();


/*
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,7) | psoc_write(3,0x31)| psoc_write(6,4)");
	port_com_send("wcr(4,0x3012,1) | wcr(4,0x301e,0) | wcr(4,0x305e,10)");
*/
	printf("Dimming face cameras!!!");
	port_com_send("wcr(4,0x3012,1) | wcr(4,0x301e,0) | wcr(4,0x305e,10)");
	//cvWaitKey(200);
	IrisFrameCtr=0;
	start_mode_change = std::chrono::system_clock::now();
}




void SetIrisMode_m()
{
	//return;
	printf("Set Iris Mode is Active!!!");
	AuxIrisSettings();


/*
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,7) | psoc_write(3,0x31)| psoc_write(6,4)");
	port_com_send("wcr(4,0x3012,1) | wcr(4,0x301e,0) | wcr(4,0x305e,10)");
*/
	printf("Dimming face cameras!!!");
	port_com_send("wcr(4,0x3012,1) | wcr(4,0x301e,0) | wcr(4,0x305e,10)");
	//cvWaitKey(200);
	IrisFrameCtr=0;
	start_mode_change = std::chrono::system_clock::now();
}


void DoStartCmd()
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
	//port_com_send("wcr(0x18,0x3012,12) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,64)");
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

	//This code is for playing sound
	if(1)
	{
		port_com_send("set_audio(1)");
		system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
		sleep(2);
	}

#if 0
	while (1)
	{
	 char b[100];
	 int a;
	 printf("Enter angle\n");
	 scanf("%d",&a);
	 printf("Moving to %d\n",a);
	 MoveTo(a);
	}
#endif
}


//Old
#define SCALE 3
Rect detect_area(30/SCALE,30/SCALE,(960-30*2)/(SCALE*SCALE),(1200-30*2)/(SCALE*SCALE));
//Rect no_move_area(0,200/SCALE,600/SCALE,50/SCALE);  //Old
//Rect no_move_area(0,220/SCALE,600/SCALE,50/SCALE);  //Mihir - 8mm Face Camera Lens
//Rect no_move_area(0,450/scaling,1200/scaling,100/scaling);  //Mihir - 3mm Face Camera Lens
Rect centerLine(1200/scaling, 960/scaling, 0, 0); //Mohammad center of the display
//Rect no_move_area(0,560/scaling,960/scaling,80/scaling); //Mohammad rect at the center
//Rect no_move_area(0,660/scaling,960/scaling,160/scaling); //Mohammad rect at the center 580, the value 80 we moved to 160
//Rect no_move_area(0,580/scaling,960/scaling,80/scaling); //Mohammad rect at the center 580, the value 80 we moved to 160

//follwoing command is for test demo 1 no_move_area
Rect no_move_area(0,52,120,14); //Mohammad rect at the center 580, the value 80 we moved to 160

/*

//follwoing command is for test demo 2 no_move_area
Rect no_move_area(0,64,120,14); //Mohammad rect at the center 580, the value 80 we moved to 160
*/


//Rect detect_area_center(120/SCALE,120/SCALE,(960-120*2)/(SCALE*SCALE),1200/SCALE)
//Rect no_move_area_aux(0,660/scaling,960/scaling,160/scaling); //Mohammad rect at the center 580


void detectAndDisplay( Mat frame );
//std::printf("Detect face = %i\n",1);

int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size);
Point eyes;

//at 40inch or 106cm distance
#define MIN_FACE_SIZE 20		// previous val 40
#define MAX_FACE_SIZE 65		//// previous val 60



extern int IrisFramesHaveEyes();
char temp[100];



float AGC(int width, int height,unsigned char *dsty)
{

	//Percentile and Average Calculation
	unsigned char *dy = (unsigned char *)dsty;
	double total = 0,Ptotal = 0,percentile = 0,hist[256]={0},average=0;
	int pix=0,i;
	int n = width * height;
	int limit = 200;    // Lower limit for percentile calculation
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

	//printf("average : %3.1f percentile : %3.1f\n",average,percentile);
	return (float)percentile;
}

int AGC_Counter = 0;

Mat rotate(Mat src, double angle){
	Mat dst;
    Point2f pt(src.cols*0.5, src.rows*0.5);
    Mat M = cv::getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(src, dst, M, src.size());
    //cv::warpAffine(src, dst, M, smallImgBeforeRotate.size(),cv::INTER_CUBIC);

    M.release();

    return dst;
}

Mat rotation90(Mat src){
	Mat dst;
	transpose(src, dst);
	flip(dst,dst,0);
	return dst;

}


int IrisFramesHaveEyes()
{
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
  float eye_size;

	// if (run_state == RUN_STATE_FACE)
	{
		    float p;
		    char temp_str[40];



		    int64 startTime = cv::getTickCount();

		    cv::resize(outImg, smallImgBeforeRotate, cv::Size(),(1/scaling),(1/scaling), INTER_NEAREST);	//Mohammad


		    smallImg = rotation90(smallImgBeforeRotate);



//FACE_GAIN_DEFAULT   0x80		128
//       0xe0		224
//FACE_GAIN_MIN       0x10		16
//FACE_GAIN_PER_GOAL   1
//FACE_GAIN_HIST_GOAL  0
//FACE_CONTROL_GAIN   20.0

//int agc_val= FACE_GAIN_DEFAULT;		128
//int agc_set_gain =0;


		    p = AGC(smallImg.cols,smallImg.rows,(unsigned char *)(smallImg.data));


			if (p<FACE_GAIN_PER_GOAL-FACE_GAIN_HIST_GOAL)
				agc_val= agc_val + (FACE_GAIN_PER_GOAL-p)*FACE_CONTROL_GAIN;
			if (p>FACE_GAIN_PER_GOAL+FACE_GAIN_HIST_GOAL)
				agc_val= agc_val + (FACE_GAIN_PER_GOAL-p)*FACE_CONTROL_GAIN;
			agc_val = MAX(FACE_GAIN_MIN,agc_val);
			agc_val = MIN(agc_val,FACE_GAIN_MAX);
			//Too close
//			if(agc_val==FACE_GAIN_MIN)
//			{
//				//SelLedDistance(150);
//				setRGBled(120,0,0,500,0,0x4);
//			}
			AGC_Counter++;
			if (agc_set_gain!=agc_val);// && AGC_Counter%2==0)
				{
			//	while (waitKey(10) != 'z');
				{
				static int agc_val_old=0;
				  if (abs(agc_val-agc_val_old)>300)
				  	  {
					//  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
					  SetExp(4,agc_val);
					  agc_val_old= agc_val;
				  	  }
				}

				agc_set_gain=agc_val;
				}



///////////////////////////////////////////////////////////////////////////////////
			if (FindEyeLocation( smallImg, eyes,eye_size))
				{
				noeyesframe = 0;

				if (detect_area.contains(eyes))
					{
					printf("eyes found\n");

					no_eye_counter=0;
					noFaceCounter=0;

					#define ERROR_LOOP_GAIN 0.08

					//if (!no_move_area.contains(eyes))
					if (!no_move_area.contains(eyes))
						{
						if ((eye_size>= MIN_FACE_SIZE) && (eye_size<= MAX_FACE_SIZE))
							{
							float err;
							int constant = 10;
							int MoveToLimitBound = 1;
							err = (no_move_area.y+no_move_area.height/2) - eyes.y;
							//printf("err: %i\n", err);
							err = (float)err * (float)SCALE * (float)ERROR_LOOP_GAIN;
							//err=err/MOTOR_STEP_MO;
							//cur_pos = err + CENTER_POS;
							//if (cur_pos<MIN_POS) cur_pos= POS_threshold + MIN_POS;
							//if (cur_pos>MAX_POS) cur_pos=MAX_POS - POS_threshold;

							////Draw images on the eyes
							//cv::rectangle(smallImg,Rect(eyes.x-5,eyes.y-5,10,10),Scalar( 255, 0, 0 ), 5, 0);


							// if we need to move
							if (abs(err) > MoveToLimitBound)
								{
									int x,w,h;

									MoveRelAngle(-1*err);
								//	while (waitKey(10) != 'z');
								//	printf("fx_abs val: %i\n", -1*err);
								//	printf("Face is IN RANGE!!!!\n");

									//flush the video buffer to get rid of frames from motion
									{
										int vid_stream_flush(void);
										vid_stream_flush();
									}
									// might need this vid_stream_get(&w,&h,(char *)outImg.data);
								}




							}
						else
							printf("Face out of range\n");
						}
					else
						{
						cv::rectangle(smallImg,Rect(eyes.x-5,eyes.y-5,10,10),Scalar( 255, 0, 0 ), 2, 0);
						SetIrisMode(eye_size);
						run_state = RUN_STATE_EYES;
						printf("Switching ON IRIS LEDs!!!!\n");

						//port_com_send("fixed_set_rgb(0,0,50)");
						}

					}

				}
			{
//				noeyesframe++;
				if (noFaceCounter<(NO_FACE_SWITCH_FACE+2))
					noFaceCounter++;

				if (noFaceCounter==NO_FACE_SWITCH_FACE)
			  	  {
					run_state = RUN_STATE_FACE;
					SetFaceMode();
					MoveTo(CENTER_POS);
			  	  }

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


			cv::rectangle(smallImg,no_move_area,Scalar( 255, 0, 0 ), 1, 0);
			cv::rectangle(smallImg,detect_area,Scalar( 255, 0, 0 ), 1, 0);
			imshow(temp,smallImg);
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

int main(int argc, char **argv)
{
    int w,h;
    char key;

    int face_mode;
    int run_mode;
    pthread_t threadId;
    pthread_t thredEcId;


	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);

	if (argc<2)
	{
		printf("error params\n");
		exit (0);
	}
	if (argc== 3)
		run_mode=1;
	else
		run_mode =0;

	if (run_mode)
		pthread_create(&threadId,NULL,init_tunnel,NULL);
	if (run_mode)
		pthread_create(&threadId,NULL,init_ec,NULL);

	vid_stream_start(atoi(argv[1]));
	sprintf(temp,"Disp %d",atoi (argv[1]) );





	if (run_mode)
		{
		face_init();
		portcom_start();
		DoStartCmd();
		}

	cv::namedWindow(temp);


	while (1)
	{
		//printf("******before Recover***********\n");
		//  Ilya removed we dont know what its for
		//RecoverModeDrop();
		//printf("******after Recover***********\n");
		//if ((run_mode==0) || (run_state == RUN_STATE_FACE))
		{
		//	printf("******calling vid_stream_get***********\n");
			vid_stream_get(&w,&h,(char *)outImg.data);
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
				printf("sub\n");
			}
			else
				cv::resize(outImg, smallImg, cv::Size(), 1, 1, INTER_NEAREST); //Time debug

			imshow(temp,smallImg);  //Time debug
			}
	    key = cv::waitKey(1);
	    //printf("Key pressed : %u\n",key);
		if (key=='q')
			break;
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%d_%d.pgm",atoi(argv[1]),fileNum++);
			cv::imwrite(fName,outImg);
			printf("saved %s\n",fName);

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

/************************************
 *
 *
 *
 *
 ************************************/
