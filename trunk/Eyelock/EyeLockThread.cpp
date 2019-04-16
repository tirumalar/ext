/*
 * ImageGrabber.cpp
 *
 *  Created on: 29-Sep-2010
 *      Author: madhav.shanbhag@mamigo.us
 */
#include "EyeLockThread.h"
#include "ELEyeMsg.h"
#include <iostream>
#define ENABLE
#define IRIS
/////////////
#ifdef ENABLE
	#include "Parsing.h"
//	#ifdef IRIS
//		#include "IrisSelector.h"
//	#endif
#endif

#include "socket.h"
#include "MessageExt.h"
#include "Safe.h"
#include "HThread.h"
//#include "EmbeddedEnrollment.h"
#include "adp8860.h"
#include "adp8860-regs.h"
#include "MT9P001FrameGrabber.h" /* UGLY hack */
#include "NetworkUtilities.h"
#include "SocketFactory.h"
#include "logging.h"
#ifdef IRIS_CAPTURE
#include "PostMessages.h"
#endif
//#undef TIME_OP
//#define TIME_OP XTIME_OP

const char logger[30] = "EyeLockThread";

/*
 *  HW abstraction layer (Eyelock)
 */

RGBControllerEyelock::RGBControllerEyelock() {
	if (rgb_led_enable(1) < 0) {
		// TODO: error
	}
}

RGBControllerEyelock::~RGBControllerEyelock() {
	if (rgb_led_enable(0) < 0) {
		// TODO: error
	}
}

void RGBControllerEyelock::SetRGB(unsigned char mask) {
	// Extract r, g, b from mask
	int r = (mask & (1 << 4)) ? 1 : 0; /* bit 4 */
	int g = (mask & (1 << 2)) ? 1 : 0; /* bit 2 */
	int b = (mask & 1) ? 1 : 0; /* bit 0 */

	rgb_led_set_color(r, g, b);
}

/*
 * Nano
 */

RGBControllerNano::RGBControllerNano(MT9P001FrameGrabber *pFrameGrabber, int brightnessR, int brightnessG, int brightnessB) : m_pFrameGrabber(pFrameGrabber), m_BrightnessR(brightnessR), m_BrightnessG(brightnessG), m_BrightnessB(brightnessB)
{
	m_pCmxHandler = NULL;
}

RGBControllerNano::~RGBControllerNano()
{

}

/*
 *  For now piggy-back on MT9P001FrameGrabber since it needs per frame control of LED banks
 */

void RGBControllerNano::SetRGB(unsigned char mask) {
	int r = (mask & (1 << 4)) ? 1 : 0; /* bit 4 */
	int g = (mask & (1 << 2)) ? 1 : 0; /* bit 2 */
	int b = (mask & 1) ? 1 : 0; /* bit 0 */

//	EyelockLog(logger, DEBUG, "RGBLedServerNano::SetRGB(%d %d %d)", r, g, b);
//	fflush( stdout);
#ifndef CMX_C1
	// NXT
	RGBTriple rgb(r * m_BrightnessR, g * m_BrightnessG, b * m_BrightnessB  );

	/*
	MT9P001FrameGrabber::m_RGB.lock();
	MT9P001FrameGrabber::m_RGB.set(rgb);
	MT9P001FrameGrabber::m_RGB.setUpdated(true);
	MT9P001FrameGrabber::m_RGB.unlock();
	*/
	if(m_pFrameGrabber)
	{
		m_pFrameGrabber->FlashRGB( rgb );
	}

#else
	unsigned char buf[256];
	buf[0] = CMX_LED_CMD;
	buf[1] = 3;
	buf[2] = r * m_BrightnessR;
	buf[3] = g * m_BrightnessG;
	buf[4] = b * m_BrightnessB;
	if (m_pCmxHandler)
		m_pCmxHandler->HandleSendMsg((char *)buf, m_pCmxHandler->m_Randomseed);
#endif

}


#include <strings.h>
#include "NwListener.h"

template <typename T>
class SafeScopeLock
{
public:
	SafeScopeLock (Safe<T> &thing) : m_Thing(thing)
	{
		m_Thing.lock();
	}
	~SafeScopeLock()
	{
		m_Thing.unlock();
	}
	Safe<T> &m_Thing;
};

EyelockModeMessageHandler::EyelockModeMessageHandler()
{
	SafeScopeLock<int> lock(m_Mode);
	m_Mode.set((EyelockProcessMode) EyelockDual);
	m_Mode.setUpdated(false);
	m_PeerAddress.clear();
}

bool EyelockModeMessageHandler::GetMode(EyelockProcessMode &value, std::string &address)
{
	bool valid = false;
	{
		SafeScopeLock<int> lock(m_Mode);
		value = (EyelockProcessMode) m_Mode.get();
		valid = m_Mode.isUpdated();
		address += m_PeerAddress;
		m_Mode.setUpdated(false); // invalidate data upon read
	}
	return valid;
}

void EyelockModeMessageHandler::Reset()
{
	SafeScopeLock<int> lock(m_Mode);
	m_PeerAddress.clear();
	m_Mode.setUpdated(false);
}

#define NIPQUAD(addr) \
         ((unsigned char *)&addr)[0], \
         ((unsigned char *)&addr)[1], \
         ((unsigned char *)&addr)[2], \
         ((unsigned char *)&addr)[3]

void EyelockModeMessageHandler::HandleEyeLockModeMsg(HTTPPOSTMsg &message, Socket *pClient){

	try
	{
		int size = message.GetSize();
		message.GetBuffer()[size] = 0;
		std::string input(message.GetBuffer());
		std::string semicolon = ";";
		std::vector< std::string > tokens = tokenize(input, semicolon);
		if(tokens.size() < 2)
			return ;
		for(int i = 0; i < tokens.size(); i++){
			EyelockLog(logger, DEBUG, "[%d]=%s", i, tokens[i].c_str()); fflush(stdout);
		}
		const char *tag = "EYELOCK_MODE";
		bool isEyelockMode = (0 == tokens[0].compare(tag));
		if(isEyelockMode){
			int value = atoi(tokens[1].c_str());
			{
				SafeScopeLock<int> lock(m_Mode);
				m_Mode.set(value);
				m_Mode.setUpdated(true);

				m_PeerAddress.clear();
				if((tokens.size() > 2) && (((value&0x1) == 0)||(value==3)))
				{
					m_PeerAddress += tokens[2];
				}
			}

			if(pClient){
				char* text = "EYELOCK_MODE;DONE";
				BinMessage ack(text,strlen(text)+1);
				pClient->SendAll(ack);
				EyelockLog(logger, DEBUG, "Sending %s",ack.GetBuffer());
			}
			EyelockLog(logger, DEBUG, "message %s", message.GetBuffer());
		}
	}
	catch(...)
	{

	}
}

