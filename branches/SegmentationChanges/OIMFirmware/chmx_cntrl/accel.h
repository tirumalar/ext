/*
 * accel.h
 *
 *  Created on: Mar 16, 2018
 *      Author: PTG
 */

#ifndef ACCEL_H_
#define ACCEL_H_



#define IDX_GX 0
#define IDX_GY 1
#define IDX_GZ 2
#define IDX_AX 3
#define IDX_AY 4
#define IDX_AZ 5
#define IDX_TEMP 6
#define PI 3.14159
void LSM6DS3Init();
void LSM6DS3Read(float *data);
#endif /* ACCEL_H_ */
