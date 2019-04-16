/*
 * CameraCalibrationGain.cpp
 *
 *  Created on: Apr 10, 2019
 *      Author: root
 */

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

//#include "extFocus.h"
//#include "geomtericCalibration.h"

using namespace cv;
//using namespace std::chrono;
using namespace std;

//#define MODE_CHANGE_FREEZE 10
//#define FRAME_DELAY 100
//Controlling states between face and Eyes
//#define RUN_STATE_FACE 0
//#define RUN_STATE_EYES 1


int currnet_mode = 0;
int previousEye_distance = 0;
const char logger[30] = "ExtSetUp";
// const char stateMachineLogger[30] = "StateMachine";
VideoStream *vs;
cv::Mat outImg, smallImg;
//Reading from faceConfig.ini
int CENTER_POS;
int IrisFrameCtr = 0;		//used for counting Iris Frame
int cur_pos=CENTER_POS;
int move_counts=0;

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

//int run_state=RUN_STATE_FACE;
//std::chrono:: time_point<std::chrono::system_clock> start_mode_change;


//void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
//void SelLedDistance(int val);
//char* GetTimeStamp();
//void RecoverModeDrop();
//void setCamera(string cam, int delay);
//void setCameraStreaming(int cam);
//bool runCalibration(geomtericCalibration gc, bool calDebug);
//void startGeometricCalibrationApp(geomtericCalibration gc, bool calDebug);
//void streamVideoFocus(extFocus fs, int cam);
//void startFocusApp();
//void RunCamFocus(extFocus fs);


//These are used by gain calibration

char cmd[512];
VideoStream *vsLeft;
VideoStream *vsRight;
VideoStream *vsFace;


