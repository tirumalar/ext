/*
 * ImageProcessor.cpp
 *
 *  Created on: 19 Feb, 2009
 *      Author: akhil
 */

#include "ImageProcessor.h"
#include "Aquisition.h"
#include "FrameGrabberFactory.h"
#include "Image.h"
#include "EyeDetectorServer.h"
#include "IrisSelectServer.h"
#include "EyeTracker.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <time.h>
#include "logging.h"
#include "ProcessorChain.h"
#include "LaplacianBasedFocusDetector.h"
#include <unistd.h>
#include "UtilityFunctions.h"
#include "ISOBiometric.h"


#ifdef IRIS_CAPTURE
#define NEW_LAPLACIAN_METHOD
#define cvSetImageData cvSetData
#endif

#define EYE_SIDE

const char logger[30] = "ImageProcessor";

extern "C"{
#ifdef __BFIN__
	#include <bfin_sram.h>
#endif
}
#include "SocketFactory.h"

using namespace std;

#ifdef __ANDROID__
#define printf LOGI
#define LOGFILE_PATH "/mnt/sdcard/Eyelock/GRIDemo.log"
#else
#define LOGFILE_PATH "GRIDemo.log"
#endif

#ifdef DEBUG_SESSION
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define DEBUG_SESSION_DIR "DebugSessions/Session"
#endif


#ifdef IRIS_CAPTURE
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

long ptime_in_ms_from_epoch1(const boost::posix_time::ptime& pt)
{
	using boost::posix_time::ptime;
	using namespace boost::gregorian;
	return (pt-ptime(date(2013, Dec, 30))).total_milliseconds();
}
#endif

//#undef TIME_OP
//#define TIME_OP XTIME_OP
//int myprintf( char *fmt, ... ) {}
//#define printf myprintf

