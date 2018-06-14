#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/i2c-dev.h>
#include "include/BobListener.h"
#include "file_manip.h"

#include <termios.h>
#if defined(HBOX_PG) || defined(CMX_C1)
//#define I2C_BUS	"/dev/ttyUSB0"
#define I2C_BUS	"/dev/ttyACM0"
#else
#ifdef CMX_C1_OLD
#define I2C_BUS	"/dev/i2c-1"
#else
#define I2C_BUS	"/dev/i2c-3"
#endif
#endif /* HBOX_PG */

#define MIN(a,b) ( ((a)>(b)) ?(b):(a))
#define MAX_WRITE_LEN 		255
#define MAX_READ_LEN 		255
//#define POLL_TIME 		50000		// 50ms
//#define POLL_TIME 		5000		// 5ms
#define POLL_TIME 			10000		// 10ms
#define MAX_TIMER			10
#define MAX_MSG_SIZE		128

enum LogType{ TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE };
extern void EyelockEvent(const char * msg, ...);
extern void EyelockLog(const char *filename, int level, const char *fmt, ...);
const char logger[30] = "BobListener";

typedef struct _Timer
{
	unsigned long startTime;
	unsigned long endTime;
	int timeout;
	int repeat;
	int used;
	void (*BobTimerCB)();
	void *thread;
} BoB_Timer;

typedef struct _Relay
{
	unsigned long startRelayTime;
	unsigned long endRelayTime;
	int relayTimerSet;
} Relay;

BoB_Timer bobTimer[10];
int bobTimerNum = 0;
unsigned long m_currentTimeInMS = 0;
Relay m_Relay1, m_Relay2;

static unsigned short inputStat = 0;
static unsigned short inputAcs = 0xff;
static unsigned short inputTamper = 0;
static unsigned short outputAcs = 0;
static char osdpPanelMsg = 0;
static char osdpReaderMsg = 0;
static int txReadyStat = 0;
static char _i2c_bus[] = I2C_BUS;
static int acsResetCount = 4000;
static int acsResetSoundCount = 0;
static int acsResetSoundTime = 0;
static int currSoundTime = 0;
static int ShouldIQuit = 0;
static int ShouldIPollACS = 1;

uint64_t timestart = 0;
int m_i2cdev = 0;




void *pF2F = NULL;
void *pCardStat = NULL;

void (*BobInputCB)()= NULL;
//void (*BobTimerCB)()= NULL;
void (*BobRelayTimerCB)()= NULL;
int (*BobOsdpCB)() = NULL; //has void parameter for ODSP message, 128 bytes max
int (*BobReaderOsdpCB)() = NULL;
int internal_read_reg(int fd, unsigned char reg, unsigned int *val);
int internal_write_reg(int fd, unsigned char reg, unsigned int val);
int internal_read_array(int fd, unsigned char reg, char *buf, int len);
int internal_write_array(int fd, unsigned char reg, void *ptr, int len);
int i2c_start_transaction();
void DumpBuffer(char *buf, int bytes);
int myi2cdump(int len);


// IRIS Data Test
char readBuffer[6000];
char osdpCommand[MAX_MSG_SIZE];
static int osdpCommandLen = 0;
static int osdpResendCount = 0;
static int osdpResetCount = 0;

