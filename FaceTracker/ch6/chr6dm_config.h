/* ------------------------------------------------------------------------------
  File: chr6d_config.f
  Author: CH Robotics
  Version: 1.0
  
  Description: Preprocessor definitions and func declarations for IMU configuration
------------------------------------------------------------------------------ */ 
#ifndef __CHR6D_CONFIG_H
#define __CHR6D_CONFIG_H

#include "matrix3x3.h"
#include "chr6dm_usart.h"


typedef struct __CHR6d_config 
{
	 // Accel biases
	 int16_t x_accel_bias;
	 int16_t y_accel_bias;
	 int16_t z_accel_bias;
	 
	 // Rate gyro scale factors
	 float x_gyro_scale;
	 float y_gyro_scale;
	 float z_gyro_scale;
	 
	 // Rate gyro biases
	 int16_t x_gyro_bias;
	 int16_t y_gyro_bias;
	 int16_t z_gyro_bias;
	 
	 // Magnetometer biases
	 int16_t x_mag_bias;
	 int16_t y_mag_bias;
	 int16_t z_mag_bias;
	 
	 // EKF configuration register
	 uint8_t EKF_config;
	 
	 float process_covariance;
	 float accel_covariance;
	 float mag_covariance;
	 
	 fmat3x3 gyro_alignment;
	 fmat3x3 accel_alignment;
	 fmat3x3 mag_cal;
	 
	 int16_t mag_ref_vector[3];
	 int16_t accel_ref_vector[3];
	 	 
	 // Channel enable/disable flags
	 uint8_t x_accel_enabled;
	 uint8_t y_accel_enabled;
	 uint8_t z_accel_enabled;
	 uint8_t x_gyro_enabled;
	 uint8_t y_gyro_enabled;
	 uint8_t z_gyro_enabled;
	 uint8_t x_mag_enabled;
	 uint8_t y_mag_enabled;
	 uint8_t z_mag_enabled;
	 uint8_t phi_enabled;
	 uint8_t theta_enabled;
	 uint8_t psi_enabled;
	 uint8_t phi_dot_enabled;
	 uint8_t theta_dot_enabled;
	 uint8_t psi_dot_enabled;
	 
	 uint8_t gyro_startup_calibration;
	 
	 uint8_t broadcast_enabled;
	 uint8_t broadcast_rate;

} CHR6d_config;

extern CHR6d_config gConfig;

void GetConfiguration( void );
int32_t WriteConfigurationToFlash( void );

void UpdateBroadcastRate( uint8_t new_rate );
void EnableBroadcastMode( uint8_t new_rate );
void DisableBroadcastMode( void );
void StartGyroCalibration( void );
void StopGyroCalibration( void );

void StartSelfTest( void );
void StopSelfTest( uint16_t result );

float getGyroScale( int channel );
float getProcessCovariance( void );
float getAccelCovariance( void );
float getMagCovariance( void );
void getGyroAlignment( fmat3x3* matrix );
void getAccelAlignment( fmat3x3* matrix );
void getMagCal( fmat3x3* matrix );
void getMagRef( int16_t* vector );
void getAccelRef( int16_t* vector );

void ResetToFactory( void );

#define 		X_GYRO		0
#define		Y_GYRO		1
#define		Z_GYRO		2

#define		MAG_UPDATE_ENABLED		0x01
#define		ACCEL_UPDATE_ENABLED		0x02
#define		MAG_RESTRICTED				0x04

// Addresses of configuration registers.  On the CHR6d, the addresses below
// place the configuration registers into flash memory at page 60.  The addresses
// are fairly arbitrary, and can be anything so long as they do not interfere with
// program memory.  If custom code brings the program size to above 60 K, these addresses
// should be increased.
#define		OFFSET_CONF1_ADDR			(uint32_t)0x0800F000
#define		OFFSET_CONF2_ADDR			(uint32_t)0x0800F004
#define		OFFSET_CONF3_ADDR			(uint32_t)0x0800F008
#define		OFFSET_CONF4_ADDR			(uint32_t)0x0800F00C
#define		OFFSET_CONF5_ADDR			(uint32_t)0x0800F010
#define		MISC_CONF_ADDR				(uint32_t)0x0800F014
#define		GYRO_X_SCALE_ADDR			(uint32_t)0x0800F018
#define		GYRO_Y_SCALE_ADDR			(uint32_t)0x0800F01C
#define		GYRO_Z_SCALE_ADDR			(uint32_t)0x0800F020

