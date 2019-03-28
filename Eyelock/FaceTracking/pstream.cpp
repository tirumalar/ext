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
#include <algorithm>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>	/* needed for sockaddr_in */
#include <pthread.h>
#include "Synchronization.h"
#include "pstream.h"
#include <signal.h>

const char loggerp[30] = "pstream";
void diep(char *s)
{
    perror(s);
    exit(1);
}
/*
struct sockaddr_in si_me, si_other;
int s, i;
unsigned int slen=sizeof(si_other);
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
*/
int m_port;

#define FREE_BUFF_SIZE 2

void VideoStream::flush() {
	ImageQueueItemF val;
	while ((m_pRingBuffer->TryPop(val)) != false)
		usleep(1000);
}
VideoStream::~VideoStream()
{
	running=0;
	while (running >= 0)
		usleep(100);

	// assuming all items from m_ProcessBuffer will be returned back to m_FreeBuffer
	ImageQueueItemF qi;
	while (!m_FreeBuffer->Empty())
	{
		if (m_FreeBuffer->TryPop(qi))
		{
			delete[] qi.m_ptr;
		}
	}

	//DMOLEAK
	delete m_pRingBuffer;
	delete m_FreeBuffer;
	delete m_ProcessBuffer;
}

int VideoStream::get(int *win,int *hin,char *m_pImageBuffer, char get_last)
{
	//printf("entering vid_stream_get\n");

	// release the previously processed buffer it it is real;

	// pop all but 1 image if get_last is enabled
	while (get_last && m_ProcessBuffer->Size()>1)
		{
		int stat;
    	ReleaseProcessBuffer(m_current_process_queue_item);
    	stat=m_ProcessBuffer->TryPop(m_current_process_queue_item);
    	if (stat == false)
    		printf("This should never never happen\n");
		}



    if (m_current_process_queue_item.m_ptr != NULL)
    {
    	ReleaseProcessBuffer(m_current_process_queue_item);
    }

	while((m_ProcessBuffer->TryPop(m_current_process_queue_item)) != true)
	{
		usleep(1000);
	}

	if (offset_sub_enable)
	{
		if (offset_image_loaded==0)
		{
			//load the offset image
			char temp[100];
			sprintf(temp,"cal%02x.bin",cam_id);
			printf("Loading %s offset file\n",temp);
			FILE *f = fopen(temp, "rb");
				if (f)
				{
				fread(offset_image ,length, 1, f);
				fclose(f);
				offset_image_loaded=1;
				printf("Loading success file\n");
				}
		}
		if (offset_image_loaded)
			for (int z = 0; z< length;z++) // add underflow logic
				m_pImageBuffer[z]= max((int)m_current_process_queue_item.m_ptr[z]-(int)offset_image[z],0);
	}
	else
		memcpy(m_pImageBuffer, m_current_process_queue_item.m_ptr, length);

	return 1;
}

//bool VideoStream::HandleReceiveImage(unsigned char *ptr, int length)
//{
//	ImageQueueItem val;
//
//	return m_pRingBuffer->TryPush(val);
//}

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

#include <stdio.h>
#include <netdb.h>
extern int h_errno;

//struct hostent *gethostbyname(const char *name);

