/* ------------------------------------------------------------------------------
  File: chr6dm_config.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Function definitions for AHRS configuration
------------------------------------------------------------------------------ */ 
#include <math.h>
#include "stm32f10x.h"
#include "chr6dm_config.h"
#include "chr6dm_ADC.h"
#include "chr6dm_usart.h"
#include "matrix3x3.h"

// Global structure storing IMU configuration parameters
CHR6d_config gConfig;

/*******************************************************************************
* Function Name  : GetConfiguration
* Input          : None
* Output         : None
* Return         : None
* Description    : Fills the gConfig structure with IMU configuration data.
						  If configuration data has been written to flash, then load it.
						  If not, then use factory defaults.
*******************************************************************************/
void GetConfiguration( void )
{
	 int i, j;
	 
	 // If flash has not been programmed yet, then use default configuration.  Otherwise, load configuration from flash
	 if( FGET_ACCEL_X_OFFSET() == 0xFFFF )
	 {
		  ResetToFactory();
	 }
	 else
	 {
		  // Gyro, accelerometer, and magnetometer biases
		  gConfig.x_accel_bias = FGET_ACCEL_X_OFFSET();
		  gConfig.y_accel_bias = FGET_ACCEL_Y_OFFSET();
		  gConfig.z_accel_bias = FGET_ACCEL_Z_OFFSET();
		  gConfig.x_gyro_bias = FGET_GYRO_X_OFFSET();
		  gConfig.y_gyro_bias = FGET_GYRO_Y_OFFSET();
		  gConfig.z_gyro_bias = FGET_GYRO_Z_OFFSET();
		  gConfig.x_mag_bias = FGET_MAG_X_OFFSET();
		  gConfig.y_mag_bias = FGET_MAG_Y_OFFSET();
		  gConfig.z_mag_bias = FGET_MAG_Z_OFFSET();
		  
		  gConfig.x_gyro_scale = getGyroScale(X_GYRO);
		  gConfig.y_gyro_scale = getGyroScale(Y_GYRO);
		  gConfig.z_gyro_scale = getGyroScale(Z_GYRO);
		  
		  gConfig.process_covariance = getProcessCovariance();
		  gConfig.accel_covariance = getAccelCovariance();
		  gConfig.mag_covariance = getMagCovariance();
		  
		  getGyroAlignment(&gConfig.gyro_alignment);
		  getAccelAlignment(&gConfig.accel_alignment);
		  getMagCal(&gConfig.mag_cal);
		  getMagRef(gConfig.mag_ref_vector);
		  getAccelRef(gConfig.accel_ref_vector);
		  
		  gConfig.EKF_config = FGET_EKF_CONFIG();
		  
		  gConfig.gyro_startup_calibration = IS_STARTUP_CAL_ENABLED();
		  
		  // Channel enable/disable flags
		  gConfig.x_accel_enabled = IS_ACCEL_X_ENABLED();
		  gConfig.y_accel_enabled = IS_ACCEL_Y_ENABLED();
		  gConfig.z_accel_enabled = IS_ACCEL_Z_ENABLED();
		  gConfig.x_gyro_enabled = IS_GYRO_X_ENABLED();
		  gConfig.y_gyro_enabled = IS_GYRO_Y_ENABLED();
		  gConfig.z_gyro_enabled = IS_GYRO_Z_ENABLED();
		  gConfig.x_mag_enabled = IS_MAG_X_ENABLED();
		  gConfig.y_mag_enabled = IS_MAG_Y_ENABLED();
		  gConfig.z_mag_enabled = IS_MAG_Z_ENABLED();
		  gConfig.phi_enabled = IS_PHI_ENABLED();
		  gConfig.theta_enabled = IS_THETA_ENABLED();
		  gConfig.psi_enabled = IS_PSI_ENABLED();
		  gConfig.phi_dot_enabled = IS_PHI_DOT_ENABLED();
		  gConfig.theta_dot_enabled = IS_THETA_DOT_ENABLED();
		  gConfig.psi_dot_enabled = IS_PSI_DOT_ENABLED();

		  // Broadcast mode or listen mode
		  gConfig.broadcast_enabled = FGET_TRANSMIT_MODE();
		  gConfig.broadcast_rate = FGET_BROADCAST_RATE();
	 }
	 
}