pthread_t com_thread = 0;
pthread_mutex_t lock;
int testB=0;
void bob_thread(void *arg)
{
//	return;
	long int tid = syscall(SYS_gettid);
	//EyelockLog(logger, INFO, "BoB => *** BobListener thread %ld ***",tid);

	printf("******************bob thread started**************\n");
	m_currentTimeInMS = 0;
	unsigned int stat = 0;
	memset(&bobTimer[0], 0, MAX_TIMER * sizeof(BoB_Timer));
	char buf[MAX_MSG_SIZE], readerBuf[MAX_MSG_SIZE];

	osdpResendCount = 0;

	pthread_mutex_init(&lock,NULL);
	//EyelockLog(logger, INFO, "BoB => BoB Thread Started");

	// read BoB version
	char sw_version[3] = {0}, hw_version[3] = {0};
	BobMutexStart();
	printf("trying to send command %d\n",BOB_SW_VERSION_OFFSET);
	BobReadArray(BOB_SW_VERSION_OFFSET, sw_version, 3);
	printf("received data %s\n",sw_version);
	BobReadArray(BOB_HW_VERSION_OFFSET, hw_version, 3);
	BobWriteReg(BOB_OSDP_STATUS_OFFSET, 0);
	BobMutexEnd();
	sleep(1);
#if defined(HBOX_PG) || defined(CMX_C1)
	system("rm BobVersion");
#else
	system("rm /home/root/BobVersion");
#endif	
	FILE *file = fopen("BobVersion", "w"); // write only
	if (file){
		fprintf(file, "ICM software version: %d.%d.%d\n", sw_version[0], sw_version[1], sw_version[2]);
		fprintf(file, "ICM hardware version: %x.%x.%x\n", hw_version[0], hw_version[1], hw_version[2]);
		printf("\n");
		printf("BoB => ICM software version: %d.%d.%d\n", sw_version[0], sw_version[1], sw_version[2]);
		printf("BoB => ICM hardware version: %x.%x.%x\n", hw_version[0], hw_version[1], hw_version[2]);
		printf("\n");
		fclose(file);
	}

	CURR_TV_AS_USEC(timecurr);
	timestart = timecurr;
	memset(readBuffer, 0, 6000);
	BobSetCardReadAck();

	memset(&m_Relay1, 0, sizeof(Relay));
	memset(&m_Relay2, 0, sizeof(Relay));

	while (!ShouldIQuit)
    {
		myi2cdump(250);
#if 1
		CURR_TV_AS_USEC(timecurr);
		unsigned int elapsed = timecurr-timestart;
#if defined(HBOX_PG) || defined(CMX_C1)
		unsigned int diff = (POLL_TIME > elapsed) ? (POLL_TIME - elapsed) : 100;
#else
		unsigned int diff = (POLL_TIME > (elapsed+100)) ? (POLL_TIME - elapsed) : 100;
#endif		
		usleep(diff);

		CURR_TV_AS_USEC(timenew);
		//printf("curr %llu start %llu\n", timenew, timestart);
		timestart = timenew;
#endif
		//testElapsedTime();
		//usleep(POLL_TIME);  //2 millisecond

		// update timer
		//m_currentTimeInMS += POLL_TIME/1000;

		m_currentTimeInMS = BobCurrentTimeInMS();
		if (bobTimerNum)
			BoBUpdateTimer();

		if (m_Relay1.relayTimerSet && m_currentTimeInMS >= m_Relay1.endRelayTime){
			EyelockLog(logger, DEBUG, "BoB => Relay1 timeout");
			BoBClearACSRelayOneOut();
			m_Relay1.relayTimerSet = 0;
		}
		if (m_Relay2.relayTimerSet && m_currentTimeInMS >= m_Relay2.endRelayTime){
			EyelockLog(logger, DEBUG, "BoB => Relay2 timeout");
			BoBClearACSRelayTwoOut();
			m_Relay2.relayTimerSet = 0;
		}


		// Reset ACS sound out and read LED
		if (acsResetSoundCount || currSoundTime)
		{
			currSoundTime -= POLL_TIME/1000;
			if (currSoundTime == 0){
				if (outputAcs & BOB_ACS_OUT_SOUNDER)
					BobClearACSOutput(BOB_ACS_OUT_SOUNDER);
				else
					BobSetACSOutput(BOB_ACS_OUT_SOUNDER);

				if ((!(outputAcs & BOB_ACS_OUT_LED_GRN)) && acsResetSoundTime == 250*1000/POLL_TIME){
					// Reset Red LED
					if (outputAcs & BOB_ACS_OUT_LED_RED)
						BobClearACSOutput(BOB_ACS_OUT_LED_RED);
					else
						BobSetACSOutput(BOB_ACS_OUT_LED_RED);
				}

				acsResetSoundCount--;
				if (acsResetSoundCount)
					currSoundTime = acsResetSoundTime;
			}
		}

		// get status of ACS - LED, Sound, Card Data, Tamper ...
		BobMutexStart();
		BobReadReg(BOB_STATUS_IN_OFFSET, &stat);
		BobMutexEnd();

		if (ShouldIPollACS){
			stat = stat | BOB_STATUS_IN_CHANGE | BOB_STATUS_IN_ACS | BOB_STATUS_IN_TAMPER;
		}
//if (stat)
	//printf("status 0x%x\n", stat);
		if (stat & BOB_STATUS_IN_CHANGE)
		{
			//EyelockLog(logger, DEBUG, "BoB => @@@ BoB state changed - Stat %x @@@",stat);
			inputStat = stat;
			if (inputStat & BOB_STATUS_IN_ACS)
			{
				stat = 0xff;
				// get ACS
				BobMutexStart();
				BobReadReg(BOB_ACS_INPUT_OFFSET, &stat);
				BobMutexEnd();
				inputAcs = ~stat & 0x7f;
			//	EyelockLog(logger, DEBUG, "BoB => inputAcs 0x%x", inputAcs);
				usleep(100);
			}

			if (inputStat & BOB_STATUS_IN_TAMPER)
			{
				stat = 0xff;
				// get ACS
				BobMutexStart();
				BobReadReg(BOB_TAMPER_INPUT_OFFSET, &stat);
				BobMutexEnd();
				inputTamper = stat;
			//	EyelockLog(logger, DEBUG, "BoB => inputTamper 0x%x", inputTamper);
				usleep(100);
			}

			// clear status
			BobMutexStart();
			BobWriteReg(BOB_STATUS_IN_OFFSET, 0);
			BobMutexEnd();

			if (BobInputCB && (inputStat & (BOB_STATUS_IN_CARD | BOB_STATUS_IN_ACS | BOB_STATUS_IN_TAMPER))) {
				BobInputCB();
			}
        }

		// get status of OSDP
		if((BobOsdpCB || BobReaderOsdpCB) && !ShouldIPollACS) {
			osdpResetCount++;
			osdpResendCount++;

			BobMutexStart();
			BobReadReg(BOB_OSDP_STATUS_OFFSET, &stat);
			BobMutexEnd();

			if (stat) {
				// EyelockLog(logger, TRACE, "BoB => Get OSDP status change 0x%x", stat);
				usleep(10);
				BobMutexStart();
				BobWriteReg(BOB_OSDP_STATUS_OFFSET, 0);
				BobMutexEnd();
				// Panel
				if(BobOsdpCB) {
					if(stat & BOB_OSDP_STAT_DATA_TO_READ) { //ODSP msg
						osdpPanelMsg = 1;
						int result = 1;
						int length = MAX_MSG_SIZE;

						//EyelockLog(logger, TRACE, "BoB => Read ODSP Message");
						//address
						BobMutexStart();
						BobReadReg(BOB_OSDP_DATA_LENGTH_OFFSET, &length);
						BobMutexEnd();
						if (length) {
							BobMutexStart();
							result = BobReadArray(BOB_OSDP_DATA_OFFSET, buf, length);
							BobMutexEnd();
							if (result == -1) {
								;//EyelockLog(logger, ERROR, "BoB => Read ODSP Message failed, length %d", length);
							}
							else {
								if (!BobOsdpCB((void*)buf, length)) {//whoever uses this buffer is responsible for freeing it.
									//BobWriteReg(BOB_OSDP_CMD_OFFSET, BOB_OSDP_CMD_READ_DATA);
									//EyelockLog(logger, ERROR, "BoB => BobOsdpCB() error - invalid OSDP message\n");
								}
							}
						}
					}
				}

				// Reader
				if(BobReaderOsdpCB) {
					if(stat & BOB_READER_OSDP_STAT_DATA_TO_READ) {
						osdpReaderMsg = 1;
						int result = 1;
						unsigned int readerLength = MAX_MSG_SIZE;
						//EyelockLog(logger, TRACE, "BoB => Read ODSP Reader Reply");
						BobMutexStart();
						BobReadReg(BOB_OSDP_READER_LENGTH_OFFSET, &readerLength);
						BobMutexEnd();
						if (readerLength) {
							BobMutexStart();
							result = BobReadArray(BOB_OSDP_READER_DATA_OFFSET, readerBuf, readerLength);
							BobMutexEnd();
							//printf("data length %d\n", readerLength);
							if (result == -1) {
								;//EyelockLog(logger, ERROR, "BoB => Read reader ODSP Message failed, length %d", readerLength);
							}
							else {
								if (!BobReaderOsdpCB((void*)readerBuf, readerLength))
									;//EyelockLog(logger, ERROR, "BoB => BobReaderOsdpCB() error - invalid OSDP message\n");
								else
									osdpResetCount = 0;
							}
						}
					}
				}
			}

			if (BobReaderOsdpCB && osdpResetCount > 1000*1000/POLL_TIME){	// if it responds exceed 1000ms, restart message // was 600ms for HBOX_PG
			//	printf("I2C buffer dump\n");
			//	system("./icm_communicator -i 1");
			//	usleep(100);
				EyelockLog(logger, ERROR, "BoB => reset reader OSDP msg");
				BobReaderOsdpCB((void*)readerBuf, 1);
				osdpResetCount = 0;
			}
#if 0
			if (BobReaderOsdpCB && osdpResendCount > 5000*1000/POLL_TIME){
				EyelockLog(logger, ERROR, "BoB => resend reader OSDP command, len %d", osdpCommandLen);
				DumpBuffer(osdpCommand, osdpCommandLen);
				BoBSendReaderOSDPCommand((void *)osdpCommand, osdpCommandLen);
				printf("\n************ reset count %d ***********\n\n", osdpResetCount);
			}
#endif
		}
		if(testB)
		{
			printf("**************can add code here**************8\n");

			if (BoBSetACSRelayOut(1) == 0)
			{
				// set relay timer
				BoBSetRelayTimerNew(1000, 1);
			}
			testB=0;
		}
    }

	EyelockLog(logger, INFO, "BoB => BoBThread stopped!");
}