ImageProcessor::ImageProcessor(Configuration* pConf, CircularAccess<SafeFrameMsg *>& outMsgQueue, SafeFrameMsg& detectedMsg, SafeFrameMsg& motionMsg):m_rotationBuff(0),m_id(0),
m_outMsgQueue(outMsgQueue), m_DetectedMsg(detectedMsg),m_MotionMsg(motionMsg),m_src(0),m_LogFileName(LOGFILE_PATH),m_spoofEnable(false),m_spoofDebug(false),m_EyeTracker(0),
m_DetectedEyeWriteIterator(m_DetectedEyesQueue),m_MotionDetection(0),m_FuturisticTime(0),m_outMsg(256),m_binType(0),m_FlipType(2),m_timestampBeforeGrabbing(0),m_IrisSelectSvr(0),
m_SpoofTrackerCount(0),m_nanoSpoofDetector(0),m_nanoSpoofDataCount(0),m_FocusImageLevel(0),m_smallCroppedEye(0),m_maxEyes(10),m_blackThreshold(0),m_enableBlackLevel(false),
m_enableAreaFocus(false),m_blackMinThreshold(0.0),m_NumTrackedEyes(0),m_socketFactory(0),m_saveDiscardedEyes(false),m_discardedSaveCount(0),m_prevTS(0),m_timegapForStaleframe(250000),
m_haloMinCount(20),m_haloMaxCount(110),m_il0(0),m_faceIndex(0),m_secondLevelImage(NULL),m_confusionTimeThresholduSec(0),m_prevConfusionTS(0),m_checkConfusion(false),
m_haloTopPixelsPercentage(25.0f),m_haloCountByBottomPixels(6),m_haloBottomPixelsIntensityThresh(91),m_MHaloNegationThresh(350),m_FaceIrisMapping(false),bIrisToFaceMapDebug(false),m_projStatus(true),m_IrisCameraPreview(false){
printf("ImageProcessor::ImageProcessor: set the configuration file\n"); fflush(stdout);

m_tsDestAddrpresent=false;
m_nwLedDispatcher = NULL;
m_LedConsolidator = NULL;
//m_pCmxHandler = NULL;

#if 1
	setConf(pConf);

#ifndef COMMENT
    //m_binType
//    int rowbin = pConf->getValue( "MT9P031.row_bin",0);
//    int rowskip = pConf->getValue( "MT9P031.row_skip",0);
//    if(rowbin||rowskip){
//    	m_binType =1;
//    }
//
//    int colbin = pConf->getValue( "MT9P031.column_bin",0);
//    int colskip = pConf->getValue( "MT9P031.column_skip",0);
//    if(colbin||colskip){
//    	m_binType = 2;
//    }

	if(FrameGrabberFactory::getSensorType(pConf)==-1)
	{
		aquisition= new AquisitionFile(pConf,m_binType);
	}
	else if(FrameGrabberFactory::getSensorType(pConf)==5){
		aquisition= new AquisitionBuffer(pConf);
	}
	else
	{
		bool singframconf = pConf->getValue("GRI.Singleframe",false);
		aquisition= new Aquisition(pConf,singframconf);
	}
	int w=0,h=0;
	aquisition->getDims(w,h);

	m_faceIndex=-1;

	m_Debug = pConf->getValue("GRI.HBDebug",false);
	m_svrAddr = (char*)pConf->getValue("GRI.StatusDestAddr","NONE");
    if(strcmp(m_svrAddr, "NONE") == 0){
        m_svrAddr = 0;
    }
    if(m_svrAddr){
        m_statusDestAddr = HostAddress::MakeHost(m_svrAddr);
        m_msgFormat = pConf->getValue("GRI.StatusNwMsgFormat", "Alive");
        m_HBFreqSec = pConf->getValue("GRI.HBFreqInSec", 5);
        m_HBTimeOutmilliSec = pConf->getValue("GRI.HBTimeoutInmilliSec", 2000);
        char *msgBuff = m_outMsg.GetBuffer();
        int len = snprintf(msgBuff, m_outMsg.GetAvailable(), m_msgFormat)+1;
        m_outMsg.SetSize(len);
	}
	else
	{
        printf("Status destination address not specified\n");
    }

// read values from config
    m_shouldDetect = checkLicense() ? pConf->getValue("GRI.shouldDetectEye", true) : false;
    m_saveCount = pConf->getValue("GRI.saveCount", 0);
    m_outSaveCount = pConf->getValue("GRI.OutsaveCount", 0);
    m_shouldLog = pConf->getValue("GRI.shouldLog", false);
    int cw = 0, ch = 0;

    cw = pConf->getValue("GRI.cropWidth", 384);
    ch = pConf->getValue("GRI.cropHeight", 288);
    m_shouldRotate = pConf->getValue("GRI.rotate", false);
    int qSize = pConf->getValue("GRI.DetectedEyesQueueSize", 20);
    m_testImageSendLevel=pConf->getValue("GRI.TestImageLevel",EYE_DETECTION_LEVEL);
    if(m_testImageSendLevel!=0&&m_testImageSendLevel!=EYE_DETECTION_LEVEL){
			printf("\ninvalid GRI.TestImageLevel:%d , setting to %d\n",m_testImageSendLevel,EYE_DETECTION_LEVEL);
			m_testImageSendLevel=EYE_DETECTION_LEVEL;
	}

    m_sframe.setBinning(m_binType);

	m_eyeDetectionLevel =pConf->getValue("GRI.DetectImageLevel",EYE_DETECTION_LEVEL);
	if((m_eyeDetectionLevel > 3)){
		printf(" Invalid config for detection Level changing it to Level 2\n");
		m_eyeDetectionLevel = EYE_DETECTION_LEVEL;
	}
	printf("EyeDetect Level = %d\n",m_eyeDetectionLevel);

    if(m_shouldDetect)
    {
        m_pSrv = new EyeDetectAndMatchServer(w, h,m_eyeDetectionLevel,m_LogFileName);
        configureDetector();
        char *adbfilename = (char*)pConf->getValue("GRI.ClassifierFilename","data/adaboostClassifier.txt");
        printf("Loading Classifier file %s \n",adbfilename);
        m_pSrv->LoadHaarClassifier(adbfilename);
        m_sframe.setScratch(m_pSrv->GetScratch());
        m_scratch = 0;
    }else{

        m_pSrv = 0;
#ifdef __BFIN__
        m_scratch = (char*)(sram_alloc(5 * w, L1_DATA_SRAM));
#else
        m_scratch = (char*)(malloc(4*1024*1023));
#endif
        assert(m_scratch);
        m_sframe.setScratch(m_scratch);
        qSize = 1;
    }
    m_DetectedEyesQueue(qSize);
    int numbits = pConf->getValue("Eyelock.NumBits",8);
    numbits = numbits > 8?16:numbits;
    for(int i = 0;i < qSize;i++){
        m_DetectedEyesQueue[i] = new DetectedEye(cw, ch,numbits);
    }
    m_FlipType = pConf->getValueIndex("GRI.Flip", -1, 2, 2, "Both", "Horizontal", "Vertical", "None");

    if(m_shouldRotate)
		m_rotationBuff=cvCreateImage(cvSize(ch,cw),IPL_DEPTH_8U,1);
    m_FocusBasedRejectionType = pConf->getValueIndex("GRI.FocusBasedRejection", 0, 4, 0, "None", "Threshold", "PeakDetection", "RunningMax", "BlockMax");
    if(!m_shouldDetect)
        m_FocusBasedRejectionType = 0;

	m_FocusImageLevel = pConf->getValue("GRI.FocusImageLevel", 1); //By default will work for level 1
	int dim=pConf->getValue("GRI.FocusSelectorROIDim",64);
	//Madhav Find the first level size
	int sw,sh;
	sw=sh = dim+4;
	m_FocusRoi=cvRect(0,0, dim, dim);

	if(1==m_FocusImageLevel){
		m_rippedImage = cvCreateImageHeader(cvSize(cw,ch),IPL_DEPTH_8U,1);
		m_rippedImage->width =  (m_FocusRoi.width+4)<<1;
		m_rippedImage->height =  (m_FocusRoi.height+4)<<1;
	}

	m_smallCroppedEye = cvCreateImage(cvSize(sw,sh),IPL_DEPTH_8U,1);
	m_IrisSelectSvr = new IrisSelectServer(sw,sh);
	m_IrisSelectSvr->SetEigenvalueThreshold(pConf->getValue("GRI.FocusSelectorEigenThreshold",7.0f));

	int threshold = pConf->getValue("GRI.FocusThresh",2500);
	int blockSize = pConf->getValue("GRI.FocusBasedRejection.BlockSize",2);

	m_spoofEnable = pConf->getValue("Eyelock.SpoofEnable",false);

    if(m_spoofEnable){
    	m_FocusBasedRejectionType = 0;
       	m_enableAreaFocus = pConf->getValue("Eyelock.EnableAreaFocus",false);
    	m_EyeTracker = new EyeTracker(threshold,false,blockSize);
    	m_spoofDebug = pConf->getValue("GRI.SpoofDebug",false);
   }

	m_nanoSpoofDetector = new NanoFocusSpecularityBasedSpoofDetector();
	m_nanoSpoofDetector->SetSpecularityValue(pConf->getValue("Eyelock.SpoofSpecVal",230));
	m_areaThreshold = pConf->getValue("Eyelock.SpoofAreaThresh",240.0f);
	m_focusThreshold = pConf->getValue("Eyelock.SpoofFocusThresh",0.85f);
	printf("Area threshold = %f Focus threshold = %f \n",m_areaThreshold,m_focusThreshold);

	m_blackThreshold = pConf->getValue("Eyelock.BlackThreshold",50.0f);
	m_blackMinThreshold = pConf->getValue("Eyelock.BlackMinThreshold",0.0f);
	m_enableBlackLevel = pConf->getValue("Eyelock.EnableBlackLevel",false);

	m_haloThreshold  = pConf->getValue("Eyelock.HaloThreshold",180.0f);
	m_currHaloThreshold = m_haloThreshold;
	m_haloMinThreshold = pConf->getValue("Eyelock.HaloMinThreshold",0.0f);
	m_haloMinCount = pConf->getValue("Eyelock.HaloMinCount",20.0f);
	m_haloMaxCount = pConf->getValue("Eyelock.HaloMaxCount",110.0f);
	m_enableHaloThreshold = pConf->getValue("Eyelock.EnableHaloThreshold",false);
	m_haloCountByBottomPixels = pConf->getValue("Eyelock.HaloCountByBottomPixels",6);
	m_haloTopPixelsPercentage = pConf->getValue("Eyelock.HaloTopPixelsPercentage",25.0f);
	m_haloBottomPixelsIntensityThresh = pConf->getValue("Eyelock.HaloBottomPixelsIntensityThresh",91);
	m_MHaloNegationThresh = pConf->getValue("Eyelock.MHaloNegationThresh",350);

	m_saveDiscardedEyes = pConf->getValue("Eyelock.SaveDiscardedEyes",false);

	if(m_FocusBasedRejectionType){
		if(m_FocusBasedRejectionType==1){
			printf("Using threshold based focus selection with thresh=%d \n",threshold);
			m_EyeTracker = new EyeTracker(threshold,false,0);
		}
		else if(m_FocusBasedRejectionType==2) {
			printf("Using peak detection based focus selection with thresh=%d\n",threshold);
			m_EyeTracker = new EyeTracker(threshold,true,0);
		}
		else if(m_FocusBasedRejectionType==3)
		{
			printf("Using running max based focus selection with thresh=%d and block=%d\n",threshold,blockSize);
			m_EyeTracker = new EyeTracker(threshold,true,blockSize);
		}
		else
		{
			if(!m_EyeTracker){
				printf("Using block max based focus selection with thresh=%d and block=%d\n",threshold,blockSize);
				m_EyeTracker = new EyeTracker(threshold,false,blockSize);
			}
		}
	}

    InitializeMotionDetection(pConf, w, h);
    writer=new ImageMsgWriter(pConf);
	defBuffSize=cw*ch*(numbits>>3) + 2048;

	m_Imageformat = pConf->getValue("Eyelock.ImageFormat","NONE");
	if(strcmp(m_Imageformat,"NONE")==0){
		m_Imageformat = NULL;
	}
	m_ImageMatchTime = pConf->getValue("Eyelock.ImageMatchTime",10);	// 10 sec
	const char *camid =  pConf->getValue("GRI.cameraID","NONE");
	m_maxFramesReset = pConf->getValue("Eyelock.MaxFramesReset",200);
	m_CamStr.assign(camid);
	m_fileIlluState=0;
	m_fileIndex=1;
	const char *fpath = pConf->getValue("GRI.FilePath","NONE");
	if(strcmp(fpath,"NONE")!=0){
		printf("Loading file %s\n",fpath);

		int w,h,bits;
		ReadPGM5WHandBits(fpath,&w,&h,&bits);
		m_src = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
		ReadPGM5(fpath,(unsigned char *)m_src->imageData,&w,&h,m_src->imageSize);
	}


	m_maxEyes = pConf->getValue("Eyelock.MaxDetectedEyesPerFrame",3);
	m_bufSize = pConf->getValue("GRI.sendBufferSize", defBuffSize);
	m_hbIntervalUSecs=1000000*pConf->getValue("GRI.heartBeatIntervalSecs",1);
	gettimeofday(&m_lastHBSent,0);
#endif
#endif
	m_socketFactory = new SocketFactory(*pConf);

	int value = m_timegapForStaleframe;
	value = pConf->getValue("Eyelock.TimeGapUsecForStaleFrame",value);
	m_timegapForStaleframe = value;

	m_checkConfusion = pConf->getValue("Eyelock.EnableConfusion",false);
	m_confusionTimeThresholduSec = pConf->getValue("Eyelock.ConfusionTimeThresholdMilliSec",3000);
	m_confusionTimeThresholduSec = m_confusionTimeThresholduSec*1000;
	m_confusionDebug = pConf->getValue("Eyelock.ConfusionDebug",false);

	const char *svrAdd = pConf->getValue("Eyelock.TSMasterDestAddr", "NONE");
	if(strcmp(svrAdd, "NONE") == 0){
    	m_tsDestAddrpresent = false;
	}else
		m_tsDestAddrpresent = true;


	m_ledPWMAddress = pConf->getValue("Eyelock.LEDPWMAddress",0);
	m_ledPWMDefaultValue = pConf->getValue("Eyelock.LEDPWMValueNormal",0);
	m_ledPWMDefaultValueAfterDetect = pConf->getValue("Eyelock.LEDPWMValueDetection",0);
	m_maxSpecValue = pConf->getValue("Eyelock.MaxSpecValue",230);
	m_currMaxSpecValue = m_maxSpecValue;
	m_maxSpecValueAfterDetect = pConf->getValue("Eyelock.MaxSpecValueAfterDetect",230);

	m_haloThresholdAfterDetect = pConf->getValue("Eyelock.HaloThresholdAfterDetect",m_haloThreshold);

	m_DcOffset = (unsigned short)pConf->getValue("GRI.dc", (int)0);
	m_ShiftRight = (unsigned short)pConf->getValue("GRI.shiftRight", (int)0);

	m_DcOffsetAfterDetect = (unsigned short)pConf->getValue("GRI.dcAfterDetect", (int)0);
	m_ShiftRightAfterDetect = (unsigned short)pConf->getValue("GRI.shiftRightAfterDetect", (int)0);

	m_enableLaplacian_focus_Threshold = pConf->getValue("Eyelock.EnableLaplacian_focus_Threshold",false);
	m_laplacian_focusThreshold  = pConf->getValue("Eyelock.Laplacian_focus_Threshold",0.15f);
	m_enableLaplacian_focus_ThresholdEnroll = pConf->getValue("Eyelock.EnableLaplacian_focus_ThresholdEnroll",false);
	m_laplacian_focusThresholdEnrollment  = pConf->getValue("Eyelock.Laplacian_focus_ThresholdEnroll",0.30f);
	m_lapdebug = pConf->getValue("Eyelock.LapDebug",0);
	printf("Lap Match %f Lap Enroll %f \n",m_laplacian_focusThreshold,m_laplacian_focusThresholdEnrollment);

	m_rotatedEyecrop = 0;
	m_centreEyecrop = 0;
	m_centreEyecropEnroll=0;
	m_lapdetector = NULL;
	m_lapdetectorEnroll = NULL;
	if (m_enableLaplacian_focus_Threshold||m_enableLaplacian_focus_ThresholdEnroll){
		int w1 = pConf->getValue("Eyelock.EnableLaplacianWidth",320);
		int h1 = pConf->getValue("Eyelock.EnableLaplacianHeight",240);
		m_lapdetector =  new LaplacianBasedFocusDetector(w1,h1);
		if(m_shouldRotate){
			m_rotatedEyecrop = cvCreateImage(cvSize(ch,cw),IPL_DEPTH_8U,1);
		}else{
			m_rotatedEyecrop = cvCreateImage(cvSize(cw,ch),IPL_DEPTH_8U,1);
		}
		m_centreEyecrop = cvCreateImage(cvSize(w1,h1),IPL_DEPTH_8U,1);

		int we = pConf->getValue("Eyelock.EnableLaplacianEnrollWidth",320);
		int he = pConf->getValue("Eyelock.EnableLaplacianEnrollHeight",240);
		m_lapdetectorEnroll =  new LaplacianBasedFocusDetector(we,he);
		m_centreEyecropEnroll = cvCreateImage(cvSize(we,he),IPL_DEPTH_8U,1);
	}
	m_SaveFullFrame = pConf->getValue("Eyelock.SaveFullFrames",false);
	m_SaveEyeCrops = pConf->getValue("Eyelock.SaveEyeCrops",false);
	m_IrisCameraPreview = pConf->getValue("Eyelock.ShowIrisCameraPreview",false);

	m_EnableOffsetCorrection = pConf->getValue("Eyelock.EnableGainOffsetCorrection",false);
	m_Imagewidth = pConf->getValue("FrameSize.width",1200);
	m_Imageheight = pConf->getValue("FrameSize.height",960);
	m_OffsetOutputImage = cvCreateImage(cvSize(m_Imagewidth, m_Imageheight),IPL_DEPTH_8U,1);

	m_ProcessImageFrame = cvCreateImage(cvSize(m_Imagewidth, m_Imageheight),IPL_DEPTH_8U,1);

	eyeCropFileNamePattern = pConf->getValue("Eyelock.EyeCropFileNamePattern","bmp");

	m_EnableExtHalo = pConf->getValue("Eyelock.EnableExtHalo",false);

	// Face Mapping Projection
	m_FaceIrisMapping = pConf->getValue("Eyelock.FaceIrisMapping",false);
	m_FaceIrisMappingStrict = pConf->getValue("Eyelock.Eyelock.FaceIrisMappingStrict",false);
	m_FaceIrisMappingBeforEyeDetection = pConf->getValue("Eyelock.FaceIrisMappingBeforEyeDetection",false);
	m_IrisProjImage = cvCreateImage(cvSize(m_Imagewidth, m_Imageheight),IPL_DEPTH_8U,1);
	m_SaveProjImage = pConf->getValue("Eyelock.SaveProjectedImage",false);
	m_showProjection = pConf->getValue("Eyelock.showProjection",false);
	m_IrisToFaceMapping = pConf->getValue("Eyelock.IrisToFaceMapping",false);

	m_IrisToFaceMapCorrectionVal = pConf->getValue("Eyelock.IrisToFaceMapCorrectionFactor", 20);

	bIrisToFaceMapDebug = pConf->getValue("Eyelock.IrisToFaceMapDebug", false);
	

	m_activeEyeSideLabeling = pConf->getValue("Eyelock.EyeSideLabeling",false);;

	m_minIrisDiameter = pConf->getValue("Eyelock.minIrisDiameter",int(150));

	FileConfiguration m_FaceConfig("/home/root/data/calibration/faceConfig.ini");

	m_OIMFTPEnabled = pConf->getValue("Eyelock.OIMFTPEnable", true);

	m_EyelockIrisMode = pConf->getValue("Eyelock.IrisMode",1);
	// printf("m_EyelockIrisModem_EyelockIrisMode...%d\n", m_EyelockIrisMode);

	if(m_OIMFTPEnabled){
		// Calibration Parameters from CalRectFromOIM.ini
		FileConfiguration CalRectConfig("/home/root/CalRect.ini");
		rectX = CalRectConfig.getValue("FTracker.targetRectX",0);
		rectY = CalRectConfig.getValue("FTracker.targetRectY",497);
		rectW = CalRectConfig.getValue("FTracker.targetRectWidth",960);
		rectH = CalRectConfig.getValue("FTracker.targetRectHeight",121);
		//ERROR_CHECK_EYES = m_FaceConfig.getValue("FTracker.ERROR_CHECK_EYES",float(0.06));
		magOffMainl = CalRectConfig.getValue("FTracker.magOffsetMainLeftCam",float(0.15));
		magOffMainR = CalRectConfig.getValue("FTracker.magOffsetMainRightCam",float(0.15));
		magOffAuxl = CalRectConfig.getValue("FTracker.magOffsetAuxLeftCam",float(0.22));
		magOffAuxR = CalRectConfig.getValue("FTracker.magOffsetAuxRightCam",float(0.22));

		magOffMainlDiv = 1.0/magOffMainl;
		magOffMainRDiv = 1.0/magOffMainR;
		magOffAuxlDiv = 1.0/magOffAuxl;
		magOffAuxRDiv = 1.0/magOffAuxR;

		constantMainl.x = CalRectConfig.getValue("FTracker.constantMainLeftCam_x",float(0.15));
		constantMainl.y = CalRectConfig.getValue("FTracker.constantMainLeftCam_y",float(0.15));
		constantMainR.x = CalRectConfig.getValue("FTracker.constantMainRightCam_x",float(0.15));
		constantMainR.y = CalRectConfig.getValue("FTracker.constantMainRightCam_y",float(0.15));
		constantAuxl.x = CalRectConfig.getValue("FTracker.constantAuxLeftCam_x",float(0.22));
		constantAuxl.y = CalRectConfig.getValue("FTracker.constantAuxLeftCam_y",float(0.22));
		constantAuxR.x = CalRectConfig.getValue("FTracker.constantAuxRightCam_x",float(0.22));
		constantAuxR.y = CalRectConfig.getValue("FTracker.constantAuxRightCam_y",float(0.22));

		useOffest_m = CalRectConfig.getValue("FTracker.projectionOffsetMain",false);
		useOffest_a = CalRectConfig.getValue("FTracker.projectionOffsetAux",true);
		projOffset_m = CalRectConfig.getValue("FTracker.projectionOffsetValMain",float(50.00));
		projOffset_a = CalRectConfig.getValue("FTracker.projectionOffsetValAux",float(200.00));

	}else{
		FileConfiguration CalibDefaultConfig("/home/root/data/calibration/Calibration.ini");

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

	}



	m_bFaceMapDebug = m_FaceConfig.getValue("FTracker.FaceMapDebug", false);

#ifdef IRIS_CAPTURE
	// bool bIrisMode = pConf->getValue("Eyelock.IrisMode", 1) == 2; // DMOTODO default to capture for now...
	m_bIrisCapture = true; // DMOTOD remove later,forced to capture until we get the config working...

	m_IrisCaptureEarlyTimeout = pConf->getValue("Eyelock.IrisCaptureEarlyTimeout", 5000);
	m_IrisCaptureTimeout = pConf->getValue("Eyelock.IrisCaptureTimeout", 5000);
	m_IrisCaptureResetDelay = pConf->getValue("Eyelock.IrisCaptureResetDelay", 2000);
	m_IrisCaptureDataMode = pConf->getValue("Eyelock.IrisCaptureDataMode", 1);
	m_IrisImageFormat = pConf->getValue("Eyelock.HttpPostSenderPayloadFormat", "FORMAT_RAW");
	m_IrisCaptureBestPairMax = pConf->getValue("Eyelock.IrisCaptureBestPairMax", 3);

	m_bIrisCaptureEnableResize = pConf->getValue("Eyelock.IrisCaptureEnableResize", false); // Hidden configged
	m_expectedIrisWidth = pConf->getValue("GRI.ExpectedIrisWidth", 200);
	m_actualIrisWidth = pConf->getValue("GRI.ActualIrisWidth", 130); //DMOTODO may want to base on segmented iris size...

	m_bSortingLogEnable = pConf->getValue("Eyelock.SortingLogEnable", false);

	m_xPixelResolutionPerCM = pConf->getValue("Eyelock.xPixelResolution", 100);
	m_yPixelResolutionPerCM = pConf->getValue("Eyelock.yPixelResolution", 100);


	// Sorting related
	m_nMinIrisDiameter = pConf->getValue("GRI.minIrisDiameter", 160);
	m_nMaxIrisDiameter = pConf->getValue("GRI.maxIrisDiameter", 210);

	int HaloThresh =  pConf->getValue("Eyelock.SortingHaloThresh",500);

	bool rejectQuality = pConf->getValue("Eyelock.EnableQualityBasedRejection",false);

	m_EnableISOSharpness  = pConf->getValue("Eyelock.EnableISOSharpness", false);

	m_ISOSharpnessThreshold  = pConf->getValue("Eyelock.ISOSharpnessThreshold",2200);

	if (m_bIrisCapture)
	{
		m_DHSScreens = pConf->getValue("Eyelock.DHSScreens", false);

		b_SaveBestEyes = pConf->getValue("Eyelock.SaveBestEyes",false);

		m_LEDBrightness = pConf->getValue("GRI.LEDBrightness", 80);
		if(m_LEDBrightness == 0)
			m_LEDBrightness = 1;

		m_pHttpPostSender = new HttpPostSender(*pConf);
		m_pHttpPostSender->init();
		m_pHttpPostSender->Begin();

		eyeSortingWrapObj= new EyeSortingWrap(6,255,m_IrisCaptureEarlyTimeout,m_IrisCaptureTimeout);
		eyeSortingWrapObj->SetDeviceType(Nano);
		eyeSortingWrapObj->SetPupilSearchAngles(-60, 90);
		eyeSortingWrapObj->SetSpecularityMaskLevel(1);
		eyeSortingWrapObj->SetEyeQualityClassifierWeights(0, 0.32f,0.38f);

		// Pupil Min and Max
		int minPupilDiameter = pConf->getValue("GRI.minPupilDiameter", 16);
		int maxPupilDiameter = pConf->getValue("GRI.maxPupilDiameter", 85);
		eyeSortingWrapObj->SetPupilRadiusSearchRange(minPupilDiameter, maxPupilDiameter);

		//Iris Min and Max
		int minIrisDiameter = pConf->getValue("GRI.minIrisDiameter", 70);
		int maxIrisDiameter = pConf->getValue("GRI.maxIrisDiameter", 145);
		eyeSortingWrapObj->SetIrisRadiusSearchRange(minIrisDiameter, maxIrisDiameter);

		//Search Area ROI
		CvRect searchArea;
		searchArea.x = pConf->getValue("GRI.EyeLocationSearchArea.x", searchArea.x);
		searchArea.y = pConf->getValue("GRI.EyeLocationSearchArea.y", searchArea.y);
		searchArea.width = pConf->getValue("GRI.EyeLocationSearchArea.width",	searchArea.width);
		searchArea.height = pConf->getValue("GRI.EyeLocationSearchArea.height", searchArea.height);
		bool fail = eyeSortingWrapObj->SetEyeLocationSearchArea(searchArea.x, searchArea.y, searchArea.width, searchArea.height);
		if (!fail) {
			EyelockLog(logger, WARN, "\nWARNING :Input given for EyeLocationSearchArea is invalid. ");
		}

#if 0
		eyeSortingWrapObj->SetIrisDiameterThresholds(70,170);
#else
		eyeSortingWrapObj->SetIrisDiameterThresholds(m_nMinIrisDiameter, m_expectedIrisWidth*1.5);//m_nMaxIrisDiameter); //DMOFIX!!! Need to hear back from Sarvesh...
#endif
		// eyeSortingWrapObj->SetHaloScoreTopPoints(6, 10, 140, 240);
		eyeSortingWrapObj->SetQualRatioMax(1.3f);
		eyeSortingWrapObj->SetQualThreshScore(9.5f);
		eyeSortingWrapObj->EnableQualityBasedRejection(rejectQuality);
		eyeSortingWrapObj->SetOldHaloRankWeight(0);

		eyeSortingWrapObj->SetOldHaloRankWeight(0.0f);
		// eyeSortingWrapObj->SetLaplacianRankWeight(0.6f);
	//DMOTODO (not in current SortingLib... is it needed?)	eyeSortingWrapObj->SetMinEyesInCluster(1);
		// eyeSortingWrapObj->SetLaplacian(m_laplacian_focusThresholdEnrollment);

		eyeSortingWrapObj->SetHaloScoreTopPoints(6, 18, 100, HaloThresh);
		eyeSortingWrapObj->SetEyeSortingLogEnable(m_bSortingLogEnable);

		terminate = false; //At the begining
		shouldIBeginSorting = false; //At the begining;
		m_scaleDest = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
		m_LastEyecrop = cvCreateImage(cvSize(640, 480),IPL_DEPTH_8U,1);
		tmpImage = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
		imgHeader1 = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
		imgHeader2 = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
	}

#endif // IRIS_CAPTURE



#ifdef DEBUG_SESSION
	m_DebugTesting = pConf->getValue("Eyelock.TestSystemPerformance",false);
	m_sessionDir = string(pConf->getValue("Eyelock.DebugSessionDir","DebugSessions/Session"));
#endif



	m_bShouldSend =pConf->getValue("Eyelock.EyeMessage",true);
	m_FaceFrameQueueSize = pConf->getValue("Eyelock.FaceQueueSize", 10);

	// If we want external screen images and we are in acquisition mode...
	// Create out window and set the initial image
#if 0
	if (m_DHSScreens && (m_EyelockIrisMode == 2))
	{
		Screen = cv::imread("/home/root/screens/Slide1.BMP", cv::IMREAD_COLOR);
		cvNamedWindow("EXT", CV_WINDOW_NORMAL);
		// cvSetWindowProperty("EXT", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		imshow("EXT", Screen);
		cvWaitKey(1); // cvWaitKey(100);
	}
#else
	if (m_DHSScreens && (m_EyelockIrisMode == 2))
	{
		IplImageScreen1 = cvLoadImage("/home/root/screens/Slide1.BMP", cv::IMREAD_COLOR);
		IplImageScreen2 = cvLoadImage("/home/root/screens/Slide2.BMP", cv::IMREAD_COLOR);
		IplImageScreen3 = cvLoadImage("/home/root/screens/Slide3.BMP", cv::IMREAD_COLOR);
		cvNamedWindow("EXT", CV_WINDOW_NORMAL);
		cvSetWindowProperty("EXT", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		cvShowImage("EXT", IplImageScreen1);
		cvWaitKey(5); // cvWaitKey(100);
	}
#endif
}

/*
//Use this if we later decided to move from no_move_area to search_eye_area for projection
#if 1
cv::Rect ImageProcessor::seacrhEyeArea(cv::Rect no_move_area){
	/// printf("no_move_area 	x: %d	y: %d	w: %d	h: %d\n", no_move_area.x, no_move_area.y, no_move_area.height, no_move_area.width);
	//printf("ERROR_CHECK_EYES %3.3f \n", ERROR_CHECK_EYES);

	float hclip = float(no_move_area.height - float(no_move_area.height * ERROR_CHECK_EYES));
	//printf("hclip::::: %3.3f\n", hclip);

	//float yclip = cvRound(hclip/2.0);
	float c = 1.0/2.0;
	float yclip = hclip * c;
	//printf("yclip::::: %3.3f\n", yclip);


	cv::Rect modRect;
	modRect.x = no_move_area.x;
	modRect.width = no_move_area.width;

	modRect.y = no_move_area.y + yclip;
	modRect.height = no_move_area.height - hclip;

	// printf("search_eye_area 	x: %d	y: %d	w: %d	h: %d\n", modRect.x, modRect.y, modRect.height, modRect.width);

#if 1 // To check if needed
	if(modRect.x < 0)
		modRect.x = 0;
	if(modRect.y < 0)
			modRect.y = 0;
	if(modRect.width > m_Imagewidth)
			modRect.width = m_Imagewidth;
	if(modRect.height > m_Imageheight)
			modRect.height = m_Imageheight;
#endif

	return modRect;
}
#endif
*/


static void flipOnYAxis(int &x, int &/*y*/, int width, int /* height */)
{
	x = width - x - 1;
}

static void flipOnXAxis(int &/*x*/, int &y, int /* width */, int height)
{
	y = height - y - 1;
}

void ImageProcessor::SetCMXHandler(CmxHandler *pCmxHandler)
{
	m_pCMXHandler = pCmxHandler;
}


void ImageProcessor::AddImageHandler(ImageHandler *pHandler)
{
	m_ImageHandlers.push_back(pHandler);
}

FrameGrabber *ImageProcessor::GetFrameGrabber()
{
	return aquisition->getFrameGrabber();
}

void ImageProcessor::FixCoordinates(int x, int y, int &u, int &v, int width, int height)
{
	if(m_shouldRotate) // really is transpose
	{
		int t = x;
		x = y;
		y = t;

		t = width;
		width = height;
		height = t;
	}
	if(m_FlipType != 2)
	{
		switch(m_FlipType)
		{

		case -1: flipOnXAxis(x, y,width, height); flipOnYAxis(x, y,width, height); break; // both axis
		case 0:  flipOnXAxis(x, y,width, height); break; // flip around x-axis
		case +1: flipOnYAxis(x, y,width, height); break; // flip around y-axis
		}
	}
	u = x;
	v = y;
}

ImageProcessor::~ImageProcessor() {
#ifndef COMMENT
	delete aquisition;
	delete writer;
	if(m_pSrv){
		delete m_pSrv; m_pSrv=0;
	}
	else{
		free(m_scratch);
	}
	if(m_rotationBuff)
		cvReleaseImage(&m_rotationBuff);
	if(m_IrisSelectSvr)
		delete m_IrisSelectSvr;
	if(m_EyeTracker)
		delete m_EyeTracker;
	if(m_smallCroppedEye)
		cvReleaseImage(&m_smallCroppedEye);

	if(1==m_FocusImageLevel){
		if(m_rippedImage)
			cvReleaseImageHeader(&m_rippedImage);
	}
	int qSize=m_DetectedEyesQueue.getSize();
	for(int i=0;i<qSize;i++){
		delete m_DetectedEyesQueue[i];
		m_DetectedEyesQueue[i]=0;
	}
	if(m_src)
		cvReleaseImage(&m_src);

	if(m_nanoSpoofDetector)
		delete m_nanoSpoofDetector;
	if(m_socketFactory)
		delete m_socketFactory;
	if(m_secondLevelImage)
		cvReleaseImage(&m_secondLevelImage);
	if(m_lapdetector)
		delete m_lapdetector;
	if(m_lapdetectorEnroll)
			delete m_lapdetectorEnroll;
	if(m_rotatedEyecrop){
		cvReleaseImage(&m_rotatedEyecrop);
	}
	if(m_centreEyecrop){
		cvReleaseImage(&m_centreEyecrop);
	}
	if(m_centreEyecropEnroll){
		cvReleaseImage(&m_centreEyecropEnroll);
	}
	if(m_OffsetImageMainCamera1){
		cvReleaseImage(&m_OffsetImageMainCamera1);
	}
	if(m_OffsetImageMainCamera2){
		cvReleaseImage(&m_OffsetImageMainCamera2);
	}
	if (m_OffsetImageAuxCamera1) {
		cvReleaseImage(&m_OffsetImageAuxCamera1);
	}
	if (m_OffsetImageAuxCamera2) {
		cvReleaseImage(&m_OffsetImageAuxCamera2);
	}
	if(m_OffsetOutputImage){
		cvReleaseImage(&m_OffsetOutputImage);
	}
	if(m_ProcessImageFrame){
		cvReleaseImage(&m_ProcessImageFrame);
	}
	if(m_IrisProjImage){
		cvReleaseImage(&m_IrisProjImage);
	}

#ifdef IRIS_CAPTURE
	if(m_pHttpPostSender)
		delete m_pHttpPostSender;

	if (eyeSortingWrapObj)
		delete eyeSortingWrapObj;

	if(m_bIrisCapture){
		if(m_scaleDest)
			cvReleaseImage(&m_scaleDest);
		if(m_LastEyecrop)
			cvReleaseImage(&m_LastEyecrop);
		if(tmpImage)
			cvReleaseImageHeader(&tmpImage);
		if(imgHeader1)
			cvReleaseImageHeader(&imgHeader1);
		if(imgHeader2)
			cvReleaseImageHeader(&imgHeader2);
	}
	if (m_DHSScreens && (m_EyelockIrisMode == 2)){
		if(IplImageScreen1)
			cvReleaseImageHeader(&IplImageScreen1);

		if(IplImageScreen2)
			cvReleaseImageHeader(&IplImageScreen2);

		if(IplImageScreen3)
			cvReleaseImageHeader(&IplImageScreen3);
	}
#endif
#endif
}

void ImageProcessor::InitializeMotionDetection(Configuration *& pConf, int w, int h)
{
    int enablemotion = pConf->getValue("GRIMotion.Enable", 0);
    if(enablemotion)
        m_MotionDetection = new SimpleMotionDetection(pConf, w, h);
}

void ImageProcessor::configureDetector(){
	// set the specularity mode
	Configuration *pConf=getConf();
	m_pSrv->SetSingleSpecMode(pConf->getValue("GRI.SingleSpecMode",false));
	m_pSrv->SetDoHaar(pConf->getValue("GRI.DoHaar",true));
	m_pSrv->SetHaarEyeZoom(pConf->getValue("GRI.HaarEyeZoom",m_pSrv->GetHaarEyeZoom()));
	m_pSrv->SetHaarImageShifts(pConf->getValue("GRI.HaarImageShifts",m_pSrv->GetHaarImageShifts()));
	m_pSrv->SetHaarImageSampling(pConf->getValue("GRI.HaarImageSampling",m_pSrv->GetHaarImageSampling()));

	bool val = pConf->getValue("GRI.CovarianceTestForDetection",false);

	m_pSrv->SetCovTestForDetection(val?1:0);
	m_pSrv->SetSpecCovEigenThresh(pConf->getValue("GRI.SpecularityCovarianceEigenvalueThreshold",m_pSrv->GetSpecCovEigenThresh()));
	m_pSrv->SetSpecEccThresh(pConf->getValue("GRI.SpecularityEccentricityThreshold",m_pSrv->GetSpecEccThresh()));



	EyeDetectorServer *detector=m_pSrv->GetEyeDetector();
	detector->SetSpecularityMagnitude(pConf->getValue("GRI.EyeDetectionSpecularityMagnitude",15));
	int a = detector->GetSpecularitySize();
#ifndef __BFIN__
	detector->SetSpecularitySize(pConf->getValue("GRI.EyeDetectionSpecularitySize",4));
#endif
	detector->SetMaskRadius(pConf->getValue("GRI.EyeDetectionMaskRadius",10));
	detector->SetVarianceThresholdMin(pConf->getValue("GRI.EyeDetectionVarianceThresholdMin",1.5f));
	detector->SetVarianceThresholdMax(pConf->getValue("GRI.EyeDetectionVarianceThresholdMax",0.666f));
	detector->SetSeparation(pConf->getValue("GRI.EyeDetectionSeparation",36));
	detector->SetSearchX(pConf->getValue("GRI.EyeDetectionSearchX",15));
	detector->SetSearchY(pConf->getValue("GRI.EyeDetectionSearchY",10));
	detector->SetBoxX(pConf->getValue("GRI.EyeDetectionBoxX",detector->GetSpecularitySize()));
	detector->SetBoxY(pConf->getValue("GRI.EyeDetectionBoxY",detector->GetSpecularitySize()));

	detector->m_shouldLog=m_shouldLog;

}

bool ImageProcessor::shouldSendHearBeat(){
	gettimeofday(&m_tempTime,0);
	return tvdelta(&m_tempTime,&m_lastHBSent)>=m_hbIntervalUSecs;
}
void ImageProcessor::CopyToOneFourth(IplImage *frame,IplImage *out){
	if(frame->depth == 16){
		for(int i=0;i< frame->height;i+=4){
			unsigned short *inpptr = ((unsigned short *)(frame->imageData)) + i* (frame->widthStep>>1);
			unsigned short *outptr = ((unsigned short *)(out->imageData)) + (i>>2)* (out->widthStep>>1);
			for(int j=0;j<frame->width;j+=4){
				outptr[j>>2] = inpptr[j];
			}
		}
	}
	else{
		if(frame->depth == 8){
			for(int i=0;i< frame->height;i+=4){
				unsigned char *inpptr = ((unsigned char *)(frame->imageData)) + i* frame->widthStep;
				unsigned char *outptr = ((unsigned char *)(out->imageData)) + (i>>2)*out->widthStep;
				for(int j=0;j<frame->width;j+=4){
					outptr[j>>2] = inpptr[j];
				}
			}
		}
	}
}

void ImageProcessor::sendLiveImages(){
	timeval currtime;
	static uint64_t pt=0;
	gettimeofday(&currtime, 0);
	TV_AS_USEC(currtime,ct);
	float freq = 0;
	if (pt!=0){
		freq = 1000000.0/(ct - pt);
	}
	pt = ct;
	IplImage *frame = GetFrame();

	if(m_Debug)
		printf("frame=> %d %5.2f\n",m_faceIndex,freq);

	m_inputImg.Init(frame);

	CvPoint2D32f irisCen = cvPoint2D32f(0,0);
	if(m_testImageSendLevel != 0){
		if(m_secondLevelImage == NULL){
			m_secondLevelImage =  cvCreateImage(cvSize(frame->width>>2,frame->height>>2),frame->depth,1);
		}
		CopyToOneFourth(frame,m_secondLevelImage);
		Image8u coarseImg;
		coarseImg.Init(m_secondLevelImage);
		coarseImg.CopyCenteredROIInto(m_DetectedEyesQueue[0]->getEyeCrop());
	}else{
		m_inputImg.CopyCenteredROIInto(m_DetectedEyesQueue[0]->getEyeCrop());
	}

    m_DetectedEyesQueue[0]->init(0, 0,m_faceIndex,0,0,0,irisCen,m_il0);
    if(m_saveCount>0) {
  		m_saveCount--;
  		IplImage *p = m_DetectedEyesQueue[0]->getEyeCrop();
  		char Buff[1024];
  		static int ctr = 0;
  		sprintf(Buff,"OutputBig%d.pgm",ctr);
  		cvSaveImage(Buff,frame);
  		sprintf(Buff,"OutputSmall%d.pgm",ctr);
  		cvSaveImage(Buff,p);
  		ctr++;
  	}
//    if(m_enableBlackLevel ){
    	m_DetectedEyesQueue[0]->setBlackLevel(m_blackMinThreshold);
//    }
    //if (m_enableLaplacian_focus_Threshold){
    	m_DetectedEyesQueue[0]->setAreaScore(1.0);
    //}
//    if(m_enableHaloThreshold){
    	m_DetectedEyesQueue[0]->setHalo(m_haloMinThreshold);
//    }
    m_DetectedEyesQueue[0]->setTimeStamp(m_timestampBeforeGrabbing);
    m_DetectedEyesQueue[0]->setUpdated(true);
	sendToNwQueue(m_DetectedEyesQueue[0]);
}

void ImageProcessor::extractSmallCropForFocus(int eyeIdx,DetectedEye * eye)
{
	int sw = (m_FocusRoi.width+4)<<1;
	int sh = (m_FocusRoi.height+4)<<1;
	if(1 == m_FocusImageLevel){
		IplImage *ptr = eye->getEyeCrop();
		cvSetData(m_rippedImage,ptr->imageData,ptr->widthStep);
		m_rippedImage->imageData += m_rippedImage->widthStep*((ptr->height-sh)>>1) + ((ptr->width-sw)>>1);

		XTIME_OP("FocusCalc.redux",
					cvPyrDown(m_rippedImage,m_smallCroppedEye)
				);
    }
    else if(2 == m_FocusImageLevel)
    {
    	int left,top;
    	CvPoint pt;
    	pt.x = (m_sframe.GetEyeCenterPointList()->at(eyeIdx).m_nCenterPointX)>>m_FocusImageLevel;
    	pt.y = (m_sframe.GetEyeCenterPointList()->at(eyeIdx).m_nCenterPointY)>>m_FocusImageLevel;

    	m_sframe.GetPyramid()->GetLevel(m_FocusImageLevel)->CopyROIInto(m_smallCroppedEye,pt,left,top);
    }
}
/*
 * write the message onto a buffer
 * return the length of the message
 */
void ImageProcessor::SendHBStatus(long int timestamp){
//	if(m_Debug) printf("Curr %ld Fut %ld \n",timestamp,m_FuturisticTime);

	if((timestamp > m_FuturisticTime)&&m_statusDestAddr)
	{
		struct timeval timeOut;
		timeOut.tv_sec = m_HBTimeOutmilliSec / 1000;
		timeOut.tv_usec = (m_HBTimeOutmilliSec % 1000) * 1000;

		try{
			SocketClient client=m_socketFactory->createSocketClient("GRI.HBDispatcherSecure");
			client.SetTimeouts(timeOut);
			client.ConnectByHostname(*m_statusDestAddr);
			client.Send(m_outMsg,MSG_DONTWAIT);
		}
		catch(Exception& nex)
		{
			nex.PrintException();
		}
		catch(const char *msg)
		{
			std::cout<< msg <<endl;
		}
		catch(...)
		{
			std::cout<< "Unknown exception during SendHB" <<endl;
		}
		m_FuturisticTime = timestamp + m_HBFreqSec;
		if(m_Debug) printf("Fut %ld \n",m_FuturisticTime);
	}
	return;
}

void ImageProcessor::SetImage(IplImage *dst){
	static int count = 0;
	if(m_Imageformat && count++ >= m_ImageMatchTime*1000/100){
		count = 0;
		char fpath[1024];
		sprintf(fpath,m_Imageformat,m_CamStr.c_str(),m_fileIndex,m_fileIlluState);

		int w=-1,h=-1,bits=-1;
		int ret = ReadPGM5WHandBits(fpath,&w,&h,&bits);
		printf("Reading %s %d %d %d %d\n",fpath,w,h,bits,ret);
		if(!m_src && (ret != -1)){
			m_src = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
		}

		if(ret == -1){
			if(m_fileIndex > m_maxFramesReset)
				m_fileIndex = 0;
			if(m_src)
				cvSetZero(m_src);
		}else{
			ReadPGM5(fpath,(unsigned char *)m_src->imageData,&w,&h,m_src->imageSize);
		}

		if(m_src){
			for(int i =0;i<min(dst->height,m_src->height);i++){
				unsigned char *inp,*out;
				inp = ((unsigned char *)m_src->imageData) + i*(m_src->widthStep);
				out = ((unsigned char *)dst->imageData) + (i+(dst->height>>1)-(m_src->height>>1))*(dst->widthStep) + (dst->widthStep>>1);
				memcpy(out,inp,min(dst->width,m_src->width));
			}
		}
		m_il0 = m_fileIlluState;
		m_fileIlluState++;
		m_fileIndex++;
		if(m_fileIlluState >1){
			m_fileIlluState=0;
		}
	}
}

void ImageProcessor::ComputeMotion()
{
    if(m_MotionDetection){
        timeval currtime;
        gettimeofday(&currtime, 0);
        Image8u *img = m_sframe.GetPyramid()->GetLevel(3);
        if(m_MotionDetection->ProcessImage(img->m_Data, &currtime)){
            m_MotionMsg.lock();
            m_MotionMsg.setUpdated(true);
            m_MotionMsg.unlock();
        }
    }

}

void ImageProcessor::setLatestFrame_raw(char *ptr){
	aquisition->setLatestFrame_raw(ptr);
}

void ImageProcessor::clearFrameBuffer(){
	aquisition->clearFrameBuffer();
}

IplImage * ImageProcessor::GetFrame(){
	// printf("ImageProcessor.....GetFrame\n");
	IplImage *frame = NULL;
	if(m_EyelockIrisMode == 2){
		frame = aquisition->getFrame_nowait();
		if (NULL == frame)
			return NULL;
	}else{
		frame = aquisition->getFrame();
	}
	__int64_t ts;
	int il0,frindx;
	aquisition->getIlluminatorState(ts,il0,frindx);
	m_timestampBeforeGrabbing = ts;
	m_il0 = il0;
	m_faceIndex = frindx;
//	printf("IP %d %d\n",frindx,il0);
	for(int i = 0; i < m_ImageHandlers.size(); i++)	{
		if(m_ImageHandlers[i])
		{
			printf("ImageProcessor::GetFrame()\n"); fflush(stdout);
			m_ImageHandlers[i]->Handle(frame);
		}
	}
	return frame;
}

void ImageProcessor::CheckForHaloScoreThreshold(CvPoint3D32f& halo){
	if(m_enableHaloThreshold){
		if(((halo.y < m_haloMinCount)||(halo.y > m_haloMaxCount))){
			if(m_Debug)	printf("Discarded eye with HALO %f on behalf of Halo count %f < %f < %f %d %f\n",halo.z,m_haloMinCount,halo.y,m_haloMaxCount,m_currMaxSpecValue,m_currHaloThreshold);
			halo.z = -1.0;
		}
	}
}
//unsigned char bufImag[640*480];
void ImageProcessor::ChangeShiftandOffset(bool test){
	//if true set to AfterDetect Value
	FrameGrabber *fg = GetFrameGrabber();
	if(test){
		fg->SetPWM(m_ledPWMAddress,m_ledPWMDefaultValueAfterDetect);
		SetHaloRelatedConf(m_DcOffsetAfterDetect,m_ShiftRightAfterDetect,m_haloThresholdAfterDetect,m_maxSpecValueAfterDetect);
		//fg->FlushAllInput();
	}else{
		fg->SetPWM(m_ledPWMAddress,m_ledPWMDefaultValue);
		SetHaloRelatedConf(m_DcOffset,m_ShiftRight,m_haloThreshold,m_maxSpecValue);
	}
}

void ImageProcessor::SetHaloRelatedConf(unsigned short dcoff,unsigned short shift,float halothresh,unsigned short maxspecval){
	FrameGrabber *fg = GetFrameGrabber();
	fg->SetShiftAndOffset(dcoff,shift);
	SetHaloThreshold(halothresh);
	SetMaxSpecValue(maxspecval);
}
void ImageProcessor::GenMsgAfterDetect(BinMessage& msg){
	char Buffer[256]={0};
	int len = sprintf(Buffer,"Halo-DC;%6.2f;%02d;%02d;%02d;",m_haloThresholdAfterDetect,m_DcOffsetAfterDetect,m_ShiftRightAfterDetect,m_maxSpecValueAfterDetect);
	msg.SetData(Buffer,len);
}

void ImageProcessor::GenMsgToNormal(BinMessage& msg){
	char Buffer[256]={0};
	int len = sprintf(Buffer,"Halo-DC;%6.2f;%02d;%02d;%02d;",m_haloThreshold,m_DcOffset,m_ShiftRight,m_maxSpecValue);
	msg.SetData(Buffer,len);
}

//ProjectPtr2() input(x,y) needs to be scaled out to the input image size
cv::Point2i ImageProcessor::ProjectPtr2(float x, float y, cv::Point2f constant, float ConstDiv)
{
	/*
	x = ((y-c)/m);

	Here, m is magnification offset (ConstDiv)
	c is the intersection in Iris plane (constant)
	y is the coordinates in Face plane
	calculated x will be the coordinate of Iris plane
	*/

	cv::Point2i ptr2;

	ptr2.x = (x-constant.x)*ConstDiv;
	ptr2.y = (y-constant.y)*ConstDiv;

	//Condition check for y
	if (ptr2.y > m_Imageheight) {
		ptr2.y = m_Imageheight;
	}

	if (ptr2.y < 0) {
		ptr2.y = 0;
	}

	//Condition check for x
	if (ptr2.x < 0)
		ptr2.x = 0;

	if (ptr2.x > m_Imagewidth)
		ptr2.x = m_Imagewidth;

	if(m_bFaceMapDebug)
		EyelockLog(logger, DEBUG, "2nd Diagonal Co-ord of Rect ptr2.x %d ptr2.y %d\n", ptr2.x, ptr2.y);

	return ptr2;

}

//projectPointsPtr1() input needs to be scaled out to the input image size
cv::Point2i ImageProcessor::projectPointsPtr1(cv::Rect projFace, cv::Point2f constant, float ConstDiv)
{
	/*
	x = ((y-c)/m);

	Here, m is magnification offset (ConstDiv)
	c is the intersection in Iris plane (constant)
	y is the coordinates in Face plane
	calculated x will be the coordinate of Iris plane
	*/

	cv::Point2i ptr1;
	ptr1.x = (projFace.x - constant.x) * ConstDiv;
	ptr1.y = (projFace.y - constant.y) * ConstDiv;

	//Condition check for x
	if (ptr1.x < 0)
		ptr1.x = 0;

	if (ptr1.x > m_Imagewidth)
		ptr1.x = m_Imagewidth;

	////Condition check for y
	if (ptr1.y > m_Imageheight) {
		ptr1.y = m_Imageheight;
	}

	if (ptr1.y < 0) {
		ptr1.y = 0;
	}

	if(m_bFaceMapDebug)
		EyelockLog(logger, DEBUG, "1st Diagonal Co-ord of Rect ptr1.x %d ptr1.y %d\n", ptr1.x, ptr1.y);

	return ptr1;
}

//projectPointsPtr1() input needs to be scaled out to the input image size
cv::Point2i ImageProcessor::projectPoints_IristoFace(cv::Point2i ptrI, cv::Point2f constant, float ConstDiv)
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
	if(m_bFaceMapDebug)
		EyelockLog(logger, DEBUG, "IRIS to Face Projection for left right eye ptr1.x %d ptr1.y %d\n", ptrF.x, ptrF.y);

	return ptrF;
}


