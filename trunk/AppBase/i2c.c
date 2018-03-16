/*
 *
 *    Rev:          $Id: i2c.c 987 2005-07-18 10:13:16Z hennerich $
 *    Revision:     $Revision: 987 $
 *    Source:       $Source$
 *    Created:      06.07.2005 18:16
 *    Author:       Michael Hennerich
 *    mail:         hennerich@blackfin.uclinux.org
 *    Description:  Simple I2C Routines
 *
 *   Copyright (C) 2005 Michael Hennerich
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 ****************************************************************************
 * MODIFICATION HISTORY:
 ***************************************************************************/


#include "i2c-dev.h"

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "i2c.h"



//#define main
#undef main


#if main
#define I2C_DEVICE "/dev/i2c-0"
#define I2C_SLAVE_ADDR 0x38 /* Randomly picked */
#define I2C_DEVID (0xB8>>1)
int main()
{

	i2c_scan_bus(I2C_DEVICE);
	i2c_write_register(I2C_DEVICE,I2C_DEVID,9,0x0248);
	i2c_dump_register(I2C_DEVICE,I2C_DEVID,0,255);
	printf("Read Register 9 = 0x%X \n",
	i2c_read_register(I2C_DEVICE, I2C_DEVID,9) );

  exit( 0 );
}
#endif

int i2cWrite2ByteToByteAddress(char * device, unsigned char client, unsigned char reg, unsigned short value){
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}

	msg.len   = 3;
	msg.flags = 0;
	msg_data[0] = reg;
	msg_data[2] = (0xFF & value);
	msg_data[1] = (value >> 8);
	msg.addr = client;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	}
	printf("Add %02x Value %04x\n",reg,value);
	close( fd );
	return 0;
}

int i2cRead2ByteFromByteAddress(char * device, unsigned char client, unsigned char reg){
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}

	msg_data[0]= reg;
	msg.addr = client;
	msg.len   = 1;
	msg.flags = 0;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	};

	msg.len   = 2;
	msg_data[0]=0;
	msg_data[1]=0;
	msg.flags = I2C_M_RD ;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not read back\n");
		close( fd );
		return -1;
	}

	close( fd );
	return (((unsigned char)msg_data[0])<<8 | ((unsigned char)msg_data[1]) );
}

int i2cDumpRegShort(char * device, unsigned char client, unsigned short start, unsigned short end)
{
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;
	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}
	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}
	for(i = start; i < end; i++) {
		msg_data[0]= i;
		msg.addr = client;
		msg.len   = 1;
		msg.flags = 0;

		if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
			fprintf(stderr, "Error: could not write \n");
		};
		msg.len   = 2;
		msg_data[0]=0;
		msg_data[1]=0;
		msg.flags = I2C_M_RD ;
		if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
			fprintf(stderr, "Error: could not read back\n");
		} else {
			fprintf(stderr, "Register %02x : %02x%02x \n",i, (unsigned char)msg_data[0],(unsigned char)msg_data[1]);
		}
	}
	close( fd );
	return;
}

void i2cScanBus(char * device){
	int    addr = 0;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}

	msg.len   = 1;
	msg.flags = 0;
	msg_data[0]=0;
	msg_data[1]=0;

	for ( i = 0; i < 128; i++){
		msg.addr = i;
		if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		}else{
			fprintf(stderr, "FOUND I2C device at 0x%X (8-bit Adrress 0x%X) \n",msg.addr,msg.addr<<1);
		}
	}
	close( fd );
	return;
}

int i2cReadByte(char * device, unsigned char client){
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}
	msg_data[0]= 0;
	msg.addr = client;
	msg.len   = 0;
	msg.flags = 0;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	};

	msg.len   = 1;
	msg_data[0]=0;
	msg.flags = I2C_M_RD ;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not read back\n");
		close( fd );
		return -1;
	}
	close( fd );
	return (msg_data[0]);
}

int i2cWriteByte(char * device, unsigned char client, unsigned char value)
{
	int    addr = client;
	unsigned char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}

	msg.len   = 1;
	msg.flags = 0;
	msg_data[0] = value;
	msg.addr = client;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	}
	printf("i2cWriteByte  Value %02x\n",value);
	close( fd );
	return 0;
}

int i2cWriteByteOnByteAddres(char * device, unsigned char client, unsigned char reg, unsigned char value){
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;

	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}

	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}

	msg.len   = 2;
	msg.flags = 0;
	msg_data[0] = reg;
	msg_data[1] = value;
	msg.addr = client;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	}
	printf("i2cWriteByteOnByteAddres  %02x Value %02x\n",reg,value);
	close( fd );
	return 0;
}

int i2cReadByteFromByteAddress(char * device, unsigned char client, unsigned char reg){
	int    addr = client;
	char   msg_data[32];
	struct i2c_msg msg = { addr, 0, 0, msg_data };
	struct i2c_rdwr_ioctl_data rdwr = { &msg, 1 };
	int fd,i;
	if ( (fd = open( device, O_RDWR ) ) < 0 ) {
		fprintf(stderr, "Error: could not open %s\n", device);
		exit( 1 );
	}
	if ( ioctl( fd, I2C_SLAVE, addr ) < 0 ) {
		fprintf(stderr, "Error: could not bind address %x \n", addr );
	}
	msg_data[0]= reg;
	msg.addr = client;
	msg.len   = 1;
	msg.flags = 0;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not write \n");
	};

	msg.len   = 1;
	msg_data[0]=0;
	msg.flags = I2C_M_RD ;

	if ( ioctl( fd, I2C_RDWR, &rdwr ) < 0 ) {
		fprintf(stderr, "Error: could not read back\n");
		close( fd );
		return -1;
	}
	close( fd );
	return ((unsigned char)msg_data[0]);
}
