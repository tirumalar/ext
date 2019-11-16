/*
 * portcom.cpp
 *
 *  Created on: May 15, 2017
 *      Author: jerryi_vaio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>	/* needed for sockaddr_in */
#include <time.h>
#include "logging.h"
#include "FileConfiguration.h"
#include <errno.h>


static int sockfd;
#define SERVER_ADDR     "192.168.4.172"     /* localhost */

const char logger[50] = "PortCom";

#define ENCRYPT 1
#define MAX_TRYS 4
#define MAX_RECONNECT_TRIES 3
#define ERROR_TIMEOUT_SECONDS	5

#define ERROR_TIMEOUT_SECONDS_SND 10

pthread_mutex_t lock1;
#if ENCRYPT
#include "aes.h"
#define AES128

/*
 uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
 */
uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
struct AES_ctx ctx;
#endif

int portcom_start(bool bEncrpytFlag) {
	EyelockLog(logger, TRACE, "portcom_start");

	pthread_mutex_init(&lock1,NULL);

	struct sockaddr_in dest;
	if(bEncrpytFlag){
		AES_init_ctx_iv(&ctx, key,iv);
	}
	// Open socket for streaming
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		EyelockLog(logger, ERROR, "portcom_start socket error");
		perror("Socket");
		exit(0);
	}

	// Initialize server address/port struct
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(30);
	if (inet_aton(SERVER_ADDR, (in_addr*) &dest.sin_addr.s_addr) == 0) {
		EyelockLog(logger, ERROR, "portcom_start can't Find server");
		// printf("Cant Find server\n");
		return (0);
	}

	// Connect to server
	if (connect(sockfd, (struct sockaddr*) &dest, sizeof(dest)) != 0) {
		EyelockLog(logger, ERROR, "portcom_start can't Connect to server");
		// printf("Cant Connect to server\n");
		return (0);
	}

	EyelockLog(logger, DEBUG, "portcom_start: sockfd=%d", sockfd);

	return (0);
}

int portcom_connect(bool bEncrpytFlag)
{
	struct sockaddr_in dest;
	if(bEncrpytFlag){
		AES_init_ctx_iv(&ctx, key,iv);
	}
	// Open socket for streaming
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		EyelockLog(logger, ERROR, "portcom_start socket error");
		perror("Socket");
		return -1;
	}

	// Initialize server address/port struct
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(30);
	if (inet_aton(SERVER_ADDR, (in_addr*) &dest.sin_addr.s_addr) == 0) {
		EyelockLog(logger, ERROR, "portcom_start can't Find server");
		// printf("Cant Find server\n");
		return -2;
	}

	// Connect to server
	if (connect(sockfd, (struct sockaddr*) &dest, sizeof(dest)) != 0) {
		EyelockLog(logger, ERROR, "portcom_start can't Connect to server");
		// printf("Cant Connect to server\n");
		return -3;
	}

	EyelockLog(logger, DEBUG, "portcom_connect: sockfd=%d", sockfd);

	return 0;
}

int in_send = 0;