/*

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


///////////////////////////////////////////////////New calibration app structure/////////////////
//Run calibration until meet all the condition based on brightness changes and motor movement
void setCamera(string cam, int delay){
	char cmd[512];

	sprintf(cmd,"set_cam_mode(%s,%d)",cam.c_str(), delay);
	port_com_send(cmd);
	usleep(100);
}


void setCameraStreaming(int cam){
	int w,h;

	vs = new VideoStream(cam);
	vs->flush();
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
}


bool runCalibration(geomtericCalibration gc, bool calDebug){
	char buff[512], cmd[512], cmd1[512],c;
	Mat DisImg(100, 900, CV_8UC3, Scalar(0,0,0));
	Mat DisImgx(600, 900, CV_8UC3, Scalar(0,0,0));

	string camID;
	int dealy = 100;	//cam delay (streaming speed)

	//setup aux left and right camera
	camID = "0x87";
	setCamera(camID, dealy);


	//Fetching images from Aux right
	setCameraStreaming(8193);

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxRight = gc.gridBooardMarker(outImg,vs->cam_id, calDebug);
	//If detected markers less then 2
	if (markerIrisAuxRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Fail to detect markers from Aux right camera");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}
	delete (vs);



	//Fetching images from Aux left
	setCameraStreaming(8192);

	//Detecting markers
	std::vector<aruco::Marker> markerIrisAuxleft = gc.gridBooardMarker(outImg,vs->cam_id, calDebug);
	if (markerIrisAuxleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Fail to detect markers from Aux left camera");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}
	delete (vs);


	//setup main left and right camera
	camID = "0x07";
	setCamera(camID, dealy);

	setCameraStreaming(8193);
	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainRight = gc.gridBooardMarker(outImg,vs->cam_id, calDebug);
	if (markerIrisMainRight.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Fail to detect markers from main right camera");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}
	delete (vs);


	setCameraStreaming(8192);
	//Detecting markers
	std::vector<aruco::Marker> markerIrisMainleft = gc.gridBooardMarker(outImg,vs->cam_id, calDebug);
	if (markerIrisMainleft.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Fail to detect markers from Main left camera");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}
	delete (vs);


	setCameraStreaming(8194);
	//Detecting markers
	std::vector<aruco::Marker> markerFace = gc.gridBooardMarker(outImg,vs->cam_id, calDebug);
	if (markerFace.size() < 2){
		delete (vs);
		printf("Not enough (less then 2 ) markers detected to Run calibration!\n ");
		sprintf(buff,"wcr(%d,0x3012,%d)",4,3);
		//port_com_send(buff);

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Fail to detect markers from Face camera");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	//initializing vectors for co-ordinate calc
	vector<float> rectLeftAux, rectRightAux, rectLeftMain, rectRightMain;
	int row = outImg.rows;
	int col = outImg.cols;

	printf("Processing Right aux camera rect-----------------> \n");
	rectRightAux = gc.calibratedRect(markerIrisAuxRight, markerFace, row, col);		//calculating xy cordinate
	if (rectRightAux.empty()){
		delete (vs);

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Not enough common markers found between cameras! ");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	printf("Processing Left aux camera rect-----------------> \n");
	rectLeftAux = gc.calibratedRect(markerIrisAuxleft, markerFace, row, col);	//calculating xy cordinate
	if (rectLeftAux.empty()){
		delete (vs);

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Not enough common markers found between cameras! ");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	printf("Processing Right main camera rect-----------------> \n");
	rectRightMain = gc.calibratedRect(markerIrisMainRight, markerFace, row, col);	//calculating xy cordinate
	if (rectRightMain.empty()){
		delete (vs);

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Not enough common markers found between cameras! ");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	printf("Processing Left main camera rect-----------------> \n");
	rectLeftMain = gc.calibratedRect(markerIrisMainleft, markerFace, row, col);	//calculating xy cordinate
	if (rectLeftMain.empty()){
		delete (vs);

		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Not enough common markers found between cameras! ");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	smallImg = gc.rotate90(outImg);
	Mat smallImgX;
	smallImg.copyTo(smallImgX);

	gc.pt1 = gc.processDigRectPoint1(rectLeftAux);
	gc.pt2 = gc.processDigRectPoint2(rectLeftAux);

	gc.pt3 = gc.processDigRectPoint1(rectRightAux);
	gc.pt4 = gc.processDigRectPoint2(rectRightAux);

	gc.pt5 = gc.processDigRectPoint1(rectLeftMain);
	gc.pt6 = gc.processDigRectPoint2(rectLeftMain);

	gc.pt7 = gc.processDigRectPoint1(rectRightMain);
	gc.pt8 = gc.processDigRectPoint2(rectRightMain);

	//Data saved for Aux Rect
	int x, y, width, height;
	x = 0; y = gc.pt1.y; width = int(smallImg.cols); height = int(abs(gc.pt1.y - gc.pt4.y));

	//check for negative value
	if (y < 0 || height < 0){
		DisImgx = 0;
		sprintf(cmd, "Results:");
		sprintf(buff, "Calibration failed ");
		sprintf(cmd1, "Projected Rect's coordinates are negative!");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		waitKey(2000);//3 seconds delay
		destroyWindow(cmd);

		return true;
	}

	Rect auxRect(x, y,width, height);

	gc.calibDataWrite(auxRect, rectRightAux,rectLeftAux,rectRightMain,rectLeftMain);

	if (1){
		cv::rectangle(smallImg, gc.pt1, gc.pt2, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, gc.pt3, gc.pt4, cv::Scalar(255, 255, 255), 3);
		cv::rectangle(smallImg, gc.pt5, gc.pt6, cv::Scalar(0, 255, 0), 3);
		cv::rectangle(smallImg, gc.pt7, gc.pt8, cv::Scalar(0, 255, 0), 3);
		imwrite("/home/root/data/calibration/MarkerRucoDetectofLeftRightAUX_Main.bmp", smallImg);
		sprintf(buff, "Each Iris Camera projected in Face camera");
		imshow(buff,smallImg);
		cv::moveWindow(buff,20,20);
/*		cvWaitKey();
		destroyWindow(buff);*/
