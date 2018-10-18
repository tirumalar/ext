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
// #include <highgui.h>
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


extern "C"{
#ifdef __BFIN__
	#include <bfin_sram.h>
#endif
}
#include "SocketFactory.h"

CmxHandler *mm_pCMXHandle;
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


#if 0
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
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
m_haloTopPixelsPercentage(25.0f),m_haloCountByBottomPixels(6),m_haloBottomPixelsIntensityThresh(91),m_MHaloNegationThresh(350),m_FaceIrisMapping(false){
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
    m_shouldDetect = pConf->getValue("GRI.shouldDetectEye", true);
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

	// check license on device
	if (m_shouldDetect && access("/etc/FactoryLicense", F_OK ) != -1) {
		// device licensed, but no user license
		if( access("/home/UserLicense", F_OK ) == -1) {
			m_shouldDetect = false;
			printf(" No user license installed - stop iris detection\n");
		}
	}
	
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

	m_EnableOffsetCorrection = pConf->getValue("Eyelock.EnableGainOffsetCorrection",false);
	int Imagewidth = pConf->getValue("FrameSize.width",1200);
	int Imageheight = pConf->getValue("FrameSize.height",960);
	m_OffsetOutputImage = cvCreateImage(cvSize(Imagewidth, Imageheight),IPL_DEPTH_8U,1);

	// Face Mapping Projection
	m_FaceIrisMapping = pConf->getValue("Eyelock.FaceIrisMapping",false);
	m_IrisProjImage = cvCreateImage(cvSize(Imagewidth, Imageheight),IPL_DEPTH_8U,1);
	m_SaveProjImage = pConf->getValue("Eyelock.SaveProjectedImage",false);
	m_showProjection = pConf->getValue("Eyelock.showProjection",false);

	FileConfiguration m_FaceConfig("/home/root/data/calibration/faceConfig.ini");
	rectX = m_FaceConfig.getValue("FTracker.targetRectX",0);
	rectY = m_FaceConfig.getValue("FTracker.targetRectY",497);
	rectW = m_FaceConfig.getValue("FTracker.targetRectWidth",960);
	rectH = m_FaceConfig.getValue("FTracker.targetRectHeight",121);

	magOffMainl = m_FaceConfig.getValue("FTracker.magOffsetMainLeftCam",float(0.15));
	magOffMainR = m_FaceConfig.getValue("FTracker.magOffsetMainRightCam",float(0.15));
	magOffAuxl = m_FaceConfig.getValue("FTracker.magOffsetAuxLeftCam",float(0.22));
	magOffAuxR = m_FaceConfig.getValue("FTracker.magOffsetAuxRightCam",float(0.22));


	constantMainl.x = m_FaceConfig.getValue("FTracker.constantMainLeftCam_x",float(0.15));
	constantMainl.y = m_FaceConfig.getValue("FTracker.constantMainLeftCam_y",float(0.15));
	constantMainR.x = m_FaceConfig.getValue("FTracker.constantMainRightCam_x",float(0.15));
	constantMainR.y = m_FaceConfig.getValue("FTracker.constantMainRightCam_y",float(0.15));
	constantAuxl.x = m_FaceConfig.getValue("FTracker.constantAuxLeftCam_x",float(0.22));
	constantAuxl.y = m_FaceConfig.getValue("FTracker.constantAuxLeftCam_y",float(0.22));
	constantAuxR.x = m_FaceConfig.getValue("FTracker.constantAuxRightCam_x",float(0.22));
	constantAuxR.y = m_FaceConfig.getValue("FTracker.constantAuxRightCam_y",float(0.22));

	useOffest_m = m_FaceConfig.getValue("FTracker.projectionOffsetMain",false);
	useOffest_a = m_FaceConfig.getValue("FTracker.projectionOffsetAux",true);
	projOffset_m = m_FaceConfig.getValue("FTracker.projectionOffsetValMain",float(50.00));
	projOffset_a = m_FaceConfig.getValue("FTracker.projectionOffsetValAux",float(200.00));



#ifdef DEBUG_SESSION
	m_DebugTesting = pConf->getValue("Eyelock.TestSystemPerformance",false);
	m_sessionDir = string(pConf->getValue("Eyelock.DebugSessionDir","DebugSessions/Session"));
#endif

#ifdef HBOX_PG
	m_SPAWAREnable = pConf->getValue("Eyelock.SPAWAREnable",false);
	if (m_SPAWAREnable)
	{
		m_pHttpPostSender = new HttpPostSender(*pConf);
		m_pHttpPostSender->init();
		m_pHttpPostSender->Begin();
	}

#endif
//	if()
	mm_pCMXHandle = new CmxHandler(*pConf);
	m_bShouldSend =pConf->getValue("Eyelock.EyeMessage",true);
}

