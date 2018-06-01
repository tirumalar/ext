/* ------------------------------------------------------------------------------
  File: chr6dm_states.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Function definitions for CHR-6dm state estimation.
------------------------------------------------------------------------------ */ 

#include <math.h>
#include "chr6dm_states.h"
#include "matrix3x3.h"
#include "chr6dm_config.h"

// Data structures for holding sensor data and estimated states.
RawSensorData gSensorData;
AHRS_states gEstimatedStates;

/*******************************************************************************
* Function Name  : EKF_Init
* Input          : None
* Output         : Pre-initialized state estimate structure
* Return         : None
* Description    : Fills an AHRS_states structure with zero initial values.
*******************************************************************************/
void EKF_Init( AHRS_states* estimated_states )
{
	 int i, j;
	 
	 estimated_states->phi = 0;
	 estimated_states->theta = 0;
	 estimated_states->psi = 0;
	 
	 estimated_states->phi_dot = 0;
	 estimated_states->theta_dot = 0;
	 estimated_states->psi_dot = 0;
	 
	 estimated_states->P.data[0][0] = .01;
	 estimated_states->P.data[1][1] = .01;
	 estimated_states->P.data[1][1] = .01;	 
	 
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++)
		  {
				gEstimatedStates.Q.data[i][j] = 0;
				gEstimatedStates.Racc.data[i][j] = 0;
				gEstimatedStates.Rmag.data[i][j] = 0;
				gEstimatedStates.P.data[i][j] = 0;
				
				if ( i == j )
				{
					 gEstimatedStates.P.data[i][j] = .1;
				}
		  }
	 }
 
	 // Set process noise matrix
	 gEstimatedStates.Q.data[0][0] = gConfig.process_covariance;
	 gEstimatedStates.Q.data[1][1] = gConfig.process_covariance;
	 gEstimatedStates.Q.data[2][2] = gConfig.process_covariance;
	 
	 // Set measurement noise matrices
	 gEstimatedStates.Racc.data[0][0] = gConfig.accel_covariance;
	 gEstimatedStates.Racc.data[1][1] = gConfig.accel_covariance;
	 gEstimatedStates.Racc.data[2][2] = gConfig.accel_covariance;
	 
	 gEstimatedStates.Rmag.data[0][0] = gConfig.mag_covariance;
	 gEstimatedStates.Rmag.data[1][1] = gConfig.mag_covariance;
	 gEstimatedStates.Rmag.data[2][2] = gConfig.mag_covariance;
}

/*******************************************************************************
* Function Name  : EKF_InitFromSensors
* Input          : AHRS_states* estimated_states, RawSensorData* sensor_data
* Output         : None
* Return         : None
* Description    : Uses sensor data to fill initial state estimate structure.
						 Specifically, accels are used to calculate an initial pitch and
						 roll guess, while the magnetomer is used to compute an initial
						 heading guess.
						 Gyro outputs are copied directly into estimated angular rate states.
*******************************************************************************/
void EKF_InitFromSensors( AHRS_states* estimated_states, RawSensorData* sensor_data )
{
	 int i, j;
	 
	 // Compute initial roll and pitch angles
	 float theta_a = -(180/3.14159)*atan2( (float)sensor_data->accel_x, (float)sensor_data->accel_z );
	 float phi_a = -(180/3.14159)*atan2( (float)sensor_data->accel_y, (float)sensor_data->accel_z );
	 
	 estimated_states->phi = phi_a;
	 estimated_states->theta = theta_a;
	 estimated_states->psi = 0;
	 
	 estimated_states->phi_dot = 0;
	 estimated_states->theta_dot = 0;
	 estimated_states->psi_dot = 0;
	 
	 estimated_states->P.data[0][0] = .01;
	 estimated_states->P.data[1][1] = .01;
	 estimated_states->P.data[1][1] = .01;
	 
	 for (i = 0; i < 3; i++ )
	 {
		  for (j = 0; j < 3; j++)
		  {
				gEstimatedStates.Q.data[i][j] = 0;
				gEstimatedStates.Racc.data[i][j] = 0;
				gEstimatedStates.Rmag.data[i][j] = 0;
		  }
	 }
 
	 // Set process noise matrix
	 gEstimatedStates.Q.data[0][0] = gConfig.process_covariance;
	 gEstimatedStates.Q.data[1][1] = gConfig.process_covariance;
	 gEstimatedStates.Q.data[2][2] = gConfig.process_covariance;
	 
	 // Set measurement noise matrices
	 gEstimatedStates.Racc.data[0][0] = gConfig.accel_covariance;
	 gEstimatedStates.Racc.data[1][1] = gConfig.accel_covariance;
	 gEstimatedStates.Racc.data[2][2] = gConfig.accel_covariance;
	 
	 gEstimatedStates.Rmag.data[0][0] = gConfig.mag_covariance;
	 gEstimatedStates.Rmag.data[1][1] = gConfig.mag_covariance;
	 gEstimatedStates.Rmag.data[2][2] = gConfig.mag_covariance;
}