void EyelockModeMessageHandler::SetEyeLockMode(string host,EyelockProcessMode value)
{
	SafeScopeLock<int> lock(m_Mode);
	m_Mode.set(value);
	m_Mode.setUpdated(true);
	m_PeerAddress.clear();
	m_PeerAddress += host;
}

void EyelockModeMessageHandler::ResetHaloDC(){
	SafeScopeLock<ShiftHaloAndDC> lock(m_ShiftHaloDC);
	m_ShiftHaloDC.setUpdated(false);
}

bool EyelockModeMessageHandler::getHaloDCState(){
	SafeScopeLock<ShiftHaloAndDC> lock(m_ShiftHaloDC);
	bool ret = m_ShiftHaloDC.isUpdated();
	return ret;
}
ShiftHaloAndDC EyelockModeMessageHandler::getHaloDC(){
	SafeScopeLock<ShiftHaloAndDC> lock(m_ShiftHaloDC);
	ShiftHaloAndDC testval = m_ShiftHaloDC.get();
	return testval;
}

void EyelockModeMessageHandler::HandleHaloDcMsg(HTTPPOSTMsg &message){
	EyelockLog(logger, DEBUG, "Got HaloDc MSG ");
	ShiftHaloAndDC shdc;
	float haloth;
	int dc,shift,maxspec;

	bool test = message.getHaloAndDc(dc,shift,maxspec,haloth);
	if(test){
		shdc.halothreshold = haloth;
		shdc.dcoffset = dc;
		shdc.shift = shift;
		shdc.maxspecval = maxspec;

		SafeScopeLock<ShiftHaloAndDC> lock(m_ShiftHaloDC);
		m_ShiftHaloDC.set(shdc);
		m_ShiftHaloDC.setUpdated(true);
	}
}
bool EyelockModeMessageHandler::Handle(HTTPPOSTMsg &message, Socket *pClient)
{
//	EyelockLog(logger, DEBUG, "EyelockModeMessageHandler::Handle()"); fflush(stdout);

	NWMESSAGETYPE msgType=message.getMsgType();
	switch(msgType){
	case EYELOCK_MODE:
		HandleEyeLockModeMsg(message,pClient);
		break;
	case HALO_DC:
		HandleHaloDcMsg(message);
	default:
		break;
	}
}
//////////////////////

int EyeLockThread::Begin()
{
	HThread::Begin();
}

int EyeLockThread::End()
{
	EyelockLog(logger, INFO, "EyeLockThread::End()"); fflush(stdout);
	FrameGrabber *pFrameGrabber = m_ImageProcessor->GetFrameGrabber();
	if(pFrameGrabber)
	{
		EyelockLog(logger, INFO, "EyeLockThread::stopping frame grabber()"); fflush(stdout);
		pFrameGrabber->stop();
	}

	HThread::End();

	EyelockLog(logger, DEBUG, "EyeLockThread::End() done"); fflush(stdout);
}




EyeLockThread::EyeLockThread(Configuration& conf) :
	GenericProcessor(conf), m_MatchProcessor(0), m_DetectedMsg(20),m_pEyeDispatcher(0),m_ScoreFactor(0.5),m_spoofEnable(false),m_nwLedDispatcher(NULL),
			m_MotionMsg(20), m_outQSize(5), m_Debug(false), m_MaxFrames(5),m_HBDestAddr(0),m_shouldProcess(true),m_shouldSegment(true),m_tsDestAddrpresent(false),
			m_ledConsolidator(0), m_Mode(EyelockDual),m_HBMsg(256),m_ProcessModeMsg(256),m_FuturisticTime(0), m_pNwListener(0),m_doMatching(true),m_sleepTimeFlow(10),
			m_MustDie(false),m_MinTrackedCount(0),m_logging(false),m_segmentationMiliSecSleep(20),m_trackingEnabled(true),m_detectDestAddr(0),m_DetectBinMsg(0)
{
	m_PreviousMode = m_Mode;
	const char* detectedMsg = conf.getValue("GRI.DetectedLEDMsg", "Name;0");
	const char* motionMsg = conf.getValue("GRI.MotionMsg", "MOTION");

	m_softwareType = (SoftwareType)conf.getValueIndex("Eyelock.Type", ePC, eNTSGFS, eNANO, "PC", "NANO", "PICO","NTSGLM","NTSGLS","NTSGFM","NTSGFS");

	m_DetectedMsg.SetData(detectedMsg, strlen(detectedMsg));
	m_MotionMsg.SetData(motionMsg, strlen(motionMsg));
	m_Debug = conf.getValue("Eyelock.Debug", false);
	m_outQSize = conf.getValue("GRI.outQSize", m_outQSize);
	m_MaxFrames = conf.getValue("Eyelock.MaxFramesBeforeSeg", 4);
	m_MinTrackedCount = conf.getValue("Eyelock.MinTrackedBeforeSeg", 0);
	m_noImageMode = conf.getValue("GRI.noImageMode", 0);
	m_shouldSegment = conf.getValue("Eyelock.ShouldSegment",true);
	m_trackingEnabled = conf.getValue("Eyelock.ShouldTrack",true);
	m_doMatching = conf.getValue("Eyelock.Matching",true);
	m_usbEnabled = conf.getValue("Eyelock.USBSlaveEnabled",false);
	m_dualAuth = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);
	m_sleepTimeFlow = conf.getValue("Eyelock.SleepTimeFlow",10);
	m_ledPWMSwitching = conf.getValue("Eyelock.LEDPWMSwitchingAfterDetect",false);
	m_currentState = false;

	outMsgQueue(m_outQSize);

	m_segmentationMiliSecSleep = conf.getValue("Eyelock.SleeptimeAfterSegMiliSec", 20);
	m_segmentationMiliSecSleep = m_segmentationMiliSecSleep *1000;
	// Would prefer to initialize EyelockImageGrabber after outMsgQueue has been initialized
	// but outMsgQueue apparently needs ImageProcessor::defBuffsize
	// we can possibly make a static method  that returns this given a config file
	m_ImageProcessor = new EyeLockImageGrabber(&conf, outMsgQueue,
			m_DetectedMsg, m_MotionMsg);

	int bufSize = conf.getValue("GRI.sendBufferSize",m_ImageProcessor->defBuffSize);
	for (int i = 0; i < m_outQSize; i++) {
		outMsgQueue[i] = new SafeFrameMsg(bufSize);
	}
	FlushAll();

	const char *svrAdd = conf.getValue("Eyelock.DetectDestAddr", "NONE");
	if(strcmp(svrAdd,"NONE") != 0){
		m_detectDestAddr = HostAddress::MakeHost(svrAdd, eIPv4, false);
	}

	const char *ledmsg = conf.getValue("GRI.LEDSetStr","LEDSET");
	char buf[100]={0};
	sprintf(buf,"%s;%d;",ledmsg,(int)DETECT);
	m_DetectBinMsg = new BinMessage(buf, strlen(buf));

	m_ledMR.init();
	m_ledMR.setState(LED_DETECT);

	//Try to implement HB for Eyelock
	m_Master = conf.getValue("GRI.EyelockMaster",false);

	m_pEyelockMessageHandler = 0;
	m_pEyelockMessageHandler = new EyelockModeMessageHandler;