unsigned int ImageProcessor::validateLeftRightEyecrops(cv::Rect FaceCoord, cv::Point2i ptrI, int CameraId, unsigned char *faceImagePtr, int m_faceIndex){

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

	cv::Rect ValidEyeRect;
	ValidEyeRect.x = FaceCoord.x;
	ValidEyeRect.y = FaceCoord.y + (float) FaceCoord.height*0.1;
	ValidEyeRect.height = FaceCoord.height/2.0;
	ValidEyeRect.width = FaceCoord.width;

	cv::Point2i ptrF;

	//Project Iris points to face image
	if (CameraId == IRISCAM_AUX_LEFT){
		ptrF = projectPoints_IristoFace(ptrI, constantAuxl, magOffAuxl);
	}else if (CameraId == IRISCAM_AUX_RIGHT){
		ptrF = projectPoints_IristoFace(ptrI, constantAuxR, magOffAuxR);
	}else if (CameraId == IRISCAM_MAIN_LEFT){
		ptrF = projectPoints_IristoFace(ptrI, constantMainl, magOffMainl);
	}else if (CameraId == IRISCAM_MAIN_RIGHT){
		ptrF = projectPoints_IristoFace(ptrI, constantMainR, magOffMainR);
	}

	if (!ValidEyeRect.contains(ptrF)){
		return 3;
	}

	if(bIrisToFaceMapDebug){

		cv::Mat face = cv::Mat(1200, 960, CV_8UC1, faceImagePtr);
		/*
		cv::Mat face;
		cv::resize(OrigImage, face, cv::Size(), (1 / 0.8), (1 / 8.0), cv::INTER_NEAREST);*/
		// char filename[100];
		int x = 0;
		int y = 0;
		if (CameraId == IRISCAM_AUX_LEFT) {
			ptrF = projectPoints_IristoFace(ptrI, constantAuxl, magOffAuxl);
			x = 30;
			y = 30;
		} else if (CameraId == IRISCAM_AUX_RIGHT) {
			ptrF = projectPoints_IristoFace(ptrI, constantAuxR, magOffAuxR);
			x = 100;
			y = 100;
		} else if (CameraId == IRISCAM_MAIN_LEFT) {
			x = 30;
			y = 30;
			ptrF = projectPoints_IristoFace(ptrI, constantMainl, magOffMainl);
		} else if (CameraId == IRISCAM_MAIN_RIGHT) {
			x = 100;
			y = 100;
			ptrF = projectPoints_IristoFace(ptrI, constantMainR, magOffMainR);
		}
		try{
			cv::rectangle(face, rightRect, cv::Scalar(255,0,0),1,0);
			cv::rectangle(face, leftRect, cv::Scalar(255,0,0),1,0);
			// cv::rectangle(face, ValidEyeRect, cv::Scalar(128,127,65),1,0);
			std::ostringstream ssCoInfo;
			std::ostringstream ssCoInfo1;
			std::ostringstream ssColeft;
			std::ostringstream ssCoright;
			ssCoInfo1 << ptrF.x << "," << ptrF.y;
			putText(face,ssCoInfo1.str().c_str(),cv::Point(x,y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(128,128,128),2);
			ssCoInfo << "+"; // ptrF.x << "," << ptrF.y;
			putText(face,ssCoInfo.str().c_str(),cv::Point(ptrF.x,ptrF.y), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(128,128,128),2);
			ssColeft << "leftRect" << leftRect.x << "," << leftRect.y << "," << leftRect.width << "," << leftRect.height;
			ssCoright << "rightRect" << rightRect.x << "," << rightRect.y << "," << rightRect.width << "," << rightRect.height;
			putText(face,ssColeft.str().c_str(),cv::Point(200, 200), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(128,128,128),2);
			putText(face,ssCoright.str().c_str(),cv::Point(300, 300), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(128,128,128),2);
			imshow("IrisToFaceMap", face);
			cvWaitKey(2);
		}catch (cv::Exception& e) {
			cout << e.what() << endl;
		}
		face.release();
	}
	// 1 - Left; 2 - Right; 0-Undefined
	if (leftRect.contains(ptrF)){
		EyelockLog(logger, DEBUG, "Left EYE found	return %d !!!!\n", 1);
		/*
		sprintf(filename,"FaceImages_%d_%d_left.pgm", CameraId, m_faceIndex);
		cv::Mat dst = face.clone();
		imwrite(filename, dst);
		dst.release();*/
		return 1;
	}else if (rightRect.contains(ptrF)){
		EyelockLog(logger, DEBUG, "Right EYE found	return %d !!!!\n", 2);
		/*sprintf(filename,"FaceImages_%d_%d_Right.pgm", CameraId, m_faceIndex);
		cv::Mat dst1 = face.clone();
		imwrite(filename, dst1);
		dst1.release();*/
		return 2;
	}else{
		EyelockLog(logger, DEBUG, "Undefined EYE found	return %d !!!!\n", 0);
		/*sprintf(filename,"FaceImages_%d_%d_Undefined.pgm", CameraId, m_faceIndex);
		cv::Mat dst2 = face.clone();
		imwrite(filename, dst2);
		dst2.release();*/
		return 0;
	}
}


bool ImageProcessor::validateEyecrops_IrisToFaceMapping(cv::Rect projFace, cv::Point2i ptrI, int CameraId){

	cv::Point2i ptrF;

	//Project Iris points to face image
	if (CameraId == IRISCAM_AUX_LEFT){
		ptrF = projectPoints_IristoFace(ptrI, constantAuxl, magOffAuxl);
	}else if (CameraId == IRISCAM_AUX_RIGHT){
		ptrF = projectPoints_IristoFace(ptrI, constantAuxR, magOffAuxR);
	}else if (CameraId == IRISCAM_MAIN_LEFT){
		ptrF = projectPoints_IristoFace(ptrI, constantMainl, magOffMainl);
	}else if (CameraId == IRISCAM_MAIN_RIGHT){
		ptrF = projectPoints_IristoFace(ptrI, constantMainR, magOffMainR);
	}


	// Check where the eye belong????
	if (projFace.contains(ptrF)){
		EyelockLog(logger, DEBUG, "the target rect contain Eyecrop %d !!!!\n", 1);
		return true;
	}else{
		EyelockLog(logger, DEBUG, "This Eyecrop don't belong to the target Rect %d !!!!\n", 0);
		return false;
	}
}


cv::Rect ImageProcessor::CeateRect(cv::Point2i ptr1, cv::Point2i ptr2, bool useOffest, float projOffset)
{
	cv::Rect ret1;
	ret1.x = ptr1.x;
	ret1.y = ptr1.y;
	ret1.width = abs(ptr1.x - ptr2.x);
	ret1.height = abs(ptr1.y - ptr2.y);

	// printf("ret1.width....%d ret1.height %d\n", ret1.width, ret1.height);
	if(m_bFaceMapDebug)
		EyelockLog(logger, DEBUG, "Calculated RECT   ret1.x %d 		ret1.y %d 	ret1.width %d 	ret1.height %d \n",  ret1.x, ret1.y, ret1.width, ret1.height);

	//Corner condition for cvSetImageROI assertion fail issue
	if (ret1.width + ret1.x >= m_Imagewidth)
		ret1.width = m_Imagewidth - ret1.x;
	if (ret1.height + ret1.y >= m_Imageheight)
		ret1.height = m_Imageheight - ret1.y;

	//Offset correction if useOffset is true
	if (useOffest) {
		ret1.height = ret1.height - int(projOffset);
	}

	//Added feature for mapping to force doing nothing where the crop height or width is less the the minIrisDiameter
	if(ret1.width < (m_minIrisDiameter - 5) || ret1.height < (m_minIrisDiameter - 5)){
		ret1.width = 0;
		ret1.height = 0;
	}

	//take the full image if projection fail
	if (ret1.width == 0 || ret1.height == 0) {
		ret1.x = 0;
		ret1.y = 0;
		ret1.width = m_Imagewidth;
		ret1.height = m_Imageheight;
	}
	if(m_bFaceMapDebug)
		EyelockLog(logger, DEBUG, "Verified RECT   ret1.x %d ret1.y %d 	ret1.width %d ret1.height %d \n",	ret1.x, ret1.y, ret1.width, ret1.height);

	return ret1;

}

cv::Rect ImageProcessor::projectRectNew(cv::Rect projFace, int CameraId)
{
	//double scaling = 8.0;	//scaling for Pyramid level 3
	//float scale = 1.0;
	//int targetOffset1;
	cv::Rect IrisProjRect;
	//int targetOffset1 = 3;	//it is used in face tracking as an offset that need/not need to be scaled (further test)
	//double scalingDiv = 1.0/scaling;


	bool useOffest = false;
	float projOffset;

	if (CameraId == IRISCAM_AUX_LEFT || CameraId == IRISCAM_AUX_RIGHT) {
		useOffest = useOffest_a;
		projOffset = projOffset_a;
	} else if (CameraId == IRISCAM_MAIN_LEFT || CameraId == IRISCAM_MAIN_RIGHT) {
		useOffest = useOffest_m;
		projOffset = projOffset_m;
	}
	// printf("projectRect face x = %d  face y = %d face width = %d  face height = %d CameraId %d\n", faceP.x, faceP.y, faceP.width, faceP.height, CameraId);

/*
// we can use this part of the code if we later decided to move from no_move_area to search_eye_area for projection
#if 0
	cv::Rect search_eye_area;
	cv::Rect no_move_area;

	no_move_area.x = rectX/scaling;
	no_move_area.y = rectY/scaling + targetOffset1;
	no_move_area.width = rectW/scaling;
	no_move_area.height = (rectH)/scaling -targetOffset1*2;

	search_eye_area = seacrhEyeArea(no_move_area);

	//faceP.y = (search_eye_area.y*scalingDiv) + targetOffset1;
	//faceP.height = (search_eye_area.height)*scalingDiv -(targetOffset1*2);

	projFace.x = face.x * scaling;		//column
	projFace.y = search_eye_area.y * scaling;	//row
	projFace.width = face.width * scaling;
	projFace.height = search_eye_area.height * scaling;

#else
	faceP.y = (rectY*scalingDiv) + targetOffset1;
	faceP.height = (rectH)*scalingDiv -(targetOffset1*2);

	projFace.x = face.x * scaling;		//column
	projFace.y = rectY + targetOffset1 * scaling;	//row
	projFace.width = face.width * scaling;
	projFace.height = rectH - (targetOffset1 *scaling * 2);
#endif

*/


/*
	//Testing targetoffset (If needed)
	projFace.x = face.x * scaling;		//column
	projFace.y = rectY + targetOffset1 * scaling;	//row
	projFace.width = face.width * scaling;
	projFace.height = rectH - (targetOffset1 *scaling * 2);
	*/

	cv::Point2i ptr1, ptr2;


	if (CameraId == IRISCAM_MAIN_LEFT) {
		ptr1 = projectPointsPtr1(projFace, constantMainl, magOffMainlDiv);
		//ptr2 = ProjectPtr2((face.x * scaling + face.width * scaling), (projFace.y + projFace.height), constantMainl, magOffMainlDiv);
		ptr2 = ProjectPtr2((projFace.x + projFace.width), (projFace.y + projFace.height), constantMainl, magOffMainlDiv);
		IrisProjRect = CeateRect(ptr1, ptr2, useOffest, projOffset);

	} else if (CameraId == IRISCAM_AUX_LEFT) {
		//project two coordinates diagonally a part in face frame into Iris
		ptr1 = projectPointsPtr1(projFace, constantAuxl, magOffAuxlDiv);
		//ptr2 = ProjectPtr2((face.x * scaling + face.width * scaling), (projFace.y + projFace.height), constantAuxl, magOffAuxlDiv);
		ptr2 = ProjectPtr2((projFace.x + projFace.width), (projFace.y + projFace.height), constantAuxl, magOffAuxlDiv);
		IrisProjRect = CeateRect(ptr1, ptr2, useOffest, projOffset);

	} else if (CameraId == IRISCAM_MAIN_RIGHT) {
		//project two coordinates diagonally a part in face frame into Iris
		ptr1 = projectPointsPtr1(projFace, constantMainR, magOffMainRDiv);
		//ptr2 = ProjectPtr2((face.x * scaling + face.width * scaling), (projFace.y + projFace.height), constantMainR, magOffMainRDiv);
		ptr2 = ProjectPtr2((projFace.x + projFace.width), (projFace.y + projFace.height), constantMainR, magOffMainRDiv);
		IrisProjRect = CeateRect(ptr1, ptr2, useOffest, projOffset);

	} else if (CameraId == IRISCAM_AUX_RIGHT) {
		//project two coordinates diagonally a part in face frame into Iris
		ptr1 = projectPointsPtr1(projFace, constantAuxR, magOffAuxRDiv);
		//ptr2 = ProjectPtr2((face.x * scaling + face.width * scaling), (projFace.y + projFace.height), constantAuxR, magOffAuxRDiv);
		ptr2 = ProjectPtr2((projFace.x + projFace.width), (projFace.y + projFace.height), constantAuxR, magOffAuxRDiv);
		IrisProjRect =  CeateRect(ptr1, ptr2, useOffest, projOffset);

	}
	// IrisProjRect.width = projFace.width;
	return (IrisProjRect);

}

FaceMapping ImageProcessor::GetFaceInfoFromQueueFacetoIrisMapping(int CameraId, char IrisFrameNo)
{
	FaceMapping m_FaceMap;
	cv::Rect IrisProj1;
	if (CameraId == IRISCAM_AUX_LEFT || CameraId == IRISCAM_MAIN_LEFT)
		g_pCameraFaceQueue = g_pLeftCameraFaceQueue;
	else
		g_pCameraFaceQueue = g_pRightCameraFaceQueue;

	if (g_pCameraFaceQueue == NULL) {
		EyelockLog(logger, TRACE, "ImageProcessor::GetFaceDataMessage(): g_pRingBufferFaceQueue uninitialized!");
	}

	while(1){
		// Might need to add a time limit
		FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Peek();
		char DiffFrameNo = (char) (FaceInfo.FaceFrameNo - IrisFrameNo);
		// Synchronization
		if (FaceInfo.FaceFrameNo == IrisFrameNo){
			//printf("Mapping: Face and Iris Match  cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n",CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);

			//Follow projPtr bool coming from FaceTrcaker to check and run Mapping
			if(FaceInfo.projPtr){
				//printf("\n \n >>>>>>>>>>>>>>>>> \n \n >>>>>>>>>>>>> active projection \n \n >>>>>>>>>>>\n \n");
				FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Pop();
				if(m_IrisToFaceMapping)
					m_FaceMap.IrisProj = cv::Rect(0,0,m_Imagewidth,m_Imageheight);
				else
					m_FaceMap.IrisProj = projectRectNew(FaceInfo.ScaledFaceCoord, CameraId);

				m_FaceMap.SFaceCoord = FaceInfo.ScaledFaceCoord;
				m_FaceMap.bDoMapping = true;
				return m_FaceMap; // return IrisProj;
			}

		}else if(DiffFrameNo > 0){
			// No Matching face for the Iris; Face is ahead of us.
			//printf("Mapping: Face is ahead cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n", CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);
			IrisProj1.x = 0;
			IrisProj1.y = 0;
			IrisProj1.width = m_Imagewidth;
			IrisProj1.height = m_Imageheight;
			m_FaceMap.IrisProj = IrisProj1;
			m_FaceMap.bDoMapping = false;
			return m_FaceMap; // return IrisProj;
		}else if(DiffFrameNo < 0){
			//printf("Mapping: Iris is ahead waiting for face cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n", CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);
			FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Pop();
			continue;
		}
	} // End of while
}
#if 0
FaceImageQueue ImageProcessor::GetFaceInfoForIristoFaceMapping(int CameraId, unsigned char IrisFrameNo, int m_faceIndex)
{
	// FaceMapping m_FaceMap;

	if (CameraId == IRISCAM_AUX_LEFT || CameraId == IRISCAM_MAIN_LEFT)
		g_pCameraFaceQueue = g_pLeftCameraFaceQueue;
	else
		g_pCameraFaceQueue = g_pRightCameraFaceQueue;

	if (g_pCameraFaceQueue == NULL) {
		EyelockLog(logger, TRACE, "ImageProcessor::GetFaceDataMessage(): g_pRingBufferFaceQueue uninitialized!");
	}


	while(!g_pCameraFaceQueue->Empty()){
		FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Peek();
		char DiffFrameNo = ( char) (FaceInfo.FaceFrameNo - IrisFrameNo);
		// printf("DiffFrameNo %d\n", DiffFrameNo);
		// Synchronization
		if (FaceInfo.FaceFrameNo == IrisFrameNo){
			// printf("Mapping: Face and Iris Match  cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n",CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);
			FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Pop();
			return FaceInfo;
		}else if(DiffFrameNo > 0){
			// No Matching face for the Iris; Face is ahead of us.
			// printf("Mapping: Face is ahead cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n", CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);
			int sizeofqueue = g_pCameraFaceQueue->Size();
			for(int idx = 0; idx < sizeofqueue; idx++){
				FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->at(idx);
				printf("Dif > 0 ....FaceInfo.FaceFrameNo....%d\n", FaceInfo.FaceFrameNo);
				if (FaceInfo.FaceFrameNo == IrisFrameNo)
					return FaceInfo;
			}
			return FaceInfo;
		}else if(DiffFrameNo < 0){
			// printf("Mapping: Iris is ahead waiting for face cam_idd %d FaceFrameNo:%d IrisFrameNo:%d \n", CameraId, FaceInfo.FaceFrameNo, IrisFrameNo);
			FaceImageQueue FaceInfo = (FaceImageQueue) g_pCameraFaceQueue->Pop();
			continue;
		}
	} // End of while
}
#endif

#if 0
FaceImageQueue ImageProcessor::GetFaceInfoForIristoFaceMapping(int CameraId, unsigned char IrisFrameNo, unsigned int m_faceIndex)
{
	FaceImageQueue FaceInfo;


	// Select correct FaceFrameQueue based on CamId of the Image we are processing
	if (CameraId == IRISCAM_AUX_LEFT || CameraId == IRISCAM_MAIN_LEFT)
		g_pCameraFaceQueue = g_pLeftCameraFaceQueue;
	else
		g_pCameraFaceQueue = g_pRightCameraFaceQueue;


	if (g_pCameraFaceQueue == NULL)
		EyelockLog(logger, TRACE, "ImageProcessor::GetFaceDataMessage(): g_pRingBufferFaceQueue uninitialized!");


	// If we have at least one element in the FaceFrameQueue...
	if (!g_pCameraFaceQueue->Empty())
	{
		FaceInfo = g_pCameraFaceQueue->Peek(); // Top of the queue

		// printf("GetFaceInfoForIristoFaceMapping CameraId  %d FaceInfo.FaceFrameNo  %d  IrisFrameNo %d m_faceIndex %d \n", CameraId, FaceInfo.FaceFrameNo, IrisFrameNo, m_faceIndex);

		char DiffFrameNo = ( char) (FaceInfo.FaceFrameNo - IrisFrameNo);

		// We found our match... Pop the FaceFrameInfo, then return
		if (FaceInfo.FaceFrameNo == IrisFrameNo)
		{
			FaceInfo = g_pCameraFaceQueue->Pop();
			return FaceInfo;
		}
		else if(DiffFrameNo > 0) //FaceFrameInfo is ahead of us, start looking from back of queue for a match and remove until we find it...
		{
			// DMO -- Need a scopeLock here to prevent FaceTrackingThread from adding stuff...while we search and delete...
			ScopeLock(g_pCameraFaceQueue->m_Lock);
			while (g_pCameraFaceQueue->TryPopBack(FaceInfo)) // false if queue is empty... and breaks us out of the loop
			{
				if (FaceInfo.FaceFrameNo < IrisFrameNo) // Already past this one, pop it off and get a newer one...
					continue;
				else if (FaceInfo.FaceFrameNo == IrisFrameNo)
					break; // Found it!
			};

			// It's possible that we never found a match... in this case, we return the 'next' faceframeinfo in the queue
			return FaceInfo;
		}
		else //if (DiffFrameNo < 0)
		{ // Top of queue FaceFrameInfo is OLD...  remove them until we find a match
			ScopeLock(g_pCameraFaceQueue->m_Lock);
			while (g_pCameraFaceQueue->TryPop(FaceInfo)) // false if queue is empty...
			{
				if (FaceInfo.FaceFrameNo < IrisFrameNo)
					continue;
				else if (FaceInfo.FaceFrameNo == IrisFrameNo)
					break; // Found it!
			}
			return FaceInfo;
		}
	} // End of while
}
#endif


// This function does its best to match a FaceFrame item with the Camera Image
// It looks through the FaceFrame queues waiting if necessary for a bit to find the correct FaceFrame item
// If it fails to find a match, it gives up and our ProcessImage function throws the camera image away...
// If it finds a match, it cleans it off the FaceFrame queue since no other cam images will use it and the Image is fully processed...
bool ImageProcessor::GetFaceInfoForIristoFaceMapping(int CameraId, unsigned char IrisFrameNo, FaceImageQueue &FaceInfo)
{
	// Select correct FaceFrameQueue based on CamId of the Image we are processing
	if (CameraId == IRISCAM_AUX_LEFT || CameraId == IRISCAM_MAIN_LEFT)
		g_pCameraFaceQueue = g_pLeftCameraFaceQueue;
	else
		g_pCameraFaceQueue = g_pRightCameraFaceQueue;


	if (g_pCameraFaceQueue == NULL)
		EyelockLog(logger, TRACE, "ImageProcessor::GetFaceDataMessage(): g_pRingBufferFaceQueue uninitialized!");

	// Timer
	unsigned long time_newms = ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

	// If we come in here and the facequeue is empty... wait MAX 100 ms for something to show before quitting...
	time_newms += 100;

	// If FaceFrame queue is empty, we wait a bit for something to appear...
	while (g_pCameraFaceQueue->Empty() && (ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time()) < time_newms))
		usleep(100);

	// Now, if we have at least one element in the FaceFrameQueue...
	if (!g_pCameraFaceQueue->Empty())
	{
		FaceInfo = g_pCameraFaceQueue->Peek(); // Top of the queue

		char DiffFrameNo = ( char) (FaceInfo.FaceFrameNo - IrisFrameNo);

		// We found our match... Pop the FaceFrameInfo, then return
		if (FaceInfo.FaceFrameNo == IrisFrameNo)
		{
			//printf("Found Frame returning true\n");
			// It's a match!  Pop the entry and use it...  success!
			FaceInfo = g_pCameraFaceQueue->Pop();
			return true;
		}
		else if ((DiffFrameNo > 0) && (DiffFrameNo < 10))
		{
			//printf("DiffFrame > 0 returning false\n");
			// If the difference is greater than 10 we are way out of sync or we are rolled over...
			// In either case we execute the else case below and don't return here...

			// If not rolling over and not totally out of sync...
			// FaceFrame info is AHEAD of us... we already missed the FaceFrame info we needed, just throw this Image away...
			return false; //  Flat out ignore, we do not have a matching frame...in our queue
		}
		else
		{
			// We check the FaceFrame queue here and start removing items until we find our match (or closest match)
			// We also wait for a bit if the queue gets emptied before we've found what we are looking for...
			// ultimately if we don't find it, we will have emptied the FaceFrame queue and we skip processing this image...
			time_newms = ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());
			time_newms += 100;  // wait 100 ms before giving up

			do
			{
				// If the lowest entry in the queue, is still lower than our image
				// Pop entries off the top (we missed them already)
				//printf("--< [%d-%ld] Looping Waiting = %d, Size = %d\n", CameraId, time_newms,FaceInfo.FaceFrameNo, g_pCameraFaceQueue->Size());

				while (g_pCameraFaceQueue->Size()) // false if queue is empty...
				{
					FaceInfo = g_pCameraFaceQueue->Peek(); // Peek top of the queue

					//printf("--< [%d] TryPop Success FrameNo = %d\n", CameraId, FaceInfo.FaceFrameNo);

					if (FaceInfo.FaceFrameNo < IrisFrameNo)
					{
					//	printf("--< [%d] Popping FrameNo = %d\n", CameraId, FaceInfo.FaceFrameNo);
						g_pCameraFaceQueue->Pop();

						continue;
					}
					else if (FaceInfo.FaceFrameNo > IrisFrameNo) // Our exact match was skipped...
					{
						//printf("--< [%d] Missed Frame!  Looking for close Match!  faceFrame = %d\n", CameraId, FaceInfo.FaceFrameNo);

						if (FaceInfo.FaceFrameNo - IrisFrameNo < 3) // If we are within 2 frames of matching, use it but don't pop it...
						{
							// printf("--< [%d] Close Match Found!\n", CameraId);
							return true;
						}
						else
							return false;
					}
					else // its an exact match...  Pop it and return
					{
					//	printf("--< [%d] Exact Match Found!  %d\n", CameraId, FaceInfo.FaceFrameNo);

						g_pCameraFaceQueue->Pop();
						return true;
					}
				};
			//	printf("--< [%d] TryPop failed, Size = %d\n", CameraId, g_pCameraFaceQueue->Size());

				usleep(10000); // Queue is empty... was 10ms then check again...
			}while (ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time()) < time_newms);

		//	printf("--< [%d-%ld] Looping Timed out\n", CameraId, time_newms);
		//	printf("--< [%d] No Match Found in Entire QUEUE\n", CameraId);
			return false;
		}
	} // End of while
	else
		return false; // FaceFrame queue is empty, no match found after patiently waiting...
}