void BobInitComs()
{
	EyelockLog(logger, DEBUG, "BoB => ***************Creating thread BobListener ********************* ");
	if (pthread_create (&com_thread, NULL, (void *) &bob_thread, NULL)) {
		fprintf(stderr, "Error creating thread BoBListener\n");
		//EyelockLog(logger, ERROR, "BoB => Error creating thread BoBListener!");
	}

}

void BobCloseComs()
{
	if (com_thread){
		pthread_join (com_thread, NULL);
		com_thread = 0;
		//EyelockLog(logger, INFO, "BoB => BoBThread killed!");
	}
}

void BobSetPollACS(int pollACS)
{
	ShouldIPollACS = pollACS;
}

void BoBSetQuit()
{
	ShouldIQuit = 1;
}
void BobSetInputCB(callback cb )
{
	//EyelockLog(logger, DEBUG, "BoB => set ACS call back\n");
	BobInputCB = cb;
}
void BobSetOsdpCB(callback2 cb )
{
	//EyelockLog(logger, DEBUG, "BoB => set OSDP call back\n");
	BobOsdpCB = cb;
}
void BobSetReaderOsdpCB(callback2 cb )
{
	//EyelockLog(logger, DEBUG, "BoB => set Reader OSDP call back\n");
	BobReaderOsdpCB = cb;
}

void BoBSetRelayTimer(int ms)
{
	BoBSetRelayTimerNew(ms, 1);
}

void BoBSetRelayTimerNew(int ms, int relay)
{
	Relay *ptr;
	if (relay == 1) {
		ptr = &m_Relay1;
	}
	else if (relay == 2) {
		ptr = &m_Relay2;
	}
	else
		return;

	ptr->relayTimerSet = 1;
	ptr->startRelayTime = m_currentTimeInMS;
	ptr->endRelayTime = ptr->startRelayTime + ms;
	//EyelockLog(logger, DEBUG, "BoB => BoBSetRelayTimer %d, current time %lu, end time %lu", ms, m_currentTimeInMS, m_endRelayTime);
}

void BobSetMyPtr(void *this)
{
	pF2F = this;
}

void BobSetMyCardStat(void *stat)
{
	pCardStat = stat;
}

void * BobGetMyPtr(){return pF2F;}

