#define VERSION "v1.46"
/*
 * main.cpp
 *
 *  Created on: 19 Feb, 2009
 *      Author: akhil
 */

#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <string>
#ifndef __HAVE_ANDROID_WATCHDOG__
#include <linux/watchdog.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>


#include "FileConfiguration.h"
#include "ImageProcessor.h"
#include "socket.h"
#include "SafeFrameMsg.h"
#include "CircularAccess.h"
#include "NWHDMatcher.h"

//using namespace eyelock;

class Main {
private:
	ImageProcessor *processor;
	pthread_t nwThread;
	pthread_t hdThread;
	pthread_cond_t data_avlbl_cv;
	pthread_mutex_t data_avlbl_mutex;

    int m_watchDogTimeOutInSecs;
    bool m_watchDogIgnoreCtrlC;

    //watchdog
    void set_wd_counter();
    void reset_wd_counter();
    static int watchdogfd;
    static struct sigaction sa;
    static void safe_exit(int);

	void startNetworkListener();
	void startHDListener();
public:
	CircularAccess<SafeFrameMsg *> outMsgQueue;
	SafeFrameMsg m_DetectedMsg;
	SafeFrameMsg m_MotionMsg;

	FileConfiguration conf;
	int bufSize;
	int outQSize;
	long nwThreadSleepUSec;
	struct timeval timeuSec;
	struct timespec timenSec;
	unsigned long timeOutuSec;
	static volatile bool m_shouldQuit;
	bool m_EnableHD;
public:
	Main() :
		conf("GRIDemo.ini"), outQSize(5),m_DetectedMsg(16),m_MotionMsg(16),m_EnableHD(false){
		m_shouldQuit=false;
		timeOutuSec=1000*conf.getValue("GRI.NwThreadSleepTimeMs",200);
		const char* detectedMsg=conf.getValue("GRI.DetectedLEDMsg","Name;0");
		const char* motionMsg=conf.getValue("GRI.MotionMsg","MOTION");

		m_DetectedMsg.SetData(detectedMsg,strlen(detectedMsg));
		m_MotionMsg.SetData(motionMsg,strlen(motionMsg));

		processor = new ImageProcessor(&conf,outMsgQueue,m_DetectedMsg,m_MotionMsg);
		bufSize = conf.getValue("GRI.sendBufferSize", processor->defBuffSize);
		nwThreadSleepUSec=1000*conf.getValue("GRI.nwThreadSleepMS", 100);
		outQSize=conf.getValue("GRI.outQSize",outQSize);
		outMsgQueue(outQSize);
		for (int i = 0; i < outQSize; i++) {
			outMsgQueue[i] = new SafeFrameMsg(bufSize);
		}


		m_watchDogTimeOutInSecs=conf.getValue("GRI.WatchDogTimeout",0);
		m_watchDogIgnoreCtrlC=conf.getValue("GRI.WatchDogIgnoresCtrl_C",false);

		// if a watchdog Timer is specified set it to reboot the system
		if(m_watchDogTimeOutInSecs>0) {
			set_wd_counter();
		}
		m_EnableHD = conf.getValue("GRI.EnableHDMatcher",false);

		pthread_mutex_init(&data_avlbl_mutex,NULL);
		pthread_cond_init(&data_avlbl_cv,NULL);

	}
	~Main() {
		for (int i = 0; i < outQSize; i++) {
			delete outMsgQueue[i];
		}
	}

	void run() {
		startNetworkListener();
		if(m_EnableHD){
			printf("HDMatcher Enabled\n");
			startHDListener();
		}
		do {

			try {
				reset_wd_counter();
				bool test=false;
				XTIME_OP("ImageProcessor",
				test = processor->process()
				);

				if(test){
					// we wrote new data to the queue hence lets tell the
					// nw dispatcher
					pthread_mutex_lock(&data_avlbl_mutex);
					pthread_cond_signal(&data_avlbl_cv);
				}
			} catch (const char *msg) {
				printf("Exception:%s\n", msg);
			} catch (...) {
				printf("Unknown Exception at %s:%d\n", __FILE__, __LINE__);
			}
			pthread_mutex_unlock(&data_avlbl_mutex);
		} while (true);
		// we dont have to make this thread sleep because frame grabbing blocks which gives CPU cycles to other threads
	}