/*

		sprintf(cmd, "Summary of Result:");
		cv::putText(DisImg,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImg);
		cv::moveWindow(cmd,20,20);

		waitKey(2000);//3 seconds delay
		destroyWindow(buff);
		destroyWindow(cmd);


		cv::rectangle(smallImgX, auxRect,cv::Scalar( 255, 255, 255 ), 4);
		imwrite("/home/root/data/calibration/MarkerRucoDetectofLeftRightAUX_Main_testRect.bmp", smallImgX);
		sprintf(buff, "Projected Target for Face Tracking");
		imshow(buff, smallImgX);
		cv::moveWindow(buff, 20, 20);

		DisImg = 0;
		sprintf(cmd, "Summary of Result:");
		cv::putText(DisImg,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImg);
		cv::moveWindow(cmd,20,20);

/*		cvWaitKey();
		destroyWindow(buff);*/
/*
		waitKey(2000);//3 seconds delay
		destroyWindow(buff);
		destroyWindow(cmd);

		DisImg = 0;
		sprintf(cmd, "Summary of Result:");
		sprintf(buff, "Calibration was successful! ");
		sprintf(cmd1, "Press 'q' to Quit ... ");
		cv::putText(DisImgx,buff,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		cv::putText(DisImgx,cmd1,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(cmd,DisImgx);
		cv::moveWindow(cmd,20,20);
		c=cvWaitKey();
		if (c=='q'){
			//printf("Continue!\n");
			destroyWindow(cmd);
		}
/*		cvWaitKey();
		destroyWindow(cmd);*/
/*
	}

	delete (vs);

	//Data saved for Main Rect -- but not being used
	x = 0; y = gc.pt5.y; width = int(smallImg.cols); height = int(abs(gc.pt5.y - gc.pt8.y));
	Rect mainRect(x, y,width, height);

	return false;
}

void startGeometricCalibrationApp(geomtericCalibration gc, bool calDebug){
	bool check = true;
	int numOfAttempt = 0;
	Mat DisImg(600, 900, CV_8UC3, Scalar(0,0,0));
	char c,cmd[128],cmd1[128],cmd2[128],cmd3[128],cmd4[128],cmd5[128],cmd6[128];
	while(check){

		check = runCalibration(gc, gc.m_calDebug);	// Rum calibration, return false if fail
		numOfAttempt++;

		if (numOfAttempt > 2){
			printf("Failed in geometric calibration!\n\nPossible reasons:\n1. may be one/more cameras are not focus\n");
			printf("2. The calibration target is not mounted\n 3. the gain settings of Iris camera may not equal or above unity gain\n\n "
					"Please check the above mentioned issues and re-run the calibration process\n\n");
			check = false;

			sprintf(cmd,"Summary of the calibration process:");
			sprintf(cmd1,"Failed in geometric calibration!");
			sprintf(cmd2,"Possible reasons: ");
			sprintf(cmd3,"1. Cameras may not focused properly");
			sprintf(cmd4,"2. The calibration target is not mounted");
			sprintf(cmd5,"3. Check the gain settings of each cameras");
			sprintf(cmd6,"Press 'q' to continue ...");



			cv::putText(DisImg,cmd1,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
			cv::putText(DisImg,cmd2,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
			cv::putText(DisImg,cmd3,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
			cv::putText(DisImg,cmd4,cvPoint(10,200), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
			cv::putText(DisImg,cmd5,cvPoint(10,250), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
			cv::putText(DisImg,cmd6,cvPoint(10,350), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);

			imshow(cmd,DisImg);
			cv::moveWindow(cmd,20,20);

			c=cvWaitKey();
			if (c=='q'){
				printf("Continue!\n");
				destroyWindow(cmd);
			}
		}
	}
}


void streamVideoFocus(extFocus fs, int cam){
	int w,h;
	char fName[512];

	VideoStream *vs;
	vs = new VideoStream(cam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	if (cam == 8194){
		printf("Star streaming of face camera for focusing\n");
	}
	else{
		sprintf(fName,"Star streaming of %s_%s camera for focusing\n",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN");
		printf(fName);
	}

	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		fs.camControlFocus(outImg, vs->cam_id);

		if(fs.m_quit)
			break;
	}
	delete(vs);
}

void startFocusApp(){
	extFocus fs("/home/root/data/calibration/faceConfig.ini");
	fs.DoStartCmd_focus();	//initialize the firmware and variables

	string camID;
	int dealy = 100;	//cam delay (streaming speed)

	//setup aux left and right camera
	camID = "0x87";
	setCamera(camID, dealy);

	//Run focus app
	streamVideoFocus(fs, fs.leftCam);
	streamVideoFocus(fs, fs.rightCam);

	//setup main left and right camera
	camID = "0x07";
	setCamera(camID, dealy);

	//Run focus app
	streamVideoFocus(fs, fs.leftCam);
	streamVideoFocus(fs, fs.rightCam);

	streamVideoFocus(fs, fs.faceCam);	//focus app for face camera

}


void RunCamFocus(extFocus fs){

	int w,h;
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
		fs.camControlFocus(outImg, vs->cam_id);

		if(fs.m_quit)
			break;
	}
	delete(vs);


	vs = new VideoStream(rightCam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	fs.m_quit = false;

	printf("Start Right Iris Aux Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		fs.camControlFocus(outImg, vs->cam_id);

		if(fs.m_quit)
			break;
	}
	delete(vs);

	//Set Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(100);


	//fs = new extFocus();
	vs = new VideoStream(leftCam);
	vs->flush();
	usleep(10000);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	printf("Start left Iris Main Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		fs.camControlFocus(outImg, vs->cam_id);
		//printf("quit:::::: %i\n", quit);

		if(fs.m_quit)
			break;
	}
	delete(vs);

	vs = new VideoStream(rightCam);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	printf("Start right Iris Main Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		fs.camControlFocus(outImg, vs->cam_id);

		if(fs.m_quit)
			break;
	}
	delete(vs);

	vs = new VideoStream(8194);
	vs->flush();
	usleep(100);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);
	vs->get(&w,&h,(char *)outImg.data);

	printf("Start face Camera \n");
	while(1){
		vs->get(&w,&h,(char *)outImg.data);
		fs.camControlFocus(outImg, faceCam);

		if(fs.m_quit)
			break;
	}
	delete(vs);

}
*/