IplImage* ImageProcessor::OffsetImageCorrection(IplImage *frame, int cam_idd)
{
	char temp[50];
	static bool m_OffsetImageLoadedMainCamera1 = false;
	static bool m_OffsetImageLoadedMainCamera2 = false;
	static bool m_OffsetImageLoadedAuxCamera1 = false;
	static bool m_OffsetImageLoadedAuxCamera2 = false;

	// int cam_id = frame->imageData[2]&0xff;
	sprintf(temp, "cal%02x.pgm", cam_idd);
	// printf("cam_id....%02x filename...%s\n", cam_id, temp);

	if (m_OffsetImageLoadedMainCamera1 == false && (cam_idd == 0x01)) {
		m_OffsetImageMainCamera1 = cvLoadImage("cal01.pgm", CV_LOAD_IMAGE_GRAYSCALE);
		m_OffsetImageLoadedMainCamera1 = true;
	}

	if (m_OffsetImageLoadedMainCamera2 == false && (cam_idd == 0x02)) {
		m_OffsetImageMainCamera2 = cvLoadImage("cal02.pgm", CV_LOAD_IMAGE_GRAYSCALE);
		m_OffsetImageLoadedMainCamera2 = true;
	}

	if (m_OffsetImageLoadedAuxCamera1 == false && (cam_idd == 0x81)) {
		m_OffsetImageAuxCamera1 = cvLoadImage("cal81.pgm", CV_LOAD_IMAGE_GRAYSCALE);
		m_OffsetImageLoadedAuxCamera1 = true;
	}

	if (m_OffsetImageLoadedAuxCamera2 == false && (cam_idd == 0x82)) {
		m_OffsetImageAuxCamera2 = cvLoadImage("cal82.pgm", CV_LOAD_IMAGE_GRAYSCALE);
		m_OffsetImageLoadedAuxCamera2 = true;
	}

	if (m_OffsetImageLoadedMainCamera1 && (cam_idd == 0x01)) {
		cvSub(frame, m_OffsetImageMainCamera1, frame);
	}
	if (m_OffsetImageLoadedMainCamera2 && (cam_idd == 0x02)) {
		cvSub(frame, m_OffsetImageMainCamera2, frame);
	}
	if (m_OffsetImageLoadedAuxCamera1 && (cam_idd == 0x81)) {
		cvSub(frame, m_OffsetImageAuxCamera1, frame);
	}
	if (m_OffsetImageLoadedAuxCamera2 && (cam_idd == 0x82)) {
		cvSub(frame, m_OffsetImageAuxCamera2, frame);
	}
}