	// called by network thread
	bool waitForDataWhenEmptyQueue(){
		bool shouldSleep=false;
		pthread_mutex_lock(&data_avlbl_mutex);
		shouldSleep=outMsgQueue.isEmpty();
		if(shouldSleep){
			gettimeofday(&timeuSec,0);
			timeuSec.tv_usec+=timeOutuSec;
			if(timeuSec.tv_usec>1000000){
				timeuSec.tv_sec+=1;
				timeuSec.tv_usec-=1000000;
			}
			timenSec.tv_sec=timeuSec.tv_sec;
			timenSec.tv_nsec=timeuSec.tv_usec*1000;

			printf(" z");
			pthread_cond_timedwait(&data_avlbl_cv,&data_avlbl_mutex,&timenSec);
		}
		pthread_mutex_unlock(&data_avlbl_mutex);

		return shouldSleep;
	}

};
////blocks until connected
//void connect(SocketClient& client, HostAddress& haddr) {
//	bool bConnected = false;
//	while (!bConnected) {
//		try {
//			client.IgnoreSigPipe();
//			client.Connect(haddr);
//			bConnected = true;
//		} catch (NetException& nex) {
//			nex.PrintException();
//			sleep(100); // there is some network issue, try after some-time
//		}
//	}
//}

bool getDetectMsgToSend(Main *srv, SafeFrameMsg& out_msg)
{
	bool bFound=false;
	// check the special DetectedLEDMsg
	srv->m_DetectedMsg.lock();
	if(srv->m_DetectedMsg.isUpdated()){
		out_msg.CopyFrom(srv->m_DetectedMsg); // make a copy
		srv->m_DetectedMsg.setUpdated(false);//sent
		bFound=true;
	}
	srv->m_DetectedMsg.unlock();
	return bFound;
}

bool getMotionMsgToSend(Main *srv, SafeFrameMsg& out_msg)
{
	bool bFound=false;
	// check the specialMotionMsg
	srv->m_MotionMsg.lock();
	if(srv->m_MotionMsg.isUpdated()){
		out_msg.CopyFrom(srv->m_MotionMsg); // make a copy
		srv->m_MotionMsg.setUpdated(false);//sent
		bFound=true;
	}
	srv->m_MotionMsg.unlock();
	return bFound;
}

//blocks until it finds an updated message
int getNextMsgToSend(Main *srv, SafeFrameMsg& out_msg, citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *>& sendIter) {

	if(getDetectMsgToSend(srv,out_msg)) return 1;
	if(getMotionMsgToSend(srv,out_msg)) return 0;

	bool bFound= false;
	bool ret = false;

	// now continue with the usual queue processing

	SafeFrameMsg *sendMsg=0;

	while(!bFound)
	{
		sendMsg=sendIter.curr();
		sendMsg->lock();
		if(sendMsg->isUpdated()){
			out_msg.CopyFrom(*sendMsg); // make a copy
			sendMsg->setUpdated(false);//sent
			bFound=true;
		}
		sendMsg->unlock();
		if(!bFound){
			if(srv->waitForDataWhenEmptyQueue()){
				bFound = getDetectMsgToSend(srv,out_msg);
				ret = getMotionMsgToSend(srv,out_msg);
				bFound = bFound || ret;
			}
		}
		else
		{
			// since we got an image message we reduce our counter
			srv->outMsgQueue.decrCounter();
		}
		sendIter.next();
	}
	if(ret) return 0;
	return 1;
}

void SendMessage(HostAddress &add,timeval &sendtimeOut,timeval &conntimeOut,SafeFrameMsg& out_msg,bool bblock=true){

	SocketClient client;

	client.SetTimeouts(conntimeOut);
	client.Connect(add);
	client.SetTimeouts(sendtimeOut);
	if(bblock){
		client.SendAll(add,out_msg);
	}
	else{
		client.SendAll(add,out_msg,MSG_DONTWAIT);
	}

}

