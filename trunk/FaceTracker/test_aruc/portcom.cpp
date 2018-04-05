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
	//printf("In port com");
	FILE *file;
	file = fopen("port.log","at");
	if (file)
	{
		fprintf(file,"<%s>\n",cmd);
		fclose(file);
	}

	if (strncmp(cmd,"ixed",strlen("ixed"))==0)
			{
		printf("I got ixed\n");
		//scanf("%d",&rv);
			return;
			}
	while (recv(sockfd,buffer,512,MSG_DONTWAIT)>0);
	int t=clock();

	if (in_send)
	{
		printf("--------------------------------------->  In send\n");
		while (in_send)
			usleep(1000);
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
		counter++;
		usleep(100);
		if (counter>500000)
			{
			printf("----------------->error no return\n");
			//printf(">%s Got K %2.3f\n",cmd,(float)(clock()-t)/CLOCKS_PER_SEC);
			break;
			}
		if(rv<=0)
			continue;
		else{
			//cmd = buffer;
	/*		for (int i=0; i < 30; i++)
				printf("%c \n",buffer[i]);
*/		}
		buffer[rv]=0;
		if (strchr(buffer,'K'))
			{
			//printf(">%s Got K %2.3f\n",cmd,(float)(clock()-t)/CLOCKS_PER_SEC);
			break;
			}
		}
	//usleep(5000);
   in_send =0;
  // printf("%u & \n",counter);
}




int port_com_send_return(char *cmd, char *buffer, int min_len)
{
	//char buffer[512];
	int rv,ret;

	if (in_send)
	{
		printf("--------------------------------------->  In send\n");
		while (in_send)
				usleep(1000);
	}
	in_send=1;

	//printf("In port com");
	//usleep(10000);
	while (recv(sockfd,buffer,512,MSG_DONTWAIT)>0);



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
		counter++;
		usleep(100);
		if (counter>2000)
		   {
			printf(" timeout error\n");
			break;
		   }
		if(rv<=min_len)
			continue;
		buffer[rv]=0;
		//if (strchr(buffer,'K'))
			{
			//printf("Got K\n");
			break;
			}

		}
	//usleep(5000);
   in_send =0;
  // printf("%u & \n",counter);

   return rv;
}

float read_angle(void)
{
	char buffer[512];

	float x,y,z,a;
	int len;
	int t=clock();
	//printf("Reading angle \n");
	if ((len=port_com_send_return("accel",buffer,20))>0)
	{
	//printf("got data %d\n",len);
	buffer[len]=0;
	sscanf(buffer,"%f %f %f %f",&x,&y,&z,&a);
//	printf("Buffer =>%s\n",buffer);
	//printf ("%3.3f %3.3f %3.3f %3.3f  readTime=%2.4f\n",x,y,z,a,(float)(clock()-t)/CLOCKS_PER_SEC);
	return a;
	}
	else
		printf("error reading angle\n");
   return 0;
}