#ifdef IRIS_CAPTURE
	m_bIrisCaptureEnabled = conf.getValue("Eyelock.IrisMode", 1) == 2;
	m_HeartBeatFrequency = conf.getValue("Eyelock.IrisCaptureHeartBeatFrequency", 5);
//	DMOOUT WHy is this here?  if (m_SPAWAREnable)  this instance is assigned by EyelockMain... or at least it should be...
//	{
//		m_HttpPostSender = new HttpPostSender(conf);
//	}
#endif	
	// Parse pipe "|" separate list of slave IP addresses for processing mode notification
	// e.g., nano019-1.local|nano019-2.local|nano019-3.local
	// If name resolution fails for any of the slaves, we need to terminate
	std::string slaveAddrList = (char *)conf.getValue("GRI.SlaveAddressList", "NONE");
	if(slaveAddrList.compare("NONE") != 0)
	{
		std::string pipe = "|";
#ifdef ENABLE		
		std::vector<std::string> slaves = tokenize(slaveAddrList, pipe);
		for(int i = 0; i < slaves.size(); i++)
		{
			HostAddress *pAddress = 0;
			try
			{
				pAddress = HostAddress::MakeHost(slaves[i].c_str());
				m_SlaveAddressList.push_back(pAddress);
			}
			catch(...)
			{
				EyelockLog(logger, DEBUG, "EyeLockThread can't find GRI.SlaveAddressList=%s, must terminate", slaves[i].c_str()); fflush(stdout);
				m_MustDie = true;
				throw;
			}
		}
#endif
	}

	//char *svrAddr = (char*)conf.getValue("GRI.HDMatcher.0.Address","NONE"); /* Don't tie HB address to HD matcher */
	char *svrAddr1 = (char*)conf.getValue("GRI.HBDestAddress","NONE");
    if(strcmp(svrAddr1, "NONE") == 0)
    {
    	svrAddr1 = 0;
    }
    else
    {
    	try
		{
    		m_HBDestAddr = HostAddress::MakeHost(svrAddr1);
		}
    	catch(...)
    	{
    		EyelockLog(logger, ERROR, "EyeLockThread can't find GRI.HBDestAddress=%s, must terminate", svrAddr1); fflush(stdout);
    		m_MustDie = true;
    		throw;
    	}
    }

    m_HBMsgFormat = conf.getValue("GRI.HBMsgFormat", "HEARTBEAT:cameraId=\"%s\"");
    m_HBFreqSec = conf.getValue("GRI.HBFreqInSec", 5);
    m_HBTimeOutmilliSec = conf.getValue("GRI.HBTimeoutInmilliSec", 2000);
    const char *cameraID = conf.getValue("GRI.cameraID","Unknown");

    {
    	char *msgBuff = m_HBMsg.GetBuffer();
    	int len = snprintf(msgBuff, m_HBMsg.GetAvailable(), m_HBMsgFormat, cameraID) + 1; /* DJH: bug fix, needs +1 to terminate message */
        m_HBMsg.SetSize(len);
    }
    FormatProcessModeMessage(m_Mode, "127.0.0.1");

    m_spoofEnable = conf.getValue("Eyelock.SpoofEnable",false);
    m_ScoreFactor = conf.getValue("Eyelock.SpoofThresholdFactor",0.5f);
 	m_logging = conf.getValue("Eyelock.Logging", false);
	m_socketFactory = new SocketFactory(conf);

	m_LastDetectedEyeTS =0;
	m_TimeToNormalusec = conf.getValue("Eyelock.LEDPWMSwitchingToNormalAfterNoDetectionMiliSec",3000);
	m_TimeToNormalusec = m_TimeToNormalusec*1000;
	EyelockLog(logger, DEBUG, "Switching will happen after %llu uSec",m_TimeToNormalusec);

	const char *svrAddr = conf.getValue("Eyelock.TSMasterDestAddr", "NONE");
    if(strcmp(svrAddr, "NONE") == 0){
    	m_tsDestAddrpresent = false;
    }else
    	m_tsDestAddrpresent = true;
#if 0
	// If we want external screen images and we are in acquisition mode...
	// Create out window and set the initial image
	// if (m_DHSScreens && (m_EyelockIrisMode == 2))
	{
		cv::Mat Screen = cv::imread("/home/root/screens/Slide1.BMP", cv::IMREAD_COLOR);
		cvNamedWindow("EXT", CV_WINDOW_NORMAL);
		cvSetWindowProperty("EXT", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		imshow("EXT", Screen);
		cvWaitKey(1); // cvWaitKey(100);
	}
 //   cvNamedWindow("EXT", CV_WINDOW_FULLSCREEN);
  //  cvSetWindowProperty("EXT", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
#endif
}

int EyeLockThread::FormatProcessModeMessage(EyelockProcessMode mode, const char *address)
{
  	char *msgBuff = m_ProcessModeMsg.GetBuffer();
  	int len = snprintf(msgBuff, m_ProcessModeMsg.GetAvailable(), "EYELOCK_MODE;%d", (int)mode);
  	m_ProcessModeMsg.SetSize(len + 1);

  	// If non NULL address is passed then we need to send this to the slave camera as well
  	if(address != 0)
  	{
  		const char *selfUSBAddress = "192.168.40.1:8083"; // usb master address - Rajesh  TBD: configurable USB ips
  		m_ProcessModeMsg.SetSize(len); // chop off NULL terminator
  		m_ProcessModeMsg.Append(";", 1);
  		if(m_usbEnabled) // add this config to Eyelock INI, default to false.
  		{
  			// use slave usb address to send the message
  			m_ProcessModeMsg.Append(selfUSBAddress, strlen(selfUSBAddress));
  		}
  		else
  		{
  			m_ProcessModeMsg.Append(address, strlen(address));
  		}
  		m_ProcessModeMsg.Append("\r\n\0", 3);
		EyelockLog(logger, DEBUG, "PROCESS MODE TO SLAVE => %d::%s",m_ProcessModeMsg.GetSize(), m_ProcessModeMsg.GetBuffer());
  	}
  	return len;
}

EyeLockThread::~EyeLockThread() {
	for (int i = 0; i < m_outQSize; i++) {
		delete outMsgQueue[i];
		outMsgQueue[i] = 0;
	}

	if(m_pEyelockMessageHandler)
	{
		delete m_pEyelockMessageHandler;
		m_pEyelockMessageHandler = 0;
	}

	if(m_DetectBinMsg)
		delete m_DetectBinMsg;
	if(m_detectDestAddr)
		delete m_detectDestAddr;
	if(m_HBDestAddr)
		delete m_HBDestAddr;
	if(m_socketFactory)
		delete m_socketFactory;
#ifdef HBOX_PG
	if(m_HttpPostSender)
		delete m_HttpPostSender;
#endif

}

