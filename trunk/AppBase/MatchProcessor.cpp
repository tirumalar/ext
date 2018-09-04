/*
 * MatchProcessor.cpp
 *
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include <iostream>


#include "MatchProcessor.h"
#include "Configuration.h"
#include "HTTPPOSTMsg.h"
#include "BiOmega.h"
#include "LEDDispatcher.h"
#include "NwDispatcher.h"
#include "F2FDispatcher.h"
#include "SpoofDetector.h"

#include "MatchManagerInterface.h"
//#include "DBWriter.h"
#include "VarianceBasedDetection.h"
#include "MatchManagerFactory.h"
#include "NwMatchManager.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include <opencv2/highgui/highgui.hpp>

//#include "IrisSelector.h"
extern "C" {
#include "file_manip.h"
#ifdef __BFIN__
	#include <bfin_sram.h>
#endif
	#include "GausPyr_private.h"
}
;
#include "logging.h"

const char logger[30] = "MatchProcessor";

using namespace std;
#define MAX(a,b) (a)>(b)?(a):(b)


MatchProcessor::MatchProcessor(Configuration& conf) :
	m_scaleDest(0), m_scaleSrcHeader(0), m_scratch(0),m_MatcherHDMStatus(true),m_ledDispatcher(0),m_nwDispatcher(0),m_spoofEnable(false),
	m_sendIter(m_inQueue),m_spoofDetector(0),m_IrisCode(0),m_matchManager(0),m_PCMatcher(0),m_IrisDataIndex(0),
	m_f2fDispatcher(0),m_PingTimeStamp(0), m_PingInterval(20),m_fVDetection(0),m_nwMatchManager(0),m_logging(false){
	mr.init();
	mr.setState(FAILED);
	m_PingInterval = conf.getValue("GRI.PingInterval",20);

	int inQSize = conf.getValue("server.inQSize", 30);
	m_width = 640;//conf.getValue("GRI.cropWidth", 640);
	m_height = 480;//conf.getValue("GRI.cropHeight", 480);
	m_Master = conf.getValue("GRI.EyelockMaster",false);
	m_softwareType = (SoftwareType)conf.getValueIndex("Eyelock.Type", ePC, eNTSGFS, eNANO, "PC", "NANO", "PICO","NTSGLM","NTSGLS","NTSGFM","NTSGFS");

	bool fine = conf.getValue("GRI.enableFineScale",false);
	int scale =1;
	if(fine) scale = 0;
	EyelockLog(logger, DEBUG, "Width %d  Height %d  ",m_width,m_height);
	m_bioInstance = new BiOmega(m_width, m_height,scale);
#ifdef __ANDROID__
	int pupilmin5 = conf.getValue("GRI.minPupilLutValue", 2);
#else
	int pupilmin5 = conf.getValue("GRI.minPupilLutValue", 5);
#endif

	int pupilmax64 = conf.getValue("GRI.maxPupilLutValue", 64);
	int cirmin5 = conf.getValue("GRI.minCircleLutValue", 5);
	int cirmax255 = conf.getValue("GRI.maxCircleLutValue", 255);

	m_bioInstance->SetLUT(pupilmin5,pupilmax64,cirmin5,cirmax255);

	bool eyelidseg = conf.getValue("GRI.enableEyelidSegmentation", true);
	m_bioInstance->SetEnableEyelidSegmentation(eyelidseg);

	//set Upper eyelid center and radius
	CvPoint depPt ;
	float defRad ;
	depPt.x = conf.getValue("GRI.upperEyelidCenterX", 360);
	depPt.y = conf.getValue("GRI.upperEyelidCenterY", 190);
	defRad = conf.getValue("GRI.upperEyelidRadius", 140.0f);
	m_bioInstance->SetUpperEyelidCenterandRadius(depPt,defRad);

	depPt.x = conf.getValue("GRI.lowerEyelidCenterX", 130);
	depPt.y = conf.getValue("GRI.lowerEyelidCenterY", 190);
	defRad = conf.getValue("GRI.lowerEyelidRadius", 135.0f);
	m_bioInstance->SetLowerEyelidCenterandRadius(depPt,defRad);

	bool bEnableF2F = conf.getValue("GRITrigger.F2FEnable",false);
	bool bEnableWeigand = conf.getValue("GRITrigger.WeigandEnable",false) || conf.getValue("GRITrigger.PACEnable",false) || conf.getValue("GRITrigger.WeigandHIDEnable",false);

//	if(bEnableF2F||bEnableWeigand){
//		int bytecnt = m_IrisDBHeader->GetF2FSize();
//		if((bytecnt >F2FKEY_MAX_SIZE)){
//			printf("ERROR in F2FNumBytes configuration\n");
//			throw("ERROR in F2FNumBytes configuration\n");
//		}
//	}

	// Pupil Min and Max
	int minPupilDiameter = conf.getValue("GRI.minPupilDiameter", 16);
	int maxPupilDiameter = conf.getValue("GRI.maxPupilDiameter", 85);
	m_bioInstance->SetPupilRadiusSearchRange(minPupilDiameter, maxPupilDiameter);

	//Iris Min and Max
	int minIrisDiameter = conf.getValue("GRI.minIrisDiameter", 70);
	int maxIrisDiameter = conf.getValue("GRI.maxIrisDiameter", 145);
	m_bioInstance->SetIrisRadiusSearchRange(minIrisDiameter, maxIrisDiameter);

	int minPupilAngle = conf.getValue("GRI.minPupilAngle", -60);
	int maxPupilAngle = conf.getValue("GRI.maxPupilAngle",  90);
	m_bioInstance->SetPupilAngleSearchRange(minPupilAngle, maxPupilAngle);
	EyelockLog(logger, DEBUG, "Pupil Angle Search Range ( %d, %d) ",minPupilAngle, maxPupilAngle);

	//Search Area ROI
	CvRect searchArea;
	m_bioInstance->GetEyeLocationSearchArea(searchArea.x, searchArea.y,
			searchArea.width, searchArea.height);
	searchArea.x = conf.getValue("GRI.EyeLocationSearchArea.x", searchArea.x);
	searchArea.y = conf.getValue("GRI.EyeLocationSearchArea.y", searchArea.y);
	searchArea.width = conf.getValue("GRI.EyeLocationSearchArea.width",
			searchArea.width);
	searchArea.height = conf.getValue("GRI.EyeLocationSearchArea.height",
			searchArea.height);
	bool fail = m_bioInstance->SetEyeLocationSearchArea(searchArea.x,
			searchArea.y, searchArea.width, searchArea.height);
	if (!fail) {
		EyelockLog(logger, WARN, "\nWARNING :Input given for EyeLocationSearchArea is invalid. ");
	}

	// max corrupt bits percentage
	m_bioInstance->SetMaxCorruptBitsPercAllowed(
			conf.getValue("GRI.MaxCorruptBitPercentage"
					,m_bioInstance->GetMaxCorruptBitsPercAllowed()));

	int msgSize = HTTPPOSTMsg::calcMessageSize(m_width, m_height);
	m_scoreThresh = conf.getValue("GRI.matchScoreThresh", 0.13f);
	unsigned int maskcode = conf.getValue("GRI.MatcherFeatureMask",255);
	m_maskval = (maskcode<<24)|(maskcode<<16)|(maskcode<<8)|(maskcode);

	m_inQueue(inQSize);
	for (int i = 0; i < inQSize; i++) {
		m_inQueue[i] = Safe<HTTPPOSTMsg *> (new HTTPPOSTMsg(msgSize));
	}
	m_HTTPMsg = new HTTPPOSTMsg(msgSize);

	m_queueFullBehaviour = DROP;

	// scaling related initialization
	m_scaleDest = cvCreateImage(cvSize(m_width, m_height), IPL_DEPTH_8U, 1);
	m_saveScaledImage = conf.getValue("GRI.SaveScaledImage", false);

	m_expectedIrisWidth = conf.getValue("GRI.ExpectedIrisWidth", 200);
	m_actualIrisWidth = conf.getValue("GRI.ActualIrisWidth", 130);

	resizeType = (ResizeType) conf.getValueIndex("GRI.ResizeType", RESIZE_NONE,
			RESIZE_ASM_2x, RESIZE_OPENCV, "None", "OpenCV", "ASM_2x");

	m_TimeofLastAuthorization = 0;
	m_RepeatAuthorizationPeriod = 1000 * conf.getValue(
			"GRI.RepeatAuthorizationPeriod", 2000);//2sec
	m_LastAuthorizationID = -1;

	if(m_Debug) EyelockLog(logger, DEBUG, "Repeat Authorization Time %lld usec ",m_RepeatAuthorizationPeriod);
	int featueVariance = conf.getValue("GRI.FVEnable",0);

	if(featueVariance){
		float fVTh = conf.getValue("GRI.FVThreshold",100000000.0f);
		unsigned long fVTimeout = 1000 * conf.getValue("GRI.FVTimeoutMilliSec", 1000);//2sec
		int val = m_RepeatAuthorizationPeriod/1000;
		unsigned long fVSilentTimeout = 1000 * conf.getValue("GRI.FVSilentTimeoutMilliSec", val);//2sec
		int debug = conf.getValue("GRI.FVDebug",0);
		int indx = conf.getValue("GRI.FVIndex",0);
		if((indx > 7) ||  (indx < 0)){
			EyelockLog(logger, WARN, " Variance index is not in valid range of 0-7 changing it to 0  ");
			indx =0;
		}
		m_fVDetection = new VarianceBasedDetection(fVTh,fVTimeout,indx,fVSilentTimeout,debug);
		if(m_Debug) EyelockLog(logger, DEBUG, "Feature Variance Threshold = %f ",fVTh);
	}

	m_IrisCode = (char*)malloc(m_bioInstance->GetFeatureLength());

	m_flushQueueOnMatch = conf.getValue("GRI.FlushQueueOnMatch", true);
	m_enablelog = conf.getValue("GRI.MatcherLogEnable",false);

	const char *irisCodeDatabaseFile = conf.getValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
	try
	{
		//Try to Read the Data base and get the values from it
		m_PCMatcher = conf.getValue("GRI.PCMatcher",0);
		EyelockLog(logger, DEBUG, "Working with %s  ",m_PCMatcher?"PCMATCHER":"BFIN MATCHER");
	}
	catch(...)
	{
		char szNewCorruptDB[256] = {0};
		strcpy(szNewCorruptDB,irisCodeDatabaseFile);
		strcat(szNewCorruptDB,".corrupt");
		EyelockLog(logger, WARN, "The database is corrupt, renaming to %s  ",szNewCorruptDB);
		rename(irisCodeDatabaseFile,szNewCorruptDB);
		m_PCMatcher = conf.getValue("GRI.PCMatcher",0);
	}

	MatchManagerFactory factory(conf);
	EyelockLog(logger, DEBUG, "Running in %s mode ",m_Master?"MASTER":"NORMAL");
	int typeofmm = m_Master?1:0;
	if(m_softwareType == ePICO) typeofmm=0;
	m_matchManager = factory.Create(conf,m_bioInstance,m_PCMatcher,typeofmm);

	m_Debug = conf.getValue("GRI.MPDebug",true);
/*	if(spoofStrategy==SPOOF_PERM)
		m_spoofDetector= new SpoofDetectorPermute(conf,m_width,m_height,m_matchManager);
	else if (spoofStrategy==SPOOF_PERM_LR)
		m_spoofDetector= new SpoofDetectorPermuteLRMatch(conf,m_width,m_height,m_matchManager);
	else if (spoofStrategy==SPOOF_LINEAR)
		m_spoofDetector= new SpoofDetectorLinear(conf,m_width,m_height,m_matchManager);
*/
	bool twoeyematcher = conf.getValue("GRI.TwoEyeMatcher",false);
	if(!twoeyematcher){
		int spoofStrategy=conf.getValueIndex("GRI.spoofDetectStrategy",SPOOF_NONE, SPOOF_FFT,SPOOF_NONE,"None","Permute","PermuteLR","Linear","FFT");
		m_spoofDetector = new SpoofDetector(conf,m_width,m_height,m_matchManager);
	}

	m_irisQSize = conf.getValue("GRI.outQSize",15);
	m_IrisData = (IrisData**)malloc(sizeof(IrisData*)*m_irisQSize);
	for(int i=0;i<m_irisQSize;i++){
		m_IrisData[i] =  new IrisData;
	}

	m_spoofEnable = conf.getValue("Eyelock.SpoofEnable",false);
	m_irisMinThreshX = conf.getValue("Eyelock.irisMinThreshX",0);
	m_irisMaxThreshX = conf.getValue("Eyelock.irisMaxThreshX",640);
	m_irisMinThreshRad = conf.getValue("Eyelock.irisMinThreshRad",0);
	m_irisMaxThreshRad = conf.getValue("Eyelock.irisMaxThreshRad",400);

 	m_logging = conf.getValue("Eyelock.Logging", false);

 	m_SaveMatchInfo = conf.getValue("Eyelock.SaveMatchInfo", false);
 	m_EyeCrop = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 1);
}

