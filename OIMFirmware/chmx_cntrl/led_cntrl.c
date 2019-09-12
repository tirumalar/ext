/*
 * led_cntrl.c
 *
 *  Created on: Aug 19, 2016
 *      Author: PTG
 */




#include <stdio.h>
#include <alt_types.h>
#include <io.h>
#include <system.h>
#include <string.h>
#include <stdlib.h>

#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "i2c_opencores.h"
#include "led_control.h"




unsigned int psoc_read( int reg)
{
	int address = PSOC_ID;
	int base = I2C_OPENCORES_0_BASE;
	int val;
	int val2;
	I2C_start(base,address,0);
	I2C_write(base,reg&0xff,1);  // write to register 3 command; no stop
	I2C_start(base,address,1);
	val = I2C_read(base,1);
	return (val);
}

unsigned int psoc_write( int reg, int val)
{
	int address = PSOC_ID;
	int base = I2C_OPENCORES_0_BASE;

	I2C_start(base,address,0);
	I2C_write(base,reg&0xff,0);  // write to register 3 command; no stop
	I2C_write(base,val,1);  // upper 16 bits
	return 1;
}
int psoc_read_version(void)
    {
    return psoc_read(PSOC_VERSON);
    }
