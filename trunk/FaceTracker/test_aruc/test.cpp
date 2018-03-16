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
#include <ctime>

using namespace cv;
#if 0 //Anita
using namespace std::chrono;
#endif
using namespace std;

VideoCapture TheVideoCapturer;

Mat TheInputImage, TheInputImageCopy, outImg, grImage, mask;
Mat smallImg;
Mat smallImgBeforeRotate;
int downScale = 4;
double scaling = 8;
//extern int IrisFramesHaveEyes(void);
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
//#define MOTOR_STEP 5  //For 8mm Lens
#define MOTOR_STEP 1 //For 3mm Lens
#define BRIGHTNESS_MAX 100
#define BRIGHTNESS_MIN 10
#define MODE_CHANGE_FREEZE 10
int cur_pos=CENTER_POS;
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
#if 0 //Anita
	static std::chrono::time_point<std::chrono::system_clock> start, end;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	if(elapsed_seconds.count()>=setTime || VIPcall==1)
		free=1;
#endif
	//printf("Current time : %u",start);
	if(free)
	{
		char temp[40];
		sprintf(temp,"fixed_set_rgbm(%d,%d,%d,%d)",R,G,B,mask);
		port_com_send(temp);
		free = 0;
#if 0 //Anita
		start = std::chrono::system_clock::now();
		setTime = (double)mtime/1000;
#endif
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

#if 0 //Anita
std::chrono::time_point<std::chrono::system_clock> start_mode_change;
#endif
void RecoverModeDrop()
{
	//If no mode change happens for a set amount of time then set face mode to recover from
	//from possible condition of loosing the mode change message
#if 0 //Anita
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
#endif
}
void SetFaceMode()
{
	// enable face camera only
	// was 70
	printf("switching to face mode : %s\n",GetTimeStamp());
	//port_com_send("grab_send(3)");
	//port_com_send("b_on_time(2,0,\"\")");
	//port_com_send("b_on_time(1,70,\"grab_send(4)\")");
	port_com_send("set_cam_mode(4,100)");
	//port_com_send("fixed_set_rgb(10,10,10)");
	//setRGBled(BRIGHTNESS_MAX,BRIGHTNESS_MAX,BRIGHTNESS_MAX,1000,0,0x1F);
#if 0 //Anita
	start_mode_change = std::chrono::system_clock::now();
#endif
}

#define IRIS_FRAME_TIME 180

void SetIrisMode()
{
	//printf("switching to  eyes mode : %s\n",GetTimeStamp());
	//port_com_send("grab_send(3)");
	printf("sent switching to  eyes mode : %s\n",GetTimeStamp());
	//port_com_send("b_on_time(1,0,\"\")");
	//port_com_send("b_on_time(1,180,\"grab_send(3)\")");
	port_com_send("set_cam_mode(3,100)");
	//port_com_send("fixed_set_rgb(0,0,10)");
	//setRGBled(0,0,BRIGHTNESS_MAX,1000,0);

	//wcr(3,0x305E,0x80)
	IrisFrameCtr=0;
#if 0 //Anita
	start_mode_change = std::chrono::system_clock::now();
#endif
}

void DoStartCmd()
{
	printf("Re Homing\n");
	//port_com_send("fixed_set_rgb(0,0,0)");
	//port_com_send("fx_home()\n");  //Mihir- temp
	//cvWaitKey(6000);
	//MoveTo(CENTER_POS);
	//port_com_send("fixed_set_rgb(10,10,10)");
	//setRGBled(BRIGHTNESS_MAX,BRIGHTNESS_MAX,BRIGHTNESS_MAX,10,0,0x1F);
	//port_com_send("psoc_write(2,24) | psoc_write(1,1) | psoc_write(4,7) | psoc_write(3,0x31) | psoc_write(5,30)");
	//port_com_send("wcr(7,0x3012,9) | wcr(7,0x301e,0) | wcr(7,0x305e,0x35)");
	//port_com_send("wcr(4,0x3012,4)");
	//port_com_send("wcr(7,0x3040,0xC000)");  //To rotate by 180 deg
	//port_com_send("b_on_time(1,70,\"grab_send(4)\")");
	//port_com_send("set_cam_mode(4,70)");
	//Increase gain for face cam
	//port_com_send("wcr(3,0x305E,0x60)");
	//port_com_send("wcr(4,0x305E,0x80)");
	//port_com_send("set_audio(1)");
	//port_com_send("/home/root/tones/auth.wav");
	//start_mode_change = std::chrono::system_clock::now();



	//Moahammad
	//setting up face cameras and motor pos
	port_com_send("fx_home()\n");
	cvWaitKey(6000);
	// MoveTo(CENTER_POS);
	port_com_send("set_cam_mode(7,100)");
	MoveTo(200);
	//Face settings
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(4,4) | psoc_write(3,4) |psoc_write(5,20) | psoc_write(6,4)");
	//port_com_send("wcr(4,0x3012,10) | wcr(4,0x301e,0) | wcr(4,0x305e,0xA0)");

	//Iris AUX camera settings
	port_com_send("psoc_write(2,40) | psoc_write(1,1) | psoc_write(4,3) | psoc_write(3,0x31) |psoc_write(5,40) | psoc_write(6,6)");
	//port_com_send("wcr(131,0x3012,10) | wcr(131,0x301e,0) | wcr(131,0x305e,128)");
	port_com_send("wcr(3,0x3012,16) | wcr(3,0x301e,0) | wcr(3,0x305e,80)");
}
//Old
#define SCALE 3
Rect detect_area(100/SCALE,00/SCALE,400/SCALE,500/SCALE);
//Rect no_move_area(0,200/SCALE,600/SCALE,50/SCALE);  //Old
//Rect no_move_area(0,220/SCALE,600/SCALE,50/SCALE);  //Mihir - 8mm Face Camera Lens
Rect no_move_area(0,450/scaling,1200/scaling,100/scaling);  //Mihir - 3mm Face Camera Lens
void detectAndDisplay( Mat frame );
//std::printf("Detect face = %i\n",1);
int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size);
Point eyes;


