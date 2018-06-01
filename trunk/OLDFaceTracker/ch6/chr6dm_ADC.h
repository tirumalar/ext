/* ------------------------------------------------------------------------------
  File: chr6d_FIR.h
  Author: CH Robotics
  Version: 1.0
  
  Description: Functions for performing FIR filtering
------------------------------------------------------------------------------ */ 

#ifndef __CHR6D_FIR_H
#define __CHR6D_FIR_H

// Number of samples to take before decimating
#define		SAMPLES_PER_BUFFER	512

// Number of samples to take when computing gyro biases.
#define		GYRO_ZERO_SAMPLE_SIZE	500

// The number of samples to ignore after the self-test feature
// has been activated.  This gives the devices time to respond
// to the self-test signal
#define		SELF_TEST_IGNORE_SAMPLES	200

// Thresholds used to determine whether self-test on gyro and accel
// channels was successfull.  This referes to the difference between
// the pre self-test sensor output and the post self-test sensor output.
#define		SELF_TEST_ACCEL_THRESHOLD	500
#define		SELF_TEST_GYRO_THRESHOLD	200

#define		CHANNEL_COUNT			8

// Filtering functions
int32_t process_input_buffers( int16_t* data_output );

// Global variables used to zero the rate gyros.  gZeroGyroEnable is a flag specifying whether
// zeroing is enabled.  gZeroGyroSampleCount specifies how many samples have been taken so far.
// gZeroGyroAverages contains the sum of all samples taken during the zeroing process; when 
// GYRO_ZERO_SAMPLE_SIZE samples have been taken, the averages are computed and stored.
extern uint16_t gZeroGyroEnable;
extern int32_t gZeroGyroAverages[3];
extern uint16_t gZeroGyroSampleCount;
extern uint16_t gGyrosCalibrated;

// Variables used for automatic self-test feature
extern uint16_t gSelfTestEnabled;
extern int16_t gSelfTestResults[6];
extern uint16_t gSelfTestSamplesIgnored;

// Buffer declarations - used for storing ADC data returned by the DMA controller.
extern __IO uint32_t ADC_DualConvertedValueTab[ CHANNEL_COUNT*SAMPLES_PER_BUFFER ];

extern __IO uint16_t *gPingBuffer;
extern __IO uint16_t *gPongBuffer;

extern volatile int32_t gPingBufferReady;
extern volatile int32_t gPongBufferReady;

#endif