int BoBStatusChangeCard()
{
	return inputStat & BOB_STATUS_IN_CARD;
}
int BoBStatusChangeAcs()
{
	return inputStat & BOB_STATUS_IN_ACS;
}
int BoBStatusChangeTamper()
{
	return inputStat & BOB_STATUS_IN_TAMPER;
}
void setinputAcs(int val){
	inputAcs = val;
}
int BoBGetACSTamperIn(int bit)
{
	int mask = BOB_TAMPER_IN_REED_1 | BOB_TAMPER_IN_REED_2;
	int value = 0;
	if (inputTamper & mask)
		value = 1;
	else {	// check BOB_ACS_IN_TAMPER bit
		if (bit) {	// check high bit
			value = (inputTamper & BOB_TAMPER_IN_READER) ? 1 : 0;
		}
		else {	// check low bit
			value = ((inputTamper ^ BOB_TAMPER_IN_READER) & BOB_TAMPER_IN_READER) ? 1 : 0;
		}
	}

	//EyelockLog(logger, INFO, "Tamper Alarm => inputAcs 0x%x, value %d\n", inputAcs, value);
	return value;
}

void BoBLogACSTamperIn()
{
	;//EyelockLog(logger, INFO, "Tamper Alarm: 0x%x\n", inputTamper);
}

void BoBSetACSTamperPol(int polFlag, int reader)
{
	//EyelockLog(logger, INFO, "set Tamper Pol %d for %d", polFlag, reader);
	if (reader) {
		if (polFlag)
			BobSetACSOutput(BOB_ACS_OUT_TAMPER_READER);
		else
			BobClearACSOutput(BOB_ACS_OUT_TAMPER_READER);
	}
	else {
		if (polFlag)
			BobSetACSOutput(BOB_ACS_OUT_TAMPER_PANEL);
		else
			BobClearACSOutput(BOB_ACS_OUT_TAMPER_PANEL);
	}
}
int BoBGetACSFactoryReset()
{
	//return (inputAcs & BOB_ACS_IN_FACTORY_RESET) ? 1 : 0;
	return 0;
}
void BoBClearACSFactoryReset()
{
	BobMutexStart();
	BobWriteReg(BOB_ACS_INPUT_OFFSET, 0xFF);
	BobMutexEnd();
}
int BoBGetACSGreenLedIn()
{
	return (inputAcs & BOB_ACS_IN_LED_GRN) ? 1 : 0;
}
void BoBSetACSLedGreenOut()
{
	BobSetACSOutput(BOB_ACS_OUT_LED_GRN);
}
void BoBClearACSLedGreenOut()
{
	BobClearACSOutput(BOB_ACS_OUT_LED_GRN);
}

int BoBGetACSRedLedIn()
{
	return (inputAcs & BOB_ACS_IN_LED_RED) ? 2 : 0;
}
void BoBSetACSLedRedOut()
{
	BobSetACSOutput(BOB_ACS_OUT_LED_RED);
}
void BoBClearACSLedRedOut()
{
	BobClearACSOutput(BOB_ACS_OUT_LED_RED);
}
int BoBGetACSSoundIn()
{
	return (inputAcs & BOB_ACS_IN_SOUNDER) ? 1 : 0;
}
void BoBSetACSSoundOut(int time, int count)
{
	if (BobSetACSOutput(BOB_ACS_OUT_SOUNDER) == 0)
	{
		acsResetSoundTime = time;
		currSoundTime = time;
		acsResetSoundCount = count;
	}

}
int BoBSetACSRelayOut(int relay)
{
	if (relay == 1)
		return BobSetACSOutput(BOB_ACS_OUT_RELAY_1);
	if (relay == 2)
		return BobSetACSOutput(BOB_ACS_OUT_RELAY_2);
}

#if defined(HBOX_PG) || defined(CMX_C1)
void BoBClearACSRelayOneOut()
{
	BobClearACSOutput(BOB_ACS_OUT_RELAY_1);
}
#else
void BoBClearACSRelayOneOut()
{
	BobClearACSOutput(BOB_ACS_OUT_RELAY_1);
}
#endif

void BoBClearACSRelayTwoOut()
{
	BobClearACSOutput(BOB_ACS_OUT_RELAY_2);
}
void BoBClearACSSoundOut()
{
	BobClearACSOutput(BOB_ACS_OUT_SOUNDER);
}
int BobReadReg(unsigned char reg, unsigned int *val)
{
	int fd = i2c_start_transaction();
	if(fd == 0)
    {
		EyelockLog(logger, ERROR, "BobReadReg:BoB => Error starting interface");
        return -1;
    }
	int result = internal_read_reg(fd, reg, val);
	//close(fd);
	return result;

}

