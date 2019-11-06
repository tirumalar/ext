
#include "FaceTracker.h"
#include "file_manip.h"
#include "EyeDetectAndMatchServer.h"
#include "EyeSegmentServer_fixed.h"
#include "EdgeImage_private.h"

using namespace cv;
// using namespace std::chrono;
using namespace std;

#define DEBUG_SESSION

Point eyes;	// hold eye info from each frame

double scaling = 8.0;		//Used for resizing the images in py lev 3

// detect_area used for finding face in a certain rect area of entire image
bool faceConfigInit;
int targetOffset;
Rect detect_area(15/SCALE,15/SCALE,(960/(SCALE*SCALE))+15/SCALE,(1200/(SCALE*SCALE))+15/SCALE); //15 was 30
Rect no_move_area, no_move_areaX;		//Face target area
Rect search_eye_area;					//Narrow down the eye search range in face
Rect projFace;

VideoStream *vs;
Mat outImg;
Mat smallImg;
Mat smallImgBeforeRotate;
Mat dst;
Mat RotatedfaceImg;

VideoStream *LeftIrisCamera;
VideoStream *RightIrisCamera;

cv::Mat LeftIrisImg;
cv::Mat RightIrisImg;


EyeDetectAndMatchServer *m_pSrv;

int m_detectLevel = 2;

CSampleFrame frame;

int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size, Rect &face, int min_face_size, int max_face_size);
int face_init();
float read_angle(void);

const char Mlogger[30] = "Motor";

