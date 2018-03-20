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
#define MIN_IRIS_FRAMES 10
int fileNum=0;
int move_counts=0;
void MoveTo(int v)
{
	char buff[100];
	if (v<MIN_POS) v= MIN_POS;
	if (v>MAX_POS) v=MAX_POS;
	//cvWaitKey(100);
	sprintf(buff,"fx_abs(%d)",v);
	printf("Sending move: %s\n",buff);
	port_com_send(buff);
	move_counts++;
	//cvWaitKey(100);
}

void accelStatus(){
	char buff[100];
	sprintf(buff,"accel()");
	printf("Sending accel command\n");
	port_com_send(buff);

/*	for(int i = 0; i <30; i++){
		printf("%c \n",buff[i]);

	}*/

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
		free=1;

	//printf("Current time : %u",start);
	if(free)
	{
		char temp[40];
		sprintf(temp,"fixed_set_rgbm(%d,%d,%d,%d)",R,G,B,mask);
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


#define FACE_GAIN_DEFAULT   0x80
#define FACE_GAIN_MAX       0xe0
#define FACE_GAIN_MIN       0x10
#define FACE_GAIN_PER_GOAL   1
#define FACE_GAIN_HIST_GOAL  0
#define FACE_CONTROL_GAIN   20.0

int agc_val= FACE_GAIN_DEFAULT;
int agc_set_gain =0;

int run_state=RUN_STATE_FACE;


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
void SetFaceMode()
{
	// enable face camera only
	// was 70
	//printf("switching to face mode : %s\n",GetTimeStamp());
	//port_com_send("grab_send(3)");
	//port_com_send("b_on_time(2,0,\"\")");
	//port_com_send("b_on_time(1,70,\"grab_send(4)\")");

	//port_com_send("fixed_set_rgb(10,10,10)");
	setRGBled(20,20,20,1000,0,0x1F);
	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
	start_mode_change = std::chrono::system_clock::now();
}

#define IRIS_FRAME_TIME 180

void SetIrisMode()
{
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,7) | psoc_write(3,0x31)| psoc_write(6,4)");
	cvWaitKey(200);
	IrisFrameCtr=0;
	start_mode_change = std::chrono::system_clock::now();
}

void DoStartCmd()
{
	setRGBled(BRIGHTNESS_MIN,BRIGHTNESS_MIN,BRIGHTNESS_MIN,10,0,0x1F);

	//port_com_send("set_audio(1)");
	//port_com_send("/home/root/tones/auth.wav");
	//start_mode_change = std::chrono::system_clock::now();

	//Mohammad
	//setting up face cameras and motor pos
	printf("Re Homing\n");
	port_com_send("fx_home()\n");
	cvWaitKey(6000);

	printf("Moving to Center\n");
	MoveTo(CENTER_POS);





	//printf("configuring face settings\n");
	//Face configuration of LED
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,1)");
	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");

	//Face cameras configuration
	port_com_send("wcr(4,0x3012,16) | wcr(4,0x301e,0) | wcr(4,0x305e,160)");

	//printf("configuring Main Iris settings\n");
	//Main Iris cameras configuration
	port_com_send("wcr(0x18,0x3012,16) | wcr(0x18,0x301e,0) | wcr(0x18,0x305e,40)");
	port_com_send("wcr(0x18,0x3040,0xC000)"); //Flipping of iris images


	//streaming Main and face cameras
	port_com_send("set_cam_mode(7,100)");
}


//Mohammad
void faceSettings(){
	printf("configuring face settings\n");
	//Face configuration of LED
	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,1)");
	//Face cameras configuration
	port_com_send("wcr(4,0x3012,12) | wcr(4,0x301e,0) | wcr(4,0x305e,160)");
}

void AuxIrisSettings(){
	printf("configuring Main Iris settings\n");
	//Iris configuration of LED
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,3) | psoc_write(3,3)| psoc_write(6,1)");
	//Face cameras configuration
	port_com_send("wcr(3,0x3012,12) | wcr(3,0x301e,0) | wcr(3,0x305e,40)");
}


void MainIrisSettings(){
	printf("configuring AUX Iris settings\n");
	//Iris configuration of LED
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(5,30) | psoc_write(4,3) | psoc_write(3,3)| psoc_write(6,1)");
	//Face cameras configuration
	port_com_send("wcr(0x83,0x3012,12) | wcr(0x83,0x301e,0) | wcr(0x83,0x305e,128)");
}


//Old
#define SCALE 3
Rect detect_area(80/SCALE,80/SCALE,1040/SCALE,800/SCALE);
//Rect no_move_area(0,200/SCALE,600/SCALE,50/SCALE);  //Old
//Rect no_move_area(0,220/SCALE,600/SCALE,50/SCALE);  //Mihir - 8mm Face Camera Lens
//Rect no_move_area(0,450/scaling,1200/scaling,100/scaling);  //Mihir - 3mm Face Camera Lens
Rect centerLine(1200/scaling, 960/scaling, 0, 0); //Mohammad center of the display
Rect no_move_area(0,560/scaling,960/scaling,80/scaling); //Mohammad rect at the center

void detectAndDisplay( Mat frame );
//std::printf("Detect face = %i\n",1);

int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size);
Point eyes;

