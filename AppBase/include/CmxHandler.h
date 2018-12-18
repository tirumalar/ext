/*
 * CmxHandler.h
 * Accepts connection over TCP/IP and saves data to the queue
 * Created on: 18 Aug, 2009
 *      Author: akhil
 */

#ifndef CMXHANDLER_H_
#define CMXHANDLER_H_

#include "HThread.h"
#include "HTTPPOSTMsg.h"
#include "CommonDefs.h"
#include "socket.h"
#include <map>
#include <string>
#include <vector>
#include "ProcessorChain.h"
#include "Synchronization.h"
#include "ImageProcessor.h"
#include <eyelock_com.h>

#ifdef HBOX_PG
#include <opencv/cv.h>
#include <opencv/cxcore.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "FlyCapture2.h"


#define CAMERA_POWER    0x610
#define PIO_DIRECTION 	0x11F8
#define TRIGGER_MODE 	0x830
#define SOFTWARE_TRIGGER_CAMERA
using namespace FlyCapture2;

#endif /* HBOX_PG */

#define WAIT_PONG_TIME 	1000	// in ms
#define SEND_PING_TIME 	3000	// in ms
#define IMAGE_SIZE		1152000	// 1200*960 in bytes
#define PKG_SIZE	1500	// 1400 bytes
//#define IMAGE_SIZE		2761728	// in bytes

enum CMXMESSAGETYPE {CMX_INIT_CMD, CMX_SEND_CMD, CMX_PING_CMD, CMX_LED_CMD, CMX_EYE_DETECT,CMX_MATCH,CMX_SOUND_CMD,CMX_MATCH_FAIL,CMX_NO_DETECT, CMX_PONG, CMX_IMAGE};

using namespace std;
//fwd decl
//class HostAddress;
class Configuration;
class SocketFactory;
class ImageProcessor;
struct PortServerInfo;

class CmxHandler: public HThread, public ProcessorChain{
#ifdef CMX_C1
public:
	CmxHandler(Configuration& pConf);
	virtual ~CmxHandler();
	virtual int End();

	virtual unsigned int MainLoop();
	const char *getName(){
		return "CmxHandler";
	}
	//bool do_serv_task(Socket & client);
	void HandleSendMsg(char *msg, unsigned short randomseed);
	void SetImageProcessor(ImageProcessor *ptr) {pImageProcessor = ptr;}
	void SetFaceTrackingQueue(OIMQueue * ptr) {m_pOIMQueue = ptr;}
	static void onConnect(Socket & client, void *arg);
	bool HandleReceiveImage(char *buffer, int length);
	int CreateUDPServer(int port);
	int CreateCMDTCPServer(int port);

	static int CheckTCPSocketForWriting(int fd);
	void DestroyCMDTCPServer();

	int m_debug;
	bool m_waitPong;
	int m_sock;
	unsigned long m_pingTime;
	pthread_t leftCThread;
	pthread_t rightCThread;
	pthread_t faceThread;

	pthread_t cmdserverThread;
	//Safe<SocketServer *> m_pSockLeftC;
	SocketFactory *m_socketFactory;

	ImageProcessor *pImageProcessor;
	OIMQueue *m_pOIMQueue;

	bool m_ImageAuthentication;
	unsigned short int m_Randomseed;
	unsigned short int GenerateSeed();
	unsigned short calc_syndrome(unsigned short syndrome, unsigned short p);
	void SetSeed(unsigned short sd);
private:
    bool HandleReceiveMsg(Socket & client);
    void SendMessage(char *out_msg, int len, unsigned short randomseed);
    bool do_serv_task(Socket& client);


    struct timeval m_timeOut,m_timeOutSend;

	//std::vector<HTTPPostMessageHandler *> m_MessageHandlers;
	pthread_t statusThread;
	HostAddress *m_resultDestAddr;
	BinMessage *m_rcvdMsg;
	//SocketClient m_cmxclient;         // add it in cmxHandler.h
	int m_exposureTime;
	int m_analogGain;

	PortServerInfo *leftCServerInfo;
	PortServerInfo *rightCServerInfo;

#endif	// CMX_C1
#ifdef HBOX_PG
public:
	int m_width;
	int m_height;
	int m_CameraID;
	bool m_RotateInImage;
	unsigned int m_numPGCameras;
	unsigned int m_SerialNoCamera1;
	unsigned int m_SerialNoCamera2;
	unsigned int m_SerialNoCamera3;
	IplImage *input_image;
	int CameraPowerOn(int CameraSerialNo);
	int CameraDisconnect();
	void CameraStatus(bool bStatus);
	IplImage* GrabFramePG(int CameraSerialNo);
	IplImage* GrabFramePGCAM0(int CamIndex);
	IplImage* GrabFramePGCAM1(int CamIndex);
	IplImage* GrabFramePGCAM2(int CamIndex);
	int CameraDisconnect(int CameraIndex);
	int Init();
	int m_SaveImages;
private:
	FlyCapture2::BusManager m_BusMgr;
	FlyCapture2::Error m_error;
	FlyCapture2::Camera m_Camera;
	FlyCapture2::CameraInfo m_CamInfo;
	FlyCapture2::Image rawImage;
	int m_PointGreyMode;
	int m_GPIOTriggerPin;
	int m_Trigger;
	int m_PulsePolarity;
	float m_Gain;
	int m_Shutter;
	float m_Brightness;
	bool m_bStatus;

	IplImage* ConvertImageToOpenCV(FlyCapture2::Image* pImage);
	void PrintError(FlyCapture2::Error error);
	void PrintCameraInfo(FlyCapture2::CameraInfo *pCamInfo);
	void PrintFormat7Capabilities(FlyCapture2::Format7Info fmt7Info);
	bool CheckSoftwareTriggerPresence(FlyCapture2::Camera *pCam);
	bool PollForTriggerReady(FlyCapture2::Camera *pCam);
	bool FireSoftwareTrigger(FlyCapture2::Camera *pCam);
	IplImage* transposeImage1(IplImage* image, int angle);

#endif

};

void *Images(void *arg);
void *pingStatus(void *arg);
void *leftCServer(void *arg);
#ifndef HBOX_PG
void *rightCServer(void *arg);
void *faceServer(void *arg);
#else
void *rightCamera(void *arg);
void *middleCamera(void *arg);
#endif
void *cmdServer(void *arg);
extern void DeleteSocketStream(SocketServer *&s);



#endif /* CMXHANDLER_H_ */
