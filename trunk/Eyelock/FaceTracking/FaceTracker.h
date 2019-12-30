/*
 * FaceTracker.h
 * 
 * Created on: 7 Oct, 2018
 *		Algorithm: Mo, Sarvesh, Ilya
 *      Author: Anita
 */

#ifndef FACETRACKER_H_
#define FACETRACKER_H_

#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <math.h>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "portcom.h"
#include "eyelock_com.h"
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "logging.h"
#include "CmxHandler.h"
#include "EyeLockMain.h"

//WIDTH and HEIGHT of input image
//#define WIDTH 1200
// #define HEIGHT 960

#define STATE_LOOK_FOR_FACE 1
#define STATE_MOVE_MOTOR    2
#define STATE_MAIN_IRIS     3
#define STATE_AUX_IRIS      4
#define STATE_PINGPONG_IRIS 5
#define STATE_GET_DARK_IMAGE 6
enum CAMERAMODE{AUX_IRIS = 0, MAIN_IRIS = 1, PINGPONG_IRIS = 2};
#define minCalBrightness 40		//orignal 40
#define maxCalBrightness 220

#define CENTER_POSITION_ANGLE 95

#define SCALE 3

#define MIN_IRIS_FRAMES 10
// #define FRAME_DELAY 	100

// this defines how many frames without a face will cause a switch back to face mode ie look for faces
#define NO_FACE_SWITCH_FACE 1
#define ANGLE_TO_STEPS 		5
#define smallMoveTo 		2		//limiting motor movement

//Controlling states between face and Eyes
#define RUN_STATE_FACE 	0
#define RUN_STATE_EYES 	1

//USed to switch cameras and keep it running while changes dont need
#define MODE_FACE 		0
#define MODE_EYES_MAIN 	1
#define MODE_EYES_AUX 	2

//USed in DoImageCal function
#define NUM_AVG_CAL 	10
#define MAX_CAL_CURENT 	20

int system_state = STATE_LOOK_FOR_FACE;
int last_system_state = STATE_LOOK_FOR_FACE;
int run_state=RUN_STATE_FACE;

int move_counts = 0;

const char logger[30] = "FACETRACKER";
// const char stateMachineLogger[30] = "FACETRACKER_STATES";

using namespace cv;
int IrisFrameCtr = 0;		//used for counting Iris Frame

int currnet_mode = 0;

// Function Declarations
extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern void  *init_tunnel(void *arg);
extern void *init_ec(void * arg);
void *DoTamper(void *arg);


#define PEDESTAL_OFFSET 8
class FaceTracker{
private:
	int FRAME_DELAY;
	bool m_ProjPtr;
	int rectX, rectY, rectW, rectH;
	int m1280shiftval_y;
	int m_CalibrationImageSize;
	bool m_bCalibImageSizeIs1280;
	bool mb1200ImageCalibration;
	int previousEye_distance;
	int cur_pos;

	int fileNum;

	int noeyesframe;
	int agc_val;

	float eye_size;
	float p; //AGC

	int agc_set_gain;

	//Used as increasing exposure time in brightnessAdjust function,
	//mainly used in Camera to Camera Calibration
	int agc_val_cal;
	int thresholdVal;

	int AGC_Counter;
	int noFaceCounter;
	float last_angle_move;

	bool switchedToIrisMode;

	//Reading from faceConfig.ini
	int CENTER_POS;

	int initialMotion, finalMotion, MotorAcceleration;	//used for increasing or decreasing motor acceleration

	//Switching threshold and Hysteresis
	int switchThreshold;		// 37
	int errSwitchThreshold;		//6
	
	
	// PingPong
	int m_NearSwitchThreshold;
	int m_FarSwitchThreshold;
	bool m_EnableIrisCameraPingPong;

	bool m_NewPingPongDesign;
	cv::Mat m_CroppedFaceImageForAGC; // For AGC
	bool foundFace;
	//AGC parameters
	int PIXEL_TOTAL;
	int FACE_GAIN_DEFAULT;
	int FACE_GAIN_MAX;
	int FACE_GAIN_MIN;
	int FACE_GAIN_PER_GOAL;
	float FACE_GAIN_HIST_GOAL;
	float FACE_CONTROL_GAIN;
	int FACE_GAIN_STEP;
	//#define FACE_CONTROL_GAIN   1000.0
	
	float ERROR_LOOP_GAIN;	// used for error cal during face tracking and motor movement
	float ERROR_CHECK_EYES; //Checking for Eyes at the center of the tracking target
	float ERROR_CHECK_EYES_WL;	// adjust the width of face from left
	float ERROR_CHECK_EYES_WR;	// adjust the width of face from right
	float ERROR_CHECK_EYES_HT;	//adjust the calibrated Rect height from top
	float ERROR_CHECK_EYES_HB;	//adjust the calibrated Rect height from bottom

	//Face size tracking from 90 to 30cm range if the images are bright enough
	int MIN_FACE_SIZE;
	int MAX_FACE_SIZE;

	//variables dedicated for motor movement in extreme conditions
	bool angleRange;
	bool high, low;
	int maxAngle, minAngle;