void ImageProcessor::DebugSessionLogging(IplImage *frame, int cam_idd)
{
	struct stat st = {0};
	if (stat(m_sessionDir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		time_t timer;
		struct tm* tm1;
		time(&timer);
		tm1 = localtime(&timer);

		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		char time_str[100];
		strftime(time_str, 100, "%Y_%m_%d_%H-%M-%S", tm1);
		char log_time_str[100];
		strftime(log_time_str, 100, "%Y %m %d %H:%M:%S", tm1);

		char filename[200];
		if (frame->imageData != NULL) {
			sprintf(filename, "%s/InputImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, m_faceIndex, cam_idd);
			// cvSaveImage(filename,frame); // terminates the application if path doesn't exists
			cv::Mat mateye = cv::cvarrToMat(frame);
			imwrite(filename, mateye);
			mateye.release();
		}

		char session_match_log[100];
		sprintf(session_match_log, "%s/Info.txt", m_sessionDir.c_str());
		FILE *file = fopen(session_match_log, "a");
		if (file) {
			fprintf(file, "%s %lu:%09lu Saved-InputImage-FrNum%d-CamID%d-%s\n",
					log_time_str, ts.tv_sec, ts.tv_nsec, m_faceIndex, cam_idd,
					filename);
			fclose(file);
		}
	}
}

FaceMapping ImageProcessor::DoFaceMapping(IplImage *frame, int cam_idd, int frame_number)
{
	char filename[150];
	sFaceMap = GetFaceInfoFromQueueFacetoIrisMapping(cam_idd, frame_number);
	if (sFaceMap.bDoMapping) {
		if (m_bFaceMapDebug)
			EyelockLog(logger, DEBUG,
					"IrisProjected Final Co-ord of cam ID %d	Frame no: %d	 x %d 		y %d 	width %d 	height %d \n",
					cam_idd, m_faceIndex, sFaceMap.IrisProj.x,
					sFaceMap.IrisProj.y, sFaceMap.IrisProj.width,
					sFaceMap.IrisProj.height);

		// IF m_FaceIrisMappingBeforEyeDetection is false mapping will validate eyecrop later
		if (m_FaceIrisMappingBeforEyeDetection) {
			try {
				cv::Mat OrigFrame = cv::cvarrToMat(frame);
				cv::Mat image_roi = OrigFrame(sFaceMap.IrisProj);
				cv::Mat m_BlackImage = cv::Mat::zeros(cv::Size(m_Imagewidth, m_Imageheight), CV_8U);
				image_roi.copyTo(m_BlackImage(cv::Rect(0, 0, sFaceMap.IrisProj.width, sFaceMap.IrisProj.height)));
				cvSetZero(frame);
				memcpy((unsigned char *) frame->imageData,(unsigned char *) m_BlackImage.data, (m_BlackImage.rows * m_BlackImage.cols));
				if (m_SaveProjImage) {
					sprintf(filename, "FaceMapImage_%d_%d.pgm", cam_idd, m_faceIndex);
					imwrite(filename, m_BlackImage);
				}
				if (m_showProjection) {
					cv::Mat mateye1 = cv::cvarrToMat(frame);
					imshow("ProjectedOutput", mateye1);
					cvWaitKey(1);
					mateye1.release();
				}
				OrigFrame.release();
				image_roi.release();
				m_BlackImage.release();
			} catch (cv::Exception& e) {
				cout << e.what() << endl;
			}

		} else {
			// Just for saving face mapping images for verification
			if (m_SaveProjImage && !(m_IrisToFaceMapping)) {
				try {
					cv::Mat OrigFrame = cv::cvarrToMat(frame);
					cv::Mat image_roi = OrigFrame(sFaceMap.IrisProj);
					cv::Mat m_BlackImage = cv::Mat::zeros(cv::Size(m_Imagewidth, m_Imageheight), CV_8U);
					image_roi.copyTo(m_BlackImage(cv::Rect(0, 0, sFaceMap.IrisProj.width,sFaceMap.IrisProj.height)));
					//cvSetZero(frame);
					sprintf(filename, "FaceMapImage_%d_%d.pgm", cam_idd, m_faceIndex);
					imwrite(filename, m_BlackImage);
					OrigFrame.release();
					image_roi.release();
					m_BlackImage.release();
				} catch (cv::Exception& e) {
					cout << e.what() << endl;
				}
			}

		}

	} else {
		if(m_FaceIrisMapping && !(m_IrisToFaceMapping)){
			sprintf(filename, "FaceMapImage_FAILED_%d_%d.pgm", cam_idd, m_faceIndex);
			cv::Mat mateye = cv::cvarrToMat(frame);
			imwrite(filename, mateye);
			mateye.release();
		}
	}

	return sFaceMap;
}


bool ImageProcessor::ValidateEyeCropUsingFaceMapping(FaceMapping sFaceMap, int cam_idd, int m_faceIndex, int eyeIdx)
{
	bool projStatus = true; // May need Initialization.
	CEyeCenterPoint& centroid = m_sframe.GetEyeCenterPointList()->at(eyeIdx);
	EyelockLog(logger, DEBUG, "Centroid of Eyes: x:		%d y:	%d",
			centroid.m_nCenterPointX, centroid.m_nCenterPointY);

	CvPoint ptCentroid = cvPoint(centroid.m_nCenterPointX,
			centroid.m_nCenterPointY);

	if (m_binType == 1) {
		ptCentroid.y = ptCentroid.y >> 1;
	}
	if (m_binType == 2) {
		ptCentroid.x = ptCentroid.x >> 1;
	}

	EyelockLog(logger, DEBUG, "Centroid normalized of Eyes: x:		%d y:	%d",
			ptCentroid.x, ptCentroid.y);

	if (sFaceMap.bDoMapping) {
		if (sFaceMap.IrisProj.contains(ptCentroid)) {
			projStatus = true;
			if (m_bFaceMapDebug)
				EyelockLog(logger, DEBUG,
						"Mapping is true--> Eyecrop Falls in the Projected RECT %d_%d Proj %d\n",
						cam_idd, m_faceIndex, projStatus);
		} else {
			projStatus = false;
			if (m_bFaceMapDebug)
				EyelockLog(logger, DEBUG,
						"Mapping is true--> Eyecrop DOESNOTTTT Fall in the Projected RECT %d_%d Proj %d\n",
						cam_idd, m_faceIndex, projStatus);
		}
	}
	//Condition of making it strict
	if ((!sFaceMap.bDoMapping) && m_FaceIrisMappingStrict) {
		projStatus = false;
		EyelockLog(logger, DEBUG,
				"Mapping is FAILED-->Eyecrop DOESNOTTTT Fall in the Projected RECT %d_%d Proj %d\n",
				cam_idd, m_faceIndex, projStatus);
	}
	return projStatus;
}


bool ImageProcessor::ValidateEyeCropUsingIrisToFaceMapping(FaceMapping sFaceMap, int cam_idd, int m_faceIndex, int eyeIdx)
{
	bool projStatus = true; // May need Initialization.
	CEyeCenterPoint& centroid = m_sframe.GetEyeCenterPointList()->at(eyeIdx);
	EyelockLog(logger, DEBUG, "Centroid of Eyes: x:		%d y:	%d",
			centroid.m_nCenterPointX, centroid.m_nCenterPointY);

	cv::Point2i ptCentroid = cv::Point2i(centroid.m_nCenterPointX,
			centroid.m_nCenterPointY);

	if (m_binType == 1) {
		ptCentroid.y = ptCentroid.y >> 1;
	}
	if (m_binType == 2) {
		ptCentroid.x = ptCentroid.x >> 1;
	}

	EyelockLog(logger, DEBUG, "Centroid normalized of Eyes: x:		%d y:	%d",
			ptCentroid.x, ptCentroid.y);

	projStatus = validateEyecrops_IrisToFaceMapping(sFaceMap.SFaceCoord, ptCentroid, cam_idd);

	return projStatus;
}

// Only eyecrops are saved in png, jpg, bmp and pgm formats - full frames are saved only in bmp and pgm formats
void ImageProcessor::SaveEyeCrops(IplImage *eyeCrop, int cam_idd, int m_faceIndex, unsigned int eyelabel)
{
	std::string labelEye;
	if (eyelabel == 1){
		labelEye = "LeftEye";
	}else if (eyelabel == 2){
		labelEye = "RightEye";
	}else if(eyelabel == 0){
		labelEye = "UndefinedEye";
	}else{
		labelEye = "NotLabelledbyMapping";
	}

	cv::Mat mateye = cv::cvarrToMat(eyeCrop);
	char filename[150];
#ifdef DEBUG_SESSION
	struct stat st = { 0 };
	if (stat(m_sessionDir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		if(strcmp(eyeCropFileNamePattern, "png") == 0){
			vector<int> compression_params;
			compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
			compression_params.push_back(9);
			 try {
				 sprintf(filename, "%s/EyeCrop_PG_CamId_%d_%d_%s.%s", m_sessionDir.c_str(), cam_idd, m_faceIndex, labelEye.c_str(), eyeCropFileNamePattern);
				 imwrite(filename, mateye, compression_params);
				 mateye.release();
			 }
			 catch (runtime_error& ex) {
				 fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
			 }
		}else if(strcmp(eyeCropFileNamePattern, "jpg") == 0){
			vector<int> compression_params;
			compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
			compression_params.push_back(95);
			try {
				sprintf(filename, "%s/EyeCrop_PG_CamId_%d_%d_%s.%s", m_sessionDir.c_str(), cam_idd, m_faceIndex, labelEye.c_str(), eyeCropFileNamePattern);
				imwrite(filename, mateye, compression_params);
				mateye.release();
			}
			catch (runtime_error& ex) {
				fprintf(stderr, "Exception converting image to JPG format: %s\n", ex.what());
			}

		}else{
			sprintf(filename, "%s/EyeCrop_PG_CamId_%d_%d_%s.%s", m_sessionDir.c_str(), cam_idd, m_faceIndex, labelEye.c_str(), eyeCropFileNamePattern);
			imwrite(filename, mateye);
			mateye.release();
		}

	} else {
		sprintf(filename, "EyeCrop_PG_CamId_%d_%d_%s.%s", cam_idd, m_faceIndex, labelEye.c_str(), eyeCropFileNamePattern);
		imwrite(filename, mateye);
		mateye.release();
	}
#else
	sprintf(filename,"EyeCrop_PG_CamId_%d_%d_%s.pgm", cam_idd, m_faceIndex, labelEye.c_str()); // No debug sessions then default format to save images is pgm
	imwrite(filename, mateye);
	mateye.release();
#endif

}

bool ImageProcessor::ProcessImage(IplImage *frame,bool matchmode)
{
	 if(m_EyelockIrisMode == 2){
		 ProcessImageAcquisitionMode(frame, matchmode);
	 }else{
		 // printf("Inside Match Mode\n");
		 ProcessImageMatchMode(frame, matchmode);
	 }
}

bool ImageProcessor::ProcessImageMatchMode(IplImage *frame,bool matchmode)
{
	char filename[150];
	int cam_idd = 0;
	unsigned char frame_number = 0;

	if(frame->imageData != NULL)
	{
		cam_idd = frame->imageData[2]&0xff;
		frame_number = frame->imageData[3]&0xff;
	}

#ifdef DEBUG_SESSION
	if(m_DebugTesting){
		DebugSessionLogging(frame, cam_idd);
	}
#endif

	if(m_IrisCameraPreview && (frame->imageData != NULL)){
		cv::Mat mateye = cv::cvarrToMat(frame);
		std::ostringstream ssCoInfo;
		ssCoInfo << "CAM " <<  cam_idd  <<  (cam_idd & 0x80 ?  "AUX":"MAIN");
		putText(mateye,ssCoInfo.str().c_str(),cv::Point(10,60), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(255,255,255),2);
		imshow("IrisCamera", mateye);
		cvWaitKey(1);
	}

	if(m_SaveFullFrame){
		sprintf(filename,"InputImage_%d_%d.pgm", cam_idd, m_faceIndex);
		cv::Mat mateye = cv::cvarrToMat(frame);
		imwrite(filename, mateye);
		mateye.release();
	}

	// Loading Offset file - PGM
	if(m_EnableOffsetCorrection){
		m_ProcessImageFrame = OffsetImageCorrection(frame, cam_idd);
		cvCopy(m_ProcessImageFrame,frame);
	}

	// If we fail to find a match after looking for one and waiting a bit, we don't process this image at all...
	FaceImageQueue FaceInfo;
	bool bSkipProcessingImage = false;
	if(m_IrisToFaceMapping)
	{
		if (!GetFaceInfoForIristoFaceMapping(cam_idd, frame_number, FaceInfo))
			bSkipProcessingImage = true; // Skip all other processing... This simulates the No Eyes Detected Case
	}

	bool bSentSomething = false;
	int maxEyes = 0;
	int left, top;
	std::map<int,EyeInfo> eyeMap;
	bool detect;
	int NoOfHaarEyes = 0;

	if (!bSkipProcessingImage)
	{
		// printf("Inside ProcessImage\n");
		XTIME_OP("SetImage",
			SetImage(frame)
		);

		m_inputImg.Init(frame);
		XTIME_OP("Pyramid",
		m_sframe.SetImage(&m_inputImg);
		);

		ComputeMotion();


		XTIME_OP("Detect",
			detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel)
		);

		NoOfHaarEyes = m_sframe.GetNumberOfHaarEyes();

	   //  printf("After Detect Eyes\n");
		if(m_shouldLog){
			logResults(m_faceIndex);
		}

		if(m_saveCount > 0){
			m_saveCount--;
			m_sframe.saveImage("result", m_faceIndex);
		}

		maxEyes = m_sframe.GetEyeCenterPointList()->size();

		if(m_Debug)
			printf("NumEyes %d \n", maxEyes);

		if(maxEyes > m_maxEyes){
			int clear = ClearEyes(m_faceIndex);
			return bSentSomething;
		}

		printf("NoofHaarEyes = %d, maxEyes = %d\n", NoOfHaarEyes, maxEyes);

		for(int eyeIdx=0;eyeIdx<maxEyes;eyeIdx++){
			CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);
			DetectedEye *eye=getNextAvailableEyeBuffer();

			CEyeCenterPoint& centroid = m_sframe.GetEyeCenterPointList()->at(eyeIdx);
			cv::Point2i ptrI= cv::Point2i(centroid.m_nCenterPointX,
					centroid.m_nCenterPointY);

			if (m_IrisToFaceMapping){
				m_eyeLabel = validateLeftRightEyecrops(FaceInfo.ScaledFaceCoord, ptrI, cam_idd, FaceInfo.faceImagePtr, m_faceIndex);
				// printf("eye Label Information Cam_idd %d frameidx %d eyeLabel %d projStatus %d\n", cam_idd, m_faceIndex, eyeLabel, m_projStatus);
				EyelockLog(logger, DEBUG, "No Eyes detected in Input Frame from Camera %d_%d label %d\n", cam_idd, m_faceIndex, m_eyeLabel);
			}

			/*if(m_eyeLabel == 3){
				printf("Not inside valid eye rect and hence don't process\n");
				// return false;
			}*/

			m_sframe.GetCroppedEye(eyeIdx, eye->getEyeCrop(), left, top);

			std::pair<float, float> score;
			XTIME_OP("Check_image",
				score = m_nanoSpoofDetector->check_image(eye->getEyeCrop()->imageData,eye->getEyeCrop()->width, eye->getEyeCrop()->height,eye->getEyeCrop()->widthStep)
			);
			CvPoint3D32f halo ;
			XTIME_OP("Halo",
					//halo =  m_nanoSpoofDetector->ComputeHaloScore(eye->getEyeCrop(),m_currMaxSpecValue)
				halo =  m_nanoSpoofDetector->ComputeTopPointsBasedHaloScore(eye->getEyeCrop(),m_currMaxSpecValue,m_haloCountByBottomPixels,m_haloTopPixelsPercentage,m_haloBottomPixelsIntensityThresh,m_haloThreshold,m_enableHaloThreshold,m_MHaloNegationThresh);
			);
			CheckForHaloScoreThreshold(halo);
			CvPoint3D32f output;
			output.x = -1.0f;

			// Save EyeCrops
			if(m_SaveEyeCrops)
			{
				SaveEyeCrops(eye->getEyeCrop(), cam_idd, m_faceIndex, m_eyeLabel);
			}

			if (m_enableLaplacian_focus_Threshold||m_enableLaplacian_focus_ThresholdEnroll){
				IplImage *im = eye->getEyeCrop();
				CvRect rect = {160, 120, 320, 240};
				IplImage *cec = NULL;
				LaplacianBasedFocusDetector *lapdetector=NULL;
				if(m_enableLaplacian_focus_Threshold && m_matchingmode){
					cec = m_centreEyecrop;
					lapdetector = m_lapdetector;
					//printf("Cmmg in Matching Lap \n");
				}else if(m_enableLaplacian_focus_ThresholdEnroll && (!m_matchingmode)){
					cec = m_centreEyecropEnroll;
					lapdetector = m_lapdetectorEnroll;
					//printf("Cmmg in Enroll Lap \n");
				}else{
					//printf("Cmmg in Junk Lap \n");
				}
				if(cec && lapdetector){
					rect.x = MAX(0,(m_rotatedEyecrop->width-cec->width)/2);
					rect.y = MAX(0,(m_rotatedEyecrop->height-cec->height)/2);
					rect.width = cec->width;
					rect.height = cec->height;
					if(m_shouldRotate){
						cvTranspose(im, m_rotatedEyecrop);
						XTIME_OP("cvFlip",cvFlip(m_rotatedEyecrop, m_rotatedEyecrop, 1););
						cvSetImageROI(m_rotatedEyecrop,rect);
						cvCopy(m_rotatedEyecrop,cec);
						cvResetImageROI(m_rotatedEyecrop);
					}else{
						cvSetImageROI(im,rect);
						cvCopy(im,cec);
						cvResetImageROI(im);
					}
					XTIME_OP("Laplacian : ", output = lapdetector->ComputeRegressionFocus(cec, m_maxSpecValue););
					printf("%d -> [LS:HS] =[%f , %f] \n",m_faceIndex,output.x,halo.z);
				}
			}

			std::pair<int, int> blt = m_nanoSpoofDetector->GetIrisPupilIntensities();
			irisCentroid = m_nanoSpoofDetector->GetSpecularityCentroid();

			if(m_shouldRotate){
				//If rotated then swap centroid values.
				float temp =  irisCentroid.x;
				irisCentroid.x = irisCentroid.y;
				irisCentroid.y = temp;
			}
			eye->init(cam_idd, eyeIdx,m_faceIndex,maxEyes,left, top,irisCentroid,m_il0,1,output.x,score.second,blt.first,halo.z);
			eye->setTimeStamp(m_timestampBeforeGrabbing);
			eye->setUpdated(true);

			if(m_FocusBasedRejectionType||m_spoofEnable)
			{
				extractSmallCropForFocus(eyeIdx,eye);
				XTIME_OP("FocusCalc.ComputeFeatureVector",
					m_IrisSelectSvr->ComputeFeatureVector(m_smallCroppedEye,m_FocusRoi,m_IrisSelectFeatVect)
				);
				//Set the score value
				eye->setScore(m_IrisSelectFeatVect[1]);

				EyeInfo eyeInfo;
				eyeInfo.x=left;
				eyeInfo.y=top;
				eyeInfo.score=m_IrisSelectFeatVect[1];
				eyeInfo.handle=eye;
				eyeMap[eyeIdx]=eyeInfo;
				if(m_shouldLog){
					logFocusScore(eyeInfo.score);
				}
				//if(m_Debug)printf("Score[%d]=%d ",eyeIdx,eyeInfo.score);
			}
			else
			{
				if(m_EnableExtHalo){
					if(halo.z > 0 && halo.z < m_haloThreshold){
						sendToNwQueue(eye);
						bSentSomething=true;
					}
				}
			}
		}

		if(m_FocusBasedRejectionType){
			std::map<std::pair<int,int> ,void*> result = m_EyeTracker->Track(m_faceIndex, eyeMap);
			if(m_shouldLog){
				logFocusResults(result);
			}
			if(!result.empty()){
				m_DetectedMsg.lock();
				m_DetectedMsg.setUpdated(true);
				m_DetectedMsg.unlock();
			}
			std::map<std::pair<int,int> ,void*>::iterator rit = result.begin();
			for(;rit != result.end();rit++){
				DetectedEye *eye = (DetectedEye*)(((rit->second)));
				if(eye->isSame(rit->first.first, rit->first.second)){
					sendToNwQueue(eye);
					bSentSomething = true;
				}
			}
		}
	}// !bSkipProcessing

    return bSentSomething;
}

#ifdef IRIS_CAPTURE

#include <cmath>
float GetISOSharpness(char *data, int width, int height)
{
	cv::Mat imgLaplacian;

	cv::Mat kernel = (cv::Mat_<float>(9, 9) <<
				  0, 1, 1, 2, 2, 2, 1, 1, 0,
				  1, 2, 4, 5, 5, 5, 4, 2, 1,
				  1, 4, 5, 3, 0, 3, 5, 4, 1,
				  2, 5, 3, -12, -24, -12, 3, 5, 2,
				  2, 5, 0, -24, -40, -24, 0, 5, 2,
				  2, 5, 3, -12, -24, -12, 3, 5, 2,
				  1, 4, 5, 3, 0, 3, 5, 4, 1,
				  1, 2, 4, 5, 5, 5, 4, 2, 1,
				  0, 1, 1, 2, 2, 2, 1, 1, 0);

	int window_size = 9;

	IplImage *in = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	memcpy(in->imageData, data, width*height);

	cv::Mat img = cv::cvarrToMat(in, true);

	static int nIndex = 0;
	char filename[1024];

#if 0
	nIndex++;
	sprintf(filename, "Laplacian_%d_%d.pgm", nIndex, nIndex);
	imwrite(filename, img);
#endif

	cvReleaseImage(&in);

	// Can't let OpenCV do our convolution...  for some reason
	// the spec calls for processing only every 4th row and column
//	cv::filter2D(img, imgLaplacian, CV_32F, kernel);

	cv::Mat frame;
	cv::Rect roi;

	// Bump it up to a flow so that we don't overflow our 8bit inputs...
	imgLaplacian = cv::Mat::zeros(img.size(), CV_32F);

	// It's not explained, but ISO only looks at every 4th row and column...
	for (int y = 0; y < img.rows - window_size; y+=4)
	{
		for (int x = 0; x < img.cols - window_size; x+=4)
		{
			roi = cv::Rect(x, y, window_size, window_size);
			frame = img(roi);
			frame.convertTo(frame, CV_32F);
			frame = frame.mul(kernel);
			float v = sum(frame)[0];
			imgLaplacian.at<float>(y/4, x/4) = v;

		}
	}

	// Back to 8 bits.
	imgLaplacian.convertTo(imgLaplacian, CV_8U);
	frame.release();

	//Write it out...
#if 0
	sprintf(filename, "Laplacian_%d.pgm", nIndex);
	imwrite(filename, imgLaplacian);
#endif


	double fSquaredSum = 0.0;

	// We have our LoG Mat...  now sum it up...
	for (int y = 0; y < (img.rows - window_size)/4; y++)
	{
		for (int x = 0; x < (img.cols - window_size)/4; x++)
			fSquaredSum += ((double)imgLaplacian.at<uchar>(y, x) * (double)imgLaplacian.at<uchar>(y, x));
	}


	double fPower = 0.0;
	fPower = fSquaredSum / (((img.rows - window_size) / 4) * ((img.cols - window_size) / 4));

	// Compute the sharpness...
	double SHARP_CONST = pow(1800000, 2);
	imgLaplacian.release();
	kernel.release();
	img.release();

	return (float)(100 * (pow(fPower, 2) / (pow(fPower, 2) + SHARP_CONST)));
}

float getVariance(short *data, int nsize)
{
	float mean = 0.0;
	float stddev = 0.0;

	for (int i = 0; i < nsize; i++)
	{
		mean += data[i];
	}

	mean /= nsize;
	std::vector<double> var(nsize);

	for (int i = 0; i < nsize; i++)
	{
		var[i] = (mean - data[i]) * (mean - data[i]);
	}
	for (int i = 0; i < nsize; i++)
	{
		stddev += var[i];
	}

	return stddev / nsize;
}


float GetSharpness(char *data, int width, int height)
{
	//Assumes 8bit grayscale
	IplImage *in = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	IplImage *out = cvCreateImage(cvSize(width, height), IPL_DEPTH_16S, 1);

	memcpy(in->imageData, data, width*height);

	cvLaplace(in, out, 1);

	float maxLap = -32767;

#ifdef SIMPLE_PEAK
	short *imgData = (short *)out->imageData;
	for (int i = 0; i < (out->imageSize / 2); i++)
		if (imgData[i] > maxLap)
			maxLap = imgData[i];
#else
	maxLap = getVariance((short *)out->imageData, (int)(out->imageSize / 2));
#endif

	cvReleaseImage(&in);
	cvReleaseImage(&out);
	return maxLap;
}





int m_width = 640;//conf.getValue("GRI.cropWidth", 640);
int m_height = 480;//conf.getValue("GRI.cropHeight", 480);
/*
IplImage *m_scaleDest;
IplImage *m_scaleSrcHeader; */

//int m_expectedIrisWidth = 254; //DMOTEST//160;//conf.getValue("GRI.ExpectedIrisWidth", 200);
//int m_actualIrisWidth = 127;//conf.getValue("GRI.ActualIrisWidth", 130);

IplImage *ImageProcessor::ResizeFrame(int width, int height, unsigned char *frame)
{
	float ratio = (m_expectedIrisWidth * 1.0) / m_actualIrisWidth;
	// m_scaleDest = cvCreateImage(cvSize(m_width, m_height), IPL_DEPTH_8U, 1);

	cvSetZero(m_scaleDest);

	if (m_scaleSrcHeader != 0)
	{
		if (m_scaleSrcHeader->width != width || m_scaleSrcHeader->height != height)
		{
			delete m_scaleSrcHeader;
			m_scaleSrcHeader = 0;
		}
	}

	if (m_scaleSrcHeader == 0)
		m_scaleSrcHeader = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);


	CvRect scaleROI;
	scaleROI.width = width * ratio;
	scaleROI.height = height * ratio;
	scaleROI.x = (640 - scaleROI.width) >> 1;
	scaleROI.y = (480 - scaleROI.height) >> 1;
	cvSetImageROI(m_scaleDest, scaleROI);

	scaleROI.width = 640 / ratio;
	scaleROI.height = 480 / ratio;
	scaleROI.x = (width - scaleROI.width) >> 1;
	scaleROI.y = (height - scaleROI.height) >> 1;
	cvSetImageROI(m_scaleSrcHeader, scaleROI);

	cvSetData(m_scaleSrcHeader, frame, m_scaleSrcHeader->width);
	XTIME_OP("cvResize", cvResize(m_scaleSrcHeader, m_scaleDest, CV_INTER_CUBIC));

	if(m_scaleSrcHeader)
		cvReleaseImageHeader(&m_scaleSrcHeader);

	return m_scaleDest;
}

