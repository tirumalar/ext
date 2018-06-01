///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, Tadas Baltrusaitis, all rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//     * The software is provided under the terms of this licence stricly for
//       academic, non-commercial, not-for-profit purposes.
//     * Redistributions of source code must retain the above copyright notice, 
//       this list of conditions (licence) and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions (licence) and the following disclaimer 
//       in the documentation and/or other materials provided with the 
//       distribution.
//     * The name of the author may not be used to endorse or promote products 
//       derived from this software without specific prior written permission.
//     * As this software depends on other libraries, the user must adhere to 
//       and keep in place any licencing terms of those libraries.
//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <math.h>

#include <PDM.h>
#include <CLM_utils.h>

#include <iostream>

using namespace CLMTracker;
//===========================================================================

//=============================================================================
// Orthonormalising the 3x3 rotation matrix
void Orthonormalise(cv::Matx33d &R)
{

	cv::SVD svd(R,cv::SVD::MODIFY_A);
  
	// get the orthogonal matrix from the initial rotation matrix
	cv::Mat_<double> X = svd.u*svd.vt;
  
	// This makes sure that the handedness is preserved and no reflection happened
	// by making sure the determinant is 1 and not -1
	cv::Mat_<double> W = Mat_<double>::eye(3,3); 
	double d = determinant(X);
	W(2,2) = determinant(X);
	Mat Rt = svd.u*W*svd.vt;

	Rt.copyTo(R);

}

//===========================================================================
// Clamping the parameter values to be within 3 standard deviations
void PDM::Clamp(cv::Mat_<double>& local_params, Vec6d& params_global, const CLMParameters& parameters)
{
	double n_sigmas = 3;
	cv::MatConstIterator_<double> e_it  = this->eigen_values.begin();
	cv::MatIterator_<double> p_it =  local_params.begin();

	double v;

	// go over all parameters
	for(; p_it != local_params.end(); ++p_it, ++e_it)
	{
		// Work out the maximum value
		v = n_sigmas*sqrt(*e_it);

		// if the values is too extreme clamp it
		if(fabs(*p_it) > v)
		{
			// Dealing with positive and negative cases
			if(*p_it > 0.0)
			{
				*p_it=v;
			}
			else
			{
				*p_it=-v;
			}
		}
	}
	
	// do not let the pose get out of hand
	if(parameters.limit_pose)
	{
		if(params_global[1] > M_PI / 2)
			params_global[1] = M_PI/2;
		if(params_global[1] < -M_PI / 2)
			params_global[1] = -M_PI/2;
		if(params_global[2] > M_PI / 2)
			params_global[2] = M_PI/2;
		if(params_global[2] < -M_PI / 2)
			params_global[2] = -M_PI/2;
		if(params_global[3] > M_PI / 2)
			params_global[3] = M_PI/2;
		if(params_global[3] < -M_PI / 2)
			params_global[3] = -M_PI/2;
	}
	

}
//===========================================================================
// Compute the 3D representation of shape (in object space) using the local parameters
void PDM::CalcShape3D(cv::Mat_<double>& out_shape, const Mat_<double>& p_local)
{
	out_shape.create(mean_shape.rows, mean_shape.cols);
	out_shape = mean_shape + princ_comp*p_local;
}

//===========================================================================
// Get the 2D shape (in image space) from global and local parameters
void PDM::CalcShape2D(Mat_<double>& out_shape, const Mat_<double>& params_local, const Vec6d& params_global) const
{

	int n = this->NumberOfPoints();

	double s = params_global[0]; // scaling factor
	double tx = params_global[4]; // x offset
	double ty = params_global[5]; // y offset

	// get the rotation matrix from the euler angles
	Vec3d euler(params_global[1], params_global[2], params_global[3]);
	Matx33d currRot = Euler2RotationMatrix(euler);
	
	// get the 3D shape of the object
	Mat_<double> Shape_3D = mean_shape + princ_comp * params_local;

	// create the 2D shape matrix (if it has not been defined yet)
	if((out_shape.rows != mean_shape.rows) || (out_shape.cols = 1))
	{
		out_shape.create(2*n,1);
	}
	// for every vertex
	for(int i = 0; i < n; i++)
	{
		// Transform this using the weak-perspective mapping to 2D from 3D
		out_shape.at<double>(i  ,0) = s * ( currRot(0,0) * Shape_3D.at<double>(i, 0) + currRot(0,1) * Shape_3D.at<double>(i+n  ,0) + currRot(0,2) * Shape_3D.at<double>(i+n*2,0) ) + tx;
		out_shape.at<double>(i+n,0) = s * ( currRot(1,0) * Shape_3D.at<double>(i, 0) + currRot(1,1) * Shape_3D.at<double>(i+n  ,0) + currRot(1,2) * Shape_3D.at<double>(i+n*2,0) ) + ty;
	}
}