MatchProcessor::~MatchProcessor() {
	for (int i = 0; i < m_inQueue.getSize(); i++) {
		delete m_inQueue[i].get();
	}
	if (m_HTTPMsg)
		delete m_HTTPMsg;
	if (m_bioInstance)
		delete m_bioInstance;
	if (m_scaleDest)
		cvReleaseImage(&m_scaleDest);
	if (m_scaleSrcHeader)
		cvReleaseImageHeader(&m_scaleSrcHeader);
	if(m_scratch){
	#ifdef __BFIN__
		sram_free(m_scratch);
	#else
		free(m_scratch);
	#endif
	}
	if(m_fVDetection)
		delete(m_fVDetection);
	if (m_IrisCode)
		free(m_IrisCode);
	if (m_matchManager)
		delete m_matchManager;
	if(m_spoofDetector)
		delete m_spoofDetector;
	for(int i=0;i<m_irisQSize;i++){
		delete m_IrisData[i];
	}
	if(m_IrisData) free(m_IrisData);
	if(m_EyeCrop)
		cvReleaseImageHeader(&m_EyeCrop);
}

bool MatchProcessor::enqueMsg(Copyable& msg) {
	int queuebehaviour = m_queueFullBehaviour;
	HTTPPOSTMsg & hMsg = (HTTPPOSTMsg &) msg;

	if (hMsg.isReloadDB()) {
		queuebehaviour = OVERWRIE_OLD;
	}

	Safe<HTTPPOSTMsg *> & currMsg = m_inQueue.getCurr();
	bool wrote = false;
	currMsg.lock();
	if (currMsg.isUpdated()) {
		if (queuebehaviour == OVERWRIE_OLD) {
			EyelockLog(logger, WARN, "MatchProcessor::input queue full, over-writing ");
			currMsg.get()->CopyFrom(msg);
			wrote = true;
		} else if (queuebehaviour == DROP) {
			EyelockLog(logger, DEBUG, ".");
		} else {
			throw "queueFullBehaviour:OVERWRIE_NEW not implemented";
			EyelockLog(logger, WARN, "queueFullBehaviour:OVERWRIE_NEW not implemented");
		}
	} else {
		// just normally write the new message
		currMsg.get()->CopyFrom(msg);
		wrote = true;
	}

	if (wrote) {
		gettimeofday(&m_timer, 0);
		TV_AS_USEC(m_timer,currtimestamp);
		if(m_Debug){
			EyelockLog(logger, DEBUG, "Image Time stamp %lu %lu -> %llu  ",m_timer.tv_sec,m_timer.tv_usec,currtimestamp);
		}
		currMsg.get()->SetTime(currtimestamp);

		currMsg.setUpdated(true);

		m_inQueue++;
	}

	currMsg.unlock();

	// if we wrote something lets inform the others who may be interested
	if (wrote) {
		m_inQueue.incrCounter();
		dataAvailable();
	}
	return true;
}

