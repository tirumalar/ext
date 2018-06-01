/* ------------------------------------------------------------------------------
  File: main.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Entry point for CHR-6dm firmware
------------------------------------------------------------------------------ */ 

// Includes 
#include <math.h>
#include "stm32f10x.h"
#include "chr6dm_startup.h"
#include "chr6dm_config.h"
#include "chr6dm_usart.h"
#include "chr6dm_packet_handler.h"
#include "chr6dm_ADC.h"
#include "chr6dm_HMC.h"
#include "chr6dm_states.h"

// Buffer to store raw sensor data
int16_t gADC_Output[7];

// Buffers for storing i2c data
uint8_t i2cTxBuf[I2C_TX_BUFSIZE];
uint8_t i2cRxBuf[I2C_RX_BUFSIZE];

void hexPrint8( uint8_t byte );
void hexPrint16( uint16_t bytes );
void DelayMs( uint16_t delay );
void DelayUs( uint16_t delay );

extern uint8_t gClockInUse;

#define MAX_HMC_INIT_RETRIES		10

/*******************************************************************************
* Function Name  : main
* Input          : None
* Output         : None
* Return         : None
* Description    : Entry point for CHR-6dm firmware
*******************************************************************************/
int main(void)
{
	 uint32_t i;
	 uint32_t j;
	 int32_t new_data;
	 int32_t temp = 0;
	 
	 uint8_t HMC_status;
	 
	 int32_t states_initialized = 0;
	 	 
	 gSensorData.new_accel_data = 0;
	 gSensorData.new_mag_data = 0;
	 
	 gGyrosCalibrated = 0;
	 
	 // Initialize the IMU clocks, ADC, DMA controller, GPIO pins, etc.
	 Initialize_IMU();
	 
	 // Fill gConfig structure from flash, or use default values if flash has not been initialized.
    GetConfiguration();
 
	 // Clear I2C RX buffer
	 for( i=0; i < I2C_RX_BUFSIZE; i++ )
	 {
		  i2cRxBuf[i] = 0;
	 }
	 
	 // Wait a while for supply voltages to stabilize
	 DelayMs( 400 );
	 
	 // Initialize HMC5843
	 int retries = 0;
	 while(!initializeHMC( &HMC_status ) && (retries < MAX_HMC_INIT_RETRIES) )
	 {
		  DelayMs( 100 );
		  
		  retries++;
	 }
	 	 
	 DelayMs( 300 );
	 
	 // Configure and enable EXTI line for magnetometer.
	 // This is performed outside chr6dm_startup.c because early erroneous interrupts
	 // on the mag. ready line can cause the unit to poll the magnetometer too quickly,
	 // causing synchronization problems.
	 EXTI_InitTypeDef EXTI_InitStructure;
	 EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  

    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
	 
	 if( !gHMC_Initialized )
	 {
//		  USART1_transmit("Failed to initialize HMC\r\n", 26);
	 }
	 
	 // Start "zero gyros" command.  When finished, the global flag gGyrosCalibrated will be set, and
	 // the main execution code will know to start estimating states and transmitting data.
	 if( gConfig.gyro_startup_calibration )
	 {
		  StartGyroCalibration();
	 }
	 else
	 {
		  gGyrosCalibrated = 1;		  
	 }
	 
	 // Start ADC1 Software Conversion.  ADC1 and ADC2 are configured to sample simultaneously and to 
	 // use DMA1 to transfer data to memory.

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 
	 // Start TIM1 if in broadcast mode
	 if( gConfig.broadcast_enabled == MODE_BROADCAST )
	 {
		  EnableBroadcastMode( gConfig.broadcast_rate );
	 }
	 
	 	 
	 // Start timer3 for prediction period tracking
	 TIM_SetCounter(TIM3,0);
	 TIM_Cmd(TIM3, ENABLE);
	 
	 // Main program loop
    while(1)
    {
		  // Filter new inertial sensor data if available (new_data = 0 if there was no new data ready, 1 otherwise)
		  new_data = process_input_buffers( gADC_Output );
		  
		  // If new data is available, make new state estimates
		  if( new_data && gGyrosCalibrated )
		  {
				gSensorData.new_accel_data = 1;
				
				// Convert to NED coordinate axes
				gSensorData.accel_x = -gADC_Output[0];
				gSensorData.accel_y = gADC_Output[1];
				gSensorData.accel_z = -gADC_Output[2];
				
				// Convert to NED coordinate axes
				gSensorData.gyro_x = gADC_Output[4];
				gSensorData.gyro_y = gADC_Output[3];
				gSensorData.gyro_z = gADC_Output[5];
				
				EKF_EstimateStates( &gEstimatedStates, &gSensorData );
				
				uint8_t output_value = GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_5 );
		  
				if( output_value )
				{
					 GPIO_ResetBits( GPIOB, GPIO_Pin_5 );
				}
				else
				{
					 GPIO_SetBits( GPIOB, GPIO_Pin_5 );
				}
		  
		  }
		  
		  // Handle any new characters received by the UART (piped into memory via DMA)
		  HandleUSART1Reception();
	  
		  // Check if a packet has been received over the UART.  If so, respond.
		  if( gRXPacketReceived )
		  {
				ProcessPacket( (USARTPacket*)&gRXPacketBuffer[gRXPacketBufferStart] );

				// Increment RX packet buffer pointer.  The RX and TX buffer is circular.  The buffer is
				// empty if the "start" and "end" pointers point to the same location.
				gRXPacketBufferStart++;
				if( gRXPacketBufferStart >= RX_PACKET_BUFFER_SIZE )
				{
					 gRXPacketBufferStart = 0;
				}
				
				// If there are no more packets ready for processing, clear the gRXPacketReceived flag.
				// If there is another packet ready, leave the flag set so that the other RX packet will
				// be processed the next time through the main loop.
				if( gRXPacketBufferStart == gRXPacketBufferEnd )
				{
					 gRXPacketReceived = 0;
				}
		  }
		  
		  
		  
    }
	 
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Input          : None
* Output         : None
* Return         : None
* Description    : Called in response to an interrupt from DMA1.  There are two
						 DMA triggered interrupts, one for a complete buffer transfer,
						 and one for a half buffer transfer.  The DMA buffer in memory
						 stores ADC data from the sensors, and is divided into two halves:
						 a "ping" buffer, and a "pong" buffer.  While one buffer is
						 filled, the data in the other is processed by the core.