//===========================================================================
// provided the bounding box of a face and the local parameters (with optional rotation), generates the global parameters that can generate the face with the provided bounding box
// This all assumes that the bounding box describes face from left outline to right outline of the face and chin to eyebrows
void PDM::CalcParams(Vec6d& out_params_global, const Rect_<double>& bounding_box, const Mat_<double>& params_local, const Vec3d rotation)
{
	
	// get the shape instance based on local params
	Mat_<double> current_shape(mean_shape.size());

	CalcShape3D(current_shape, params_local);

	// rotate the shape
	Matx33d rotation_matrix = Euler2RotationMatrix(rotation);

	Mat_<double> reshaped = current_shape.reshape(1, 3);

	Mat rotated_shape = (Mat(rotation_matrix) * reshaped);

	// Get the width of expected shape
	double min_x;
	double max_x;
	cv::minMaxLoc(rotated_shape.row(0), &min_x, &max_x);	

	double min_y;
	double max_y;
	cv::minMaxLoc(rotated_shape.row(1), &min_y, &max_y);

	double width = abs(min_x - max_x);
	double height = abs(min_y - max_y);

	double scaling = ((bounding_box.width / width) + (bounding_box.height / height)) / 2;

	// The estimate of face center also needs some correction
	double tx = bounding_box.x + bounding_box.width / 2;
	double ty = bounding_box.y + bounding_box.height / 2;

	// Correct it so that the bounding box is just around the minimum and maximum point in the initialised face	
	tx = tx - scaling * (min_x + max_x)/2;
    ty = ty - scaling * (min_y + max_y)/2;

	out_params_global = Vec6d(scaling, rotation[0], rotation[1], rotation[2], tx, ty);
}

//===========================================================================
// provided the model parameters, compute the bounding box of a face
// The bounding box describes face from left outline to right outline of the face and chin to eyebrows
void PDM::CalcBoundingBox(Rect& out_bounding_box, const Vec6d& params_global, const Mat_<double>& params_local)
{
	
	// get the shape instance based on local params
	Mat_<double> current_shape;
	CalcShape2D(current_shape, params_local, params_global);
	
	// Get the width of expected shape
	double min_x;
	double max_x;
	cv::minMaxLoc(current_shape(Rect(0, 0, 1, this->NumberOfPoints())), &min_x, &max_x);

	double min_y;
	double max_y;
	cv::minMaxLoc(current_shape(Rect(0, this->NumberOfPoints(), 1, this->NumberOfPoints())), &min_y, &max_y);

	double width = abs(min_x - max_x);
	double height = abs(min_y - max_y);

	out_bounding_box = Rect((int)min_x, (int)min_y, (int)width, (int)height);
}