bool MatchProcessor::shouldWait() {
	return m_inQueue.isEmpty();
}
// blocks till it gets a new message
HTTPPOSTMsg *MatchProcessor::getNextMsgToProcess() {

	bool bFound = false;
	while (!bFound && !ShouldIQuit()) {
		// if the system is in bad state loop till recovered
//		if(!m_MatcherHDMStatus){
//			m_matchManager->RecoverFromBadState();
//			FlushQueue(false);
//			m_MatcherHDMStatus = true;
//		}

		Safe<HTTPPOSTMsg *> & currMsg = m_sendIter.curr();
		currMsg.lock();
		if (currMsg.isUpdated()) {
			m_HTTPMsg->CopyFrom(*currMsg.get()); // make a copy
			currMsg.setUpdated(false);//empty
			bFound = true;
		}
		currMsg.unlock();
		if (bFound) {
			m_inQueue.decrCounter();
		} else {
			//RunHDMDiagnostics();
			waitForDataAndTimeOut();
		}
		m_sendIter.next();
	}
	if (bFound)
		return m_HTTPMsg;
	return 0;
}

void MatchProcessor::RunHDMDiagnostics(){
	struct timeval curTime;
	gettimeofday(&curTime, 0);
	int currtimestamp = (curTime.tv_sec);
	if (currtimestamp > m_PingTimeStamp){
		m_matchManager->SaveLogResult(); // Save log file
		m_MatcherHDMStatus = m_matchManager->HDMDiagnostics();
		m_PingTimeStamp = currtimestamp + m_PingInterval;
	}
}