bool ImageProcessor::ProcessImageAcquisitionMode(IplImage *frame,bool matchmode)
{
	char filename[150];
	static int p;
	int cam_idd = 0;
	unsigned char frame_number = 0;

	bool bSentSomething = false;

	if (NULL != frame)
	{
		if(frame->imageData != NULL)
		{
			cam_idd = frame->imageData[2]&0xff;
			frame_number = frame->imageData[3]&0xff;
		}

	#ifdef DEBUG_SESSION
		if(m_DebugTesting){
			DebugSessionLogging(frame, cam_idd);
		}
	#endif

		if(m_IrisCameraPreview && (frame->imageData != NULL)){
			cv::Mat mateye = cv::cvarrToMat(frame);
			std::ostringstream ssCoInfo;
			ssCoInfo << "CAM " <<  cam_idd  <<  (cam_idd & 0x80 ?  "AUX":"MAIN");
			putText(mateye,ssCoInfo.str().c_str(),cv::Point(10,60), cv::FONT_HERSHEY_SIMPLEX, 1.5,cv::Scalar(255,255,255),2);
			imshow("IrisCamera", mateye);
			cvWaitKey(1);
		}

		if(m_SaveFullFrame){
			sprintf(filename,"InputImage_%d_%d.pgm", cam_idd, m_faceIndex);
			cv::Mat mateye = cv::cvarrToMat(frame);
			imwrite(filename, mateye);
			mateye.release();
		}

		// Loading Offset file - PGM
		if(m_EnableOffsetCorrection){
			m_ProcessImageFrame = OffsetImageCorrection(frame, cam_idd);
			cvCopy(m_ProcessImageFrame,frame);
		}
		// DMO
		// Check the face queues for a matching FaceFrame at the very beginning...
		// If we fail to find a match after looking for one and waiting a bit, we don't process this image at all...
		FaceImageQueue FaceInfo;
		bool bSkipProcessingImage = false;

		if(m_IrisToFaceMapping)
		{
			if (!GetFaceInfoForIristoFaceMapping(cam_idd, frame_number, FaceInfo))
				bSkipProcessingImage = true; // Skip all other processing... This simulates the No Eyes Detected Case
		}


		bool detect;
		int NoOfHaarEyes = 0;
		int left, top;
		std::map<int,EyeInfo> eyeMap;
		int maxEyes = 0;

		if (!bSkipProcessingImage)
		{

			XTIME_OP("SetImage",
				SetImage(frame)
			);

			m_inputImg.Init(frame);
			XTIME_OP("Pyramid",
			m_sframe.SetImage(&m_inputImg);
			);
			ComputeMotion();


			XTIME_OP("Detect",
				detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel)
			);

			NoOfHaarEyes = m_sframe.GetNumberOfHaarEyes();

			if(m_shouldLog){
				logResults(m_faceIndex);
			}

			if(m_saveCount > 0){
				m_saveCount--;
				m_sframe.saveImage("result", m_faceIndex);
			}

			maxEyes = m_sframe.GetEyeCenterPointList()->size();
			if(m_Debug)
				printf("NumEyes %d \n", maxEyes);

			if(maxEyes > m_maxEyes){
				//printf("RETURNING!  NumEyes %d \n", maxEyes);
				int clear = ClearEyes(m_faceIndex);
				return bSentSomething;
			}

		}// !bSkipProcessing

		printf("NoofHaarEyes = %d, maxEyes = %d\n", NoOfHaarEyes, maxEyes);

		if ((maxEyes > 0) && !bSkipProcessingImage)
		{
			for(int eyeIdx=0;eyeIdx<maxEyes;eyeIdx++)
			{
				CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);
				DetectedEye *eye=getNextAvailableEyeBuffer();

				CEyeCenterPoint& centroid = m_sframe.GetEyeCenterPointList()->at(eyeIdx);
				cv::Point2i ptrI= cv::Point2i(centroid.m_nCenterPointX,
						centroid.m_nCenterPointY);

				if (m_IrisToFaceMapping){
					m_eyeLabel = validateLeftRightEyecrops(FaceInfo.ScaledFaceCoord, ptrI, cam_idd, FaceInfo.faceImagePtr, m_faceIndex);
				//	if(m_eyeLabel == 0)
					//	return false;
					// printf("eye Label Information Cam_idd %d frameidx %d eyeLabel %d projStatus %d\n", cam_idd, m_faceIndex, eyeLabel, m_projStatus);
					EyelockLog(logger, DEBUG, "No Eyes detected in Input Frame from Camera %d_%d label %d\n", cam_idd, m_faceIndex, m_eyeLabel);
				}

				/*
				if (m_eyeLabel == 3){
					printf("eyeLabel is 3 hence returning from ImageProcessor\n");
					// return false;
				}*/

				m_sframe.GetCroppedEye(eyeIdx, eye->getEyeCrop(), left, top);

				std::pair<float, float> score;
				XTIME_OP("Check_image",
					score = m_nanoSpoofDetector->check_image(eye->getEyeCrop()->imageData,eye->getEyeCrop()->width, eye->getEyeCrop()->height,eye->getEyeCrop()->widthStep)
				);
				CvPoint3D32f halo ;
			//	XTIME_OP("Halo",
						//halo =  m_nanoSpoofDetector->ComputeHaloScore(eye->getEyeCrop(),m_currMaxSpecValue)
		//			halo =  m_nanoSpoofDetector->ComputeTopPointsBasedHaloScore(eye->getEyeCrop(),m_currMaxSpecValue,m_haloCountByBottomPixels,m_haloTopPixelsPercentage,m_haloBottomPixelsIntensityThresh,m_haloThreshold,m_enableHaloThreshold,m_MHaloNegationThresh);
		//		);
			//	CheckForHaloScoreThreshold(halo);
				CvPoint3D32f output;
				output.x = -1.0f;

					if (m_enableLaplacian_focus_Threshold || m_enableLaplacian_focus_ThresholdEnroll) {
						// printf("Inside focus thershold\n");
					IplImage *im = eye->getEyeCrop();
					CvRect rect = {160, 120, 320, 240};
					IplImage *cec = NULL;
					LaplacianBasedFocusDetector *lapdetector=NULL;
					if(m_enableLaplacian_focus_Threshold && m_matchingmode)
					{
						cec = m_centreEyecrop;
						lapdetector = m_lapdetector;
					}
					else if(m_enableLaplacian_focus_ThresholdEnroll && (!m_matchingmode))
					{
						cec = m_centreEyecropEnroll;
						lapdetector = m_lapdetectorEnroll;
					}
					else
					{
						//printf("Cmmg in Junk Lap \n");
					}

					if (cec && lapdetector)
					{
						rect.x = MAX(0,(m_rotatedEyecrop->width-cec->width)/2);
						rect.y = MAX(0,(m_rotatedEyecrop->height-cec->height)/2);
						rect.width = cec->width;
						rect.height = cec->height;
						if(m_shouldRotate)
						{
							cvTranspose(im, m_rotatedEyecrop);
							XTIME_OP("cvFlip",cvFlip(m_rotatedEyecrop, m_rotatedEyecrop, 1););
							cvSetImageROI(m_rotatedEyecrop,rect);
							cvCopy(m_rotatedEyecrop,cec);
							cvResetImageROI(m_rotatedEyecrop);
						}
						else
						{
							cvSetImageROI(im,rect);
							cvCopy(im,cec);
							cvResetImageROI(im);
						}

	#ifndef NEW_LAPLACIAN_METHOD
						XTIME_OP("Laplacian : ", output = lapdetector->ComputeRegressionFocus(cec, m_maxSpecValue) ;);
						printf("%d -> [LS:HS] =[%f , %f] \n",m_faceIndex,output.x,halo.z);
	#endif
					}


					if(m_SaveEyeCrops)
					{
						SaveEyeCrops(eye->getEyeCrop(), cam_idd, m_faceIndex, m_eyeLabel);
					}

					if (m_bIrisCapture) // For Iris Capture mode we do sorting...
					{

	#ifndef NEW_LAPLACIAN_METHOD
						if (output.x > m_laplacian_focusThreshold)
	#endif
						{
							// printf("*******putting through sorting-good lap score\n");

							unsigned long time_elaps = 0, time_newms = 0, currTimems = 0;

							// Critical... resize the image
							tmpImage = ResizeFrame(640, 480, im->imageData);

							if(m_EnableISOSharpness){
								cec = m_centreEyecrop;
								rect.x = MAX(0,(m_rotatedEyecrop->width-cec->width)/2);
								rect.y = MAX(0,(m_rotatedEyecrop->height-cec->height)/2);
								rect.width = cec->width;
								rect.height = cec->height;
								cvSetImageROI(tmpImage,rect);
								cvCopy(tmpImage,cec);
								cvResetImageROI(tmpImage);

								float sharpness = 100000 * GetISOSharpness(cec->imageData, cec->width, cec->height);
								output.x = sharpness;
								// printf("output.x  output.xoutput.x %f\n", output.x);

							 if(output.x  > m_ISOSharpnessThreshold){

								if (shouldIBeginSorting == false) // Begining a sorting
								{
#if 0
									if (m_DHSScreens)
									{
										Screen = cv::imread("/home/root/screens/Slide2.BMP", cv::IMREAD_COLOR);
										imshow("EXT", Screen);
										cvWaitKey(1);
									}
#else
									if (m_DHSScreens)
									{
										cvShowImage("EXT", IplImageScreen2);
										cvWaitKey(1);
									}

#endif

									time_newms= ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

									//DMOREMOVE
								//	printf("\n#########   BeginSorting! TIME:  %ld ms, TIMEOUT: %d ms   #######\n", time_newms, m_IrisCaptureEarlyTimeout);

									eyeSortingWrapObj->BeginSorting((long) time_newms);
									terminate = false; //We begin sorting. So terminate is false
	#ifdef EYE_SIDE
									terminate = eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) tmpImage->imageData,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, time_newms, 0.0, m_eyeLabel, output.x/*DMOTODarEyeSide[eyeIdx]*/); //Need to pass all the halo, laplacian, and all other info here from computation
	#else
									terminate = eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) tmpImage->imageData,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, time_newms, 0.0, output.x/*DMOTODarEyeSide[eyeIdx]*/); //Need to pass all the halo, laplacian, and all other info here from computation
	#endif
									p++;
									shouldIBeginSorting = true; // We already begin sorting. So it is false now
								}
								else
								{
									currTimems= ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

									//Sorting is running
									//printf("Sorting is running\*****%d\n", p);
									if (!terminate)
									{
	#ifdef EYE_SIDE
										terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) tmpImage->imageData,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, currTimems, 0.0, m_eyeLabel, output.x/*DMOTODarEyeSide[eyeIdx]*/); //Need to pass all the halo, laplacian, and all other info here from computation
	#else
										terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) tmpImage->imageData,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, currTimems, 0.0, output.x/*DMOTODarEyeSide[eyeIdx]*/); //Need to pass all the halo, laplacian, and all other info here from computation
	#endif
										p++;
									}
								}
							}
							 else
							 {
									if (shouldIBeginSorting == true)
									{
										unsigned long NoEyeCurrTimems = 0;

									NoEyeCurrTimems= ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

									if (!terminate)
									{
						#ifdef EYE_SIDE
										terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0, m_eyeLabel); //Need to pass all the halo, laplacian, and all other info here from computation
						#else
										terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0); //Need to pass all the halo, laplacian, and all other info here from computation
						#endif
										// usleep (10); // usleep(10000);
									}
								}
							}

							} // EnableSharpnessloop


						}
					} // if (bIrisCapture)

				} //end of laplacian if

				std::pair<int, int> blt = m_nanoSpoofDetector->GetIrisPupilIntensities();
				irisCentroid = m_nanoSpoofDetector->GetSpecularityCentroid();

				if(m_shouldRotate)
				{
					//If rotated then swap centroid values.
					float temp =  irisCentroid.x;
					irisCentroid.x = irisCentroid.y;
					irisCentroid.y = temp;
				}
				eye->init(cam_idd, eyeIdx,m_faceIndex,maxEyes,left, top,irisCentroid,m_il0,1,output.x,score.second,blt.first,halo.z);
				eye->setTimeStamp(m_timestampBeforeGrabbing);
				eye->setUpdated(true);

				if (m_FocusBasedRejectionType||m_spoofEnable)
				{
					extractSmallCropForFocus(eyeIdx,eye);
					XTIME_OP("FocusCalc.ComputeFeatureVector",
						m_IrisSelectSvr->ComputeFeatureVector(m_smallCroppedEye,m_FocusRoi,m_IrisSelectFeatVect)
					);
					//Set the score value
					eye->setScore(m_IrisSelectFeatVect[1]);

					EyeInfo eyeInfo;
					eyeInfo.x=left;
					eyeInfo.y=top;
					eyeInfo.score=m_IrisSelectFeatVect[1];
					eyeInfo.handle=eye;
					eyeMap[eyeIdx]=eyeInfo;
					if(m_shouldLog){
						logFocusScore(eyeInfo.score);
					}
					//if(m_Debug)printf("Score[%d]=%d ",eyeIdx,eyeInfo.score);
				}
			} // end of (for)
		}
		else // No eyes detected...
		{
			if (shouldIBeginSorting == true)
			{
				unsigned long NoEyeCurrTimems = 0;

				NoEyeCurrTimems= ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

				if (!terminate)
				{
	#ifdef EYE_SIDE
					terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0, m_eyeLabel); //Need to pass all the halo, laplacian, and all other info here from computation
	#else
					terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0); //Need to pass all the halo, laplacian, and all other info here from computation
	#endif
					// usleep(10); // usleep(10000);
				}
			}
		}
	}
	else
	{
		// printf("\n\n F######### ProcessImageAcquisitionMode FRAME NULL ############### \n\n");

		if (shouldIBeginSorting == true)
		{
			// printf("\n\n F######### SORTING IS ON! ############### \n\n");

			unsigned long NoEyeCurrTimems = 0;

			NoEyeCurrTimems= ptime_in_ms_from_epoch1(boost::posix_time::microsec_clock::local_time());

			if (!terminate)
			{
#ifdef EYE_SIDE
				terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0, m_eyeLabel); //Need to pass all the halo, laplacian, and all other info here from computation
#else
				terminate =	eyeSortingWrapObj->GetBestPairOfEyes( (unsigned char *) NULL,-1, -1, -1, p, p, 1, 1, -1, 640, 480, -1, -1, NoEyeCurrTimems, 0); //Need to pass all the halo, laplacian, and all other info here from computation
#endif
				// usleep(10); // usleep(10000);
			}
		}

		// printf("\n\n F######### SORTING IS OFF! ############### \n\n");

	}

    // If we're done, we'll 0 to 2 best eyes... process them and send them out via http.
	if (terminate) // End of the sorting. Now gets the output
	{
		bool bSentCaptureImage = false;
//		std::pair<unsigned char*, unsigned char*> images = eyeSortingWrapObj->GetBestSortedPairOfEye();
		std::pair<IrisDetail *, IrisDetail *> images = eyeSortingWrapObj->GetSortedPairEyesDetail();

		printf("SORTING COMPLETE!\n");
		if (images.first == NULL && images.second == NULL)
			printf("Sorting returned with NO eyes\n");

		//If we are going to send the message out, we always use the Biometric format now...
		ISOBiometric theBiometric(getConf());
		bool bAddedAtLeastOne = false;
		BDBIrisRecord::ISO_IMAGEFORMAT imgFormat;

		// Determine image fromat
		if (!strcmp(m_IrisImageFormat, "FORMAT_PNG"))
			imgFormat = BDBIrisRecord::IMAGEFORMAT_MONO_PNG;
		else if (!strcmp(m_IrisImageFormat, "FORMAT_J2K"))
			imgFormat = BDBIrisRecord::IMAGEFORMAT_MONO_JPEG2000;
		else
			imgFormat = BDBIrisRecord::IMAGEFORMAT_MONO_RAW;

		IplImage *resizeImage;

		if (images.first != NULL)
		{
			// If we are sending out images...
			// 1.  Use INI file settings to determine format of data output...
			// 2.  Convert image data as appropriate
			// 3.  Create HTTPPostImage structure with IRIS data
			// 4.  Enque the data with the HTTPPostSender for formatting and transmission...
			if (m_pHttpPostSender != NULL)
			{
				// Add in our single image...
				BDBIrisRecord *pTheRecord = theBiometric.AddIrisBiometric();

				char *pImage = images.first->GetImage();

				IrisSelectorCircles circle1 = images.first->GetIrisSelectorCircles();
				if (m_bIrisCaptureEnableResize)
				{
					// First resize
					resizeImage = cvCreateImageHeader(cvSize(640, 480), 8, 1);
					resizeImage = ResizeFrame(640, 480, images.first->GetImage());
					pImage = resizeImage->imageData;
				}

				if (b_SaveBestEyes)
				{
					IplImage *imgHeader = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, 1);
					cvSetImageData(imgHeader, images.first->GetImage(), 640);

					sprintf(filename, "BestEye 1 -- %f_%s_Iris_%f_%f_%f.pgm", images.first->GetLaplacian(), (images.first->getSide() == 0) ? "UNKNOWN" : ((images.first->getSide() == 1) ? "LEFT" : "RIGHT"), circle1.IrisCircle.x, circle1.IrisCircle.y, circle1.IrisCircle.r);
					cv::Mat mateye = cv::cvarrToMat(imgHeader);
					imwrite(filename, mateye);

					cvReleaseImageHeader(&imgHeader);
				}

				// Add the iris image to our Biometric...  Specify the image format here...
				pTheRecord->SetImageData((uint8_t *)pImage, (uint32_t)640, (uint32_t)480,
						(images.first->getSide() == 0) ? BDBIrisRecord::SUBJECT_EYE_LABEL_UNDEF : ((images.first->getSide() == 1) ? BDBIrisRecord::SUBJECT_EYE_LABEL_LEFT : BDBIrisRecord::SUBJECT_EYE_LABEL_RIGHT),
						imgFormat);


				if (m_bIrisCaptureEnableResize)
					cvReleaseImageHeader(&resizeImage);

				bAddedAtLeastOne = true;
			}
		}

		if (images.second != NULL)
		{
			if (m_pHttpPostSender != NULL)
			{
				// Add in our single image...
				BDBIrisRecord *pTheRecord = theBiometric.AddIrisBiometric();

				char *pImage = images.second->GetImage();

				IrisSelectorCircles circle2 = images.first->GetIrisSelectorCircles();
				if (m_bIrisCaptureEnableResize)
				{
					// First resize
					resizeImage = cvCreateImageHeader(cvSize(640, 480), 8, 1);
					resizeImage = ResizeFrame(640, 480, images.second->GetImage());
					pImage = resizeImage->imageData;
				}

				if (b_SaveBestEyes)
				{
					IplImage *imgHeader = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, 1);
					cvSetImageData(imgHeader, images.second->GetImage(), 640);



					sprintf(filename, "BestEye 2 -- %f_%s_Iris_%f_%f_%f.pgm", images.second->GetLaplacian(), (images.second->getSide() == 0) ? "UNKNOWN" : ((images.second->getSide() == 1) ? "LEFT" : "RIGHT"), circle2.IrisCircle.x, circle2.IrisCircle.y, circle2.IrisCircle.r);
					cv::Mat mateye = cv::cvarrToMat(imgHeader);
					imwrite(filename, mateye);

					cvReleaseImageHeader(&imgHeader);
				}

				// Add the iris image to our Biometric...  Specify the image format here...
				pTheRecord->SetImageData((uint8_t *)pImage, (uint32_t)640, (uint32_t)480,
						(images.second->getSide() == 0) ? BDBIrisRecord::SUBJECT_EYE_LABEL_UNDEF : ((images.second->getSide() == 1) ? BDBIrisRecord::SUBJECT_EYE_LABEL_LEFT : BDBIrisRecord::SUBJECT_EYE_LABEL_RIGHT),
						imgFormat);

				if (m_bIrisCaptureEnableResize)
					cvReleaseImageHeader(&resizeImage);

				bAddedAtLeastOne = true;
			}

		}

		if (bAddedAtLeastOne)
		{
			m_pHttpPostSender->enquePostIris(theBiometric);
			bSentCaptureImage = true;
		}

		if (!m_DHSScreens) //  && bSentCaptureImage)
		{
			unsigned char buf[256];
			buf[0] = CMX_LED_CMD;
			buf[1] = 3;
			buf[2] = 0;
			buf[3] = 255;
			buf[4] = 0;
			if (m_pCMXHandler){
				m_pCMXHandler->HandleSendMsg((char *)buf, m_pCMXHandler->m_Randomseed);
			}
			printf("******************** sleeping for 1 sec**********\n");
		}

		unsigned char buf[256];
		// DHS Screen functions
		if(m_DHSScreens)
		{

			buf[0] = CMX_LED_CMD;
			buf[1] = 3;
			buf[2] = 0;
			buf[3] = 255;
			buf[4] = 0;
			if (m_pCMXHandler) {
				m_pCMXHandler->HandleSendMsg((char *) buf, m_pCMXHandler->m_Randomseed);
			}

#if 0
			Screen = cv::imread("/home/root/screens/Slide3.BMP", cv::IMREAD_COLOR);
			imshow("EXT", Screen);
#else
			cvShowImage("EXT", IplImageScreen3);
#endif
			int nDelay = 0;

			while (nDelay < 3000)
			{
				cvWaitKey(1);
				usleep(50000);
				nDelay += 50;
			};

			if (m_IrisCaptureResetDelay > 3000)
			{
				nDelay = 0;
				while (nDelay < m_IrisCaptureResetDelay-3000)
				{
					cvWaitKey(1);
					usleep(50000);
					nDelay += 50;
				};

			}
#if 0
			Screen = cv::imread("/home/root/screens/Slide1.BMP", cv::IMREAD_COLOR);
			imshow("EXT", Screen);
			cvWaitKey(1);
#else
			cvShowImage("EXT", IplImageScreen1);
			cvWaitKey(1);
#endif
			buf[0] = CMX_LED_CMD;
			buf[1] = 3;
			buf[2] = m_LEDBrightness * 8/10;
			buf[3] = m_LEDBrightness;
			buf[4] = m_LEDBrightness;
			if (m_pCMXHandler) {
				m_pCMXHandler->HandleSendMsg((char *) buf, m_pCMXHandler->m_Randomseed);
			}
			/*
			IplImage *frame1;
			do{
				frame1 = GetFrame();
			}while(frame1 != NULL); */

		}else
		{
			usleep(m_IrisCaptureResetDelay *1000); // wait for next
			buf[0] = CMX_LED_CMD;
			buf[1] = 3;
			buf[2] = m_LEDBrightness * 8/10;
			buf[3] = m_LEDBrightness;
			buf[4] = m_LEDBrightness;
			if (m_pCMXHandler) {
				m_pCMXHandler->HandleSendMsg((char *) buf, m_pCMXHandler->m_Randomseed);
			}
		}
		//clearing the eyeSorting variables
		aquisition->clearFrameBuffer(); // Clear frame buffer
		g_pLeftCameraFaceQueue->Clear(); // Clear FaceQueue
		g_pRightCameraFaceQueue->Clear(); // Clear FaceQueue
		eyeSortingWrapObj->clearAllEyes(); // Check Anita later should be uncommented
		terminate = false; //Making terminate false to restart our sorting
		shouldIBeginSorting = false;  //Next image we should start our sorting
		// eyeSortingWrapObj->clearAllEyes(); // Check Anita later should be uncommented
	} // End of terminate
    return bSentSomething;
}

