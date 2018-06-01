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
#define PORT 8194

  void diep(char *s)
  {
    perror(s);
    exit(1);
  }


#define min(a,b)((a<b)?(a):(b))
#define WIDTH 1200
#define HEIGHT 960

#define PORT_TIME       13              /* "time" (not available on RedHat) */
#define PORT_FTP        21              /* FTP connection port */
#define SERVER_ADDR     "192.168.4.172"     /* localhost */
#define MAXBUF          2048

int simple()
{   int sockfd;
    struct sockaddr_in dest;
    char buffer[MAXBUF];
    int rv;
    int x;
    int ret;
    short *sb=buffer;
    short vv=0;
    FILE *f;

    f=fopen("pi.raw","rb");
    if (f==NULL)
    {
    	printf("File not found\n");
    	return 0;
    }

    /*---Open socket for streaming---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
        exit(0);
    }

    /*---Initialize server address/port struct---*/
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(35);
    if ( inet_aton(SERVER_ADDR, &dest.sin_addr.s_addr) == 0 )
    {
        perror(SERVER_ADDR);
        exit(0);
    }

    /*---Connect to server---*/
    if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
    {
        perror("Connect ");
        exit(0);
    }
    usleep(10000);
    /*---Get "Hello?"---*/
    bzero(buffer, MAXBUF);
    //.recv(sockfd, buffer, sizeof(buffer), 0);
    //printf("%s", buffer);

#define SZ 512
    //fread(buffer,1,1,f);
    for(x=0;x<100;x++)
    	while(1)
    	{
    	int z;


    	if (vv>4095)
    		vv=0;
    	if (fread(buffer,1,SZ,f)!=SZ)
    		break;
    	rv= send(sockfd,buffer,SZ, 0);
    	//printf("%d rv send %d\n",x,rv);
    	rv=recv(sockfd,&ret,1,0);
    //	printf("rv rx %d\n",rv);
    	}

    printf("rv = %d\n",rv);
    /*---Clean up---*/
    close(sockfd);
    return 0;
}

  int main( int argc, char **argv )
  {
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    int ret;
    int bytes_to_get = WIDTH*HEIGHT;
    short *s_ptr = buf;
    int x;
    int frames;
    int num_frames;
    int wr;
    int bytes_to_write;
    int wait_for_sync;

    simple();
    return 0;
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, &si_me, sizeof(si_me))==-1)
    {
    	printf("error\n");
    	return 1;
    }
    if (argc>1)
    	num_frames = atoi(argv[1]);
    else
    	num_frames = 1;

    for (frames =0; frames< num_frames;frames++)
    {
    	bytes_to_get = WIDTH*HEIGHT;
    	wait_for_sync=1;
		while(bytes_to_get>0)
		{
		  ret = recvfrom(s, buf, BUFLEN, 0, &si_other, &slen);
		  if(ret<0)
			   exit(0);
		  if (wait_for_sync && (s_ptr[0]!=0x5555))
			  {
			//  printf ("No sync\n");
			  continue;
			  }
		  wait_for_sync=0;

		  for (x=0;x<ret/2;x++)
			  s_ptr[x]=s_ptr[x];
		  bytes_to_write = min (bytes_to_get,ret);

		 while(bytes_to_write>0)
		  {
		  wr = fwrite(buf,1,bytes_to_write,stdout);
		  bytes_to_write-=wr;
		  }
		//  printf("slen %d %d\n",bytes_to_get,ret);
		  bytes_to_get-=ret;
		}
		//printf("done\n");
    }


//      printf("Received packet from %s:%d\nData: %s\n\n",
 // inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);

    close(s);

}