void *networkDispatcher(void *args) {
	Main *srv = (Main *) (args);
	const char *svrAddr = srv->conf.getValue("GRI.serverAddress",
			"192.168.10.2:8081");
	const char *MotsvrAddr = srv->conf.getValue("GRIMotion.ServerAddress",
				"192.168.10.102:8081");

	const char *svr1Addr = srv->conf.getValue("GRI.serverAddress1","NONE");
	if(strcmp(svr1Addr,"NONE")==0) svr1Addr =0;

	int timeOutConnms = srv->conf.getValue("GRI.socketConnectTimeOutMillis", 0);
	int timeOutSendms = srv->conf.getValue("GRI.socketSendingTimeOutMillis", 500);
	struct timeval timeOutConn,timeOutSend;
	timeOutConn.tv_sec = timeOutConnms / 1000;
	timeOutConn.tv_usec = (timeOutConnms % 1000) * 1000;
	timeOutSend.tv_sec = timeOutSendms / 1000;
	timeOutSend.tv_usec = (timeOutSendms % 1000) * 1000;

	printf("Sending Motion %s\n",MotsvrAddr);

	HostAddress haddr(svrAddr);
	HostAddress haddr1(svr1Addr);
	HostAddress haddrMot(MotsvrAddr);

	SafeFrameMsg out_msg(srv->bufSize);
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(srv->outMsgQueue);
	while (!Main::m_shouldQuit) {
		try {

			int ret = getNextMsgToSend(srv, out_msg,sendIter);
			if( 0 == ret){
				SendMessage(haddrMot,timeOutSend,timeOutConn,out_msg);
			}
			else{
				XTIME_OP("SENDMSG",
						SendMessage(haddr,timeOutSend,timeOutConn,out_msg)
				);
				if(svr1Addr){
					SendMessage(haddr1,timeOutSend,timeOutConn,out_msg);
				}
			}
		} catch (NetException& nex) {
			nex.PrintException();
		} catch (const char *msg) {
			printf("Exception:%s\n", msg);
		} catch (...) {
			printf("Unknown Exception at %s:%d\n", __FILE__, __LINE__);
		}
	}
	return 0;
}

void Main::startNetworkListener() {
	pthread_create(&nwThread, NULL, ::networkDispatcher, (void *) (this));
}

void *hdDispatcher(void *args) {
	Main *srv = (Main *) (args);

	NWHDMatcher n(srv->conf);
	n.run();
}

void Main::startHDListener() {
	pthread_create(&hdThread, NULL, ::hdDispatcher, (void *) (this));
}

void Main::set_wd_counter()
{
	watchdogfd = open("/dev/watchdog", O_WRONLY);
	if(watchdogfd==-1){
		printf("\n Could not find watch dog timer\n");
	}
	else
	{
#ifndef __HAVE_ANDROID_WATCHDOG__
		ioctl(watchdogfd, WDIOC_SETTIMEOUT, &m_watchDogTimeOutInSecs);
#endif
		printf("Watchdog will reboot the system after %d secs of inactivity\n",m_watchDogTimeOutInSecs);

		if(!m_watchDogIgnoreCtrlC){
			sa.sa_handler = Main::safe_exit;
			sigaction(SIGINT, &sa, NULL);
			sigaction(SIGTERM, &sa, NULL);
		}
	}
}

void Main::reset_wd_counter()
{
	if(watchdogfd!=-1){
		int dummy;
#ifndef __HAVE_ANDROID_WATCHDOG__
	ioctl(watchdogfd, WDIOC_KEEPALIVE, &dummy);
#endif
	}
}

void Main::safe_exit(int)
{
	if (watchdogfd != -1) {
		write(watchdogfd, "V", 1);
		close(watchdogfd);
	}
	m_shouldQuit=true;
	exit(0);
}


int Main::watchdogfd=-1;
struct sigaction Main::sa;
volatile bool Main::m_shouldQuit=false;

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


int main(int count, char *strings[]) {

	if(count >1){
		if(strncasecmp(strings[1],"-v",2)==0){
			printf("%s\n Version %s\n",strings[0],VERSION);
		}
		else{
			printf("Usage:\n %s [-version]\n",strings[0]);
			printf("-version prints the version exits\n");
		}
	return 0;
	}
	printf("%s Version %s\n",strings[0],VERSION);	
#ifndef __BFIN__
	if (!acquireProcessLock(strings[0])) {
		printf("\nCould not lock, another instance may be running...Exiting!\n");
		return -1;
	}
#endif
	Main m;

	m.run();
	return 0;
}

