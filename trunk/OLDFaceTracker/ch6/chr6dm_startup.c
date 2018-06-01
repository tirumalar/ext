/* ------------------------------------------------------------------------------
  File: chr6dm_startup.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Function definitions for AHRS initialization
------------------------------------------------------------------------------ */ 
#include "stm32f10x.h"
#include "chr6dm_startup.h"
#include "chr6dm_ADC.h"
#include "chr6dm_usart.h"

uint8_t gClockInUse;

/*******************************************************************************
* Function Name  : Initialize_IMU
* Input          : None
* Output         : None
* Return         : None
* Description    : Initializes the system clock and all peripherals of the CHR-6d
*******************************************************************************/
void Initialize_IMU( void )
{
	 USART_InitTypeDef USART_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	 I2C_InitTypeDef   I2C_InitStructure;
	 EXTI_InitTypeDef EXTI_InitStructure;
	 	 
	 // System Clocks Configuration
    RCC_Configuration();
       
    // NVIC configuration
    NVIC_Configuration();

    // Configure the GPIO ports 
    GPIO_Configuration();

	 // DMA1 channel1 configuration
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = CHANNEL_COUNT*SAMPLES_PER_BUFFER;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    /* Enable DMA1 Channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    // Enable the DMA1 interrupt for transfer complete and half-transfer complete on channel 1
    DMA_ITConfig( DMA1_Channel1, DMA_IT_TC, ENABLE );
    DMA_ITConfig( DMA1_Channel1, DMA_IT_HT, ENABLE );
	 
	 // USART1 RX DMA1 Channel (triggered by USARTy Rx event) Config 
	 DMA_DeInit(DMA1_Channel5);
	 DMA_StructInit(&DMA_InitStructure);
	 DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
	 DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gRXBuf;
	 DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	 DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	 DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	 DMA_InitStructure.DMA_BufferSize = RX_BUF_SIZE;
	 DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	 DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	 DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	 
	 DMA_Init( DMA1_Channel5, &DMA_InitStructure );
	 
	 // USART1  ------------------------------------------------------
    /* USART1 is configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART1 */
    USART_Init(USART1, &USART_InitStructure);

    /* Enable USART1 Receive and Transmit interrupts */
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	 // Enable USART1 Receive DMA requests
	 USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	 
	 // Enable DMA1 USART RX channel
	 DMA_Cmd(DMA1_Channel5, ENABLE);
    
    // Enable USART1
    USART_Cmd(USART1, ENABLE);
    
	 // Enable internal temperature sensor
//	 ADC_TempSensorVrefintCmd( ENABLE );
	 
    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 4;
    ADC_Init(ADC1, &ADC_InitStructure);
    /* ADC1 regular channels configuration */ 
	  
	 ADC_RegularChannelConfig(ADC1, ACCEL_X_CHANNEL, 1, ADC_SAMPLE_TIME);    
    ADC_RegularChannelConfig(ADC1, ACCEL_Z_CHANNEL, 2, ADC_SAMPLE_TIME);
    ADC_RegularChannelConfig(ADC1, GYRO_Y_CHANNEL, 3, ADC_SAMPLE_TIME);
	 ADC_RegularChannelConfig(ADC1, VREF_XY_CHANNEL, 4, ADC_SAMPLE_TIME);
	
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* ADC2 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 4;
    ADC_Init(ADC2, &ADC_InitStructure);
    /* ADC2 regular channels configuration */ 
	 
	 ADC_RegularChannelConfig(ADC2, ACCEL_Y_CHANNEL, 1, ADC_SAMPLE_TIME);
    ADC_RegularChannelConfig(ADC2, GYRO_X_CHANNEL, 2, ADC_SAMPLE_TIME);
    ADC_RegularChannelConfig(ADC2, GYRO_Z_CHANNEL, 3, ADC_SAMPLE_TIME);
	 ADC_RegularChannelConfig(ADC2, VREF_XY_CHANNEL, 4, ADC_SAMPLE_TIME);
	
    /* Enable ADC2 external trigger conversion (to be controlled by ADC1)*/
    ADC_ExternalTrigConvCmd(ADC2, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
    /* Enable Vrefint channel17 */
    ADC_TempSensorVrefintCmd(ENABLE);

    /* Enable ADC1 reset calibration register */   
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));

    /* Enable ADC2 */
    ADC_Cmd(ADC2, ENABLE);

    /* Enable ADC2 reset calibaration register */   
    ADC_ResetCalibration(ADC2);
    /* Check the end of ADC2 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC2));

    /* Start ADC2 calibaration */
    ADC_StartCalibration(ADC2);
    /* Check the end of ADC2 calibration */
    while(ADC_GetCalibrationStatus(ADC2));

	 
	 // Configure Timer2 for state transmission