FaceTracker::FaceTracker(char* filename)
:FaceConfig(filename)
,previousEye_distance(0)
,cur_pos(0)
,fileNum(0)
,noeyesframe(0)
,agc_val(0)
,agc_set_gain(0)
,agc_val_cal(3)
,thresholdVal(10)
,AGC_Counter(0)
,noFaceCounter(0)
,last_angle_move(0)
,switchedToIrisMode(false)
,bDebugSessions(false)
,bShowFaceTracking(false)
,high(false)
,low(false)
,maxAngle(136)
,minAngle(72)
,m_ProjPtr(false)
,FRAME_DELAY(60)
,bFaceMapDebug(false)
,bActiveCenterPos(false)
,bIrisToFaceMapDebug(false)
,m_nCalculatedBrightness(0)
,m_eyeLabel(0)
,status(false)
,m_CameraNo(1)
{

	FRAME_DELAY = FaceConfig.getValue("FTracker.FRAMEDELAY",60);
	CENTER_POS = FaceConfig.getValue("FTracker.centerPos",164);

	cur_pos = CENTER_POS;

	//For motor acceleration
	initialMotion = FaceConfig.getValue("FTracker.IntialSpeedMotion",150);
	finalMotion = FaceConfig.getValue("FTracker.FinalSpeedMotion",150);
	MotorAcceleration = FaceConfig.getValue("FTracker.MotorAcceleration",150);

	switchThreshold = FaceConfig.getValue("FTracker.switchThreshold",37);
	errSwitchThreshold = FaceConfig.getValue("FTracker.errSwitchThreshold",2);
	
	m_IrisLEDVolt = FaceConfig.getValue("FTracker.IrisLEDVolt",30);
	m_IrisLEDcurrentSet = FaceConfig.getValue("FTracker.IrisLEDcurrentSet",40);
	m_IrisLEDtrigger = FaceConfig.getValue("FTracker.IrisLEDtrigger",3);
	m_IrisLEDEnable = FaceConfig.getValue("FTracker.IrisLEDEnable",49);
	m_IrisLEDmaxTime = FaceConfig.getValue("FTracker.IrisLEDmaxTime",4);
	m_minPos = FaceConfig.getValue("FTracker.minPos",0);
	
	m_allLEDhighVoltEnable = FaceConfig.getValue("FTracker.allLEDhighVoltEnable",1);
	m_faceLEDVolt = FaceConfig.getValue("FTracker.faceLEDVolt",30);
	m_faceLEDcurrentSet = FaceConfig.getValue("FTracker.faceLEDcurrentSet",20);
	m_faceLEDtrigger = FaceConfig.getValue("FTracker.faceLEDtrigger",4);
	m_faceLEDEnable = FaceConfig.getValue("FTracker.faceLEDEnable",4);
	m_faceLEDmaxTime = FaceConfig.getValue("FTracker.faceLEDmaxTime",4);
	
	m_faceCamExposureTime = FaceConfig.getValue("FTracker.faceCamExposureTime",12);
	m_faceCamDigitalGain = FaceConfig.getValue("FTracker.faceCamDigitalGain",240);
	m_faceAnalogGain = FaceConfig.getValue("FTracker.faceAnalogGain",128);
	m_faceCamDataPedestal = FaceConfig.getValue("FTracker.faceCamDataPedestal",0);
	
	m_DimmingfaceAnalogGain = FaceConfig.getValue("FTracker.DimmingfaceAnalogGain",0);
	m_DimmingfaceExposureTime = FaceConfig.getValue("FTracker.DimmingfaceExposureTime",7);
	m_DimmingfaceDigitalGain = FaceConfig.getValue("FTracker.DimmingfaceDigitalGain",32);
	
	m_AuxIrisCamDataPedestal = FaceConfig.getValue("FTracker.AuxIrisCamDataPedestal",0);
	m_MainIrisCamDataPedestal = FaceConfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	// Left Aux Camera
	m_LeftAuxIrisCamExposureTime = FaceConfig.getValue("FTracker.LeftAuxIrisCamExposureTime",8);
	m_LeftAuxIrisCamDigitalGain = FaceConfig.getValue("FTracker.LeftAuxIrisCamDigitalGain",80);

	// Right Aux Camera
	m_RightAuxIrisCamExposureTime = FaceConfig.getValue("FTracker.RightAuxIrisCamExposureTime",8);
	m_RightAuxIrisCamDigitalGain = FaceConfig.getValue("FTracker.RightAuxIrisCamDigitalGain",80);

	// Left Main Camera
	m_LeftMainIrisCamExposureTime = FaceConfig.getValue("FTracker.LeftMainIrisCamExposureTime",8);
	m_LeftMainIrisCamDigitalGain = FaceConfig.getValue("FTracker.LeftMainIrisCamDigitalGain",128);

	// Right Main Camera
	m_RightMainIrisCamExposureTime = FaceConfig.getValue("FTracker.RightMainIrisCamExposureTime",8);
	m_RightMainIrisCamDigitalGain = FaceConfig.getValue("FTracker.RightMainIrisCamDigitalGain",128);

	b_EnableFaceAGC = FaceConfig.getValue("FTracker.EnableFaceAGC", true);

	m_irisAnalogGain = FaceConfig.getValue("FTracker.irisAnalogGain",144);
	m_faceAnalogGain = FaceConfig.getValue("FTracker.faceAnalogGain",128);
	m_capacitorChargeCurrent = FaceConfig.getValue("FTracker.capacitorChargeCurrent",60);

	//Reading AGC parameters
	PIXEL_TOTAL = FaceConfig.getValue("FTracker.PIXEL_TOTAL",900);

	FACE_GAIN_DEFAULT = FaceConfig.getValue("FTracker.FACE_GAIN_DEFAULT",10);
	FACE_GAIN_DEFAULT = FACE_GAIN_DEFAULT * PIXEL_TOTAL;

	agc_val = FACE_GAIN_DEFAULT;

	FACE_GAIN_STEP = FaceConfig.getValue("FTracker.FACE_GAIN_STEP", 20);
	FACE_GAIN_STEP = FACE_GAIN_STEP * PIXEL_TOTAL;
	FACE_GAIN_MAX = FaceConfig.getValue("FTracker.FACE_GAIN_MAX",80);
	FACE_GAIN_MAX = FACE_GAIN_MAX * PIXEL_TOTAL;
	FACE_GAIN_MIN = FaceConfig.getValue("FTracker.FACE_GAIN_MIN",8);
	FACE_GAIN_MIN = FACE_GAIN_MIN * PIXEL_TOTAL;
	FACE_GAIN_PER_GOAL = FaceConfig.getValue("FTracker.FACE_GAIN_PER_GOAL",10);
	FACE_GAIN_HIST_GOAL = FaceConfig.getValue("FTracker.FACE_GAIN_HIST_GOAL",float(0.1));
	FACE_CONTROL_GAIN = FaceConfig.getValue("FTracker.FACE_CONTROL_GAIN",float(500.0));
	ERROR_LOOP_GAIN = FaceConfig.getValue("FTracker.ERROR_LOOP_GAIN",float(0.08));
	ERROR_CHECK_EYES = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES",float(0.06));
	ERROR_CHECK_EYES_HT = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_HT",float(0.0));
	ERROR_CHECK_EYES_HB = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_HB",float(0.06));
	ERROR_CHECK_EYES_WL = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_WL",float(0.0));
	ERROR_CHECK_EYES_WR = FaceConfig.getValue("FTracker.ERROR_CHECK_EYES_WR",float(0.0));

	MIN_FACE_SIZE = FaceConfig.getValue("FTracker.MIN_FACE_SIZE",10);
	MAX_FACE_SIZE = FaceConfig.getValue("FTracker.MAX_FACE_SIZE",70);

	bShowFaceTracking = FaceConfig.getValue("FTracker.ShowFaceTracking", false);

	bActiveCenterPos = FaceConfig.getValue("FTracker.ActivateCenterPosTest", false);

	// Flag to enable/disable AES Encrption in port com
	bDoAESEncryption = FaceConfig.getValue("FTracker.AESEncrypt", false);

	m_Motor_Bottom_Offset = FaceConfig.getValue("FTracker.MotorBottomOffset",5);
	m_Motor_Range = FaceConfig.getValue("FTracker.MotorRange",55);

	// Eyelock.ini Parameters	
	FileConfiguration EyelockConfig("/home/root/Eyelock.ini");

	// Get the width and height of Image from Eyelock.ini
	m_ImageWidth = EyelockConfig.getValue("FrameSize.width", 1200);
	m_ImageHeight = EyelockConfig.getValue("FrameSize.height", 960);


	m_ToneVolume = EyelockConfig.getValue("GRI.AuthorizationToneVolume", 40);
	m_FixedAudSetVal = EyelockConfig.getValue("Eyelock.FixedAudSetValue", 5);
	m_ImageAuthentication = EyelockConfig.getValue("Eyelock.ImageAuthentication", true);
	m_CameraTestTime = EyelockConfig.getValue("Eyelock.ExtTestEachCameraTime", 120);
	// printf("m_CameraTestTime.......%d\n", m_CameraTestTime);

	bIrisToFaceMapDebug = EyelockConfig.getValue("Eyelock.IrisToFaceMapDebug", false);

	m_IrisToFaceMapCorrectionVal = EyelockConfig.getValue("Eyelock.IrisToFaceMapCorrectionFactor", 20);

	FileConfiguration CalibDefaultConfig("/home/root/data/calibration/CalRect.ini");
	// To support old devices which don't have cal rect file stored on OIM
	rectX = CalibDefaultConfig.getValue("FTracker.targetRectX",0);
	rectY = CalibDefaultConfig.getValue("FTracker.targetRectY",497);
	rectW = CalibDefaultConfig.getValue("FTracker.targetRectWidth",960);
	rectH = CalibDefaultConfig.getValue("FTracker.targetRectHeight",121);

	//ERROR_CHECK_EYES = m_FaceConfig.getValue("FTracker.ERROR_CHECK_EYES",float(0.06));
	magOffMainl = CalibDefaultConfig.getValue("FTracker.magOffsetMainLeftCam",float(0.15));
	magOffMainR = CalibDefaultConfig.getValue("FTracker.magOffsetMainRightCam",float(0.15));
	magOffAuxl = CalibDefaultConfig.getValue("FTracker.magOffsetAuxLeftCam",float(0.22));
	magOffAuxR = CalibDefaultConfig.getValue("FTracker.magOffsetAuxRightCam",float(0.22));

	magOffMainlDiv = 1.0/magOffMainl;
	magOffMainRDiv = 1.0/magOffMainR;
	magOffAuxlDiv = 1.0/magOffAuxl;
	magOffAuxRDiv = 1.0/magOffAuxR;

	constantMainl.x = CalibDefaultConfig.getValue("FTracker.constantMainLeftCam_x",float(0.15));
	constantMainl.y = CalibDefaultConfig.getValue("FTracker.constantMainLeftCam_y",float(0.15));
	constantMainR.x = CalibDefaultConfig.getValue("FTracker.constantMainRightCam_x",float(0.15));
	constantMainR.y = CalibDefaultConfig.getValue("FTracker.constantMainRightCam_y",float(0.15));
	constantAuxl.x = CalibDefaultConfig.getValue("FTracker.constantAuxLeftCam_x",float(0.22));
	constantAuxl.y = CalibDefaultConfig.getValue("FTracker.constantAuxLeftCam_y",float(0.22));
	constantAuxR.x = CalibDefaultConfig.getValue("FTracker.constantAuxRightCam_x",float(0.22));
	constantAuxR.y = CalibDefaultConfig.getValue("FTracker.constantAuxRightCam_y",float(0.22));

	useOffest_m = CalibDefaultConfig.getValue("FTracker.projectionOffsetMain",false);
	useOffest_a = CalibDefaultConfig.getValue("FTracker.projectionOffsetAux",true);
	projOffset_m = CalibDefaultConfig.getValue("FTracker.projectionOffsetValMain",float(50.00));
	projOffset_a = CalibDefaultConfig.getValue("FTracker.projectionOffsetValAux",float(200.00));

	m_LeftCameraIrisImage = cvCreateImage(cvSize(m_ImageWidth, m_ImageHeight),IPL_DEPTH_8U,1);
	m_RightCameraIrisImage = cvCreateImage(cvSize(m_ImageWidth, m_ImageHeight),IPL_DEPTH_8U,1);

	m_EyeCrop = cvCreateImage(cvSize(640, 480),IPL_DEPTH_8U,1);
	m_pSrv = new EyeDetectAndMatchServer(m_ImageWidth, m_ImageHeight, m_detectLevel, "Eyelock.log");

	m_pSrv->SetSingleSpecMode(EyelockConfig.getValue("GRI.SingleSpecMode",false));
	m_pSrv->SetDoHaar(EyelockConfig.getValue("GRI.DoHaar",true));
	m_pSrv->SetHaarEyeZoom(EyelockConfig.getValue("GRI.HaarEyeZoom",m_pSrv->GetHaarEyeZoom()));
	m_pSrv->SetHaarImageShifts(EyelockConfig.getValue("GRI.HaarImageShifts",m_pSrv->GetHaarImageShifts()));
	m_pSrv->SetHaarImageSampling(EyelockConfig.getValue("GRI.HaarImageSampling",m_pSrv->GetHaarImageSampling()));

	bool val = EyelockConfig.getValue("GRI.CovarianceTestForDetection",false);

	m_pSrv->SetCovTestForDetection(val?1:0);
	m_pSrv->SetSpecCovEigenThresh(EyelockConfig.getValue("GRI.SpecularityCovarianceEigenvalueThreshold",m_pSrv->GetSpecCovEigenThresh()));
	m_pSrv->SetSpecEccThresh(EyelockConfig.getValue("GRI.SpecularityEccentricityThreshold",m_pSrv->GetSpecEccThresh()));

	EyeDetectorServer *detector=m_pSrv->GetEyeDetector();
	detector->SetSpecularityMagnitude(EyelockConfig.getValue("GRI.EyeDetectionSpecularityMagnitude",15));
	int a = detector->GetSpecularitySize();

	detector->SetMaskRadius(EyelockConfig.getValue("GRI.EyeDetectionMaskRadius",10));
	detector->SetVarianceThresholdMin(EyelockConfig.getValue("GRI.EyeDetectionVarianceThresholdMin",1.5f));
	detector->SetVarianceThresholdMax(EyelockConfig.getValue("GRI.EyeDetectionVarianceThresholdMax",0.666f));
	detector->SetSeparation(EyelockConfig.getValue("GRI.EyeDetectionSeparation",36));
	detector->SetSearchX(EyelockConfig.getValue("GRI.EyeDetectionSearchX",15));
	detector->SetSearchY(EyelockConfig.getValue("GRI.EyeDetectionSearchY",10));
	detector->SetBoxX(EyelockConfig.getValue("GRI.EyeDetectionBoxX",detector->GetSpecularitySize()));
	detector->SetBoxY(EyelockConfig.getValue("GRI.EyeDetectionBoxY",detector->GetSpecularitySize()));



	detector->m_shouldLog=0;

	m_pSrv->LoadHaarClassifier("/home/root/data/adaboostClassifier.txt");
	frame.setScratch(m_pSrv->GetScratch());


	leftCam = 8192;
	rightCam = 8193;
	faceCam = 8194;

	// DisImg = cv::Mat::zeros(400, 650, CV_8UC3);

	resizeFaceWidth = DISPLAY_WIN_HEIGHT * m_ImageHeight / m_ImageWidth;

	resizeIrisWidth = DISPLAY_WIN_WIDTH - resizeFaceWidth;

	resizeIrisHeight = resizeIrisWidth * m_ImageHeight / m_ImageWidth;

	IrisPointYLoc = ((DISPLAY_WIN_HEIGHT - resizeIrisHeight)/2);
}

FaceTracker::~FaceTracker()
{
	if(m_LeftCameraFaceInfo.faceImagePtr)
		delete [] m_LeftCameraFaceInfo.faceImagePtr;
	if(m_RightCameraFaceInfo.faceImagePtr)
		delete [] m_RightCameraFaceInfo.faceImagePtr;
}

void setCamera(string cam, int delay){
	char cmd[512];

	sprintf(cmd,"set_cam_mode(%s,%d)",cam.c_str(), delay);
	port_com_send(cmd);
	usleep(1);
}

void FaceTracker::SetExp(int cam, int val)
{
	EyelockLog(logger, TRACE, "SetExp");
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	// int fine = val - coarse*PIXEL_TOTAL;
	//sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	EyelockLog(logger, TRACE, "Setting Gain %d\n",coarse);
	//port_com_send(buff);
}