*******************************************************************************/
void DMA1_Channel1_IRQHandler( void )
{

    // Interrupt service handler for half-transfer
    if( DMA_GetITStatus( DMA1_IT_HT1 ) != RESET )
    {
		  gPingBufferReady = 1;
		      
        DMA_ClearFlag( DMA1_FLAG_HT1 );
    }

    // Interrupt service handler for full transfer
    if( DMA_GetITStatus( DMA1_IT_TC1 ) != RESET )
    {
		  gPongBufferReady = 1;
        
        DMA_ClearFlag( DMA1_FLAG_TC1 );
    }

}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Input          : None
* Output         : None
* Return         : None
* Description    : Called in response to a interrupt from Timer2.  Timer 2 is
						 used to trigger transmission of SENSOR_DATA packets periodically.
						 While the sampling sampling frequency of the ADC converter
						 remains fixed, data is transmitted at variable rates based
						 on the timer configuration.
*******************************************************************************/
void TIM2_IRQHandler( void )
{
	 if( TIM_GetITStatus(TIM2, TIM_IT_Update ) != RESET )
	 {
		  // If in broadcast mode and the gyros have been calibrated, transmit data packet
		  
		  if( (gConfig.broadcast_enabled == MODE_BROADCAST) && gGyrosCalibrated )
		  {
				// If the TX Packet Buffer is being used, then do not send data this time.  This is required since
				// the timer interrupt may have occured when the IMU was responding to a packet received over the
				// UART.
				if( TXPacketBufferReady )
				{
					 TXPacketBufferReady = 0;
					 SendDataPacket( );
					 TXPacketBufferReady = 1;
				}

		  }
		  
		  // Clear pending interrupt bit
		  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	 }
}

