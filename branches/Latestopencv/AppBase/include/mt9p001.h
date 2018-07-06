/*
 * Memory constants for the mt9p001 driver.
 */

#ifndef MT9P001_H
#define MT9P001_H


#define DRIVER_NAME 	"mt9p001"
/* #define DRIVER_NAME	"ov3640" */


/* 
 ====================================================================
 Specify the correct total physical memory for your system 
 and the amount of memory to reserve for the ISP.
 ==================================================================== 
*/
#define PHY_MEM_MB 			256
#define RESERVED_ISP_MEM_MB		32

/* 
 ====================================================================
 The driver assumes that you then pass the following command line 
 arg to the kernel on boot.

	mem=<PHY_MEM_MB - RESERVED_ISP_MEM_MB>M@<PHY_MEM_START>

 where PHY_MEM_START is fixed at 0x80000000

 So for example with a 256 MB system and 32 MB reserved for the ISP,
 the command line arg would be

	mem=224M@0x80000000

 The values below are calculated from the above values and should 
 not be modified.
 ====================================================================
*/


#define PHY_MEM_SIZE (PHY_MEM_MB * 1024 * 1024)
#define PHY_MEM_START 0x80000000
#define PHY_MEM_END (PHY_MEM_START + (PHY_MEM_SIZE - 1))

/* reserved ISP memory is at the end of physical memory */
#define ISP_MEM_SIZE (RESERVED_ISP_MEM_MB * 1024 * 1024)
#define ISP_MEM_START (PHY_MEM_END - (ISP_MEM_SIZE - 1))



#endif /* ifndef MT9P001_H */