// go over the queue and set image messages update=false
void MatchProcessor::ClearNwMatcherQ(){
	if(m_nwMatchManager)
		m_nwMatchManager->FlushQueue(true);
}

void MatchProcessor::FlushQueue(bool onlyEyes) {
	int flushed = 0;
	bool emptied = false;
	for (int i = 0; i < m_inQueue.getSize(); i++) {
		Safe<HTTPPOSTMsg *> & currMsg = m_inQueue[i];
		emptied = false;
		currMsg.lock();
		if (currMsg.isUpdated()) {
			if ((currMsg.get()->getMsgType() == IMG_MSG)  || (!onlyEyes)) {
				currMsg.setUpdated(false);
				flushed++;
				emptied = true;
			}
		}
		currMsg.unlock();
		if (emptied)
			m_inQueue.decrCounter();
	}
	EyelockLog(logger, DEBUG, "MatchProcessor::Flushed %d Match requests ", flushed);
}

float MatchProcessor::getScaleRatio()
{
	return (m_expectedIrisWidth * 1.0) / m_actualIrisWidth;
}
unsigned char *MatchProcessor::ResizeFrame(int width, int height,
		unsigned char *frame) {
	bool Resized = false;
	float ratio = (m_expectedIrisWidth * 1.0) / m_actualIrisWidth;
	switch (resizeType) {
	case RESIZE_ASM_2x:
		assert(m_width == 2 * width && m_height == 2 * height);
		// use mamigo optimized
		if (m_scratch == 0){
			#ifdef __BFIN__
			m_scratch = (unsigned char *) sram_alloc(6 * m_width, L1_DATA_SRAM);
			#else
			m_scratch = (unsigned char*)(malloc(6 * m_width));
			#endif

		}

		assert(m_scratch != 0);
		assert(((int) frame & 3) == 0);
		dims d;
		d.width = width;
		d.height = height;
		d.output = (unsigned char*) (m_scaleDest->imageData);
#ifndef HBOX_PG
		XTIME_OP("RESIZE_ASM",
				gaus5_expand_reflect(frame, m_scratch, &d)
		)
#endif
		;
		Resized = true;
		break;

	case RESIZE_OPENCV:

		cvSetZero(m_scaleDest);
		if (m_scaleSrcHeader != 0) {
			if (m_scaleSrcHeader->width != width || m_scaleSrcHeader->height
					!= height) {
				delete m_scaleSrcHeader;
				m_scaleSrcHeader = 0;
			}
		}
		if (m_scaleSrcHeader == 0)
			m_scaleSrcHeader = cvCreateImageHeader(cvSize(width, height),
					IPL_DEPTH_8U, 1);

		CvRect scaleROI;
		scaleROI.width = width * ratio;
		scaleROI.height = height * ratio;
		scaleROI.x = (m_width - scaleROI.width) >> 1;
		scaleROI.y = (m_height - scaleROI.height) >> 1;
		cvSetImageROI(m_scaleDest, scaleROI);
		//		printf("dest(%d,%d,%d,%d)\n",m_scaleSrcROI.x,m_scaleSrcROI.y,m_scaleSrcROI.width,m_scaleSrcROI.height);

		scaleROI.width = m_width / ratio;
		scaleROI.height = m_height / ratio;
		scaleROI.x = (width - scaleROI.width) >> 1;
		scaleROI.y = (height - scaleROI.height) >> 1;
		cvSetImageROI(m_scaleSrcHeader, scaleROI);
		//		printf("scr(%d,%d,%d,%d)\n",m_scaleSrcROI.x,m_scaleSrcROI.y,m_scaleSrcROI.width,m_scaleSrcROI.height);

		cvSetData(m_scaleSrcHeader, frame, m_scaleSrcHeader->width);
		XTIME_OP("cvResize",
		cvResize(m_scaleSrcHeader, m_scaleDest)
		);
		Resized = true;

		break;
	default:
		break;
	}

	if (Resized) {

		frame = (unsigned char *) (m_scaleDest->imageData);

		if (m_saveScaledImage) {
			static int rIndex = 0;
			rIndex++;
			SAVE_NAME_INDX(m_scaleDest,"scaled",rIndex%3);
		}
	}

	return frame;
}
void MatchProcessor::MatchDetected(int indx,float res,uint64_t timestamp)
{
    cout << "xxBest matched ::" << timestamp << " :: with record " << indx << " with score " << res << endl;
    int personId = indx >> 1;
    uint64_t currtimestamp = timestamp;
    int64_t elapsedTime = currtimestamp - m_TimeofLastAuthorization;
	if ((elapsedTime > m_RepeatAuthorizationPeriod)
			|| (personId!= m_LastAuthorizationID))
	{
  		m_LastAuthorizationID = personId;
		m_TimeofLastAuthorization = currtimestamp;
		mr.CopyFrom(*(m_matchManager->GetResult()));
		mr.setState(PASSED);
		mr.setTimeStamp(timestamp);

		callNext(mr);
		if(m_flushQueueOnMatch)
			FlushQueue(true);
	}
}