void port_com_send(char *cmd_in, float *pr_time)
{
	static bool bEncrpytFlag;
	static int firstEntry = 1;
	static int nFailCount = 0;

	pthread_mutex_lock(&lock1);

	if(firstEntry){		
		FileConfiguration FaceConfig("/home/root/data/calibration/Face.ini");
		bEncrpytFlag = FaceConfig.getValue("FTracker.AESEncrypt", false);
		firstEntry = 0;
	}

	char encr_sig[0] = "@";
	char buffer[512];
	char cmd[512];
	char cmd_encr[512];
	int rv;
	FILE *file;

	strcpy(cmd,cmd_in);
	// REMOVE TRAILING CRS
	while( cmd[strlen(cmd)-1]=='\n')
		cmd[strlen(cmd)-1]=0;

	PortComLog(logger, TRACE, "Entering > port_com_send()%s ", cmd);

	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0); // clearing input buffer (flush)

	int t = clock();
	sprintf(buffer, "%s\n", cmd);

	int nReceiveTimeOut = 0;

	if (strstr(cmd, "play_snd"))
		nReceiveTimeOut = ERROR_TIMEOUT_SECONDS_SND;
	else
		nReceiveTimeOut = ERROR_TIMEOUT_SECONDS;

	int nSendTimeOut = ERROR_TIMEOUT_SECONDS;

	/**************Encryption********************************************/
	int size = 0;
	if(bEncrpytFlag){
		sprintf(cmd_encr, "%s\n", cmd);
		// printf("\n before enc: %s ",cmd_encr);
		int enc_size = strlen(cmd_encr);
		enc_size = (enc_size/16+1)*16;
		AES_init_ctx_iv(&ctx, key,iv);
		AES_CBC_encrypt_buffer(&ctx, cmd_encr,enc_size);
		memset(buffer,0x00,512);
		buffer[0]='@';
		memcpy(buffer+1,cmd_encr,enc_size);
	//	printf("sending enc: %s \n",buffer);
		size = enc_size+1;
	}else{
		size = strlen(buffer);
	}

	int trys = 0;
	struct timespec ts;
	struct timespec tsTimeout;
	// snapshot of time for timeout on send
	clock_gettime(CLOCK_REALTIME, &tsTimeout);
	tsTimeout.tv_sec += nSendTimeOut;

	bool send_succeeded = false;

	while (true)
	{
		clock_gettime(CLOCK_REALTIME, &ts);

		if (ts.tv_sec >= tsTimeout.tv_sec)
		{
			PortComLog(logger, ERROR, "port_com_send(%s): send: trys = %d, nFailCount = %d, Timed out for send...", cmd, trys, nFailCount);
			break;
		}
		rv = send(sockfd, buffer, size, 0);
		if (rv != size)
		{
			PortComLog(logger, ERROR, "rv & command length don't match %d %d (errno=%d (%s)", rv, size, errno, strerror(errno));
			if (trys > MAX_TRYS)
			{
				break;
			}

			usleep(50);
			trys++;

			if (rv < 0)
			{
				PortComLog(logger, ERROR, "port_com_send(%s): send: rv = %d, trys = %d, Failcount = %d, ErroNo = %d, cannot receive data (errno=(%s)", cmd, rv, trys, nFailCount, errno, strerror(errno));
				if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN || errno == EBADF || errno == ENOTSOCK)
				{
					// reconnect
					PortComLog(logger, DEBUG, "port_com_send(%s): send: reconnecting");
					close(sockfd);

					int reconnect_tries = MAX_RECONNECT_TRIES;
					while (reconnect_tries > 0)
					{
						int reconnect_status = portcom_connect(bEncrpytFlag);
						if (reconnect_status != 0)
						{
							PortComLog(logger, ERROR, "port_com_send(%s): send: reconnect status: %d", reconnect_status);
						}
						else
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			send_succeeded = true;
			break;
		}
		//printf("%d rv send %d\n",x,rv);
	}

	if (!send_succeeded)
	{
		PortComLog(logger, ERROR, "port_com_send(%s): send totally failed, exiting");
		pthread_mutex_unlock(&lock1);
		return;
	}

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void*) &tv, sizeof(tv));

	trys = 0;
	buffer[0] = 0x00;
	// snapshot of time for timeout on recv
	clock_gettime(CLOCK_REALTIME, &tsTimeout);


	tsTimeout.tv_sec += nReceiveTimeOut;


	while (true)
	{

		clock_gettime(CLOCK_REALTIME, &ts);


		if (ts.tv_sec >= tsTimeout.tv_sec)
		{

			PortComLog(logger, ERROR, "port_com_send(%s): trys = %d, nFailCount = %d, Timed out waiting for response...", cmd, trys, nFailCount);
			break;
		}

		rv = recv(sockfd, buffer, 512, 0);

		// If the response is EAGAIN, it's not a failure, we sleep for 1 second then try recv again
		if (rv == 0)
		{
			PortComLog(logger, DEBUG, "port_com_send(%s): trys = %d, Failcount = %d, 0 bytes received, waiting...", cmd, trys, nFailCount);
			sleep(1); //No bytes there? but no error?  Just sleep then try again
		}
		else if (rv < 0)
		{
			if (errno != EAGAIN)
			{
				PortComLog(logger, ERROR, "port_com_send(%s): rv = %d, trys = %d, Failcount = %d, ErroNo = %d, cannot receive data (errno=(%s)", cmd, rv, trys, nFailCount, errno, strerror(errno));

				// exit(0);
				if (trys > MAX_TRYS)
				{

					nFailCount++;
					break; // Break out even if no OK is received
				}

				usleep(50);
				trys++;
			}
			else
				sleep(1);
		}
		else
		{
			buffer[rv] = 0;
			if (strstr(buffer, "OK"))
			{
				nFailCount = 0; //reset
				break;
			}

			if (trys > MAX_TRYS)
			{
				PortComLog(logger, ERROR, "port_com_send(%s): trys = %d, FailCount = %d, timed out receiving response (errno=(%s)", cmd, trys, nFailCount, strerror(errno));
				nFailCount++;

				break; // Break out even if no OK is received
			}
		}
	}

	if (nFailCount >= 5)
	{

		PortComLog(logger, ERROR, "port_com_send(%s): 5 failures, rebooting! (errno=(%s)", cmd, strerror(errno));
		system("/home/root/forcereboot.sh &");
	}


	PortComLog(logger, TRACE, "Leaving port_com_send()-- recv Trys = %d, Current time = %2.4f, ProcessingTme = %2.4f %s %s", trys, (float) clock() / CLOCKS_PER_SEC,
				(float) (clock() - t) / CLOCKS_PER_SEC, cmd, buffer);


	if (pr_time)
		*pr_time=(float) (clock() - t) / CLOCKS_PER_SEC;

	pthread_mutex_unlock(&lock1);
}

