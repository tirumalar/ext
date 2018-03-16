/* ------------------------------------------------------------------------------
  File: chr6dm_ADC.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Functins for retrieving data from the ADC.  Includes oversampling
					and decimation.
------------------------------------------------------------------------------ */ 
#include "stm32f10x.h"
#include "chr6dm_ADC.h"
#include "chr6dm_startup.h"
#include "chr6dm_config.h"
#include "math.h"

// Allocate space for DMA data transfer.
// Each DMA transfer returns 32 bits of data, with the first 16 from ADC1, and
// the second 16 from ADC2.  The two ADC converters make simultaneous samples.
// Allocate enough space to hold 2*SAMPLES_PER_BUFFER
// samples from each channel (for ping-pong buffer).  Since each 32-bit value 
// actually holds two ADC conversions, we need (CHANNEL_COUNT/2)*2*SAMPLES_PER_BUFFER
// 32-bit values to be stored.
__IO uint32_t ADC_DualConvertedValueTab[ CHANNEL_COUNT*SAMPLES_PER_BUFFER ];

// Set up ping-pong buffer.  When a DMA transfer is halfway complete, an interrupt will
// fire, and the data in the 'ping' buffer will be processed.  When a DMA transfer finishes
// completely, then another interrupt fires and the 'pong' buffer is processed.
__IO uint16_t *gPingBuffer = (__IO uint16_t *)ADC_DualConvertedValueTab;
__IO uint16_t *gPongBuffer = (__IO uint16_t *)(&ADC_DualConvertedValueTab[(CHANNEL_COUNT*SAMPLES_PER_BUFFER)/2]);

volatile int32_t gPingBufferReady = 0;
volatile int32_t gPongBufferReady = 0;

// Variables used for automatic gyro bias calibration
uint16_t gZeroGyroEnable;
int32_t gZeroGyroAverages[3];
uint16_t gZeroGyroSampleCount;
uint16_t gGyrosCalibrated;
						  
// Variables used for automatic self-test feature
uint16_t gSelfTestEnabled;
int16_t gSelfTestResults[6];
uint16_t gSelfTestSamplesIgnored;