void MatchProcessor::UpdateDB(char* dbptr,char*keyptr){
	//BUFFER BASED UPDQATE : Reqd for ANDROID
#ifdef MADHAV
	m_IrisDBHeader->ReadHeader(dbptr,keyptr);
#endif
	//m_matchManager->AssignDB(dbptr,1);
}

void MatchProcessor::ReloadDB(){
	//todo: bookmark RELOAD DB
#ifdef MADHAV
	m_IrisDBHeader->ReadHeader((char *)m_matchManager->GetIrisCodeDatabaseFile());
	m_IrisDBHeader->PrintAll();
	m_matchManager->InitHeader(m_IrisDBHeader);
	m_matchManager->AssignDB();
#endif

}

void MatchProcessor::process(HTTPPOSTMsg *msg) {
	if(m_Debug) EyelockLog(logger, DEBUG, "xMSG TYPE %d ", msg->getMsgType());

	bool matched = false;
	if(CheckMessage(msg))
		return ;

	try {
#ifdef MADHAV
		int numsec = (1500000+2*m_IrisDBHeader->GetNumRecord()*1000)/1000000;
		setHealthy(false,numsec);
#endif
		unsigned char *frame = (unsigned char *) ((msg->getImage()));
		int width = 0;
		int height = 0;
		if (!msg->getImageDims(width, height)) {
			throw "could not determine Image dims";
		}
	    if(m_matchManager->GetLogEnable()){
	    	m_matchManager->ExtractLogData(msg);
	    }

		XTIME_OP("Resize",
			frame = ResizeFrame(width, height, frame)
			);

		mr.setState(FAILED);
		mr.setVar(-1);

		float varience[8] ={0};

		XTIME_OP("SPOOF",
				matched = m_spoofDetector->process(msg, frame, m_bioInstance,varience,m_Master?0:1)
		);
		if(m_Master){
			BinMessage msg1(m_spoofDetector->getIris(),2560);
			if(m_nwMatchManager)
				m_nwMatchManager->enqueMsg(msg1);
		}

		if(m_Debug){
			for(int i=0;i<8;i++)
				printf("%f, ",varience[i]);
			printf("\n");
		}

		if(m_fVDetection){
			bool fvresult = m_fVDetection->Process(m_spoofDetector->getMatchTimeStamp(),matched,varience);
			if(fvresult){
				mr.setVar(m_fVDetection->GetVarianceMaxValue());
				mr.setTimeStamp(m_spoofDetector->getMatchTimeStamp());
				if(m_Debug) EyelockLog(logger, DEBUG, "Dispatch variance based Detection ");
				if(m_nwDispatcher)
					m_nwDispatcher->enqueMsg(mr);
			}
		}

		if(matched){
			MatchDetected(m_spoofDetector->getDBIndex(),m_spoofDetector->getMatchScore(),m_spoofDetector->getMatchTimeStamp());
		}else{
			//its possible we got here due to bad state.
			// Remember the status of all matchers
			m_MatcherHDMStatus=m_matchManager->CheckValidityOfHDM();
		}

	} catch (const char *msg) {
		cout << msg << endl;
	} catch (exception ex) {
		cout << ex.what() << endl;
	}
}