bool EyeLockThread::enqueMsg(Copyable& msg) {
	HTTPPOSTMsg & hMsg = (HTTPPOSTMsg &) msg;
	if (hMsg.isResetEyelock()) {
		EyelockLog(logger, DEBUG, "EyeLockThread::Got Reset Message");
		m_shouldProcess = false;
	}else
		GenericProcessor::enqueMsg(msg);
	return true;
}

Copyable* EyeLockThread::createNewQueueItem() {
	HTTPPOSTMsg *ptr = new HTTPPOSTMsg(30);
	return ptr;
}


void EyeLockThread::SendHBStatus() {

	//if(m_Master) return;   /* DJH: Allow both master and slave to return status for uniformity */

	// Can't send a heartbeat if we don't have an address to send to
//	if(!m_HBDestAddr)
//		return;

	timeval pcurtime;
	static uint64_t ppt=0;
	gettimeofday(&pcurtime, 0);
	TV_AS_USEC(pcurtime,pct);

	long int timestamp = pcurtime.tv_sec;

//	if(m_Debug) EyelockLog(logger, DEBUG, "Curr %ld Fut %ld ",timestamp,m_FuturisticTime);

	if((timestamp > m_FuturisticTime))
	{
		if(m_Master)
		{
			if(m_pNwListener)
			{
				HTTPPOSTMsg message(m_HBMsg.GetBuffer(), m_HBMsg.GetSize());
				m_pNwListener->LogHeartBeat(message);
			}
		}else if(m_HBDestAddr)
		{
			struct timeval timeOut;
			timeOut.tv_sec = m_HBTimeOutmilliSec / 1000;
			timeOut.tv_usec = (m_HBTimeOutmilliSec % 1000) * 1000;
			try{
				SocketClient client=m_socketFactory->createSocketClient("GRI.HBDispatcherSecure");
				client.SetTimeouts(timeOut);
				client.ConnectByHostname(*m_HBDestAddr);
				client.Send(m_HBMsg,MSG_DONTWAIT);
			}
			catch(Exception& nex)
			{
				EyelockLog(logger, ERROR, "HB Exception -=--------------------------------"); fflush(stdout);
				nex.PrintException();
			}
			catch(const char *msg)
			{
				EyelockLog(logger, ERROR, "HB Exception -=--------------------------------"); fflush(stdout);
				std::cout<< msg <<endl;
			}
			catch(...)
			{
				EyelockLog(logger, ERROR, "Unknown exception during SendHB for eyelock");
				std::cout<< "Unknown exception during SendHB for eyelock" <<endl;
			}
		}else{

		}
		m_FuturisticTime = timestamp + m_HBFreqSec;
	}
	return;
}

bool EyeLockThread::IsQueueFull()
{
	citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(outMsgQueue);

	SafeFrameMsg *msg = 0;
	bool bFound = false;

	for (int i = 0; i < m_outQSize; i++) {
		msg = sendIter.curr();
		msg->lock();
		//if any one updated was false means its not full
		if (!msg->isUpdated()) {
			bFound = true;
		}
		msg->unlock();
		sendIter.next();
		if (bFound)
			break;
	}
	return (!bFound);
	// True means full
}

bool EyeLockThread::IsQueueEmpty()
{
	citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(outMsgQueue);

	SafeFrameMsg *msg = 0;
	bool bFound = false;

	for (int i = 0; i < m_outQSize; i++) {
		msg = sendIter.curr();
		msg->lock();
		//if any one updated was true means its not empty
		if (msg->isUpdated()) {
			bFound = true;
		}
		msg->unlock();
		sendIter.next();
		if (bFound)
			break;
	}
	return (!bFound);
	// true means empty
}

bool EyeLockThread::FindBest(int& position) {
	citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(
			outMsgQueue);
	SafeFrameMsg *msg = 0;
	int score = 0, minscore = -10;
	position = -1;
	for (int i = 0; i < m_outQSize; i++) {
		msg = sendIter.curr();
		msg->lock();
		if (msg->isUpdated()) {
			ELEyeMsg hMsg;
			hMsg.CopyFrom(*msg);

			if (hMsg.getMsgType() == IMG_MSG) {
				hMsg.getScore(score);
				if (score > minscore) {
					m_bestEye.CopyFrom(*msg);
					minscore = score;
					position = i;
				}
			} else {
				// database reload request !
				EyelockLog(logger, DEBUG, "NON IMG Msg");
				m_bestEye.CopyFrom(*msg);
				minscore = 0;
				position = i;
				msg->unlock();
				FlushAll();
				break;
			}
		}
		msg->unlock();
		sendIter.next();
	}

	return (minscore > (-10));
}

bool EyeLockThread::FindBestSpoofScore(int& position) {
	citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(
			outMsgQueue);
	SafeFrameMsg *msg = 0;
	float score = 0, minscore = -10;
	position = -1;
	for (int i = 0; i < m_outQSize; i++) {
		msg = sendIter.curr();
		msg->lock();
		if (msg->isUpdated()) {
			ELEyeMsg hMsg;
			hMsg.CopyFrom(*msg);

			if (hMsg.getMsgType() == IMG_MSG) {
//				hMsg.getfScore(score);
//				if (score > minscore)
				{
					m_bestEye.CopyFrom(*msg);
					minscore = score;
					position = i;
				}
			} else {
				// database reload request !
				EyelockLog(logger, DEBUG, "NON IMG Msg");
				m_bestEye.CopyFrom(*msg);
				minscore = 0;
				position = i;
				msg->unlock();
				FlushAll();
				break;
			}
		}
		msg->unlock();
		sendIter.next();
	}

	return (minscore > (-10));
}


void EyeLockThread::SetUpdatedFalse(int positon) {
	SafeFrameMsg *msg = outMsgQueue[positon];
	msg->setUpdated(false);
}

void EyeLockThread::FlushAll(void) {
	 if(m_ImageProcessor)
		 m_ImageProcessor->ClearEyes(0,true);

	for (int i = 0; i < m_outQSize; i++) {
		SafeFrameMsg *msg = outMsgQueue[i];
		msg->setUpdated(false);
	}
}

int EyeLockThread::GetAvailableImages(void) {
	int cnt = 0;
	for (int i = 0; i < m_outQSize; i++) {
		SafeFrameMsg *msg = outMsgQueue[i];
		if (msg->isUpdated())
			cnt++;
	}
	return cnt;
}

void EyeLockThread::SendDetectLED(void) {
	if(m_tsDestAddrpresent){
		LEDResult l;
		l.setState(LED_DETECT);
		m_nwLedDispatcher->enqueMsg(l);
	}
	if(m_Master){
		if (m_ledConsolidator)
			m_ledConsolidator->enqueMsg(m_ledMR);
	}
}

