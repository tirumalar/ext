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

#include "extFocus.h"
#include "geomtericCalibration.h"

using namespace cv;
using namespace std::chrono;
using namespace std;

#define MODE_CHANGE_FREEZE 10
#define FRAME_DELAY 100
//Controlling states between face and Eyes
#define RUN_STATE_FACE 0
#define RUN_STATE_EYES 1


int currnet_mode = 0;
int previousEye_distance = 0;
const char logger[30] = "ExtSetUp";
const char stateMachineLogger[30] = "StateMachine";
VideoStream *vs;
Mat outImg, smallImg;
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

int run_state=RUN_STATE_FACE;
std::chrono:: time_point<std::chrono::system_clock> start_mode_change;


void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
void SelLedDistance(int val);
char* GetTimeStamp();
void RecoverModeDrop();
void setCamera(string cam, int delay);
void setCameraStreaming(int cam);
bool runCalibration(geomtericCalibration gc, bool calDebug);
void startGeometricCalibrationApp(geomtericCalibration gc, bool calDebug);
void streamVideoFocus(extFocus fs, int cam);
void startFocusApp();
void RunCamFocus(extFocus fs);


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

	vs = new VideoStream(cam, false);
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
	vs = new VideoStream(cam, false);
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
	vs = new VideoStream(leftCam, false);
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


	vs = new VideoStream(rightCam, false);
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
	vs = new VideoStream(leftCam, false);
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

	vs = new VideoStream(rightCam, false);
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

	vs = new VideoStream(8194, false);
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


int main(int argc, char **argv)
{
	EyelockLog(logger, TRACE, "Inside main function");

	geomtericCalibration gc("/home/root/data/calibration/faceConfig.ini");
	bool bDoAESEncryption = gc.geoConfig.getValue("FTracker.AESEncrypt", false);

	outImg = Mat(Size(WIDTH,HEIGHT), CV_8U);

	// Focus the cameras
	portcom_start(bDoAESEncryption);
	startFocusApp();

	while(1){
		char buff[512];
		// Window prompting to enter the OIM Number on the console
		string Msg = "Focusing of Cameras is complete";
		string Msg1 = "Place the target for Geometric Calibration";
		sprintf(buff, "Press 'q' to continue!");
		// string Msg2 = ;
		Mat MatImage(600, 900, CV_8UC3, Scalar(0,0,0));
		cv::putText(MatImage,Msg, cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		cv::putText(MatImage,Msg1, cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		cv::putText(MatImage,buff, cvPoint(10,200), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
		imshow(buff, MatImage);
		char key = cv::waitKey();
		if(key == 'q')
			break;
	};

	sleep(2);

	//Camera to camera geometric calibration
	gc.DoStartCmd_CamCal();
	startGeometricCalibrationApp(gc, gc.m_calDebug);

	outImg.release();
	return 0;

}




