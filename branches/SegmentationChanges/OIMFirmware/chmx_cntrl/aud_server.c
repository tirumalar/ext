/*
 * aud_server.c
 *
 *  Created on: Apr 13, 2017
 *      Author: PTG
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* MicroC/OS-II definitions */
#include "includes.h"

/* Simple Socket Server definitions */
#include "simple_socket_server.h"
#include "alt_error_handler.h"

/* Nichestack definitions */
#include "ipport.h"
#include "tcpport.h"
#define AUD_PORT 35

 extern OS_EVENT           * MyMutex;

char g_AudioMode = 0;
/*
 * sss_reset_connection()
 *
 * This routine will, when called, reset our SSSConn struct's members
 * to a reliable initial state. Note that we set our socket (FD) number to
 * -1 to easily determine whether the connection is in a "reset, ready to go"
 * state.
 */

#define audConn SSSConn
void aud_reset_connection(audConn* conn)
{
  memset(conn, 0, sizeof(audConn));

  conn->fd = -1;
  conn->state = READY;
  conn->rx_wr_pos = conn->rx_buffer;
  conn->rx_rd_pos = conn->rx_buffer;
  return;
}
void aud_handle_accept(int listen_socket, audConn* conn)
{
  int                 socket, len;
  struct sockaddr_in  incoming_addr;

  len = sizeof(incoming_addr);

  if ((conn)->fd == -1)
  {
     if((socket=accept(listen_socket,(struct sockaddr*)&incoming_addr,&len))<0)
     {
         alt_NetworkErrorHandler(EXPANDED_DIAGNOSIS_CODE,
                                 "[aud_handle_accept] accept failed");
     }
     else
     {

        (conn)->fd = socket;
//        aud_send_image(conn);
        //sss_send_menu(conn);
       // printf("[aud_handle_accept] accepted connection request from %s\n",
    //           inet_ntoa(incoming_addr.sin_addr));
        // figure out dest mac and ip

     }
  }
  else
  {
    //printf("[aud_handle_accept] rejected connection request from %s\n",
       //    inet_ntoa(incoming_addr.sin_addr));
  }

  return;
}


#define AUD_CHUNK_SIZE 2048*2
short aud_buff[AUD_CHUNK_SIZE];
int aud_play_array(short *aud_buff,int samples);
extern SSSConn* g_conn;

#include "data_store.h"
#include "cam.h"

AUDIO_STORE a_store[NUM_AUD_STORES];
char *current_store=NULL;
int current_store_max=0;
int current_store_len=0;
int current_id=0;
char *get_img_buffer(void);

int data_store_get_cur_len(void)
{
    return current_store_len;
}

char * data_store_info(int id, int *max_len)
    {
    char *p=0;
    *max_len=0;
    switch (id)
   	    {
   	    case 0: p=a_store[0].audio;*max_len=AUDIO_STORE_SIZE;break;
   	    case 1: p=a_store[1].audio;*max_len=AUDIO_STORE_SIZE;break;
   	    case 2: p=a_store[2].audio;*max_len=AUDIO_STORE_SIZE;break;
   	    case 3: p=a_store[3].audio;*max_len=AUDIO_STORE_SIZE;break;
   	    case 4: p=get_img_buffer();
   		    *max_len=1000000;break;
   	    }
    current_id=id;
    return p;
    }

extern volatile int g_cam_set;
void data_store_set(int id)
    {
    current_store=data_store_info(id,&current_store_max);
    // incase we are using the image buffer stop capture
    if (id==4)
	g_cam_set=0;

    current_store_len=0;
    cli_printf("data store set %x",current_store);
    }

void aud_handle_receive(audConn* conn)
{
  int data_used = 0, rx_code = 0;
  fd_set readfds;
  char *lf_addr;
  struct timeval timeout;


  timeout.tv_sec=1;
  conn->rx_rd_pos = conn->rx_buffer;
  conn->rx_wr_pos = conn->rx_buffer;

 // printf("[aud_handle_receive] processing RX data\n");
  FD_ZERO(&readfds);
  FD_SET(conn->fd, &readfds);

  while(conn->state != CLOSE)
  {
      {
    	 // printf("W");
    	  rx_code=select(1, &readfds, NULL, NULL,0);//&timeout);
    	  if (rx_code<1)
    	  {
    		  printf("time out\n");
    		  continue;
    	  }

    	  //printf("Selected\n");
      rx_code = recv(conn->fd, (char *)aud_buff,
      AUD_CHUNK_SIZE, 0);
//printf("Got %d\n",rx_code);
     // printf(".");
     if(rx_code > 0)
      {
    	  SSSConn* t_conn;
    	 unsigned char err;

//    	 if (g_AudioMode)
    	 if (1)
    	 {
    	     //aud_play_array(aud_buff,rx_code/2);
    	 if (current_store && (current_store_len+rx_code<current_store_max))
    		 {
    		 memcpy(&current_store[current_store_len],aud_buff,rx_code);
    		 cli_printf("Writint to %x %d\n",&current_store[current_store_len],rx_code);
    		 current_store_len+=rx_code;
    		 if (current_id <NUM_AUD_STORES)
    		      a_store[current_id].len=current_store_len;
    		 }
       //       memcpy(&aud_store[aud_store_len],aud_buff,rx_code);
       //       aud_store_len+=rx_code;
    	     send(conn->fd, "A",1, 0);
    	   // printf("Audio %d\n",rx_code);
    	 }
    	 else
    	 {
    	 aud_buff[rx_code]=0;
    	 OSMutexPend(MyMutex, 0, &err);

    	 t_conn=g_conn;
    	 g_conn=conn;
    	   if (aud_buff[0]=='!')
    	   {
    		   conn->close = CLOSE;
    		   return;
    	   }
    	   parse_prog(aud_buff);
      	 g_conn=t_conn;
      	 OSMutexPost(MyMutex);
    	 }
      }
     else
     {
    	 //close connection
    	 conn->close = CLOSE;
     }
    }

    /*
     * When the quit command is received, update our connection state so that
     * we can exit the while() loop and close the connection
     */
    conn->state = conn->close ? CLOSE : READY;


  }

 // printf("[aud_handle_receive] closing connection\n");
  close(conn->fd);
  aud_reset_connection(conn);

  return;
}



