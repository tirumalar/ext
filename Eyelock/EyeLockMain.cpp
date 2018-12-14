//#define EYELOCK_VERSION "5.00.XXX.UN-OFFICIAL"
#define EYELOCK_VERSION "5.00.0000"

/*
 * Main.h
 *
 *  Created on: 28-Sep-2010
 *      Author: madhav.shanbhag@mamigo.us
 */
#include <fcntl.h>
#include "EyeLockMain.h"
#include "eyelock_com.h"

#ifdef __ARM__
#define __SOUND__
#else
#define __SOUND__
#endif



#ifdef __SOUND__
#include "AudioDispatcher.h"
#include "TonePlayer.h"
#include "SpeakServer.h"
#include "SPIBus.h"
#define HAS_SPI_WDT
#endif

#include "Aquisition.h"
#include "MT9P001FrameGrabber.h"
#include "LiquidLens.h"
#include "LoiteringDetector.h"
#include "LEDDispatcher.h"
#include "LEDConsolidator.h"
#include "NwLEDDispatcher.h"
#include "AESClass.h"
#include "DBReceive.h"
#include "logging.h"
#include "I2CBus.h"
#include "UtilityFunctions.h"
#define __NW_THREAD__


int EyeLockMain::m_watchdogfd=-1;
struct sigaction EyeLockMain::sa;
struct iniParameter_t {
	char dc[10];
	char shiftRight[10];
	char EnableDiffIllumination[10];
	char LeftIlluminationValue[10];
	char RightIlluminationValue[10];
	char global_gain_val[10];
} intCalib;

extern bool DoTamper1(void);
extern void *init_facetracking(void * arg);

enum LEDType { MiniLED, NanoLED, EyelockLED, PicoLED, NoLED }; // TODO: Make LEDControllerFctory for thisvoid
const char logger[30] = "EyelockMain";