#include <sys/socket.h>
void SendUdpImage(int port, char *image, int bytes_left)
{
	int sock;
	 struct hostent *hp; /* host information */

	struct sockaddr_in servaddr; /* server address */
	char *my_messsage = "this is a test message";
	char *host = "127.0.0.1";
	int index=0;
	int bytes_to_send;
	int r;
	 int fd; /* our socket */
	 if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { perror("cannot create socket\n"); return ; }

	/* fill in the server's address and data */
	memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	/* look up the address of the server given its name */
	hp = gethostbyname(host);
	if (!hp) { fprintf(stderr, "could not obtain address of %s\n", host); return ;
	}
	/* put the host's address into the server address structure */
	memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    image[0] = 0x55;
    image[1] = 0x55;
    memset(&image[2],0x10,2000);
	while (bytes_left)
	{
	bytes_to_send = min(bytes_left,1200);
	r = sendto(fd, &image[index], bytes_to_send,0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	index+=r;
	bytes_left -=r;
   // printf("Bytes left %d %d\n",bytes_left,r);

	}


}

void VideoStream::ReleaseProcessBuffer(ImageQueueItemF m)
{
	m_FreeBuffer->Push(m);
}

ImageQueueItemF VideoStream::GetFreeBuffer()
{
	ImageQueueItemF qi;
	if (m_FreeBuffer->TryPop(qi))
		return qi;
	qi.m_ptr=0;
//	printf("Free buffer returns %d\n",qi.item_id);
	return qi;
}

void VideoStream::PushProcessBuffer (ImageQueueItemF m)
{
//	printf("pushing image buffer %d\n",m.item_id);
	static int cntr=0;


	if(cntr & 0x1){
		m.m_ill0 = 1;
	}else{
		m.m_ill0 = 0;
	}
	m.m_frameIndex = cntr;
	cntr++;

	m.m_startTime = 0;
	m.m_endTime = m.m_startTime;

	m_ProcessBuffer->Push(m);
}

unsigned short VideoStream::calc_syndrome(unsigned short syndrome, unsigned short p)
{
	return syndrome ^=((syndrome <<5) + (p) +(syndrome >>2));
	return syndrome ^=((syndrome >>5) + (p));
	//return syndrome +p +1;
}

void VideoStream ::SetSeed(unsigned short sd)
{
	seed=sd;
}

void *VideoStream::ThreadServer(void *arg)
{
	VideoStream *vs = (VideoStream *) arg;
	if(vs->m_UseImageAuthentication){
		printf("Image Authentication ThreadServer() start\n");
		int length;
		int pckcnt = 0;
		char buf[IMAGE_SIZE];
		char dummy_buff[IMAGE_SIZE];

		int datalen = 0;
		short *pShort = (short *) buf;
		bool b_syncReceived = false;
		struct sockaddr_in from;
		socklen_t fromlen = sizeof(struct sockaddr_in);
		int bytes_to_read = IMAGE_SIZE;

		int pkgs_received = 0;
		int pkgs_missed = 0;
		int rx_idx = 0;
		unsigned short *hash_data;

		ImageQueueItemF queueItem = vs->GetFreeBuffer();
		char *databuf = (char *) queueItem.m_ptr;

		int leftCSock = CreateUDPServer(vs->m_port);
		if (leftCSock < 0) {
			printf("Failed to create leftC Server()\n");
			return NULL;
		}
		vs->running = 1;
		while (vs->running) {
			length = recvfrom(leftCSock, &databuf[rx_idx], min(1500, bytes_to_read),
					0, (struct sockaddr *) &from, &fromlen);
			if (length < 0)
				printf("recvfrom error in leftCServer()");
			else {
				pkgs_received++;
				if (!b_syncReceived && ((short *) databuf)[0] == 0x5555) {
					datalen = 0;
					memcpy(databuf, &databuf[rx_idx + 2], length - 2);
					rx_idx = length - 2;
					datalen = length - 2;
					b_syncReceived = true;
					pckcnt = 1;
					vs->cam_id = databuf[2] & 0xff;
					vs->frameId = databuf[3] & 0xff;
					// printf("pstream: seed in %d\n", vs->seed);
					vs->syndrome = vs->seed;
					//printf("vs->frameId %02x %02x %02x %02x\n", databuf[0]&0xff, databuf[1]&0xff, databuf[2]&0xff, databuf[3]&0xff);
					//printf("vs->cam_id %02x\n", vs->cam_id);
					// printf("Sync\n");
				} else if (b_syncReceived) {
					hash_data = (unsigned short *) &databuf[rx_idx];

					length =
							(datalen + length <= IMAGE_SIZE - 4) ?
									length : IMAGE_SIZE - 4 - datalen;
					datalen += length;
					pckcnt++;

					// dont do this on the last frame
					if (datalen < IMAGE_SIZE - 5)
						vs->syndrome = vs->calc_syndrome(vs->syndrome, *hash_data);

	//                        memcpy(databuf+datalen, buf, length);
					rx_idx += length;
				}
				bytes_to_read -= length;
				if (datalen >= IMAGE_SIZE - 5) {
					//vs->HandleReceiveImage(databuf, datalen);
					unsigned char valid_image;
					// lets see if calculated matches received
					valid_image = *hash_data == vs->syndrome ? 1 : 0;

					if (valid_image == 0)
						printf("Pstream: Bad Image Calc %x got %x\n", vs->syndrome, *hash_data);
					//else
					//	printf ("Good image\n");
					// if its good we push it
					if (databuf != dummy_buff) {
						 // printf("Pstream: Frame is valid\n");
						if (valid_image)
							vs->PushProcessBuffer(queueItem);
					}
					if (valid_image)
						queueItem = vs->GetFreeBuffer();
					if (!queueItem.m_ptr) {
						pkgs_missed++;
						// printf("no free buffers. Packages received: %d, packages missed: %d\n",	pkgs_received, pkgs_missed);
						databuf = dummy_buff;
					} else {
						databuf = queueItem.m_ptr;
					}

					// if not put data into dummy buffer

					// printf("Got image bytes to read = %d\n",bytes_to_read);
					datalen = 0;
					b_syncReceived = false;
					bytes_to_read = IMAGE_SIZE;
					rx_idx = 0;
				}
				if (bytes_to_read <= 0)
					bytes_to_read = IMAGE_SIZE;
			}
		}
		vs->running = -1;
		close(leftCSock);
	}else{
        printf("No Image Authentication ThreadServer() start\n");
        int length;
        int pckcnt=0;
        char buf[IMAGE_SIZE];
        char dummy_buff[IMAGE_SIZE];

        int datalen = 0;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);
        int bytes_to_read=IMAGE_SIZE;

        int pkgs_received = 0;
        int pkgs_missed = 0;
        int rx_idx=0;

        ImageQueueItemF queueItem = vs->GetFreeBuffer();
        char *databuf = (char *)queueItem.m_ptr;


        int leftCSock = CreateUDPServer(vs->m_port);
        if (leftCSock < 0)
        {
                printf("Failed to create leftC Server()\n");
                return NULL;
        }
        vs->running=1;
        while (vs->running)
        {
            length = recvfrom(leftCSock, &databuf[rx_idx], min(1500,bytes_to_read), 0, (struct sockaddr *)&from, &fromlen);
            if (length < 0)
                printf("recvfrom error in leftCServer()");
            else
            {
            	pkgs_received++;
            	if(!b_syncReceived && ((short *)databuf)[0] == 0x5555)
                {
                        datalen = 0;
                        memcpy(databuf, &databuf[rx_idx+2], length-2);
                        rx_idx = length-2;
                        datalen = length - 2;
                        b_syncReceived = true;
                        pckcnt=1;
                        vs->cam_id=databuf[2]&0xff;
                        vs->frameId=databuf[3]&0xff;
                        //printf("vs->frameId %02x %02x %02x %02x\n", databuf[0]&0xff, databuf[1]&0xff, databuf[2]&0xff, databuf[3]&0xff);
                        //printf("vs->cam_id %02x\n", vs->cam_id);
                       // printf("Sync\n");
                }
                else if(b_syncReceived)
                {
                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
//                        memcpy(databuf+datalen, buf, length);
                        rx_idx+=length;
                        datalen += length;
                        pckcnt++;
                }/*else{
                	//printf("FaceTracker: No sync\n");
                	continue;
                }*/
                bytes_to_read-=  length;
                if(datalen >= IMAGE_SIZE-5)
                {
                    //vs->HandleReceiveImage(databuf, datalen);

                	if (databuf != dummy_buff)
                	{
						vs->PushProcessBuffer(queueItem);
						pkgs_missed=0;
						pkgs_received=0;
						pckcnt = 0;
                	}

					queueItem = vs->GetFreeBuffer();
					// if not put data into dummy buffer
					if (!queueItem.m_ptr)
					{
						pkgs_missed++;
						// printf("FaceTracker: no free buffers. Packages received: %d, packages missed: %d\n", pkgs_received, pkgs_missed);
						databuf = dummy_buff;
					}
					else
					{
						databuf = queueItem.m_ptr;
					}

                   // printf("Got image bytes to read = %d\n",bytes_to_read);
                   	datalen = 0;
                   	b_syncReceived=false;
                   	bytes_to_read=IMAGE_SIZE;
                   	rx_idx=0;
               }
               if(bytes_to_read<=0)
            	   bytes_to_read=IMAGE_SIZE;
          }
      }
       vs->running =-1;
      close(leftCSock);
	}
}

