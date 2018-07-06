/* ------------------------------------------------------------------------------
  File: chr6d_usart.h
  Author: CH Robotics
  Version: 1.0
  
  Description: Function declarations for USART communication
------------------------------------------------------------------------------ */ 
#include "stm32f10x.h"

#ifndef __CHR6D_USART_H
#define __CHR6D_USART_H

// Size of TX and RX buffers. These buffers hold raw transmitted and received data from the USART.
// These buffers should never have to be accessed directly by user code.
#define	RX_BUF_SIZE					50
#define 	TX_BUF_SIZE     			50

// Maximum number of data bytes that can be stored in a packet
#define	MAX_PACKET_DATA			40

// Sizes of buffers for storing RX and TX packet data.  The TX Packet Buffer is used
// to store packets that are "waiting" to be transmitted.  This is necessary since 
// multiple resources may need access to the USART hardware simultaneously.  The TX buffer
// acts as a moderator, storing packets and transmitting them when the hardware becomes available.
// The RX Packet Buffer stores packets received over the USART receiver.  Since multiple packets
// may arrive before a packet is processed, the RX Packet Buffer is neccessary to ensure that no data
// is lost.  Both the TX and RX Packet Buffers are circular.
#define	TX_PACKET_BUFFER_SIZE	20
#define	RX_PACKET_BUFFER_SIZE	20

// Definitions of states for USART receiver state machine (for receiving packets)
#define	USART_STATE_WAIT			1
#define	USART_STATE_TYPE			2
#define	USART_STATE_LENGTH		3
#define	USART_STATE_DATA			4
#define	USART_STATE_CHECKSUM		5

// Definitions of TX packet types (packets that are transmitted by the CHR-6d)
#define	SET_ACTIVE_CHANNELS				0x80
#define	SET_SILENT_MODE					0x81
#define	SET_BROADCAST_MODE				0x82
#define	SET_GYRO_BIAS						0x83
#define	SET_ACCEL_BIAS						0x84
#define	SET_ACCEL_REF_VECTOR				0x85
#define	AUTO_SET_ACCEL_REF				0x86
#define	ZERO_RATE_GYROS					0x87
#define	SELF_TEST							0x88
#define	SET_START_CAL						0x89
#define	SET_PROCESS_COVARIANCE			0x8A
#define	SET_MAG_COVARIANCE				0x8B
#define	SET_ACCEL_COVARIANCE				0x8C
#define	SET_EKF_CONFIG						0x8D
#define	SET_GYRO_ALIGNMENT				0x8E
#define	SET_ACCEL_ALIGNMENT				0x8F
#define	SET_MAG_REF_VECTOR				0x90
#define	AUTO_SET_MAG_REF					0x91
#define	SET_MAG_CAL							0x92
#define	SET_MAG_BIAS						0x93
#define	SET_GYRO_SCALE						0x94
#define	EKF_RESET							0x95
#define	RESET_TO_FACTORY					0x96
        
#define	WRITE_TO_FLASH						0xA0

#define	GET_DATA								0x01
#define	GET_ACTIVE_CHANNELS				0x02
#define	GET_BROADCAST_MODE				0x03
#define	GET_ACCEL_BIAS						0x04
#define	GET_ACCEL_REF_VECTOR				0x05
#define	GET_GYRO_BIAS						0x06
#define	GET_GYRO_SCALE						0x07
#define	GET_START_CAL						0x08
#define	GET_EKF_CONFIG						0x09
#define	GET_ACCEL_COVARIANCE				0x0A
#define	GET_MAG_COVARIANCE				0x0B
#define	GET_PROCESS_COVARIANCE			0x0C
#define	GET_STATE_COVARIANCE				0x0D
#define	GET_GYRO_ALIGNMENT				0x0E
#define	GET_ACCEL_ALIGNMENT				0x0F
#define	GET_MAG_REF_VECTOR				0x10
#define	GET_MAG_CAL							0x11
#define	GET_MAG_BIAS						0x12
        