// -------------------------------------------------------------------------------
//Start of new code (LR)

void msleep(int sleepTime)
{
	usleep(1000*sleepTime);
}


void saveValues(int gainML,int expML,int gainMR,int expMR,int gainAL,int expAL,int gainAR,int expAR)

//Saves gain and exposure values to .ini file.

{
	FileConfiguration faceConfig("/home/root/data/calibration/faceConfig.ini");
		stringstream ssI;
		string ssO;

		// Main Camera Left
		ssI << gainML;
		ssI >> ssO;
		faceConfig.setValue("FTracker.LeftMainIrisCamDigitalGain",ssO.c_str());

		ssI.clear();
		ssI << expML;
		ssI >> ssO;
		faceConfig.setValue("FTracker.LeftMainIrisCamExposureTime",ssO.c_str());

		// Main Camera Right
		ssI.clear();
		ssI << gainMR;
		ssI >> ssO;
		faceConfig.setValue("FTracker.RightMainIrisCamDigitalGain",ssO.c_str());


		ssI.clear();
		ssI << expMR;
		ssI >> ssO;
		faceConfig.setValue("FTracker.RightMainIrisCamExposureTime",ssO.c_str());


		// AUX Camera Left
		ssI.clear();
		ssI << gainAL;
		ssI >> ssO;
		faceConfig.setValue("FTracker.LeftAuxIrisCamDigitalGain",ssO.c_str());

		ssI.clear();
		ssI << expAL;
		ssI >> ssO;
		faceConfig.setValue("FTracker.LeftAuxIrisCamExposureTime",ssO.c_str());

		// AUX Camera Right
		ssI.clear();
		ssI << gainAR;
		ssI >> ssO;
		faceConfig.setValue("FTracker.RightAuxIrisCamDigitalGain",ssO.c_str());

		ssI.clear();
		ssI << expAR;
		ssI >> ssO;
		faceConfig.setValue("FTracker.RightAuxIrisCamExposureTime",ssO.c_str());

}