EyeLockMain::EyeLockMain(char* filename):conf(filename),nwListener(conf),pMatchProcessor(0),pNwDispatcher(0),pImageProcessor(0),
			   pF2FDispatcher(0),pDBReceive(0),pledDispatcher(0),pEyeDispatcher(0),pNwMatchManager(0),m_SendLed(true),m_curval(0),
			   m_outMsg(256),m_FuturisticTime(0),m_svrAddr(0),m_Debug(false),m_Master(false),m_Slave(false),pEyelockNanoSdkThread(NULL),
			   pAudioDispatcher(0), m_pLiquidLens(0),pLoiteringDetector(0),m_matchDispatcher(0),pledConsolidator(0),pnwLEDDispatcher(0),
			   pSDKDispatcher(NULL),pMasterSlaveNwListener(NULL),pCMXHandle(NULL){

	void EyelockLogInit();
	EyelockLog(logger, DEBUG, "EyeLockMain() Start");

	m_KeepAlive.lock();
	m_KeepAlive.set(true);
	m_KeepAlive.unlock();
	m_softwareType = (SoftwareType)conf.getValueIndex("Eyelock.Type", ePC, eNTSGFS, eNANO, "PC", "NANO", "PICO","NTSGLM","NTSGLS","NTSGFM","NTSGFS");

	// update calibration parameters
	if (strlen(intCalib.dc) && strcmp(intCalib.dc, conf.getValue("GRI.dc",""))) {
		conf.setValue("GRI.dc",intCalib.dc);
	}
	if (strlen(intCalib.shiftRight) && strcmp(intCalib.shiftRight, conf.getValue("GRI.shiftRight",""))) {
		conf.setValue("GRI.shiftRight",intCalib.shiftRight);
	}
	if (strlen(intCalib.EnableDiffIllumination) && strcmp(intCalib.EnableDiffIllumination, conf.getValue("Eyelock.EnableDiffIllumination",""))) {
		conf.setValue("Eyelock.EnableDiffIllumination",intCalib.EnableDiffIllumination);
	}
	if (strlen(intCalib.LeftIlluminationValue) && strcmp(intCalib.LeftIlluminationValue, conf.getValue("Eyelock.LeftIlluminationValue",""))) {
		conf.setValue("Eyelock.LeftIlluminationValue",intCalib.LeftIlluminationValue);
	}
	if (strlen(intCalib.RightIlluminationValue) && strcmp(intCalib.RightIlluminationValue, conf.getValue("Eyelock.RightIlluminationValue",""))) {
		conf.setValue("Eyelock.RightIlluminationValue",intCalib.RightIlluminationValue);
	}
	if (strlen(intCalib.global_gain_val) && strcmp(intCalib.global_gain_val, conf.getValue("MT9P001.global_gain_val",""))) {
		conf.setValue("MT9P001.global_gain_val",intCalib.global_gain_val);
	}

	m_pRGBController = 0;
	m_Debug = conf.getValue("GRI.HBDebug",false);
	m_svrAddr = (char*)conf.getValue("GRI.StatusDestAddr","NONE");
	if(strcmp(m_svrAddr,"NONE")==0){
		m_svrAddr =0;
	}
	if(m_svrAddr){
		m_statusDestAddr=new HostAddress(m_svrAddr);
		m_msgFormat=conf.getValue("GRI.StatusNwMsgFormat","Alive");
		m_HBFreqSec=conf.getValue("GRI.HBFreqInSec",5);
		m_HBTimeOutmilliSec=conf.getValue("GRI.HBTimeoutInmilliSec",2000);

		//Now fill the bin msg
		char *msgBuff=m_outMsg.GetBuffer();
		int len=snprintf(msgBuff,m_outMsg.GetAvailable(),m_msgFormat);
		m_outMsg.SetSize(len);
	}
	else
	{
		EyelockLog(logger, INFO, "Status destination address not specified");
	}

	m_Master = conf.getValue("GRI.EyelockMaster",false);
	m_Slave = conf.getValue("GRI.EyelockSlave",false);
	// initialize strings for HTTPPostMsg here
	const char *hbStr=conf.getValue("GRI.HeartBeatStr","HEARTBEAT");
	const char *ledStr=conf.getValue("GRI.LEDSetStr","LEDSET");
	const char *reloadStr=conf.getValue("GRI.ReloadDBStr","RELOADDB");
	const char *recStr=conf.getValue("GRI.ReloadDBStr","RECEIVEDB");
	const char *recEnrollStr=conf.getValue("GRI.DoEnrollStr","DOENROLL");
	const char *faceGrabStr=conf.getValue("GRI.FaceGrabStr","FACEGRAB");
	const char *motionStr=conf.getValue("GRI.MotionStr","MOTION");
	const char *f2fStr=conf.getValue("GRI.F2FStr","F2F");
	const char *setPinStr=conf.getValue("GRI.SetPinStr","SETPIN");
	const char *matchStr=conf.getValue("GRI.MatchStr","MATCH");
	const char *pingStr=conf.getValue("GRI.PingStr","PING");
	const char *resetStr=conf.getValue("GRI.ResetStr","RESETEYELOCK");
	const char *threadPropStr=conf.getValue("GRI.ThreadPropStr","THREADDISPLAY");
	const char *downloadDBStr=conf.getValue("Eyelock.DownloadDB","DOWNLOADDB");
	const char *sqliteStr=conf.getValue("Eyelock.sqliteDB","RECEIVESQLITE");

	HTTPPOSTMsg::init(hbStr,ledStr,reloadStr,recStr,recEnrollStr,faceGrabStr,motionStr,f2fStr,setPinStr,matchStr,pingStr,resetStr,threadPropStr,sqliteStr,downloadDBStr);

	pImageProcessor= new EyeLockThread(conf);
	pImageProcessor->init();

	nwListener.m_imageProcessor=pImageProcessor;
	nwListener.m_frameGrabber =  dynamic_cast<MT9P001FrameGrabber*>(pImageProcessor->m_ImageProcessor->GetFrameGrabber());
	nwListener.m_pIp = pImageProcessor->m_ImageProcessor;

	//Start the NW HD Listner
	bool enableHD = conf.getValue("GRI.EnableNWHDMatcher",false);
	if(enableHD){
		startHDListener();
	}

	m_svrAddr = (char*)conf.getValue("GRI.StatusDestAddr","NONE");
	if(strcmp(m_svrAddr,"NONE")==0){
		m_svrAddr =0;
	}
	// We will always need an EyeDispatcher in order to service dynamic enrollment requests
	//char *dstAddr = (char*)conf.getValue("GRI.EyeDestAddr","NONE");
	//if(strcmp(dstAddr,"NONE")!=0)
	{
		pEyeDispatcher = new EyeDispatcher(conf);
		pEyeDispatcher->init();
	}

	bool usematcher = conf.getValue("GRI.UseMatcher",true);
	if(usematcher){
		pMatchProcessor = new MatchProcessor(conf);

#ifdef __NW_THREAD__
		// Nw Dispatcher runs only if MatchProcessor runs
		if(strlen(conf.getValue("GRI.MatchResultDestAddr",""))>0){
			pNwDispatcher=new NwDispatcher(conf);
			pNwDispatcher->init();
		}
		pDBReceive=new DBReceive(conf);
		if(pDBReceive)
		{
			pDBReceive->init();
		}
#endif
	}

	// Both master and slave need message handler
	nwListener.AddMessageHandler(pImageProcessor->GetEyelockModeMessageHandler());
	pImageProcessor->SetNetworkListener(&nwListener);

	pnwLEDDispatcher = new NwLEDDispatcher(conf);
	pnwLEDDispatcher->init();


	if(m_Master){
#ifdef CMX_C1
		//Allocate our Facetracking <--->  Eyelock message queue
		AllocateOIMQueue(10); // 10 is queue sizee.  Can be replaced with config value later if necessary
		AllocateFaceQueue(10);

		// Start facetracking threads
		startFaceTracking();

		// Create Cmx handler
		pCMXHandle = new CmxHandler(conf);
		if (pCMXHandle)
		{
			pCMXHandle->SetImageProcessor(pImageProcessor->m_ImageProcessor);
			pCMXHandle->SetFaceTrackingQueue(g_pOIMQueue); //DMO configure facetracking queue
		}
#endif

		m_SendLed=conf.getValue("GRI.LED.SendResult",true);

		bool bEnableF2F = conf.getValue("GRITrigger.F2FEnable",false);
		bool bEnableWeigand = conf.getValue("GRITrigger.WeigandEnable",false) || conf.getValue("GRITrigger.PACEnable",false) || conf.getValue("GRITrigger.WeigandHIDEnable",false);;
		bool bEnableRelay = conf.getValue("GRITrigger.RelayEnable",false);

		if(	bEnableF2F||bEnableWeigand||bEnableRelay){
			if(	bEnableF2F&&bEnableWeigand){
				EyelockLog(logger, ERROR, "F2F and Weigand both are enabled, Can't proceed");
				throw("Invalid Configuration");
			}
			pF2FDispatcher = new F2FDispatcher(conf);
			pF2FDispatcher->init();

			if(pDBReceive){
				pDBReceive->SetF2FDispatcher(pF2FDispatcher);
			}
			if (pNwDispatcher)
				pF2FDispatcher->addProcessor(pNwDispatcher);
				// pF2FDispatcher->SetNwDispatcher(pNwDispatcher);
		}

		if(NULL == m_matchDispatcher){
			m_matchDispatcher = new MatchDispatcher(conf);
			m_matchDispatcher->init();
			m_matchDispatcher->SetF2FDispatcher(pF2FDispatcher);
			m_matchDispatcher->SetNwLEDDispatcher(pnwLEDDispatcher);
			m_matchDispatcher->SetEyelockThread(pImageProcessor);
		}

		if(pMatchProcessor&&(m_softwareType!=ePICO)){
			pNwMatchManager = new NwMatchManager(conf);
			pNwMatchManager->init();
		}
		nwListener.m_nwMatchManager = pNwMatchManager;

		pMasterSlaveNwListener = new MasterSlaveNwListner(conf);
		pMasterSlaveNwListener->m_imageProcessor = pImageProcessor;
		pMasterSlaveNwListener->m_nwMatchManager = pNwMatchManager;
		pMasterSlaveNwListener->m_pEyeDispatcher = pEyeDispatcher;

		if(pNwMatchManager){
			pNwMatchManager->SetEyelockThread(pImageProcessor);
			pNwMatchManager->SetF2FDispatcher(pF2FDispatcher);
			pNwMatchManager->addProcessor(m_matchDispatcher);
		}
		m_matchDispatcher->SetNwMatchMgr(pNwMatchManager);

		bool loitering = conf.getValue("Eyelock.EnableLoiteringDetector",false);
		if(loitering){
			pLoiteringDetector = new LoiteringDetector(conf);
			if(pNwMatchManager)pNwMatchManager->SetLoitering(pLoiteringDetector);
			pLoiteringDetector->SetNwLedDispatcher(pnwLEDDispatcher);
		}

		if(pMatchProcessor)
			pMatchProcessor->SetNwMatchManager(pNwMatchManager);


		// { MiniLED, NanoLED, EyelockLED, NoLED }
		int ledType=conf.getValueIndex("GRI.LEDType", (int)MiniLED, (int)NoLED, (int)NanoLED, "MiniLED", "NanoLED", "EyelockLED","PicoLED", "NoLED" ); /* DJH NEW */
		if(ledType != NoLED){

			int brightness = conf.getValue("GRI.LEDBrightness", 80); /* DJH NEW */
			int brightnessR = brightness * 8/10; // conf.getValue("GRI.LEDBrightnessR", 65);
			int brightnessG = brightness; // conf.getValue("GRI.LEDBrightnessG", 80);
			int brightnessB = brightness; // conf.getValue("GRI.LEDBrightnessB", 80);

            FileConfiguration calib("/etc/Calibration.ini");

            int left_ir_addressoff_pwm          = calib.getValue("Eyelock.LeftIrPWMAddress",0);
            int left_ir_pwm_val                 = calib.getValue("Eyelock.LeftIrPWMValueNormal",128);
            int left_ir_addressoff_pwm_dur      = calib.getValue("Eyelock.LeftIrPulseWidthAddress",0);
            int left_ir_pwm_dur_val             = calib.getValue("Eyelock.LeftIrPulseWidthValue",128);

            int right_ir_addressoff_pwm         = calib.getValue("Eyelock.RightIrPWMAddress",0);
            int right_ir_pwm_val                = calib.getValue("Eyelock.RightIrPWMValueNormal",128);
            int right_ir_addressoff_pwm_dur     = calib.getValue("Eyelock.RightIrPulseWidthAddress",0);
            int right_ir_pwm_dur_val            = calib.getValue("Eyelock.RightIrPulseWidthValue",128);

			EyelockLog(logger, INFO, "BrightnessR = %d, BrightnessG = %d, BrightnessB = %d ", brightnessR, brightnessG, brightnessB);

			switch(ledType)
			{
			case NanoLED:
			{
				MT9P001FrameGrabber *pFrameGrabber = dynamic_cast<MT9P001FrameGrabber*>(pImageProcessor->m_ImageProcessor->GetFrameGrabber());
				m_pRGBController = new RGBControllerNano(pFrameGrabber, brightnessR, brightnessG, brightnessB);
				if(pFrameGrabber){
					pFrameGrabber->SetPWM(left_ir_addressoff_pwm,left_ir_pwm_val);
					pFrameGrabber->SetPWM(left_ir_addressoff_pwm_dur,left_ir_pwm_dur_val);
					pFrameGrabber->SetPWM(right_ir_addressoff_pwm,right_ir_pwm_val);
					pFrameGrabber->SetPWM(right_ir_addressoff_pwm_dur,right_ir_pwm_dur_val);
				}

				if (m_pRGBController) ((RGBControllerNano *)m_pRGBController)->SetCmxHandler(pCMXHandle);
	
			}
			break;
			case EyelockLED:
				m_pRGBController = new RGBControllerEyelock;
				break;
			case MiniLED:
				m_pRGBController = new RGBControllerMini;
				break;
			case PicoLED:
				m_pRGBController = new RGBControllerPico;
				break;
			}

			pledDispatcher = new LEDDispatcher(conf);
			pledConsolidator = new LEDConsolidator(conf);
			pledConsolidator->SetLedDispatcher(pledDispatcher);
			pImageProcessor->m_ImageProcessor->SetNwLEDDispatcher(pnwLEDDispatcher);
			pImageProcessor->m_ImageProcessor->SetLEDConsolidator(pledConsolidator);

	#ifdef __SOUND__

			float volume = conf.getValue("GRI.AuthorizationToneVolume", 1.0f); /* DJH: NEW */
			int seconds = conf.getValue("GRI.AuthorizationToneDurationSeconds", 1); /* DJH: NEW */
			if((volume > 0) || (seconds > 0))
			{
				pAudioDispatcher = new AudioDispatcher(conf);
				if(pAudioDispatcher){
					pAudioDispatcher->init();
					//m_matchDispatcher->addProcessor(pAudioDispatcher);
					//pledConsolidator->addProcessor(pAudioDispatcher);
					pF2FDispatcher->addProcessor(pAudioDispatcher);
					pAudioDispatcher->SetCmxHandler(pCMXHandle);
				}
			}
	#endif

			if(pledDispatcher){
				pledDispatcher->init();
				if(pDBReceive){
					pDBReceive->m_ledDispatcher = pledDispatcher;
				}
				if(pLoiteringDetector){
					pLoiteringDetector->SetLEDConsolidator(pledConsolidator);
				}
				pledDispatcher->SetRGBController(m_pRGBController);
				pledDispatcher->SetInitialState();
				if(pNwMatchManager){
					pNwMatchManager->SetMatchDispatcher(m_matchDispatcher);
					pNwMatchManager->SetLedDispatcher(pledDispatcher);
					if(pledConsolidator)m_matchDispatcher->SetLEDConsolidator(pledConsolidator);
					if(pF2FDispatcher)
					{
						m_matchDispatcher->addProcessor(pF2FDispatcher);
						pF2FDispatcher->SetLEDConsolidator(pledConsolidator);
					}
				}
			}
		}
	}

	if(pledConsolidator){
		pledConsolidator->SetLedDispatcher(pledDispatcher);
		pledConsolidator->SetF2FDispatcher(pF2FDispatcher);
		pledConsolidator->SetNwLedDispatcher(pnwLEDDispatcher);
	}

	// Enable liquid lens for both master and slave cameras
	bool doLiquidLens = conf.getValue("MAX14515A.Enable",false);
	if(doLiquidLens && pImageProcessor->m_ImageProcessor){
		//m_pLiquidLens = new LiquidLens(conf);
		if(m_pLiquidLens){
			pImageProcessor->m_ImageProcessor->AddImageHandler(m_pLiquidLens);
		}
	}

	m_watchDogTimeOutInSecs=0;//conf.getValue("GRI.WatchDogTimeout",0);
	m_watchDogIgnoreCtrlC=conf.getValue("GRI.WatchDogIgnoresCtrl_C",false);
	m_enable_newWD =conf.getValue("GRI.enableWatchDog",false);

	//m_mainSleepuSecs=conf.getValue("GRI.HeartBeatTimeoutmilliSec",250)*1000; /* DJH: Rename to WatchDogTickleIntervalInMillisec */
	//if(!m_enable_newWD){
	//	m_mainSleepuSecs=2*1000*1000;
	//}

	m_mainSleepuSecs=conf.getValue("GRI.WatchDogTickleIntervalInMillisec",250)*1000;
	m_dualAuthN = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	m_dualAuthCheckInterval = conf.getValue("GRITrigger.DualAuthenticationCardCheckInterval",1000);
	m_temperature = false;
	m_tamper = false;

	bool supportNanoSDK = conf.getValue("GRI.SupportNanoSDK",0);
	if(supportNanoSDK){
		startTNIListener ();
		if(m_Master){
			pSDKDispatcher = new SDKDispatcher(&conf);
			pSDKDispatcher->init();
			if(pEyelockNanoSdkThread){
				pEyelockNanoSdkThread->SetSDKDispatcher(pSDKDispatcher);
				if(m_matchDispatcher)m_matchDispatcher->SetSDKDispatcher(pSDKDispatcher);
				if(pF2FDispatcher)pF2FDispatcher->SetSDKDispatcher(pSDKDispatcher);
			}
		}
	}

	m_timeSync = (char *)conf.getValue("GRI.InternetTimeSync","false");
	m_timeServAddr = (char *)conf.getValue("GRI.InternetTimeAddr","");

	EyelockLog(logger, DEBUG, "EyeLockMain() End");
}