#endif


bool ImageProcessor::ProcessSpoofFlowImage(IplImage *frame,bool matchmode){
	XTIME_OP("SetImage",
		SetImage(frame)
	);

    m_inputImg.Init(frame);
    XTIME_OP("Pyramid",
    		m_sframe.SetImage(&m_inputImg);
    );
    ComputeMotion();
    bool detect;
    XTIME_OP("Detect",
    	detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel)
    );
    if(m_shouldLog){
        logResults(m_faceIndex);
    }
    if(m_saveCount > 0){
        m_saveCount--;
        m_sframe.saveImage("result", m_faceIndex);
    }
    bool bSentSomething = false;
    int left, top;
    std::map<int,EyeInfo> eyeMap;
    int maxEyes = m_sframe.GetEyeCenterPointList()->size();
    if(m_Debug)
    	printf("NumEyes %d \n", maxEyes);

    if(maxEyes > m_maxEyes){
    	int clear = ClearEyes(m_faceIndex);
        return bSentSomething;
    }
//    static int prevmaxEyes = 0;
//
//    IplImage *ptr = m_sframe.GetPyramid()->GetLevel(2)->GetData();
//
//    if((prevmaxEyes > 0) && (maxEyes == 0) ){
//    	m_sframe.saveImage("MadhavSave",m_faceIndex,2);
//    	savefile_OfSize_asPGM_index(bufImag,ptr->widthStep,ptr->height,"MadhavprevSave",m_faceIndex-1);
//    }
//
//    memcpy(bufImag,ptr->imageData,ptr->imageSize);
//    prevmaxEyes = maxEyes;

    int CameraId = frame->imageData[2]&0xff;

    for(int eyeIdx=0;eyeIdx<maxEyes;eyeIdx++){
    	CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);
		DetectedEye *eye=getNextAvailableEyeBuffer();
		std::pair<float, float> score;
		CvPoint3D32f halo;
		XTIME_OP("HALO",
			m_sframe.GetCroppedEye(eyeIdx, eye->getEyeCrop(), left, top);
			score = m_nanoSpoofDetector->check_image(eye->getEyeCrop()->imageData,eye->getEyeCrop()->width, eye->getEyeCrop()->height,eye->getEyeCrop()->widthStep);
			//halo =  m_nanoSpoofDetector->ComputeHaloScore(eye->getEyeCrop(),m_currMaxSpecValue);
			halo =  m_nanoSpoofDetector->ComputeTopPointsBasedHaloScore(eye->getEyeCrop(),m_currMaxSpecValue,m_haloCountByBottomPixels,m_haloTopPixelsPercentage,m_haloBottomPixelsIntensityThresh,m_haloThreshold,m_enableHaloThreshold,m_MHaloNegationThresh);
			CheckForHaloScoreThreshold(halo);
		);
		std::pair<int, int> blt = m_nanoSpoofDetector->GetIrisPupilIntensities();
		irisCentroid = m_nanoSpoofDetector->GetSpecularityCentroid();

		if(m_shouldRotate){
			//If rotated then swap centroid values.
			float temp =  irisCentroid.x;
			irisCentroid.x = irisCentroid.y;
			irisCentroid.y = temp;
		}

		eye->init(CameraId, eyeIdx,m_faceIndex,maxEyes,left, top,irisCentroid,m_il0,1,score.first,score.second,blt.first,halo.z);
		eye->setTimeStamp(m_timestampBeforeGrabbing);
		eye->setUpdated(true);

	    extractSmallCropForFocus(eyeIdx,eye);
		XTIME_OP("FocusCalc.ComputeFeatureVector",
			m_IrisSelectSvr->ComputeFeatureVector(m_smallCroppedEye,m_FocusRoi,m_IrisSelectFeatVect)
		);
		//Set the score value
		eye->setScore(m_IrisSelectFeatVect[1]);

		EyeInfo eyeInfo;
		eyeInfo.x=left;
		eyeInfo.y=top;
		eyeInfo.score=m_IrisSelectFeatVect[1];
		eyeInfo.handle=eye;
		eyeMap[eyeIdx]=eyeInfo;
	}


    if(m_spoofEnable){
		std::list<std::pair<void *, void *> > result;
		result = m_EyeTracker->generate_pairings(m_faceIndex, eyeMap);
		DetectedEye *eyep,*eyec;
		if(result.size()>0){
			for(std::list<std::pair<void *, void *> >::iterator lit=result.begin(); lit!= result.end(); lit++){
				eyep = (DetectedEye *)lit->first;
				eyec = (DetectedEye *)lit->second;
				eyec->setPrev(eyec->getEyeIndex());
				m_NumTrackedEyes++;
			}
		}
    }
    bSentSomething = SendSpoofCheckEyes(bSentSomething,m_faceIndex);
    int clear = ClearEyes(m_faceIndex);
    return bSentSomething;
}