VideoStream ::VideoStream(int port, bool ImageAuthFlag) : frameId(0)
{
	m_port = port;
	m_UseImageAuthentication = ImageAuthFlag;
	offset_sub_enable=0;
	offset_image_loaded=0;
	ImageQueueItemF imageQueueItem;
	seed = 0;

	m_pRingBuffer = new RingBufferImageQueueF(2);
	length = (HEIGHT * WIDTH);

	m_current_process_queue_item.m_ptr = NULL;
	m_FreeBuffer = new RingBufferImageQueueF(FREE_BUFF_SIZE);
	m_ProcessBuffer = new RingBufferImageQueueF(FREE_BUFF_SIZE);
	for (int x = 0 ; x < FREE_BUFF_SIZE; x++)
	{
		imageQueueItem.m_ptr = new unsigned char[length];
		m_FreeBuffer->Push(imageQueueItem);
	}


	if (pthread_create (&Thread, NULL, ThreadServer, (void *)this))
	{
			printf("MainLoop(): Error creating thread FaceServer\n");
			//return 0;
	}
	//return 1;
}

#ifdef old_stuff

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





int vid_stream_flush(void)

{
	ImageQueue val;
		while((m_pRingBuffer->TryPop(val)) != false);

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
		ret = recvfrom(s, buf, min(BUFLEN,bytes_to_get), 0, (struct sockaddr*)&si_other, &slen);
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

#endif
