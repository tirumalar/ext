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


static int sockfd;
#define SERVER_ADDR     "192.168.4.172"     /* localhost */

int portcom_start()
{


    struct sockaddr_in dest;

    int ret;


    /*---Open socket for streaming---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
        exit(0);
    }

    /*---Initialize server address/port struct---*/
    memset(&dest,0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(30);
    if ( inet_aton(SERVER_ADDR, (in_addr*) &dest.sin_addr.s_addr) == 0 )
    {
        printf("Cant Find server\n");
        return (0);
    }

    /*---Connect to server---*/
    if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
    {
    	printf("Cant Connect to server\n");
        return (0);
    }
}


int in_send=0;
void port_com_send(char *cmd)
{
	char buffer[512];
	int rv,ret;

	if (in_send)
	{
		printf("--------------------------------------->  In send\n");
	}
	in_send=1;
	sprintf(buffer,"%s\n",cmd);
	rv= send(sockfd,buffer,strlen(buffer), 0);
	if(rv!=(int)strlen(buffer))
		printf("rv & command length don't match\n");
	//printf("%d rv send %d\n",x,rv);

//	printf("# ");
	int counter=0;
	while (1)
		{
		rv=recv(sockfd,buffer,512,MSG_DONTWAIT);
		if(rv<=0)
			continue;
		else{
			cmd = buffer;
	/*		for (int i=0; i < 30; i++)
				printf("%c \n",buffer[i]);
*/		}
		buffer[rv]=0;
		//if (strchr(buffer,'K'))
			{
			//printf("Got K\n");
			break;
			}
		counter++;
		}
	//usleep(5000);
   in_send =0;
  // printf("%u & \n",counter);
}
