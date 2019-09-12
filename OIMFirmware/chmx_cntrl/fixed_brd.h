/*
 * fixed_brd.h
 *
 *  Created on: Apr 5, 2017
 *      Author: PTG
 */

#ifndef FIXED_BRD_H_
#define FIXED_BRD_H_


void fixed_motor_home(void);

void fixed_motor_rel(int steps);
void fixed_motor_abs(int steps);
void fixed_set_rgb(int r,int g, int b);
void fixed_set_rgbm(int r,int g, int b, int m);
void fixed_aud_set_low(int val);
int fixed_version(void);
void fx_mot_set(int a, int b, int c);
int  fixed_read_plate();

#define FIXED_ID 0x22
#define FIXED_I2C_BASE I2C_OPENCORES_2_BASE
#define FIXED_REG_R  3
#define FIXED_REG_G  4
#define FIXED_REG_B  5
#define FIXED_REG_TOP 1
#define FIXED_REG_ACTION 0
#define FIXED_MOT_LO  6
#define FIXED_MOT_HI  7
#define FIXED_MOT_STAT  8
#define FIXED_AUD_SETTINGS 9
#define FIXED_VERSION 10
#define FIXED_PLATE_DETECT 11
#define FIXED_MOT_MIN_SPEED_A 12
#define FIXED_MOT_MIN_SPEED_D  13
#define FIXED_MOT_ACCEL  14




#define FIXED_ACTION_LED 1
#define FIXED_ACTION_MOT_HOME 2
#define FIXED_ACTION_MOT_REL  4
#define FIXED_ACTION_MOT_ABS  3
#define FIXED_ACTION_AUD_SET  5
#define FIXED_ACTION_REBOOT  6


#endif /* FIXED_BRD_H_ */