short snd_bad[]={195,362,473,512,473,362,195,0,-196,-363,-474,-512,-474,-363,-196,-1};
void AudServerTask()
{
  int fd_listen, max_socket;
  struct sockaddr_in addr;
  static audConn conn;
  fd_set readfds;
  short *aud_p;
  int x;
  int as;

  /*
   * Sockets primer...
   * The socket() call creates an endpoint for TCP of UDP communication. It
   * returns a descriptor (similar to a file descriptor) that we call fd_listen,
   * or, "the socket we're listening on for connection requests" in our aud
   * server example.
   *
   * Traditionally, in the Sockets API, PF_INET and AF_INET is used for the
   * protocol and address families respectively. However, there is usually only
   * 1 address per protocol family. Thus PF_INET and AF_INET can be interchanged.
   * In the case of NicheStack, only the use of AF_INET is supported.
   * PF_INET is not supported in NicheStack.
   */

  {
  void snd_init(void);
  snd_init();
  }

  for ( as =0 ; as <NUM_AUD_STORES;as++)
      {
      aud_p=&a_store[as].audio[0];
      for (x=0;x<4000;x++)
	  {
	  *aud_p++ = snd_bad[((x*(as+1)))%16]*50;
	  }
      a_store[as].len=8000;
      }

  if ((fd_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    alt_NetworkErrorHandler(EXPANDED_DIAGNOSIS_CODE,"[aud_task] Socket creation failed");
  }

  /*
   * Sockets primer, continued...
   * Calling bind() associates a socket created with socket() to a particular IP
   * port and incoming address. In this case we're binding to aud_PORT and to
   * INADDR_ANY address (allowing anyone to connect to us. Bind may fail for
   * various reasons, but the most common is that some other socket is bound to
   * the port we're requesting.
   */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(AUD_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;

  if ((bind(fd_listen,(struct sockaddr *)&addr,sizeof(addr))) < 0)
  {
    alt_NetworkErrorHandler(EXPANDED_DIAGNOSIS_CODE,"[aud_task] Bind failed");
  }

  /*
   * Sockets primer, continued...
   * The listen socket is a socket which is waiting for incoming connections.
   * This call to listen will block (i.e. not return) until someone tries to
   * connect to this port.
   */
  if ((listen(fd_listen,1)) < 0)
  {
    alt_NetworkErrorHandler(EXPANDED_DIAGNOSIS_CODE,"[aud_task] Listen failed");
  }

  /* At this point we have successfully created a socket which is listening
   * on aud_PORT for connection requests from any remote address.
   */
  aud_reset_connection(&conn);
  //printf("[aud_task] Simple Socket Server listening on port %d\n", AUD_PORT);

  while(1)
  {
    /*
     * For those not familiar with sockets programming...
     * The select() call below basically tells the TCPIP stack to return
     * from this call when any of the events I have expressed an interest
     * in happen (it blocks until our call to select() is satisfied).
     *
     * In the call below we're only interested in either someone trying to
     * connect to us, or data being available to read on a socket, both of
     * these are a read event as far as select is called.
     *
     * The sockets we're interested in are passed in in the readfds
     * parameter, the format of the readfds is implementation dependant
     * Hence there are standard MACROs for setting/reading the values:
     *
     *   FD_ZERO  - Zero's out the sockets we're interested in
     *   FD_SET   - Adds a socket to those we're interested in
     *   FD_ISSET - Tests whether the chosen socket is set
     */
    FD_ZERO(&readfds);
    FD_SET(fd_listen, &readfds);
    max_socket = fd_listen+1;

    if (conn.fd != -1)
    {
      FD_SET(conn.fd, &readfds);
      if (max_socket <= conn.fd)
      {
        max_socket = conn.fd+1;
      }
    }

    select(max_socket, &readfds, NULL, NULL, NULL);

    /*
     * If fd_listen (the listening socket we originally created in this thread
     * is "set" in readfs, then we have an incoming connection request. We'll
     * call a routine to explicitly accept or deny the incoming connection
     * request (in this example, we accept a single connection and reject any
     * others that come in while the connection is open).
     */
    if (FD_ISSET(fd_listen, &readfds))
    {
      aud_handle_accept(fd_listen, &conn);
     aud_handle_receive(&conn);
    }
    /*
     * If aud_handle_accept() accepts the connection, it creates *another*
     * socket for sending/receiving data over aud. Note that this socket is
     * independant of the listening socket we created above. This socket's
     * descriptor is stored in conn.fd. If conn.fs is set in readfs... we have
     * incoming data for our aud server, and we call our receiver routine
     * to process it.
     */
    else
    {
      if ((conn.fd != -1) && FD_ISSET(conn.fd, &readfds))
      {
        aud_handle_receive(&conn);
      }
    }
  } /* while(1) */
}