void draw1( IplImage* img1, CvPoint3D32f pt, CvScalar color )
{
	CvPoint center = {(int)(pt.x), (int)(pt.y)};
	int radius = (int)(pt.z);
	cvCircle( img1, center, radius, color, 1, 8, 0 );
}
IrisData * MatchProcessor::SegmentEye(HTTPPOSTMsg *msg,float *variance){
#ifdef MADHAV
	int numsec = (1500000+2*m_IrisDBHeader->GetNumRecord()*1000)/1000000;
	setHealthy(false,numsec);
#endif

	unsigned char *frame = (unsigned char *) ((msg->getImage()));
	int width = 0;
	int height = 0;
	if (!msg->getImageDims(width, height)) {
		throw "could not determine Image dims";
	}

	XTIME_OP("Resize",
		frame = ResizeFrame(width, height, frame)
		);
	mr.setState(FAILED);
	mr.setVar(-1);

	bool iris = m_spoofDetector->GetIris(msg->GetTime(),frame,variance);
// Try to get IrisData
	IrisData *irisData = m_spoofDetector->GetIrisData();

	if(m_SaveMatchInfo)
	{
		char filename[100];
		CvPoint3D32f ip = irisData->getIrisCircle();
		CvPoint3D32f pp = irisData->getPupilCircle();
		sprintf(filename,"EyeCrop_%d.pgm",msg->getFrameIndex());
		cvSetData(m_EyeCrop, (unsigned char*)frame, m_EyeCrop->width);
		CvScalar color = cvRealScalar(255);
		draw1( m_EyeCrop, pp, color );
		draw1( m_EyeCrop, ip, color );
		cv::Mat mateye = cv::cvarrToMat(m_EyeCrop);
		imwrite(filename, mateye);
	}
	return irisData;
}