EyeLockMain::~EyeLockMain(){
	if(pMasterSlaveNwListener){
		EyelockLog(logger, INFO, "delete MasterSlaveNwListener");
		delete pMasterSlaveNwListener;
	}
	EyelockLog(logger, INFO, "delete Match Dispatcher");
	if(m_matchDispatcher) delete m_matchDispatcher;
	EyelockLog(logger, INFO, "delete LoiteringDetector");
	if(pLoiteringDetector) delete pLoiteringDetector;
	EyelockLog(logger, INFO, "delete MatchProcessor");
	if(pMatchProcessor) delete pMatchProcessor;
	EyelockLog(logger, INFO, "delete F2FDispatcher");
	if(pF2FDispatcher) delete pF2FDispatcher;
	EyelockLog(logger, INFO, "delete NwLEDDispatcher");
	if(pnwLEDDispatcher) delete pnwLEDDispatcher;
	EyelockLog(logger, INFO, "delete NwDispatcher");
	if(pNwDispatcher) delete pNwDispatcher;
	EyelockLog(logger, INFO, "delete DBReceive");
	if(pDBReceive) delete pDBReceive;
	EyelockLog(logger, INFO, "delete LEDDispatcher");
	if(pledDispatcher) delete pledDispatcher;
	EyelockLog(logger, INFO, "delete LedConsolidator");
	if(pledConsolidator&&(m_softwareType==eNTSGLM)) delete pledConsolidator;
	EyelockLog(logger, INFO, "delete AudioDispatcher");
	if(pAudioDispatcher) delete pAudioDispatcher;
	EyelockLog(logger, INFO, "delete EyeDispatcher");
	if(pEyeDispatcher) delete pEyeDispatcher;
	EyelockLog(logger, INFO, "delete NwMatchManager");
	if(pNwMatchManager) delete pNwMatchManager;
	EyelockLog(logger, INFO, "delete RGBController");
	if(m_pRGBController) delete m_pRGBController;
	EyelockLog(logger, INFO, "delete NanoSdkThread");
   	if(pEyelockNanoSdkThread)	delete pEyelockNanoSdkThread;
	EyelockLog(logger, INFO, "delete SDKDispatcher");
   	if(pSDKDispatcher)	delete pSDKDispatcher;

   	EyelockLog(logger, INFO, "delete LiquidLens");
	if(m_pLiquidLens) delete m_pLiquidLens;

	EyelockLog(logger, INFO, "delete CMX Handle");
	if(pCMXHandle) delete pCMXHandle;

	EyelockLog(logger, INFO, "delete ImageProcessor / EyelockThread");
	if(pImageProcessor) delete pImageProcessor;
   	EyelockLogClose();
}

