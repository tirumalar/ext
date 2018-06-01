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

#include <PAW.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "CLM_utils.h"

using namespace CLMTracker;

using namespace cv;

// A constructor from destination shape and triangulation
PAW::PAW(const Mat_<double>& destination_shape, const Mat_<int>& triangulation)
{
	// Initialise some variables directly
	this->destination_landmarks = destination_shape;
	this->triangulation = triangulation;

	int num_points = destination_shape.rows/2;

	int num_tris = triangulation.rows;
	
	// Pre-compute the rest
    alpha = Mat_<double>(num_tris, 3);
    beta = Mat_<double>(num_tris, 3);
    
    Mat_<double> xs = destination_shape(Rect(0, 0, 1, num_points));
    Mat_<double> ys = destination_shape(Rect(0, num_points, 1, num_points));
    
	for (int tri = 0; tri < num_tris; ++tri)
	{	
		int j = triangulation.at<int>(tri, 0);
		int k = triangulation.at<int>(tri, 1);
		int l = triangulation.at<int>(tri, 2);

        double c1 = ys.at<double>(l) - ys.at<double>(j);
        double c2 = xs.at<double>(l) - xs.at<double>(j);
        double c4 = ys.at<double>(k) - ys.at<double>(j);
        double c3 = xs.at<double>(k) - xs.at<double>(j);
        		
        double c5 = c3*c1 - c2*c4;

        alpha.at<double>(tri, 0) = (ys.at<double>(j) * c2 - xs.at<double>(j) * c1) / c5;
        alpha.at<double>(tri, 1) = c1/c5;
        alpha.at<double>(tri, 2) = -c2/c5;

        beta.at<double>(tri, 0) = (xs.at<double>(j) * c4 - ys.at<double>(j) * c3)/c5;
        beta.at<double>(tri, 1) = -c4/c5;
        beta.at<double>(tri, 2) = c3/c5;
	}

	double max_x;
	double max_y;

	minMaxLoc(xs, &min_x, &max_x);
	minMaxLoc(ys, &min_y, &max_y);

	int w = (int)(max_x - min_x + 1.5);
    int h = (int)(max_y - min_y + 1.5);
    
	// Round the min_x and min_y for simplicity?

    pixel_mask = Mat_<uchar>(h, w, (uchar)0);
    triangle_id = Mat_<int>(h, w, -1);
        
	int curr_tri = -1;

	for(int y = 0; y < pixel_mask.rows; y++)
	{
		for(int x = 0; x < pixel_mask.cols; x++)
		{
			curr_tri = findTriangle(Point_<double>(x + min_x, y + min_y), triangulation, destination_shape, curr_tri);
			// If there is a triangle at this location
            if(curr_tri != -1)
			{
				triangle_id.at<int>(y, x) = curr_tri;
                pixel_mask.at<uchar>(y, x) = 1;
			}	
		}
	}
    	
	// Preallocate maps and coefficients
	coefficients.create(num_tris, 6);
	map_x.create(pixel_mask.rows,pixel_mask.cols);
	map_y.create(pixel_mask.rows,pixel_mask.cols);


}

//===========================================================================
void PAW::Read(std::ifstream& stream)
{

	stream.read ((char*)&number_of_pixels, 4);
	stream.read ((char*)&min_x, 8);
	stream.read ((char*)&min_y, 8);

	CLMTracker::ReadMatBin(stream, destination_landmarks);

	CLMTracker::ReadMatBin(stream, triangulation);

	CLMTracker::ReadMatBin(stream, triangle_id);
	
	cv::Mat tmpMask;	
	CLMTracker::ReadMatBin(stream, tmpMask);	
	tmpMask.convertTo(pixel_mask, CV_8U);	
	
	CLMTracker::ReadMatBin(stream, alpha);

	CLMTracker::ReadMatBin(stream, beta);

	map_x.create(pixel_mask.rows,pixel_mask.cols);
	map_y.create(pixel_mask.rows,pixel_mask.cols);

	coefficients.create(this->NumberOfTriangles(),6);
	
	source_landmarks = destination_landmarks;
}

//=============================================================================
// cropping from the source image to the destination image using the shape in s, used to determine if shape fitting converged successfully
void PAW::Warp(const Mat& image_to_warp, Mat& destination_image, const Mat_<double>& landmarks_to_warp)
{
  
	// set the current shape
	source_landmarks = landmarks_to_warp.clone();

	// prepare the mapping coefficients using the current shape
	this->CalcCoeff();

	// Do the actual mapping computation (where to warp from)
	this->WarpRegion(map_x, map_y);
  	
	// Do the actual warp (with bi-linear interpolation)
	remap(image_to_warp, destination_image, map_x, map_y, CV_INTER_LINEAR);
  
}


