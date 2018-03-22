/*
 ============================================================================
 Name        : pstream.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>	/* needed for sockaddr_in */
#include <pthread.h>
#include "Synchronization.h"

#define BUFLEN 1500
#define IMAGE_SIZE 1152000
#define NPACK 10
// 8192 left camer
// 8193 right
#define PORT 8193

typedef struct ImageQueue{
	unsigned char m_ptr[1152000];	// image size 1152000
	int m_ill0;
	int m_frameIndex;
	__int64_t m_startTime,m_endTime;
};


typedef RingBuffer<ImageQueue> RingBufferImageQueue;

void diep(char *s)
{
    perror(s);
    exit(1);
}
#define min(a,b)((a<b)?(a):(b))
#define WIDTH 1200
#define HEIGHT 960
struct sockaddr_in si_me, si_other;
int s, i;
unsigned int slen=sizeof(si_other);
char buf[BUFLEN];
int ret;
int bytes_to_get = WIDTH*(HEIGHT-1);
char *s_ptr = buf;
int x;
int frames;
int num_frames;
int wr;
int bytes_to_write;
int wait_for_sync;
pthread_t leftCThread;
void *leftCServer(void *arg);
RingBufferImageQueue *m_pRingBuffer;
int m_port;
int vid_stream_start(int port)
  {
#if 1
	m_port = port;
	if (pthread_create (&leftCThread, NULL, leftCServer, NULL))
	{
			printf("MainLoop(): Error creating thread FaceServer\n");
			return 0;
	}
	m_pRingBuffer = new RingBufferImageQueue(2);
#else
	printf("opening port %d\n",port);
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
    {
    	printf("error\n");
    	return 1;
    }
#endif
    return 1;
  }

int vid_stream_get(int *win,int *hin,char *m_pImageBuffer){

	int length = (HEIGHT * WIDTH);
//	if (m_numbits != 8)
//		length = length * 2;

	ImageQueue val;
#if 0
	bool status = m_pRingBuffer->TryPop(val);
	if (status == true) {
		printf("image poped from queue\n");
		unsigned char *ptr = val.m_ptr;
//		m_ill0 = val.m_ill0;
//		m_frameIndex = val.m_frameIndex;
//		m_ts = val.m_endTime;
		memcpy(m_pImageBuffer, ptr, length);
	}
	else {
		printf("no image the queue\n");
		memset(m_pImageBuffer, 0, length);
	}
#endif
	bool status;
	//printf("entering vid_stream_get\n");
	while((m_pRingBuffer->TryPop(val)) != true);

	{
		//printf("image poped from queue\n");
				unsigned char *ptr = val.m_ptr;
		//		m_ill0 = val.m_ill0;
		//		m_frameIndex = val.m_frameIndex;
		//		m_ts = val.m_endTime;
				memcpy(m_pImageBuffer, ptr, length);
	}

	return 1;
}
int vid_stream_get_old(int *win,int *hin, char *wbuffer)
{

	int p_count=0;
	*hin=HEIGHT;
	*win=WIDTH;
	bytes_to_get = WIDTH*HEIGHT-2;// should investigate the -2
    wait_for_sync=1;
	while(bytes_to_get>0)
	{
		ret = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen);
		if(ret<0)
		{
		    printf("error read image socket\n");
		    exit(0);
		}
		if (wait_for_sync && (buf[0]!=0x55 && buf[1]!=0x55))
		 // if (wait_for_sync && (!memcmp(s_ptr,"5555",2)))
		  //if (wait_for_sync && (s_ptr[0]!=0x5555))//s_ptr
		{
			  	  /*for(int j=0;j<10;j++)
			  	  {
				  	  printf("%x",buf[j]);
			  	  }
			  	  printf("\n");*/
			 	  //printf(".");
			  p_count++;
			  	  continue;
		 }
		 if(p_count > 0)
		 {
			//  printf("skipped %d packets\n",p_count);
		 }
		 p_count=0;
		 usleep(100);
		 wait_for_sync=0;
		  //printf("received sync\n");
		 bytes_to_write = min (bytes_to_get,ret);
		 memcpy(wbuffer,buf,bytes_to_write);
		 wbuffer+=bytes_to_write;
		 bytes_to_get-=ret;
	}
}

bool HandleReceiveImage(char *ptr, int length)
{
//	printf("in Handle Receive Image %d\n",length);
	if(ptr)
    {
	//	printf("ptr is valid...in Handle Receive Image %d\n",length);
#if 0
		printf("ptr is valid...in Handle Receive Image %d\n",length);

		char filename[100];
		static int i;
		sprintf(filename,"bbSF_%d.bin",i++);
		FILE *f = fopen(filename, "wb");
		fwrite(ptr, (WIDTH*HEIGHT), 1, f);
#endif
			ImageQueue val;
#if 0
			static int cntr=0;
			int length = (m_Width * m_Height);
			if (m_numbits != 8)
				length = length * 2;

			if(cntr & 0x1){
				val.m_ill0 = 1;
			}else{
				val.m_ill0 = 0;
			}
			val.m_frameIndex = cntr;
			cntr++;
#endif
			unsigned char *pdata = val.m_ptr;
			memcpy(pdata, ptr, length);
#if 0
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer,starttimestamp);
			val.m_startTime = starttimestamp;
			val.m_endTime = val.m_startTime;
#endif
			bool status;
			status=m_pRingBuffer->TryPush(val);
			//if(status != true)
			//	printf("Queue full\n");
    }
    return true;
}
int CreateUDPServer(int port) {

	int sock, length;
	struct sockaddr_in server;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return sock;
	}
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(port);
	if (bind(sock,(struct sockaddr *)&server,length)<0) {
		return -1;
	}
	return sock;
}

void *leftCServer(void *arg)
{
        printf("leftCServer() start\n");
        int length;
        int pckcnt=0;
        char buf[IMAGE_SIZE];
        char databuf[IMAGE_SIZE];
        int datalen = 0;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);

        int leftCSock = CreateUDPServer(m_port);
        if (leftCSock < 0)
        {
                printf("Failed to create leftC Server()\n");
                return NULL;
        }
        while (1)
        {
            length = recvfrom(leftCSock, buf, 1500, 0, (struct sockaddr *)&from, &fromlen);
            if (length < 0)
                printf("recvfrom error in leftCServer()");
            else
            {
                if(!b_syncReceived && pShort[0] == 0x5555)
                {
                        datalen = 0;
                        memcpy(databuf, buf+2, length-2);
                        datalen = length - 2;
                        b_syncReceived = true;
                        pckcnt=1;
                       // printf("Sync\n");
                }
                else if(b_syncReceived)
                {
                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
                        memcpy(databuf+datalen, buf, length);
                        datalen += length;
                        pckcnt++;
                }
                if(datalen >= IMAGE_SIZE-5)
                {
                    HandleReceiveImage(databuf, datalen);
                   	datalen = 0;
                   	b_syncReceived=false;
               }
          }
      }
      close(leftCSock);
      if (leftCThread)
      {
      		pthread_join(leftCThread, NULL);
      		leftCThread = 0;
      }
}

