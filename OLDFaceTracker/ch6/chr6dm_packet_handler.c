/* ------------------------------------------------------------------------------
  File: chr6dm_packet_handler.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Functions used to handle packets sent and received by the CHR-6dm
------------------------------------------------------------------------------ */ 

#include "stm32f10x.h"
#include "chr6dm_packet_handler.h"
#include "chr6dm_config.h"
#include "chr6dm_ADC.h"
#include "chr6dm_states.h"

/*******************************************************************************
* Function Name  : ProcessPacket
* Input          : USARTPacket*
* Output         : None
* Return         : None
* Description    : Takes a packet received over the UART and processes it, calling
*						the appropriate functions to act on the new data.
*******************************************************************************/
void ProcessPacket( USARTPacket* new_packet )
{
	 USARTPacket response_packet;
	 int32_t result;
	 fConvert iTof;
	 int i, j;
	 
	 // Determine the packet type
	 switch( new_packet->PT )
	 {
		  // -----------------------------------------------------------------------------
		  // SET_ACTIVE_CHANNELS packet - Sets channels whose data will be transmitted
		  // when a GET_DATA packet is sent, or when Broadcast Mode is enabled
		  // -----------------------------------------------------------------------------
		  case SET_ACTIVE_CHANNELS:
				gConfig.psi_enabled = (uint8_t)( (new_packet->packet_data[0] >> 7) & 0x01);
				gConfig.theta_enabled = (uint8_t)( (new_packet->packet_data[0] >> 6) & 0x01);
				gConfig.phi_enabled = (uint8_t)( (new_packet->packet_data[0] >> 5) & 0x01);
				gConfig.psi_dot_enabled = (uint8_t)( (new_packet->packet_data[0] >> 4) & 0x01);
				gConfig.theta_dot_enabled = (uint8_t)( (new_packet->packet_data[0] >> 3) & 0x01);
				gConfig.phi_dot_enabled = (uint8_t)( (new_packet->packet_data[0] >> 2) & 0x01);
				gConfig.x_mag_enabled = (uint8_t)( (new_packet->packet_data[0] >> 1) & 0x01);
				gConfig.y_mag_enabled = (uint8_t)( (new_packet->packet_data[0]) & 0x01);
				
				gConfig.z_mag_enabled = (uint8_t)( (new_packet->packet_data[1] >> 7) & 0x01);
				gConfig.x_gyro_enabled = (uint8_t)( (new_packet->packet_data[1] >> 6) & 0x01);
				gConfig.y_gyro_enabled = (uint8_t)( (new_packet->packet_data[1] >> 5) & 0x01);
				gConfig.z_gyro_enabled = (uint8_t)( (new_packet->packet_data[1] >> 4) & 0x01);
				gConfig.x_accel_enabled = (uint8_t)( (new_packet->packet_data[1] >> 3) & 0x01);
				gConfig.y_accel_enabled = (uint8_t)( (new_packet->packet_data[1] >> 2) & 0x01);
				gConfig.z_accel_enabled = (uint8_t)( (new_packet->packet_data[1] >> 1) & 0x01);
						  
				SendCommandSuccessPacket( SET_ACTIVE_CHANNELS );
				
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_SILENT_MODE packet - Turns off broadcast mode.
		  // -----------------------------------------------------------------------------
		  case SET_SILENT_MODE:
				DisableBroadcastMode( );
		  
				SendCommandSuccessPacket( SET_SILENT_MODE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_BROADCAST_MODE packet - turns on broadcast mode with the frequency
		  // specified in packet_data[0].
		  // -----------------------------------------------------------------------------
		  case SET_BROADCAST_MODE:
				EnableBroadcastMode( new_packet->packet_data[0] );
		  
				SendCommandSuccessPacket( SET_BROADCAST_MODE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_GYRO_BIAS packet - Sets bias term for rate gyros.
		  // -----------------------------------------------------------------------------
		  case SET_GYRO_BIAS:
				gConfig.z_gyro_bias = ((uint16_t)new_packet->packet_data[0] << 8) | ((uint16_t)new_packet->packet_data[1] & 0x0FF);
				gConfig.y_gyro_bias = ((uint16_t)new_packet->packet_data[2] << 8) | ((uint16_t)new_packet->packet_data[3] & 0x0FF);
				gConfig.x_gyro_bias = ((uint16_t)new_packet->packet_data[4] << 8) | ((uint16_t)new_packet->packet_data[5] & 0x0FF);
		  
				SendCommandSuccessPacket( SET_GYRO_BIAS );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_ACCEL_BIAS packet - Sets bias term for accels
		  // -----------------------------------------------------------------------------
		  case SET_ACCEL_BIAS:
				gConfig.z_accel_bias = ((int16_t)new_packet->packet_data[0] << 8) | ((uint16_t)new_packet->packet_data[1] & 0x0FF);
				gConfig.y_accel_bias = ((int16_t)new_packet->packet_data[2] << 8) | ((uint16_t)new_packet->packet_data[3] & 0x0FF);
				gConfig.x_accel_bias = ((int16_t)new_packet->packet_data[4] << 8) | ((uint16_t)new_packet->packet_data[5] & 0x0FF);
		  
				SendCommandSuccessPacket( SET_ACCEL_BIAS );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_MAG_BIAS packet - Sets bias term for magnetometer
		  // -----------------------------------------------------------------------------
		  case SET_MAG_BIAS:
				gConfig.z_mag_bias = ((int16_t)new_packet->packet_data[0] << 8) | ((uint16_t)new_packet->packet_data[1] & 0x0FF);
				gConfig.y_mag_bias = ((int16_t)new_packet->packet_data[2] << 8) | ((uint16_t)new_packet->packet_data[3] & 0x0FF);
				gConfig.x_mag_bias = ((int16_t)new_packet->packet_data[4] << 8) | ((uint16_t)new_packet->packet_data[5] & 0x0FF);
		  
				SendCommandSuccessPacket( SET_MAG_BIAS );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // ZERO_RATE_GYROS packet - starts gyro zero command
		  // -----------------------------------------------------------------------------
		  case ZERO_RATE_GYROS:
				StartGyroCalibration();
		  
				// No COMMAND_COMPLETE packet is sent from here.  When the gyro zeroing is finished,
				// a COMMAND_COMPLETE packet is sent from within the filtering code.
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SELF_TEST packet - starts automatic self-test command
		  // -----------------------------------------------------------------------------
		  case SELF_TEST:
				StartSelfTest();
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_START_CAL packet - Specifies whether AHRS should calibrate rate gyros
		  // on startup.
		  // -----------------------------------------------------------------------------
		  case SET_START_CAL:
				gConfig.gyro_startup_calibration = new_packet->packet_data[0] & 0x01;
		  
				SendCommandSuccessPacket( SET_START_CAL );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_PROCESS_COVARIANCE packet - sets the value of the diagonal elements of
		  // the process covariance matrix.
		  // -----------------------------------------------------------------------------
		  case SET_PROCESS_COVARIANCE:
				iTof.uint32_val = (new_packet->packet_data[0] << 24) | (new_packet->packet_data[1] << 16) | (new_packet->packet_data[2] << 8) | (new_packet->packet_data[3]);
				gConfig.process_covariance = iTof.float_val;
		  
				gEstimatedStates.Q.data[0][0] = gConfig.process_covariance;
				gEstimatedStates.Q.data[1][1] = gConfig.process_covariance;
				gEstimatedStates.Q.data[2][2] = gConfig.process_covariance;
		  
				SendCommandSuccessPacket( SET_PROCESS_COVARIANCE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_MAG_COVARIANCE packet - sets the value of the diagonal elements of
		  // the magnetometer covariance matrix.
		  // -----------------------------------------------------------------------------
		  case SET_MAG_COVARIANCE:
				iTof.uint32_val = (new_packet->packet_data[0] << 24) | (new_packet->packet_data[1] << 16) | (new_packet->packet_data[2] << 8) | (new_packet->packet_data[3]);
				gConfig.mag_covariance = iTof.float_val;
		  
				gEstimatedStates.Rmag.data[0][0] = gConfig.mag_covariance;
				gEstimatedStates.Rmag.data[1][1] = gConfig.mag_covariance;
				gEstimatedStates.Rmag.data[2][2] = gConfig.mag_covariance;
		  
				SendCommandSuccessPacket( SET_MAG_COVARIANCE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_ACCEL_COVARIANCE packet - sets the value of the diagonal elements of
		  // the accel covariance matrix.
		  // -----------------------------------------------------------------------------
		  case SET_ACCEL_COVARIANCE:
				iTof.uint32_val = (new_packet->packet_data[0] << 24) | (new_packet->packet_data[1] << 16) | (new_packet->packet_data[2] << 8) | (new_packet->packet_data[3]);
				gConfig.accel_covariance = iTof.float_val;
		  
				gEstimatedStates.Racc.data[0][0] = gConfig.accel_covariance;
				gEstimatedStates.Racc.data[1][1] = gConfig.accel_covariance;
				gEstimatedStates.Racc.data[2][2] = gConfig.accel_covariance;
		  
				SendCommandSuccessPacket( SET_ACCEL_COVARIANCE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_EKF_CONFIG packet.
		  // -----------------------------------------------------------------------------
		  case SET_EKF_CONFIG:
				gConfig.EKF_config = new_packet->packet_data[0];
		  
				SendCommandSuccessPacket( SET_EKF_CONFIG );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_GYRO_ALIGNMENT packet
		  // -----------------------------------------------------------------------------
		  case SET_GYRO_ALIGNMENT:
				for (i = 0; i < 3; i++ )
				{
					 for (j = 0; j <3; j++ )
					 {
						  iTof.uint32_val = (new_packet->packet_data[4*(3*i+j)] << 24) | (new_packet->packet_data[4*(3*i+j)+1] << 16) | (new_packet->packet_data[4*(3*i+j)+2] << 8) | (new_packet->packet_data[4*(3*i+j)+3]);
						  
						  gConfig.gyro_alignment.data[i][j] = iTof.float_val;
					 }
				}
		  
				SendCommandSuccessPacket( SET_GYRO_ALIGNMENT );
		  break;
				
		  // -----------------------------------------------------------------------------
		  // SET_ACCEL_ALIGNMENT packet
		  // -----------------------------------------------------------------------------
		  case SET_ACCEL_ALIGNMENT:
				for (i = 0; i < 3; i++ )
				{
					 for (j = 0; j <3; j++ )
					 {
						  iTof.uint32_val = (new_packet->packet_data[4*(3*i+j)] << 24) | (new_packet->packet_data[4*(3*i+j)+1] << 16) | (new_packet->packet_data[4*(3*i+j)+2] << 8) | (new_packet->packet_data[4*(3*i+j)+3]);
						  
						  gConfig.accel_alignment.data[i][j] = iTof.float_val;
					 }
				}
		  
				SendCommandSuccessPacket( SET_ACCEL_ALIGNMENT );
		  break;
				
		  // -----------------------------------------------------------------------------
		  // SET_MAG_CAL packet
		  // -----------------------------------------------------------------------------
		  case SET_MAG_CAL:
				for (i = 0; i < 3; i++ )
				{
					 for (j = 0; j <3; j++ )
					 {
						  iTof.uint32_val = (new_packet->packet_data[4*(3*i+j)] << 24) | (new_packet->packet_data[4*(3*i+j)+1] << 16) | (new_packet->packet_data[4*(3*i+j)+2] << 8) | (new_packet->packet_data[4*(3*i+j)+3]);
						  
						  gConfig.mag_cal.data[i][j] = iTof.float_val;
					 }
				}
		  
				SendCommandSuccessPacket( SET_MAG_CAL );
		  break;
				
		  // -----------------------------------------------------------------------------
		  // SET_MAG_REF_VECTOR packet
		  // -----------------------------------------------------------------------------
		  case SET_MAG_REF_VECTOR:
				gConfig.mag_ref_vector[2] = (new_packet->packet_data[0] << 8) | new_packet->packet_data[1];
				gConfig.mag_ref_vector[1] = (new_packet->packet_data[2] << 8) | new_packet->packet_data[3];
				gConfig.mag_ref_vector[0] = (new_packet->packet_data[4] << 8) | new_packet->packet_data[5];				
		  
				SendCommandSuccessPacket( SET_MAG_REF_VECTOR );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // SET_ACCEL_REF_VECTOR packet
		  // -----------------------------------------------------------------------------
		  case SET_ACCEL_REF_VECTOR:
				gConfig.accel_ref_vector[2] = (new_packet->packet_data[0] << 8) | new_packet->packet_data[1];
				gConfig.accel_ref_vector[1] = (new_packet->packet_data[2] << 8) | new_packet->packet_data[3];
				gConfig.accel_ref_vector[0] = (new_packet->packet_data[4] << 8) | new_packet->packet_data[5];				
		  
				SendCommandSuccessPacket( SET_ACCEL_REF_VECTOR );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // AUTO_SET_MAG_REF packet
		  // -----------------------------------------------------------------------------
		  case AUTO_SET_MAG_REF:
				gConfig.mag_ref_vector[2] = gSensorData.mag_z;
				gConfig.mag_ref_vector[1] = gSensorData.mag_y;
				gConfig.mag_ref_vector[0] = gSensorData.mag_x;
		  
				SendMagRefVectorPacket();
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // AUTO_SET_ACCEL_REF packet
		  // -----------------------------------------------------------------------------
		  case AUTO_SET_ACCEL_REF:
				gConfig.accel_ref_vector[2] = gSensorData.accel_z;
				gConfig.accel_ref_vector[1] = gSensorData.accel_y;
				gConfig.accel_ref_vector[0] = gSensorData.accel_x;
		  
				SendAccelRefVectorPacket();
		  break;
				
		  // -----------------------------------------------------------------------------
		  // SET_GYRO_SCALE packet
		  // -----------------------------------------------------------------------------
		  case SET_GYRO_SCALE:
				iTof.uint32_val = (new_packet->packet_data[0] << 24) | (new_packet->packet_data[1] << 16) | (new_packet->packet_data[2] << 8) | (new_packet->packet_data[3]);
				gConfig.z_gyro_scale = iTof.float_val;
				
				iTof.uint32_val = (new_packet->packet_data[4] << 24) | (new_packet->packet_data[5] << 16) | (new_packet->packet_data[6] << 8) | (new_packet->packet_data[7]);
				gConfig.y_gyro_scale = iTof.float_val;
		  
				iTof.uint32_val = (new_packet->packet_data[8] << 24) | (new_packet->packet_data[9] << 16) | (new_packet->packet_data[10] << 8) | (new_packet->packet_data[11]);
				gConfig.x_gyro_scale = iTof.float_val;
		  
				SendCommandSuccessPacket( SET_GYRO_SCALE );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // EKF_RESET packet
		  // -----------------------------------------------------------------------------
		  case EKF_RESET:
				
				EKF_Init( &gEstimatedStates );
		  
				SendCommandSuccessPacket( EKF_RESET );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // RESET_TO_FACTORY packet
		  // -----------------------------------------------------------------------------
		  case RESET_TO_FACTORY:
				
				ResetToFactory();
		  
				SendCommandSuccessPacket( RESET_TO_FACTORY );
		  break;
		  
		  // -----------------------------------------------------------------------------
		  // WRITE_TO_FLASH packet - saves all settings to flash.
		  // -----------------------------------------------------------------------------
		  case WRITE_TO_FLASH:
				
				// Disable the ADC temporarily - the WRITE_TO_FLASH command takes longer to complete
				// than it takes to fill an input buffer with data.
				ADC_SoftwareStartConvCmd(ADC1, DISABLE);
				
				// If BROADCAST_MODE is enabled, turn it off while writing configuration to flash
				if( gConfig.broadcast_enabled == MODE_BROADCAST )
				{
					 // Disable Timer 2
					 TIM_Cmd( TIM2, DISABLE );	
		  
					 result = WriteConfigurationToFlash( );
					 
					 // Enable Timer 2
					 TIM_Cmd( TIM2, ENABLE );	
				}
				else
				{
					 result = WriteConfigurationToFlash( );
				}
		  
				ADC_SoftwareStartConvCmd(ADC1, ENABLE);
								
				if( result == FLASH_COMPLETE )
				{
					 SendCommandSuccessPacket( WRITE_TO_FLASH );
				}
				else
				{
					 SendCommandFailedPacket( WRITE_TO_FLASH, result );
				}
				
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_DATA packet - ignored if IMU is in broadcast mode.  If in listen mode,
		  // return data from all active channels.
		  // ------------------------------------------------------------------------------
		  case GET_DATA:
				if( !gConfig.broadcast_enabled )
				{
					 SendDataPacket();
				}
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_GYRO_BIAS packet - Sends a packet containing the bias values
		  // for each gyro channel.
		  // ------------------------------------------------------------------------------
		  case GET_GYRO_BIAS:
				SendGyroBiasPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_ACCEL_BIAS packet - Sends a packet containing the bias values
		  // for each accel channel.
		  // ------------------------------------------------------------------------------
		  case GET_ACCEL_BIAS:
				SendAccelBiasPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_MAG_BIAS packet - Sends a packet containing the bias values
		  // for each mag channel.
		  // ------------------------------------------------------------------------------
		  case GET_MAG_BIAS:
				SendMagBiasPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_GYRO_SCALE packet
		  // ------------------------------------------------------------------------------
		  case GET_GYRO_SCALE:
				SendGyroScalePacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_START_CAL packet
		  // ------------------------------------------------------------------------------
		  case GET_START_CAL:
				SendStartCalPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_EKF_CONFIG packet
		  // ------------------------------------------------------------------------------
		  case GET_EKF_CONFIG:
				SendEKFConfigPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_ACCEL_COVARIANCE packet
		  // ------------------------------------------------------------------------------
		  case GET_ACCEL_COVARIANCE:
				SendAccelCovariancePacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_MAG_COVARIANCE packet
		  // ------------------------------------------------------------------------------
		  case GET_MAG_COVARIANCE:
				SendMAGCovariancePacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_PROCESS_COVARIANCE packet
		  // ------------------------------------------------------------------------------
		  case GET_PROCESS_COVARIANCE:
				SendProcessCovariancePacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_STATE_COVARIANCE packet
		  // ------------------------------------------------------------------------------
		  case GET_STATE_COVARIANCE:
				SendStateCovariancePacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_GYRO_ALIGNMENT packet
		  // ------------------------------------------------------------------------------
		  case GET_GYRO_ALIGNMENT:
				SendGyroAlignmentPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_ACCEL_ALIGNMENT packet
		  // ------------------------------------------------------------------------------
		  case GET_ACCEL_ALIGNMENT:
				SendAccelAlignmentPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_MAG_CAL packet
		  // ------------------------------------------------------------------------------
		  case GET_MAG_CAL:
				SendMagCalPacket();
		  break;

		  // ------------------------------------------------------------------------------
		  // GET_MAG_REF_VECTOR packet
		  // ------------------------------------------------------------------------------
		  case GET_MAG_REF_VECTOR:
				SendMagRefVectorPacket();
		  break;
		  
		  // ------------------------------------------------------------------------------
		  // GET_ACCEL_REF_VECTOR packet
		  // ------------------------------------------------------------------------------
		  case GET_ACCEL_REF_VECTOR:
				SendAccelRefVectorPacket();
		  break;
		  
		  
		  // ------------------------------------------------------------------------------
		  // GET_ACTIVE_CHANNELS packet - Returns a packet specifying whether each sensor
		  // channel is active or inactive.
		  // ------------------------------------------------------------------------------
		  case GET_ACTIVE_CHANNELS:
				SendActiveChannelPacket();
		  break;
		  
		  case GET_BROADCAST_MODE:
				SendTransmitModePacket();
		  break;
		  
		  default:
				// The packet was not recognized.  Send an UNRECOGNIZED_PACKET packet
				response_packet.PT = UNRECOGNIZED_PACKET;
				response_packet.length = 1;
				response_packet.packet_data[0] = new_packet->PT;
				response_packet.checksum = ComputeChecksum( &response_packet );
				
				SendTXPacketSafe( &response_packet );
		  
		  break;
	 }
	 
}


/*******************************************************************************
* Function Name  : SendCommandSuccessPacket
* Input          : int32_t
* Output         : None
* Return         : None
* Description    : Sends a COMMAND_COMPLETE packet over the UART transmitter.
*******************************************************************************/
void SendCommandSuccessPacket( int32_t command )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = COMMAND_COMPLETE;
	 NewPacket.length = 1;
	 NewPacket.packet_data[0] = command;
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}


/*******************************************************************************
* Function Name  : SendCommandFailedPacket
* Input          : int32_t
* Output         : None
* Return         : None
* Description    : Sens a COMMAND_FAILED packet over the UART transmitter
*******************************************************************************/
void SendCommandFailedPacket( int32_t command, int32_t flag  )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = COMMAND_FAILED;
	 NewPacket.length = 2;
	 NewPacket.packet_data[0] = (char)command;
	 NewPacket.packet_data[1] = (char)flag;
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}


/*******************************************************************************
* Function Name  : SendDataPacket
* Input          : Uses global data buffers defined in main.c, and a global configuration
						  structure defined in chr6d_config.c
* Output         : None
* Return         : None
* Description    : Sends a packet containing the most recent data from all the 
						  active IMU channels
*******************************************************************************/
void SendDataPacket( )
{
	 USARTPacket NewPacket;
	 int32_t active_channels = 0;
	  
	 NewPacket.PT = SENSOR_DATA;

	 // Convert angle estimates from floats to int16_ts for transmission.
	 // There are 2^15 - 1 possible positive values in a 16-bit integer.  We would
	 // like to split the possible range of possitive angles from 0 to 360 degrees
	 // into 2^15 - 1 angles.  (2^15 - 1)/360 = 91.0194 ticks/degree.  
	 int16_t yaw = (int16_t)(91.0194*gEstimatedStates.yaw);
	 int16_t pitch = (int16_t)(91.0194*gEstimatedStates.pitch);
	 int16_t roll = (int16_t)(91.0194*gEstimatedStates.roll);
	 
	 if( gConfig.psi_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((yaw >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((yaw) & 0x0FF);
		  
		  active_channels++;
	 }
	 if( gConfig.theta_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((pitch >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((pitch) & 0x0FF);
		  
		  active_channels++;
	 }
	 if( gConfig.phi_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((roll >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((roll) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 // Convert angle rate estimates from floats to int16_ts for transmission.
	 // There are 2^16 possible possible values in a 16-bit integer.  We would
	 // like to split the possible range of rates from -450 to 450 degrees
	 // into 2^16 angles.  (2^16)/900 = 72.8178 ticks/degree.  
	 int16_t yaw_rate = (int16_t)(72.8178*gEstimatedStates.yaw_rate);
	 int16_t pitch_rate = (int16_t)(72.8178*gEstimatedStates.pitch_rate);
	 int16_t roll_rate = (int16_t)(72.8178*gEstimatedStates.roll_rate);
	 
	 if( gConfig.psi_dot_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((yaw_rate >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((yaw_rate) & 0x0FF);
		  
		  active_channels++;
	 }
	 if( gConfig.theta_dot_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((pitch_rate >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((pitch_rate) & 0x0FF);
		  
		  active_channels++;
	 }
	 if( gConfig.phi_dot_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((roll_rate >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((roll_rate) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 // Magnetic Sensors
	 if( gConfig.x_mag_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.mag_x >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.mag_x) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 if( gConfig.y_mag_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.mag_y >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.mag_y) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 if( gConfig.z_mag_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.mag_z >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.mag_z) & 0x0FF);
		  
		  active_channels++;
	 }
	 	 
	 
	 // Rate gyros
	 if( gConfig.x_gyro_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.gyro_x >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.gyro_x) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 if( gConfig.y_gyro_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.gyro_y >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.gyro_y) & 0x0FF);
		  
		  active_channels++;		  
	 }
	 
	 if( gConfig.z_gyro_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.gyro_z >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.gyro_z) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 // Accelerometers
	 if( gConfig.x_accel_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.accel_x >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.accel_x) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 if( gConfig.y_accel_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.accel_y >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.accel_y) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 if( gConfig.z_accel_enabled )
	 {
		  NewPacket.packet_data[2*active_channels+2] = (uint8_t)((gSensorData.accel_z >> 8) & 0x0FF);
		  NewPacket.packet_data[2*active_channels+3] = (uint8_t)((gSensorData.accel_z) & 0x0FF);
		  
		  active_channels++;
	 }
	 
	 NewPacket.length = 2*active_channels + 2;
	 
	 NewPacket.packet_data[0] = (gConfig.psi_enabled << 7) | (gConfig.theta_enabled << 6) | (gConfig.phi_enabled << 5)
										  | (gConfig.psi_dot_enabled << 4) | (gConfig.theta_dot_enabled << 3) | (gConfig.phi_dot_enabled << 2)
										  | (gConfig.x_mag_enabled << 1) | (gConfig.y_mag_enabled);
	 NewPacket.packet_data[1] = (gConfig.z_mag_enabled << 7) | (gConfig.x_gyro_enabled << 6) | (gConfig.y_gyro_enabled << 5) | (gConfig.z_gyro_enabled << 4)
										  | (gConfig.x_accel_enabled  << 3) | (gConfig.y_accel_enabled << 2) | (gConfig.z_accel_enabled << 1);
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 // This is the ONLY packet that is not required to send packets using the SendTXPacketSafe() function call.
	 SendTXPacket( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendGyroBiasPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a GYRO_BIAS_REPORT packet
*******************************************************************************/
void SendGyroBiasPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = GYRO_BIAS_REPORT;
	 NewPacket.length = 6;
	 
	 // X gyro bias
	 NewPacket.packet_data[5] = (uint8_t)(gConfig.x_gyro_bias & 0x0FF);
	 NewPacket.packet_data[4] = (uint8_t)((gConfig.x_gyro_bias >> 8) & 0x0FF);
	 
	 // Y gyro bias
	 NewPacket.packet_data[3] = (uint8_t)(gConfig.y_gyro_bias & 0x0FF);
	 NewPacket.packet_data[2] = (uint8_t)((gConfig.y_gyro_bias >> 8) & 0x0FF);
	 
	 // Z gyro bias
	 NewPacket.packet_data[1] = (uint8_t)(gConfig.z_gyro_bias & 0x0FF);
	 NewPacket.packet_data[0] = (uint8_t)((gConfig.z_gyro_bias >> 8) & 0x0FF);
	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}


/*******************************************************************************
* Function Name  : SendAccelBiasPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACCEL_BIAS_REPORT packet
*******************************************************************************/
void SendAccelBiasPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = ACCEL_BIAS_REPORT;
	 NewPacket.length = 6;
	 
	 // X accel bias
	 NewPacket.packet_data[5] = (uint8_t)(gConfig.x_accel_bias & 0x0FF);
	 NewPacket.packet_data[4] = (uint8_t)((gConfig.x_accel_bias >> 8) & 0x0FF);
	 
	 // Y accel bias
	 NewPacket.packet_data[3] = (uint8_t)(gConfig.y_accel_bias & 0x0FF);
	 NewPacket.packet_data[2] = (uint8_t)((gConfig.y_accel_bias >> 8) & 0x0FF);
	 
	 // Z accel bias
	 NewPacket.packet_data[1] = (uint8_t)(gConfig.z_accel_bias & 0x0FF);
	 NewPacket.packet_data[0] = (uint8_t)((gConfig.z_accel_bias >> 8) & 0x0FF);
	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendMagBiasPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACCEL_SCALE_REPORT packet
*******************************************************************************/
void SendMagBiasPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = MAG_BIAS_REPORT;
	 NewPacket.length = 6;
	 
	 // X mag bias
	 NewPacket.packet_data[5] = (uint8_t)(gConfig.x_mag_bias & 0x0FF);
	 NewPacket.packet_data[4] = (uint8_t)((gConfig.x_mag_bias >> 8) & 0x0FF);
	 
	 // Y mag bias
	 NewPacket.packet_data[3] = (uint8_t)(gConfig.y_mag_bias & 0x0FF);
	 NewPacket.packet_data[2] = (uint8_t)((gConfig.y_mag_bias >> 8) & 0x0FF);
	 
	 // Z mag bias
	 NewPacket.packet_data[1] = (uint8_t)(gConfig.z_mag_bias & 0x0FF);
	 NewPacket.packet_data[0] = (uint8_t)((gConfig.z_mag_bias >> 8) & 0x0FF);
	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendActiveChannelPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACTIVE_CHANNEL_REPORT packet
*******************************************************************************/
void SendActiveChannelPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = ACTIVE_CHANNEL_REPORT;
	 NewPacket.length = 2;
	 
	 NewPacket.packet_data[0] = (gConfig.psi_enabled << 7) | (gConfig.theta_enabled << 6) | (gConfig.phi_enabled << 5)
										  | (gConfig.psi_dot_enabled << 4) | (gConfig.theta_dot_enabled << 3) | (gConfig.phi_dot_enabled << 2)
										  | (gConfig.x_mag_enabled << 1) | (gConfig.y_mag_enabled);
	 NewPacket.packet_data[1] = (gConfig.z_mag_enabled << 7) | (gConfig.x_gyro_enabled << 6) | (gConfig.y_gyro_enabled << 5) | (gConfig.z_gyro_enabled << 4)
										  | (gConfig.x_accel_enabled  << 3) | (gConfig.y_accel_enabled << 2) | (gConfig.z_accel_enabled << 1);
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}


/*******************************************************************************
* Function Name  : SendTransmitModePacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a TRANSMIT_MODE_REPORT packet
*******************************************************************************/
void SendTransmitModePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = BROADCAST_MODE_REPORT;
	 NewPacket.length = 2;
	 
	 NewPacket.packet_data[0] = (gConfig.broadcast_rate & 0x0FF);
	 NewPacket.packet_data[1] = (gConfig.broadcast_enabled & 0x01);
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendStatusReportPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a STATUS_REPORT packet.  Sent after self-test has been run.
*******************************************************************************/
void SendStatusReportPacket( uint16_t result )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = STATUS_REPORT;
	 NewPacket.length = 1;
	 
	 NewPacket.packet_data[0] = (result & 0x0FF);
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendMagCalPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a MAG_CAL_REPORT packet.
*******************************************************************************/
void SendMagCalPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = MAG_CAL_REPORT;
	 NewPacket.length = 36;
	 
	 PackageMatrixForTransmit( &NewPacket, &gConfig.mag_cal );
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendMagRefVectorPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a MAG_REF_VECTOR_REPORT packet
*******************************************************************************/
void SendMagRefVectorPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = MAG_REF_VECTOR_REPORT;
	 NewPacket.length = 6;
	 
	 NewPacket.packet_data[0] = (gConfig.mag_ref_vector[2] >> 8) & 0x0FF;
	 NewPacket.packet_data[1] = gConfig.mag_ref_vector[2] & 0x0FF;
	 
	 NewPacket.packet_data[2] = (gConfig.mag_ref_vector[1] >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = gConfig.mag_ref_vector[1] & 0x0FF;
	 
	 NewPacket.packet_data[4] = (gConfig.mag_ref_vector[0] >> 8) & 0x0FF;
	 NewPacket.packet_data[5] = gConfig.mag_ref_vector[0] & 0x0FF;
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendAccelRefVectorPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACCEL_REF_VECTOR_REPORT packet
*******************************************************************************/
void SendAccelRefVectorPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = ACCEL_REF_VECTOR_REPORT;
	 NewPacket.length = 6;
	 
	 NewPacket.packet_data[0] = (gConfig.accel_ref_vector[2] >> 8) & 0x0FF;
	 NewPacket.packet_data[1] = gConfig.accel_ref_vector[2] & 0x0FF;
	 
	 NewPacket.packet_data[2] = (gConfig.accel_ref_vector[1] >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = gConfig.accel_ref_vector[1] & 0x0FF;
	 
	 NewPacket.packet_data[4] = (gConfig.accel_ref_vector[0] >> 8) & 0x0FF;
	 NewPacket.packet_data[5] = gConfig.accel_ref_vector[0] & 0x0FF;
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendAccelAlignmentPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACCEL_ALIGNMENT_REPORT packet
*******************************************************************************/
void SendAccelAlignmentPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = ACCEL_ALIGNMENT_REPORT;
	 NewPacket.length = 36;
	 
	 PackageMatrixForTransmit( &NewPacket, &gConfig.accel_alignment );
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendGyroAlignmentPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a GYRO_ALIGNMENT_REPORT packet.
*******************************************************************************/
void SendGyroAlignmentPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = GYRO_ALIGNMENT_REPORT;
	 NewPacket.length = 36;
	 
	 PackageMatrixForTransmit( &NewPacket, &gConfig.gyro_alignment );
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendStateCovariancePacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a STATE_COVARIANCE_REPORT packet.
*******************************************************************************/
void SendStateCovariancePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = STATE_COVARIANCE_REPORT;
	 NewPacket.length = 36;
	 
	 PackageMatrixForTransmit( &NewPacket, &gEstimatedStates.P );
	 // Temporary - send out a different matrix for analysis
//	 PackageMatrixForTransmit( &NewPacket, &gTempData.P );
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendProcessCovarianceReport
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a PROCESS_COVARIANCE_REPORT packet.
*******************************************************************************/
void SendProcessCovariancePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = PROCESS_COVARIANCE_REPORT;
	 NewPacket.length = 4;
	 
	 fConvert fToInt;
	 
	 fToInt.float_val = gConfig.process_covariance;
	 NewPacket.packet_data[0] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[1] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[2] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = (fToInt.uint32_val) & 0x0FF;	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendMAGCovariancePacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a MAG_COVARIANCE_REPORT packet.
*******************************************************************************/
void SendMAGCovariancePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = MAG_COVARIANCE_REPORT;
	 NewPacket.length = 4;
	 
	 fConvert fToInt;
	 
	 fToInt.float_val = gConfig.mag_covariance;
	 NewPacket.packet_data[0] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[1] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[2] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = (fToInt.uint32_val) & 0x0FF;	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendAccelCovariancePacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a ACCEL_COVARIANCE_REPORT packet.
*******************************************************************************/
void SendAccelCovariancePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = ACCEL_COVARIANCE_REPORT;
	 NewPacket.length = 4;
	 
	 fConvert fToInt;
	 
	 fToInt.float_val = gConfig.accel_covariance;
	 NewPacket.packet_data[0] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[1] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[2] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = (fToInt.uint32_val) & 0x0FF;	 
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendEKFConfigPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a EKF_CONFIG_REPORT packet.
*******************************************************************************/
void SendEKFConfigPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = EKF_CONFIG_REPORT;
	 NewPacket.length = 1;
	 
	 NewPacket.packet_data[0] = gConfig.EKF_config;
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendStartCalPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a START_CAL_REPORT packet.
*******************************************************************************/
void SendStartCalPacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = START_CAL_REPORT;
	 NewPacket.length = 1;
	 
	 NewPacket.packet_data[0] = gConfig.gyro_startup_calibration;
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : SendGyroScalePacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Sends a GYRO_SCALE_REPORT packet.
*******************************************************************************/
void SendGyroScalePacket( )
{
	 USARTPacket NewPacket;
	 
	 NewPacket.PT = GYRO_SCALE_REPORT;
	 NewPacket.length = 12;
	 
	 fConvert fToInt;
	 
	 fToInt.float_val = gConfig.z_gyro_scale;
	 NewPacket.packet_data[0] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[1] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[2] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[3] = (fToInt.uint32_val) & 0x0FF;	 
	 
	 fToInt.float_val = gConfig.y_gyro_scale;
	 NewPacket.packet_data[4] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[5] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[6] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[7] = (fToInt.uint32_val) & 0x0FF;	
	 
	 fToInt.float_val = gConfig.x_gyro_scale;
	 NewPacket.packet_data[8] = (fToInt.uint32_val >> 24) & 0x0FF;
	 NewPacket.packet_data[9] = (fToInt.uint32_val >> 16) & 0x0FF;
	 NewPacket.packet_data[10] = (fToInt.uint32_val >> 8) & 0x0FF;
	 NewPacket.packet_data[11] = (fToInt.uint32_val) & 0x0FF;
	 
	 NewPacket.checksum = ComputeChecksum( &NewPacket );
	 
	 SendTXPacketSafe( &NewPacket );
}

/*******************************************************************************
* Function Name  : PackageMatrixForTransmit( USARTPacket* packet, fmat3x3* matrix )
* Input          : None
* Output         : None
* Return         : None
* Description    : Takes the matrix pointed to by 'matrix' and copies the data
into the packet data section for transmission.
*******************************************************************************/
void PackageMatrixForTransmit( USARTPacket* packet, fmat3x3* matrix )
{
	 fConvert fToInt;
	 int i, j;
	 
	 for (i=0; i < 3; i++)
	 {
		  for (j=0; j < 3; j++)
		  {
				fToInt.float_val = matrix->data[i][j];
				packet->packet_data[4*(3*i+j)] = (fToInt.uint32_val >> 24) & 0x0FF;
				packet->packet_data[4*(3*i+j)+1] = (fToInt.uint32_val >> 16) & 0x0FF;
				packet->packet_data[4*(3*i+j)+2] = (fToInt.uint32_val >> 8) & 0x0FF;
				packet->packet_data[4*(3*i+j)+3] = fToInt.uint32_val & 0x0FF;
		  }
	 }	 
}
