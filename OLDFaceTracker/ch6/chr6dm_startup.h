/* ------------------------------------------------------------------------------
  File: chr6d_startup.h
  Author: CH Robotics
  Version: 1.0
  
  Description: Function declarations for IMU initialization
------------------------------------------------------------------------------ */ 

#ifndef __CHR6D_STARTUP_H
#define __CHR6D_STARTUP_H

#define		ACCEL_X_CHANNEL		ADC_Channel_7
#define		ACCEL_Y_CHANNEL		ADC_Channel_8
#define		ACCEL_Z_CHANNEL		ADC_Channel_9
#define		GYRO_X_CHANNEL			ADC_Channel_4
#define		GYRO_Y_CHANNEL			ADC_Channel_6
#define		GYRO_Z_CHANNEL			ADC_Channel_0
#define		VREF_XY_CHANNEL		ADC_Channel_5
#define		TEMP_SENSOR_CHANNEL	ADC_Channel_16

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define USART1_DR_Base     ((uint32_t)0x40013804)
/*
NOTE:
The actual number of cycles used to sample the ADC channel is given by
    
	 CYCLES = SAMPLE_TIME + 12.5

So, with an an ADC setting of ADC_SampleTime_7Cycles5, the total number
of cycles is given by 7.5 + 12.5 = 20.

The frequency of the ADC is given by
    
	 ADC_CLK/CYCLES
	 
In our case, we use DMA to sample the ADC and pipe the data into memory.
ADC1 and ADC2 are used to sample multiple channels simultaneously.  The
rate at which the memory buffer is filled with new data is given by
    
	 FULL_BULL_FREQ = ADC_CLK/(CYCLES*BUFFER_SIZE*CHANNELS)

where BUFFER_SIZE is the number of each channel that must be taken to
fill the buffer, and CHANNELS is the number of channels being sampled.
Note that since ADC1 and ADC2 sample simultaneously, CHANNELS represents
half the channels actually being read.  For example, if we are sampling
six channels but ADC1 samples three of them and ADC2 samples the other
three, then CHANNELS should equal 3.

Options for ADC_SAMPLE_TIME include
    * ADC_SampleTime_1Cycles5: Sample time equal to 1.5 cycles
    * ADC_SampleTime_7Cycles5: Sample time equal to 7.5 cycles
    * ADC_SampleTime_13Cycles5: Sample time equal to 13.5 cycles
    * ADC_SampleTime_28Cycles5: Sample time equal to 28.5 cycles
    * ADC_SampleTime_41Cycles5: Sample time equal to 41.5 cycles
    * ADC_SampleTime_55Cycles5: Sample time equal to 55.5 cycles
    * ADC_SampleTime_71Cycles5: Sample time equal to 71.5 cycles
    * ADC_SampleTime_239Cycles5: Sample time equal to 239.5 cycles

*/
#define	ADC_SAMPLE_TIME			ADC_SampleTime_7Cycles5

#define	I2C_CLOCK_SPEED			200000
#define	THIS_SLAVE_ADDRESS7		0x30

// Function declarations
void Initialize_IMU(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);

#endif