void MatchProcessor::UpdateIrisData(HTTPPOSTMsg *msg,IrisData *irisData){
	float irx=0.0,iry=0.0;
	bool ret1 = msg->getIrx(irx);
	ret1 = msg->getIry(iry);
	irisData->setSpecCentroid(irx,iry);

	float blc = -1.0;
	ret1 = msg->getBLC(blc);
	irisData->setBLC(blc);

	float ascore = -1.0;
	ret1 = msg->getaScore(ascore);
	irisData->setAreaScore(ascore);

	float fscore = -1.0;
	ret1 = msg->getfScore(fscore);
	irisData->setFocusScore(fscore);

	float halo = -1.0;
	ret1 = msg->getHalo(halo);
	irisData->setHalo(halo);

	irisData->setFrameIndex(msg->getFrameIndex());
	irisData->setEyeIndex(msg->getEyeIndex());
	irisData->setIlluminatorState(msg->getIlluminator());

	uint64_t ts = 0;
	ret1 = msg->getTimeStamp(ts);
	irisData->setTimeStamp(ts);

	int prev = -1;
	ret1 = msg->getPrevIndex(prev);
	irisData->setPrevIndex(prev);
	if(m_logging){
		struct timeval m_timer;
		gettimeofday(&m_timer, 0);
		TV_AS_USEC(m_timer,a);
		FILE *fp = fopen("dump.txt","a");
		int le =0;
		CvPoint3D32f ir = irisData->getIrisCircle();
		CvPoint3D32f pp= irisData->getPupilCircle();
		float ascore=0,fscore=0,blc=0;
		msg->getfScore(fscore);
		msg->getaScore(ascore);
		msg->getBLC(blc);
//			printf("EYEDPASS;%llu;%f;%f;%f;%f;%f;%f;%d;%d;%f;%f;%f;%f;%f;\n",a,ir.x,ir.y,ir.z,pp.x,pp.y,pp.z,msg->getFrameIndex(),msg->getEyeIndex(),irx,iry,ascore,fscore,blc);
		fprintf(fp,"EYEDPASS;%llu;%f;%f;%f;%f;%f;%f;%d;%d;%f;%f;%f;%f;%f;%d;%d;%d; ",a,ir.x,ir.y,ir.z,pp.x,pp.y,pp.z,msg->getFrameIndex(),msg->getEyeIndex(),irx,iry,ascore,fscore,blc,prev,irisData->getSegmentation()?1:0,msg->getIlluminator());
		fclose(fp);
	 }
}

bool MatchProcessor::CheckIrisFromSameFrame(){
	bool spoof= false;
	try {
		for(int i=0;i < m_IrisDataIndex && (!spoof) ;i++){
			for(int j=i+1;j<m_IrisDataIndex && (!spoof) ;j++){
				if(m_IrisData[i]->getSegmentation() && m_IrisData[j]->getSegmentation()){
					std::pair<int, float> res = m_bioInstance->MatchIrisCodeSingle((char*)(m_IrisData[i]->getIris()),(char*)(m_IrisData[j]->getIris()),m_maskval);
					//printf("%d %d %lf < %lf %d %d \n",m_IrisData[i]->getFrameIndex(),m_IrisData[i]->getEyeIndex(),res.second,m_scoreThresh,m_IrisData[j]->getFrameIndex(),m_IrisData[j]->getEyeIndex());
					if( res.second < m_scoreThresh){
						spoof = true;
						EyelockLog(logger, DEBUG, "Spoof Spoof Spoof on account of Iris from same Frame ");
					}
				}
			}
		}
	} catch (const char *msg) {
		cout << msg << endl;
	} catch (exception ex) {
		cout << ex.what() << endl;
	}
	return spoof;
}

void MatchProcessor::SendIrisFromSameFrame(){
	float var[8]={0};
	for(int i=0;i < m_IrisDataIndex;i++){
		if(m_IrisData[i]->getSegmentation()){
			SendForMatching(m_IrisData[i],var);
		}
	}
	m_IrisDataIndex = 0;
}

bool MatchProcessor::SendForMatching(IrisData *irisData, float* varience){
	bool matched = false;
    NwMatcherSerialzer ns;
    int ret = ns.GetSizeOfNwMsg(irisData);
    BinMessage msg1(ret);
    ns.MakeNwMsg(msg1.GetBuffer(),irisData);
    if(m_Master&&(m_softwareType!=ePICO)){
        if(m_nwMatchManager){
        	//printf("MatchProcessor::SendForMatching %d %d %d\n",irisData->getFrameIndex(),irisData->getEyeIndex(),irisData->getIrisRadiusCheck()?1:0);
            m_nwMatchManager->enqueMsg(msg1);
        }
    }else{
        matched = m_spoofDetector->DoMatch(&msg1);
        if(m_fVDetection){
            bool fvresult = m_fVDetection->Process(irisData->getTimeStamp(), matched, varience);
            if(fvresult){
                mr.setVar(m_fVDetection->GetVarianceMaxValue());
                mr.setTimeStamp(irisData->getTimeStamp());
                if(m_Debug)
                    EyelockLog(logger, DEBUG, "Dispatch variance based Detection ");

                if(m_nwDispatcher)
                    m_nwDispatcher->enqueMsg(mr);
            }
        }
		if(matched)
			MatchDetected(m_spoofDetector->getDBIndex(),m_spoofDetector->getMatchScore(),m_spoofDetector->getMatchTimeStamp());
    }
    if(!matched)
        m_MatcherHDMStatus = m_matchManager->CheckValidityOfHDM();
    return matched;
}