#if defined(HBOX_PG) || defined(CMX_C1)
int internal_read_reg(int fd, unsigned char reg, unsigned int *val)
{
	int result;
	unsigned char buff[5];

	buff[0] = 56;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = 0x01;

	result = write(fd, buff, 4);
	if (result != 4) {
		//EyelockLog(logger, ERROR, "BoB => write error in internal_read_reg() - write %s, result %d", strerror(errno), result);
		//perror("write");
		return -1;
	}

	buff[0] = 0;
    //usleep(100);
	usleep(50);
	result = read(fd, buff, 5);
	if (result != 5) {
		//EyelockLog(logger, ERROR, "BoB => read error in internal_read_reg() - read %s, result %d", strerror(errno), result);
		//perror("read");
		return -1;
	}

	if (val)
		*val = buff[4];
	return 0;
}
#else
int internal_read_reg(int fd, unsigned char reg, unsigned int *val)
{
	int result;
	unsigned char buff[2];

	buff[0] = 0;
	buff[1] = reg;
	result = write(fd, buff, 2);
	if (result != 2) {
		EyelockLog(logger, ERROR, "BoB => write error in internal_read_reg() - write %s, result %d", strerror(errno), result);
		perror("write");
		return -1;
	}

	buff[0] = 0;
    //usleep(100);
	usleep(50);
	result = read(fd, buff, 1);
	if (result != 1) {
		EyelockLog(logger, ERROR, "BoB => read error in internal_read_reg() - read %s, result %d", strerror(errno), result);
		perror("read");
		return -1;
	}

	if (val)
		*val = buff[0];
	return 0;
}
#endif
int BobReadArray(unsigned char reg, char *buf, int len)
{
	int fd = i2c_start_transaction();
	if(fd == 0)
    {
		EyelockLog(logger, ERROR, "BobReadArray:BoB => Error starting interface");
        return -1;
    }
	int result = internal_read_array(fd, reg, buf, len);
	//close(fd);
	return result;

}
#if defined(HBOX_PG) || defined(CMX_C1)
int internal_read_array(int fd, unsigned char reg, char *buf, int len)
{
	int result;
	char buff[6000];	// 0xffff=65535

	buff[0] = 56;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = len;
	//EyelockLog(logger, DEBUG, "BoB => @@@ read data array fd=%d reg=%d, len=%d @@@", fd, reg, len);
	//flush(fd);
	result = write(fd, buff, 4);
	if (result != 4) {
		//EyelockLog(logger, ERROR, "BoB => write error in internal_read_array() - write %s, result %d", strerror(errno), result);
		//perror("write");
		return -1;
	}

	usleep(100);
	//usleep(50);
	len=len+4;
	result = read(fd, buff, len);
	if (result != len)
	{
		//EyelockLog(logger, ERROR, "BoB => read error in internal_read_array() - read %s, result %d", strerror(errno), result);
        //perror("read");
        return -1;
	}

	memcpy(buf, buff+4, len-4);

	return result;
}
#else
int internal_read_array(int fd, unsigned char reg, char *ptr, int len)
{
	int result;
	char buff[2];

	buff[0] = 0;
	buff[1] = reg;
	//EyelockLog(logger, DEBUG, "BoB => @@@ read data array fd=%d reg=%d, len=%d @@@", fd, reg, len);
	result = write(fd, buff, 2);
	if (result != 2) {
		EyelockLog(logger, ERROR, "BoB => write error in internal_read_array() - write %s, result %d", strerror(errno), result);
		perror("write");
		return -1;
	}

	usleep(100);
	//usleep(50);
	result = read(fd, ptr, len);
	if (result != len)
	{
		EyelockLog(logger, ERROR, "BoB => read error in internal_read_array() - read %s, result %d", strerror(errno), result);
        perror("read");
        return -1;
	}

	return result;
}
#endif

int BobWriteReg(unsigned char reg, unsigned int val)
{
	int fd = i2c_start_transaction();
	if(fd == 0)
    {
		EyelockLog(logger, ERROR, "BobWriteReg:BoB => Error starting interface");
        return -1;
    }
	//EyelockLog(logger, DEBUG, "BoB => @@@ write data reg=%d value=%d @@@", reg, val);
	int result = internal_write_reg(fd, reg, val);
	//close(fd);
	return result;

}
#if defined(HBOX_PG) || defined(CMX_C1)
int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result = -1;
	unsigned char buff[5];

	if(fd == 0)
    {
		//EyelockLog(logger, ERROR, "BoB => Error starting interface");
        return -1;
    }

	buff[0] = 57;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = 0x01;
	buff[4] = val;

	result = write(fd, buff, 5);

	if (result != 5) {
		EyelockLog(logger, ERROR, "BoB => internal_write_reg - write %s", strerror(errno));
		perror("write");
	}
	else
		result = 0;

	usleep(50);
	read(fd, buff,4);
	return result;
}
#else
int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result = -1;
	unsigned char buff[3];

	if(fd == 0)
    {
		EyelockLog(logger, ERROR, "BoB => Error starting interface");
        return -1;
    }

	buff[0] = 0;
	buff[1] = reg;
	buff[2] = val;

	result = write(fd, buff, 3);

	if (result != 3) {
		EyelockLog(logger, ERROR, "BoB => internal_write_reg - write %s", strerror(errno));
		perror("write");
	}
	else
		result = 0;
	usleep(50);
	return result;
}
#endif
int BobWriteArray(unsigned char reg, void *ptr, int len)
{
	int fd = i2c_start_transaction();
	if(fd == 0)
    {
	 EyelockLog(logger, ERROR, "BobWriteArray : BoB => Error starting interface");
        return -1;
    }
	int result = internal_write_array(fd, reg, ptr, len);
	//close(fd);
	return result;

}

#if defined(HBOX_PG) || defined(CMX_C1)
int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result;
	unsigned char buff[MAX_WRITE_LEN];
	//EyelockLog(logger, DEBUG, "BoB => @@@ write data array reg=%d, len=%d @@@", reg, len);
	if(fd == 0)
    {
		// EyelockLog(logger, ERROR, "BoB => Error starting interface");
        return -1;
    }
	buff[0] = 57;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = len;
	// buff[5] = val;
	if (ptr)
		memcpy(&buff[4],ptr,len);
	else
		memset(&buff[4],0,len);

	result = write(fd, buff, len+4);

	if (result != len+4) {
		//EyelockLog(logger, ERROR, "BoB => internal_write_array - write %s", strerror(errno));
		//perror("write");
	}
	else
		result = 0;

	usleep(50);
	return result;
}
#else
int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result;
	unsigned char buff[MAX_WRITE_LEN];
	//EyelockLog(logger, DEBUG, "BoB => @@@ write data array reg=%d, len=%d @@@", reg, len);
	if(fd == 0)
    {
		EyelockLog(logger, ERROR, "BoB => Error starting interface");
        return -1;
    }
	buff[0] = 0;
	buff[1] = reg;
	if (ptr)
		memcpy(&buff[2],ptr,len);
	else
		memset(&buff[2],0,len);

	result = write(fd, buff, len+2);

	if (result != len+2) {
		EyelockLog(logger, ERROR, "BoB => internal_write_array - write %s", strerror(errno));
		perror("write");
	}
	else
		result = 0;

	usleep(50);
	return result;
}
#endif

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

