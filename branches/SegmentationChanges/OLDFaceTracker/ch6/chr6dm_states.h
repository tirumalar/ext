/* ------------------------------------------------------------------------------
  File: chr6dm_states.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Function declarations for CHR-6dm state estimation.
------------------------------------------------------------------------------ */ 

#include "stm32f10x.h"

#ifndef _CHR6DM_STATES__
#define _CHR6DM_STATES__

#include "matrix3x3.h"

// Structure for holding raw sensor data
typedef struct __RawSensorData {
	 int16_t gyro_x;
	 int16_t gyro_y;
	 int16_t gyro_z;
	 
	 int16_t accel_x;
	 int16_t accel_y;
	 int16_t accel_z;
	 
	 // Flag specifies whether there is new accel data in the sensor data structure
	 int16_t new_accel_data;
	 
	 int16_t mag_x;
	 int16_t mag_y;
	 int16_t mag_z;
	 
	 // Flag specifies whether there is new magnetometer data in the sensor data structure
	 int16_t new_mag_data;
} RawSensorData;

// Structure for holding sensor data converted to actual values
typedef struct __ConvertedSensorData {
	 float gyro_x;
	 float gyro_y;
	 float gyro_z;
	 
	 float accel_x;
	 float accel_y;
	 float accel_z;
	 
	 // Flag specifies whether there is new accel data in the sensor data structure
	 int16_t new_accel_data;
	 
	 float mag_x;
	 float mag_y;
	 float mag_z;
	 
	 // Flag specifies whether there is new magnetometer data in the sensor data structure
	 int16_t new_mag_data;

} ConvertedSensorData;


// Structure for storing AHRS states
typedef struct __AHRS_states {
	 
	 // Orientation states
	 union {
		  float heading;
		  float yaw;
		  float psi;
	 };
	 union {
		  float pitch;
		  float theta;
	 };
	 union {
		  float roll;
		  float phi;
	 };
	 
	 // Orientation rate states
	 union {
		  float heading_rate;
		  float yaw_rate;
		  float psi_dot;
	 };
	 
	 union {
		  float pitch_rate;
		  float theta_dot;
	 };
	 
	 union {
		  float roll_rate;
		  float phi_dot;
	 };
	 
	 // Process noise matrix
	 fmat3x3 Q;
	 
	 // Measurement noise matrix
	 fmat3x3 Racc;
	 fmat3x3 Rmag;
	 
	 // EKF covariance
	 fmat3x3 P;
	 
} AHRS_states;

extern RawSensorData gSensorData;
extern AHRS_states gEstimatedStates;

// Function declarations
void EKF_Init( AHRS_states* estimated_states );
void EKF_InitFromSensors( AHRS_states* estimated_states, RawSensorData* sensor_data );
void EKF_EstimateStates( AHRS_states* estimated_states, RawSensorData* sensor_data );
void EKF_Predict( AHRS_states* estimated_states, RawSensorData* sensor_data );
void EKF_Update( AHRS_states* estimated_states, RawSensorData* sensor_data );

void unroll_states( AHRS_states* states );

#endif