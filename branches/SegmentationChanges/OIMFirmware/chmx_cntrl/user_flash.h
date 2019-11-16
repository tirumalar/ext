/*
 * user_flash.h
 *
 *  Created on: Jun 20, 2016
 *      Author: fso
 */

#ifndef USER_FLASH_H_
#define USER_FLASH_H_


void user_flash_write(int *buffer, int size);
void user_flash_read(int *buffer, int size);
void user_prog_write(char *buffer, int size);
void user_prog_read(char *buffer, int size);
#define USER_FLASH_AREA 0x8000
#define USER_FLASH_SIZE 0x1000

#define USER_PROG_AREA 0x9000
#define USER_PROG_SIZE 0x1000


#endif /* USER_FLASH_H_ */