//===========================================================================
// Calculate the PDM's Jacobian over rigid parameters (rotation, translation and scaling), the additional input W represents trust for each of the landmarks and is part of Non-Uniform RLMS 
void PDM::ComputeRigidJacobian(const Mat_<double>& p_local, const Vec6d& params_global, cv::Mat_<double> &Jacob, const Mat_<double> W, cv::Mat_<double> &Jacob_t_w)
{
  	
	// number of verts
	int n = this->NumberOfPoints();
  
	Jacob.create(n * 2, 6);

	double X,Y,Z;

	double s = params_global[0];
  	
	Mat_<double> shape_3D;

	// Get the current 3D shape (in the object coordinate space)
	this->CalcShape3D(shape_3D, p_local);

	 // Get the rotation matrix
	Vec3d euler(params_global[1], params_global[2], params_global[3]);
	Matx33d currRot = Euler2RotationMatrix(euler);
	
	double r11 = currRot(0,0);
	double r12 = currRot(0,1);
	double r13 = currRot(0,2);
	double r21 = currRot(1,0);
	double r22 = currRot(1,1);
	double r23 = currRot(1,2);
	double r31 = currRot(2,0);
	double r32 = currRot(2,1);
	double r33 = currRot(2,2);

	cv::MatIterator_<double> Jx = Jacob.begin();
	cv::MatIterator_<double> Jy = Jx + n * 6;

	for(int i = 0; i < n; i++)
	{
    
		X = shape_3D.at<double>(i,0);
		Y = shape_3D.at<double>(i+n,0);
		Z = shape_3D.at<double>(i+n*2,0);    
		
		// The rigid jacobian from the axis angle rotation matrix approximation using small angle assumption (R * R')
		// where R' = [1, -wz, wy
		//             wz, 1, -wx
		//             -wy, wx, 1]
		// And this is derived using the small angle assumption on the axis angle rotation matrix parametrisation

		// scaling term
		*Jx++ =  X  * r11 + Y * r12 + Z * r13;
		*Jy++ =  X  * r21 + Y * r22 + Z * r23;
		
		// rotation terms
		*Jx++ = s * (Y * r13 - Z * r12);
		*Jy++ = s * (Y * r23 - Z * r22);
		*Jx++ = -s * (X * r13 - Z * r11);
		*Jy++ = -s * (X * r23 - Z * r21);
		*Jx++ = s * (X * r12 - Y * r11);
		*Jy++ = s * (X * r22 - Y * r21);
		
		// translation terms
		*Jx++ = 1.0;
		*Jy++ = 0.0;
		*Jx++ = 0.0;
		*Jy++ = 1.0;

	}

	Mat Jacob_w = Mat::zeros(Jacob.rows, Jacob.cols, Jacob.type());
	
	Jx =  Jacob.begin();
	Jy =  Jx + n*6;

	cv::MatIterator_<double> Jx_w =  Jacob_w.begin<double>();
	cv::MatIterator_<double> Jy_w =  Jx_w + n*6;

	// Iterate over all Jacobian values and multiply them by the weight in diagonal of W
	for(int i = 0; i < n; i++)
	{
		double w_x = W.at<double>(i, i);
		double w_y = W.at<double>(i+n, i+n);

		for(int j = 0; j < Jacob.cols; ++j)
		{
			*Jx_w++ = *Jx++ * w_x;
			*Jy_w++ = *Jy++ * w_y;
		}		
	}

	Jacob_t_w = Jacob_w.t();
}