inline bool GetLockedValue(Safe<bool> &thing)
{
	thing.lock();
	bool value = thing.get();
	thing.unlock();
	return value;
}



#define CHECK_CAMERA_TIME 4*1000000


void EyeLockMain::run(){
	EyelockLog(logger, DEBUG, "EyeLockMain::run() Start");
	// we need to make nwListener know about a few processors;
	// if they were not instantiated it will not matter
#ifdef __NW_THREAD__
	if(pMasterSlaveNwListener){
		pMasterSlaveNwListener->m_pEyeDispatcher = pEyeDispatcher;
		pMasterSlaveNwListener->m_ledConsolidator = pledConsolidator;
	}
	nwListener.m_ledConsolidator=pledConsolidator;
	nwListener.m_DBDispatcher = pDBReceive;
	nwListener.m_F2FDispatcher = pF2FDispatcher;
	nwListener.m_pEyeDispatcher = pEyeDispatcher;
#endif

	pImageProcessor->m_MatchProcessor =  pMatchProcessor;
	pImageProcessor->m_ledConsolidator = pledConsolidator;
	pImageProcessor->m_pEyeDispatcher = pEyeDispatcher;
	pImageProcessor->m_nwLedDispatcher = pnwLEDDispatcher;

	if(pMatchProcessor && pledDispatcher)
		pMatchProcessor->SetLedDispatcher(pledDispatcher);

	EyelockLog(logger, INFO, "startWithMatching");
	startWithMatching();
	PushAllThreads();

#ifdef HAS_SPI_WDT
	if(m_Master && m_enable_newWD)
		do_wdt_en(); /* DJH: No return code for these ? */
#endif
	int sleepTimeUSecs = 0;
	int mbVersion = GetMotherBoardVersion();
	EyelockLog(logger, INFO, "Eyelock Mother Board Software Version: 0.0.%d", mbVersion);

	unsigned int count = 0;
	unsigned int loop = 0;
	while( GetLockedValue(m_KeepAlive) ){
		usleep(m_mainSleepuSecs/10);	// 250ms/10=25ms

		if (m_Master) {
			if(pF2FDispatcher){
				// Check the BoB event
				pF2FDispatcher->CheckBoBEvents();
			}
		}
		if (loop++ % 10)
			continue;

		sleepTimeUSecs += m_mainSleepuSecs;

		if (m_Master) {
			if(pF2FDispatcher){
				// Check tamper
#ifndef CMX_C1 // always check for EXT
				if (mbVersion >= 1)
#endif
				{
					if (pF2FDispatcher->m_tamperEnable && count % 4*5 == 0) { // check tamper every 5 second
						// check intrusion tamper every second
						bool tamper = CheckTamperSensor();
						if (tamper != pF2FDispatcher->isMBTamper) {
							EyelockLog(logger, INFO, "Tamper Alarm: intrusion tamper state changed current %d, new %d", pF2FDispatcher->isMBTamper, tamper);
							pF2FDispatcher->ProcessTamper(tamper, true);
						}
					}
				}
			}

			// Check temperature
			if (mbVersion >= 1 && count % 240 == 0) {
				// 1 min = 250*4*60
				// temperature
				int tempe = CheckTemperature();
				if (tempe == 1 && !m_temperature) {
					// turn off the cameras, disable the watchdog, turn off the LEDs and stop processing eye data, kill slave
					EyelockEvent("Temperature is too high and terminate everything cleanly");
					EyelockLog(logger, ERROR, "Temperature is too high and terminate everything cleanly");
					// report
					if (pF2FDispatcher) {
						pF2FDispatcher->SendAlarmMsg("Temperature is too high");
						pF2FDispatcher->m_temperature = true;
					}
					EnableIRLEDMonitor(false);
					stopEverything();
					// Exit cleanly
					RunSystemCmd("/home/root/killSlave.sh");
					m_temperature = true;
				}
				else if (tempe == 2 && m_temperature) {
					// enable the watchdog, start Eyelock again
					EyelockEvent("Temperature is normal and restart now");
					EyelockLog(logger, INFO, "Temperature is normal and restart now");
					//system("/home/root/startSlave.sh");
					RunSystemCmd("reboot");
				}


			}
#if 0	// move to script
		   //if (count == 4*60*60*24) {	// 24 hr = 250*4*60*60*24
			if (count == 4*60*5) {	// 5 min
				if (strcmp(m_timeSync,"true")==0 && strcmp(m_timeServAddr,"")!=0) {
					char syncCmd[150];
					EyelockLog(logger, INFO, "Start Clock Sync: server - %s \n", m_timeServAddr);
					//sprintf(syncCmd, "rdate -s %s; /sbin/hwclock -w;", m_timeServAddr);
					//sprintf(syncCmd, "/bin/ntpclient -s -c 2 -i 3 -h %s >/dev/null; /sbin/hwclock -w", m_timeServAddr);
					sprintf(syncCmd, "/bin/ntpclient -s -c 2 -i 3 -h %s >/dev/null;", m_timeServAddr);
					RunSystemCmd(syncCmd);
					EyelockLog(logger, INFO, "End Clock Sync: Command - %s\n", syncCmd);
				}
				count = 0;
			}
#endif
			count++;
		}
		else {	// slave
			if (mbVersion >= 1 && access("temperature", F_OK ) != -1)
			{
				stopEverything();
				RunSystemCmd("rm /home/root/temperature");
				if (m_Debug)
					EyelockLog(logger, DEBUG, "high temperature on slave");
			}
		}
#ifndef TRIGGER_LED
		//		if(m_svrAddr)SendHBStatus();
		if(!m_enable_newWD) continue;
		if(IsAllHealthy()){
			GRITrigger::instance().HeartBeat();
		}
		else{
			cout << " Reboot it now "<<endl;
		}
#endif

#ifdef HAS_SPI_WDT
		if(m_Master && m_enable_newWD)
		{
			bool tickle = true;
			if(sleepTimeUSecs > CHECK_CAMERA_TIME)
			{
				tickle = AreCamerasHealthy();
				sleepTimeUSecs = 0;
			}

			if(tickle)
			{
				do_vibe_tickle();
				if(m_Debug)
				{
					EyelockLog(logger, INFO, "Tickle");
				}
			}
		}
#endif
	}

	EyelockLog(logger, INFO, "Terminate everything cleanly");
	stopEverything(); // Exit cleanly
	EyelockLog(logger, INFO, "F2FDispatcher::End");
	if(pF2FDispatcher)
		pF2FDispatcher->End();
}