/*	 TIM_TimeBaseStructure.TIM_Period = 30000;
	 TIM_TimeBaseStructure.TIM_Prescaler = 100;
	 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	*/ 
	 TIM_TimeBaseStructure.TIM_Period = 1000;
	 TIM_TimeBaseStructure.TIM_Prescaler = 1;
	 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	 // Configure Timer3 for prediction loop (used to compute elapsed time between
	 // state measurement updates)
	 TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	 TIM_TimeBaseStructure.TIM_Prescaler = 72;
	 TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	 TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	 
	 // Pull Self-Test pin low
	 GPIO_WriteBit( GPIOA, GPIO_Pin_13, Bit_RESET );
	 
	 // Enable I2C
	 I2C_Cmd(I2C1, ENABLE);
	 
	 // I2C1 configuration
	 I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	 I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	 I2C_InitStructure.I2C_OwnAddress1 = THIS_SLAVE_ADDRESS7;
	 I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	 I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	 I2C_InitStructure.I2C_ClockSpeed = I2C_CLOCK_SPEED;
	 I2C_Init(I2C1, &I2C_InitStructure);
	 
	 // Enable external interrupt 15 (DRDY pin for magnetometer)
	 // Map PA15 to EXTI 15
	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource15);

    
	 
	 // (Interrupt priority for EXTI line set in NVIC configuration function)
}


/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
	 ErrorStatus HSEStartUpStatus;
	 
	 RCC_DeInit();
	 
    /* Disable HSI */
//    RCC_HSICmd(ENABLE);
	 
	 /* Enable external oscillator */
	 RCC_HSEConfig(RCC_HSE_ON);
	 
	 /* Wait till HSE is ready */
	 HSEStartUpStatus = RCC_WaitForHSEStartUp();
//	 while( RCC_WaitForHSEStartUp() != SUCCESS );
	 
	 /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK/1 */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* ADCCLK = PCLK2/6 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); 

	 if( HSEStartUpStatus == SUCCESS )
	 {
		  gClockInUse = 'E';
		  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	// 8 / 1 * 9 = 72 Mhz
	 }
	 else
	 {
		  gClockInUse = 'I';
		  RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
	 }
	 
    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
     {;}

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
     {;}

    /* Enable peripheral clocks --------------------------------------------------*/

    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
    
    // Enable ADC1 and ADC2 clocks
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE );
                             
    // Enable DMA clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
			
	 // Enable TIM2 clock
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			
	 // Enable TIM3 clock
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			
	 // Enable i2c1 clock
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Input          : None
* Output         : None
* Return         : None
* Description    : Configures the GPIO ports on the IMU
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure USART1 Rx as input floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure USART1 Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 // Configure USART2 Rx as input floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure USART2 Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 // Temp - output clock on pin A8
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 // CONFIGURE A/D PINS.  A/D INPUTS COME IN THROUGH CHANNELS 1 THROUGH 9

    // Configure PA.0,4,5,6,7 as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure PB0 and PB1 as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	 
	 // Configure PA13 to use alternate function (default is
	 // JTAG - we want GPIO).  PA13 is used as the self-test control
	 // line.
	 GPIO_PinRemapConfig( GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	 GPIO_PinRemapConfig( GPIO_Remap_SWJ_Disable , ENABLE);
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 // Configure PA15 as input floating (alternate function, open-drain for PA15)
	 GPIO_PinRemapConfig( GPIO_PartialRemap1_TIM2, DISABLE);
	 GPIO_PinRemapConfig( GPIO_PartialRemap2_TIM2, DISABLE);
	 GPIO_PinRemapConfig( GPIO_FullRemap_TIM2, DISABLE);
	 GPIO_PinRemapConfig( GPIO_Remap_SPI1 , DISABLE);
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 //   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 // Configure I2C1 pins: SCL and SDA
	 GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
	 
	 // Set GPIO pin B5 as output Push-Pull
	 GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
}   


/*******************************************************************************
* Function Name  : NVIC_Configuration
* Input          : None
* Output         : None
* Return         : None
* Description    : Configures the vectored interrupt table on the IMU
*******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the DMA1 channel1 interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	 
	 // Enable the TIM2 global Interrupt and set at lowest priority.
	 // This is used to tell the MCU to transmit newest state data over the UART
	 NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	 
	 // Enable and set EXTI Interrupt to highest priority (once an i2c transfer starts,
	 // it shouldn't be interrupted.
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
	 
	 // Configure and enable I2C1 event interrupt
	 /* i2c bus not using interrupts 
	 NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	 */
}
