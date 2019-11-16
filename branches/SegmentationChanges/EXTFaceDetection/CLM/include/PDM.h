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

#ifndef __PDM_h_
#define __PDM_h_

#include <opencv/cv.h>

#include "CLMParameters.h"

using namespace cv;

namespace CLMTracker
{
//===========================================================================
// A linear 3D Point Distribution Model (constructed using Non-Rigid structure from motion or PCA)
// Only describes the model but does not contain an instance of it (no local or global parameters are stored here)
// Contains the utility functions to help manipulate the model

class PDM{
	public:    
    
		// The 3D mean shape vector of the PDM [x1,..,xn,y1,...yn,z1,...,zn]
		cv::Mat_<double> mean_shape;	
  
		// Principal components or variation bases of the model, 
		cv::Mat_<double> princ_comp;	

		// Eigenvalues (variances) corresponding to the bases
		cv::Mat_<double> eigen_values;	

		PDM(){;}
		
		// A copy constructor
		PDM(const PDM& other){
			
			// Make sure the matrices are allocated properly
			this->mean_shape = other.mean_shape.clone();
			this->princ_comp = other.princ_comp.clone();
			this->eigen_values = other.eigen_values.clone();
		}
			
		void Read(string location);

		// Number of vertices
		inline int NumberOfPoints() const {return mean_shape.rows/3;}
		
		// Listing the number of modes of variation
		inline int NumberOfModes() const {return princ_comp.cols;}

		void Clamp(Mat_<double>& params_local, Vec6d& params_global, const CLMParameters& params);

		// Compute shape in object space (3D)
		void CalcShape3D(Mat_<double>& out_shape, const Mat_<double>& params_local);

		// Compute shape in image space (2D)
		void CalcShape2D(Mat_<double>& out_shape, const Mat_<double>& params_local, const Vec6d& params_global) const;
    
		// provided the bounding box of a face and the local parameters (with optional rotation), generates the global parameters that can generate the face with the provided bounding box
		void CalcParams(Vec6d& out_params_global, const Rect_<double>& bounding_box, const Mat_<double>& params_local, const Vec3d rotation = Vec3d(0.0));

		// provided the model parameters, compute the bounding box of a face
		void CalcBoundingBox(Rect& out_bounding_box, const Vec6d& params_global, const Mat_<double>& params_local);

		// Helpers for computing Jacobians, and Jacobians with the weight matrix
		void ComputeRigidJacobian(const Mat_<double>& params_local, const Vec6d& params_global, Mat_<double> &Jacob, const Mat_<double> W, cv::Mat_<double> &Jacob_t_w);
		void ComputeJacobian(const Mat_<double>& params_local, const Vec6d& params_global, Mat_<double> &Jacobian, const Mat_<double> W, cv::Mat_<double> &Jacob_t_w);

		// Given the current parameters, and the computed delta_p compute the updated parameters
		void UpdateModelParameters(const Mat_<double>& delta_p, Mat_<double>& params_local, Vec6d& params_global);

  };
  //===========================================================================
}
#endif
