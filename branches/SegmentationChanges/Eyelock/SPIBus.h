/*
 * SPIBus.h
 *
 *  Created on: Jan 3, 2012
 *      Author: dhirvonen
 */

#ifndef SPIBUS_H_
#define SPIBUS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <linux/types.h>
#include <pthread.h>
//#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

// #include "pflags.h"

#define 	SPIDEV_NAME "/dev/spidev1.1"
#define CMD_READ_BUTTONS  0x10
#define CMD_CHARGE_EN     0x11
#define CMD_CHARGE_DIS    0x12
#define CMD_OFF           0x13
#define CMD_BATT           0x14
#define CMD_ZIG_LINK      0x20
#define CMD_ZIG_SEND      0x21
#define CMD_ZIG_READ      0x22
#define CMD_ZIG_UNLINK      0x23

#define CMD_GPS_PEEK      0x30
#define CMD_GET_BYTE      0x31
#define CMD_GPS_SEND_STRING      0x32

#define CMD_DISP_ENABLE      0x40
#define CMD_DISP_WRITE_DATA  0x41
#define CMD_DISP_REFRESH     0x42

#define CMD_VIBE_ON             0x50
#define CMD_VIBE_OFF             0x51

#define WDT_ON             0x60
#define WDT_TICKLE         0x61

#define FAN_ON             0x70

#define SPI_SPEED 1000000

int spi_open();

void spi_close(int fd);

void spi_send(int fd, char *dat, char len);

void spi_disp_write_dat(int fd,char r, char c, char v); // write a value to the dipslay memory

void spi_disp_test();

void do_chargen(int en);

void do_vibe_en(int en);

void do_wdt_en(void);

void do_vibe_tickle(void);

void do_fan(char f1, char f2);

void do_shutdown();

void gps_add_cs_cr_lf(char *p);

int spi_gps_write(char *dat);

//$PLSC,200,2,300,1000,300000,30000*0E

int spi_gps_test();

char spi_wait_result(int fd);

int spi_zigbee_open();

#define MAX_SEND 255

int spi_zigbee_read(char *msg, int *len);

int spi_zigbee_send(char *msg, int len);

int spi_zigbee_read_batt();

int spi_read_button();

#ifdef __cplusplus
}
#endif

#endif /* SPIBUS_H_ */