/*
 *  Added this callback to intercept images from slave process via NWListener.
 *  To be responsible to other listeners we should quickly copy the data and then
 *  attempt an enrollment.
 */

void EyeLockThread::process(Copyable *msg)
{
	// slave process sticks eye into enrollment queue
	EyelockLog(logger, DEBUG, "EyelockThread::process()");
	if(!m_Master)
	{
		HTTPPOSTMsg* pMsg=(HTTPPOSTMsg*)msg;
		NWMESSAGETYPE msgType = pMsg->getMsgType();		
	}
}

int EyeLockThread::RemoveUntrackedEyes(int lastindex,int debug) {
// Make a Array Spoof results.
	int k=0;
	int frindx = -1;
	int prev = -1;
	int eyeidx = -1;
	int frindx1 = -1;
	int prev1 = -1;
	int eyeidx1 = -1;

	//Get all the frame Index
	for (int i = 0; i < m_outQSize; i++) {
		SafeFrameMsg *msg = outMsgQueue[i];
		frindx = -1;
		prev = -1;
		eyeidx = -1;
		frindx1 = -1;
		prev1 = -1;
		eyeidx1 = -1;

		bool updated = false;
		msg->lock();
		updated = msg->isUpdated();
		if(updated) {
			ELEyeMsg hMsg;
			hMsg.CopyFrom(*msg);
			if (hMsg.getMsgType() == IMG_MSG) {
				frindx = hMsg.getFrameIndex();
				hMsg.getPrevIndex(prev);
				eyeidx =  hMsg.getEyeIndex();
			}
		}
		msg->unlock();
		if(updated){
			if(debug)EyelockLog(logger, DEBUG, "Eyes::%d -> %d %d %d",i,frindx,prev,eyeidx);
			// Loop again to figure out any msg which is tracked or untracked.
			int keep = false;
//			if((frindx != -1) && (prev != -1) && (eyeidx != -1))
			{
				for (int j = 0; j < m_outQSize; j++) {
					frindx1 = -1;
					prev1 = -1;
					eyeidx1 = -1;
					bool update1;
					SafeFrameMsg *msg1 = outMsgQueue[j];
					msg1->lock();
					update1 = msg1->isUpdated();
					if(update1) {
						ELEyeMsg hMsg1;
						hMsg1.CopyFrom(*msg1);
						if (hMsg1.getMsgType() == IMG_MSG) {
							frindx1 = hMsg1.getFrameIndex();
							hMsg1.getPrevIndex(prev1);
							eyeidx1 =  hMsg1.getEyeIndex();
						}
					}
					msg1->unlock();


					if(update1){
//						EyelockLog(logger, DEBUG, "%d %d %d -> %d %d %d",frindx,prev,eyeidx,frindx1,prev1,eyeidx1);
						if(((frindx1 == (frindx -1))&&(prev == eyeidx1))||((frindx1 == (frindx+1))&&(prev1 == eyeidx))){
							keep = true;
//							EyelockLog(logger, DEBUG, "Dont remove");
							break;
						}
					}
				}
			}

			if((!keep)){
				if(frindx < lastindex){
					EyelockLog(logger, DEBUG, "Removed %d %d ",frindx,eyeidx);
					msg->lock();
					msg->setUpdated(false);
					msg->unlock();
					k++;
				}
			}
		}
	}
	return k;
}


int EyeLockThread::ExtractFrameInfo(void) {
// Make a Array Spoof results.
	int k=0;
	//Get all the frame Index
	for (int i = 0; i < m_outQSize; i++) {
		SafeFrameMsg *msg = outMsgQueue[i];
		msg->lock();
		if(msg->isUpdated()) {
			ELEyeMsg hMsg;
			hMsg.CopyFrom(*msg);
			if (hMsg.getMsgType() == IMG_MSG) {
				int frindx = hMsg.getFrameIndex();
				if(k== 0){
					m_FrameInfo[k].m_frameIndex = frindx;
					m_FrameInfo[k].m_eyeCount = 1;
					k++;
				}else{
					bool found = false;
					for(int n=0;n<k;n++){
						if(m_FrameInfo[n].m_frameIndex == frindx){
							m_FrameInfo[n].m_eyeCount++;
							found= true;
							break;
						}
					}
					if(!found){
						m_FrameInfo[k].m_frameIndex = frindx;
						m_FrameInfo[k].m_eyeCount = 1;
						k++;
					}
				}
			}
		}
		msg->unlock();
	}

	m_FrameCount = k;
//	PrintFrameInfo();
	FrameInfo tempFrameInfo;

	for(int i=0;i<m_FrameCount-1;i++){
		for(int j=i+1;j<m_FrameCount;j++){
			if(m_FrameInfo[i].m_frameIndex > m_FrameInfo[j].m_frameIndex){
				tempFrameInfo = m_FrameInfo[i];
				m_FrameInfo[i] =  m_FrameInfo[j];
				m_FrameInfo[j] =  tempFrameInfo;
			}
		}
	}

	int cntr = 0;
	for(int i=0;i<m_FrameCount;i++){
		cntr += m_FrameInfo[i].m_eyeCount;
	}
	if(m_logging){
		if(cntr){
				struct timeval m_timer;
				gettimeofday(&m_timer, 0);
				TV_AS_USEC(m_timer,a);
				FILE *fp = fopen("dump.txt","a");
				int le =0;
				fprintf(fp,"EYEDETECTED;%llu;%d;\n",a,cntr);
				fclose(fp);
		 }
	}
	return m_FrameCount;
}

void EyeLockThread::PrintFrameInfo(void) {
	EyelockLog(logger, DEBUG, "Frame Info:: %d",m_FrameCount);
	for(int i=0;i< m_FrameCount;i++){
		EyelockLog(logger, DEBUG, " %d -> %d",m_FrameInfo[i].m_frameIndex,m_FrameInfo[i].m_eyeCount);
	}
}