/*******************************************************************************
* Function Name  : getGyroScale
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the gyro scale factor from flash, packages as a float,
and returns
*******************************************************************************/
float getGyroScale( int channel )
{
	 uint32_t address;
	 fConvert iTof;
	 
	 if( channel == X_GYRO )
		  address = GYRO_X_SCALE_ADDR;
	 else if( channel == Y_GYRO )
		  address = GYRO_Y_SCALE_ADDR;
	 else
		  address = GYRO_Z_SCALE_ADDR;
	 
	 iTof.uint32_val = (uint32_t)(*(__IO uint32_t*)(address));
	 
	 return iTof.float_val;	 
}

/*******************************************************************************
* Function Name  : getProcessCovariance
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the process covariance, packages as a float, and
returns
*******************************************************************************/
float getProcessCovariance()
{
	 fConvert iTof;
	 
	 iTof.uint32_val = (uint32_t)(*(__IO uint32_t*)(PROCESS_COV_ADDR));
	 
	 return iTof.float_val;
}

/*******************************************************************************
* Function Name  : getAccelCovariance
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the accel covariance, packages as a float, and
returns
*******************************************************************************/
float getAccelCovariance()
{
	 fConvert iTof;
	 
	 iTof.uint32_val = (uint32_t)(*(__IO uint32_t*)(ACCEL_COV_ADDR));
	 
	 return iTof.float_val;
}

/*******************************************************************************
* Function Name  : getMagCovariance
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the magnetometer covariance, packages as a float, and
returns
*******************************************************************************/
float getMagCovariance()
{
	 fConvert iTof;
	 
	 iTof.uint32_val = (uint32_t)(*(__IO uint32_t*)(MAG_COV_ADDR));
	 
	 return iTof.float_val;
}

/*******************************************************************************
* Function Name  : getGyroAlignment
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the 3x3 gyro alignment correction matrix
*******************************************************************************/
void getGyroAlignment( fmat3x3* matrix )
{
	 int i, j;
	 fConvert fToInt;
	 
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.uint32_val = (uint32_t)(*(__IO uint32_t*)(GYRO_ALIGN_START_ADDR + 4*(3*i+j)));
				
				matrix->data[i][j] = fToInt.float_val;
		  }
	 }	 
}

/*******************************************************************************
* Function Name  : getAccelAlignment
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the 3x3 accel alignment correction matrix
*******************************************************************************/
void getAccelAlignment( fmat3x3* matrix )
{
	 int i, j;
	 fConvert fToInt;
	 
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.uint32_val = (uint32_t)(*(__IO uint32_t*)(ACCEL_ALIGN_START_ADDR + 4*(3*i+j)));
				
				matrix->data[i][j] = fToInt.float_val;
		  }
	 }	
}

/*******************************************************************************
* Function Name  : getMagCal
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the 3x3 mag calibration matrix
*******************************************************************************/
void getMagCal( fmat3x3* matrix )
{
	 int i, j;
	 fConvert fToInt;
	 
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.uint32_val = (uint32_t)(*(__IO uint32_t*)(MAG_CAL_START_ADDR + 4*(3*i+j)));
				
				matrix->data[i][j] = fToInt.float_val;
		  }
	 }	
}

/*******************************************************************************
* Function Name  : getMagRef
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the magnetic field reference vector from flash
*******************************************************************************/
void getMagRef( int16_t* vector )
{
	 vector[0] = (int16_t)( (*(__IO uint32_t*)(MAG_REF_VECT_ADDR1)) & 0x0FFFF );
	 vector[1] = (int16_t)( ((*(__IO uint32_t*)(MAG_REF_VECT_ADDR1)) >> 16) & 0x0FFFF );
	 vector[2] = (int16_t)( (*(__IO uint32_t*)(MAG_REF_VECT_ADDR2)) & 0x0FFFF );
}