void EyeLockMain::startWithMatching(){
#ifdef __NW_THREAD__
	nwListener.m_matchProcessor = pMatchProcessor;
	nwListener.addProcessor(pImageProcessor);
#endif

	if((pledDispatcher)&&(m_SendLed)&&(pMatchProcessor)){
		pMatchProcessor->addProcessor(pledDispatcher);
	}
	if(pF2FDispatcher&&(pMatchProcessor))
	{
		pMatchProcessor->addProcessor(pF2FDispatcher);
		EyelockLog(logger, DEBUG, "Add F2FDispatcher");
	}// Launch Threads

	if(m_Master){
		if(pNwMatchManager && pDBReceive)
			pDBReceive->addProcessor(pNwMatchManager);

		if((pF2FDispatcher && pNwDispatcher)){
			pF2FDispatcher->addProcessor(pNwDispatcher);
			if(pNwDispatcher)pNwDispatcher->Begin();
		}
		if(m_softwareType==ePICO){
			if(pDBReceive){
				pDBReceive->addProcessor(pImageProcessor);
			}
			if(pMatchProcessor)pMatchProcessor->addProcessor(pNwDispatcher);
			if(pNwDispatcher)pNwDispatcher->Begin();
		}
	}else{
		if(pDBReceive){
			pDBReceive->addProcessor(pImageProcessor);
		}
		if(pNwDispatcher&&(pMatchProcessor)){
			pMatchProcessor->addProcessor(pNwDispatcher);
			pNwDispatcher->Begin();
		}
	}
#ifdef __SOUND__
	if(pAudioDispatcher)
		pAudioDispatcher->Begin();
#endif
	if(pnwLEDDispatcher)
		pnwLEDDispatcher->Begin();

	if(pledConsolidator&&(m_softwareType==eNTSGLM))
	   pledConsolidator->Begin();

	if(pledDispatcher)
		pledDispatcher->Begin();

	if(pF2FDispatcher)
		pF2FDispatcher->Begin();

	if(pDBReceive)
		pDBReceive->Begin();

	if(pEyeDispatcher)
		pEyeDispatcher->Begin();

	if(pNwMatchManager)
		pNwMatchManager->Begin();

	if(m_pLiquidLens)
		m_pLiquidLens->Begin();

	if(pLoiteringDetector)
		pLoiteringDetector->Begin();

	if(pSDKDispatcher)
		pSDKDispatcher->Begin();

   	if(pEyelockNanoSdkThread)
   		pEyelockNanoSdkThread->Begin();

	// start listening for messages last
   	if(pMasterSlaveNwListener)pMasterSlaveNwListener->Begin();
   	if(pCMXHandle) pCMXHandle->Begin();

	pImageProcessor->Begin();

#define DO_SLAVE_NWLISTENER 1 /* DJH: Added to support NWListener thread on slave boards for dynamic configuration */
#ifdef __NW_THREAD__
#if !DO_SLAVE_NWLISTENER
	if(!m_Slave)
#endif
	{
		strcpy(nwListener.m_version, EYELOCK_VERSION);
		nwListener.Begin();
	}

#endif

	//match dispatcher
	if(m_matchDispatcher)
	{
		m_matchDispatcher->Begin();
	}
}