	int m_IrisLEDVolt;
	int m_IrisLEDcurrentSet;
	int m_IrisLEDtrigger;
	int m_IrisLEDEnable;
	int m_IrisLEDmaxTime;
	int m_minPos;
	int m_allLEDhighVoltEnable;
	int m_faceLEDVolt;
	int m_faceLEDcurrentSet;
	int m_faceLEDtrigger;
	int m_faceLEDEnable;
	int m_faceLEDmaxTime;
	int m_faceCamExposureTime;
	int m_faceCamDigitalGain;
	int m_faceAnalogGain;
	int m_faceCamDataPedestal;
	
	int m_DimmingfaceAnalogGain;
	int m_DimmingfaceExposureTime;
	int m_DimmingfaceDigitalGain;

	// Anita to set Left/Right camera exposure and gain separately
	int m_AuxIrisCamDataPedestal;
	int m_MainIrisCamDataPedestal;
	
	int m_LeftAuxIrisCamExposureTime;
	int m_LeftAuxIrisCamDigitalGain;

	int m_RightAuxIrisCamExposureTime;
	int m_RightAuxIrisCamDigitalGain;

	int m_LeftMainIrisCamExposureTime;
	int m_LeftMainIrisCamDigitalGain;

	int m_RightMainIrisCamExposureTime;
	int m_RightMainIrisCamDigitalGain;

	int m_irisAnalogGain;
	int m_capacitorChargeCurrent;
	
	// ToneControl from Eyelock.ini
	int m_ToneVolume;
	int m_FixedAudSetVal;
	bool m_OIMFTPEnabled;

	float motorBottom;
	float motorTop;
	float motorCenter;

	int m_Motor_Bottom_Offset;
	int m_Motor_Range;

	pthread_t threadIdtamper;
	pthread_t threadIdtemp;
	pthread_t threadIdFace;

	FaceImageQueue m_FaceCameraInfo;

	bool bFaceMapDebug;

	bool bActiveCenterPos;

	bool bIrisToFaceMapDebug;

	unsigned int m_EyelockIrisMode;

	bool b_EnableFaceAGC;

	bool bDoAESEncryption;

	int m_nCalculatedBrightness;

	Rect detect_area;

	bool m_AdaptiveGain;

	int m_AdaptiveGainMainFactor;
	// float m_AdaptiveGainAuxAdjust;
	int m_AdaptiveGainAuxFactor;
	int m_AdaptiveGainMainHysFactor;
	int m_AdaptiveGainAuxHysFactor;
	float m_AdaptiveGainKGMain;
	float m_AdaptiveGainKGAux;

	std::map<int,int> FaceWidthGainMap;
	bool m_EnableColumnNoiseReduction;
	int m_DarkImageGenFrameCnt;
	bool flag_dark_main;

	void SetExp(int cam, int val);
	void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
	void SelLedDistance(int val); // val between 0 and 100

	void MainIrisSettings(int FaceWidth, int CameraState);
//	void SwitchIrisCameras(float eye_size, bool mode);
	void SwitchIrisCameras(float eye_size, CAMERAMODE eCamMode);
	void SetFaceMode();

	void DimmFaceForIris();
	void LEDbrightnessControl(Mat smallImg);

	void SetIrisMode(float CurrentEye_distance);
	int IrisFramesHaveEyes();

	void MoveRelAngle(float a);
	void MoveToAngle(float a);
	void MoveTo(int v);
	void MoveToAbs(int v);
	void moveMotorToFaceTarget(float eye_size, bool bShowFaceTracking, bool bDebugSessions);
	void motorInit();
	void motorInitCenterPos(int v);

	void faceModeState(bool bDebugSessions);
	int SelectWhichIrisCam(float eye_size, int cur_state);
	char * StateText(int state);
	
	void DoAgc(void);
	void SweepFaceBrightness();
	float CalcExposureLevel(int width, int height,unsigned char *dsty, int limit);
	void SetFaceExposure(int val);

	// Mat rotate(Mat src, double angle);
	Mat rotation90(Mat src);
	Mat preProcessingImg(Mat outImg);

	//adjust eye Tracking target at the center
	cv::Rect seacrhEyeArea(cv::Rect no_move_area);
	cv::Rect adjustWidthDuringFaceDetection(cv::Rect face);

	void readFaceAnalogGainReg(uint32_t Value);
	void readDimFaceGainregVal(uint32_t Value);
	void CreateFaceWidthGainMap();
	void AdaptiveGain(int faceWidth, int CameraState);
	int CalculateGainWithKH(int facewidth, int CameraState);
	int CalculateGain(int facewidth);
public:

	bool bDebugSessions;
	bool bShowFaceTracking;
	bool m_ImageAuthentication;
	int m_ImageWidth;
	int m_ImageHeight;

	CmxHandler *m_CmxHandler;

	//bool faceConfigInit;

	FileConfiguration FaceConfig;

	// FaceImageQueueItem *faceInfo;

	FaceTracker(char* filename);
	virtual ~FaceTracker();

	void DoStartCmd();
		
	void DoRunMode_test(bool bShowFaceTracking, bool bDebugSessions);


};

#endif /* FACETRACKER_H_ */

