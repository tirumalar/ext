/******************************************************************************
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
* All rights reserved. All use of this software and documentation is          *
* subject to the License Agreement located at the end of this file below.     *
*******************************************************************************                                                                             *
* Date - October 24, 2006                                                     *
* Module - iniche_init.c                                                      *
*                                                                             *                                                                             *
******************************************************************************/

/******************************************************************************
 * NicheStack TCP/IP stack initialization and Operating System Start in main()
 * for Simple Socket Server (SSS) example. 
 * 
 * This example demonstrates the use of MicroC/OS-II running on NIOS II.       
 * In addition it is to serve as a good starting point for designs using       
 * MicroC/OS-II and Altera NicheStack TCP/IP Stack - NIOS II Edition.                                                                                           
 *      
 * Please refer to the Altera NicheStack Tutorial documentation for details on 
 * this software example, as well as details on how to configure the NicheStack
 * TCP/IP networking stack and MicroC/OS-II Real-Time Operating System.  
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
#include "accel.h"

/* Definition of task stack for the initial task which will initialize the NicheStack
 * TCP/IP Stack and then initialize the rest of the Simple Socket Server example tasks. 
 */
OS_STK    SSSInitialTaskStk[TASK_STACKSIZE];

/* Declarations for creating a task with TK_NEWTASK.  
 * All tasks which use NicheStack (those that use sockets) must be created this way.
 * TK_OBJECT macro creates the static task object used by NicheStack during operation.
 * TK_ENTRY macro corresponds to the entry point, or defined function name, of the task.
 * inet_taskinfo is the structure used by TK_NEWTASK to create the task.
 */
TK_OBJECT(to_ssstask);
TK_ENTRY(SSSSimpleSocketServerTask);

struct inet_taskinfo ssstask = {
      &to_ssstask,
      "simple socket server",
      SSSSimpleSocketServerTask,
      SSS_SIMPLE_SOCKET_SERVER_TASK_PRIORITY,
      APP_STACK_SIZE,
};

/* SSSInitialTask will initialize the NicheStack
 * TCP/IP Stack and then initialize the rest of the Simple Socket Server example 
 * RTOS structures and tasks. 
 */
#include "i2c_opencores.h"
int i2c_probe(unsigned core,unsigned address)
{
	int x;


	// do the v I2C_OPENCORES_0_BASE
	x=I2C_start(core,address,0);
	I2C_write(core,0,1);  // write to register 3 command; no stop
	return x==I2C_NOACK?0:1;
}
void SSSInitialTask(void *task_data)
{
  INT8U error_code;
  int ch;
  int x;
  
  /*
   * Initialize Altera NicheStack TCP/IP Stack - Nios II Edition specific code.
   * NicheStack is initialized from a task, so that RTOS will have started, and 
   * I/O drivers are available.  Two tasks are created:
   *    "Inet main"  task with priority 2
   *    "clock tick" task with priority 3
   */   


  I2C_init(I2C_OPENCORES_0_BASE,112000000,200000);
  I2C_init(I2C_OPENCORES_1_BASE,112000000,200000);
 // I2C_init(I2C_OPENCORES_1_BASE,100000000,400000);
  I2C_init(I2C_OPENCORES_2_BASE,112000000,200000);
  I2C_init(I2C_OPENCORES_3_BASE,112000000,200000);


//  for (x=0;x<0xf0;x++)
//      if (i2c_probe(I2C_OPENCORES_1_BASE,x))
 //     printf("%x\n", x);

  {
  //    int tof_init();
   //   tof_init();
  }
  LSM6DS3Init();

	if (i2c_probe(I2C_OPENCORES_0_BASE,0x32))
		printf("Got Ack from Cam Controller------\n");
	else
		printf("Cam Controller Does not respond\n");
	if (i2c_probe(I2C_OPENCORES_2_BASE,0x22))
			{
		void fixed_set_rgb(int r,int g, int b);
		fixed_set_rgb(10,10,0);
		printf("Got Ack from Fixed Board------\n");
			}
	else
		printf("Fixed Board Does not respond\n");

	OSTimeDly(100);
  alt_iniche_init();
  netmain(); 

  /* Wait for the network stack to be ready before proceeding. 
   * iniche_net_ready indicates that TCP/IP stack is ready, and IP address is obtained.
   */
  while (!iniche_net_ready)
    TK_SLEEP(1);

  /* Now that the stack is running, perform the application initialization steps */
  
  /* Application Specific Task Launching Code Block Begin */

  printf("\nSimple Socket Server starting up\n");

  /* Create the main simple socket server task. */
  TK_NEWTASK(&ssstask);
  
  /*create os data structures */
  SSSCreateOSDataStructs(); 

  /* create the other tasks */
  SSSCreateTasks();
  b_timer_Init();
  /* Application Specific Task Launching Code Block End */
  
  /*This task is deleted because there is no need for it to run again */
  error_code = OSTaskDel(OS_PRIO_SELF);
  alt_uCOSIIErrorHandler(error_code, 0);
  
  while (1); /* Correct Program Flow should never get here */
}

/* Main creates a single task, SSSInitialTask, and starts task scheduler.
 */

#include "ffs/ff.h"

FATFS FatFs;		/* FatFs work area needed for each volume */

void fs_init()
    {
    disk_initialize (0 );
    FIL Fil;			/* File object needed for each open file */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    UINT bw;
    f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
     return;
       /* Create FAT volume */
       //f_mkfs("", FM_ANY, 0, work, sizeof work);

	if (f_open(&Fil, "newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

	    		f_read(&Fil, work, 16, &bw);	/* Write data to the file */
	    		work[bw]=0;
	    		printf("file read %s all good\n",work);
	    		f_close(&Fil);								/* Close the file */
	    		  while(1);
	    		}

    	if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

    		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
    		printf("all good\n");
    		f_close(&Fil);								/* Close the file */

    		}
printf("all done\n");
while(1);
    }



int main (int argc, char* argv[], char* envp[])
																	{
  
  INT8U error_code;

  fs_init();
  /* Clear the RTOS timer */
  OSTimeSet(0);


  /* SSSInitialTask will initialize the NicheStack
   * TCP/IP Stack and then initialize the rest of the Simple Socket Server example 
   * RTOS structures and tasks. 
   */  
  error_code = OSTaskCreateExt(SSSInitialTask,
                             NULL,
                             (void *)&SSSInitialTaskStk[TASK_STACKSIZE],
                             SSS_INITIAL_TASK_PRIORITY,
                             SSS_INITIAL_TASK_PRIORITY,
                             SSSInitialTaskStk,
                             TASK_STACKSIZE,
                             NULL,
                             0);
  alt_uCOSIIErrorHandler(error_code, 0);

  /*
   * As with all MicroC/OS-II designs, once the initial thread(s) and 
   * associated RTOS resources are declared, we start the RTOS. That's it!
   */
  OSStart();

  
  while(1); /* Correct Program Flow never gets here. */

  return -1;
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
