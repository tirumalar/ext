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

int m_port;

#define FREE_BUFF_SIZE 2
#if 1
void VideoStream::flush() {
	ImageQueueItemF val;
	while ((m_pRingBuffer->TryPop(val)) != false)
		usleep(1000);
}
#else
void VideoStream::flush()
{

	// Move any stale face images from process buffer into Free buffer...

	// Push any active item back into the Free buffer...

	if (m_current_process_queue_item.m_ptr != NULL)

	ReleaseProcessBuffer(m_current_process_queue_item); // Push item into free buffer...


	// Remove item from process buffer...

	while (m_ProcessBuffer->TryPop(m_current_process_queue_item))

	ReleaseProcessBuffer(m_current_process_queue_item); //Push item into Free buffer


	m_current_process_queue_item.m_ptr = NULL; //Required to prevent pushing this item into free buffer twice!

}

#endif
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

int VideoStream::get(int *win,int *hin,char *m_pImageBuffer, bool bDebugFlag, char get_last)
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
    	//EyelockLog(loggerp, INFO, "vs->get() Before ReleaseProcessBuffer...");
    	ReleaseProcessBuffer(m_current_process_queue_item);
    	//EyelockLog(loggerp, INFO, "vs->get() After ReleaseProcessBuffer...");
    }

    if(bDebugFlag)
    {
    	int nwaitCount = 0;
    	while((m_ProcessBuffer->TryPop(m_current_process_queue_item)) != true)
		{
    		usleep(1000);
			if((nwaitCount++) > 15000) // If we don't get an image within 15 seconds return false
				return false;

			//EyelockLog(loggerp, INFO, "vs->get() TryPop failed!");
		}
    }
    else
    {
		while((m_ProcessBuffer->TryPop(m_current_process_queue_item)) != true)
		{
			usleep(1000);
		}
    }
#if 0
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
#endif
	memcpy(m_pImageBuffer, m_current_process_queue_item.m_ptr, m_ImageSize);

	// EyelockLog(loggerp, INFO, "vs->get() OK returning image");

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

#if 0 // Anita for listening from another application remove later
	int enable = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int));
#endif

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
		char buf[vs->m_ImageSize];
		char dummy_buff[vs->m_ImageSize];

		int datalen = 0;
		short *pShort = (short *) buf;
		bool b_syncReceived = false;
		struct sockaddr_in from;
		socklen_t fromlen = sizeof(struct sockaddr_in);
		int bytes_to_read = vs->m_ImageSize;

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
			{
				EyelockLog(loggerp, ERROR, "socket error in receiving face images! %s %d\n", strerror(length), vs->m_port);
				 printf("recvfrom error in leftCServer()");
				 perror;
			}else {
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
							(datalen + length <= vs->m_ImageSize - 4) ?
									length : vs->m_ImageSize - 4 - datalen;
					datalen += length;
					pckcnt++;

					// dont do this on the last frame
					if (datalen < vs->m_ImageSize - 5)
						vs->syndrome = vs->calc_syndrome(vs->syndrome, *hash_data);

	//                        memcpy(databuf+datalen, buf, length);
					rx_idx += length;
				}
				bytes_to_read -= length;
				if (datalen >= vs->m_ImageSize - 5) {
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
					bytes_to_read = vs->m_ImageSize;
					rx_idx = 0;
				}
				if (bytes_to_read <= 0)
					bytes_to_read = vs->m_ImageSize;
			}
		}
		vs->running = -1;
		close(leftCSock);
	}else{
        printf("No Image Authentication ThreadServer() start\n");
        int length;
        int pckcnt=0;
        char buf[vs->m_ImageSize];
        char dummy_buff[vs->m_ImageSize];

        int datalen = 0;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);
        int bytes_to_read=vs->m_ImageSize;

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
        // unsigned int count = 0;
        while (vs->running)
        {
            length = recvfrom(leftCSock, &databuf[rx_idx], min(1500,bytes_to_read), 0, (struct sockaddr *)&from, &fromlen);
            if (length < 0)
            {
            	EyelockLog(loggerp, ERROR, "socket error in receiving face images! %s %d\n", strerror(length), vs->m_port);

                printf("recvfrom error in leftCServer()");
                perror;
            }
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
                        length = (datalen+length <= vs->m_ImageSize-4) ? length : vs->m_ImageSize-4-datalen;
//                        memcpy(databuf+datalen, buf, length);
                        rx_idx+=length;
                        datalen += length;
                        pckcnt++;
                }/*else{
                	//printf("FaceTracker: No sync\n");
                	continue;
                }*/
                bytes_to_read-=  length;
                if(datalen >= vs->m_ImageSize-5)
                {
                    //vs->HandleReceiveImage(databuf, datalen);

                	if (databuf != dummy_buff)
                	{
						vs->PushProcessBuffer(queueItem);
						/*
						if(count % 4*5 == 0){ // Every 5 seconds
							const cv::Mat img(cv::Size(1200, 960), CV_8U, queueItem.m_ptr);
							cv::imwrite("FaceSocket.pgm",img);
						}*/
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
                   	bytes_to_read=vs->m_ImageSize;
                   	rx_idx=0;
               }
               if(bytes_to_read<=0)
            	   bytes_to_read=vs->m_ImageSize;
          }
           // count ++;
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

	// Read RGB LED Brightness
	FileConfiguration Config("/home/root/Eyelock.ini");
	rgbLEDBrightness = Config.getValue("GRI.LEDBrightness", 80);

	m_ImageWidth = Config.getValue("FrameSize.width", 1200);
	m_ImageHeight = Config.getValue("FrameSize.height", 960);

	m_ImageSize = (m_ImageWidth * m_ImageHeight);

	m_current_process_queue_item.m_ptr = NULL;
	m_FreeBuffer = new RingBufferImageQueueF(FREE_BUFF_SIZE);
	m_ProcessBuffer = new RingBufferImageQueueF(FREE_BUFF_SIZE);



	for (int x = 0 ; x < FREE_BUFF_SIZE; x++)
	{
		imageQueueItem.m_ptr = new unsigned char[m_ImageSize];
		m_FreeBuffer->Push(imageQueueItem);
	}

	if (pthread_create (&Thread, NULL, ThreadServer, (void *)this))
	{
			printf("MainLoop(): Error creating thread FaceServer\n");
			//return 0;
	}
	pthread_setname_np(Thread, "ThreadServer");

	//return 1;
}