void EyeLockMain::stopEverything()
{
#if 0
	for(int i = 0; i < m_Threads.size(); i++)
	{
		if(m_Threads[i])
			m_Threads[i]->End();
	}
#else
	// Terminate threads cleanly, proceeding mostly from highest level to lower level dependencies
	
	EyelockLog(logger, INFO, "NwMatchManager::End");
	if(pNwMatchManager)
		pNwMatchManager->End();

#ifdef __NW_THREAD__
#if !DO_SLAVE_NWLISTENER
	if(!m_Slave)
#endif
	{
		EyelockLog(logger, INFO, "NwListener::End");
		nwListener.End();
	}
#endif

	EyelockLog(logger, INFO, "NwDispatcher::End");
	if(pNwDispatcher)
		pNwDispatcher->End();

	EyelockLog(logger, INFO, "NwledDispatcher::End");
	if(pnwLEDDispatcher)
		pnwLEDDispatcher->End();

	EyelockLog(logger, INFO, "LedConsolidator::End");
	if(pledConsolidator)
		pledConsolidator->End();

	EyelockLog(logger, INFO, "LedDispatcher::End");
	if(pledDispatcher)
		pledDispatcher->End();

#if 0	// don't stop F2FDispatcher
	EyelockLog(logger, INFO, "F2FDispatcher::End");
	if(pF2FDispatcher)
		pF2FDispatcher->End();
#endif

	EyelockLog(logger, INFO, "DBReceive::End");
	if(pDBReceive)
		pDBReceive->End();

	EyelockLog(logger, INFO, "Match Dispatcher:End");
	if(m_matchDispatcher)
		m_matchDispatcher->End();

	EyelockLog(logger, INFO, "EyeDispatcher::End");
	if(pEyeDispatcher)
		pEyeDispatcher->End();
#ifdef __SOUND__
	EyelockLog(logger, INFO, "AudioDispatcher::End");
	if(pAudioDispatcher)
		pAudioDispatcher->End();
#endif

	if(m_pLiquidLens)
		m_pLiquidLens->End();
	EyelockLog(logger, INFO, "LoiteringDetector::End");
	if(pLoiteringDetector)
		pLoiteringDetector->End();
#endif
	if(pSDKDispatcher){
		pSDKDispatcher->End();
	}
   	if(pEyelockNanoSdkThread)
   		pEyelockNanoSdkThread->End();

   	if(pCMXHandle){
   		pCMXHandle->End();
   	}

   	EyelockLog(logger, INFO, "Done terminating, last EyelockThread");

	EyelockLog(logger, INFO, "EyelockThread::End");
	if(pImageProcessor)
		pImageProcessor->End();

}




void *hdDispatcher(void *args) {
	EyeLockMain *srv = (EyeLockMain *) (args);

	NWHDMatcher n(srv->conf);
	n.run();
}

void EyeLockMain::startHDListener() {
	pthread_create(&hdThread, NULL, ::hdDispatcher, (void *) (this));
}


void EyeLockMain::startFaceTracking() {
	pthread_create(&ftThread, NULL, init_facetracking, (void *) (this));
}

void EyeLockMain::startTNIListener() {
	pEyelockNanoSdkThread = new EyelockNanoSdkThread(conf);
	pEyelockNanoSdkThread->Setup();
    pEyelockNanoSdkThread->SetEyelockThread(pImageProcessor);
	pEyelockNanoSdkThread->SetEyelockFirmwareVersion(EYELOCK_VERSION);

	pEyelockNanoSdkThread->SetConsolidator(pledConsolidator);
	pEyelockNanoSdkThread->SetAudioDispatcher(pAudioDispatcher);
	pEyelockNanoSdkThread->SetNwMatchManager(pNwMatchManager);
	pEyelockNanoSdkThread->SetSDKDispatcher(pSDKDispatcher);
	pEyelockNanoSdkThread->SetF2FDispatcher(pF2FDispatcher);
}