//===========================================================================
// Calculate the PDM's Jacobian over all parameters (rigid and non-rigid), the additional input W represents trust for each of the landmarks and is part of Non-Uniform RLMS
void PDM::ComputeJacobian(const Mat_<double>& params_local, const Vec6d& params_global, Mat_<double> &Jacobian, const Mat_<double> W, cv::Mat_<double> &Jacob_t_w)
{ 
	
	// number of vertices
	int n = this->NumberOfPoints();
		
	// number of non-rigid parameters
	int m = this->NumberOfModes();

	Jacobian.create(n * 2, 6 + m);
	
	double X,Y,Z;
	
	double s = params_global[0];
  	
	Mat_<double> shape_3D;

	this->CalcShape3D(shape_3D, params_local);
	
	Vec3d euler(params_global[1], params_global[2], params_global[3]);
	Matx33d currRot = Euler2RotationMatrix(euler);
	
	double r11 = currRot(0,0);
	double r12 = currRot(0,1);
	double r13 = currRot(0,2);
	double r21 = currRot(1,0);
	double r22 = currRot(1,1);
	double r23 = currRot(1,2);
	double r31 = currRot(2,0);
	double r32 = currRot(2,1);
	double r33 = currRot(2,2);

	cv::MatIterator_<double> Jx =  Jacobian.begin();
	cv::MatIterator_<double> Jy =  Jx + n * (6 + m);
	cv::MatConstIterator_<double> Vx =  this->princ_comp.begin();
	cv::MatConstIterator_<double> Vy =  Vx + n*m;
	cv::MatConstIterator_<double> Vz =  Vy + n*m;

	for(int i = 0; i < n; i++)
	{
    
		X = shape_3D.at<double>(i,0);
		Y = shape_3D.at<double>(i+n,0);
		Z = shape_3D.at<double>(i+n*2,0);    
    
		// The rigid jacobian from the axis angle rotation matrix approximation using small angle assumption (R * R')
		// where R' = [1, -wz, wy
		//             wz, 1, -wx
		//             -wy, wx, 1]
		// And this is derived using the small angle assumption on the axis angle rotation matrix parametrisation

		// scaling term
		*Jx++ =  X  * r11 + Y * r12 + Z * r13;
		*Jy++ =  X  * r21 + Y * r22 + Z * r23;
		
		// rotation terms
		*Jx++ = s * (Y * r13 - Z * r12);
		*Jy++ = s * (Y * r23 - Z * r22);
		*Jx++ = -s * (X * r13 - Z * r11);
		*Jy++ = -s * (X * r23 - Z * r21);
		*Jx++ = s * (X * r12 - Y * r11);
		*Jy++ = s * (X * r22 - Y * r21);
		
		// translation terms
		*Jx++ = 1.0;
		*Jy++ = 0.0;
		*Jx++ = 0.0;
		*Jy++ = 1.0;

		for(int j = 0; j < m; j++,++Vx,++Vy,++Vz)
		{
			// How much the change of the non-rigid parameters (when object is rotated) affect 2D motion
			*Jx++ = s*(r11*(*Vx) + r12*(*Vy) + r13*(*Vz));
			*Jy++ = s*(r21*(*Vx) + r22*(*Vy) + r23*(*Vz));
		}
	}	

	// Adding the weights here
	Mat Jacob_w = Jacobian.clone();
	
	if(cv::trace(W)[0] != W.rows) 
	{
		Jx =  Jacobian.begin();
		Jy =  Jx + n*(6+m);

		cv::MatIterator_<double> Jx_w =  Jacob_w.begin<double>();
		cv::MatIterator_<double> Jy_w =  Jx_w + n*(6+m);

		// Iterate over all Jacobian values and multiply them by the weight in diagonal of W
		for(int i = 0; i < n; i++)
		{
			double w_x = W.at<double>(i, i);
			double w_y = W.at<double>(i+n, i+n);

			for(int j = 0; j < Jacobian.cols; ++j)
			{
				*Jx_w++ = *Jx++ * w_x;
				*Jy_w++ = *Jy++ * w_y;
			}
		}
	}
	Jacob_t_w = Jacob_w.t();

}

//===========================================================================
// Updating the parameters (more details in my thesis)
void PDM::UpdateModelParameters(const Mat_<double>& delta_p, Mat_<double>& params_local, Vec6d& params_global)
{
	// The scaling and translation parameters can be just added
	params_global[0] += delta_p.at<double>(0,0);
	params_global[4] += delta_p.at<double>(4,0);
	params_global[5] += delta_p.at<double>(5,0);

	// get the original rotation matrix	
	Vec3d eulerGlobal(params_global[1], params_global[2], params_global[3]);
	Matx33d R1 = Euler2RotationMatrix(eulerGlobal);

	// construct R' = [1, -wz, wy
	//               wz, 1, -wx
	//               -wy, wx, 1]
	Matx33d R2 = Matx33d::eye();

	R2(1,2) = -1.0*(R2(2,1) = delta_p.at<double>(1,0));
	R2(2,0) = -1.0*(R2(0,2) = delta_p.at<double>(2,0));
	R2(0,1) = -1.0*(R2(1,0) = delta_p.at<double>(3,0));
	
	// Make sure it's orthonormal
	Orthonormalise(R2);

	// Combine rotations
	Matx33d R3 = R1 *R2;

	// Extract euler angle (through axis angle first to make sure it's legal)
	Vec3d axis_angle = RotationMatrix2AxisAngle(R3);	
	Vec3d euler = AxisAngle2Euler(axis_angle);

	params_global[1] = euler[0];
	params_global[2] = euler[1];
	params_global[3] = euler[2];

	// Local parameter update, just simple addition
	if(delta_p.rows > 6)
	{
		params_local += delta_p(cv::Rect(0,6,1, this->NumberOfModes()));
	}
}

void PDM::Read(string location)
{
  	
	ifstream pdmLoc(location.c_str());

	CLMTracker::SkipComments(pdmLoc);

	// Reading mean values
	CLMTracker::ReadMat(pdmLoc,mean_shape);
	
	CLMTracker::SkipComments(pdmLoc);

	// Reading principal components
	CLMTracker::ReadMat(pdmLoc,princ_comp);
	
	CLMTracker::SkipComments(pdmLoc);
	
	// Reading eigenvalues	
	CLMTracker::ReadMat(pdmLoc,eigen_values);

}
