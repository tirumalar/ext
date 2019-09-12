/*
 * fixed_brd.c
 *
 *  Created on: Mar 7, 2017
 *      Author: PTG
 */


#include <stdlib.h>

#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "i2c_opencores.h"
/* MicroC/OS-II definitions */
#include "includes.h"
#include "fixed_brd.h"


unsigned int fixed_read( int reg)
{
	int address = FIXED_ID;
	int base = FIXED_I2C_BASE;
	int val;
	int val2;

	//return 0;
	I2C_start(base,address,0);
	I2C_write(base,reg&0xff,1);  // write to register 3 command; no stop
	I2C_start(base,address,1);
	val = I2C_read(base,1);
	return (val);
}

unsigned int fixed_write( int reg, int val)
{
	int address = FIXED_ID;
	int base = FIXED_I2C_BASE;

	//return 0;
	I2C_start(base,address,0);
	I2C_write(base,reg&0xff,0);  // write to register 3 command; no stop
	I2C_write(base,val,1);  // upper 16 bits
	return 1;
}
void fixed_set_rgb(int r,int g, int b)
{
    int to=200;
  //  printf("rgb %d %d %d\n",r,g,b);
	fixed_write( FIXED_REG_R,r);
	fixed_write( FIXED_REG_G,g);
	fixed_write( FIXED_REG_B,b);
	fixed_write( FIXED_REG_TOP,0x1f);
	fixed_write( FIXED_REG_ACTION,FIXED_ACTION_LED);
	while (fixed_read(FIXED_REG_ACTION)!=0)
	    {
	    OSTimeDly(1);
	    if (to--==0)
		break;
	    }

}
void fixed_set_rgbm(int r,int g, int b, int m)
{
        int to=1000000;
	fixed_write( FIXED_REG_R,r);
	fixed_write( FIXED_REG_G,g);
	fixed_write( FIXED_REG_B,b);
	fixed_write( FIXED_REG_TOP,m);
	fixed_write( FIXED_REG_ACTION,FIXED_ACTION_LED);

	while ((fixed_read(FIXED_REG_ACTION)!=0) && (to))
		to--;
	if (to==0)
	    printf("Time out on rgb\n");

}

void fixed_motor_home(void)
{
    int to = 3000;
	fixed_write( FIXED_REG_ACTION,FIXED_ACTION_MOT_HOME);
	while (fixed_read(FIXED_REG_ACTION)!=0)
	    {
		    OSTimeDly(1);
		    if (to--==0)
			break;
		    }

}
void fixed_motor_rel(int steps)
{
    int to = 2000;
	fixed_write( FIXED_MOT_LO,steps&0xff);
	fixed_write( FIXED_MOT_HI,(steps>>8)&0xff);
	fixed_write( FIXED_REG_ACTION,FIXED_ACTION_MOT_REL);
	while (fixed_read(FIXED_MOT_STAT)!=0)
	    {
	    OSTimeDly(1);
	    if (to--==0)
		break;
	    }

}
void fixed_motor_abs(int steps)
{
	fixed_write( FIXED_MOT_LO,steps&0xff);
	fixed_write( FIXED_MOT_HI,(steps>>8)&0xff);
	fixed_write( FIXED_REG_ACTION,FIXED_ACTION_MOT_ABS);
	while (fixed_read(FIXED_MOT_STAT)!=0);


}

void fixed_aud_set_low(int val)
    {
    fixed_write (FIXED_AUD_SETTINGS,val);
    fixed_write(  FIXED_REG_ACTION,FIXED_ACTION_AUD_SET);
    }
int fixed_version(void)
    {
    return fixed_read(FIXED_VERSION);

    }
int fixed_read_plate()
    {
    return fixed_read(FIXED_PLATE_DETECT);
    }

void fx_mot_set(int a, int b, int c)
    {
    fixed_write (FIXED_MOT_MIN_SPEED_A,a);
    fixed_write (FIXED_MOT_MIN_SPEED_D,b);
    fixed_write (FIXED_MOT_ACCEL,c);
    }