int findTarget()

//Returns the row number in the Face image that is centered on the target.

{
	printf("starting findTarget \n");

	//We will need a face image and an array of 1280 char to store pixel levels, and another array to track the target.

	Mat faceImage;
	int h, w;
	int ribbon[1280];
	int ribbonHeight = 40;			//Width of target search bar, in pixels.


	//Set LEDs to get a face image with good dynamic range at target distance (28")

	//NEED CODE HERE!!!!!!!!!!!!

	//Get a face camera image.

	vsFace->get(&w,&h,(char *)faceImage.data);
//	faceImage = imread("EXT_target_1.png",0);
		if (! faceImage.data)
		{
			printf("Image read failed \n");
		}

		namedWindow("window",CV_WINDOW_AUTOSIZE);
		imshow("window",faceImage);


	//Assume the target is centered along the y axis (i.e. horizontally for a portrait image).
	//Consider a ribbon 1" wide along the image horizontal axis.
	//At 28", there are about 1.4 pixels/mm.
	//ribbon[] is an array of 1280 points with the average values of a 1" wide strip along the center of the image

		printf("starting ribbon \n");

	for (int iw=0;iw<1280;iw++)
	{
		double level = 0;

		for (int ih=480-ribbonHeight/2; ih<480+ribbonHeight/2; ih++)
		{
			level = level + (int)faceImage.at<uchar>(ih,iw);
		}
		ribbon[iw] = level / ribbonHeight;		//Average level at center of the row.
	}




	//Look for a pattern that is bright for 180 pixels surrounded by dark on both sides for 32 pixels.

	int	imageWidth = 1280;
	int frameWidth = 25*1.3;
	int	paperWidth = 5.8*25*1.3;
	int patternWidth = 2.*frameWidth + paperWidth;
	float pattern[patternWidth];
	float patternMatch[1280 - patternWidth];
	float darkValue = -0.5 * (float)paperWidth / (float)frameWidth;


	for (int i=0;i<frameWidth;i++)
	{
		pattern[i]=darkValue;
	}
	for (int i=frameWidth+1;i<=frameWidth+paperWidth;i++)
	{
		pattern[i]=1;
	}
	for (int i=frameWidth+paperWidth+1;i<patternWidth;i++)
	{
		pattern[i]=darkValue;
	}


	for (int i = 0;i<imageWidth-patternWidth;i++)

	{
		patternMatch[i]=0;
		for(int ip = 0; ip<patternWidth;ip++)
		{
			patternMatch[i] = patternMatch[i] + ribbon[i+ip] * pattern[ip];		//patternMatch has a peak at the location of the target.
		}
	}

	//Look for a peak in patternMatch

	int bright = 0;
	int brightSpot;

	for (int i = 0; i < imageWidth-patternWidth; i++)
	{
		if (patternMatch[i] > bright)
		{
			bright = patternMatch[i];
			brightSpot = i+patternWidth/2;
		}
	}

	printf("bright spot = %i \n",brightSpot);
	//CHECK THE SIGN!!!!!
	printf("finished findTarget \n");

	//Return the center of this region.
	return brightSpot;


}


void driveMotor(int dist)

// Drives the motor through (approximately) the number of motorUnits in the face camera
// (about 10 pixels/motorUnit)
// Then wait about 300msec for motor to settle.

{
	sprintf(cmd, "fx_rel(%i)\n",dist);
	port_com_send(cmd);
	printf(cmd);
	msleep(300);
}

void pointAtTarget()

	//Drives the camera board motor to point the face camera at the gain calibration target.