//=============================================================================
// Calculate the warping coefficients
void PAW::CalcCoeff()
{
	int p = this->NumberOfLandmarks();

	for(int l = 0; l < this->NumberOfTriangles(); l++)
	{
	  
		int i = triangulation.at<int>(l,0);
		int j = triangulation.at<int>(l,1);
		int k = triangulation.at<int>(l,2);

		double c1 = source_landmarks.at<double>(i    , 0);
		double c2 = source_landmarks.at<double>(j    , 0) - c1;
		double c3 = source_landmarks.at<double>(k    , 0) - c1;
		double c4 = source_landmarks.at<double>(i + p, 0);
		double c5 = source_landmarks.at<double>(j + p, 0) - c4;
		double c6 = source_landmarks.at<double>(k + p, 0) - c4;

		// Get a pointer to the coefficient we will be precomputing
		double *coeff = coefficients.ptr<double>(l);

		// Extract the relevant alphas and betas
		double *c_alpha = alpha.ptr<double>(l);
		double *c_beta  = beta.ptr<double>(l);

		coeff[0] = c1 + c2 * c_alpha[0] + c3 * c_beta[0];
		coeff[1] =      c2 * c_alpha[1] + c3 * c_beta[1];
		coeff[2] =      c2 * c_alpha[2] + c3 * c_beta[2];
		coeff[3] = c4 + c5 * c_alpha[0] + c6 * c_beta[0];
		coeff[4] =      c5 * c_alpha[1] + c6 * c_beta[1];
		coeff[5] =      c5 * c_alpha[2] + c6 * c_beta[2];
	}
}

//======================================================================
// Compute the mapping coefficients
void PAW::WarpRegion(Mat_<float>& mapx, Mat_<float>& mapy)
{
	
	cv::MatIterator_<float> xp = mapx.begin();
	cv::MatIterator_<float> yp = mapy.begin();
	cv::MatIterator_<uchar> mp = pixel_mask.begin();
	cv::MatIterator_<int>   tp = triangle_id.begin();
	
	// The coefficients corresponding to the current triangle
	double * a;

	// Current triangle being processed	
	int k=-1;

	for(int y = 0; y < pixel_mask.rows; y++)
	{
		double yi = double(y) + min_y;
	
		for(int x = 0; x < pixel_mask.cols; x++)
		{
			double xi = double(x) + min_x;

			if(*mp == 0)
			{
				*xp = -1;
				*yp = -1;
			}
			else
			{
				// triangle corresponding to the current pixel
				int j = *tp;

				// If it is different from the previous triangle point to new coefficients
				// This will always be the case in the first iteration, hence a will not point to nothing
				if(j != k)
				{
					// Update the coefficient pointer if a new triangle is being processed
					a = coefficients.ptr<double>(j);			
					k = j;
				}  	

				//ap is now the pointer to the coefficients
				double *ap = a;							

				//look at the first coefficient (and increment). first coefficient is an x offset
				double xo = *ap++;						
				//second coefficient is an x scale as a function of x
				xo += *ap++ * xi;						
				//third coefficient ap(2) is an x scale as a function of y
				*xp = float(xo + *ap++ * yi);			

				//then fourth coefficient ap(3) is a y offset
				double yo = *ap++;						
				//fifth coeff adds coeff[4]*x to y
				yo += *ap++ * xi;						
				//final coeff adds coeff[5]*y to y
				*yp = float(yo + *ap++ * yi);			

			}
			mp++; tp++; xp++; yp++;	
		}
	}
}

// ============================================================
// Helper functions to determine which point a triangle lies in
// ============================================================

// Is the point on same side as a half-plane defined by v1, v2, v3
bool sameSide(const Point_<double>& to_test, const Point_<double>& v1, const Point_<double>& v2, const Point_<double>& v3)
{
    double x0 = to_test.x;
	double y0 = to_test.y;
    
    double x1 = v1.x;
    double x2 = v2.x;
    double x3 = v3.x;
    
    double y1 = v1.y;
    double y2 = v2.y;
    double y3 = v3.y;

    double x = (x3-x2)*(y0-y2) - (x0-x2)*(y3-y2);
    double y = (x3-x2)*(y1-y2) - (x1-x2)*(y3-y2);

    return x*y >= 0;

}

// if point is on same side for all three half-planes it is in a triangle
bool pointInTriangle(const Point_<double>& point, const Point_<double>& v1, const Point_<double>& v2, const Point_<double>& v3)
{
    return sameSide(point, v1, v2, v3) && sameSide(point, v2, v1, v3) && sameSide(point, v3, v1, v2);

}

// Find if a given point lies in the triangles
int PAW::findTriangle(const cv::Point_<double>& point, const Mat_<int> triangles, const Mat_<double> control_points, int guess) const
{
    
    int num_tris = triangles.rows;
	int num_points = control_points.rows / 2;

	int tri = -1;
    
	// Allow a guess for speed (so as not to go through all triangles)
	if(guess != -1)
	{
		int j = triangles.at<int>(guess, 0);
		int k = triangles.at<int>(guess, 1);
		int l = triangles.at<int>(guess, 2);

		Point_<double> v1(control_points.at<double>(j), control_points.at<double>(j + num_points));
		Point_<double> v2(control_points.at<double>(k), control_points.at<double>(k + num_points));
		Point_<double> v3(control_points.at<double>(l), control_points.at<double>(l + num_points));

		bool in_triangle = pointInTriangle(point, v1, v2, v3);
		if(in_triangle)
		{
			return guess;
		}
	}

    for (int i = 0; i < num_tris; ++i)
	{
		int j = triangles.at<int>(i, 0);
		int k = triangles.at<int>(i, 1);
		int l = triangles.at<int>(i, 2);

		Point_<double> v1(control_points.at<double>(j), control_points.at<double>(j + num_points));
		Point_<double> v2(control_points.at<double>(k), control_points.at<double>(k + num_points));
		Point_<double> v3(control_points.at<double>(l), control_points.at<double>(l + num_points));

		bool in_triangle = pointInTriangle(point, v1, v2, v3);

        if(in_triangle)
		{
           tri = i;
           break;
		}        
	}
	return tri;
}