#define MAX_LINE_LEN 256

char *LoadEyelockINIFile(){
	static FILE *fp;
	char line[MAX_LINE_LEN + 1] ;
	char *token; char *Value;
	char *i2cbus = (char *)calloc(50,sizeof(char));
	fp = fopen("Eyelock.ini", "rb");
	if(fp == NULL) {
		EyelockLog(logger, ERROR, "Can't open Eyelock.ini %s", strerror(errno));
		perror("open");
		//return -1;
	}else{
		while( fgets( line, MAX_LINE_LEN, fp ) != NULL )
		{
			token = strtok( line, "\t =\n\r" ) ;
			if( token != NULL && token[0] != '#' )
			{
				Value = strtok( NULL, "\t =\n\r" ) ;
				if(strcmp(token,"Eyelock.ICMI2CUSBPort") == 0){
					strcpy(i2cbus, Value);
				}
			}
		}
		fclose(fp);
	}
	return i2cbus;
}
int i2c_start_transaction()
{
	int fd, result;
#if defined(HBOX_PG) || defined(CMX_C1)
	if(m_i2cdev == 0)
	{
		m_i2cdev = open(_i2c_bus, O_RDWR | O_NOCTTY | O_SYNC);
		if (m_i2cdev < 0) {
			//EyelockLog(logger, ERROR, "BoB => i2c_start_transaction - open %s", strerror(errno));
			//perror("**************open*************************************");
			return 0;
		}

		// Baudrate 19200, 8 bits, no parity, 1 stop bit
		set_interface_attribs(m_i2cdev, B19200);
	}
//#endif
#else
	fd = open(_i2c_bus, O_RDWR);
	if (fd < 0) {
		EyelockLog(logger, ERROR, "BoB => i2c_start_transaction - open %s", strerror(errno));
		perror("open");
		return 0;
	}

	result = ioctl(fd, I2C_SLAVE, NANO_BOB_I2C_ADDR);
	if (result < 0) {
		EyelockLog(logger, ERROR, "BoB => i2c_start_transaction - ioctl %s", strerror(errno));
		perror("ioctl");
		close(fd);
		return 0;
	}
#endif

	usleep(100);

	return m_i2cdev;
}



void   BobMutexStart(void)
{
	pthread_mutex_lock(&lock);
}
void   BobMutexEnd(void)
{
	pthread_mutex_unlock(&lock);
}

int BobSetCardReadAck()
{
	printf("inside BobSetCardReadAck\n ");
	int result = 1;
	usleep(100);
	BobMutexStart();
	unsigned int status = 0;
	BobReadReg(BOB_STATUS_OUT_OFFSET, &status);
	printf("inside BobSetCardReadAck reading status reg 0x%0x\n",status);
	usleep(100);
	result = BobWriteReg(BOB_STATUS_OUT_OFFSET, status|BOB_STATUS_OUT_CHANGE|BOB_STATUS_OUT_CARD_ACK);
	printf("inside BobSetCardReadAck -- writing ack retrun %d\n",result);

	BobMutexEnd();
	return result;
}

int BobSetACSOutput(unsigned int value)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	outputAcs = value | outputAcs;
	result = BobWriteReg(BOB_ACS_OUTPUT_OFFSET, outputAcs);
	if (result == 0)
	{
		usleep(100);
		unsigned int status;
		BobReadReg(BOB_STATUS_OUT_OFFSET, &status);
		usleep(100);
		result = 1;
		result = BobWriteReg(BOB_STATUS_OUT_OFFSET, status|BOB_STATUS_OUT_CHANGE|BOB_STATUS_OUT_ACS);
	}

	BobMutexEnd();
	return result;
}

int BobClearACSOutput(unsigned int value)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	outputAcs = ~value & outputAcs;
	result = BobWriteReg(BOB_ACS_OUTPUT_OFFSET, outputAcs);
	if (result == 0)
	{
		usleep(100);
		int value;
		BobReadReg(BOB_STATUS_OUT_OFFSET, &value);
		result = 1;
		usleep(100);
		result = BobWriteReg(BOB_STATUS_OUT_OFFSET, value|BOB_STATUS_OUT_CHANGE|BOB_STATUS_OUT_ACS);
		if (value)
			acsResetCount = 0;
	}

	BobMutexEnd();
	return result;
}
/*
void BobSetBitVal(unsigned char bit, int val)
{
	BobMutexStart();

	int val = BobWriteReg(CMD_OFFSET+1, bit);
	if (val)
		BobWriteReg(CMD_OFFSET, CMD_SET_OUTPUT_BITS);
	else
		BobWriteReg(CMD_OFFSET, CMD_CLEAR_OUTPUT_BITS);

	BobMutexEnd();
}
*/
int BobSetData(void *ptr, int len)
{
	int result = 1;

	usleep(100);
	BobMutexStart();
	result = BobWriteArray(BOB_ACCESS_DATA_OFFSET, ptr, len);
	BobMutexEnd();
	usleep(100);
	return result;
}

