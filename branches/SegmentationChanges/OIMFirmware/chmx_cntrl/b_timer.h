/*
 * b_timer.h
 *
 *  Created on: May 6, 2016
 *      Author: PTG
 */

#ifndef B_TIMER_H_
#define B_TIMER_H_



void b_timer_Init();
void b_on_time(int timer, int time_out, char *cmd);
void b_on_time_print(char *text);
#endif /* B_TIMER_H_ */