void FaceTracker::SetFaceExposure(int val)
{
	EyelockLog(logger, TRACE, "SetFaceExposure");
	char buff[100];
	int coarse = val/PIXEL_TOTAL;
	// int fine = val - coarse*PIXEL_TOTAL;
	//sprintf(buff,"wcr(%d,0x3012,%d) | wcr(%d,0x3014,%d)",cam,coarse,cam,fine);
	// sprintf(buff,"wcr(%d,0x3012,%d)",cam,coarse);
	sprintf(buff,"wcr(4,0x305e,%d)", coarse);
	// printf("%s\n", buff);
	EyelockLog(logger, TRACE, "Setting Exposure %d\n",coarse);
	port_com_send(buff);
}


void FaceTracker::MoveToAngle(float a)
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

void FaceTracker::MoveTo(int v)
{
	EyelockLog(logger, TRACE, "MoveTo");
	EyelockLog(logger, DEBUG,"Move to command %d ",v);

	v=v-CENTER_POS;
	//v=v/ANGLE_TO_STEPS+CENTER_POSITION_ANGLE;
	v=v/ANGLE_TO_STEPS+motorCenter;

	EyelockLog(logger, DEBUG,"Value to MoveToAngle:angle = %d",v);

	MoveToAngle((float) v);
}
void FaceTracker::MoveToAbs(int v)
{
	char cmd[128];
	EyelockLog(logger, TRACE, "MoveToAbs");
	EyelockLog(logger, DEBUG,"Move to Absolute command %d ",v);

	port_com_send("fx_home");

	sprintf(cmd, "fx_abs(%i)",v);
	//printf(cmd);
	port_com_send(cmd);
	//EyelockLog(logger, DEBUG, "Moving to center position");
	EyelockLog(logger, DEBUG, "Move to ABS CENTER Position after Match/nonMatch:%d", v);
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

void FaceTracker::MainIrisSettings()
{
	EyelockLog(logger, TRACE, "MainIrisSettings");

	char cmd[512];

	//return;
	EyelockLog(logger, DEBUG, "configuring Main Iris settings");
	// printf("configuring Main Iris settings\n");
	//Iris configuration of LED
	sprintf(cmd,"psoc_write(3,%i)| psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(6,%i)", m_IrisLEDEnable, m_IrisLEDVolt, m_IrisLEDcurrentSet, m_IrisLEDtrigger, m_IrisLEDmaxTime);
	//printf(cmd);
	port_com_send(cmd);
	//port_com_send("psoc_write(3,0x31)| psoc_write(2,30) | psoc_write(5,40) | psoc_write(4,3) | psoc_write(6,4)");
	//Face cameras configuration
	//port_com_send("wcr(0x83,0x3012,12) | wcr(0x83,0x301e,0) | wcr(0x83,0x305e,128)");

	EyelockLog(logger, DEBUG, "Values in MainIrisSettings IrisLEDEnable:%d, IrisLEDVolt:%d, IrisLEDcurrentSet:%d, IrisLEDtrigger:%d, IrisLEDmaxTime:%d ", m_IrisLEDEnable, m_IrisLEDVolt, m_IrisLEDcurrentSet, m_IrisLEDtrigger, m_IrisLEDmaxTime);
}

void FaceTracker::SwitchIrisCameras(bool mode)
{
	EyelockLog(logger, TRACE, "SwitchIrisCameras");

	char cmd[100];
	if (mode)
		sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY); // --- Main
	else
		sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY); // --- AUX

	port_com_send(cmd);
}

void FaceTracker::readFaceAnalogGainReg(uint32_t Value)
{
	int count = 0;
	char cmd[512];
	int len;
	char buffer[512];
	while(count < 2)
	{
		sprintf(cmd,"rcr(0x04,0x30b0)");

		if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
		{
			PortComLog(logger, ERROR, "Failed to read register 0x30b0!!");
		}
		if (strstr(buffer,"0x90"))
		{
			// printf("readFaceAnalogGainReg %d %s\n", Value, buffer);
			PortComLog(logger, DEBUG, "Face camera Analog gain write successful attempts..%d", count);
			break;
		}
		else
		{
			sprintf(cmd,"wcr(0x04,0x30b0,%i)",Value);
			port_com_send(cmd);
			PortComLog(logger, ERROR, "Face camera Analog gain write again attempts..%d", count);
		}
		count++;
	}
}

void FaceTracker::readDimFaceGainregVal(uint32_t Value)
{
	int count = 0;
	char cmd[512];
	int len;
	char buffer[512];

	while(count < 2)
	{
		sprintf(cmd,"rcr(0x04,0x30b0)");
		if (!(len = port_com_send_return(cmd, buffer, 20) > 0))
		{
			PortComLog(logger, ERROR, "Failed to read register 0x30b0!!");
		}

		if (strstr(buffer,"0x80"))
		{
			// printf("readDimFaceGainregVal %d %s\n", Value, buffer);
			PortComLog(logger, DEBUG, "Face camera Analog dimming gain write successful attempts..%d", count);
			break;
		}
		else
		{
			sprintf(cmd,"wcr(0x04,0x30b0,%i)",Value);
			port_com_send(cmd);
			PortComLog(logger, ERROR, "Face camera Analog dimming gain write again attempts..%d", count);
		}
		count++;
	}
}


void FaceTracker::SetFaceMode()
{
	// printf("Inside setFaceMode\n");
	EyelockLog(logger, TRACE, "SetFaceMode");
	if (currnet_mode==MODE_FACE)
	{
		EyelockLog(logger, DEBUG, "Don't need to change Face camera");
		return;
	}

	char cmd[512];
	EyelockLog(logger, DEBUG, "Setting Face Mode\n");
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)", m_faceLEDVolt, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable);
	EyelockLog(logger, DEBUG, "Face camera settings-faceLEDVolt:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d", m_faceLEDVolt, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable);
	//printf(cmd);
	port_com_send(cmd);
//	port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
//	port_com_send("psoc_write(2,30) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)");
	sprintf(cmd, "wcr(4,0x3012,%i)  | wcr(4,0x305e,%i)", m_faceCamExposureTime, m_faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "Face camera exposure and gain settings -faceCamExposureTime:%d, faceCamDigitalGain:%d",m_faceCamExposureTime, m_faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(4,0x3012,7)  | wcr(4,0x305e,0xfe)");
	sprintf(cmd,"wcr(0x04,0x30b0,%i)",((m_faceAnalogGain&0x3)<<4) | 0X80);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Face camera Analog gain:%d",(((m_faceAnalogGain&0x3)<<4) | 0X80));

	// Read the register 0x30b0
	unsigned int Value = ((m_faceAnalogGain&0x3)<<4) | 0X80;
	readFaceAnalogGainReg(Value);

	agc_val= FACE_GAIN_DEFAULT;
	EyelockLog(logger, DEBUG, "Face camera gain default:%d", FACE_GAIN_DEFAULT);
	// start_mode_change = std::chrono::system_clock::now();
	currnet_mode = MODE_FACE;
	//port_com_send("fixed_set_rgb(100,100,100)");

	sprintf(cmd,"set_cam_mode(0x04,%d)",FRAME_DELAY);
	port_com_send(cmd);
}