{
	int distance;						//distance to move in pixels
	int motorUnits = 10;				//face camera pixels per motor unit   (CHECK ON THE VALUE!!!!!)
	int distanceLimit = 10;				//Allowed position error in face camera pixels  (About 14 mm)

	printf("starting pointAtTarget \n");

	distance = 640 - findTarget();			//Get the target position along the horizontal axis in pixels.
	printf("distance = %i \n",distance);


  	do
	{
		driveMotor(distance / motorUnits);		//The number of pixels needed to move
		distance = 640 - findTarget();				//See how it did
	}while (abs(distance) > distanceLimit);		//Run until it is close enough

	printf("Pointed at target \n");
	printf("Waiting for key \n");
	cvWaitKey(0);

}


int LeftCameraLevel()

// Returns the average level within a region of interest for an image from the left camera.

{


	Mat image;
	int h,w,lvl;
	double pixel,level;
	int imageCount = 10;	//Number of images to be averaged.
	int roiWidth = 200;		//roi is a small rectangle in the center of the image where the level is measured.
	int roiHeight = 200;


	level=0;
	for (int i=0;i<imageCount;i++)
	{
		//Get an image
		vsLeft->get(&w,&h,(char *)image.data);

		pixel=0;
		for(int iw=(w-roiWidth)/2; iw<(w+roiWidth)/2; iw++)
		{
			for(int ih=(h-roiHeight)/2; ih<(h+roiHeight)/2; ih++)
			{
				//Add up the pixel levels in the roi.
				pixel = pixel + (int)image.at<uchar>(ih,iw);
			}
		}
		//Divide by the number of pixels to get the average level, and add to previous levels.
		level = level + pixel/(roiWidth*roiHeight);
	}
	//Divide by the number of images to get the overall average level.
	lvl=level/imageCount;

	return lvl;

}

int RightCameraLevel()

// Returns the average level within a region of interest for an image from the right camera.
// See LeftCamerLevel() for comments.

{
	Mat image;
	int h,w,lvl;
	double pixel,level;
	int imageCount = 10;
	int roiWidth = 200;
	int roiHeight = 200;


	level=0;
	for (int i=0;i<imageCount;i++)
	{
		vsRight->get(&w,&h,(char *)image.data);

		pixel=0;
		for(int iw=(w-roiWidth)/2; iw<=(w+roiWidth)/2; iw++)
		{
			for(int ih=(h-roiHeight)/2; ih<=(h+roiHeight)/2; ih++)
			{
				pixel = pixel + (int)image.at<uchar>(ih,iw);
			}
		}
		level=pixel/(roiWidth*roiHeight);
	}
	lvl=level/imageCount;

	return lvl;

}


void calibrateGainAndExposure(int gainML,int expML,int gainMR,int expMR,int gainAL,int expAL,int gainAR,int expAR)

//Returns gain and exposure settings for all four iris cameras (MainLeft, Main Right, AuxLeft, AuxRight).
//It calls LeftCameraLevel and RightCameraLevel functions to get the level in a central ROI, averaged over many images.
//This level is compared to a desired level (refLevel).
//If, as is usual, this is lower than desired, then the gain is increased to compensate.
//If the level is too high, then the exposure is reduced to compensate, and the gain is left at the