/*******************************************************************************
* Function Name  : getAccelRef
* Input          : None
* Output         : None
* Return         : None
* Description    : Retrieves the magnetic field reference vector from flash
*******************************************************************************/
void getAccelRef( int16_t* vector )
{
	 vector[0] = (int16_t)( (*(__IO uint32_t*)(ACCEL_REF_VECT_ADDR1)) & 0x0FFFF );
	 vector[1] = (int16_t)( ((*(__IO uint32_t*)(ACCEL_REF_VECT_ADDR1)) >> 16) & 0x0FFFF );
	 vector[2] = (int16_t)( (*(__IO uint32_t*)(ACCEL_REF_VECT_ADDR2)) & 0x0FFFF );
}

/*******************************************************************************
* Function Name  : WriteConfigurationToFlash
* Input          : None
* Output         : None
* Return         : None
* Description    : Writes the current IMU configuration to flash.
*******************************************************************************/
int32_t WriteConfigurationToFlash( void )
{
	 FLASH_Status FLASHStatus;
	 uint32_t FLASHData;
	 int i, j;
	 fConvert fToInt;
	 
	 FLASH_Unlock();
	 
	 // Clear all pending flags
	 FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	 // Clear FLASH pages in preparation for write operations
	 for (i = 0; i < 46; i++ )
	 {
		  FLASHStatus = FLASH_ErasePage(OFFSET_CONF1_ADDR + 4*i);
		  
		  if( FLASHStatus != FLASH_COMPLETE )
		  {
				return FLASHStatus;
		  }
	 }
	 
	 // Write rate gyro, accel, and mag biases to flash
	 FLASHData = ((uint32_t)gConfig.x_accel_bias) | ((uint32_t)gConfig.y_accel_bias << 16);
	 FLASHStatus = FLASH_ProgramWord(OFFSET_CONF1_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.z_accel_bias) | ((uint32_t)gConfig.x_gyro_bias << 16);
	 FLASHStatus = FLASH_ProgramWord(OFFSET_CONF2_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.y_gyro_bias) | ((uint32_t)gConfig.z_gyro_bias << 16);
	 FLASHStatus = FLASH_ProgramWord(OFFSET_CONF3_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.x_mag_bias) | ((uint32_t)gConfig.y_mag_bias << 16);
	 FLASHStatus = FLASH_ProgramWord(OFFSET_CONF4_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.z_mag_bias);
	 FLASHStatus = FLASH_ProgramWord(OFFSET_CONF5_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Set Misc config register (active channels, EKF config, gyro startup calibration, broadcast mode and freq.)
	 FLASHData = (((uint32_t)gConfig.x_accel_enabled) & 0x01) | (((uint32_t)gConfig.y_accel_enabled << 1) & 0x02) | (((uint32_t)gConfig.z_accel_enabled << 2) & 0x04);
	 FLASHData = FLASHData | (((uint32_t)gConfig.x_gyro_enabled << 3) & 0x08) | (((uint32_t)gConfig.y_gyro_enabled << 4) & 0x010) | (((uint32_t)gConfig.z_gyro_enabled << 5) & 0x020);
	 FLASHData = FLASHData | (((uint32_t)gConfig.x_mag_enabled << 6) & 0x040) | (((uint32_t)gConfig.y_mag_enabled << 7) & 0x080) | (((uint32_t)gConfig.z_mag_enabled << 8) & 0x100);
	 FLASHData = FLASHData | (((uint32_t)gConfig.phi_enabled << 9) & 0x200) | (((uint32_t)gConfig.theta_enabled << 10) & 0x400) | (((uint32_t)gConfig.psi_enabled << 11) & 0x800);
	 FLASHData = FLASHData | (((uint32_t)gConfig.phi_dot_enabled << 12) & 0x1000) | (((uint32_t)gConfig.theta_dot_enabled << 13) & 0x2000) | (((uint32_t)gConfig.psi_dot_enabled << 14) & 0x4000);
	 FLASHData = FLASHData | (((uint32_t)gConfig.gyro_startup_calibration << 15) & 0x8000) | (((uint32_t)gConfig.broadcast_enabled << 16) & 0x10000);
	 FLASHData = FLASHData | ((uint32_t)gConfig.broadcast_rate << 17) | (((uint32_t)gConfig.EKF_config << 25) & (0x0F << 25));
	 
	 FLASHStatus = FLASH_ProgramWord(MISC_CONF_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Gyro X scale factor
	 fToInt.float_val = gConfig.x_gyro_scale;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(GYRO_X_SCALE_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Gyro Y scale factor
	 fToInt.float_val = gConfig.y_gyro_scale;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(GYRO_Y_SCALE_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Gyro Z scale factor
	 fToInt.float_val = gConfig.z_gyro_scale;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(GYRO_Z_SCALE_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Accel alignment matrix
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.float_val = gConfig.accel_alignment.data[i][j];
				FLASHData = fToInt.int32_val;
				
				FLASHStatus = FLASH_ProgramWord(ACCEL_ALIGN_START_ADDR + 4*(3*i+j), FLASHData);
				
				if( FLASHStatus != FLASH_COMPLETE )
				{
					 return FLASHStatus;
				}
		  }
	 }
	 
	 // Gyro alignment matrix
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.float_val = gConfig.gyro_alignment.data[i][j];
				FLASHData = fToInt.int32_val;
				
				FLASHStatus = FLASH_ProgramWord(GYRO_ALIGN_START_ADDR + 4*(3*i+j), FLASHData);
				
				if( FLASHStatus != FLASH_COMPLETE )
				{
					 return FLASHStatus;
				}
		  }
	 }
	 
	 // Magnetometer calibratin matrix
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				fToInt.float_val = gConfig.mag_cal.data[i][j];
				FLASHData = fToInt.int32_val;
				
				FLASHStatus = FLASH_ProgramWord(MAG_CAL_START_ADDR + 4*(3*i+j), FLASHData);
				
				if( FLASHStatus != FLASH_COMPLETE )
				{
					 return FLASHStatus;
				}
		  }
	 }
	 
	 // Process covariance
	 fToInt.float_val = gConfig.process_covariance;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(PROCESS_COV_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Magnetometer covariance
	 fToInt.float_val = gConfig.mag_covariance;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(MAG_COV_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Accelerometer covariance
	 fToInt.float_val = gConfig.accel_covariance;
	 FLASHData = fToInt.int32_val;
	 
	 FLASHStatus = FLASH_ProgramWord(ACCEL_COV_ADDR, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Magnetometer reference vector
	 FLASHData = (((uint32_t)gConfig.mag_ref_vector[0]) & 0x0FFFF) | ((uint32_t)gConfig.mag_ref_vector[1] << 16);
	 FLASHStatus = FLASH_ProgramWord(MAG_REF_VECT_ADDR1, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.mag_ref_vector[2]);
	 FLASHStatus = FLASH_ProgramWord(MAG_REF_VECT_ADDR2, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 // Accel ref vector
	 FLASHData = (((uint32_t)gConfig.accel_ref_vector[0]) & 0x0FFFF) | ((uint32_t)gConfig.accel_ref_vector[1] << 16);
	 FLASHStatus = FLASH_ProgramWord(ACCEL_REF_VECT_ADDR1, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASHData = ((uint32_t)gConfig.accel_ref_vector[2]);
	 FLASHStatus = FLASH_ProgramWord(ACCEL_REF_VECT_ADDR2, FLASHData);
	 
	 if( FLASHStatus != FLASH_COMPLETE )
	 {
		  return FLASHStatus;
	 }
	 
	 FLASH_Lock();
	 
	 return FLASH_COMPLETE;
	 
}

/*******************************************************************************
* Function Name  : UpdateBroadcastRate
* Input          : None
* Output         : None
* Return         : None
* Description    : Sets the Timer2 period to adjust IMU broadcast frequency.
						  In Broadcast Mode, data is transmitted on every timer interrupt
*******************************************************************************/
void UpdateBroadcastRate( uint8_t new_rate )
{
	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	 uint16_t new_period;
	 	 
	 // Update gConfig parameter
	 gConfig.broadcast_rate = new_rate;
	 
	 // Calculate new period.  The desired broadcast frequency is given by
	 // ft = (280/255)*new_rate + 20
	 // which yields rates ranging from 20 Hz to ~ 400 Hz in ~ 1.4 Hz increments.
	 // With a prescaler value of 100, a system clock of 72Mhz, and no clock
	 // division, the timer period should be:
	 // new_period = 720000/ft
	 new_period = (uint16_t)(720000.0/(1.09803921*(float)new_rate + 20.0));

	 // Update TIM1
	 TIM_TimeBaseStructure.TIM_Period = new_period;
	 TIM_TimeBaseStructure.TIM_Prescaler = 100;
	 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	 	 
}

/*******************************************************************************
* Function Name  : EnableBroadcastMode
* Input          : None
* Output         : None
* Return         : None
* Description    : Turns on broadcast mode and calls UpdateBroadcastRate
*******************************************************************************/
void EnableBroadcastMode( uint8_t new_rate )
{
	 // Set global configuration variable
	 gConfig.broadcast_enabled = MODE_BROADCAST;
	 
	 // Set broadcast rate
	 UpdateBroadcastRate( new_rate );
	 
	 // Enable Timer 2
	 TIM_Cmd(TIM2, ENABLE);
	 
	 // Clear pending interrupt bit
	 TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	 
	 // TIM IT enable
	 TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

/*******************************************************************************
* Function Name  : DisableBroadcastMode
* Input          : None
* Output         : None
* Return         : None
* Description    : Disables Broadcast Mode by turning off Timer 2
*******************************************************************************/
void DisableBroadcastMode( void )
{
	 // Set global configuration variable
	 gConfig.broadcast_enabled = MODE_LISTEN;
	 
	 // Disable Timer 2
	 TIM_Cmd( TIM2, DISABLE );	 
}

/*******************************************************************************
* Function Name  : StartGyroCalibration
* Input          : None
* Output         : None
* Return         : None
* Description    : Sets the gZeroGyroEnable flag, which triggers the FIR filtering
						  code in process_input_buffers to start summing outputs to 
						  obtain an average.  The average will be used as the new
						  zero point.
*******************************************************************************/
void StartGyroCalibration( void )
{
	 gZeroGyroSampleCount = 0;
	 gZeroGyroAverages[0] = 0;
	 gZeroGyroAverages[1] = 0;
	 gZeroGyroAverages[2] = 0;

	 // Enable gyro zeroing code.  When zeroing is complete, Timer 2 will be reactivated
	 // if the IMU is in Broadcast Mode.
	 gZeroGyroEnable = 1;
	 
}

/*******************************************************************************
* Function Name  : StopGyroCalibration
* Input          : None
* Output         : None
* Return         : None
* Description    : Finishes gyro calibration cycle. This function is called by 
						  FIR filtering software in process_input_buffers after enough
						  data has been collected to make an average.
*******************************************************************************/
void StopGyroCalibration( void )
{
	 gZeroGyroEnable = 0;
	 
	 gZeroGyroAverages[0] = (int16_t)(round((float)gZeroGyroAverages[0]/(float)(gZeroGyroSampleCount)));
	 gZeroGyroAverages[1] = (int16_t)(round((float)gZeroGyroAverages[1]/(float)(gZeroGyroSampleCount)));
	 gZeroGyroAverages[2] = (int16_t)(round((float)gZeroGyroAverages[2]/(float)(gZeroGyroSampleCount)));
	 
	 gConfig.y_gyro_bias = gZeroGyroAverages[0];
	 gConfig.x_gyro_bias = gZeroGyroAverages[1];
	 gConfig.z_gyro_bias = gZeroGyroAverages[2];
	 
	 SendGyroBiasPacket( );
}


/*******************************************************************************
* Function Name  : StartSelfTest
* Input          : None
* Output         : None
* Return         : None
* Description    : Sets the gSelfTestEnable flag, which triggers the self-test
				       sequence on the IMU.
*******************************************************************************/
void StartSelfTest( void )
{
	 gSelfTestEnabled = 1;
	 gSelfTestSamplesIgnored = 0;

	 // Assert self-test pin
	 GPIO_WriteBit( GPIOA, GPIO_Pin_13, Bit_SET );
}

/*******************************************************************************
* Function Name  : StopSelfTest
* Input          : None
* Output         : None
* Return         : None
* Description    : 
*******************************************************************************/
void StopSelfTest( uint16_t result )
{
	 // Clear self-test pin
	 GPIO_WriteBit( GPIOA, GPIO_Pin_13, Bit_RESET );
	 
	 gSelfTestEnabled = 0;
	 
	 SendStatusReportPacket( result );

}

/*******************************************************************************
* Function Name  : ResetToFactory
* Input          : None
* Output         : None
* Return         : None
* Description    : Resets all AHRS settings to factory default.
*******************************************************************************/
void ResetToFactory( void )
{
	 int i, j;
	 
	 // Gyro and accelerometer default biases - these biases are applied to raw data measurements
	 gConfig.x_accel_bias = 0;
	 gConfig.y_accel_bias = 0;
	 gConfig.z_accel_bias = 0;
	 
	 gConfig.x_gyro_bias = -122;
	 gConfig.y_gyro_bias = 106;
	 gConfig.z_gyro_bias = 35;
	 
	 gConfig.x_mag_bias = -20;
	 gConfig.y_mag_bias = -22;
	 gConfig.z_mag_bias = -9;
	 
	 // Gyro scale factor
	 gConfig.x_gyro_scale = .0181;
	 gConfig.y_gyro_scale = .0181;
	 gConfig.z_gyro_scale = .0181;
	 
	 // By default, both accel and magnetometer updates are enabled,
	 // and MAG updates are restricted to yaw angles only
	 gConfig.EKF_config = ACCEL_UPDATE_ENABLED | MAG_UPDATE_ENABLED | MAG_RESTRICTED;
	 
	 gConfig.accel_covariance = 1000;
	 gConfig.mag_covariance = .00001;
	 gConfig.process_covariance = .01;
	 
	 gConfig.mag_ref_vector[0] = 255;
	 gConfig.mag_ref_vector[1] = 0;
	 gConfig.mag_ref_vector[2] = 666;
	 
	 gConfig.accel_ref_vector[0] = 0;
	 gConfig.accel_ref_vector[1] = 0;
	 gConfig.accel_ref_vector[2] = -6000;
	 
	 // Calibration matrices set to identity be default - (ie. no special calibration is performed)
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++ )
		  {
				if( i == j )
				{
					 gConfig.gyro_alignment.data[i][j] = 1;
					 gConfig.accel_alignment.data[i][j] = 1;
				}
				else
				{
					 gConfig.gyro_alignment.data[i][j] = 0;
					 gConfig.accel_alignment.data[i][j] = 0;
					 gConfig.mag_cal.data[i][j] = 0;
				}
		  }
	 }
	 
	 gConfig.mag_cal.data[0][0] = 0.001218163;
	 gConfig.mag_cal.data[1][1] = 0.001197028;
	 gConfig.mag_cal.data[2][2] = 0.001296065;
	 
	 // Channel enable/disable flags
	 gConfig.x_accel_enabled = CHANNEL_ENABLED;
	 gConfig.y_accel_enabled = CHANNEL_ENABLED;
	 gConfig.z_accel_enabled = CHANNEL_ENABLED;
	 gConfig.x_gyro_enabled = CHANNEL_ENABLED;
	 gConfig.y_gyro_enabled = CHANNEL_ENABLED;
	 gConfig.z_gyro_enabled = CHANNEL_ENABLED;
	 gConfig.x_mag_enabled = CHANNEL_ENABLED;
	 gConfig.y_mag_enabled = CHANNEL_ENABLED;
	 gConfig.z_mag_enabled = CHANNEL_ENABLED;
	 gConfig.phi_enabled = CHANNEL_ENABLED;
	 gConfig.theta_enabled = CHANNEL_ENABLED;
	 gConfig.psi_enabled = CHANNEL_ENABLED;
	 gConfig.phi_dot_enabled = CHANNEL_ENABLED;
	 gConfig.theta_dot_enabled = CHANNEL_ENABLED;
	 gConfig.psi_dot_enabled = CHANNEL_ENABLED;
	 
	 gConfig.gyro_startup_calibration = 0;
	 
	 // Broadcast mode or listen mode
	 gConfig.broadcast_enabled = MODE_BROADCAST;
	 gConfig.broadcast_rate = 100;
}


