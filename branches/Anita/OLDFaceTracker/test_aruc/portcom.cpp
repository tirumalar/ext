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

int portcom_start() {
	EyelockLog(logger, TRACE, "portcom_start");
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
}

int in_send = 0;

void port_com_send(char *cmd) {
	EyelockLog(logger, TRACE, "port_com_send");

	char buffer[512];
	int rv, ret;
	//printf("In port com");
	FILE *file;
	if (strncmp(cmd, "ixed", strlen("ixed")) == 0) {
		printf("I got ixed\n");
		//scanf("%d",&rv);
		return;
	}
	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0)
		;
	int t = clock();

	if (in_send) {

		// printf("--------------------------------------->  In send\n");
#ifdef NOOPTIMIZE
		while (in_send)
		usleep(1000);
#endif
	}
	in_send = 1;
	sprintf(buffer, "%s\n", cmd);
	rv = send(sockfd, buffer, strlen(buffer), 0);
	if (rv != (int) strlen(buffer))
		printf("rv & command length don't match %d %d\n", rv, strlen(buffer));
	//printf("%d rv send %d\n",x,rv);

//	printf("# ");
	int counter = 0;
	while (1) {
		rv = recv(sockfd, buffer, 512, MSG_DONTWAIT);
		counter++;
		// usleep(100);
		if (counter > 500000) {
			EyelockLog(logger, ERROR,
					"port_com_send didnot return from while loop");
			printf("----------------->error no return\n");
			//printf(">%s Got K %2.3f\n",cmd,(float)(clock()-t)/CLOCKS_PER_SEC);
			break;
		}
		if (rv <= 0)
			continue;
		else {
			//cmd = buffer;
			/*		for (int i=0; i < 30; i++)
			 printf("%c \n",buffer[i]);
			 */}
		buffer[rv] = 0;
		if (strchr(buffer, 'K')) {
			//printf(">%s Got K %2.3f\n",cmd,(float)(clock()-t)/CLOCKS_PER_SEC);
			break;
		}
	}

	file = fopen("port.log", "at");
	if (file) {
		fprintf(file, "Current time = %2.4f, ProcessingTme = %2.4f, <%s>\n",
				(float) clock() / CLOCKS_PER_SEC,
				(float) (clock() - t) / CLOCKS_PER_SEC, cmd);
		fclose(file);
	}

	//usleep(5000);
	in_send = 0;
	// printf("%u & \n",counter);
}

int port_com_send_return(char *cmd, char *buffer, int min_len) {
	EyelockLog(logger, TRACE, "port_com_send_return");
	int rv, ret;

	if (in_send) {
		EyelockLog(logger, DEBUG, "port_com_send_return inside in_send loop");
#ifdef NOOPTIMIZE
		while (in_send)
		usleep(1000);
#endif
	}
	in_send = 1;

#ifdef NOOPTIMIZE
	//usleep(10000);
#endif
	while (recv(sockfd, buffer, 512, MSG_DONTWAIT) > 0)
		;

	sprintf(buffer, "%s\n", cmd);
	rv = send(sockfd, buffer, strlen(buffer), 0);
	if (rv != (int) strlen(buffer))
		printf("rv & command length don't match\n");
	//printf("%d rv send %d\n",x,rv);

	int counter = 0;
	while (1) {
		rv = recv(sockfd, buffer, 512, MSG_DONTWAIT);
		counter++;
		// usleep(100);
		if (counter > 2000) {
			EyelockLog(logger, ERROR, "port_com_send_return timeout error");
			printf(" timeout error\n");
			break;
		}
		if (rv <= min_len)
			continue;
		buffer[rv] = 0;
		//if (strchr(buffer,'K'))
		{
			//printf("Got K\n");
			break;
		}

	}
	//usleep(5000);
	in_send = 0;
	// printf("%u & \n",counter);

	return rv;
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

