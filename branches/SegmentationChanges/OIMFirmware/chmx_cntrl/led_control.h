/*
 * led_control.h
 *
 *  Created on: Oct 18, 2018
 *      Author: PTG
 */

#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_


#define PSOC_ID 0x32
#define PSOC_LED_REG_ACTION 0
#define PSOC_LED_HV_EN   1
#define PSOC_LED_VOLT    2
#define PSOC_LED_LED_EN     3
#define PSOC_LED_TRIG_SOURCE 4
#define PSOC_LED_I_SET		 5
#define PSOC_LED_MAX_TIMET		 6
#define PSOC_VERSON   10
#define PSOC_ACTION_REBOOT  11
unsigned int psoc_read( int reg);
unsigned int psoc_write( int reg, int val);



#endif /* LED_CONTROL_H_ */