#define	COMMAND_COMPLETE					0xB0
#define	COMMAND_FAILED						0xB1
#define	BAD_CHECKSUM						0xB2
#define	BAD_DATA_LENGTH					0xB3
#define	UNRECOGNIZED_PACKET				0xB4
#define	BUFFER_OVERFLOW					0xB5
#define	STATUS_REPORT						0xB6
#define	SENSOR_DATA							0xB7
#define	GYRO_BIAS_REPORT					0xB8
#define	GYRO_SCALE_REPORT					0xB9
#define	START_CAL_REPORT					0xBA
#define	ACCEL_BIAS_REPORT					0xBB
#define	ACCEL_REF_VECTOR_REPORT			0xBC
#define	ACTIVE_CHANNEL_REPORT			0xBD
#define	ACCEL_COVARIANCE_REPORT			0xBE
#define	MAG_COVARIANCE_REPORT			0xBF
#define	PROCESS_COVARIANCE_REPORT		0xC0
#define	STATE_COVARIANCE_REPORT			0xC1
#define	EKF_CONFIG_REPORT 				0xC2
#define	GYRO_ALIGNMENT_REPORT			0xC3
#define	ACCEL_ALIGNMENT_REPORT			0xC4
#define	MAG_REF_VECTOR_REPORT			0xC5
#define	MAG_CAL_REPORT						0xC6
#define	MAG_BIAS_REPORT					0xC7
#define	BROADCAST_MODE_REPORT			0xC8

// Union for working with floats at the byte level
typedef union __fconvert 
{
	 uint32_t uint32_val;
	 int32_t int32_val;
	 float float_val;
} fConvert;

// Buffer, buffer index, and TX status flag for USART transmit
extern volatile char gTXBuf[TX_BUF_SIZE];
extern volatile int32_t gTXBufPtr;
extern volatile char gTXBusy;

// USART RX buffer and associated index and flags
extern volatile char gRXBuf[RX_BUF_SIZE];
extern volatile int32_t gRXBufPtr;
extern volatile char gRXPacketReceived;
extern volatile char gRXBufOverrun;
	 
// Structure for storing TX and RX packet data
typedef struct __USARTPacket
{
	 uint8_t PT;				// Packet type
	 uint8_t length;			// Number of bytes in data section
	 uint16_t checksum;		// Checksum
	 
	 uint8_t packet_data[MAX_PACKET_DATA];
} USARTPacket;

// USART interface functions
int32_t USART1_transmit( char* txdata, int32_t length );
int32_t TXBufPush( char txdata );
char TXBufPop( void );
void USART1_TX_start( void );

void USART1_IRQHandler(void);

// Function for copying next packet in the TXPacketBuffer into the TX buffer.
void SendNextPacket( void );
void AddTXPacket( USARTPacket* new_packet );
void AddRXPacket( USARTPacket* new_packet );

void HandleUSART1Reception( void );
void ProcessNextCharacter( void );

uint16_t ComputeChecksum( USARTPacket* new_packet );

void SendTXPacket( USARTPacket* new_packet );
void SendTXPacketSafe( USARTPacket* new_packet );

// Global queue for packets to be transmitted when the UART transmitter becomes available.
extern volatile USARTPacket gTXPacketBuffer[TX_PACKET_BUFFER_SIZE];
extern volatile uint8_t gTXPacketBufferStart;
extern volatile uint8_t gTXPacketBufferEnd;

extern volatile char TXPacketBufferReady;
extern volatile char gCopyingTXPacketToBuffer;

// Global queue for storing received packets
extern volatile USARTPacket gRXPacketBuffer[TX_PACKET_BUFFER_SIZE];
extern volatile uint8_t gRXPacketBufferStart;
extern volatile uint8_t gRXPacketBufferEnd;


#endif