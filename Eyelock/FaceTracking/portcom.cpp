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
#include "log.h"

static int sockfd;
#define SERVER_ADDR     "192.168.4.172"     /* localhost */

const char logger[30] = "PortCom";

pthread_mutex_t lock;

int portcom_start() {
	EyelockLog(logger, TRACE, "portcom_start");

	pthread_mutex_init(&lock,NULL);

	struct sockaddr_in dest;

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

	return (0);
}

int in_send = 0;

void port_com_send(char *cmd_in)
{
	EyelockLog(logger, TRACE, "port_com_send");
	pthread_mutex_lock(&lock);

	char buffer[512];
	char cmd[512];
	int rv;
	FILE *file;

	strcpy(cmd,cmd_in);
	// REMOVE TRAILING CRS
	while( cmd[strlen(cmd)-1]=='\n')
		cmd[strlen(cmd)-1]=0;

	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0); // clearing input buffer (flush)
	int t = clock();

	sprintf(buffer, "%s\n", cmd);
	rv = send(sockfd, buffer, strlen(buffer), 0);
	if (rv != (int) strlen(buffer))
		printf("rv & command length don't match %d %d\n", rv, strlen(buffer));
	//printf("%d rv send %d\n",x,rv);

	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void*) &tv, sizeof(tv));

	rv = recv(sockfd, buffer, 512, 0);

	if (rv <= 0)
	{
		EyelockLog(logger, ERROR, "cannot receive data");
	}
	else
	{
		buffer[rv] = 0;
		if (!strchr(buffer, 'K'))
		{
		}
	}

	file = fopen("port.log", "at");
	if (file)
	{
		fprintf(file, "Current time = %2.4f, ProcessingTme = %2.4f, <%s>\n-->%s>\n",
				(float) clock() / CLOCKS_PER_SEC,
				(float) (clock() - t) / CLOCKS_PER_SEC, cmd,buffer);
		fclose(file);
	}

	pthread_mutex_unlock(&lock);
}

int port_com_send_return(char *cmd, char *buffer, int min_len) {
	EyelockLog(logger, TRACE, "port_com_send");
	pthread_mutex_lock(&lock);


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
			EyelockLog(logger, ERROR, "cannot receive data");
		}
		else
		{
			rx_buffer[rv]=0;
			strcat(buffer,rx_buffer);
			if (strstr(buffer,"OK")==0)
			{

			if (trys++==MAX_TRYS)
				{
				EyelockLog(logger, ERROR, "cannot find token");
				
				break;
				}
			}
			else
				break;
		}
	}

	file = fopen("port.log", "at");
	if (file)
	{
		fprintf(file, "Current time = %2.4f, ProcessingTme = %2.4f, <%s>\n-->%s>\n",
				(float) clock() / CLOCKS_PER_SEC,
				(float) (clock() - t) / CLOCKS_PER_SEC, cmd,buffer);
		fclose(file);
	}

	pthread_mutex_unlock(&lock);

	return strlen(buffer);
}

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
		EyelockLog(logger, TRACE, "Buffer =>%s\n", buffer);
		EyelockLog(logger, TRACE, "%3.3f %3.3f %3.3f %3.3f  readTime=%2.4f\n",
				x, y, z, a, (float) (clock() - t) / CLOCKS_PER_SEC );
		return a;
	} else
		EyelockLog(logger, TRACE, "\n error reading angle");
	return 0;
}

