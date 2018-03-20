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

#define BUFLEN 1500
#define NPACK 10
// 8192 left camer
// 8193 right
#define PORT 8193

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

int vid_stream_start(int port)
  {


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
  }

int vid_stream_get(int *win,int *hin, char *wbuffer)
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