static struct sigaction sa;
void EyeLockMain::set_wd_counter()
{
	m_watchdogfd = open("/dev/watchdog", O_WRONLY);
	if(m_watchdogfd==-1){

		EyelockLog(logger, ERROR, " Could not find watch dog timer");
	}
	else
	{
#ifndef __HAVE_ANDROID_WATCHDOG__
		ioctl(m_watchdogfd, WDIOC_SETTIMEOUT, &m_watchDogTimeOutInSecs);
		EyelockLog(logger, INFO, "Watchdog will reboot the system after %d secs of inactivity",m_watchDogTimeOutInSecs);

#endif
		if(!m_watchDogIgnoreCtrlC){
			sa.sa_handler = (EyeLockMain::safe_exit);
			sigaction(SIGINT, &sa, NULL);
			sigaction(SIGTERM, &sa, NULL);
		}
	}
}
void EyeLockMain::PushAllThreads(){
	if(pMatchProcessor) m_Threads.push_back(pMatchProcessor);
	if(pF2FDispatcher) m_Threads.push_back(pF2FDispatcher);
	if(pNwDispatcher) m_Threads.push_back(pNwDispatcher);
	if(pnwLEDDispatcher) m_Threads.push_back(pnwLEDDispatcher);
	if(pDBReceive) m_Threads.push_back(pDBReceive);
	if(pledConsolidator)m_Threads.push_back(pledConsolidator);
	if(pledDispatcher) m_Threads.push_back(pledDispatcher);
	if(pEyeDispatcher) m_Threads.push_back(pEyeDispatcher);
	if(m_matchDispatcher) m_Threads.push_back(m_matchDispatcher);
	if(pSDKDispatcher) m_Threads.push_back(pSDKDispatcher);
	if(pEyelockNanoSdkThread) m_Threads.push_back(pEyelockNanoSdkThread);
#ifdef __NW_THREAD__
	m_Threads.push_back(&nwListener);
#endif
}

bool EyeLockMain::AreCamerasHealthy()
{
	bool healthy = true;

	struct timeval curTime;

	gettimeofday(&curTime, 0);
	uint64_t timestamp = curTime.tv_sec;
	timestamp *= 1000000u;
	timestamp += curTime.tv_usec;
	timestamp = timestamp/1000; //millisec

	// Returns TRUE iff all camera HB are active with gaps < NwListener::m_healthTimeOutMS
	healthy = nwListener.IsHealthy(timestamp);

	return healthy;
}

bool EyeLockMain::IsAllHealthy(){
	int sz = m_Threads.size();
	struct timeval curTime;
	bool ret = true;
	gettimeofday(&curTime, 0);
	uint64_t timestamp = curTime.tv_sec;
	timestamp *= 1000000u;
	timestamp += curTime.tv_usec;
	timestamp = timestamp/1000; //millisec

	if(timestamp < nwListener.GetDBtimeStamp())
	{
//		EyelockLog(logger, INFO, "%s DB:: %lu %lu %ld ",nwListener.getName(),timestamp,nwListener.GetDBtimeStamp(),nwListener.GetDBtimeStamp() - timestamp);
		return true;
	}

	// lets check one thread every time.
	if(!m_Threads[m_curval]->IsHealthy(timestamp))
	{
		ret= false;
		FILE *logFile=fopen("/mnt/mmc/app/MatchSvr.log","a+");
		if(logFile){
			fprintf(logFile,"Unhealthy:: %s\n",m_Threads[m_curval]->getName());
			EyelockLog(logger, WARN, "Unhealthy:: %s",m_Threads[m_curval]->getName());
			fclose(logFile);
		}
	}

	m_curval=(m_curval+1)%sz; //circular shift

	return ret;
}

void EyeLockMain::reset_wd_counter()
{
#ifndef __HAVE_ANDROID_WATCHDOG__
	if(m_watchdogfd!=-1){
		int dummy;
		ioctl(m_watchdogfd, WDIOC_KEEPALIVE, &dummy);
	}
#endif
}

void EyeLockMain::safe_exit(int){
	if (m_watchdogfd != -1) {
		write(m_watchdogfd, "V", 1);
		close(m_watchdogfd);
	}
	exit(0);
}


int acquireProcessLock(char *name) {
	char lockName[100];
	sprintf(lockName, "%s.lock", name);

	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;

	int fdlock;
	if ((fdlock = open(lockName, O_WRONLY|O_CREAT, 0666)) == -1)
		return 0;

	if (fcntl(fdlock, F_SETLK, &fl) == -1)
		return 0;

	return 1;
}

#ifndef ANDROID_DLLEyeLockMain

EyeLockMain *pTheApp = 0;

#include <signal.h>
#include <stdio.h>

/*
 * Simple class to test termination of EyelockMain from within gdb, since signal handler is intercepted
 */
class Killer : public HThread
{

public:
	Killer() {}
	const char * getName() { return "Killer"; }
	unsigned int MainLoop()
	{
		while(true)
		{
			int c = getchar();
			EyelockLog(logger, INFO, "Killer::MainLoop: received %c", c);
			if(c == 'k')
			{
				if(pTheApp)
					pTheApp->kill();
			}
		}
	}
};

void
termination_handler (int sig)
{
	signal(sig, SIG_IGN); /* ignore the signal */

	// send a message to the application to shutdown gracefully
	if(pTheApp)
		pTheApp->kill();
}

void EyeLockMain::kill()
{
	m_KeepAlive.lock();
	m_KeepAlive.set(false);
	m_KeepAlive.unlock();
}

int EyeLockMain::CheckTemperature() // internal API
{
	unsigned int temp1 = 0, temp2 = 0;
	int status;

	if(m_Master){
		ScopeLock lock(I2CBusNanoAPI::instance().GetLock());

		if(0 > I2CBusNanoAPI::instance().Assign(NANO_LED_I2C_ADDR0)){
			EyelockLog(logger, ERROR, "Failed to assign LED address on I2C bus ");
			return 0;
		}

		status = I2CBusNanoAPI::instance().Write( 4, 4 );	// COMMAND=4, TEMPERATURE=4
		if(status)
			EyelockLog(logger, ERROR, "Failed to write register COMMAND 4, value 4 for temperature " );
		usleep(100);
		status = I2CBusNanoAPI::instance().Read( 6, &temp1 );	// SENSOR 1
		usleep(100);
		status = I2CBusNanoAPI::instance().Read( 7, &temp2 );	// SENSOR 2
		if(status)
			EyelockLog(logger, ERROR, "Failed to read register temperature 2" );

		//EyelockLog(logger, INFO, "temperature measure: temp1=%d, temp2=%d ", temp1, temp2);

		if (temp1 > 70 || temp2 > 70)
			return 1;
		else if (temp1 < 55 && temp2 < 55)
			return 2;
		else
			return 0;
	}
	return 0;

}