/*******************************************************************************
* Function Name  : EKF_EstimateStates
* Input          : AHRS_states* estimated_states, RawSensorData* sensor_data
* Output         : None
* Return         : None
* Description    : 
*******************************************************************************/
void EKF_EstimateStates( AHRS_states* estimated_states, RawSensorData* sensor_data )
{
	 static int initialized = 0;
	 
	 if( !initialized )
	 {
//		  EKF_InitFromSensors( estimated_states, sensor_data );
		  EKF_Init( estimated_states );
		  initialized = 1;
	 }
	 
	 // Run EKF prediction step
	 EKF_Predict( estimated_states, sensor_data );
	 
	 // Run EKF update step
	 EKF_Update( estimated_states, sensor_data );
}

/*******************************************************************************
* Function Name  : EKF_Predict
* Input          : AHRS_states* estimated_states, RawSensorData* sensor_data
* Output         : None
* Return         : None
* Description    : EKF prediction step.  Uses rate gyros to make new orientation
						 estimate.
*******************************************************************************/
void EKF_Predict( AHRS_states* estimated_states, RawSensorData* sensor_data )
{
	 // roll, pitch, yaw
	 float phi, theta, psi;
	 float phi_dot, theta_dot, psi_dot;
	 float p, q, r;
	 float T;
	 uint16_t timer_value;
	 
	 fmat3x3 A, A_transpose, AP, PA_transpose, temp;
	 
	 int32_t i;
	 
	 // Multiply gyro outputs by gyro alignment matrix
	 fvect3x1 pqr;
	 pqr.data[0] = gConfig.x_gyro_scale*(float)sensor_data->gyro_x;
	 pqr.data[1] = gConfig.y_gyro_scale*(float)sensor_data->gyro_y;
	 pqr.data[2] = gConfig.z_gyro_scale*(float)sensor_data->gyro_z;
	 
	 MatVectMult3( &gConfig.gyro_alignment, &pqr, &pqr );
	 
	 // Retrieve angular rates from vector
	 p = pqr.data[0];
	 q = pqr.data[1];
	 r = pqr.data[2];

	 // Get current state estimates
	 phi = estimated_states->phi;
	 theta = estimated_states->theta;
	 psi = estimated_states->psi;
	 
	 // Get elapsed time since last prediction (Timer3 should be configured
	 // to increment once every microsecond.  It is a 16-bit timer, which means
	 // that a maximum of 2^16 = 65536 microseconds can pass before overflow.
	 // The prediction step should thus be run at least once every 65 milliseconds (15.6 Hz),
	 // but preferably more quickly.  This shouldn't be a problem - the prediction step
	 // should nominally run at roughly 400 Hz or greater).
	 TIM_Cmd(TIM3, DISABLE);
	 timer_value = TIM_GetCounter(TIM3);
	 TIM_SetCounter(TIM3,0);
	 TIM_Cmd(TIM3, ENABLE);
	 
	 T = (float)(0.000001)*(float)timer_value;
	 
	 float cos_phi = cos(phi*3.14159/180);
	 float tan_theta = tan(theta*3.14159/180);
	 float sin_phi = sin(phi*3.14159/180);
	 float cos_theta = cos(theta*3.14159/180);
	 float sin_theta = sin(theta*3.14159/180);
	 
	 // Compute expected angle rates based on gyro outputs.  Note that to do this, the measured rotations
	 // must be transformed into the inertial frame (we can't just integrate the gyro outputs).
	 phi_dot = p + sin_phi*tan_theta*q + cos_phi*tan_theta*r;
	 theta_dot = cos_phi*q - sin_phi*r;
	 psi_dot = (sin_phi/cos_theta)*q + (cos_phi/cos_theta)*r;
	 
	 estimated_states->phi_dot = phi_dot;
	 estimated_states->theta_dot = theta_dot;
	 estimated_states->psi_dot = psi_dot;

	 // Compute new angle estimates
	 phi = phi + T*phi_dot;
	 theta = theta + T*theta_dot;
	 psi = psi + T*psi_dot;
	 
	 // Linearize propogation equation for covariance estimation (build the matrix A)
	 /*
	 [     q*cos(phi)*tan(theta) - r*sin(phi)*tan(theta),               r*cos(phi)*(tan(theta)^2 + 1) + q*sin(phi)*(tan(theta)^2 + 1), 0]
	 [                         - r*cos(phi) - q*sin(phi),                                                                           0, 0]
	 [ (q*cos(phi))/cos(theta) - (r*sin(phi))/cos(theta), (r*cos(phi)*sin(theta))/cos(theta)^2 + (q*sin(phi)*sin(theta))/cos(theta)^2, 0]
	 */
	 
//	 A.data[0][0] = q*cos(phi)*tan(theta) - r*sin(phi)*tan(theta);
	 A.data[0][0] = q*cos_phi*tan_theta - r*sin_phi*tan_theta;
	 
//	 A.data[0][1] = r*cos(phi)*(tan(theta)^2 + 1) + q*sin(phi)*(tan(theta)^2 + 1);
	 A.data[0][1] = r*cos_phi*(tan_theta*tan_theta + 1) + q*sin_phi*(tan_theta*tan_theta + 1);
	 
	 A.data[0][2] = 0;
	 
//	 A.data[1][0] = -r*cos(phi) - q*sin(phi);
	 A.data[1][0] = -r*cos_phi - q*sin_phi;
	 
	 A.data[1][1] = 0;
	 
	 A.data[1][2] = 0;
	 
//	 A.data[2][0] = (q*cos(phi))/cos(theta) - (r*sin(phi))/cos(theta);
	 A.data[2][0] = (q*cos_phi)/cos_theta - (r*sin_phi)/cos_theta;
	 
//	 A.data[2][1] = (r*cos(phi)*sin(theta))/cos(theta)^2 + (q*sin(phi)*sin(theta))/cos(theta)^2;
	 A.data[2][1] = (r*cos_phi*sin_theta)/(cos_theta*cos_theta) + (q*sin_phi*sin_theta)/(cos_theta*cos_theta);
	 
	 A.data[2][2] = 0;
	 
	 // Compute new covariance: P = P + T*(AP + PA^T + Q)
	 MatMult3x3( &A, &estimated_states->P, &AP );
	 MatTrans3x3( &A, &A_transpose );
	 MatMult3x3( &estimated_states->P, &A_transpose, &PA_transpose );
	 MatAdd3x3( &AP, &PA_transpose, &temp );
	 MatAdd3x3( &temp, &estimated_states->Q, &temp );
	 ScalarMatMult3x3( T, &temp, &temp );
	 MatAdd3x3( &temp, &estimated_states->P, &estimated_states->P );

	 
	 // Update states estimates in data structure
	 estimated_states->phi = phi;
	 estimated_states->theta = theta;
	 estimated_states->psi = psi;
}