bool ImageProcessor::SendSpoofCheckEyes(bool & bSentSomething,int faceindex)
{
	timeval t;
	gettimeofday(&t, 0);

	TV_AS_MSEC(t,t1);

    int count = 0;
    int size = m_DetectedEyeWriteIterator.getsize();
    for(int i = 0;i < size;i++, m_DetectedEyeWriteIterator.next()){
        DetectedEye *eye = m_DetectedEyeWriteIterator.curr();
        if(eye->isUpdated() && eye->getFaceIndex() < faceindex){
            bool tobesend = true;
            if(m_enableBlackLevel && tobesend){
                if(((eye->getBlackLevel() > m_blackThreshold) || (eye->getBlackLevel() < m_blackMinThreshold)))
                    tobesend = false;
            }
            if(m_enableAreaFocus && tobesend){
                if(!(eye->getAreaScore() < m_areaThreshold) && (eye->getFocusScore() > m_focusThreshold)){
                    tobesend = false;
                }
            }
            if(tobesend){
                if(m_spoofDebug)
                    printf("%llu::Added [FI]%d [EI]%d [AS]%f [FS]%f [BL]%f [CX]%f [CY]%f [HS]%f\n",t1,eye->getFaceIndex(), eye->getEyeIndex(), eye->getAreaScore(), eye->getFocusScore(), eye->getBlackLevel(), eye->getIrisCentroid().x, eye->getIrisCentroid().y,eye->getHalo());
                sendToNwQueue(eye);
                bSentSomething = true;
                count++;
            }
        }
    }
    return bSentSomething;
}

bool ImageProcessor::process(bool matchmode) {
	bool bSentSomething = false;
	// printf("Inside ImageProcessor Process Inside ImageProcessor Process Inside ImageProcessor Process Inside ImageProcessor Process \n");
	if(!m_shouldDetect){
		printf("Inside Send Live Images\n");
		sendLiveImages();
		return true;
	}
	m_matchingmode = matchmode;
	timeval pcurtime;
	static uint64_t ppt=0;
	gettimeofday(&pcurtime, 0);
	TV_AS_USEC(pcurtime,pct);

	float freq = 0;
	if (ppt!=0){
		freq = 1000000.0/(pct - ppt);
	}
	ppt = pct;

	if(m_svrAddr)
		SendHBStatus(pcurtime.tv_sec);

	if(m_Debug && !(m_faceIndex % 10))
		printf("face=> %d %5.2f\n",m_faceIndex,freq);

#if 0
	// Here we grab L2 image directly from 16 bit buffer

#else

	IplImage *frame = NULL; //
	while(frame == NULL){
	XTIME_OP("GetFrame",
			frame = GetFrame()

	);
	/*
	if(frame == NULL)
		printf("FRAME IS NULL FRAME IS NULL FRAME IS NULL FRAME IS NULL FRAME IS NULL\n");
	else
		printf("Get Frame Frame is not NULL\n"); */
	// cvSaveImage("Before.pgm", frame);
#if 0
	IplImage *test = cvCreateImage(cvSize(1200, 960),IPL_DEPTH_8U,1);
	cvFlip(frame, test, 0);
	cvSaveImage("FLIPPED.pgm", test);
	cvCopyImage(test, frame);
	cvSaveImage("FRAME.pgm", frame);
	cvReleaseImage(&test);
#endif
#if 0
	cvFlip(frame,frame,0);
	cvSaveImage("FLIPPED.pgm", frame);
#endif
#if 0
		char filename[50];
		static int Index = 0;
		sprintf(filename,"InputImage_%d.pgm",Index++);
		// cvSaveImage(filename, frame);
		cvSaveImage("AfterGetFrame.pgm", frame);
		// return false;
#endif
	// printf("After GetFrame\n");
	CURR_TV_AS_USEC(ts);

		XTIME_OP("Process",
				if(m_Debug)printf("%llu ID:%d \n",m_timestampBeforeGrabbing,m_faceIndex);

				// printf("m_spoofEnable...%d\n", m_spoofEnable);

				if(m_spoofEnable)
					bSentSomething = ProcessSpoofFlowImage(frame,matchmode);
				else
					bSentSomething = ProcessImage(frame,matchmode);

			m_prevTS = m_timestampBeforeGrabbing;
		);

#endif
		if (m_DHSScreens && (m_EyelockIrisMode == 2))
			cvWaitKey(1);
	} // end while
	if (m_DHSScreens && (m_EyelockIrisMode == 2))
		cvWaitKey(1);
    return bSentSomething;
}


bool ImageProcessor::SendHB(bool bSentSomething){
	if(!bSentSomething)
	{
		// if we have not sent any thing in this call, lets at least send a HB
		bSentSomething=sendHeartBeat();
	}
	return bSentSomething;
}
bool ImageProcessor::sendHeartBeat(){
	if(!shouldSendHearBeat()) return false;
	// sending null is equivalent to sending HB
	sendToNwQueue();
	return true;
}

void ImageProcessor::CheckConfusion(DetectedEye *eye,bool tobesend){
	if(tobesend){
		m_prevConfusionTS = 0;
		return;
	}
	if(m_prevConfusionTS == 0){
		if(!tobesend){
			m_prevConfusionTS = eye->getTimeStamp();
			m_lastEyeConfusionTS = eye->getTimeStamp();
			if(m_confusionDebug)printf("Start the Time For Conf %llu \n",m_prevConfusionTS);
		}
	}else{
		int64_t value = (eye->getTimeStamp() - m_lastEyeConfusionTS);
		value = value>0?value:-value;
		if(value > m_confusionTimeThresholduSec){
			m_prevConfusionTS = eye->getTimeStamp();
			m_lastEyeConfusionTS = eye->getTimeStamp();
			return;
		}
		m_lastEyeConfusionTS = eye->getTimeStamp();
		value = (eye->getTimeStamp() - m_prevConfusionTS);
		value = value>0?value:-value;
		if(m_confusionDebug)printf("Time Diff For Confusion %llu - %llu = %lld\n",eye->getTimeStamp(),m_prevConfusionTS,value);
		if(value > m_confusionTimeThresholduSec){
			LEDResult ld;
			ld.setState(LED_CONFUSION);
			if(m_confusionDebug)printf("Confusion Detected \n");
			if(m_tsDestAddrpresent){
				if(m_nwLedDispatcher)m_nwLedDispatcher->enqueMsg(ld);
				m_prevConfusionTS = 0;
				m_lastEyeConfusionTS=0;
				return;
			}
			if(m_LedConsolidator){
				if(m_LedConsolidator)m_LedConsolidator->enqueMsg(ld);
			}
			m_prevConfusionTS = 0;
			m_lastEyeConfusionTS=0;
		}
	}

}


bool ImageProcessor::CheckHaloAndBLC(DetectedEye *eye){
    bool tobesend = true;
    static int cnt=0;
     if(m_enableBlackLevel && tobesend){
         if(((eye->getBlackLevel() > m_blackThreshold) || (eye->getBlackLevel() < m_blackMinThreshold))){
        	 if(m_Debug)printf("Discarding on behalf of BLC %d %d %f %f\n",eye->getFaceIndex(),eye->getEyeIndex(),eye->getBlackLevel(),eye->getHalo());
             tobesend = false;
         }
     }

     if ((m_enableLaplacian_focus_Threshold ||m_enableLaplacian_focus_ThresholdEnroll)&& tobesend){
    	 float lapscore = m_laplacian_focusThreshold;
     	 if(!m_matchingmode){
    		 lapscore = m_laplacian_focusThresholdEnrollment;
    	 }
    	 bool test = true;
    	 if((eye->getAreaScore()) < lapscore){
         	 test = false;
         }
    	 if(m_matchingmode && m_enableLaplacian_focus_Threshold){
    	 	 if(!test) tobesend = false;
    	 }
    	 if((!m_matchingmode)&&m_enableLaplacian_focus_ThresholdEnroll){
    		 if(!test) tobesend = false;
    	 }
    	 if(!tobesend){
    		 if(m_Debug||m_lapdebug)printf("Discarding on behalf of Laplacian %d %d %f %f\n",eye->getFaceIndex(), eye->getEyeIndex(), eye->getAreaScore(), lapscore);
    	 }
     }

     if (m_enableLaplacian_focus_Threshold && tobesend){
      	 if((eye->getAreaScore()) < m_laplacian_focusThreshold){
         	 if(m_Debug)printf("Discarding on behalf of Laplacian %d %d %f %f\n",eye->getFaceIndex(), eye->getEyeIndex(), eye->getAreaScore(), m_laplacian_focusThreshold);
              tobesend = false;

          }
     }

     if(m_enableHaloThreshold && tobesend){
         //if((eye->getHalo() > m_currHaloThreshold) || (eye->getHalo() < m_haloMinThreshold)){
    	 if(eye->getHalo() < m_haloMinThreshold){
        	 if(m_Debug)printf("Discarding on behalf of HALO %d %d %f %f %d %f\n",eye->getFaceIndex(),eye->getEyeIndex(),eye->getBlackLevel(),eye->getHalo(),m_currMaxSpecValue,m_currHaloThreshold);
             tobesend = false;
         }
     }
     if(m_saveDiscardedEyes&&(!tobesend)){
    	 printf("Saving discarded eyes %d %d\n",cnt,m_discardedSaveCount);
    	 SAVE_NAME_INDX(eye->getEyeCrop(),"Discarded",m_discardedSaveCount++);
     }
     cnt++;

     if(m_checkConfusion){
    	 CheckConfusion(eye,tobesend);
     }


     unsigned char buf[256];
     buf[0] = CMX_EYE_DETECT;

     if(tobesend)
     {
    	 printf("************sending LED Detect************\n");
    	 LEDResult ld;
    	 ld.setState(LED_DETECT);
    	 if(m_nwLedDispatcher)m_nwLedDispatcher->enqueMsg(ld);
    	 if(m_LedConsolidator)
    	 {
    		 printf("************sending LED Detect - LEDCOnsi************\n");
    	 				if(m_LedConsolidator)m_LedConsolidator->enqueMsg(ld);
    	 }
     }

     return tobesend;
}

void ImageProcessor::sendToNwQueue(DetectedEye *info){

	if(info){
		if(info->getSentStatus())
			return;
		if(!CheckHaloAndBLC(info))
			return;
	}
	// find an available networkBuffer
	incrNwQueueToNextAvailableBuffer();
	// write this frame to that Buffer
	SafeFrameMsg *outMsg = m_outMsgQueue.getCurr();

	IplImage *outImg=0;
	if(info){
		outImg=info->getEyeCrop();

		int width = m_inputImg.GetWidth(), height = m_inputImg.GetHeight();

		//we may have to rotate it
		if(m_shouldRotate){
			cvTranspose(outImg,m_rotationBuff);
			outImg=m_rotationBuff;
			info->swapLeftTop();

			int tmp = height;
			height = width;
			width = tmp;
		}

		//we may need to flip it
		if(m_FlipType != 2){
			//if(m_Debug)printf("Flipping %d %d \n",info->getFaceIndex(),info->getEyeIndex());
			cvFlip(outImg,NULL,m_FlipType);
			switch(m_FlipType)
			{
			case -1: info->flipOnXAxis(width, height); info->flipOnYAxis(width, height); break; // both axis
			case 0:  info->flipOnXAxis(width, height); break; // flip around x-axis
			case +1: info->flipOnYAxis(width, height); break; // flip around y-axis
			}
		}

	}

	if(m_outSaveCount && outImg){
		SAVE_NAME_INDX(outImg,"Detected",m_outSaveCount--);
		//savefile_OfSize_asPGM_index((unsigned char*)outImg->imageData,outImg->width, outImg->height,"Detected",m_outSaveCount--);
	}


	int len;

	outMsg->lock();

	__int64_t currtimestamp = m_timestampBeforeGrabbing;


	outMsg->SetTime(currtimestamp);

	// printf("CamrraId....%d\n", info->getCameraIndex());


	if(info){
		if (m_Debug)
			printf("%llu::ImageDetection %d %d \n",info->getTimeStamp()/1000,info->getFaceIndex(),info->getEyeIndex());
		info->setAlreadySent(true);
		int il0=info->getIlluminatorStatus();
		len = writer->write(
				outMsg->GetBuffer(), m_bufSize, outImg, ++m_id,info->getCameraIndex(),info->getFaceIndex(),info->getEyeIndex(),
			info->getNumEyes(),info->getEyeLeft(), info->getEyeTop(),1.0f,info->getScore(),info->getTimeStamp(),il0,info->getSpoof(),info->getAreaScore(),info->getFocusScore(),
			info->getBlackLevel(),info->getIrisCentroid().x,info->getIrisCentroid().y,info->getPrev(),info->getHalo(),info->getNumBits());
		//printf("Score %d",info->getScore());
	}else{
		// write an heartbeat
		gettimeofday(&m_lastHBSent,0);
		len= writer->writeHB(outMsg->GetBuffer(), m_bufSize);
	}

	bool bIncr=true;
	if (len>0) {
		outMsg->SetSize(len);
		// if it was already marked updated we are reducing the count
		if(outMsg->isUpdated()){
			bIncr=false;
		}
		outMsg->setUpdated(true);
		m_outMsgQueue++;
	}

	outMsg->unlock();

	if(bIncr) m_outMsgQueue.incrCounter();

	// finally make sure that this item is available for our processing
	if(!m_spoofEnable){
		if(info)  info->setUpdated(false);
	}
}

// go over the detected Eyes queue and find a slot which is available
DetectedEye * ImageProcessor::getNextAvailableEyeBuffer(){
	bool bFound=false;
	int size=m_DetectedEyeWriteIterator.getsize();
	DetectedEye *currItem=0;
	for(int i=0;i<size;i++,m_DetectedEyeWriteIterator.next()){
		currItem=m_DetectedEyeWriteIterator.curr();
		if(!currItem->isUpdated()) {
			bFound=true;
			break;
		}
		if(currItem->getFaceIndex()+1 < m_faceIndex){
			bFound=true;
			break;
		}
	}
	if(!bFound){
		printf("overwriting detected Eye Buffer\n");
	}
	currItem->setUpdated(false);
	m_DetectedEyeWriteIterator.next();
	return currItem;
}

int ImageProcessor::ClearEyes(int faceindex,bool all){
	int count = 0;
	int size=m_DetectedEyeWriteIterator.getsize();
	DetectedEye *currItem=0;
	for(int i=0;i<size;i++,m_DetectedEyeWriteIterator.next()){
		currItem=m_DetectedEyeWriteIterator.curr();
		if((all)||((currItem->getFaceIndex() < faceindex)&&(currItem->isUpdated()))){
			currItem->setUpdated(false);
			count++;
		}
	}
	return count;
}

void ImageProcessor::incrNwQueueToNextAvailableBuffer(){
	SafeFrameMsg *outMsg=0;
	int size=m_outMsgQueue.getSize();
	for(int i=0;i<size;i++,m_outMsgQueue++)
	{
		outMsg=m_outMsgQueue.getCurr();
		if(!outMsg->isUpdated()) break;
	}
}

void ImageProcessor::logFocusScore(int score){
	FILE *logFile=fopen(m_LogFileName.c_str(),"a+");
	fprintf(logFile,"ES[%d] ",score);
	fclose(logFile);
}


void ImageProcessor::logFocusResults(std::map<std::pair<int,int>,void* > focusResults){
	FILE *logFile=fopen(m_LogFileName.c_str(),"a+");
	std::map<std::pair<int, int>, void * > ::iterator rit=focusResults.begin();
	fprintf(logFile,"\n");
	for(;rit!=focusResults.end();rit++){
		fprintf(logFile,"FS[%d,%d] ",rit->first.first, rit->first.second);
	}
	fclose(logFile);
}
#if 0
void ImageProcessor::logResults(int frameIndex){
	FILE *logFile=fopen(m_LogFileName.c_str(),"a+");
	fprintf(logFile,"\n");
	if(m_pSrv->IsSingleSpecMode())
	{
	EyeCenterPointList* pEyeList=m_sframe.GetEyeCenterPoints();
	for(unsigned int i=0;i<pEyeList->size();i++){
		CEyeCenterPoint& eye=pEyeList->at(i);
		fprintf(logFile,"%c(%d,%d) ",eye.GetIsHaarEye()?'H':'S', eye.m_nCenterPointX, eye.m_nCenterPointY);
	}
	}
	else
	{
		EyeCenterPointList* pEyeList=m_sframe.GetEyeCenterPoints();
		for(unsigned int i=0;i<pEyeList->size();i++){
			CEyeCenterPoint& eye=pEyeList->at(i);
			fprintf(logFile,"%c[(%d,%d) (%d,%d)]",eye.GetIsHaarEye()?'H':'S', eye.m_nLeftSpecularityX, eye.m_nLeftSpecularityY, eye.m_nRightSpecularityX, eye.m_nRightSpecularityY);
		}
	}
	fprintf(logFile,"\n");
	fprintf(logFile,"Frame: %d\tSpec Eyes:%d\tHaar Eyes:%d\n",frameIndex,m_sframe.GetNumberOfSpecularityEyes(),m_sframe.GetNumberOfHaarEyes());
	fprintf(logFile,"\n-----------------------------------\n");
	fclose(logFile);
}
#else
void ImageProcessor::logResults(int frameIndex){
	if(m_pSrv->IsSingleSpecMode())
	{
	EyeCenterPointList* pEyeList=m_sframe.GetEyeCenterPoints();
	LOGI(" pEyeList=%#x",pEyeList);
	for(unsigned int i=0;i<pEyeList->size();i++){
		CEyeCenterPoint& eye=pEyeList->at(i);
		LOGI("%c(%d,%d) ",eye.GetIsHaarEye()?'H':'S', eye.m_nCenterPointX, eye.m_nCenterPointY);
	}
	}
	else
	{
		EyeCenterPointList* pEyeList=m_sframe.GetEyeCenterPoints();
		for(unsigned int i=0;i<pEyeList->size();i++){
			CEyeCenterPoint& eye=pEyeList->at(i);
			LOGI("%c[(%d,%d) (%d,%d)]",eye.GetIsHaarEye()?'H':'S', eye.m_nLeftSpecularityX, eye.m_nLeftSpecularityY, eye.m_nRightSpecularityX, eye.m_nRightSpecularityY);
		}
	}
	LOGI("\n");
	LOGI("Frame: %d\tSpec Eyes:%d\tHaar Eyes:%d\n",frameIndex,m_sframe.GetNumberOfSpecularityEyes(),m_sframe.GetNumberOfHaarEyes());
	LOGI("\n-----------------------------------\n");
}

#endif

bool ImageProcessor::checkLicense()
{
	// check license on device
	if (access("/etc/FactoryLicense", F_OK ) != -1) {
		// device licensed, but no user license
		if( access("/home/UserLicense", F_OK ) == -1) {
			printf(" No user license installed - stop iris detection\n");
			return false;
		}
	}
	return true;
}

