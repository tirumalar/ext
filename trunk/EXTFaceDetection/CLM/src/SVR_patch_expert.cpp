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

#include "SVR_patch_expert.h"

#include "CLM_utils.h"

#include <stdio.h>
#include <iostream>
#include <opencv/highgui.h>

using namespace CLMTracker;

//===========================================================================
// Computing the image gradient
void Grad(const cv::Mat& im, cv::Mat& grad)
{
	
	/*float filter[3] = {1, 0, -1};
	float dfilter[1] = {1};	
	cv::Mat filterX = cv::Mat(1,3,CV_32F, filter).clone();
	cv::Mat filterY = cv::Mat(1,1,CV_32F, dfilter).clone();
			
	cv::Mat gradX;
	cv::Mat gradY;
	cv::sepFilter2D(im, gradX, CV_32F, filterY, filterX, cv::Point(-1,-1), 0);
	cv::sepFilter2D(im, gradY, CV_32F, filterX.t(), filterY, cv::Point(-1,-1), 0);
	cv::pow(gradX,2, gradX);
	cv::pow(gradY,2, gradY);
	grad = gradX + gradY;
			
	grad.row(0).setTo(0);
	grad.col(0).setTo(0);
	grad.col(grad.cols-1).setTo(0);
	grad.row(grad.rows-1).setTo(0);		*/

	// A quicker alternative
	int x,y,h = im.rows,w = im.cols;
	float vx,vy;

	// Initialise the gradient
	grad.create(im.size(), CV_32F);
	grad.setTo(0.0f);

	cv::MatIterator_<float> gp  = grad.begin<float>() + w+1;
	cv::MatConstIterator_<float> px1 = im.begin<float>()   + w+2;
	cv::MatConstIterator_<float> px2 = im.begin<float>()   + w;
	cv::MatConstIterator_<float> py1 = im.begin<float>()   + 2*w+1;
	cv::MatConstIterator_<float> py2 = im.begin<float>()   + 1;

	for(y = 1; y < h-1; y++)
	{ 
		for(x = 1; x < w-1; x++)
		{
			vx = *px1++ - *px2++;
			vy = *py1++ - *py2++;
			*gp++ = vx*vx + vy*vy;
		}
		px1 += 2;
		px2 += 2;
		py1 += 2;
		py2 += 2;
		gp += 2;
	}

}

//===========================================================================
void SVR_patch_expert::Read(ifstream &stream)
{

	// A sanity check when reading patch experts
	int read_type;
	stream >> read_type;
	assert(read_type == 2);
  
	stream >> type >> confidence >> scaling >> bias;
	CLMTracker::ReadMat(stream, weights); 
	
	// OpenCV and Matlab matrix cardinality is different, hence the transpose
	weights = weights.t();

}

//===========================================================================
void SVR_patch_expert::Response(const Mat_<float>& area_of_interest, Mat_<double>& response)
{

	int response_height = area_of_interest.rows - weights.rows + 1;
	int response_width = area_of_interest.cols - weights.cols + 1;
	
	// the patch area on which we will calculate reponses
	cv::Mat_<float> normalised_area_of_interest;
  
	if(response.rows != response_height || response.cols != response_width)
	{
		response.create(response_height, response_width);
	}

	// If type is raw just normalise mean and standard deviation
	if(type == 0)
	{
		// Perform normalisation across whole patch
		cv::Scalar mean;
		cv::Scalar std;

		cv::meanStdDev(area_of_interest, mean, std);
		// Avoid division by zero
		if(std[0] == 0)
		{
			std[0] = 1;
		}
		normalised_area_of_interest = (area_of_interest - mean[0]) / std[0];
	}
	// If type is gradient, perform the image gradient computation
	else if(type == 1)
	{
		Grad(area_of_interest, normalised_area_of_interest);
	}
  	else
	{
		printf("ERROR(%s,%d): Unsupported patch type %d!\n", __FILE__,__LINE__, type);
		abort();
	}
	
	Mat_<float> svr_response;

	// The empty matrix as we don't pass precomputed dft's of image
	Mat_<double> empty_matrix_0(0,0,0.0);
	Mat_<float> empty_matrix_1(0,0,0.0);
	Mat_<float> empty_matrix_2(0,0,0.0);

	// Efficient calc of patch expert SVR response across the area of interest
	matchTemplate_m(normalised_area_of_interest, empty_matrix_0, empty_matrix_1, empty_matrix_2, weights, weights_dfts, svr_response, CV_TM_CCOEFF_NORMED); 

	response.create(svr_response.size());
	MatIterator_<double> p = response.begin();

	cv::MatIterator_<float> q1 = svr_response.begin(); // respone for each pixel
	cv::MatIterator_<float> q2 = svr_response.end();

	while(q1 != q2)
	{
		// the SVR response passed into logistic regressor
		*p++ = 1.0/(1.0 + exp( -(*q1++ * scaling + bias )));
	}

}