void MatchProcessor::CheckSegmentationThresholds(IrisData *irisData){
	bool segmentation = irisData->getSegmentation();
	if(segmentation){
		CvPoint3D32f ip = irisData->getIrisCircle();
//		printf("(%f > %d && %f < %d) && (%f > %d && %f < %d)",ip.x,m_irisMinThreshX,ip.x,m_irisMaxThreshX,ip.z , m_irisMinThreshRad, ip.z , m_irisMaxThreshRad);
		if(!((ip.x > m_irisMinThreshX && ip.x < m_irisMaxThreshX) && (ip.z > m_irisMinThreshRad && ip.z < m_irisMaxThreshRad)))	{
			if(m_logging){
				struct timeval m_timer;
				gettimeofday(&m_timer, 0);
				TV_AS_USEC(m_timer,a);
				FILE *fp = fopen("dump.txt","a");
				fprintf(fp,"EYETAGGED;%llu;%s;%d;%d;%f;%f;%f;\n",a,irisData->getCamID(),irisData->getFrameIndex(),irisData->getEyeIndex(),ip.x,ip.z,irisData->getSpecCentroid().x);
				fclose(fp);
			}
			irisData->setIrisRadiusCheck(false);
		}
	}
}



void MatchProcessor::DoSegmentation(HTTPPOSTMsg *msg){
	try {
		float var[8]={0};
		IrisData *irisData = SegmentEye(msg,var);
		UpdateIrisData(msg,irisData);
		if(m_spoofEnable)
			CheckSegmentationThresholds(irisData);
		*m_IrisData[m_IrisDataIndex++] = *irisData;
	} catch (const char *msg) {
		cout << msg << endl;
	} catch (exception ex) {
		cout << ex.what() << endl;
	}
}



bool MatchProcessor::CheckMessage(HTTPPOSTMsg *msg){
	if (msg->isReloadDB()&&(m_PCMatcher==0)) {
		if(m_ledDispatcher)m_ledDispatcher->SetDBUploadState();
		if(m_f2fDispatcher)m_f2fDispatcher->CloseDispatcher();
		ReloadDB();
		if(m_ledDispatcher)m_ledDispatcher->SetInitialState();
		if(m_f2fDispatcher)m_f2fDispatcher->RestartDispatcher();
		return true;
	}

	if(m_PCMatcher == 0){
		if(msg->getMsgType()==IMG_MSG){
			m_matchManager->PrintAllHDMatcher();
			return true;
		}
	}

	if(msg->getMsgType() != IMG_MSG){
		EyelockLog(logger, DEBUG, "Other than image msg on MP ");
		return true;
	}
	return false;
}

bool MatchProcessor::DoMatch(HTTPPOSTMsg *msg) {
	if(m_Debug) EyelockLog(logger, DEBUG, "MSG TYPE %d ", msg->getMsgType());
	bool matched = false;
	if(CheckMessage(msg))
		return false;

	try {
// Try to get IrisData
		float varience[8] ={0};
		IrisData *irisData = SegmentEye(msg,varience);
		UpdateIrisData(msg,irisData);
		matched = SendForMatching(irisData,varience);

	} catch (const char *msg) {
		cout << msg << endl;
	} catch (exception ex) {
		cout << ex.what() << endl;
	}
	return matched;
}

void MatchProcessor::Matched(){
	MatchDetected(m_spoofDetector->getDBIndex(),m_spoofDetector->getMatchScore(),m_spoofDetector->getMatchTimeStamp());
}

unsigned int MatchProcessor::MainLoop() {

	std::string name = "MatchProcessor::";
	int MatchProcLine = 0;
	try {

		while (!ShouldIQuit()) {
			cout << "Is the match processor blocking? "<< MatchProcLine << " changes if not."<<endl;
			HTTPPOSTMsg *msg = getNextMsgToProcess();
			//throw "This isn't used is it?";
			if (msg)
				process(msg);
			Frequency();
			MatchProcLine++;

		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}

	return 0;
}

bool MatchProcessor::MatchStatus(){
	bool ret = true;
	if(mr.getState() != PASSED)
		ret = false;
	return ret;
}

int MatchProcessor::AppendDB(char* eye1,char* eye2,char* name,char* dbfile){
//	DBWriter test;
//	return test.AppendDB(eye1,eye2,name,dbfile);
}

