/* ------------------------------------------------------------------------------
  File: chr6d_packet_handler.h
  Author: CH Robotics
  Version: 1.0
  
  Description: Definitions used for handling packets.
------------------------------------------------------------------------------ */ 
#ifndef __CHR6D_PACKET_HANDLER_H
#define __CHR6D_PACKET_HANDLER_H

#include "chr6dm_usart.h"
#include "chr6dm_ADC.h"
#include "chr6dm_startup.h"
#include "chr6dm_states.h"

void ProcessPacket( USARTPacket* new_packet );

void SendCommandSuccessPacket( int32_t command );
void SendCommandFailedPacket( int32_t command, int32_t flag );

void SendDataPacket( );

void SendGyroBiasPacket( void );
void SendAccelBiasPacket( void );
void SendMagBiasPacket( void );
void SendActiveChannelPacket( void );
void SendTransmitModePacket( void );
void SendStatusReportPacket( uint16_t result );
void SendMagCalPacket( void );
void SendMagRefVectorPacket( void );
void SendAccelAlignmentPacket( void );
void SendGyroAlignmentPacket( void );
void SendStateCovariancePacket( void );
void SendProcessCovariancePacket( void );
void SendMAGCovariancePacket( void );
void SendAccelCovariancePacket( void );
void SendEKFConfigPacket( void );
void SendStartCalPacket( void );
void SendGyroScalePacket( void );
void SendAccelRefVectorPacket( void );

void PackageMatrixForTransmit( USARTPacket* packet, fmat3x3* matrix );

// Global data buffers defined in main.c

extern RawSensorData gSensorData;
extern AHRS_states gEstimatedStates;

#endif
