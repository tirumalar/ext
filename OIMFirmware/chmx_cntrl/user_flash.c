/*
 * user_flash.c
 *
 *  Created on: Jun 20, 2016
 *      Author: fso
 */

#include <stdio.h>
#include <system.h>
#include <alt_types.h>
#include <sys/alt_sys_init.h>
#include <sys/alt_flash.h>


#include "user_flash.h"



void user_flash_read(int *buffer, int size)
{
	alt_flash_fd * fd;	// File descriptor to access onchip flash device

	// Initialize all of the HAL drivers used in this design (JTAG UART, onchip flash, etc.)

	// Open up the flash device so we can write to it
	//fd = alt_flash_open_dev(ONCHIP_FLASH_0_NAME);
	//alt_read_flash(fd, USER_FLASH_AREA, buffer, size);
	//memcpy(buffer,ONCHIP_FLASH_0_BASE+USER_FLASH_AREA,size);
	//AskForRegion(fd);
}

void user_flash_write(int *buffer, int size)
{
	alt_flash_fd * fd;	// File descriptor to access onchip flash device

	// Initialize all of the HAL drivers used in this design (JTAG UART, onchip flash, etc.)

	// Open up the flash device so we can write to it
	//fd = alt_flash_open_dev(ONCHIP_FLASH_0_NAME);
	//alt_erase_flash_block(fd, USER_FLASH_AREA, USER_FLASH_SIZE);
	//alt_write_flash(fd, USER_FLASH_AREA, buffer, size);
	//memcpy(ONCHIP_FLASH_0_BASE+USER_FLASH_AREA,buffer,size);
	//AskForRegion(fd);
}


void user_prog_read(char *buffer, int size)
{
	alt_flash_fd * fd;	// File descriptor to access onchip flash device

	// Initialize all of the HAL drivers used in this design (JTAG UART, onchip flash, etc.)

	// Open up the flash device so we can write to it
	//fd = alt_flash_open_dev(ONCHIP_FLASH_0_NAME);
	//alt_read_flash(fd, USER_PROG_AREA, buffer, size);
	//AskForRegion(fd);
	//memcpy(buffer,ONCHIP_FLASH_0_BASE+USER_PROG_AREA,size);
}

void user_prog_write(char *buffer, int size)
{
	alt_flash_fd * fd;	// File descriptor to access onchip flash device

	// Initialize all of the HAL drivers used in this design (JTAG UART, onchip flash, etc.)

	// Open up the flash device so we can write to it
	//fd = alt_flash_open_dev(ONCHIP_FLASH_0_NAME);
	//alt_erase_flash_block(fd, USER_PROG_AREA, USER_PROG_SIZE);
	//alt_write_flash(fd, USER_PROG_AREA, buffer, size);
	//memcpy(ONCHIP_FLASH_0_BASE+USER_PROG_AREA,buffer,size);
	//AskForRegion(fd);
}