#define MIN_FACE_SIZE 40
#define MAX_FACE_SIZE 65

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


void DoRunMode()
{
  float eye_size;

	if (run_state == RUN_STATE_FACE)
	{
		    float p;
		    char temp_str[40];

			/// create the small image

		    /*
		     *
		    static int test_count=0;
		    test_count=test_count+5;
		    if (test_count>120)
		    	test_count =5;

		    SelLedDistance(test_count);
*/


/*		    printf("cols: %d    Rows: %d\n",outImg.cols, outImg.rows);
		    printf("cols: %d    Rows: %d\n",outImg.cols/2, outImg.rows/2);
		    printf("cols: %d    Rows: %d\n",outImg.cols/4, outImg.rows/4);
		    printf("cols: %d    Rows: %d\n",outImg.cols/8, outImg.rows/8);
		    printf("cols: %d    Rows: %d\n",outImg.cols/16, outImg.rows/16);*/

		    //printf("cols: %d    Rows: %d\n",outImg.cols, outImg.rows);
		    int64 startTime = cv::getTickCount();

		    cv::resize(outImg, smallImgBeforeRotate, cv::Size(),(1/scaling),(1/scaling), INTER_NEAREST);	//Mohammad

/*		    //pyramid test
		    pyrDown(outImg, smallImgBeforeRotate, Size(outImg.cols/downScale, outImg.rows/downScale));
		    pyrDown(smallImgBeforeRotate, smallImgBeforeRotate, Size(smallImgBeforeRotate.cols/downScale, smallImgBeforeRotate.rows/downScale));
		    pyrDown(smallImgBeforeRotate, smallImgBeforeRotate, Size(smallImgBeforeRotate.cols/downScale, smallImgBeforeRotate.rows/downScale));
		    pyrDown(smallImgBeforeRotate, smallImgBeforeRotate, Size(smallImgBeforeRotate.cols/downScale, smallImgBeforeRotate.rows/downScale));
		    */

		    smallImg = rotation90(smallImgBeforeRotate);
		    //printf("cols: %i    Rows: %i\n",smallImg.cols, smallImg.rows);

/*		    int64 lastTime = cv::getTickCount();
		    double tm = (lastTime - startTime)/cv::getTickFrequency();

		    fileName << t << "," << tm << endl;

		    imwrite("x.bmp", smallImg);*/


		    //detectAndDisplay(smallImg);


			p = AGC(smallImg.cols,smallImg.rows,(unsigned char *)(smallImg.data));

		//	printf("P = %d\n",p);
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


			if (FindEyeLocation( smallImg, eyes,eye_size))
				{

				  //cv::rectangle(smallImg,eyes,Scalar( 127, 0, 0 ), 1, 0);
				  //ellipse( smallImg, eyes, Size( 20,20), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
				  printf("Working121321321321321321321321321321321321321321321321321");
				if (detect_area.contains(eyes))
					{
					printf("eyes found\n");
					//if (no_eye_counter!=0)
					//	port_com_send("fixed_set_rgb(10,10,10)");

					no_eye_counter=0;
					if (no_move_area.contains(eyes))
						{
						if (1)
						{
						char temp[100];
						int val;
						val =  (eye_size-10)*100/55; //Ask Ilya
						//if (val<25) val =0;
						if (val>100) val = 120; //ask ilya

					    printf("Val = %d size = %d\n",val,eye_size);

					    //Logic for too close
					    if(agc_val==FACE_GAIN_MIN)
					    {
					    	//SelLedDistance(150);
					    	setRGBled(120,0,0,500,0,0x4);
					    }
					    else
					    {
					    	//if(val>25)
					    		//SelLedDistance(val);
					    }

						//sprintf(temp,"fixed_set_rgb(%d,%d,%d)",val,val,val);
						//port_com_send(temp);
						//setRGBled(val,val,val,500,0); // multi leveled brightness
						//setRGBled(BRIGHTNESS_MAX,BRIGHTNESS_MAX,BRIGHTNESS_MAX,500,0,0x1F);  //Two leveled brightness
						}
						if ((eye_size>= MIN_FACE_SIZE) && (eye_size<= MAX_FACE_SIZE))
							{
							run_state = RUN_STATE_EYES;
							printf("Face is IN RANGE!!!!\n");
							SetIrisMode();
							}
						else
							printf("Face out of range\n");
						}
					else
						{
#if 0 //15Mar -face
						int err;

						err = (no_move_area.y+no_move_area.height/2) - eyes.y;
						err = err*SCALE;
						//err=err*-1;  //Image rotated by 180deg so no need to do this
						err=err/MOTOR_STEP;  //Old
						//err=err/1;  //New
						cur_pos+=err;
						if (cur_pos<MIN_POS) cur_pos= MIN_POS;
						if (cur_pos>MAX_POS) cur_pos=MAX_POS;
						printf("Eyes out of range must move %d  current %d\n",err,cur_pos);
						MoveTo(cur_pos);
#endif
						}
					}
				}
			else
				{
				//printf("No eyes %d\n",no_eye_counter);  //Mihir commented
				if (no_eye_counter>=5)
				{
					//port_com_send("fixed_set_rgb(10,10,10)");
					//printf("No Eyes for 10 frames\n");
					setRGBled(BRIGHTNESS_MAX,BRIGHTNESS_MAX,BRIGHTNESS_MAX,1000,0,0x1F);
				}

				if (no_eye_counter<NO_EYE_HOME_FRAMES+2)
					no_eye_counter++;
				if (no_eye_counter==NO_EYE_CENTER_FRAMES)
					{
					printf("Recentering no eyse\n");
			//		port_com_send("fx_home()\n");
				//	usleep(100000);
					agc_val=FACE_GAIN_DEFAULT;
					//MoveTo(CENTER_POS);
					}
#define MOVE_COUNTS_REHOME 30
				if
				(
						//(no_eye_counter==NO_EYE_HOME_FRAMES)||
						( move_counts> MOVE_COUNTS_REHOME))
							{
							printf("Re Homing\n");
							//port_com_send("fixed_set_rgb(0,0,0)");
							//setRGBled(0,0,0,10,0,0x1F);
							port_com_send("fx_home()\n");
							cvWaitKey(6000);
							MoveTo(CENTER_POS);
						    //port_com_send("fixed_set_rgb(10,10,10)");
						    //setRGBled(BRIGHTNESS_MAX,BRIGHTNESS_MAX,BRIGHTNESS_MAX,100,0,0x1F);
						    move_counts=0;
							}

				}

		//	cv::rectangle(smallImg,detect_area,Scalar( 255, 0, 0 ), 8, 0);
			cv::rectangle(smallImg,no_move_area,Scalar( 127, 0, 0 ), 2, 0);

			//imwrite("x.bmp", smallImg);
			imshow(temp,smallImg);
	}
  if (run_state == RUN_STATE_EYES)
  	  {
	  if (IrisFramesHaveEyes()==0)
	  	  {
			run_state = RUN_STATE_FACE;
			agc_val=FACE_GAIN_DEFAULT;
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

    //printf("%d\n", argc);

    //Subsampled
    //Rect detect_area(50,50,250,190);
    //Rect no_move_area(5,5,5,5);

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

	//run_mode = atoi(argv[2]);
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
		//MoveTo(cur_pos);
		 DoStartCmd();
		//Mihir - Testing
//		while(1)
//		{
//			MoveTo(MIN_POS);
//			cvWaitKey(2000);
//			MoveTo(MAX_POS);
//			cvWaitKey(2000);
//		}
		}

	cv::namedWindow(temp);

/*

	//experimenting motor cycle calc
	int countCycle = 0;

	clock_t startTime = clock();
	while(countCycle < 50){
		motorCycleTest(328);	//Max value that represt to go up is 328
		countCycle++;
		printf("cycle : %d\n", countCycle);

	}
	clock_t stopTime = clock();
	double elapsed = double(stopTime - startTime) * 1000.0 / CLOCKS_PER_SEC;
	printf("elapsedTime: %d\n", elapsed);

*/



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

//		imshow(temp,smallImg);
		if (run_mode)
			{
			 DoRunMode();
			}
		else
			{
			cv::resize(outImg, smallImg, cv::Size(), 0.5, 0.5, INTER_NEAREST); //Time debug
			//Mat rotateImg = rotation90(smallImg, run_mode);
			{
				//int EyeFinderNew(cv::Mat &mt);
				//EyeFinderNew(outImg);
			}
			imshow(temp,smallImg);  //Time debug

			//printf("Got Image : %s\n",GetTimeStamp());
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