static void flipOnYAxis(int &x, int &/*y*/, int width, int /* height */)
{
	x = width - x - 1;
}

static void flipOnXAxis(int &/*x*/, int &y, int /* width */, int height)
{
	y = height - y - 1;
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
	if(m_IrisProjImage){
		cvReleaseImage(&m_IrisProjImage);
	}
#ifdef HBOX_PG
	if(m_pHttpPostSender){
		delete m_pHttpPostSender;
	}

#endif
#if 0	
	if(mm_pCMXHandle){
		delete mm_pCMXHandle;
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
	IplImage *frame = aquisition->getFrame();
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

extern double scaling;

float ImageProcessor::projectPoints(float y, float c, float m)
{
	//y = mx + c
	//y is Face points and x is Iris points
	//x = (y-c)/m
	//printf("y::: %3.3f, c::: %3.3f, m::: %3.3f, x::: %3.3f\n", y,c,m, float((y-c)/m));
	return ((y-c)/m);

}

cv::Rect ImageProcessor::projectRect(cv::Rect face, int CameraId, IplImage *InputFrame, int FaceFrameNo, int IrisFrameNo)
{
	float scale = 1.0;
	int projDebug = true;
	int targetOffset1;
	cv::Rect IrisProjRect;
	targetOffset1 = 3;

	cv::Rect ret1(0,0,0,0);
	cv::Rect ret2(0,0,0,0);
	cv::Rect retd1(0,0,0,0);
	cv::Rect retd2(0,0,0,0);
	cv::Rect projFace;
	cv::Rect faceP = face;

	faceP.y = (rectY/scaling) + targetOffset1;
	faceP.height = (rectH)/scaling -(targetOffset1*2);

	if (projDebug)
	{
		projFace.x = face.x * scaling;		//column
		projFace.y = rectY + targetOffset1;	//row
		projFace.width = face.width * scaling;
		projFace.height = rectH -(targetOffset1*2);
		cv::Point2i ptr1, ptr2, ptr3, ptr4;

		if(CameraId == 01 ){
			//project two coordinates diagonally a part in face frame into Iris
			ptr1.x = projectPoints(projFace.x, constantMainl.x, magOffMainl);
			ptr1.y = projectPoints(projFace.y, constantMainl.y, magOffMainl);
			ptr2.x = projectPoints((face.x+face.width), constantMainl.x, magOffMainl);
			ptr2.y = projectPoints((projFace.y+projFace.height), constantMainl.y, magOffMainl);

			//check extreme conditions
			if (ptr1.x > 1200){
				ptr1.x = 1200;
			}
			if (ptr2.y > 960){
				ptr2.y = 960;
			}
			if (ptr1.x < 0){
				ptr1.x = 0;
			}
			if (ptr2.y < 0){
				ptr2.y = 0;
			}

			//create RECT
			ret1.x = ptr1.x;
			ret1.y = ptr1.y;
			ret1.width = abs(ptr1.x - ptr2.x);
			ret1.height = abs(ptr1.y - ptr2.y);

			//check extreme conditions
			if(ret1.width > 1200)
				ret1.width = 1200;
			if(ret1.height > 960)
				ret1.height = 960;

			//Offset correction if useOffset is true
			if(useOffest_m){
				ret1.height = ret1.height - int(projOffset_m);
			}

			IrisProjRect = ret1;

		}else if(CameraId == 129 ){
			//project two coordinates diagonally a part in face frame into Iris
			ptr1.x = projectPoints(projFace.x, constantAuxl.x, magOffAuxl);
			ptr1.y = projectPoints(projFace.y, constantAuxl.y, magOffAuxl);

			ptr2.x = projectPoints((face.x+face.width), constantAuxl.x, magOffAuxl);
			ptr2.y = projectPoints((projFace.y+projFace.height), constantAuxl.y, magOffAuxl);


			//check extreme conditions
			if (ptr1.x > 1200){
				ptr1.x = 1200;
			}
			if (ptr2.y > 960){
				ptr2.y = 960;
			}

			if (ptr1.x < 0){
				ptr1.x = 0;
			}
			if (ptr2.y < 0){
				ptr2.y = 0;
			}

			//create RECT
			ret1.x = ptr1.x;
			ret1.y = ptr1.y;
			ret1.width = abs(ptr1.x - ptr2.x);
			ret1.height = abs(ptr1.y - ptr2.y);

			//check extreme conditions
			if(ret1.width > 1200)
				ret1.width = 1200;
			if(ret1.height > 960)
				ret1.height = 960;

			//Offset correction if useOffset is true
			if(useOffest_a){
				ret1.height = ret1.height - int(projOffset_a);
			}

			IrisProjRect = ret1;

		}else if(CameraId == 02  ){
			//project two coordinates diagonally a part in face frame into Iris
			ptr3.x = projectPoints(projFace.x, constantMainR.x, magOffMainR);
			ptr3.y = projectPoints(projFace.y, constantMainR.y, magOffMainR);

			ptr4.x = projectPoints((face.x+face.width), constantMainR.x, magOffMainR);
			ptr4.y = projectPoints((projFace.y+projFace.height), constantMainR.y, magOffMainR);


			//check extreme conditions
			if (ptr3.x > 1200){
				ptr3.x = 1200;
			}
			if (ptr4.y > 960){
				ptr4.y = 960;
			}

			if (ptr3.x < 0){
				ptr3.x = 0;
			}
			if (ptr4.y < 0){
				ptr4.y = 0;
			}

			//create RECT
			ret2.x = ptr3.x;
			ret2.y = ptr3.y;
			ret2.width = abs(ptr3.x - ptr4.x);
			ret2.height = abs(ptr3.y - ptr4.y);

			//check extreme conditions
			if(ret2.width > 1200)
				ret2.width = 1200;
			if(ret2.height > 960)
				ret2.height = 960;

			//Offset correction if useOffset is true
			if(useOffest_m){
				ret2.height = ret2.height - int(projOffset_m);
			}

			IrisProjRect = ret2;

		}else if(CameraId == 130 ){
			//project two coordinates diagonally a part in face frame into Iris
			ptr3.x = projectPoints(projFace.x, constantAuxR.x, magOffAuxR);
			ptr3.y = projectPoints(projFace.y, constantAuxR.y, magOffAuxR);

			ptr4.x = projectPoints((face.x+face.width), constantAuxR.x, magOffAuxR);
			ptr4.y = projectPoints((projFace.y+projFace.height), constantAuxR.y, magOffAuxR);

			//check extreme conditions
			if (ptr3.x > 1200){
				ptr3.x = 1200;
			}
			if (ptr4.y > 960){
				ptr4.y = 960;
			}

			if (ptr3.x < 0){
				ptr3.x = 0;
			}
			if (ptr4.y < 0){
				ptr4.y = 0;
			}

			//create RECT
			ret2.x = ptr3.x;
			ret2.y = ptr3.y;
			ret2.width = abs(ptr3.x - ptr4.x);
			ret2.height = abs(ptr3.y - ptr4.y);

			//check extreme conditions
			if(ret2.width > 1200)
				ret2.width = 1200;
			if(ret2.height > 960)
				ret2.height = 960;

			//Offset correction if useOffset is true
			if(useOffest_a){
				ret2.height = ret2.height - int(projOffset_a);
			}
			IrisProjRect = ret2;

		}

	}
	return (IrisProjRect);

}

extern cv::Rect getFaceData();

extern int getFaceFrameNo();

struct FaceData1{
	int FaceFrameNo;
	cv::Rect FaceRect;
};

extern FaceData1 getFaceData1();

bool ImageProcessor::ProcessImage(IplImage *frame,bool matchmode)
{
	char filename[150];
#if 0
		boost::filesystem::path temp_session_dir(DEBUG_SESSION_DIR);
		if (boost::filesystem::is_directory(temp_session_dir))
		{
			char filename[100];
			sprintf(filename, "%s/InputImage_%d.pgm", temp_session_dir.c_str(), FrameNo++);
			cvSaveImage(filename,frame);
		}
#endif

#ifdef DEBUG_SESSION
	int cam_idd = 0;
	int frame_number = 0;
	if(frame->imageData != NULL)
	{
		cam_idd = frame->imageData[2]&0xff;
		frame_number = frame->imageData[3]&0xff;
	}

	if(m_DebugTesting){
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
			if(frame->imageData != NULL)
			{
				sprintf(filename, "%s/InputImage_%s_%lu_%09lu_%d_%d.pgm", m_sessionDir.c_str(), time_str, ts.tv_sec, ts.tv_nsec, frame_number, cam_idd);
				// cvSaveImage(filename,frame); // terminates the application if path doesn't exists
				cv::Mat mateye = cv::cvarrToMat(frame);
				imwrite(filename, mateye);
				
			}

			char session_match_log[100];
			sprintf(session_match_log, "%s/Info.txt", m_sessionDir.c_str());
			FILE *file = fopen(session_match_log, "a");
			if (file){
				fprintf(file, "%s %lu:%09lu Saved-InputImage-FrNum%d-CamID%d-%s\n", log_time_str, ts.tv_sec, ts.tv_nsec, frame_number, cam_idd, filename);
				fclose(file);
			}
		}
	}
#endif

	char temp[50];
#if 0
	char offset_image[IMAGE_SIZE];
	int length = frame->width*frame->height;
	// Loading an offset file - binary file
	if(m_EnableOffsetCorrection)
	{
		if(m_offset_image_loaded == false)
		{
			IplImage *DiffImage;
			int cam_id = frame->imageData[2]&0xff;
			sprintf(temp,"cal%02x.bin",cam_id);
			printf("Loading %s offset file\n",temp);
			FILE *fp = fopen(temp, "rb");
			if (fp)
			{
				fread(offset_image ,length, 1, fp);
				fclose(fp);
				m_offset_image_loaded = true;
				printf("Loading success file\n");
			}
		}
	}
#endif
#if 1 // Loading Offset file - PGM
	int cam_id = frame->imageData[2]&0xff;
	if(m_EnableOffsetCorrection){
		static bool m_OffsetImageLoadedMainCamera1 = false;
		static bool m_OffsetImageLoadedMainCamera2 = false;
		static bool m_OffsetImageLoadedAuxCamera1 = false;
		static bool m_OffsetImageLoadedAuxCamera2 = false;

		// int cam_id = frame->imageData[2]&0xff;
		sprintf(temp,"cal%02x.pgm",cam_id);
		// printf("cam_id....%02x filename...%s\n", cam_id, temp);

		if (m_OffsetImageLoadedMainCamera1 == false && (cam_id == 0x01)) {
			m_OffsetImageMainCamera1 = cvLoadImage("cal01.pgm", CV_LOAD_IMAGE_GRAYSCALE);
			m_OffsetImageLoadedMainCamera1 = true;
		}

		if (m_OffsetImageLoadedMainCamera2 == false && (cam_id == 0x02)) {
			m_OffsetImageMainCamera2 = cvLoadImage("cal02.pgm", CV_LOAD_IMAGE_GRAYSCALE);
			m_OffsetImageLoadedMainCamera2 = true;
		}

		if (m_OffsetImageLoadedAuxCamera1 == false && (cam_id == 0x81)) {
			m_OffsetImageAuxCamera1 = cvLoadImage("cal81.pgm", CV_LOAD_IMAGE_GRAYSCALE);
			m_OffsetImageLoadedAuxCamera1 = true;
		}

		if (m_OffsetImageLoadedAuxCamera2 == false && (cam_id == 0x82)) {
			m_OffsetImageAuxCamera2 = cvLoadImage("cal82.pgm", CV_LOAD_IMAGE_GRAYSCALE);
			m_OffsetImageLoadedAuxCamera2 = true;
		}

		if (m_OffsetImageLoadedMainCamera1  && (cam_id == 0x01)){
			cvSub(frame,m_OffsetImageMainCamera1,frame);
		}
		if (m_OffsetImageLoadedMainCamera2  && (cam_id == 0x02)){
			cvSub(frame,m_OffsetImageMainCamera2,frame);
		}
		if (m_OffsetImageLoadedAuxCamera1  && (cam_id == 0x81)){
			cvSub(frame,m_OffsetImageAuxCamera1,frame);
		}
		if (m_OffsetImageLoadedAuxCamera2  && (cam_id == 0x82)){
			cvSub(frame,m_OffsetImageAuxCamera2,frame);
		}

	}
#endif

	if(m_FaceIrisMapping)
	{
		FaceCoord = getFaceData();
		int FaceFrameNo = getFaceFrameNo();
		int IrisFrameNo = frame->imageData[3]&0xff;
		FaceData1 FaceData = getFaceData1();

		cv::Rect IrisProj = projectRect(FaceCoord, cam_id, frame, FaceFrameNo, IrisFrameNo);
		// printf("IrisProj  IrisProj.x %d IrisProj.y %d IrisProj.width %d IrisProj.height %d\n", IrisProj.x, IrisProj.y, IrisProj.width, IrisProj.height);
		cvSetData(m_IrisProjImage,frame->imageData,frame->widthStep);
		cvSetImageROI(m_IrisProjImage,IrisProj);
		cvSetData(frame,m_IrisProjImage->imageData,frame->widthStep);
		cvResetImageROI(m_IrisProjImage);

		if(m_SaveProjImage){
			sprintf(filename,"IrisImage_%d_%d.pgm", cam_idd, m_faceIndex);
			cv::Mat mateye1 = cv::cvarrToMat(m_IrisProjImage);
			imwrite(filename, mateye1);
		}

		if (m_showProjection){
			cv::Rect retd1(0,0,0,0);
			bool useOffest = false;
			float projOffset;
			float scale = 1.0;

			if(cam_id == 129 || cam_id == 130)
			{
				useOffest = useOffest_a;
				projOffset = projOffset_a;
			}
			else if(cam_id == 1 || cam_id == 2)
			{
				useOffest = useOffest_m;
				projOffset = projOffset_m;
			}
			retd1.x = IrisProj.x/scale;
			retd1.y = IrisProj.y/scale;
			retd1.width = IrisProj.width/scale;

			if (useOffest)
				retd1.height = (IrisProj.height - projOffset)/scale;
			else
				retd1.height = IrisProj.height /scale;

			cv::Mat imgIS1 =  cv::cvarrToMat(frame);
			cv::Mat imgl;
			cv::resize(imgIS1, imgl, cv::Size(), (1 / scale),(1 / scale), cv::INTER_NEAREST);
			cv::rectangle(imgl, retd1, cv::Scalar(255, 0, 0), 2, 0);
			imshow("Output", imgl);
			cvWaitKey(1);
		}
	}

	// printf("Inside ProcessImage\n");
	XTIME_OP("SetImage",
		SetImage(frame)
	);

	if(m_SaveFullFrame){
		sprintf(filename,"InputImage_%d_%d.pgm", cam_idd, m_faceIndex);
		cv::Mat mateye = cv::cvarrToMat(frame);
		imwrite(filename, mateye);
		// cvSaveImage(filename, frame);
	}
    m_inputImg.Init(frame);
    XTIME_OP("Pyramid",
    m_sframe.SetImage(&m_inputImg);
    );
    ComputeMotion();
    bool detect;

    XTIME_OP("Detect",
    	detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel)
    );
   //  printf("After Detect Eyes\n");
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

    for(int eyeIdx=0;eyeIdx<maxEyes;eyeIdx++){
    	CvPoint2D32f irisCentroid = cvPoint2D32f(0,0);
		DetectedEye *eye=getNextAvailableEyeBuffer();
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

#if 0
    if(maxEyes != 0)
    {
    	unsigned char buf[256];
    	buf[0] = CMX_EYE_DETECT;

       if(mm_pCMXHandle && m_bShouldSend)
       	{
        	   mm_pCMXHandle->HandleSendMsg((char *)buf);
       	}
    }
#endif
		if(m_SaveEyeCrops)
		{
#ifdef DEBUG_SESSION
			struct stat st = {0};
			if (stat(m_sessionDir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
				sprintf(filename,"%s/EyeCrop_PG_CamId_%d_%d.pgm", m_sessionDir.c_str(), cam_idd, m_faceIndex);
			}
			else
			{
				sprintf(filename,"EyeCrop_PG_CamId_%d_%d.pgm", cam_idd, m_faceIndex);
			}
#else
			sprintf(filename,"EyeCrop_PG_%d.pgm", m_faceIndex);
#endif
			cv::Mat mateye = cv::cvarrToMat(eye->getEyeCrop());
			imwrite(filename, mateye);
		}else{
			// sprintf(filename,"EyeCrop_PG_%d.pgm",eyeIdx);
			// cvSaveImage(filename, eye->getEyeCrop());
		}
#ifdef HBOX_PG
		if(m_SPAWAREnable){
			// printf("EyeCropSize.....%d\n", eyeCropSize);

			HTTPPOSTImage iris;

			iris.type = HTTPPOSTImage::IRIS;
			iris.bppx = 8;

			iris.horLineLength = eye->getEyeCrop()->width;
			iris.verLineLength = eye->getEyeCrop()->height;
			iris.size = iris.horLineLength * iris.verLineLength;

			// TODO: review
			// most likely this copying is not needed. Just provide enquePostIris with a pointer to imageData
			iris.data = new char[iris.size];
			memcpy(iris.data, eye->getEyeCrop()->imageData, iris.size);

			int eyePos = (eyeIdx == 1) ? 2 : 1;

			time_t rawtime;
			time (&rawtime);
			struct tm timeinfo = *localtime(&rawtime);
			iris.acquisTime = &timeinfo;

			m_pHttpPostSender->enquePostIris(iris, eyePos);

			delete[] iris.data;
		}
#endif
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
			sendToNwQueue(eye);
			bSentSomething=true;
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
    return bSentSomething;
}


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
	XTIME_OP("GetFrame",
			frame = GetFrame()

	);
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
#if 0 // Anita on Jul 10 for skipping of first frame
	bool goodframe=false;
//	printf("%d %ll %ll %ll \n",m_timestampBeforeGrabbing,m_prevTS,m_timegapForStaleframe);&&((pct - m_timestampBeforeGrabbing) < m_timegapForStaleframe*3)
	if(/*((m_timestampBeforeGrabbing - m_prevTS) < m_timegapForStaleframe)&&*/(ts - m_timestampBeforeGrabbing < 1000000)){
		goodframe = true;
	}
#else
	bool goodframe=true;
#endif
	bool bSentSomething = false;
	XTIME_OP("Process",
		if(frame && goodframe) // Deal with potential for NULL frame
		{
			if(m_Debug)printf("%llu ID:%d \n",m_timestampBeforeGrabbing,m_faceIndex);

			// printf("m_spoofEnable...%d\n", m_spoofEnable);

			if(m_spoofEnable)
				bSentSomething = ProcessSpoofFlowImage(frame,matchmode);
			else
				bSentSomething = ProcessImage(frame,matchmode);
		}else{
			if (m_Debug)
				printf("Discarding Image on timeStamp as stale %llu %llu %llu \n",m_prevTS,m_timestampBeforeGrabbing,ts);
		}
		m_prevTS = m_timestampBeforeGrabbing;
	);

#endif

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

    // if(mm_pCMXHandle && tobesend)
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
    	 //mm_pCMXHandle->HandleSendMsg((char *)buf);
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