void FaceTracker::MoveRelAngle(float a)
{
	FILE *file;
	float pr_time;
	EyelockLog(logger, TRACE, "MoveRelAngle");
	// add a limit check to make sure we are not out of bounds
	char buff[100];
	float move;
	float current_a = read_angle();
	EyelockLog(logger, DEBUG, "Current_a=%f ; next_a=%f\n",current_a,a);

	MotorLog(Mlogger, DEBUG, "Current angle = ,%03.3f, next Angle = ,%03.3f, ",current_a,a);
	 // make sure we are not asked to go bellow the bottom
	if ((a>0)&&((current_a -a)<motorBottom))
	{
		MotorLog(Mlogger, DEBUG, "Before correction %3.3f  bot %3.3f ",a,motorBottom);
		a = -1*(motorBottom-current_a);
		// dont move in opposite direction we are there so too bad
		if (a<0)
			a=0;
		MotorLog(Mlogger, DEBUG, "Bottom Hit---------- ---  %3.3f",a);
	}
	if((a < 0)&&(current_a-a)>motorTop)
	{
		MotorLog(Mlogger, DEBUG, "Before correction for Motor top %3.3f  top %3.3f ",a,motorTop);
		a = -1*(motorTop-current_a);

		MotorLog(Mlogger, DEBUG, "motorTop Hit---------- ---  %3.3f",a);

	}


	move=-1*a*ANGLE_TO_STEPS;



	EyelockLog(logger, DEBUG, "limiting small movements based on relative changes and face size changes:diffEyedistance %f", move);
	//limiting small movements based on relative changes and face size changes
	//printf("Current_a=%f ; next_a=%f\n",current_a,a);
	
	//low angle condition but the motor wants to move up
	low = ((int)(abs(current_a)) < minAngle && (a < 0)) ? true : false;

	//high angle conditions but the motor wats top move down
	high = ((int)(abs(current_a)) > maxAngle && (a > 0)) ? true : false;

	//move angle range
	angleRange = ((int)(abs(current_a)) <= maxAngle && (int)(abs(current_a)) >= minAngle) ? true : false;

	//printf("low and high::::::::::::::::::::: %i 	%i\n", low, high);
	

#if 0
	if (abs(move) > smallMoveTo && angleRange)
	{
		sprintf(buff,"fx_rel(%d)",(int)move);
		EyelockLog(logger, DEBUG, "In move angle range --- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
		port_com_send(buff);
		//printf("fx_rel(%d)\n",(int)(abs(move)));
	}
	else{

		if (low){
			sprintf(buff,"fx_rel(%d)",(int)move);
			EyelockLog(logger, DEBUG, "low angle condition but the motor wants to move up --- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
			port_com_send(buff);
			//printf("fx_rel(%d)\n",(int)(abs(move)));
		}

		if(high){
			sprintf(buff,"fx_rel(%d)",(int)move);
			EyelockLog(logger, DEBUG, "high angle conditions but the motor wats top move down -- Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
			port_com_send(buff);
			//printf("fx_rel(%d)\n",(int)(abs(move)));
		}

	}

#else
	if (abs(move) > smallMoveTo)	//&& diffEyedistance >= errSwitchThreshold
	{
		sprintf(buff,"fx_rel(%d)",(int)move);
		EyelockLog(logger, DEBUG, "Sending by angle(current %3.3f dest %3.3f: %s\n",current_a,a,buff);
		port_com_send(buff,&pr_time);
	}
#endif

	usleep(50000);
	current_a = read_angle();

	MotorLog(Mlogger, DEBUG, "After move angle = , %03.3f, Processing Time = ,%03.3f",current_a,pr_time);

}


void FaceTracker::DimmFaceForIris()
{
	EyelockLog(logger, TRACE, "DimmFaceForIris");
	char cmd[512];
	
	// printf("Dimming face cameras!!!");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x305e,%i)", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	EyelockLog(logger, DEBUG, "Dimming face cameras -DimmingfaceExposureTime:%d, DimmingfaceDigitalGain:%d", m_DimmingfaceExposureTime, m_DimmingfaceDigitalGain);
	port_com_send(cmd);

	//Need to change this analog gain setting
	{
		char cmd[512];
		sprintf(cmd,"wcr(0x04,0x30b0,%i)",((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80);
		EyelockLog(logger, DEBUG, "FACE_GAIN_MIN:%d DimmingfaceAnalogGain:%d", FACE_GAIN_MIN, (((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80));
		port_com_send(cmd);
	}
	unsigned int Value = ((m_DimmingfaceAnalogGain&0x3)<<4) | 0X80;
	readDimFaceGainregVal(Value);
	agc_val = FACE_GAIN_MIN;

}

cv::Rect FaceTracker::seacrhEyeArea(cv::Rect no_move_area){
	EyelockLog(logger, TRACE, "Entering > seacrhEyeArea()");
	if (float(ERROR_CHECK_EYES_HT +  ERROR_CHECK_EYES_HB) >= 0.9){
		ERROR_CHECK_EYES_HT = 0.0;
		ERROR_CHECK_EYES_HT = 0.4;
	}

	float hclipT = float(no_move_area.height * ERROR_CHECK_EYES_HT);
	float hclipB = float(no_move_area.height * ERROR_CHECK_EYES_HB);

	Rect modRect;
	modRect.x = no_move_area.x;
	modRect.width = no_move_area.width;


	//adjust the search REect to Top
	modRect.y = no_move_area.y + int(hclipT);
	modRect.height = no_move_area.height - int(hclipT + hclipB);

	// printf("search_eye_area 	x: %d	y: %d	w: %d	h: %d\n", modRect.x, modRect.y, modRect.height, modRect.width);

	if(modRect.x < 0)
		modRect.x = 0;
	if(modRect.y < 0)
			modRect.y = 0;
	if(modRect.width > m_ImageWidth)
			modRect.width = m_ImageWidth;
	if(modRect.height > m_ImageHeight)
			modRect.height = m_ImageHeight;

	return modRect;
}


cv::Rect FaceTracker::adjustWidthDuringFaceDetection(cv::Rect face){
	EyelockLog(logger, TRACE, "Entering > adjustWidthDuringFaceDetection()");
	cv::Rect modFace;
     
	if(bFaceMapDebug) 
		EyelockLog(logger, DEBUG, "Before adjustWidthDuringFaceDetection Function face.x	%d face.y	%d face.height	%d face.width	%d\n", face.x, face.y, face.height, face.width);

	if (float(ERROR_CHECK_EYES_WL +  ERROR_CHECK_EYES_WR) >= 0.8){
		ERROR_CHECK_EYES_WL = 0.1;
		ERROR_CHECK_EYES_WR = 0.1;
	}

	float wclipL = float(face.width * ERROR_CHECK_EYES_WL);
	float wclipR = float(face.width * ERROR_CHECK_EYES_WR);

	modFace.x = face.x + int(wclipL);
	modFace.width = face.width - int(wclipL + wclipR);

	modFace.y = face.y;
	modFace.height = face.height;

	if(bFaceMapDebug) 
		EyelockLog(logger, DEBUG, "After adjustWidthDuringFaceDetection Function face.x	%d face.y	%d face.height	%d face.width	%d\n", face.x, face.y, face.height, face.width);

	return modFace;
}

#define NUMAVG 10

void FaceTracker::motorInit(){
	EyelockLog(logger, TRACE, "Entering > motorInit()");
	char cmd[512];
	sprintf(cmd,"fx_home");
	port_com_send(cmd);

	float sum = 0;
	for(int i = 0; i < NUMAVG; i++){
		sum+= read_angle();

	}
  motorBottom=sum/NUMAVG + m_Motor_Bottom_Offset;
  motorTop= motorBottom + m_Motor_Range;
  MotorLog(Mlogger, DEBUG, "Motor Bottom   %3.3f ------------",motorBottom);

}

void FaceTracker::motorInitCenterPos( int v){
	EyelockLog(logger, TRACE, "Entering > motorInitCenterPos()");
	char cmd[512];
	sprintf(cmd,"fx_abs(%d)", v);
	port_com_send(cmd);

	float sum = 0;
	for(int i = 0; i < NUMAVG; i++){
		sum+= read_angle();

	}
	motorCenter=sum/NUMAVG;
	MotorLog(Mlogger, DEBUG, "Motor motorCenter   %3.3f ------------",motorCenter);
}

// Main DoStartCmd configuration for Eyelock matching
void FaceTracker::DoStartCmd()
{
	EyelockLog(logger, TRACE, "DoStartCmd");
	faceConfigInit = false;

	char cmd[512];

	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);


	//adjust Motor Motion
	sprintf(cmd,"fx_mot_set(%i,%i,%i)",initialMotion, finalMotion, MotorAcceleration);
	//printf(cmd);
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "Motor acceleration setting is issued");
	EyelockLog(logger, DEBUG, cmd);

	//Homing
	EyelockLog(logger, DEBUG, "Re Homing");
	printf("Re Homing\n");
	sprintf(cmd,"fx_home");
	port_com_send(cmd);
	EyelockLog(logger, DEBUG, "port_com_send fx_home command is issued");

	printf("Motor Int\n");
	motorInit();

	/*//Reset the lower motion
	sprintf(cmd, "fx_abs(%i)",MIN_POS);
	EyelockLog(logger, DEBUG, "Reset to lower position minPos:%d", MIN_POS);
	port_com_send(cmd);*/

	motorInitCenterPos(CENTER_POS);

	//move to center position
	printf("Moving to Center\n");
	//MoveToAbs(CENTER_POS_TEST);
	MoveTo(CENTER_POS);
	EyelockLog(logger, DEBUG, "Move to center Position:%d", CENTER_POS);
/*
	sprintf(cmd, "fx_abs(%i)",CENTER_POS);
	EyelockLog(logger, DEBUG, "Moving to center position");
	port_com_send(cmd);
	//MoveTo(CENTER_POS);
*/
	read_angle();		//read current angle

	EyelockLog(logger, DEBUG, "Configuring face LEDs");
	//Face configuration of LED
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n", m_faceLEDVolt, m_allLEDhighVoltEnable, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable, m_faceLEDmaxTime);

	EyelockLog(logger, DEBUG, "Configuring face LEDs faceLEDVolt:%d, allLEDhighVoltEnable:%d, faceLEDcurrentSet:%d, faceLEDtrigger:%d, faceLEDEnable:%d, faceLEDmaxTime:%d", m_faceLEDVolt, m_allLEDhighVoltEnable, m_faceLEDcurrentSet, m_faceLEDtrigger, m_faceLEDEnable, m_faceLEDmaxTime);
	port_com_send(cmd);
	//port_com_send("psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");

	//port_com_send("psoc_write(9,90)");	// charge cap for max current 60 < range < 95
	sprintf(cmd, "psoc_write(9,%i)\n", m_capacitorChargeCurrent);
	EyelockLog(logger, DEBUG, "capacitorChargeCurrent:%d", m_capacitorChargeCurrent);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1f,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images
	EyelockLog(logger, DEBUG, "FLIPPING OF IRIS CAMERAS");

	//Face cameras configuration
	EyelockLog(logger, DEBUG, "Configuring face Cameras");
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",m_faceCamExposureTime, m_faceCamDataPedestal, m_faceCamDigitalGain);
	EyelockLog(logger, DEBUG, "faceCamExposureTime:%d faceCamDataPedestal:%d faceCamDigitalGain:%d", m_faceCamExposureTime, m_faceCamDataPedestal, m_faceCamDigitalGain);
	port_com_send(cmd);
	//port_com_send("wcr(0x04,0x3012,7) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xFE)");
	//port_com_send("wcr(0x04,0x3012,12) | wcr(0x04,0x301e,0) | wcr(0x04,0x305e,0xF0)");	//Demo Config

	// Aux Left Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring AUX Left Iris Cameras");
	sprintf(cmd, "wcr(0x01,0x3012,%i) | wcr(0x01,0x301e,%i) | wcr(0x01,0x305e,%i)\n",m_LeftAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_LeftAuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", m_LeftAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_LeftAuxIrisCamDigitalGain);
	port_com_send(cmd);

	// Aux Right Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring AUX Right Iris Cameras");
	sprintf(cmd, "wcr(0x02,0x3012,%i) | wcr(0x02,0x301e,%i) | wcr(0x02,0x305e,%i)\n",m_RightAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_RightAuxIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "AuxIrisCamExposureTime:%d AuxIrisCamDataPedestal:%d AuxIrisCamDigitalGain:%d", m_RightAuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_RightAuxIrisCamDigitalGain);
	port_com_send(cmd);

	//Main Left Camera Exposure and Gain
	EyelockLog(logger, DEBUG, "Configuring Main Left Iris Cameras");
	sprintf(cmd, "wcr(0x08,0x3012,%i) | wcr(0x08,0x301e,%i) | wcr(0x08,0x305e,%i)\n",m_LeftMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_LeftMainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", m_LeftMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_LeftMainIrisCamDigitalGain);
	port_com_send(cmd);

	//Main Right Iris Cameras Configuration
	EyelockLog(logger, DEBUG, "Configuring Main Right Iris Cameras");
	sprintf(cmd, "wcr(0x10,0x3012,%i) | wcr(0x10,0x301e,%i) | wcr(0x10,0x305e,%i)\n",m_RightMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_RightMainIrisCamDigitalGain);
	EyelockLog(logger, DEBUG, "MainIrisCamExposureTime:%d MainIrisCamDataPedestal:%d MainIrisCamDigitalGain:%d", m_RightMainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_RightMainIrisCamDigitalGain);
	port_com_send(cmd);

	EyelockLog(logger, DEBUG, "Setting up PLL");

	//following process will activate PLL for all cameras
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((m_irisAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Iris analog gain: %d", (((m_irisAnalogGain&0x3)<<4) | 0X80));

	port_com_send(cmd);
	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((m_faceAnalogGain&0x3)<<4) | 0X80);
	EyelockLog(logger, DEBUG, "Face analog gain: %d", (((m_faceAnalogGain&0x3)<<4) | 0X80));
	port_com_send(cmd);

	readFaceAnalogGainReg((((m_faceAnalogGain&0x3)<<4) | 0X80));
	//port_com_send("wcr(0x1f,0x30b0,0x80");		//all 4 Iris cameras gain is x80
	//port_com_send("wcr(0x4,0x30b0,0x90");		//Only face camera gain is x90

	EyelockLog(logger, DEBUG, "Turning on Alternate cameras");
	// sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);		//Turn on Alternate cameras
	port_com_send(cmd);

	EyelockLog(logger, DEBUG, "set_cam_mode(0x04,%d)");

	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");

	EyelockLog(logger, DEBUG, "Leave the PLL always ON");

	//Reading the calibrated Rect
	int targetOffset = 0; // 3;		//Adding offset

	no_move_area.x = rectX/scaling;
	no_move_area.y = rectY/scaling + targetOffset;
	no_move_area.width = rectW/scaling;
	no_move_area.height = (rectH)/scaling -targetOffset*2;

	// printf("no_move_area 	x: %d	y: %d	w: %d	h: %d\n", no_move_area.x, no_move_area.y, no_move_area.height, no_move_area.width);
	search_eye_area = seacrhEyeArea(no_move_area);

	// system("touch /home/root/Eyelock.run");
	faceConfigInit = true;

	// Init the seed with 0 during startup
	char cmd1[100];
	sprintf(cmd1,"cam_set_seed(%i)", 0);		//Set the seed
	port_com_send(cmd1);

	MainIrisSettings();
	string camID;
	camID = "0x47"; //camID = "0x08"; 	//
	setCamera(camID, FRAME_DELAY);

	DimmFaceForIris();	//Dim face settings

}

float FaceTracker::CalcExposureLevel(int width, int height,unsigned char *dsty, int limit)
{
	EyelockLog(logger, TRACE, "CalcExposureLevel");
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

	EyelockLog(logger, TRACE, "average : %3.1f percentile : %3.1f\n",average,percentile);

	// imshow("AGC", smallImg);
	// cvWaitKey(1);
	return (float)percentile;
}

Mat FaceTracker::rotation90(Mat src)
{
	EyelockLog(logger, TRACE, "rotation90");
	transpose(src, dst);
	flip(dst,dst,0);
	return dst;
}

int FaceTracker::IrisFramesHaveEyes()
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

Mat FaceTracker::preProcessingImg(Mat outImg)
{
	EyelockLog(logger, TRACE, "preProcessing");

	cv::resize(outImg, smallImgBeforeRotate, cv::Size(), (1 / scaling),
			(1 / scaling), INTER_NEAREST);	//py level 3

	EyelockLog(logger, TRACE, "After resize in preProcessingImg");
	if(bFaceMapDebug){
		imshow("OriginalFaceImage", outImg);
		cvWaitKey(1);
	}
	
	smallImg = rotation90(smallImgBeforeRotate);	//90 deg rotation

	//AGC control to block wash out images
	EyelockLog(logger, TRACE, "AGC Calculation");

	return smallImg;
}


void FaceTracker::moveMotorToFaceTarget(float eye_size, bool bShowFaceTracking, bool bDebugSessions)
{
	EyelockLog(logger, TRACE, "Entering > moveMotorToFaceTarget");
	if ((eye_size >= MIN_FACE_SIZE) && (eye_size <= MAX_FACE_SIZE)) {// check face size

		float err;
		int MoveToLimitBound = 1;
		//err = (no_move_area.y + no_move_area.height / 2) - eyes.y;		//Following no_move_area
		//instead of following no_move_area we will use search_eye_area to make eyes at the center of no_move_area

		float denum;

		if(m_EyelockIrisMode == 2)
			denum = 1.25;
		else
			denum = 1.0/2.0;

		err = (search_eye_area.y + search_eye_area.height * denum) - eyes.y;

		EyelockLog(logger, DEBUG,
				"abs err----------------------------------->  %d\n", abs(err));
		err = (float) err * (float) SCALE * (float) ERROR_LOOP_GAIN;

		// if we need to move
		if (abs(err) > MoveToLimitBound) {
			EyelockLog(logger, DEBUG, "Switching ON IRIS LEDs!!!!\n");

			MoveRelAngle(-1 * err);
			last_angle_move = -1 * err;
			//Flash the streaming
			vs->flush();
		}

	} else {
		EyelockLog(logger, DEBUG, "Face out of range\n");
	}

}

void FaceTracker::faceModeState(bool bDebugSessions)
{
	EyelockLog(logger, TRACE, "Entering > faceModeState");
	MoveTo(CENTER_POS);
	//MoveToAbs(CENTER_POS_TEST);

	run_state = RUN_STATE_FACE;
	SetFaceMode();
}


int FaceTracker::SelectWhichIrisCam(float eye_size, int cur_state)
{
	EyelockLog(logger, TRACE, "Entering > SelectWhichIrisCam");

	if ((cur_state != STATE_MAIN_IRIS) && (cur_state != STATE_AUX_IRIS)) {
		// this is we are just getting into irises so hard decision not hysterises
		if (eye_size >= (switchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;

	}
	if (cur_state == STATE_MAIN_IRIS)
		if (eye_size < (switchThreshold - errSwitchThreshold))
			return STATE_AUX_IRIS;
		else
			return STATE_MAIN_IRIS;

	if (cur_state == STATE_AUX_IRIS)
		if (eye_size >= (switchThreshold + errSwitchThreshold))
			return STATE_MAIN_IRIS;
		else
			return STATE_AUX_IRIS;
}


void FaceTracker::SweepFaceBrightness(void)
{
	EyelockLog(logger, TRACE, "Entering > SweepFaceBrightness");
	// Sweep over the range step by step...
	if (m_nCalculatedBrightness < FACE_GAIN_MIN)
		m_nCalculatedBrightness = FACE_GAIN_MIN;

	m_nCalculatedBrightness += FACE_GAIN_STEP;

	if (m_nCalculatedBrightness > FACE_GAIN_MAX)
		m_nCalculatedBrightness = FACE_GAIN_MIN;

	// printf("SweepFaceBrightnessm nCurrentBrightnessLevel.%d\n", m_nCalculatedBrightness);

	agc_val = m_nCalculatedBrightness;

	SetFaceExposure(m_nCalculatedBrightness);
}



void FaceTracker::DoAgc(void)
{
	EyelockLog(logger, TRACE, "Entering > DoAgc");

	if(!foundFace)
		p = CalcExposureLevel(smallImg.cols, smallImg.rows, (unsigned char *) (smallImg.data),180);
	else
		p = CalcExposureLevel(m_CroppedFaceImageForAGC.cols, m_CroppedFaceImageForAGC.rows, (unsigned char *) (m_CroppedFaceImageForAGC.data),180);


	//if (p < FACE_GAIN_PER_GOAL - FACE_GAIN_HIST_GOAL)
	agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	//if (p > FACE_GAIN_PER_GOAL + FACE_GAIN_HIST_GOAL)
	//	agc_val = agc_val + (FACE_GAIN_PER_GOAL - p) * FACE_CONTROL_GAIN;
	agc_val = MAX(FACE_GAIN_MIN,agc_val);
	agc_val = MIN(agc_val,FACE_GAIN_MAX);

	//printf("Outside condition >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %3.3f Agc value = %d\n",p,agc_val);
	AGC_Counter++;
	// AgcCntr++;
	if (agc_set_gain != agc_val) 	;	// && AGC_Counter%2==0)
	{
		//	while (waitKey(10) != 'z');
		{
			static int agc_val_old = 0;
			if (abs(agc_val - agc_val_old) > 300) {
				// printf("[AGC]  %05.6f Agc value = %d\n",p,agc_val);
				SetFaceExposure(agc_val);		//comment out if O2 led is connected
				agc_val_old = agc_val;
			}
		}

		agc_set_gain = agc_val;
	}
}

//projectPointsPtr1() input needs to be scaled out to the input image size
cv::Point2i FaceTracker::projectPoints_IristoFace(cv::Point2i ptrI, cv::Point2f constant, float ConstDiv)
{
	/*
	x = ((y-c)/m);

	y = x*m + c

	Here, m is magnification offset (ConstDiv)
	c is the intersection in Iris plane (constant)
	y is the coordinates in Face plane
	calculated x will be the coordinate of Iris plane
	*/

	cv::Point2i ptrF;

	ptrF.x = ptrI.x *ConstDiv + constant.x;
	ptrF.y = ptrI.y *ConstDiv + constant.y;

#if 0 // Anita made it configurable due to calibration error
	ptrF.x-=20;
	ptrF.y+=20;
#else
	ptrF.x-=m_IrisToFaceMapCorrectionVal;
	ptrF.y+=m_IrisToFaceMapCorrectionVal;
#endif

	return ptrF;
}

int RunSystemCmdCal(const char *ptr){
	int status = system(ptr);
	return status;
}

void Motorfxrel(int dist)
{
	char cmd[512];
	sprintf(cmd, "fx_rel(%i)\n",dist);
	port_com_send(cmd);
	usleep(5000);
}


int FaceTracker::validateLeftRightEyecrops( int CameraId, cv::Point2i ptrI, cv::Rect leftRect, cv::Rect rightRect, cv::Mat IrisImage, cv::Mat faceImage, IplImage *eyeCrop)
{
	char filename[100];
	char key;
	cv::Point2i ptrF;
	std::ostringstream ssCoInfo;

	//Project Iris points to face image
	// try
	{
		cv::rectangle(faceImage, rightRect, cv::Scalar(255,255,255),1,0);
		cv::rectangle(faceImage, leftRect, cv::Scalar(255,255,255),1,0);
		ssCoInfo << "+";
		if(foundFace){
			if (CameraId == IRISCAM_AUX_LEFT){
				// light green
				ptrF = projectPoints_IristoFace(ptrI, constantAuxl, magOffAuxl);
				putText(faceImage,ssCoInfo.str().c_str(),cv::Point(ptrF.x,ptrF.y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(144,238,144),2);
			}else if (CameraId == IRISCAM_AUX_RIGHT){
				// light blue
				ptrF = projectPoints_IristoFace(ptrI, constantAuxR, magOffAuxR);
				putText(faceImage,ssCoInfo.str().c_str(),cv::Point(ptrF.x,ptrF.y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(230,216,173),2);
			}else if (CameraId == IRISCAM_MAIN_LEFT){
				// Red
				ptrF = projectPoints_IristoFace(ptrI, constantMainl, magOffMainl);
				putText(faceImage,ssCoInfo.str().c_str(),cv::Point(ptrF.x,ptrF.y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(0,0,255),2);
			}else if (CameraId == IRISCAM_MAIN_RIGHT){
				// Orange
				ptrF = projectPoints_IristoFace(ptrI, constantMainR, magOffMainR);
				putText(faceImage,ssCoInfo.str().c_str(),cv::Point(ptrF.x,ptrF.y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv:: Scalar(0,165,255),2);
			}
		}

		cv::Mat DisplayImage(cv::Size(DISPLAY_WIN_WIDTH, DISPLAY_WIN_HEIGHT), CV_8UC3, Scalar(0,0,0));

		cv::Mat resizeFace;
		resize(faceImage, resizeFace, cvSize(resizeFaceWidth, DISPLAY_WIN_HEIGHT), 0,0, INTER_LINEAR);

		cv::Mat resizeIris;
		resize(IrisImage, resizeIris, cvSize(resizeIrisWidth, resizeIrisHeight), 0,0, INTER_LINEAR);

		cv::Mat ColorIrisImage;
		cv::cvtColor(resizeIris, ColorIrisImage, CV_GRAY2BGR);

		resizeFace.copyTo(DisplayImage(cv::Rect(0,0,resizeFaceWidth,DISPLAY_WIN_HEIGHT)));
		ColorIrisImage.copyTo(DisplayImage(cv::Rect(resizeFaceWidth,IrisPointYLoc,resizeIrisWidth,resizeIrisHeight)));

		std::ostringstream ssCoInfo1;
		ssCoInfo1 << m_Deviceid << " CAM " <<   CameraId  <<  (CameraId & 0x80 ?  " AUX":" MAIN ") <<   m_Distance << "\"";
		putText(DisplayImage,ssCoInfo1.str().c_str(),cv::Point(980,60), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(255,255,255),2);

		cvNamedWindow("EXT", CV_WINDOW_NORMAL);
		cvSetWindowProperty("EXT", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		imshow("EXT", DisplayImage);
		key = cv::waitKey(1);
		switch (key)
		{
			case '1':
				m_CameraNo = 1;
				return 1;
			case '2':
				m_CameraNo = 2;
				return 2;
			case '3':
				m_CameraNo = 3;
				return 3;
			case '4':
				m_CameraNo = 4;
				return 4;
			case 'q':
				destroyWindow("EXT");
				system("killall -KILL ExtStreamImages");
				break;
			case 'm':
				//bool eyesInDetect = foundFace? detect_area.contains(eyes):false;		// Face in face camera field of view
				//bool eyesInViewOfIriscam = eyesInDetect ? search_eye_area.contains(eyes):false;		// Face/eyes in narrow Rect calculated from calibration
				////bool eyesInViewOfIriscamNoMove = eyesInDetect ? no_move_area.contains(eyes):false;	//Face/eyes in Bigger Rect calculated from calibration
				// if (eyesInDetect && !eyesInViewOfIriscam){
				moveMotorToFaceTarget(eye_size,bShowFaceTracking, bDebugSessions);
				//}
				break;
			case 'd':
				destroyWindow("EXT");
				printf("Enter the new Distance\n");
				cin >> m_Distance;
				break;
			case 's':
				// char DistanceValue;
				time_t timer;
				struct tm* tm1;
				time(&timer);
				tm1 = localtime(&timer);
				char time_str[100];
				strftime(time_str, 100, "%Y_%m_%d_%H-%M-%S", tm1);
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				/*if ((key != 0) && (key != 's') && (key >= LowerBound || key <= UpperBound)){
					DistanceValue = key;
				}*/
				if((CameraId == IRISCAM_MAIN_LEFT) || (CameraId == IRISCAM_MAIN_RIGHT))
					sprintf(filename,"DeviceTestingFiles/%d_Main_%d_%dInches_%s_%lu.bmp", m_Deviceid, CameraId, m_Distance, time_str, ts.tv_sec);
				else
					sprintf(filename,"DeviceTestingFiles/%d_Aux_%d_%dInches_%s_%lu.bmp", m_Deviceid, CameraId, m_Distance, time_str, ts.tv_sec);
				imwrite(filename, IrisImage);
				sprintf(filename,"DeviceTestingFiles/%d_Face_%dInches_%s_%lu.bmp", m_Deviceid, m_Distance, time_str, ts.tv_sec);
				imwrite(filename, faceImage);
				cv::Mat mateyeCrop = cv::cvarrToMat(eyeCrop);
				sprintf(filename,"DeviceTestingFiles/%d_eyeCrop_%dInches_%s_%lu.bmp", m_Deviceid, m_Distance, time_str, ts.tv_sec);
				imwrite(filename, mateyeCrop);
				break;

			case 'r':
				destroyWindow("EXT");
				printf("Enter the motor movement distance\n");
				int dist;
				cin >> dist;
				Motorfxrel(dist);
				break;
			case 'h':
				motorInit();
				break;
			default:
				break;

		};

	}
	/*catch (cv::Exception& e) {
		cout << e.what() << endl;
	}*/
	return m_CameraNo;

}

void setImage(IplImage *dst, IplImage *m_src) {

	int m_fileIndex = 1, m_fileEyeIndex = 0, m_maxFramesReset = 10;

	char *m_Imageformat = "./data/Good_255.pgm";

	char fpath[1024];

	sprintf(fpath, m_Imageformat, m_fileIndex, m_fileEyeIndex);

	int w = -1, h = -1, bits = -1;

	if (m_src) {
		for (int i = 0; i < min(dst->height, m_src->height); i++) {

			unsigned char *inp, *out;

			inp = ((unsigned char *) m_src->imageData) + i * (m_src->widthStep);

			out = ((unsigned char *) dst->imageData)

			+ (i + (dst->height >> 1) - (m_src->height >> 1))

			* (dst->widthStep) + (dst->widthStep >> 1);

			memcpy(out, inp, min(dst->width, m_src->width));
		}
	}

	m_fileEyeIndex++;
	if (m_fileEyeIndex > 1) {
		m_fileEyeIndex = 0;
		m_fileIndex++;
	}

}

#if 0

bool FaceTracker::ProcessIrisImage(IplImage *inputImage, cv::Mat faceImage, cv::Mat IrisImage, cv::Rect leftRect, cv::Rect rightRect)
{
	char filename[100];

	int cam_idd = 0;
	unsigned char frame_number = 0;

	if(inputImage->imageData != NULL)
	{
		cam_idd = inputImage->imageData[2]&0xff;
		frame_number = inputImage->imageData[3]&0xff;
	}

	char key = cvWaitKey(1);
	if(key == 's'){
		sprintf(filename,"IrisImage_%d_%d.pgm", m_Deviceid, cam_idd);
		cv::Mat mateye = cv::cvarrToMat(inputImage);
		imwrite(filename, mateye);
		mateye.release();
	}

	m_pSrv = new EyeDetectAndMatchServer(WIDTH, HEIGHT, m_detectLevel, "Eyelock.log");

	m_pSrv->LoadHaarClassifier("/home/root/data/adaboostClassifier.txt");

	// set the specularity mode
	m_pSrv->SetSingleSpecMode(true);
	m_pSrv->SetDoHaar(true);
	m_pSrv->SetHaarEyeZoom(7);

	m_pSrv->SetHaarImageShifts(m_pSrv->GetHaarImageShifts());
	m_pSrv->SetHaarImageSampling(m_pSrv->GetHaarImageSampling());

	bool val = false;
	m_pSrv->SetCovTestForDetection(val ? 1 : 0);
	m_pSrv->SetSpecCovEigenThresh(m_pSrv->GetSpecCovEigenThresh());
	m_pSrv->SetSpecEccThresh(m_pSrv->GetSpecEccThresh());


	EyeDetectorServer *detector = m_pSrv->GetEyeDetector();
	detector->SetSpecularityMagnitude(50);
	int a = detector->GetSpecularitySize();

	detector->SetMaskRadius(6);
	detector->SetVarianceThresholdMin(10.0f);
	detector->SetVarianceThresholdMax(0.1f);

	detector->SetSeparation(36);

	detector->SetSearchX(25);
	detector->SetSearchY(25);
	// printf("detector->GetSpecularitySize()...%d\n", detector->GetSpecularitySize());
	detector->SetBoxX(detector->GetSpecularitySize());
	detector->SetBoxY(detector->GetSpecularitySize());

	frame.setScratch(m_pSrv->GetScratch());
	Image8u img(inputImage, false);
	frame.SetImage(&img);
	bool result = false;
	result = m_pSrv->Detect(&frame);
	int left, top;
	CvScalar color = cvRealScalar(255);

	int NoOfHaarEyes = frame.GetNumberOfHaarEyes();

    int maxEyes = frame.GetEyeCenterPointList()->size();


	for(int eyeIdx = 0; eyeIdx < frame.GetNumberOfHaarEyes(); eyeIdx++)
	{
		CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);


		CEyeCenterPoint& centroid = frame.GetEyeCenterPointList()->at(eyeIdx);
		cv::Point2i ptrI= cv::Point2i(centroid.m_nCenterPointX, centroid.m_nCenterPointY);

		validateLeftRightEyecrops(cam_idd, ptrI, leftRect, rightRect, IrisImage, faceImage);
		// printf("eye Label Information Cam_idd %d frameidx %d eyeLabel %d projStatus %d\n", cam_idd, m_faceIndex, eyeLabel, m_projStatus);

	}

	return 0;
}
#else

int FaceTracker::ProcessIrisImage(IplImage *inputImage, cv::Mat faceImage, cv::Mat IrisImage, cv::Rect leftRect, cv::Rect rightRect)
{

	int cam_idd = 0;
	unsigned char frame_number = 0;

	if(inputImage->imageData != NULL)
	{
		cam_idd = inputImage->imageData[2]&0xff;
		frame_number = inputImage->imageData[3]&0xff;
	}

	Image8u img(inputImage, false);
	frame.SetImage(&img);
	bool detect = m_pSrv->Detect(&frame);

	//int NoOfHaarEyes = frame.GetNumberOfHaarEyes();

    // int maxEyes = frame.GetEyeCenterPointList()->size();
	int left, top;
	cv::Point2i ptrI;
	if(detect){
		for(int eyeIdx = 0; eyeIdx < frame.GetNumberOfHaarEyes(); eyeIdx++)
		{
			// CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);

			CEyeCenterPoint& centroid = frame.GetEyeCenterPointList()->at(eyeIdx);
			ptrI= cv::Point2i(centroid.m_nCenterPointX, centroid.m_nCenterPointY);

			// Get Eyecrop
			frame.GetCroppedEye(eyeIdx, m_EyeCrop, left, top);

			
			m_CameraNo = validateLeftRightEyecrops(cam_idd, ptrI, leftRect, rightRect, IrisImage, faceImage, m_EyeCrop);

		}
	}else{
		m_CameraNo = validateLeftRightEyecrops(cam_idd, ptrI, leftRect, rightRect, IrisImage, faceImage, m_EyeCrop);
	}

	return m_CameraNo;
}

#endif

int FaceTracker::streamVideo(int cam, cv::Rect FaceCoord, cv::Mat FaceImg)
{

	int w,h;

	cv::Rect rightRect, leftRect;

	//Separate Right eye Rect
	rightRect.x = FaceCoord.x;
	rightRect.y = FaceCoord.y;
	rightRect.height = FaceCoord.height;
	rightRect.width = FaceCoord.width/2.0;

	//Separate left eye Rect
	leftRect.x = FaceCoord.x + (FaceCoord.width/2.0);
	leftRect.y = FaceCoord.y;
	leftRect.height = FaceCoord.height;
	leftRect.width = FaceCoord.width/2.0;

	cv::Mat RotatedfaceImg = rotation90(FaceImg);
	cv::Mat ColorFaceImage;
	cv::cvtColor(RotatedfaceImg, ColorFaceImage, CV_GRAY2BGR);

	if(cam == 8192)
	{
		LeftIrisCamera->get(&w,&h,(char *)LeftIrisImg.data, false);
		cvSetData(m_LeftCameraIrisImage, LeftIrisImg.data, (int)LeftIrisImg.step[0]);
		m_CameraNo = ProcessIrisImage(m_LeftCameraIrisImage, ColorFaceImage, LeftIrisImg, leftRect, rightRect);
	}
	else if (cam == 8193)
	{
		RightIrisCamera->get(&w,&h,(char *)RightIrisImg.data, false);
		cvSetData(m_RightCameraIrisImage, RightIrisImg.data, (int)RightIrisImg.step[0]);
		m_CameraNo = ProcessIrisImage(m_RightCameraIrisImage, ColorFaceImage, RightIrisImg, leftRect, rightRect);
	}

	return m_CameraNo;
}

Mat msgImage = Mat(450, 1800, CV_8U);
Mat msgImage1 = Mat(450, 1800, CV_8U);	//for showing messages

string msgLine1 = "";			//for showing messages
string msgLine2 = "";
string msgLine3 = "";

void showMessage()

//Draws a window with up to five lines of text, ~60 characters per line.

{

	int fontFace = FONT_HERSHEY_COMPLEX;
	double fontScale = 1.5;
	int thickness = 2;

	msgImage = 0;

	putText(msgImage, msgLine1, Point(50,75), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine2, Point(50,150), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine3, Point(50,225), fontFace, fontScale, Scalar::all(255), thickness, 8);
//	putText(msgImage, msgLine4, Point(50,300), fontFace, fontScale, Scalar::all(255), thickness, 8);
//	putText(msgImage, msgLine5, Point(50,375), fontFace, fontScale, Scalar::all(255), thickness, 8);

	imshow("Message", msgImage);
	waitKey(1);
}

void FaceTracker::displayInstruction(){


	sprintf(textI9,"Instruction for Testing EXT");
	sprintf(textI10,"Press '1' for Main Left");
	//sprintf(extFocus::textI12,"2. The Window says TARGET VERIFIED!");
	////sprintf(extFocus::textI13,"Press 's' to save images");
	//sprintf(extFocus::textI14,"Press 'q' to continue ...");

	cv::putText(DisImg,textI9,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,textI10,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);

//	cv::putText(DisImg,extFocus::textI11,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
//	cv::putText(DisImg,extFocus::textI12,cvPoint(10,200), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
//	cv::putText(DisImg,extFocus::textI13,cvPoint(10,250), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
//	cv::putText(DisImg,extFocus::textI14,cvPoint(10,300), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
}

#if 0
	msgLine1 = "Enter the camera number as below";
	msgLine2 = "Sequence of Cameras is Main left Main Right Camera";
	msgLine3 = "3 for Aux left Camera; 4 for Aux Right Camera";
//	msgLine3 = "Press 'n' to move to new camera";
	showMessage();
	char key;
	key = cv::waitKey(1);

#endif
#if 0
	displayInstruction();
	cv::moveWindow(textI9, 1260, 10);
	cv::imshow(textI9, DisImg);
	key = cv::waitKey(1);

	sleep(10);
	static int firstEntry = 1;
	if(firstEntry){
		cin >> CameraNo;
		firstEntry = 0;
	}
#endif

static void help()
{
	cout << "\nThis Application is used for testing EXT\n"
			"Enter device ID and face target distance to proceed\n"
			"To switch between cameras use keys 1, 2, 3 and 4:\n"
			"To save images at new target distance press d, enter the distance, press enter "
			"Move the mouse on the display screen after entering the distance and then press 's' to save images\n"
			"Images are saved in /home/root/DeviceTestingFiles folder\n"
			"Hot keys work only if the mouse is on the displayed image screen, i.e EXT Window Screen\n"
			<< endl;
	cout << "Hot keys: \n"
	            "q - quit/exit the application\n"
	            "s - save images\n"
	            "d - Enter new target distance on console and then press enter, to save images at new distance press 's'\n"
				"m - motor movement based on face size\n"
	            "h - fx_home \n"
	            "r - Enter distance to move motor - fx_rel\n"
	            "1 - Main Left Camera\n"
	            "2 - Main Right Camera\n"
				"3 - Aux Left Camera\n"
				"4 - Aux Right Camera\n"
	            << endl;
	}

int FaceTracker::DoRunMode(bool bShowFaceTracking, bool bDebugSessions, int CameraNo)
{
	string camID;
	cv::Rect face;

	smallImg = preProcessingImg(outImg);

	foundFace = FindEyeLocation(smallImg, eyes, eye_size, face, MIN_FACE_SIZE, MAX_FACE_SIZE);

	face = adjustWidthDuringFaceDetection(face);

	//Scaling out projFace coordinates
	cv::Rect FaceCoord;
	FaceCoord.x = face.x * scaling;		//column
	FaceCoord.y = face.y * scaling; // rectY - (targetOffset * scaling);
	FaceCoord.width = face.width * scaling;
	FaceCoord.height = face.height *scaling; // rectH + (targetOffset * scaling);



	if(CameraNo == 1)
	{
		// Main Left Camera
		camID = "0x07"; // camID = "0x47"; //camID = "0x08"; 	//
		setCamera(camID, FRAME_DELAY);
		m_CameraNo = streamVideo(leftCam, FaceCoord, outImg);
	}
	else if(CameraNo == 2)
	{
		//For Main Right Camera
		camID = "0x07"; // camID= "0x10"; //
		setCamera(camID, FRAME_DELAY);
		m_CameraNo = streamVideo(rightCam, FaceCoord, outImg);
	}
	else if(CameraNo == 3)
	{
		//For Aux Left Camera
		camID = "0x87"; //camID = "0x01"; //
		setCamera(camID, FRAME_DELAY);
		m_CameraNo = streamVideo(leftCam, FaceCoord, outImg);
	}
	else if(CameraNo == 4)
	{
		//For Aux Right Camera
		camID = "0x87"; // camID = "0x02"; //
		setCamera(camID, FRAME_DELAY);
		m_CameraNo = streamVideo(rightCam, FaceCoord, outImg);
	}

	if(bShowFaceTracking)
	{
		EyelockLog(logger, TRACE, "Imshow");
		cv::rectangle(smallImg, no_move_area, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(smallImg, search_eye_area, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(smallImg, detect_area, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(smallImg, face, Scalar(255,0,0),1,0);
		imshow("FaceTracker", smallImg);
		cvWaitKey(1);
	}
	return m_CameraNo;
}

int main(int argc, char **argv)
{
	help();

	FaceTracker m_faceTracker("/home/root/data/calibration/Face.ini");

	EyelockLog(logger, TRACE, "init_facetracking Start FaceTracking Thread");

	printf("INSERT OIM DEVICE NUMBER THEN PRESS ENTER\n");
	cin >> m_faceTracker.m_Deviceid;

	printf("Enter the Distance\n");
	cin >> m_faceTracker.m_Distance;

	// Flag to enable/disable AES Encrption in port com
	bool bDoAESEncryption = m_faceTracker.FaceConfig.getValue("FTracker.AESEncrypt", false);

	int w, h;

	//vid_stream_start
	vs = new VideoStream(8194, m_faceTracker.m_ImageAuthentication); //Facecam is 8194...

	// Iris streams
	LeftIrisCamera = new VideoStream(8192, m_faceTracker.m_ImageAuthentication); //LeftCam is 8192...

	RightIrisCamera = new VideoStream(8193, m_faceTracker.m_ImageAuthentication); //RightCam is 8193...

	// Read the width and height of image from the Eyelock.ini file
	FileConfiguration Config("/home/root/Eyelock.ini");
	m_faceTracker.m_ImageWidth = Config.getValue("FrameSize.width", 1200);
	m_faceTracker.m_ImageHeight = Config.getValue("FrameSize.height", 960);

	LeftIrisImg = Mat(Size(m_faceTracker.m_ImageWidth, m_faceTracker.m_ImageHeight), CV_8U);

	RightIrisImg = Mat(Size(m_faceTracker.m_ImageWidth, m_faceTracker.m_ImageHeight), CV_8U);

	// Initialize Face Detection
	face_init();

	// Intialize Portcom for device control
	portcom_start(bDoAESEncryption);

	// Load FaceTracking config... Configure all hardware
	m_faceTracker.DoStartCmd();

	// Main Loop...
	outImg = Mat(Size(m_faceTracker.m_ImageWidth, m_faceTracker.m_ImageHeight), CV_8U);

#if 1
	while(1){
		vs->get(&w, &h, (char *) outImg.data, false);
		// printf("m_CameraNo....%d\n", m_faceTracker.m_CameraNo);
		if(m_faceTracker.m_CameraNo == 1){
			// printf("Testing Left Main Camera\n");


				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(m_faceTracker.bShowFaceTracking, false, 1);

		}else if (m_faceTracker.m_CameraNo == 2){
			// printf("Testing Right Main Camera\n");


			m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(m_faceTracker.bShowFaceTracking, false, 2);
		}else if (m_faceTracker.m_CameraNo == 3){

				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(m_faceTracker.bShowFaceTracking, false, 3);

		}else if (m_faceTracker.m_CameraNo == 4){
			// printf("Testing Right Aux Camera\n");

			m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(m_faceTracker.bShowFaceTracking, false, 4);
		}

	}
#else
	string camID;
	while (1) {
		printf("m_CameraNo....%d\n", m_faceTracker.m_CameraNo);
		if (m_faceTracker.m_CameraNo == 1) {
			printf("Testing Left Main Camera\n");
			static int firstEntry = 1;
			if (firstEntry) {
				camID = "0x07"; //camID = "0x08"; 	//
				setCamera(camID, m_faceTracker.FRAME_DELAY);
				firstEntry = 0;
			}
			while (1) {
				vs->get(&w, &h, (char *) outImg.data, false);
				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(
						m_faceTracker.bShowFaceTracking, false, 1);
				if (m_faceTracker.m_CameraNo != 1) {
					firstEntry = 1;
					break;
				}
			}

		} else if (m_faceTracker.m_CameraNo == 2) {
			printf("Testing Right Main Camera\n");
			static int firstEntry = 1;
			if (firstEntry) {
				camID = "0x07"; //camID = "0x08"; 	//
				setCamera(camID, m_faceTracker.FRAME_DELAY);
				firstEntry = 0;
			}
			while (1) {
				vs->get(&w, &h, (char *) outImg.data, false);
				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(
						m_faceTracker.bShowFaceTracking, false, 2);
				if (m_faceTracker.m_CameraNo != 2) {
					firstEntry = 1;
					break;
				}
			}
		} else if (m_faceTracker.m_CameraNo == 3) {
			static int firstEntry = 1;
			if (firstEntry) {
				camID = "0x07"; //camID = "0x08"; 	//
				setCamera(camID, m_faceTracker.FRAME_DELAY);
				firstEntry = 0;
			}
			while (1) {
				vs->get(&w, &h, (char *) outImg.data, false);
				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(
						m_faceTracker.bShowFaceTracking, false, 3);
				if (m_faceTracker.m_CameraNo != 3) {
					firstEntry = 1;
					break;
				}
			}

		} else if (m_faceTracker.m_CameraNo == 4) {
			printf("Testing Right Aux Camera\n");
			static int firstEntry = 1;
			if (firstEntry) {
				camID = "0x07"; //camID = "0x08"; 	//
				setCamera(camID, m_faceTracker.FRAME_DELAY);
				firstEntry = 0;
			}
			while (1) {
				vs->get(&w, &h, (char *) outImg.data, false);
				m_faceTracker.m_CameraNo = m_faceTracker.DoRunMode(
						m_faceTracker.bShowFaceTracking, false, 4);
				if (m_faceTracker.m_CameraNo != 4) {
					firstEntry = 1;
					break;
				}
			}
		}

	}

#endif
	outImg.release();

}