/*******************************************************************************
* Function Name  : EXT15_10_IRQHandler
* Input          : None
* Output         : None
* Return         : None
* Description    : External interrupt handler for interupts 15 to 10.  The HMC5843
						 DRDY pin is attached to pin PA15.  When the DRDY pin goes low,
						 the magnetometer has new data to be read.
*******************************************************************************/
void EXTI15_10_IRQHandler( void )
{
	 
	 if(EXTI_GetITStatus(EXTI_Line15) != RESET)
	 {
		  if( gHMC_Initialized )
		  {
				// Get data.  If valid (return val = 1), then process and fill HMC data structure
				if( getHMCData( i2cRxBuf ) )
				{
					 gSensorData.new_mag_data = 1;
							 
					 // NED coordinates
					 gSensorData.mag_y = ((i2cRxBuf[0] << 8) | i2cRxBuf[1]);
					 gSensorData.mag_x = ((i2cRxBuf[2] << 8) | i2cRxBuf[3]);
					 gSensorData.mag_z = -((i2cRxBuf[4] << 8) | i2cRxBuf[5]);
					 
//					 hexPrint16( gSensorData.mag_x );
//					 USART1_transmit("\r\n",2);
					 
				}
				else
				{
	//				 USART1_transmit("Failed to get data\r\n", 20);			 
				}
		  }
		 
		  
		  /* Clear the EXTI line pending bit */
		  EXTI_ClearITPendingBit(EXTI_Line15);
	 }
}

void hexPrint8( uint8_t byte )
{
	 uint8_t char_data[4];
	 uint8_t n0 = (byte & 0x0F);
	 uint8_t n1 = ((byte >> 4) & 0x0F);
	 
	 if( n0 < 10 )
	 {
		  n0 += 48;
	 }
	 else
	 {
		  n0 += 55;
	 }
	 
	 if( n1 < 10 )
	 {
		  n1 += 48;
	 }
	 else
	 {
		  n1 += 55;
	 }

	 char_data[0] = '0';
	 char_data[1] = 'x';
	 char_data[2] = n1;
	 char_data[3] = n0;
	 
	 USART1_transmit( char_data, 4 );	 	 
}

void hexPrint16( uint16_t byte )
{
	 uint8_t char_data[6];
	 uint8_t n0 = (byte & 0x0F);
	 uint8_t n1 = ((byte >> 4) & 0x0F);
	 uint8_t n2 = ((byte >> 8) & 0x0F);
	 uint8_t n3 = ((byte >> 12) & 0x0F);
	 
	 if( n0 < 10 )
	 {
		  n0 += 48;
	 }
	 else
	 {
		  n0 += 55;
	 }
	 
	 if( n1 < 10 )
	 {
		  n1 += 48;
	 }
	 else
	 {
		  n1 += 55;
	 }
	 
	 if( n2 < 10 )
	 {
		  n2 += 48;
	 }
	 else
	 {
		  n2 += 55;
	 }
	 
	 if( n3 < 10 )
	 {
		  n3 += 48;
	 }
	 else
	 {
		  n3 += 55;
	 }

	 char_data[0] = '0';
	 char_data[1] = 'x';
	 char_data[2] = n3;
	 char_data[3] = n2;
	 char_data[4] = n1;
	 char_data[5] = n0;
	 
	 USART1_transmit( char_data, 6 );	 	 
}

void DelayMs( uint16_t delay )
{
	 uint32_t iterations;
	 uint32_t index;
	 
	 iterations = delay*5656;
	 
	 for( index = 0; index < iterations; index++ )
	 {
		  asm volatile("mov r0, r0");
	 }
}

void DelayUs( uint16_t delay )
{
	 uint32_t iterations;
	 uint32_t index;
	 
	 iterations = delay*5.656;
	 
	 for( index = 0; index < iterations; index++ )
	 {
		  asm volatile("mov r0, r0");
	 }
}