/*******************************************************************************
* Function Name  : EKF_Update
* Input          : AHRS_states* estimated_states, RawSensorData* sensor_data
* Output         : None
* Return         : None
* Description    : EKF update step.  Uses accels to correct pitch and roll errors,
						 and magnetic sensors to correct yaw errors.  Compensation is
						 only applied when new data is available, as specified by the
						 new_mag_data and new_accel_data flags in the sensor_data structure.
*******************************************************************************/
void EKF_Update( AHRS_states* estimated_states, RawSensorData* sensor_data )
{
	 fmat3x3 R, L, C, Ctrans, PCtrans, LC, temp, I;
	 
	 CreateIdentity3x3( &I );
	 
	 // If new accelerometer data is available, perform update
	 if( sensor_data->new_accel_data && (gConfig.EKF_config & ACCEL_UPDATE_ENABLED) )
	 {
		  sensor_data->new_accel_data = 0;
		  
		  float theta = estimated_states->theta;
		  float phi = estimated_states->phi;
		  float psi = estimated_states->psi;
		  
		  float cos_phi = cos(phi*3.14159/180);
		  float cos_theta = cos(theta*3.14159/180);
		  float cos_psi = cos(psi*3.14159/180);
		  
		  float sin_phi = sin(phi*3.14159/180);
		  float sin_theta = sin(theta*3.14159/180);
		  float sin_psi = sin(psi*3.14159/180);
		  
		  // Build rotation matrix from inertial frame to body frame (for computing expected sensor outputs given yaw, pitch, and roll angles)
		  /*
				[                              cos(psi)*cos(theta),                              cos(theta)*sin(psi),         -sin(theta)]
				[ cos(psi)*sin(phi)*sin(theta) - cos(phi)*sin(psi), cos(phi)*cos(psi) + sin(phi)*sin(psi)*sin(theta), cos(theta)*sin(phi)]
				[ sin(phi)*sin(psi) + cos(phi)*cos(psi)*sin(theta), cos(phi)*sin(psi)*sin(theta) - cos(psi)*sin(phi), cos(phi)*cos(theta)]
		  */
		  R.data[0][0] = cos_psi*cos_theta;
		  R.data[0][1] = cos_theta*sin_psi;
		  R.data[0][2] = -sin_theta;
		  
		  R.data[1][0] = cos_psi*sin_phi*sin_theta - cos_phi*sin_psi;
		  R.data[1][1] = cos_phi*cos_psi + sin_phi*sin_psi*sin_theta;
		  R.data[1][2] = cos_theta*sin_phi;
		  
		  R.data[2][0] = sin_phi*sin_psi + cos_phi*cos_psi*sin_theta;
		  R.data[2][1] = cos_phi*sin_psi*sin_theta - cos_psi*sin_phi;
		  R.data[2][2] = cos_phi*cos_theta;
		  
		  fvect3x1 accel_ref;
		  accel_ref.data[0] = gConfig.accel_ref_vector[0];
		  accel_ref.data[1] = gConfig.accel_ref_vector[1];
		  accel_ref.data[2] = gConfig.accel_ref_vector[2];
		  
		  // Subtract accel bias vector
		  accel_ref.data[0] = accel_ref.data[0] - (float)gConfig.x_accel_bias;
		  accel_ref.data[1] = accel_ref.data[1] - (float)gConfig.y_accel_bias;
		  accel_ref.data[2] = accel_ref.data[2] - (float)gConfig.z_accel_bias;
		  
		  // Multiply accel reference vector by calibration matrix
		  MatVectMult3( &gConfig.accel_alignment, &accel_ref, &accel_ref );
		  
		  float ax = accel_ref.data[0];
		  float ay = accel_ref.data[1];
		  float az = accel_ref.data[2];
		  
		  fvect3x1 acc_hat;
		  	  
		  // Compute expected accelerometer output based on yaw, pitch, and roll angles
		  MatVectMult3( &R, &accel_ref, &acc_hat );
		  
		  // Compute C matrix for Kalman gain calculation
		  /*
		  [                                                                                                                                      0,                          - az*cos(theta) - ax*cos(psi)*sin(theta) - ay*sin(psi)*sin(theta),                                                                 ay*cos(psi)*cos(theta) - ax*cos(theta)*sin(psi)]
		  [ ax*(sin(phi)*sin(psi) + cos(phi)*cos(psi)*sin(theta)) - ay*(cos(psi)*sin(phi) - cos(phi)*sin(psi)*sin(theta)) + az*cos(phi)*cos(theta), ax*cos(psi)*cos(theta)*sin(phi) - az*sin(phi)*sin(theta) + ay*cos(theta)*sin(phi)*sin(psi), - ax*(cos(phi)*cos(psi) + sin(phi)*sin(psi)*sin(theta)) - ay*(cos(phi)*sin(psi) - cos(psi)*sin(phi)*sin(theta))]
		  [ ax*(cos(phi)*sin(psi) - cos(psi)*sin(phi)*sin(theta)) - ay*(cos(phi)*cos(psi) + sin(phi)*sin(psi)*sin(theta)) - az*cos(theta)*sin(phi), ax*cos(phi)*cos(psi)*cos(theta) - az*cos(phi)*sin(theta) + ay*cos(phi)*cos(theta)*sin(psi),   ax*(cos(psi)*sin(phi) - cos(phi)*sin(psi)*sin(theta)) + ay*(sin(phi)*sin(psi) + cos(phi)*cos(psi)*sin(theta))]
		  */
		  C.data[0][0] = 0;
		  C.data[0][1] = -az*cos_theta - ax*cos_psi*sin_theta - ay*sin_psi*sin_theta;
		  C.data[0][2] = ay*cos_psi*cos_theta - ax*cos_theta*sin_psi;
		  
		  C.data[1][0] = ax*(sin_phi*sin_psi + cos_phi*cos_psi*sin_theta) - ay*(cos_psi*sin_phi - cos_phi*sin_psi*sin_theta) + az*cos_phi*cos_theta;
		  C.data[1][1] = ax*cos_psi*cos_theta*sin_phi - az*sin_phi*sin_theta + ay*cos_theta*sin_phi*sin_psi;
		  C.data[1][2] = -ax*(cos_phi*cos_psi + sin_phi*sin_psi*sin_theta) - ay*(cos_phi*sin_psi - cos_psi*sin_phi*sin_theta);
		  
		  C.data[2][0] = ax*(cos_phi*sin_psi - cos_psi*sin_phi*sin_theta) - ay*(cos_phi*cos_psi + sin_phi*sin_psi*sin_theta) - az*cos_theta*sin_phi;
		  C.data[2][1] = ax*cos_phi*cos_psi*cos_theta - az*cos_phi*sin_theta + ay*cos_phi*cos_theta*sin_psi;
		  C.data[2][2] = ax*(cos_psi*sin_phi - cos_phi*sin_psi*sin_theta) + ay*(sin_phi*sin_psi + cos_phi*cos_psi*sin_theta);
		  
		  // Compute Kalman gain: L = PC^T * (R + CPC^T)^(-1)
		  MatTrans3x3( &C, &Ctrans );
		  MatMult3x3( &estimated_states->P, &Ctrans, &PCtrans );
		  MatMult3x3( &C, &PCtrans, &temp );
		  MatAdd3x3( &temp, &estimated_states->Racc, &temp );
		  MatInv3x3( &temp, &temp );
		  MatMult3x3( &PCtrans, &temp, &L );		

		  // Compute new covariance
		  MatMult3x3( &L, &C, &LC );
		  ScalarMatMult3x3( -1, &LC, &temp );
		  MatAdd3x3( &I, &temp, &temp );
		  MatMult3x3( &temp, &estimated_states->P, &estimated_states->P );
		  
		  fvect3x1 acc_vect, correction;
		  acc_vect.data[0] = sensor_data->accel_x;
		  acc_vect.data[1] = sensor_data->accel_y;
		  acc_vect.data[2] = sensor_data->accel_z;
		  
		  // Subtract accel bias
		  acc_vect.data[0] = acc_vect.data[0] - (float)gConfig.x_accel_bias;
		  acc_vect.data[1] = acc_vect.data[1] - (float)gConfig.y_accel_bias;
		  acc_vect.data[2] = acc_vect.data[2] - (float)gConfig.z_accel_bias;
		  
		  // Apply alignment correction
		  MatVectMult3( &gConfig.accel_alignment, &acc_vect, &acc_vect );
		  
		  // Now subtract the reference vector
		  acc_vect.data[0] = acc_vect.data[0] - acc_hat.data[0];
		  acc_vect.data[1] = acc_vect.data[1] - acc_hat.data[1];
		  acc_vect.data[2] = acc_vect.data[2] - acc_hat.data[2];
		  
		  // Multiply by Kalman gain
		  MatVectMult3( &L, &acc_vect, &correction );
		  
		  // Apply correction
		  estimated_states->phi = estimated_states->phi + correction.data[0];
		  estimated_states->theta = estimated_states->theta + correction.data[1];
		  estimated_states->psi = estimated_states->psi + correction.data[2];
		  
		  // "Unroll" angle estimates to be in the range from -360 to 360 degrees
		  unroll_states( estimated_states );
	 }
	 
	 // If new magnetometer data is available, perform update
	 if( sensor_data->new_mag_data && (gConfig.EKF_config & MAG_UPDATE_ENABLED) )
	 {
		  sensor_data->new_mag_data = 0;
		  
		  float theta = estimated_states->theta;
		  float phi = estimated_states->phi;
		  float psi = estimated_states->psi;
		  
		  float cos_phi = cos(phi*3.14159/180);
		  float cos_theta = cos(theta*3.14159/180);
		  float cos_psi = cos(psi*3.14159/180);
		  
		  float sin_phi = sin(phi*3.14159/180);
		  float sin_theta = sin(theta*3.14159/180);
		  float sin_psi = sin(psi*3.14159/180);
		  
		  // Compute magnetic field reference vector
		  fvect3x1 mag_ref;
		  mag_ref.data[0] = (float)gConfig.mag_ref_vector[0];
		  mag_ref.data[1] = (float)gConfig.mag_ref_vector[1];
		  mag_ref.data[2] = (float)gConfig.mag_ref_vector[2];
		  
		  
		  // Subtract magnetic field bias vector
		  mag_ref.data[0] = mag_ref.data[0] - (float)gConfig.x_mag_bias;
		  mag_ref.data[1] = mag_ref.data[1] - (float)gConfig.y_mag_bias;
		  mag_ref.data[2] = mag_ref.data[2] - (float)gConfig.z_mag_bias;
		  
		  // Apply mag. calibration matrix to mag reference vector
		  MatVectMult3( &gConfig.mag_cal, &mag_ref, &mag_ref );
		  
		  // Compute C based on magnetic field data in inertial frame
		  float mx = mag_ref.data[0];
		  float my = mag_ref.data[1];
		  float mz = mag_ref.data[2];
		  
		  C.data[0][0] = 0;
		  C.data[0][1] = 0;
		  C.data[0][2] = my*cos_psi - mx*sin_psi;
		  
		  C.data[1][0] = 0;
		  C.data[1][1] = 0;
		  C.data[1][2] = -mx*cos_psi - my*sin_psi;
		  
		  C.data[2][0] = 0;
		  C.data[2][1] = 0;
		  C.data[2][2] = 0;
		  
		  // Rotate reference vector into vehicle-1 frame
		  R.data[0][0] = cos_psi;
		  R.data[0][1] = sin_psi;
		  R.data[0][2] = 0;
		  
		  R.data[1][0] = -sin_psi;
		  R.data[1][1] = cos_psi;
		  R.data[1][2] = 0;
		  
		  R.data[2][0] = 0;
		  R.data[2][1] = 0;
		  R.data[2][2] = 1;
		  
		  MatVectMult3( &R, &mag_ref, &mag_ref );		  
		  
		  fvect3x1 mag_vect, correction;
		  // Get magnetic field measurement
		  mag_vect.data[0] = (float)sensor_data->mag_x;
		  mag_vect.data[1] = (float)sensor_data->mag_y;
		  mag_vect.data[2] = (float)sensor_data->mag_z;
		  
		  
		  // Subtract magnetic field bias
		  mag_vect.data[0] = mag_vect.data[0] - (float)gConfig.x_mag_bias;
		  mag_vect.data[1] = mag_vect.data[1] - (float)gConfig.y_mag_bias;
		  mag_vect.data[2] = mag_vect.data[2] - (float)gConfig.z_mag_bias;
		  
		  // Apply mag calibration matrix to sensor data
		  MatVectMult3( &gConfig.mag_cal, &mag_vect, &mag_vect );
		  
		  
		  // Build rotation matrix from body frame to vehicle-1 frame (ie. only yaw remains uncorrected)
		  /*
		  [  cos(theta), sin(phi)*sin(theta), cos(phi)*sin(theta)]
		  [           0,            cos(phi),           -sin(phi)]
		  [ -sin(theta), cos(theta)*sin(phi), cos(phi)*cos(theta)]
		  */
		  R.data[0][0] = cos_theta;
		  R.data[0][1] = sin_phi*sin_theta;
		  R.data[0][2] = cos_phi*sin_theta;
		  
		  R.data[1][0] = 0;
		  R.data[1][1] = cos_phi;
		  R.data[1][2] = -sin_phi;
		  
		  R.data[2][0] = -sin_theta;
		  R.data[2][1] = cos_theta*sin_phi;
		  R.data[2][2] = cos_phi*cos_theta;
		  
		  // Rotate measurement into vehicle-1 frame
		  MatVectMult3( &R, &mag_vect, &mag_vect );

		  // Subtract reference vector
		  mag_vect.data[0] = mag_vect.data[0] - mag_ref.data[0];
		  mag_vect.data[1] = mag_vect.data[1] - mag_ref.data[1];
		  mag_vect.data[2] = mag_vect.data[2] - mag_ref.data[2];
		  
		  // Compute Kalman gain: L = PC^T * (R + CPC^T)^(-1)
		  MatTrans3x3( &C, &Ctrans );
		  MatMult3x3( &estimated_states->P, &Ctrans, &PCtrans );
		  MatMult3x3( &C, &PCtrans, &temp );
		  MatAdd3x3( &temp, &estimated_states->Rmag, &temp );
		  MatInv3x3( &temp, &temp );
		  MatMult3x3( &PCtrans, &temp, &L );
		  
		  // Compute new covariance
		  MatMult3x3( &L, &C, &LC );
		  ScalarMatMult3x3( -1, &LC, &temp );
		  MatAdd3x3( &I, &temp, &temp );
		  MatMult3x3( &temp, &estimated_states->P, &estimated_states->P );
		  
		  // Perform state update
		  MatVectMult3( &L, &mag_vect, &correction );
				  
		  estimated_states->phi = estimated_states->phi + correction.data[0];
		  estimated_states->theta = estimated_states->theta + correction.data[1];
		  estimated_states->psi = estimated_states->psi + correction.data[2];
		  
		  // "Unroll" angle estimates to be in the range from -360 to 360 degrees
		  unroll_states( estimated_states );
	 }
}

/*******************************************************************************
* Function Name  : unroll_states
* Input          : AHRS_states* states
* Output         : None
* Return         : None
* Description    : Keeps all angle estimates in the range of -360 to 360 degrees
*******************************************************************************/
void unroll_states( AHRS_states* states )
{
	 while( states->phi > 360 )
	 {
		  states->phi -= 360;
	 }
	 while( states->phi < -360 )
	 {
		  states->phi += 360;
	 }
	 
	 while( states->theta > 360 )
	 {
		  states->theta -= 360;
	 }
	 while( states->theta < -360 )
	 {
		  states->theta += 360;
	 }
	 
	 while( states->psi > 360 )
	 {
		  states->psi -= 360;
	 }
	 while( states->psi < -360 )
	 {
		  states->psi += 360;
	 }
	 
}