#define		ACCEL_ALIGN_START_ADDR	(uint32_t)0x0800F024
#define		GYRO_ALIGN_START_ADDR	(uint32_t)0x0800F048
#define		MAG_CAL_START_ADDR		(uint32_t)0x0800F06C

#define		PROCESS_COV_ADDR			(uint32_t)0x0800F090
#define		MAG_COV_ADDR				(uint32_t)0x0800F094
#define		ACCEL_COV_ADDR				(uint32_t)0x0800F098
#define		MAG_REF_VECT_ADDR1		(uint32_t)0x0800F09C
#define		MAG_REF_VECT_ADDR2		(uint32_t)0x0800F0A0
#define		ACCEL_REF_VECT_ADDR1		(uint32_t)0x0800F0A4
#define		ACCEL_REF_VECT_ADDR2		(uint32_t)0x0800F0A8

// Preprocessor definitions used to simplify access to parameters stored in FLASH
#define		FGET_ACCEL_X_OFFSET()		(uint16_t)( (*(__IO uint32_t*)(OFFSET_CONF1_ADDR)) & 0x0FFFF )
#define		FGET_ACCEL_Y_OFFSET()		(uint16_t)( ((*(__IO uint32_t*)(OFFSET_CONF1_ADDR)) >> 16) & 0x0FFFF )
#define		FGET_ACCEL_Z_OFFSET()		(uint16_t)( (*(__IO uint32_t*)(OFFSET_CONF2_ADDR)) & 0x0FFFF )
#define		FGET_GYRO_X_OFFSET()	 		(uint16_t)( ((*(__IO uint32_t*)(OFFSET_CONF2_ADDR)) >> 16) & 0x0FFFF )
#define		FGET_GYRO_Y_OFFSET()			(uint16_t)( (*(__IO uint32_t*)(OFFSET_CONF3_ADDR)) & 0x0FFFF )
#define		FGET_GYRO_Z_OFFSET()			(uint16_t)( ((*(__IO uint32_t*)(OFFSET_CONF3_ADDR)) >> 16) & 0x0FFFF )
#define		FGET_MAG_X_OFFSET()			(uint16_t)( (*(__IO uint32_t*)(OFFSET_CONF4_ADDR)) & 0x0FFFF )
#define		FGET_MAG_Y_OFFSET()			(uint16_t)( ((*(__IO uint32_t*)(OFFSET_CONF4_ADDR)) >> 16) & 0x0FFFF )
#define		FGET_MAG_Z_OFFSET()			(uint16_t)( (*(__IO uint32_t*)(OFFSET_CONF5_ADDR)) & 0x0FFFF )

#define		CHANNEL_ENABLED			1
#define		CHANNEL_DISABLED			0

#define		IS_ACCEL_X_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) ) & 0x01 )
#define		IS_ACCEL_Y_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 1) & 0x01 )
#define		IS_ACCEL_Z_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 2) & 0x01 )
#define		IS_GYRO_X_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 3) & 0x01 )
#define		IS_GYRO_Y_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 4) & 0x01 )
#define		IS_GYRO_Z_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 5) & 0x01 )
#define		IS_MAG_X_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 6) & 0x01 )
#define		IS_MAG_Y_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 7) & 0x01 )
#define		IS_MAG_Z_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 8) & 0x01 )
#define		IS_PHI_ENABLED()			(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 9) & 0x01 )
#define		IS_THETA_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 10) & 0x01 )
#define		IS_PSI_ENABLED()			(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 11) & 0x01 )
#define		IS_PHI_DOT_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 12) & 0x01 )
#define		IS_THETA_DOT_ENABLED()	(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 13) & 0x01 )
#define		IS_PSI_DOT_ENABLED()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 14) & 0x01 )

#define		IS_STARTUP_CAL_ENABLED()	(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 15) & 0x01 )
#define		FGET_TRANSMIT_MODE()			(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 16) & 0x01 )
#define		FGET_BROADCAST_RATE()		(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 17) & 0x00FF )

#define		FGET_EKF_CONFIG()			(uint8_t)( ((*(__IO uint32_t*)(MISC_CONF_ADDR)) >> 25) & 0x00F )

#define		MODE_BROADCAST				1
#define		MODE_LISTEN					0



#endif