int BobGetData(void *ptr, int len)
{
	int result = 0;
	char tmp[2];
	usleep(100);
	BobMutexStart();
	if (!len) {
		BobReadArray(BOB_DATA_LENGTH_OFFSET, tmp, 2);
		len = tmp[0];
		len = (len << 8) + tmp[1];
		usleep(100);
	}
	//printf("data length in buffer %d\n", len);
	if (len > 0 && len <= 6000)
		result = BobReadArray(BOB_ACCESS_DATA_OFFSET, ptr, len);
	//usleep(100);
	//memset(tmp, 0, 2);
	//BobWriteArray(BOB_DATA_LENGTH_OFFSET, tmp, 2);
	usleep(100);
	BobMutexEnd();
	usleep(100);
	return result;
}

int BobSetCommand(int val)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	result = BobWriteReg(BOB_COMMAND_OFFSET, val);
	BobMutexEnd();
	return result;
}

int BobSetDataAndRunCommand(void *ptr, int len, int cmd)
{
	int result = 1;

	usleep(100);
	BobMutexStart();
	result = BobWriteArray(BOB_ACCESS_DATA_OFFSET, ptr, len);
	usleep(5000);
	result = BobWriteReg(BOB_COMMAND_OFFSET, cmd);
	BobMutexEnd();
	usleep(100);
	return result;
}


int BoBGetCommand()
{
	int value = 0xFF;
	BobMutexStart();
	BobReadReg(BOB_COMMAND_OFFSET, &value);
	BobMutexEnd();
	return value;
}

int BobSetDataLength(int val)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	result = BobWriteReg(BOB_DATA_LENGTH_OFFSET, 0);
	result = BobWriteReg(BOB_DATA_LENGTH_OFFSET+1, val);
	BobMutexEnd();
	return result;
}

int BobSetACSType(int val)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	result = BobWriteReg(BOB_ACS_TYPE_OFFSET, val);
	BobMutexEnd();
//	printf("BobSetACSType %d\n", val);
	return result;
}
int BobSetACSTypeBits(int val)
{
	int result = 1;
	usleep(100);
	BobMutexStart();
	result = BobWriteReg(BOB_ACS_TYPE_BITS_OFFSET, val);
	BobMutexEnd();
	return result;
}
int BobSetOSDPBaudRate(int rate, int reader)
{
	int result = 1;
	int baudValue = 0;
	int command = 0;

	switch (rate) {
	case 9600:
		baudValue = BOB_OSDP_BAUDRATE_9600;
		break;
	case 19200:
		baudValue = BOB_OSDP_BAUDRATE_19200;
		break;
	case 38400:
		baudValue = BOB_OSDP_BAUDRATE_38400;
		break;
	case 115200:
		baudValue = BOB_OSDP_BAUDRATE_115200;
		break;
	default:
		baudValue = BOB_OSDP_BAUDRATE_DISABLE;
		break;
	}

	command = baudValue ? BOB_OSDP_CMD_SET_BAUDRATE : BOB_OSDP_CMD_DISABLE;

	usleep(100);
	BobMutexStart();
	if (reader)
		result = BobWriteReg(BOB_OSDP_READER_CMD_OFFSET, command);
	else
		result = BobWriteReg(BOB_OSDP_CMD_OFFSET, command);
	if (result == 0) {
		usleep(100);
		if (reader)
			result = BobWriteReg(BOB_OSDP_READER_BAUDRATE_OFFSET, baudValue);
		else
			result = BobWriteReg(BOB_OSDP_BAUDRATE_OFFSET, baudValue);
	}
	BobMutexEnd();
	return result;
}

void BoBSendOSDPResponse(void * data, int length)
{
	int result = 1;

	if (BobWriteArray(BOB_OSDP_DATA_OFFSET, data, length) || BobWriteReg(BOB_OSDP_DATA_LENGTH_OFFSET, length))
		;//EyelockLog(logger, ERROR, "BoB => Error to write OSDP data");
	else {
		usleep(10); //usleep(100);
		result = BobWriteReg(BOB_OSDP_CMD_OFFSET, BOB_OSDP_CMD_SEND_DATA);
	}
}

void BoBSendReaderOSDPCommand(void * data, int length)
{
	int result = 1;
	unsigned int stat = 0;

	if (length > 0) { // reset READER_CMD
#if 0
		int count = 1000;

		while (count--) {	// 1000*50 = 50ms
			if (BobReadReg(BOB_OSDP_READER_CMD_OFFSET, &stat)) {
				EyelockLog(logger, ERROR, "BoB => Error to read reader OSDP command");
				return;
			}
			if (stat != BOB_OSDP_CMD_BUSY){
				break;
			}
			else
				printf("$$$$$$$$$$ buffer busy $$$$$$$$$$\n");
			usleep(50);
		}

		if (!count)
			EyelockLog(logger, ERROR, "BoB => Error to write OSDP data - buffer is busy");
		usleep(50);

		BobWriteReg(BOB_OSDP_READER_CMD_OFFSET, BOB_OSDP_CMD_BUSY);
		usleep(50);
#endif
		// store the command for resend
		memcpy(osdpCommand, data, length);
		osdpCommandLen = length;
		EyelockLog(logger, DEBUG, "BoB => write OSDP data for card reader");
		DumpBuffer(data, length);
		if (BobWriteArray(BOB_OSDP_READER_DATA_OFFSET, data, length) || BobWriteReg(BOB_OSDP_READER_LENGTH_OFFSET, length))
			; //EyelockLog(logger, ERROR, "BoB => Error to write OSDP data for card reader");
		else {
			usleep(10);
			if (BobReadReg(BOB_OSDP_STATUS_OFFSET, &stat)) {
				EyelockLog(logger, ERROR, "BoB => Error to read reader OSDP command");
				return;
			}
			if (stat & BOB_READER_OSDP_STAT_DATA_TO_READ){
				BobWriteReg(BOB_OSDP_STATUS_OFFSET, stat&~BOB_READER_OSDP_STAT_DATA_TO_READ);
			}
			result = BobWriteReg(BOB_OSDP_READER_CMD_OFFSET, BOB_OSDP_CMD_SEND_DATA);

			osdpResendCount = 0;
			EyelockLog(logger, TRACE, "BoB => Send OSDP data to card reader");
		}
	}

	// clear osdp status bit
	//BobClearOSDPStatus(BOB_READER_OSDP_STAT_DATA_TO_SEND | BOB_READER_OSDP_STAT_DATA_TO_READ);

}