{

	int level, newGain, newExposure;
	int defaultGain = 32;
	int defaultExposure = 15;
	int refLevelMain = 85;			//Desired level on image roi for Main cameras
	int refLevelAux = 190;			//Desired level on image roi for Aux cameras


	sprintf(cmd, "wcr(27,0x305e,%i)\n",defaultGain);		//Set all iris camera gains to 32 (1x)
	port_com_send(cmd);
	printf(cmd);

	sprintf(cmd, "wcr(27,0x3012,%i)\n", defaultExposure);	//Set all iris camera exposures to 15 (~0.8 msec)
	port_com_send(cmd);
	printf(cmd);


	//Start Main cameras

	port_com_send("set_cam_mode(7,30) \n");					//Connects to Main cameras

	//Main Left   (Camera 1)

	level =  LeftCameraLevel();

	if (level <= refLevelMain)
	{
		newGain = defaultGain * refLevelMain / level;		//If level is too low, increase the gain to correct it.
		newExposure = defaultExposure;
	}
	else
	{
		newGain = defaultGain;
		newExposure = defaultExposure * refLevelMain / level;	//If level is too high, reduce exposure to correct it.
	}
	gainMR = newGain;								//Returns these values.
	expML = newExposure;

	sprintf(cmd, "wcr(1,0x305e,%i)\n",newGain);				//Updates gain setting on Cameras 1
	port_com_send(cmd);

	sprintf(cmd, "wcr(1,0x3012,%i)\n",newExposure);			//Updates exposure setting on Camera 1
	port_com_send(cmd);

	printf("Left Main Gain = %i, Exposure = %i \n",newGain,newExposure);

	//Main Right   (Camera 2)

	level =  RightCameraLevel();

	if (level <= refLevelMain)
	{
		newGain = defaultGain * refLevelMain / level;
		newExposure = defaultExposure;
	}
	else
	{
		newGain = defaultGain;
		newExposure = defaultExposure * refLevelMain / level;
	}
	gainMR = newGain;
	expMR = newExposure;

	sprintf(cmd, "wcr(2,0x305e,%i)\n",newGain);
	port_com_send(cmd);

	sprintf(cmd, "wcr(2,0x3012,%i)\n",newExposure);
	port_com_send(cmd);

	printf("Right Main Gain = %i, Exposure = %i \n",newGain,newExposure);



	//Start Aux cameras

	port_com_send("set_cam_mode(135,30) \n");

	//Aux Left   (Camera 8)

	level =  LeftCameraLevel();

	if (level <= refLevelAux)
	{
		newGain = defaultGain * refLevelAux / level;
		newExposure = defaultExposure;
	}
	else
	{
		newGain = defaultGain;
		newExposure = defaultExposure * refLevelAux / level;
	}
	gainAL = newGain;
	expAL = newExposure;

	sprintf(cmd, "wcr(8,0x305e,%i)\n",newGain);
	port_com_send(cmd);

	sprintf(cmd, "wcr(8,0x3012,%i)\n",newExposure);
	port_com_send(cmd);

	printf("Left Aux Gain = %i, Exposure = %i \n",newGain,newExposure);

	//Aux Right   (Camera 16)

	level =  RightCameraLevel();

	if (level <= refLevelAux)
	{
		newGain = defaultGain * refLevelAux / level;
		newExposure = defaultExposure;
	}
	else
	{
		newGain = defaultGain;
		newExposure = defaultExposure * refLevelAux / level;
	}
	gainAR = newGain;
	expAR = newExposure;

	sprintf(cmd, "wcr(16,0x305e,%i)\n",newGain);
	port_com_send(cmd);

	sprintf(cmd, "wcr(16,0x3012,%i)\n",newExposure);
	port_com_send(cmd);

	printf("Right Aux Gain = %i, Exposure = %i \n",newGain,newExposure);

}

void tiltCameraBoard()         ///NOT USED.  SEE pointAtTarget()
{
	//This points the cameras straight out.  They must all face a 6" square sheet of paper, 28" away.
	//NOTE:  if this is not repeatable, we may need to seek a pattern on the square, like a black frame.

	int centerPosition = 100;							//Motor position for viewing straight out

	printf("Tilting the camera board \n");

	port_com_send("fx_home()\n");						//Home
	usleep(100);

	sprintf(cmd,"fx_abs(%i)\n",centerPosition);		//Point straight out
	port_com_send(cmd);
	printf(cmd);


}

void startEXT()