/*******************************************************************************
* Function Name  : process_input_buffers
* Input          : NA
* Output         : int16_t* data_output
* Return         : 1 if new data was available, 0 otherwise
* Description    : 

	 If new ADC data is available (ie. the ping buffer or the pong buffer is filled),
	 then this function processes the new data.

	 First, the ADC data in the ping or pong buffer is decimated.  Then, gyro and
	 accelerometer biases are subtracted to create signed versions of the ADC data.

	 This function also handles automatic bias calculation for the rate gyros.

*******************************************************************************/
int32_t process_input_buffers( int16_t* data_output )
{
	 int32_t i,j;
	 __IO uint16_t* input_buffer;
	 int32_t sum_data[CHANNEL_COUNT];
	 int16_t signed_decimated[CHANNEL_COUNT];
	 uint16_t gyro_ref_voltage;
	 	 
	 // ----------------------------------------------------------------------------------
	 // Determine whether gPingBuffer or gPongBuffer are ready.  If not, return 0
	 // ----------------------------------------------------------------------------------
	 if( !gPingBufferReady && !gPongBufferReady )
	 {
		  return 0;
	 }
	 else
	 {		  
		  // Determine which buffer is ready.
		  if( gPingBufferReady )
		  {
				gPingBufferReady = 0;
				input_buffer = gPingBuffer;
		  }
		  else
		  {
				gPongBufferReady = 0;
				input_buffer = gPongBuffer;
		  }
		  
		  // ---------------------------------------------------------------------------------------
		  // Decimation step.  Take ADC data in buffer, convert to signed data, sum, and decimate.
		  // ---------------------------------------------------------------------------------------	  
		  // Pre-initialize sum data
		  for( i = 0; i < CHANNEL_COUNT; i++ )
		  {
				sum_data[i] = 0;
		  }
		  
		  // Convert to signed numbers and sum data
		  for( i = 0; i < SAMPLES_PER_BUFFER; i++ )
		  {
				gyro_ref_voltage = input_buffer[CHANNEL_COUNT*i + 6];
				
				sum_data[0] += (int32_t)(input_buffer[CHANNEL_COUNT*i] - 2048);
				sum_data[1] += (int32_t)(input_buffer[CHANNEL_COUNT*i + 1] - 2048);
				sum_data[2] += (int32_t)(input_buffer[CHANNEL_COUNT*i + 2] - 2048);

				sum_data[3] += (int32_t)(input_buffer[CHANNEL_COUNT*i + 3] - gyro_ref_voltage);
				sum_data[4] += (int32_t)(input_buffer[CHANNEL_COUNT*i + 4] - gyro_ref_voltage);
				sum_data[5] += (int32_t)(input_buffer[CHANNEL_COUNT*i + 5] - gyro_ref_voltage);				
		  }
	  
		  // Decimate
		  signed_decimated[0] = (int16_t)((sum_data[0] >> 5) & 0x0FFFF);		// Accel X
		  signed_decimated[1] = (int16_t)((sum_data[1] >> 5) & 0x0FFFF);		// Accel Y
		  signed_decimated[2] = (int16_t)((sum_data[2] >> 5) & 0x0FFFF);		// Accel Z
		  signed_decimated[3] = (int16_t)((sum_data[3] >> 5) & 0x0FFFF);		// Gyro Y
		  signed_decimated[4] = (int16_t)((sum_data[4] >> 5) & 0x0FFFF);		// Gyro X
		  signed_decimated[5] = (int16_t)((sum_data[5] >> 5) & 0x0FFFF);		// Gyro Z
		  signed_decimated[6] = (int16_t)((sum_data[6] >> 5) & 0x0FFFF);		// Temperature
		  
		  data_output[0] = signed_decimated[0] - gConfig.x_accel_bias;
		  data_output[1] = signed_decimated[1] - gConfig.y_accel_bias;
		  data_output[2] = signed_decimated[2] - gConfig.z_accel_bias;
		  data_output[3] = signed_decimated[3] - gConfig.y_gyro_bias;
		  data_output[4] = signed_decimated[4] - gConfig.x_gyro_bias;
		  data_output[5] = signed_decimated[5] - gConfig.z_gyro_bias;
		  data_output[6] = signed_decimated[6];
		  		  
	
		  // If the "zero gyro" command is enabled, then add the newest measurement
		  // to the avg. buffer
		  if( gZeroGyroEnable )
		  {
				gZeroGyroSampleCount++;
				
				gZeroGyroAverages[0] += signed_decimated[3];
				gZeroGyroAverages[1] += signed_decimated[4];
				gZeroGyroAverages[2] += signed_decimated[5];
				
				if( gZeroGyroSampleCount == GYRO_ZERO_SAMPLE_SIZE )
				{
					 StopGyroCalibration( );
					 
					 gGyrosCalibrated = 1;
				}
		  }
		  
		  // Self-test command
		  if( gSelfTestEnabled )
		  {
				// If self test was just enabled, sample
				// current outputs of sensors for comparison
				if( gSelfTestSamplesIgnored == 0 )
				{
					 gSelfTestResults[0] = signed_decimated[0];
					 gSelfTestResults[1] = signed_decimated[1];
					 gSelfTestResults[2] = signed_decimated[2];
					 gSelfTestResults[3] = signed_decimated[3];
					 gSelfTestResults[4] = signed_decimated[4];
					 gSelfTestResults[5] = signed_decimated[5];
				}
				// ignore SELF_TEST_IGNORE_SAMPLES beforetesting output for change. 
				// This allows the sensors time to react to the self-test signal.
				else if( gSelfTestSamplesIgnored == SELF_TEST_IGNORE_SAMPLES )
				{
					 gSelfTestResults[0] -= signed_decimated[0];
					 gSelfTestResults[1] -= signed_decimated[1];
					 gSelfTestResults[2] -= signed_decimated[2];
					 gSelfTestResults[3] -= signed_decimated[3];
					 gSelfTestResults[4] -= signed_decimated[4];
					 gSelfTestResults[5] -= signed_decimated[5];
					 
					 uint16_t self_test_result = 0;
					 
					 // Check that the sensor outputs responded to the self-test signal
					 if( abs(gSelfTestResults[0]) < SELF_TEST_ACCEL_THRESHOLD )
					 {
						  self_test_result = 0x01;
					 }
					 if( abs(gSelfTestResults[1]) < SELF_TEST_ACCEL_THRESHOLD )
					 {
						  self_test_result = self_test_result | 0x02;
					 }
					 if( abs(gSelfTestResults[2]) < SELF_TEST_ACCEL_THRESHOLD )
					 {
						  self_test_result = self_test_result | 0x04;
					 }
					 if( abs(gSelfTestResults[3]) < SELF_TEST_GYRO_THRESHOLD )
					 {
						  self_test_result = self_test_result | 0x08;
					 }
					 if( abs(gSelfTestResults[4]) < SELF_TEST_GYRO_THRESHOLD )
					 {
						  self_test_result = self_test_result | 0x10;
					 }
					 if( abs(gSelfTestResults[5]) < SELF_TEST_GYRO_THRESHOLD )
					 {
						  self_test_result = self_test_result | 0x20;
					 }
					 
					 // Stop self test and send result packet
					 StopSelfTest( self_test_result );
				}
				
				gSelfTestSamplesIgnored++;
		  }
 
	 }
	 
	 return 1;
}