//at 40inch or 106cm distance
#define MIN_FACE_SIZE 20		// previous val 40
#define MAX_FACE_SIZE 30		//// previous val 60



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
	printf("Iris with eyes %d\n",IrisFrameCtr);
	//cvWaitKey(30);
	if (IrisFrameCtr>MIN_IRIS_FRAMES)
		return 0;
	else
		return 1;
}


void DoRunMode()
{
  float eye_size;

	if (run_state == RUN_STATE_FACE)
	{
		    float p;
		    char temp_str[40];


		    int64 startTime = cv::getTickCount();

		    cv::resize(outImg, smallImgBeforeRotate, cv::Size(),(1/scaling),(1/scaling), INTER_NEAREST);	//Mohammad


		    smallImg = rotation90(smallImgBeforeRotate);

/*
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
			if (agc_set_gain!=agc_val && AGC_Counter%2==0)
				{
				sprintf(temp_str,"wcr(4,0x305E,0x%2x)",agc_val);
				port_com_send(temp_str);
				//printf("agc gain = %x p = %f\n",agc_val,p);
				agc_set_gain=agc_val;
				}
*/

		    accelStatus();


			if (FindEyeLocation( smallImg, eyes,eye_size))
				{

				if (detect_area.contains(eyes))
					{
					printf("eyes found\n");

					no_eye_counter=0;

					if (!no_move_area.contains(eyes))
						{
						if ((eye_size>= MIN_FACE_SIZE) && (eye_size<= MAX_FACE_SIZE))
							{
							int err;
							int constant = 10;
							int MoveToLimitBound = 10;
							err = (no_move_area.y+no_move_area.height/2) - eyes.y;
							//printf("err: %i\n", err);
							err = err * 3;
							//printf("err: %i\n", err);
							//err+=constant;
							//printf("err: %i\n", err);
							//err = err*SCALE;
							//err=err*-1;  //Image rotated by 180deg so no need to do this
							//err=err/MOTOR_STEP;  //Old
							//err=err/1;  //New
							//cur_pos+=err;
							err=err/MOTOR_STEP_MO;
							printf("err: %i, previous cur_pos: %i\n", err, cur_pos);
							cur_pos = cur_pos + err;
							printf("err: %i, New cur_pos: %i\n", err, cur_pos);
							//cur_pos = err + CENTER_POS;
							if (cur_pos<MIN_POS) cur_pos= POS_threshold + MIN_POS;
							if (cur_pos>MAX_POS) cur_pos=MAX_POS - POS_threshold;

							if (abs(err) > MoveToLimitBound){
								printf("Eyes out of range must move %d  current %d\n",err,cur_pos);
								MoveTo(cur_pos);
								printf("fx_abs val: %i\n", cur_pos);
								printf("Face is IN RANGE!!!!\n");
							}

							}
						else
							printf("Face out of range\n");
						}

					{
					if ((eye_size>= 25) && (eye_size<= 65))
					{
						SetIrisMode();
						run_state = RUN_STATE_EYES;
						printf("Switching ON IRIS LEDs!!!!\n");
						port_com_send("fixed_set_rgb(0,0,50)");
					}
				}

#if 0
					else
						{
						//Code for multistep motor movement
						int err;
						int constant = 10;
						err = (no_move_area.y+no_move_area.height/2) - eyes.y;
						err = err * 8;
						//err = err*SCALE;
						//err=err*-1;  //Image rotated by 180deg so no need to do this
						//err=err/MOTOR_STEP;  //Old
						//err=err/1;  //New
						//cur_pos+=err;
						err=err/MOTOR_STEP_MO;
						cur_pos = cur_pos + err;
						//cur_pos = err + CENTER_POS;
						if (cur_pos<MIN_POS) cur_pos= MIN_POS;
						if (cur_pos>MAX_POS) cur_pos=MAX_POS;
						printf("Eyes out of range must move %d  current %d\n",err,cur_pos);
						MoveTo(cur_pos);
						//motorMoveTo(cur_pos);
						}
#endif

					}

				}



				if (move_counts> MOVE_COUNTS_REHOME)
				{
					printf("Re Homing\n");
					setRGBled(0,0,0,100,0,0x1F);
					port_com_send("fx_home()\n");
					cvWaitKey(6000);
					MoveTo(CENTER_POS);
					setRGBled(20,20,20,100,0,0x1F);
					move_counts=0;
				}


			cv::rectangle(smallImg,no_move_area,Scalar( 255, 0, 0 ), 1, 0);
			imshow(temp,smallImg);
	}





  if (run_state == RUN_STATE_EYES)
  	  {
	  if (IrisFramesHaveEyes()==0)
	  	  {
			run_state = RUN_STATE_FACE;
			SetFaceMode();
	  	  }
  	  }
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
		RecoverModeDrop();
		if ((run_mode==0) || (run_state == RUN_STATE_FACE))
			vid_stream_get(&w,&h,(char *)outImg.data);
		else
		{
			printf("waiting for iris frame time\n");
			cv::waitKey(IRIS_FRAME_TIME);
		}


		if (run_mode)
			{
			 DoRunMode();
			}
		else
			{
			cv::resize(outImg, smallImg, cv::Size(), 0.5, 0.5, INTER_NEAREST); //Time debug
			imshow(temp,smallImg);  //Time debug
			}
	    key = cv::waitKey(1);
	    //printf("Key pressed : %u\n",key);
		if (key=='q')
			break;
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%d_%d.bmp",atoi(argv[1]),fileNum++);
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
