#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <ctime>

#include "eyelock_com.h"
#include "portcom.h"
#include "pstream.h"

#if 1
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv/cv.h>
#include <opencv2/photo/photo.hpp>
#endif

#include "logging.h"

extern int IrisFrameCtr;

int match_Lock=0;


bool b_goHome=false;

extern int move_counts;
// extern void port_com_send(char *cmd);

const char logger[30] = "eyelock_com";


int g_MatchState = 0;


int ec_read_from_client(OIMQueueItem &theItem)
{
	EyelockLog(logger, DEBUG, "ec_read_from_client");

	char buffer[MAXMSG];
	//int nbytes;
	static int No_eyes_counter = 0, first_detect = 0;

	memset(buffer, 0, MAXMSG);
	//nbytes = read(filedes, buffer, MAXMSG);

	strcpy(buffer, theItem.m_Message);

	int nbytes = strlen(buffer);

	if (nbytes <= 0)
	{
		/* Bogus message */
//		perror("read");
		EyelockLog(logger, ERROR, "ec_read_from_client received zero-length Message");
		return 0;
	}
	else
	{
		/* Data read. */
		buffer[nbytes] = 0;
		if (strchr(buffer, '\n'))
			*strchr(buffer, '\n') = ' ';
		if (strchr(buffer, '\r'))
			*strchr(buffer, '\r') = ' ';


		//ilya added this code for testing
		if (strstr(buffer,"fixed_set_rgb(0,0,80)"))
			g_MatchState=MATCHER_STAT_DETECT;
		if (strstr(buffer,"fixed_set_rgb(64,80,80)"))
			g_MatchState=MATCHER_STAT_MATCH;
		if (strstr(buffer,"fixed_set_rgb(80,0,0)"))
			g_MatchState=MATCHER_STAT_FAIL;

		//printf ("EC Server: got message: `%s'\n", buffer);

		//Process the commands sent by EyeLock app
		if (strstr(buffer, "MATCH_FAIL"))
		{
			fprintf(stderr, "EC Server: got message: `%s  %s'\n", buffer, GetTimeStamp());

			//IrisFrameCtr = MIN_IRIS_FRAMES;
			//No_eyes_counter=0;
			//port_com_send("fixed_set_rgb(0,100,0)");
			//setRGBled(BRIGHTNESS_MAX,0,0,2000,0,0x1F);
			// system("nc -O 512 192.168.4.172 35 < /home/root/tones/rej.raw");
			//SetFaceMode();
    	 // port_com_send("play_snd(1)");
		}
		else if (strstr(buffer, "MATCH"))
		{
			fprintf(stderr, "EC Server: got message: `%s  %s'\n", buffer, GetTimeStamp());

			//IrisFrameCtr = MIN_IRIS_FRAMES;
			No_eyes_counter = 0;
			b_goHome = true;
			//port_com_send("fixed_set_rgb(0,100,0)");
			//setRGBled(0,BRIGHTNESS_MIN,0,2000,1,0x1F);
//    	  port_com_send("set_audio(1)");
//    	  system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
    	 // port_com_send("play_snd(0)");

			//  MoveTo(CENTER_POS);
			//sleep(2);
			//setRGBled(BRIGHTNESS_MIN,BRIGHTNESS_MIN,BRIGHTNESS_MIN,10,0,0x1F);
			move_counts = 0;
			//SetFaceMode();
			//match_Lock=0;
		}
		else if (strstr(buffer, "DETECT"))
		{
			if (first_detect == 0)
				first_detect = 1;
			else
			{
				No_eyes_counter = 0;
				//port_com_send("fixed_set_rgb(0,0,100)");
				// setRGBled(0,0,BRIGHTNESS_MELLOW,1000,0,0x1F);
				match_Lock = 0;
			}
			/*    	  IrisFrameCtr++;		//count detect
			 printf("count: ---------------------------------------- %i\n", IrisFrameCtr);*/
		}
		else if (buffer[0] == 'b')
		{
//    	  char temp[100];
//    	  sprintf(temp,"\"%s\"",buffer);
//    	  port_com_send(buffer);
//    	  fprintf (stderr, "EC Server: got message: `%s  %s'\n", buffer,GetTimeStamp());
//    	  No_eyes_counter=0;
			//printf("buffer content %s\n",buffer);
		}
		else if (buffer[0] == 'N')
		{
//    	  fprintf (stderr, "EC Server: got message: `%s  %s'\n", buffer,GetTimeStamp());
//    	  No_eyes_counter++;
//    	  if(No_eyes_counter>NO_EYES_THRESHOLD)
//    	  {
//    		  No_eyes_counter=0;
//    		  IrisFrameCtr = MIN_IRIS_FRAMES;
//    	  }
//    	  RecoverModeDrop();
		}
		else //if(buffer[0]=='f')
		{
			char temp[100];
			sprintf(temp, "\"%s\"", buffer);
			EyelockLog(logger, DEBUG, "EC Server: got message: `%s  %s'\n", buffer, GetTimeStamp());
			port_com_send(buffer);

          	if(!(strncmp(temp,"fixed_set_rgb(0,80,0)",20)))
			//if(0)
			{
				EyelockLog(logger, DEBUG, "EC Server: Audio got message: `<%s>  %s'\n", buffer, GetTimeStamp());
				usleep(10000);
				//   		port_com_send("set_audio(1)");
				//	system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
				// 		port_com_send("set_audio(0)");
			}

			if (b_goHome)
			{
				//usleep(10000)
				//MoveTo(CENTER_POS);
				/*port_com_send("psoc_write(3,0x00)");
				 cvWaitKey(1000);*/
				//port_com_send("set_audio(2)"); // to play auth sound
				b_goHome = false;
			}

		}

		return 0;
	}
}