int EyeLockMain::CheckTamperSensor() // internal API
{
	//unsigned int tamper = 0;
	//int status;
    bool tamper = false;
    tamper = DoTamper1();
	/*if (access("/home/root/OIMtamper", F_OK ) != -1)  // tamper file will generated by FaceTracker
	{
		if (m_Debug)
				EyelockLog(logger, DEBUG, "tamper file existing");
		tamper = 1;
	}*/
	// printf("MBBoard tamper %d\n",tamper);
    if(tamper)
    	return 1;
    return 0;
	//return tamper;
}

int EyeLockMain::GetMotherBoardVersion()
{
	unsigned int version = 0;
#if 1 // disable the version check until we get the actual register value, for now we always return 0.
	if(m_Master){
		EyelockLog(logger, DEBUG, "EyeLockMain::GetMotherBoardVersion() start ");
		ScopeLock lock(I2CBusNanoAPI::instance().GetLock());

		if(0 > I2CBusNanoAPI::instance().Assign(NANO_LED_I2C_ADDR0)){
			EyelockLog(logger, ERROR, "Failed to assign LED address on I2C bus ");
			return version;
		}

		I2CBusNanoAPI::instance().Read(14, &version);	// version register 10
		if(version == 0xff) //support old motherboards.
			version = 0;
		EyelockLog(logger, INFO, "EyeLockMain::GetMotherBoardVersion(): version %d ",version);
	}
#endif
	return version;
}

#include "EyeLockThread.h"
int main(int count, char *strings[]){

#if 0
	EyelockLog(logger, DEBUG, "Enable Image Processor");
	FileConfiguration conf("Eyelock.ini");
	EyeLockThread thread(conf);
	thread.init();
#else
	setlocale(LC_ALL, "en_US.UTF-8");
	//setlocale(LC_ALL, "fr_FR.UTF-8");
    if(count >1){
        if(strncasecmp(strings[1],"-v",2)==0){
        #if defined(DES_ENCRYPTION)
            printf("%s\n Version(DES) %s \n",strings[0],EYELOCK_VERSION);
            EyelockLog(logger, INFO, "%s Version(DES) %s ",strings[0],EYELOCK_VERSION);
		#elif defined(AES_ENCRYPTION)
            printf("%s\n Version(AES) %s \n",strings[0],EYELOCK_VERSION);
            EyelockLog(logger, INFO, "%s Version(AES) %s ",strings[0],EYELOCK_VERSION);
		#elif defined(NO_ENCRYPTION)
            printf("%s\n Version(NO) %s \n",strings[0],EYELOCK_VERSION);
            EyelockLog(logger, INFO, "%s Version(NO) %s ",strings[0],EYELOCK_VERSION);
        #endif
        }
        else if(strncasecmp(strings[1],"-t",2)==0){
        	// Eyelock test

        }
        else{
            printf("Usage:\n %s [-version]\n",strings[0]);
            printf("-version prints the version exits\n");
        }
        return 0;
    }
#if defined(DES_ENCRYPTION)
    printf("%s Version(DES) %s \n",strings[0],EYELOCK_VERSION);
    EyelockLog(logger, INFO, "%s Version(DES) %s",strings[0],EYELOCK_VERSION);
#elif defined(AES_ENCRYPTION)
    printf("%s Version(AES) %s \n",strings[0],EYELOCK_VERSION);
    EyelockLog(logger, INFO, "%s Version(AES) %s",strings[0],EYELOCK_VERSION);
#elif defined(NO_ENCRYPTION)
    EyelockLog(logger, INFO, "%s Version(NO) %s",strings[0],EYELOCK_VERSION);
#endif

#ifndef __BFIN__
	if (!acquireProcessLock(strings[0])) {
		EyelockLog(logger, ERROR, "Could not lock, another instance may be running...Exiting!");
		return -1;
	}
#endif
	try
	{
		// check Calibration.ini

		const char *data;
		FileConfiguration calib("/etc/Calibration.ini");
		data=calib.getValue("MT9P001.global_gain_val","");
		strcpy(intCalib.global_gain_val, data);
		data=calib.getValue("GRI.dc","");
		strcpy(intCalib.dc, data);
		data=calib.getValue("GRI.shiftRight","");
		strcpy(intCalib.shiftRight, data);
		data = calib.getValue("Eyelock.EnableDiffIllumination", "");
		strcpy(intCalib.EnableDiffIllumination, data);
		data = calib.getValue("Eyelock.LeftIlluminationValue", "");
		strcpy(intCalib.LeftIlluminationValue, data);
		data = calib.getValue("Eyelock.RightIlluminationValue", "");
		strcpy(intCalib.RightIlluminationValue, data);

		EyeLockMain m("Eyelock.ini");
		pTheApp = &m; // get global pointer for signal handler

		//Killer killer; killer.Begin(); // test clean exit/kill mechanism from gdb

		if (signal (SIGINT, termination_handler) == SIG_IGN)
			signal (SIGINT, SIG_IGN);
		if (signal (SIGHUP, termination_handler) == SIG_IGN)
			signal (SIGHUP, SIG_IGN);
		if (signal (SIGTERM, termination_handler) == SIG_IGN)
			signal (SIGTERM, SIG_IGN);

		// ignore sigpipe errors always
		signal (SIGPIPE, SIG_IGN);
		m.run();
	}
	catch(Exception& nex)
	{
		EyelockLog(logger, ERROR, "EyeLockMain Exception -=--------------------------------\n"); fflush(stdout);
		nex.PrintException();
	}
	catch(const char *msg)
	{
		EyelockLog(logger, ERROR, "EyeLockMain Exception: %s", msg);
		std::cout<< msg <<endl;
	}
	catch(...)
	{
		EyelockLog(logger, ERROR, "Exception found in EyeLockMain");
		std::cout<< "Exception found in EyeLockMain " <<endl;
	}

	EyelockLog(logger, INFO, "Exiting!");
#endif
	return 0;
}
#endif