//Sets parameters to normal values for all iris cameras.
{

	printf("Running StartEXT \n");

	sprintf(cmd, "wcr(27,0x3012,10)\n");	//Pulse duration = 0.46msec
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "wcr(3,0x305e,128)\n");	//Aux camera gain = 4x
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "wcr(24,0x305e,80)\n");	//Main camera gain = 2.5x
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(2,37)\n");		//HV = 37V.  This is the voltage on the capacitor bank that provides current to the LEDs.
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(5,40)\n");		//LED current = 4A
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(6,8)\n");		//Pulse limit = 1 msec
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(9,85)\n");		//Power PWM = 66%.  This limits the current to the capacitor banki.
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(1,1)\n");		//HV on
	port_com_send(cmd);
	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(3,49)\n");		//LEDs alternate
	port_com_send(cmd);
	printf(cmd);

}

/*
void testWindow()
{

	printf("Running testWindow \n");

	Mat testImage;

//	testImage = imread("/home/eyelock/workspace/trunk/ExtFactoryTools/ExtCalibrationFocus/EXT_target_1.png",0);
	testImage = imread("EXT_target_1.png",0);
	if (! testImage.data)
	{
		printf("Image read failed \n");

	}

	namedWindow("window",CV_WINDOW_AUTOSIZE);
	imshow("window",testImage);

	printf("Waiting for key \n");
	cvWaitKey(0);
	printf("testWindow finished \n");

}

*/


int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");

//    int cal_cam_mode = 0;		//initializing camera to camera geometric calibration
//    int focusMode = 0;



	/*


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

	//Set environment for camera to camera calibration
	if (cal_cam_mode){
		geomtericCalibration gc("/home/root/data/calibration/faceConfig.ini");

		portcom_start();
		gc.DoStartCmd_CamCal();
		startGeometricCalibrationApp(gc, gc.m_calDebug);

		return 0;

	}

	if(focusMode){
		portcom_start();
		startFocusApp();

		outImg.release();
		return 0;

	}

	*/

	int gML, eML, gMR, eMR, gAL, eAL, gAR, eAR;			//Gain and exposure values to be saved to .ini file

	printf("Running Gain Calibration \n");

	portcom_start();

	startEXT();

#if 0
	//setup aux left and right camera
	string camID = "0x87";
	int delay = 100;

	char cmd[512];
	sprintf(cmd,"set_cam_mode(%s,%d)", camID.c_str(), delay);
	port_com_send(cmd);
	usleep(100);
#endif
	// vsLeft = new VideoStream(atoi(argv[1]));			//Start left iris camera stream
	vsLeft = new VideoStream(8192);			//Start left iris camera stream
	vsLeft->flush();
	usleep(100);

	/*
	cv::Mat ImageData = cv::Mat(Size(1200, 960), CV_8U);

	namedWindow("window",CV_WINDOW_AUTOSIZE);
	int w,h;
	vsLeft->get(&w, &h, (char*)ImageData.data);
	cv::imshow("window", ImageData);
	printf("Waiting for key \n");
	cvWaitKey(0);
	*/

	//  Anitha's code that works
	int w,h;
	cv::Mat ImageData = cv::Mat(Size(1200, 960), CV_8U);

	while(1)
	{
		vsLeft->get(&w, &h, (char*)ImageData.data);
		cv::imshow("Test", ImageData);
		cvWaitKey(1);

	}
	// delete(vsLeft);
/*

	vsRight = new VideoStream(8193);		//Start right iris camera stream
	vsRight->flush();
	usleep(100);

	vsFace = new VideoStream(8194);			//Start face camera stream, to find the target.
	vsFace->flush();
	usleep(100);


//	startEXT();								//Set EXT parameters

	//Point the cameras at the target
//	tiltCameraBoard();						//Simple open-loop drive
//	pointAtTarget();						//Recursive drive that locates target and centers it

	//Gain and exposure for proper image levels, for all iris cameras
//	calibrateGainAndExposure(gML, eML, gMR, eMR, gAL, eAL, gAR, eAR);

	//Save the values in the .ini file
//	saveValues(gML, eML, gMR, eMR, gAL, eAL, gAR, eAR);

	//Stop the image streams
	delete(vsLeft);
	delete(vsRight);
	delete(vsFace);
*/

	printf("finished Gain Calibration \n");
	return 0;

}