bool EyeLockThread::SpoofFlow(){
	bool ret = false;
	if(m_shouldProcess){
		DetectEyes();
		if(m_spoofEnable && m_shouldProcess &&(!m_pEyelockMessageHandler->getHaloDCState())){
			if(m_trackingEnabled){
				int nontracked = RemoveUntrackedEyes(0x7fffffff, 0);
				if(nontracked){
					EyelockLog(logger, DEBUG, "Removed Eyes %d",nontracked);
				}
			}
		}
		ExtractFrameInfo();

		if(m_FrameCount == 0){
			CURR_TV_AS_USEC(t);
			//EyelockLog(logger, DEBUG, "%llu > %llu",t,m_LastDetectedEyeTS);
			if((t > m_LastDetectedEyeTS) &&(m_LastDetectedEyeTS>0)){
				// Lets make the max spec value to detect mode.
				if(m_Master && m_ledPWMSwitching && m_currentState){
					m_currentState = false;
					//EyelockLog(logger, DEBUG, "Lets Set I2C value to Normal");
					m_ImageProcessor->ChangeShiftandOffset(m_currentState);
					BinMessage msg(256);
					m_ImageProcessor->GenMsgToNormal(msg);
					SendMessageToSlave(msg);
					m_LastDetectedEyeTS = 0;
				}
			}
		}else{
			CURR_TV_AS_USEC(t);
			m_LastDetectedEyeTS = t;
			m_LastDetectedEyeTS += m_TimeToNormalusec;
			//EyelockLog(logger, DEBUG, "Normal Time %llu",m_LastDetectedEyeTS);
		}
		MatchEyesWithIntraSpoofComputation();
		FlushAll();
		m_ImageProcessor->ClearSpoofQ();

		if(m_pEyelockMessageHandler->getHaloDCState()){
			ShiftHaloAndDC testval = m_pEyelockMessageHandler->getHaloDC();
			EyelockLog(logger, DEBUG, "Got a Halo Conf Msg  %d %d %f %d",testval.dcoffset,testval.shift,testval.halothreshold,testval.maxspecval);
			m_ImageProcessor->SetHaloRelatedConf(testval.dcoffset,testval.shift,testval.halothreshold,testval.maxspecval);
			m_pEyelockMessageHandler->ResetHaloDC();
		}
	}else{
		EyelockLog(logger, DEBUG, "EyeLockThread:: RESET start AGAIN the flow");
		usleep(5000);//Sleep for 5 msec to give other thread chance to work
		m_MatchProcessor->ClearNwMatcherQ();
		FlushAll();// clear the eyes queue which were grabbed.Imageprocessor output queue
		m_MatchProcessor->FlushQueue(true);
		m_shouldProcess = true;
	}
	return ret;
}

void EyeLockThread::PrintTrackerResult(SpoofTrackerResult* ptr, int cnt){
	EyelockLog(logger, DEBUG, "Tracker Result");
	for(int i=0;i<cnt;i++){
		EyelockLog(logger, DEBUG, "[%d %d %d]->[%d %d %d]",ptr[i].m_frIdx1,ptr[i].m_eyIdx1,ptr[i].m_score1,ptr[i].m_frIdx2,ptr[i].m_eyIdx1,ptr[i].m_score2);
	}
}

bool EyeLockThread::CheckTrackedCount(){
	bool ret = false;
	if(m_spoofEnable){
		if(m_ImageProcessor->GetTrackedCount() < m_MinTrackedCount)
			ret = true;
		else
			ret = false;
	}
	return ret;
}

void EyeLockThread::SendMessageToSlave(BinMessage& inpmsg){
	if(m_Master)
	{
		for(int i = 0; i < m_SlaveAddressList.size(); i++)
		{
			try{
				if(m_SlaveAddressList[i]){
					struct timeval timeOut;
					timeOut.tv_sec = 0;
					timeOut.tv_usec = 200 * 1000; // convert milliseconds to microseconed
					EyelockLog(logger, DEBUG, "SEND MESSAGE TO CLIENT => ");
					char *ptr = inpmsg.GetBuffer();
					for(int k=0;k<30;k++)
						EyelockLog(logger, DEBUG, "%c",*ptr++);
					EyelockLog(logger, DEBUG, "\n");
					fflush(stdout);
					SocketClient client=m_socketFactory->createSocketClient("Eyelock.MasterSlaveCommSecure");
					client.SetTimeouts(timeOut);
					client.ConnectByHostname(*m_SlaveAddressList[i]);
					client.SendAll(inpmsg,MSG_DONTWAIT);
				}
			}
			catch(Exception& nex)
			{
				EyelockLog(logger, ERROR, "Slave mode exception -=-------------------------------"); fflush(stdout);
				nex.PrintException();
			}
			catch(const char *msg)
			{
				EyelockLog(logger, ERROR, "Slave mode exception -=--------------------------------"); fflush(stdout);
				std::cout<< msg <<endl;
			}
			catch(...)
			{
				EyelockLog(logger, ERROR, "Unknown exception during slave process mode update");
				std::cout<< "Unknown exception during slave process mode update" <<endl;
			}
		}
	}
}

void EyeLockThread::DetectEyes() {
//	if (m_Debug)
//		EyelockLog(logger, DEBUG, "Grab the images");

	int frcnt = 0;
	int firstimg = -1;
#if 1
	while ((!IsQueueFull()) && (frcnt < m_MaxFrames) && m_shouldProcess && (!m_pEyelockMessageHandler->getHaloDCState()))
#else
	while (IsQueueEmpty())
#endif
	{
		bool test = false;
		bool match = false;
		if(m_Mode == EyelockMatcher || m_Mode == EyelockDual ){
			match = true;
		}
		XTIME_OP("ImageProcessor", test = m_ImageProcessor->process(match));
		usleep(m_sleepTimeFlow/10);

		frcnt++;
		if(firstimg == -1){
			if(test){
				firstimg = frcnt;
				frcnt = 0;
				//EyelockLog(logger, DEBUG, "Started with index %d %d ", m_ImageProcessor->GetFaceIndex(),frcnt);
				//Try to change the shift and the illuminator.
				if(m_Master && m_ledPWMSwitching && (!m_currentState)){
					m_currentState = true;
					//EyelockLog(logger, DEBUG, "Lets Set I2C value to AD ");
					m_ImageProcessor->ChangeShiftandOffset(m_currentState);
					BinMessage msg(256);
					m_ImageProcessor->GenMsgAfterDetect(msg);
					SendMessageToSlave(msg);
					CURR_TV_AS_USEC(t);
					m_LastDetectedEyeTS = t;
					m_LastDetectedEyeTS += m_TimeToNormalusec;
				}
			}
		}
		if (m_Debug) {
			EyelockLog(logger, DEBUG, "%d %d Num Eyes in Queue %d %llu", frcnt,firstimg,GetAvailableImages(),m_ImageProcessor->getTS());
		}
		if(ShouldIQuit())
			return;
	}
	//SendHBStatus();
	bool test = false;
	if((!m_pEyelockMessageHandler->getHaloDCState())){
		test = m_ImageProcessor->SendSpoofCheckEyes(test,0x7fffffff);
	}
	//Clear the Detected Q
	m_ImageProcessor->ClearEyes(0x7fffffff,true);
	if((!m_pEyelockMessageHandler->getHaloDCState())){
		if (!IsQueueEmpty()) {
			if (m_Debug)
				EyelockLog(logger, DEBUG, "Sending Detect LED");
			SendDetectLED();
		}
	}
	usleep(m_sleepTimeFlow);
}