#if 1
int port_com_send_return(char *cmd, char *buffer, int min_len) {
	pthread_mutex_lock(&lock1);

	static bool bEncrpytFlag;
	static int firstEntry = 1;

	char cmd_encr[512];
	if(firstEntry){
		FileConfiguration FaceConfig("/home/root/data/calibration/Face.ini");
		bEncrpytFlag = FaceConfig.getValue("FTracker.AESEncrypt", false);
		firstEntry = 0;
	}
	// REMOVE TRAILING CRS
	while( cmd[strlen(cmd)-1]=='\n')
		cmd[strlen(cmd)-1]=0;

	PortComLog(logger, TRACE, "Entering > port_com_send_return(%s)", cmd);

	int rv;
	FILE *file;
	static int nFailCount = 0;

	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0); // clearing input buffer (flush)
	int t = clock();

	int nSendTimeOut = ERROR_TIMEOUT_SECONDS;

	sprintf(buffer, "%s\n", cmd);
	int size = 0;
	if(bEncrpytFlag){
		sprintf(cmd_encr, "%s\n", cmd);
		// printf("\n before enc: %s ",cmd_encr);
		int enc_size = strlen(cmd_encr);
		enc_size = (enc_size/16+1)*16;
		AES_init_ctx_iv(&ctx, key,iv);
		AES_CBC_encrypt_buffer(&ctx, cmd_encr,enc_size);
		memset(buffer,0x00,512);
		buffer[0]='@';
		memcpy(buffer+1,cmd_encr,enc_size);
	//	printf("sending enc: %s \n",buffer);
		size = enc_size+1;
	}else{
		size = strlen(buffer);
	}

	int trys = 0;
	struct timespec ts;
	struct timespec tsTimeout;
	// snapshot of time for timeout on send
	clock_gettime(CLOCK_REALTIME, &tsTimeout);
	tsTimeout.tv_sec += nSendTimeOut;

	bool send_succeeded = false;

	while (true)
	{
		clock_gettime(CLOCK_REALTIME, &ts);

		if (ts.tv_sec >= tsTimeout.tv_sec)
		{
			PortComLog(logger, ERROR, "port_com_send_return(%s): send: trys = %d, nFailCount = %d, Timed out for send...", cmd, trys, nFailCount);
			break;
		}
		rv = send(sockfd, buffer, size, 0);
		if (rv != size)
		{
			PortComLog(logger, ERROR, "rv & command length don't match %d %d (errno=%d (%s)", rv, size, errno, strerror(errno));
			if (trys > MAX_TRYS)
			{
				break;
			}

			usleep(50);
			trys++;

			if (rv < 0)
			{
				PortComLog(logger, ERROR, "port_com_send_return(%s): send: rv = %d, trys = %d, Failcount = %d, ErroNo = %d, cannot receive data (errno=(%s)", cmd, rv, trys, nFailCount, errno, strerror(errno));
				if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN || errno == EBADF || errno == ENOTSOCK)
				{
					// reconnect
					PortComLog(logger, DEBUG, "port_com_send_return(%s): send: reconnecting");
					close(sockfd);

					int reconnect_tries = MAX_RECONNECT_TRIES;
					while (reconnect_tries > 0)
					{
						int reconnect_status = portcom_connect(bEncrpytFlag);
						if (reconnect_status != 0)
						{
							PortComLog(logger, ERROR, "port_com_send_return(%s): send: reconnect status: %d", reconnect_status);
						}
						else
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			send_succeeded = true;
			break;
		}
		//printf("%d rv send %d\n",x,rv);
	}

	if (!send_succeeded)
	{
		PortComLog(logger, ERROR, "port_com_send_return(%s): send totally failed, exiting");
		pthread_mutex_unlock(&lock1);
		return 0;
	}

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void*) &tv, sizeof(tv));


	char rx_buffer[512];

	// clear the return buffer
	buffer[0]=0;
	trys = 0;
	// snapshot of time for timeout on recv
	clock_gettime(CLOCK_REALTIME, &tsTimeout);

	tsTimeout.tv_sec += ERROR_TIMEOUT_SECONDS;

	while (1)
	{
		clock_gettime(CLOCK_REALTIME, &ts);

		if (ts.tv_sec >= tsTimeout.tv_sec)
		{
			PortComLog(logger, ERROR, "port_com_send_return(%s): trys = %d, nFailCount = %d, Timed out waiting for response...", cmd, trys, nFailCount);
			break;
		}

		rv = recv(sockfd, rx_buffer, 512, 0);

		// If the response is EAGAIN, it's not a failure, we sleep for 1 second then try recv again
		if (rv == 0)
		{
			PortComLog(logger, DEBUG, "port_com_send_return(%s): trys = %d, Failcount = %d, 0 bytes received, waiting...", cmd, trys, nFailCount);
			sleep(1); //No bytes there? but no error?  Just sleep then try again
		}
		else if (rv < 0)
		{
			if (errno != EAGAIN)
			{
				PortComLog(logger, ERROR, "port_com_send_return(%s): rv = %d, trys = %d, Failcount = %d, ErroNo = %d, cannot receive data (errno=(%s)", cmd, rv, trys, nFailCount, errno, strerror(errno));

				// exit(0);
				if (trys > MAX_TRYS)
				{

					nFailCount++;
					break; // Break out even if no OK is received
				}

				usleep(50);
				trys++;
			}
			else
				sleep(1);
		}
		else
		{
			rx_buffer[rv]=0;
			strcat(buffer,rx_buffer);
			if (strstr(buffer,"OK")==0)
			{

				if (trys >= MAX_TRYS)
				{
					nFailCount++;
					PortComLog(logger, ERROR, "port_com_send_return(%s): trys = %d, FailCount = %d, timed out receiving response (errno=(%s)", cmd, trys, nFailCount, strerror(errno));
					break;
				}
			}
			else
			{
				nFailCount = 0;
				break;
			}
		}
	}

	if (nFailCount >= 5)
	{
		PortComLog(logger, ERROR, "port_com_send_return(%s): 5 failures, rebooting!(errno=(%s)", cmd, strerror(errno));
		system("/home/root/forcereboot.sh &");
	}


	PortComLog(logger, TRACE, "Leaving port_com_send_return -- recv Trys = %d, Current time = %2.4f, ProcessingTme = %2.4f %s %s", trys, (float) clock() / CLOCKS_PER_SEC,
				(float) (clock() - t) / CLOCKS_PER_SEC, cmd, buffer);
	pthread_mutex_unlock(&lock1);

	return strlen(buffer);
}