#if 0
int ec_read_from_client(int filedes) {

	EyelockLog(logger, DEBUG, "ec_read_from_client");

	char buffer[MAXMSG];
	int nbytes;
	static int No_eyes_counter = 0, first_detect = 0;
	memset(buffer, 0, MAXMSG);
	nbytes = read(filedes, buffer, MAXMSG);

	if (nbytes < 0) {
		/* Read error. */
		perror("read");
		EyelockLog(logger, ERROR, "ec_read_from_client socket read error %d", nbytes);
		exit (EXIT_FAILURE);
	} else if (nbytes == 0){
		EyelockLog(logger, ERROR, "ec_read_from_client bytes read is zero %d", nbytes);
		/* End-of-file. */
		return 0;
	}else {
		/* Data read. */
		buffer[nbytes] = 0;
		if (strchr(buffer, '\n'))
			*strchr(buffer, '\n') = ' ';
		if (strchr(buffer, '\r'))
			*strchr(buffer, '\r') = ' ';


		//ilya added this code for testing
		if (strstr(buffer,"fixed_set_rgb(0,0,80)"))
			g_MatchState=MATCHER_STAT_DETECT;
		if (strstr(buffer,"fixed_set_rgb(64,80,80)"))
			g_MatchState=MATCHER_STAT_MATCH;
		if (strstr(buffer,"fixed_set_rgb(80,0,0)"))
			g_MatchState=MATCHER_STAT_FAIL;

		//printf ("EC Server: got message: `%s'\n", buffer);

		//Process the commands sent by EyeLock app
		if (strstr(buffer, "MATCH_FAIL")) {
			fprintf(stderr, "EC Server: got message: `%s  %s'\n", buffer,
					GetTimeStamp());

			//IrisFrameCtr = MIN_IRIS_FRAMES;
			//No_eyes_counter=0;
			//port_com_send("fixed_set_rgb(0,100,0)");
			//setRGBled(BRIGHTNESS_MAX,0,0,2000,0,0x1F);
			// system("nc -O 512 192.168.4.172 35 < /home/root/tones/rej.raw");
			//SetFaceMode();
    	 // port_com_send("play_snd(1)");
		} else if (strstr(buffer, "MATCH")) {
			fprintf(stderr, "EC Server: got message: `%s  %s'\n", buffer,
					GetTimeStamp());

			//IrisFrameCtr = MIN_IRIS_FRAMES;
			No_eyes_counter = 0;
			b_goHome = true;
			//port_com_send("fixed_set_rgb(0,100,0)");
			//setRGBled(0,BRIGHTNESS_MIN,0,2000,1,0x1F);
//    	  port_com_send("set_audio(1)");
//    	  system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
    	 // port_com_send("play_snd(0)");

			//  MoveTo(CENTER_POS);
			//sleep(2);
			//setRGBled(BRIGHTNESS_MIN,BRIGHTNESS_MIN,BRIGHTNESS_MIN,10,0,0x1F);
			move_counts = 0;
			//SetFaceMode();
			//match_Lock=0;
		} else if (strstr(buffer, "DETECT")) {
			if (first_detect == 0) {
				first_detect = 1;
			} else {

				No_eyes_counter = 0;
				//port_com_send("fixed_set_rgb(0,0,100)");
				// setRGBled(0,0,BRIGHTNESS_MELLOW,1000,0,0x1F);
				match_Lock = 0;

			}
			/*    	  IrisFrameCtr++;		//count detect
			 printf("count: ---------------------------------------- %i\n", IrisFrameCtr);*/

		} else if (buffer[0] == 'b') {
//    	  char temp[100];
//    	  sprintf(temp,"\"%s\"",buffer);
//    	  port_com_send(buffer);
//    	  fprintf (stderr, "EC Server: got message: `%s  %s'\n", buffer,GetTimeStamp());
//    	  No_eyes_counter=0;
			//printf("buffer content %s\n",buffer);
		} else if (buffer[0] == 'N') {
//    	  fprintf (stderr, "EC Server: got message: `%s  %s'\n", buffer,GetTimeStamp());
//    	  No_eyes_counter++;
//    	  if(No_eyes_counter>NO_EYES_THRESHOLD)
//    	  {
//    		  No_eyes_counter=0;
//    		  IrisFrameCtr = MIN_IRIS_FRAMES;
//    	  }
//    	  RecoverModeDrop();
		} else //if(buffer[0]=='f')
		{
			char temp[100];
			sprintf(temp, "\"%s\"", buffer);
			EyelockLog(logger, DEBUG, "EC Server: got message: `%s  %s'\n", buffer, GetTimeStamp());
			port_com_send(buffer);

          	if(!(strncmp(temp,"fixed_set_rgb(0,80,0)",20)))
			//if(0)
					{
				EyelockLog(logger, DEBUG, "EC Server: Audio got message: `<%s>  %s'\n", buffer, GetTimeStamp());
				usleep(10000);
				//   		port_com_send("set_audio(1)");
				//	system("nc -O 512 192.168.4.172 35 < /home/root/tones/auth.raw");
				// 		port_com_send("set_audio(0)");
			}

			if (b_goHome) {
				//usleep(10000)
				//MoveTo(CENTER_POS);
				/*port_com_send("psoc_write(3,0x00)");
				 cvWaitKey(1000);*/
				//port_com_send("set_audio(2)"); // to play auth sound
				b_goHome = false;
			}

		}

		return 0;
	}
}