void EyeLockThread::MatchEyesWithIntraSpoofComputation() {
	citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(outMsgQueue);
	SafeFrameMsg *msg = 0;
	bool test = false;
	for(int i=0;i< m_FrameCount;i++){
		for(int j=0;j< m_FrameInfo[i].m_eyeCount;j++){
			bool test= false;
			for (int k = 0; k < m_outQSize; k++) {
				msg = sendIter.curr();
				msg->lock();
				if(msg->isUpdated()) {
					ELEyeMsg hMsg;
					hMsg.CopyFrom(*msg);
					if ((hMsg.getMsgType() == IMG_MSG) && (hMsg.getFrameIndex() == m_FrameInfo[i].m_frameIndex) && (hMsg.getEyeIndex() == j)) {
						m_bestEye.CopyFrom(*msg);
						test = true;
					}
				}
				msg->unlock();
				sendIter.next();
			}

			if (((m_Mode == EyelockCamera) || (m_Mode == EyelockCamera2) || (m_Mode == EyelockDual))&&(!m_pEyelockMessageHandler->getHaloDCState())) {
				if (m_pEyeDispatcher&&test&& m_shouldProcess) {
					m_pEyeDispatcher->enqueMsg(m_bestEye);
				}
			}

			if ((m_Mode == EyelockEnroll) && (m_shouldSegment) )  {
				if (m_MatchProcessor&&test&&m_shouldProcess) {
					timeval t;
					gettimeofday(&t, 0);
					TV_AS_MSEC(t,t1);
					//EyelockLog(logger, DEBUG, "<<ENROLLMENT>> Segmenting frameIndex = %d eyeIndex = %d",m_bestEye.getFrameIndex(),m_bestEye.getEyeIndex());
					XTIME_OP("SEGMENTATION",
						m_MatchProcessor->DoSegmentation(&m_bestEye) );
					
					if(m_segmentationMiliSecSleep)
						usleep(m_segmentationMiliSecSleep);
				}
			}


			if (((((m_Mode == EyelockMatcher) || (m_Mode == EyelockDual)||(m_Mode == EyelockWakeUP))&&(m_shouldSegment)))&&(!m_pEyelockMessageHandler->getHaloDCState())) {
				if (m_MatchProcessor&&test&& m_shouldProcess) {
					timeval t;
					gettimeofday(&t, 0);
					TV_AS_MSEC(t,t1);
					//printf("%llu::Segmentation %d %d\n",t1,m_bestEye.getFrameIndex(),m_bestEye.getEyeIndex());
					XTIME_OP("SEGMENTATION",
							m_MatchProcessor->DoSegmentation(&m_bestEye)
							);
					if(m_segmentationMiliSecSleep)
						usleep(m_segmentationMiliSecSleep);
				}
			}
		}
		//Do the Matching for the segmented Eyes.
		if (m_MatchProcessor && !(m_Mode == EyelockEnroll)) {			
			if(m_shouldProcess && m_doMatching && (!m_pEyelockMessageHandler->getHaloDCState())){
				bool ret = m_MatchProcessor->CheckIrisFromSameFrame();
				if(!ret){
					m_MatchProcessor->SendIrisFromSameFrame();
				}else{
					EyelockLog(logger, DEBUG, "Spoof Detected in frame %d",m_bestEye.getFrameIndex());
				}
			}
			m_MatchProcessor->ClearIrisDataIndex();
		}
	}

	if (m_Mode == EyelockEnroll) {
//		if (m_MatchProcessor && m_shouldProcess) {
//			for(int i=0; i<m_MatchProcessor->getIrisDataIndex(); i++)
//			{
//				//EyelockLog(logger, DEBUG, " Value of i = %d",i);
//				if( (m_MatchProcessor->getIrisDataIndexElement(i))->getSegmentation() )
//					m_pEmbeddedEnrollment->pushIris( *(m_MatchProcessor->getIrisDataIndexElement(i)) );
//				else
//					EyelockLog(logger, DEBUG, "Iris not send to Enrollment Vector due to Bad Segmentation");
//			}
//			m_ResultBestPair = m_pEmbeddedEnrollment->getBestPairofEyes();
//			if(m_ResultBestPair.first && m_Debug)
//				EyelockLog(logger, DEBUG, " Sorting Result  First.FrameID = %d",m_ResultBestPair.first->GetFrameId());
//			if(m_ResultBestPair.second && m_Debug)
//				EyelockLog(logger, DEBUG, " Second.FrameID = %d",m_ResultBestPair.second->GetFrameId());
//
//			int flagDone = 0;
//			for (int k = 0; k < m_outQSize; k++) {
//				msg = sendIter.curr();
//				msg->lock();
//				if(msg->isUpdated()) {
//					ELEyeMsg hMsg;
//					hMsg.CopyFrom(*msg);
//					char camID[32];
//					if(m_ResultBestPair.first) {
//						if ((hMsg.getFrameIndex() == m_ResultBestPair.first->GetFrameId()) &&
//							(hMsg.getEyeIndex() == m_ResultBestPair.first->GetImageId()) ) {
//							if (hMsg.getCameraID(camID) && (atoi(camID) == m_ResultBestPair.first->GetCameraId()) ) {
//								//EyelockLog(logger, DEBUG, "CIterator k = %d hmsg.FrameID = %d First.FrameID = %d hmsg.CamID = %d First.CamID = %d",k,hMsg.getFrameIndex(),m_ResultBestPair.first->GetFrameId(),atoi(camID),m_ResultBestPair.first->GetCameraId());
//								m_BestFirstEye.CopyFrom(*msg);
//								flagDone++;
//								if (m_pEyeDispatcher && m_shouldProcess) {
//									m_pEyeDispatcher->enqueMsg(m_BestFirstEye);
//								}
//							}
//						}
//					}
//					else if(m_ResultBestPair.second) {
//							if ((hMsg.getFrameIndex() == m_ResultBestPair.second->GetFrameId()) &&
//								(hMsg.getEyeIndex() == m_ResultBestPair.second->GetImageId()) ) {
//								if (hMsg.getCameraID(camID) && (atoi(camID) == m_ResultBestPair.second->GetCameraId()) ) {
//									//EyelockLog(logger, DEBUG, "CIterator k = %d hmsg.FrameID = %d Second.FrameID = %d",k,hMsg.getFrameIndex(),m_ResultBestPair.second->GetFrameId());
//									m_BestSecondEye.CopyFrom(*msg);
//									flagDone++;
//								}
//						}
//					}
//				}
//				msg->unlock();
//				sendIter.next();
//				if (flagDone==2) break;
//			}
//			m_pEmbeddedEnrollment->clearEnrollmentVector();
//			m_MatchProcessor->ClearIrisDataIndex();
//		}
	}

	FlushAll();
}