#else
int port_com_send_return(char *cmd, char *buffer, int min_len) {
	EyelockLog(logger, TRACE, "port_com_send");
	pthread_mutex_lock(&lock1);


	// REMOVE TRAILING CRS
	while( cmd[strlen(cmd)-1]=='\n')
		cmd[strlen(cmd)-1]=0;

	int rv;
	FILE *file;

	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0); // clearing input buffer (flush)
	int t = clock();

	sprintf(buffer, "%s\n", cmd);
	rv = send(sockfd, buffer, strlen(buffer), 0);
	if (rv != (int) strlen(buffer))
		printf("rv & command length don't match %d %d\n", rv, strlen(buffer));
	//printf("%d rv send %d\n",x,rv);

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void*) &tv, sizeof(tv));


	char rx_buffer[512];

	// clear the return buffer
	buffer[0]=0;
	int trys = 0;
   #define MAX_TRYS 4

while (1)
	{
		rv = recv(sockfd, rx_buffer, 512, 0);

		if (rv <= 0)
		{
			PortComLog(logger, ERROR, "cannot receive data");
		//	break;
		}
		else
		{
			rx_buffer[rv]=0;
			strcat(buffer,rx_buffer);
			if (strstr(buffer,"OK")==0)
			{

			if (trys++==MAX_TRYS)
				{
				PortComLog(logger, ERROR, "cannot find token");
				
				break;
				}
			}
			else
				break;
		}
	}
	PortComLog(logger, DEBUG, "Current time = %2.4f, ProcessingTme = %2.4f, <%s>-->%s>",
					(float) clock() / CLOCKS_PER_SEC,
					(float) (clock() - t) / CLOCKS_PER_SEC, cmd,buffer);
	pthread_mutex_unlock(&lock1);

	return strlen(buffer);
}
#endif

float read_angle(void) {
	EyelockLog(logger, TRACE, "read_angle");
	char buffer[512];
	float x, y, z, a;
	int len;
	int t = clock();
	if ((len = port_com_send_return("accel", buffer, 20)) > 0) {
		EyelockLog(logger, TRACE, "got data %d", len);
		buffer[len] = 0;
		sscanf(buffer, "%f %f %f %f", &x, &y, &z, &a);
		EyelockLog(logger, TRACE, "Buffer =>%s", buffer);
		EyelockLog(logger, TRACE, "%3.3f %3.3f %3.3f %3.3f  readTime=%2.4f",
				x, y, z, a, (float) (clock() - t) / CLOCKS_PER_SEC );
		return a;
	} else
		EyelockLog(logger, TRACE, "error reading angle");
	return 0;
}

