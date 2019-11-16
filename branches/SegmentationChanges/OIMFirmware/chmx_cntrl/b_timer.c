/*
 * b_timer.c
 *
 *  Created on: May 5, 2016
 *      Author: PTG
 */



#include <stdio.h>

/* MicroC/OS-II definitions */
#include "includes.h"

/* Simple Socket Server definitions */
#include "simple_socket_server.h"
#include "alt_error_handler.h"

/* Nichestack definitions */
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "basicli.h"



OS_STK    b_timer_task_stack[TASK_STACKSIZE];

typedef struct
{
	char timer_cmd[100];
	int  timer_timeout;
	int  time_to_do;
}B_TIMER_STRUCT;

#define NUM_TIMERS 2
#define B_TIMER_DELAY_INTERVAL 10
volatile B_TIMER_STRUCT timers[NUM_TIMERS];


void b_timer_set_fd(int fd)
{

}

char g_on_time_text[1000];

void b_on_time_print(char *text)
{
	printf("TPRint:%s\n",text);
	strcat(g_on_time_text,text);
}

OS_EVENT  *mutex_timer;


void b_on_time(int timer, int time_out, char *cmd)
{
  INT8U return_code = OS_NO_ERR;
  printf("B on time pend \n");
  OSMutexPend(mutex_timer, 0, &return_code);
  if (return_code)
     printf("error timer 2 mutex %d\n",return_code);

  if (strlen(cmd)>50)
      printf("Length error\n");
  strcpy(timers[timer].timer_cmd,cmd);
  timers[timer].timer_timeout = time_out;
  timers[timer].time_to_do = 30;

  printf("setting timer %d %d %s\n",timer,time_out,cmd);
  return_code = OSMutexPost(mutex_timer);
 // alt_ucosii_check_return_code(return_code);
}

#define MS_PER_TICK 2
extern volatile int g_cam_set;
extern volatile int g_time_set;
char cmd_t_buff[100];

int g_fcount=0;
int g_f_state=0;

void b_timer_task(void *task_data)
{
	int x;
	int div_count=50;
	INT8U return_code;

	//  alt_ucosii_check_return_code(return_code);

	memset(g_on_time_text,0,sizeof(g_on_time_text));
	while (1)
	    {
	    int del;
	    g_fcount++;
	    g_f_state=3;
	    del = g_time_set==0?1:g_time_set;
	    if (del>200)
		del=100;
	    OSTimeDly(del);

	    g_f_state=4;

	    if(g_cam_set>0)
		{
		void grab_send(int cam);
		g_f_state=1;
		grab_send(g_cam_set);
		if (g_cam_set&0x40)
		    g_cam_set^=0x80;

		g_f_state=2;
		//sprintf(cmd_t_buff,"grab_send(%d)\n",g_cam_set);
		//   printf ("calling %s",cmd_t_buff);
				  // parse_prog(cmd_t_buff);
			//	   timers[x].time_to_do+=timers[x].timer_timeout;
			//	   printf ("done \n");
		}

	    }
	while (1)
	{
		//if (strlen(g_on_time_text)>0)
		//{
			//cli_printf(g_on_time_text);
			//memset(g_on_time_text,0,sizeof(g_on_time_text));
		//}
		OSTimeDly(B_TIMER_DELAY_INTERVAL);
		OSMutexPend(mutex_timer, 0, &return_code);
		    if (return_code)
		       printf("error timer 1 mutex %d\n",return_code);
		for (x=0; x < NUM_TIMERS;x++)
			if (timers[x].timer_timeout>0)
			{
			   timers[x].time_to_do -=  B_TIMER_DELAY_INTERVAL*MS_PER_TICK;
			   if (timers[x].time_to_do<=0)
				   {
				   printf ("calling %s",timers[x].timer_cmd);
				   parse_prog(timers[x].timer_cmd);
				   timers[x].time_to_do+=timers[x].timer_timeout;
				   printf ("done \n");
				   }

			}
		 return_code = OSMutexPost(mutex_timer);
//		 alt_ucosii_check_return_code(return_code);


	}

}



void b_timer_Init()
{
INT8U error_code;
int x;

for (x=0; x < NUM_TIMERS;x++)
	timers[x].timer_timeout=0;


mutex_timer = OSMutexCreate(MUTEX_BTIMER_PRIORITY, &error_code);

error_code = OSTaskCreateExt(b_timer_task,
                           NULL,
                           (void *)&b_timer_task_stack[TASK_STACKSIZE],
                           B_TIMER_PRIORITY,
                           B_TIMER_PRIORITY,
                           b_timer_task_stack,
                           TASK_STACKSIZE,
                           NULL,
                           0);
alt_uCOSIIErrorHandler(error_code, 0);
}