void EyeLockThread::MatchBestEyes() {

	bool matched = false;
	while (!IsQueueEmpty()) {
		int pos = -1;
		SendHBStatus();
		unsigned char *pImage = 0; // used in case of enrollment
		bool test= false;

		if(m_spoofEnable)
			test = FindBestSpoofScore(pos);
		else
			test = FindBest(pos);

		if(test){
//			EyelockLog(logger, DEBUG, "FindBest => found an eye!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1"); fflush(stdout);

			if ((m_Mode == EyelockCamera) || (m_Mode == EyelockCamera2) || (m_Mode == EyelockDual)) {
				if (m_pEyeDispatcher) {
					m_pEyeDispatcher->enqueMsg(m_bestEye);
				}
			}

			if ((m_Mode == EyelockMatcher) || (m_Mode == EyelockDual)) {
				if (m_MatchProcessor) {
					//TIME_OP("MatchProcessor", m_MatchProcessor->process(&m_bestEye));
					matched = m_MatchProcessor->DoMatch(&m_bestEye);
					if(matched){
						m_MatchProcessor->Matched();
						break;
					}
				}
			}
		}

		if (pos >= 0)
			SetUpdatedFalse(pos);
	}

	if (m_Debug)
		EyelockLog(logger, DEBUG, "Flush the Eyes");

	if (matched) {
		FlushAll();
#if 1
		if(m_pEyeDispatcher) {
			m_pEyeDispatcher->FlushAll();
		}
#endif
	}
}

unsigned int EyeLockThread::ChangeProcessingMode(EyelockProcessMode mode, const char *address)
{
	if(m_Master){
		// EyEnroll can set either EyelockCamera or EyelockMatcher
		// EyEnroll is not aware of current mode, so basing on this check return to EyelockSleep after enrollment
		if((m_PreviousMode == EyelockSleep && mode == EyelockMatcher)){
			mode = EyelockSleep;
		}
	}
	m_PreviousMode = m_Mode;
	m_Mode = mode;

	m_pEyeDispatcher->ClearAddressList(); // Remove first address list (locking)
	if(address != 0)
	{
		m_pEyeDispatcher->AddAddress(address);
	}
	MT9P001FrameGrabber *pFrameGrabber = dynamic_cast<MT9P001FrameGrabber*>(m_ImageProcessor->GetFrameGrabber());

	if((EyelockSleep == m_Mode))
	{
		m_ImageProcessor->setShouldDetectEyes(false);
	}
	else
	{
		m_ImageProcessor->setShouldDetectEyes(true);
	}

	m_ImageProcessor->clearFrameBuffer();

	if(m_Master)
	{
		FormatProcessModeMessage(m_Mode, address);
		SendMessageToSlave(m_ProcessModeMsg);
	}

	EyelockLog(logger, DEBUG, "Eyelock processing mode change: %d; flush buffers...", (int)m_Mode); fflush( stdout);

	FlushAll();
	if(m_pEyeDispatcher) {
		m_pEyeDispatcher->FlushAll();
	}

	if(pFrameGrabber)
	{
		if(m_Mode == EyelockCamera2)
		{	EyelockLog(logger, DEBUG, "Set index: 1");fflush(stdout);
			pFrameGrabber->SetRecipeIndex(1);
			pFrameGrabber->SetDoAlternating(false);
		}
		else
		{   EyelockLog(logger, DEBUG, "Set index: 0");fflush(stdout);
			pFrameGrabber->SetRecipeIndex(0);
			pFrameGrabber->SetDoAlternating(true);
		}
	}
	return 0;
}

void EyeLockThread::ProcessEyelockMode(){
    std::string peer;
    EyelockProcessMode mode = EyelockDual;
    if(m_pEyelockMessageHandler->GetMode(mode, peer)) // reports true if updated
	{
		const char *address = (peer.length() > 0) ? peer.c_str() : NULL;
		EyelockLog(logger, DEBUG, "mode=%d; peer=%s", mode, address); fflush(stdout);
		ChangeProcessingMode(mode, address);
		m_pEyelockMessageHandler->Reset();

		// change LED in dual authentication
		if ((m_dualAuth || m_transTOC) && m_ledConsolidator) {
			LEDResult l;
			if (mode == 0) {
				// Enroll - set LED white
				l.setState(LED_DUAL_AUTHN_CARD);
				m_ledConsolidator->enqueMsg(l);
			}
			else {
				// set LED off
				l.setState(LED_INITIAL);
				m_ledConsolidator->enqueMsg(l);
			}
		}

	}
}


unsigned int EyeLockThread::MainLoop() {

	std::string name = "EyeLockThread::";
	try {
#ifdef IRIS_CAPTURE
	struct timeval start;
	if(m_bIrisCaptureEnabled)
	{
//	DMOOUT - should be started arlready by EyelockMain...	m_HttpPostSender->init();
//		m_HttpPostSender->Begin();
		gettimeofday(&start, 0);
	}

#endif		

		while (!ShouldIQuit())
		{
			if (m_pEyelockMessageHandler)
			{
				ProcessEyelockMode();
			}

			if(m_ImageProcessor->ShouldDetectEyes())
			{
				if(m_Mode != EyelockSleep){
					bool ret = SpoofFlow();
				}
				//else
				usleep(m_sleepTimeFlow);
				if(ShouldIQuit())
					break;

				HTTPPOSTMsg *msg = (HTTPPOSTMsg *) getNextMsgToProcess();
				if (msg && ((m_Mode == EyelockMatcher || m_Mode == EyelockDual)))
				{
					m_MatchProcessor->process(msg);
				}
			}
			else
			{
				if (m_Mode != EyelockSleep) // skipping image processing in EyelockSleep mode
				{
					m_ImageProcessor->process(); // increment m_FaceIndex then call sendLiveImages()
				}
				citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(outMsgQueue);
				SafeFrameMsg *msg = 0;
				for (int i = 0; i < m_outQSize; i++) {
					msg = sendIter.curr();
					msg->lock();
					//if any one updated was false means its not full
					if (msg->isUpdated()) {
						m_pEyeDispatcher->enqueMsg(*msg);
						msg->setUpdated(false);
					}
					msg->unlock();
					sendIter.next();
				}
			}
#ifdef IRIS_CAPTURE
			if(m_bIrisCaptureEnabled)
			{
				struct timeval end;
				gettimeofday(&end, 0);
				long mtime, seconds, useconds;
				seconds  = end.tv_sec  - start.tv_sec;
				useconds = end.tv_usec - start.tv_usec;
				mtime = ((seconds) + useconds/1000000.0);
				if(mtime > m_HeartBeatFrequency){
  					m_HttpPostSender->enqueSignalHeartbeat();
					start = end;
				}
			}
#endif
			Frequency();
		}

	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}

	return 0;
}

// blocks till it gets a new message
Copyable *EyeLockThread::getNextMsgToProcess() {

	//EyelockLog(logger, DEBUG, ".");
	bool bFound = false;
	while (!bFound && !ShouldIQuit()) {
		Safe<Copyable *> & currMsg = m_sendIter.curr();
		currMsg.lock();
		if (currMsg.isUpdated()) {
			m_result->CopyFrom(*currMsg.get()); // make a copy
			currMsg.setUpdated(false);//empty
			bFound = true;
		}
		currMsg.unlock();
		if (bFound) {
			m_inQueue.decrCounter();
		} else {
			break;
		}
		m_sendIter.next();
	}
	if (bFound) {
		return m_result;
	}
	return 0;
}

