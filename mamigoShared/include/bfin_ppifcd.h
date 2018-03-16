/*
 * File:         drivers/char/bfin_ppifcd.h
 * Based on:
 * Author:       Michael Hennerich
 *
 * Created:      12.07.2005 17:09
 * Description:  Simple PPI Frame Capture driver for ADSP-BF5xx
 *
 * Rev:          $Id: bfin_ppifcd.h 4054 2007-12-18 07:39:37Z hennerich $
 *
 * Modified:
 *               Copyright 2005-2006 Analog Devices Inc.
 *
 * Bugs:         Enter bugs at http://blackfin.uclinux.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _ADSP_PPIADC_H_
#define _ADSP_PPIADC_H_

#define PPI_READ              0
#define PPI_WRITE             1

#define CMD_PPI_SET_PIXELS_PER_LINE   0
#define CMD_PPI_SET_LINES_PER_FRAME   1
#define CMD_PPI_SET_PPICONTROL_REG    2
#define CMD_PPI_SET_PPIDEALY_REG      3
#define CMD_PPI_SET_PPICOUNT_REG      4
#define CMD_SET_TRIGGER_GPIO          5
#define CMD_PPI_GET_SYSTEMCLOCK       6
#define CMD_PPI_GET_ALLCONFIG	      7 	/* For debug */
#define CMD_FREE_BUFFER               8 	/* explicitely free the buffer */
#define CMD_CRP_RECT_PNTR	     	  9 	/* For specifying the crop rectangle */
#define CMD_SETUP_VIDEO_STREAM	 	  10	/* setup the frame capture in a flow manner*/
#define CMD_TEARDOWN_VIDEO_STREAM	  11	/* teardown(stop) the frame capture */
#define CMD_READFRAME_VIDEO_STREAM	  12    /* read a frame */
#define CMD_READSTILL_FRAME			  13


#define TRIGGER_PF0 0
#define TRIGGER_PF1 1
#define TRIGGER_PF2 2
#define TRIGGER_PF3 3
#define TRIGGER_PF4 4
#define TRIGGER_PF5 5
#define TRIGGER_PF6 6
#define TRIGGER_PF7 7
#define TRIGGER_PF8 8
#define TRIGGER_PF9 9
#define TRIGGER_PF10 10
#define TRIGGER_PF11 11
#define TRIGGER_PF12 12
#define TRIGGER_PF13 13
#define TRIGGER_PF14 14
#define TRIGGER_PF15 15

#define NO_TRIGGER  (-1)


/* Some Sensor Sepcific Defaults */

#undef MT9M001
#define  MT9V022

#ifdef MT9M001
#define POL_C 			0x4000
#define POL_S 			0x0000
#define PIXEL_PER_LINE	1280
#define LINES_PER_FRAME	1024
#define CFG_GP_Input_3Syncs 	0x0020
#define GP_Input_Mode			0x000C
#define PPI_DATA_LEN				DLEN_8
#define PPI_PACKING					PACK_EN
#define DMA_FLOW_MODE			0x0000 //STOPMODE
#define DMA_WDSIZE_16			WDSIZE_16
#endif

#ifdef MT9V022
#define POL_C 				0x0000
#define POL_S 				0x0000
#define PIXEL_PER_LINE			2593
#define LINES_PER_FRAME			1944
#define CFG_GP_Input_3Syncs	 	0x0020
#define GP_Input_Mode			0x000C
#define PPI_DATA_LEN			DLEN_8
#define PPI_PACKING				PACK_EN
/*#define DMA_FLOW_MODE			0x0000 //STOPMODE*/
//  #define DMA_FLOW_MODE		0x7000  // Large
// #define DMA_FLOW_MODE		0x0001 //Auto Buffer
#define DMA_FLOW_MODE_STOP		DMA_FLOW_STOP //STOPMODE


#define DMA_FLOW_MODE 			DMAFLOW_LARGE

#define DMA_WDSIZE_16			WDSIZE_16
// #define NDSIZE				9
#endif

#define NUM_IMAGES		1

#include "mamigo/dmaparams.h"

struct dmadesc {
	unsigned long next_desc_addr;
	unsigned long start_addr;
} ;

struct frameRead {
	//request
	unsigned short isBlocking;	//1: block till the frame appears, 0: nonblocking return EAGAIN if the frame is not yet come
								//TODO use the blocking mode of file open in future
	unsigned long minIdx;		// request is for this index or later
	unsigned short isLatest;	//1: return the latest frame which meets the request 0: exact match or fail
	//response
	unsigned long start_addr;	// the start address is set here
	unsigned long actualIdx;	// the index of the frame actually return
} ;

#endif /* _ADSP_PPIADC_H_ */