void BobClearOSDPStatus(unsigned int value)
{
	unsigned int stat = 0;
	usleep(100);
	BobMutexStart();
	BobReadReg(BOB_OSDP_STATUS_OFFSET, &stat);
	stat = ~value & stat;
	usleep(100);
	BobWriteReg(BOB_OSDP_STATUS_OFFSET, &stat);
	BobMutexEnd();
}


int testElapsedTime()
{
    struct timeval start, end;

    long mtime, seconds, useconds;

    gettimeofday(&start, NULL);
    usleep(POLL_TIME);
    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    //if (mtime > 1)
    	//printf("Elapsed time: %ld milliseconds\n", mtime);

    return 0;
}


unsigned long BobCurrentTimeInMS() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long milliseconds = te.tv_sec*1000 + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

void BoBSetTimer(void (*cb)(void *), int ms, int repeat, void *thread)
{
	int i;
	//printf("BoBSetTimer %d\n", ms);
	if (bobTimerNum >= MAX_TIMER)
		;//EyelockLog(logger, ERROR, "Timer => All timers are used!");

	for (i = 0; i < MAX_TIMER; i++) {
		if (!bobTimer[i].used)
			break;
	}
	m_currentTimeInMS = BobCurrentTimeInMS();
	bobTimer[i].BobTimerCB = cb;
	bobTimer[i].startTime = m_currentTimeInMS;
	bobTimer[i].endTime = bobTimer[i].startTime + ms;
	bobTimer[i].timeout = ms;
	bobTimer[i].repeat = repeat;
	bobTimer[i].used = 1;
	bobTimer[i].thread = thread;
	bobTimerNum++;
}

void BoBUpdateTimer()
{
	int i;

	for (i = 0; i < MAX_TIMER; i++) {
		if (bobTimer[i].used) {
			if (bobTimer[i].BobTimerCB && m_currentTimeInMS >= bobTimer[i].endTime){
				bobTimer[i].BobTimerCB(bobTimer[i].thread);
				if (!bobTimer[i].repeat){
					bobTimer[i].BobTimerCB = NULL;
					bobTimer[i].used = 0;
					bobTimerNum--;
				}
				else {
					bobTimer[i].repeat--;
					m_currentTimeInMS = BobCurrentTimeInMS();
					bobTimer[i].endTime = m_currentTimeInMS + bobTimer[i].timeout;
				}
			}
		}
	}

}

void BoBCancelTimer(void (*cb)(void *))
{
	int i;
	// printf("BoBCancelTimer\n");

	for (i = 0; i < MAX_TIMER; i++) {
		if (bobTimer[i].used)
			if (bobTimer[i].BobTimerCB == cb){
				bobTimer[i].BobTimerCB = NULL;
				bobTimer[i].used = 0;
				bobTimerNum--;
			}
	}
}
int myi2cdump(int len)
{
	int val;
	if (access("i2cdump", F_OK ) != -1) {
		remove( "i2cdump" );
		BobMutexStart();
		BobReadArray(0, readBuffer, len);
		BobMutexEnd();
		DumpBuffer(readBuffer, len);
	}
}

void DumpBuffer(char *buf, int bytes)
{
	int i;
    for (i=0; i < bytes; i++) {
        if (i%16)
            printf(" %2X", buf[i]);
        else {
        	printf("\n");
        	printf("0x%2X", buf[i]);
        }
    }
    printf("\n");
}

int BoBOSDPPanelMsg() {
	return osdpPanelMsg;
}

int BoBOSDPReaderMsg() {
	return osdpReaderMsg;
}

int BobGetOSDPPanelData(unsigned char *ptr)
{
	int result = -1;
	int len = 0;
	usleep(100);
	BobMutexStart();
	BobReadReg(BOB_OSDP_DATA_LENGTH_OFFSET, &len);
	usleep(100);

	//printf("data length in buffer %d\n", len);
	if (len > 0 && len <= MAX_MSG_SIZE)
		result = BobReadArray(BOB_OSDP_DATA_OFFSET, ptr, len);

	usleep(100);
	BobMutexEnd();
	osdpPanelMsg = 0;
	if (result == -1)
		return 0;
	return len;
}

int BobGetOSDPReaderData(unsigned char *ptr)
{
	int result = -1;
	int len = 0;
	usleep(100);
	BobMutexStart();
	BobReadReg(BOB_OSDP_READER_LENGTH_OFFSET, &len);
	usleep(100);

	//printf("data length in buffer %d\n", len);
	if (len > 0 && len <= MAX_MSG_SIZE)
		result = BobReadArray(BOB_OSDP_READER_DATA_OFFSET, ptr, len);

	usleep(100);
	BobMutexEnd();
	osdpReaderMsg = 0;
	if (result == -1)
		return 0;
	return len;
}