int ec_make_socket(uint16_t port) {
	EyelockLog(logger, TRACE, "ec_make_socket");
	int sock;
	struct sockaddr_in name;

	/* Create the socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if (sock < 0) {
		EyelockLog(logger, ERROR, "ec_make_socket socket error");
		perror("socket");
		exit (EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	unlink((char*) sock);
	if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
		EyelockLog(logger, ERROR, "ec_make_socket socket bind error");
		EyelockLog(logger, DEBUG, "Port %d ", port);
		perror("bind");
		exit (EXIT_FAILURE);
	}

	return sock;
}

#endif

//DMOOUT static int sock;
//DMOOUT static fd_set active_fd_set, read_fd_set;


OIMQueue *g_pOIMQueue = NULL;


void AllocateOIMQueue(int nSize)
{
	g_pOIMQueue = new OIMQueue(nSize);
}

LeftCameraFaceQueue *g_pLeftCameraFaceQueue = NULL;
RightCameraFaceQueue *g_pRightCameraFaceQueue = NULL;
FaceCameraQueue *g_pCameraFaceQueue = NULL;

void AllocateFaceQueue(int nSize)
{
	printf("Inside AllocateFaceQueue\n");
	g_pLeftCameraFaceQueue = new LeftCameraFaceQueue(nSize);
	g_pRightCameraFaceQueue = new RightCameraFaceQueue(nSize);
}

/*
void deallocateOIMQueue()
{
	if (OIMQueue)
		delete [] OIMQueue;
	if (RingBufferFaceQueue)
		delete [] RingBufferFaceQueue;
}
*/


void *init_ec(void * arg) {
	EyelockLog(logger, TRACE, "init_ec");
	//extern int make_socket(uint16_t port);
	int i;
	//struct sockaddr_in clientname;
	//unsigned int size;

	VideoStream *vs = (VideoStream*) arg;
	// Create Queue for communicating with Eyelock CMXHandler
//	g_pOIMQueue = new OIMQueue(10); //DMO pull from config later...
	// Wait until a message arrives in the queue...
	while (1)
	{
		//wait for something to arrive in the queue...
		OIMQueueItem theItem = (OIMQueueItem)g_pOIMQueue->Pop();

		if(vs->m_UseImageAuthentication){
			char cmd[100];
			sprintf(cmd,"cam_set_seed(%i)", theItem.m_RandomSeed);		//Set the seed
			port_com_send(cmd);
			vs->seed = theItem.m_RandomSeed;
		}
		//Process the item returned from the queue...
		ec_read_from_client(theItem);
	}
}


#if 0 //DMO OLD Socket code
void *init_ec(void * arg) {
	EyelockLog(logger, TRACE, "init_ec");
	extern int make_socket(uint16_t port);
	int i;
	struct sockaddr_in clientname;
	unsigned int size;

	/* Create the socket and set it up to accept connections. */

	printf("starting server \n");
	sock = make_socket(PORT);
	if (listen(sock, 1) < 0) {
		EyelockLog(logger, ERROR, "init_ec socket listen error");
		perror("listen");
		exit (EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	while (1) {
		//printf("Inside : Block until input arrives on one or more active sockets\n");
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			EyelockLog(logger, ERROR, "init_ec socket select error");
			perror("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET(i, &read_fd_set)) {
				if (i == sock) {
					/* Connection request on original socket. */
					int new1;
					size = sizeof(clientname);
					new1 = accept(sock, (struct sockaddr *) &clientname, &size);
					if (new1 < 0) {
						EyelockLog(logger, ERROR, "init_ec socket accept error");
						perror("accept");
						exit (EXIT_FAILURE);
					}
					EyelockLog(logger, TRACE, "Server: connect from host (eyelock_com)");
					//inet_ntoa (clientname.sin_addr),
					//ntohs (clientname.sin_port));
					FD_SET(new1, &active_fd_set);
				} else {
					/* Data arriving on an already-connected socket. */
					if (ec_read_from_client(i) < 0) {
						close(i);
						FD_CLR(i, &active_fd_set);
					}
				}
			}
	}
}
#endif