void SVR_patch_expert::ResponseDepth(const Mat_<float>& area_of_interest, cv::Mat_<double> &response)
{

	// How big the response map will be
	int response_height = area_of_interest.rows - weights.rows + 1;
	int response_width = area_of_interest.cols - weights.cols + 1;
	
	// the patch area on which we will calculate reponses
	Mat_<float> normalised_area_of_interest;
  
	if(response.rows != response_height || response.cols != response_width)
	{
		response.create(response_height, response_width);
	}

	if(type == 0)
	{
		// Perform normalisation across whole patch
		cv::Scalar mean;
		cv::Scalar std;
		
		// ignore missing values
		cv::Mat_<uchar> mask = area_of_interest > 0;
		cv::meanStdDev(area_of_interest, mean, std, mask);

		// if all values the same don't divide by 0
		if(std[0] == 0)
		{
			std[0] = 1;
		}

		normalised_area_of_interest = (area_of_interest - mean[0]) / std[0];

		// Set the invalid pixels to 0
		normalised_area_of_interest.setTo(0, mask == 0);
	}
	else
	{
		printf("ERROR(%s,%d): Unsupported patch type %d!\n", __FILE__,__LINE__,type);
		abort();
	}
  
	Mat_<float> svr_response;
		
	// The empty matrix as we don't pass precomputed dft's of image
	Mat_<double> empty_matrix_0(0,0,0.0);
	Mat_<float> empty_matrix_1(0,0,0.0);
	Mat_<float> empty_matrix_2(0,0,0.0);

	// Efficient calc of patch expert response across the area of interest
	matchTemplate_m(normalised_area_of_interest, empty_matrix_0, empty_matrix_1, empty_matrix_2, weights, weights_dfts, svr_response, CV_TM_CCOEFF); 

	response.create(svr_response.size());
	MatIterator_<double> p = response.begin();

	cv::MatIterator_<float> q1 = svr_response.begin(); // respone for each pixel
	cv::MatIterator_<float> q2 = svr_response.end();

	while(q1 != q2)
	{
		// the SVR response passed through a logistic regressor
		*p++ = 1.0/(1.0 + exp( -(*q1++ * scaling + bias )));
	}	
}

//===========================================================================
void Multi_SVR_patch_expert::Read(ifstream &stream)
{
	// A sanity check when reading patch experts
	int type;
	stream >> type;
	assert(type == 3);

	// The number of patch experts for this view (with different modalities)
	int number_modalities;

	stream >> width >> height >> number_modalities;
	
	svr_patch_experts.resize(number_modalities);
	for(int i = 0; i < number_modalities; i++)
		svr_patch_experts[i].Read(stream);

}
//===========================================================================
void Multi_SVR_patch_expert::Response(const Mat_<float> &area_of_interest, Mat_<double> &response)
{
	
	int response_height = area_of_interest.rows - height + 1;
	int response_width = area_of_interest.cols - width + 1;

	if(response.rows != response_height || response.cols != response_width)
	{
		response.create(response_height, response_width);
	}

	// For the purposes of the experiment only use the response of normal intensity, for fair comparison

	if(svr_patch_experts.size() == 1)
	{
		svr_patch_experts[0].Response(area_of_interest, response);		
	}
	else
	{
		// responses from multiple patch experts these can be gradients, LBPs etc.
		response.setTo(1.0);
		
		Mat_<double> modality_resp(response_height, response_width);

		for(size_t i = 0; i < svr_patch_experts.size(); i++)
		{			
			svr_patch_experts[i].Response(area_of_interest, modality_resp);			
			response = response.mul(modality_resp);	
		}	
		
	}

}

void Multi_SVR_patch_expert::ResponseDepth(const Mat_<float>& area_of_interest, Mat_<double>& response)
{
	int response_height = area_of_interest.rows - height + 1;
	int response_width = area_of_interest.cols - width + 1;

	if(response.rows != response_height || response.cols != response_width)
	{
		response.create(response_height, response_width);
	}
	
	// With depth patch experts only do raw data modality
	svr_patch_experts[0].ResponseDepth(area_of_interest, response);
}
//===========================================================================
