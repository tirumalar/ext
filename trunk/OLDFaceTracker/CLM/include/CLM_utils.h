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


//  Header for all external CLM methods of interest to the user
//
//
//  Tadas Baltrusaitis
//  28/03/2014

#ifndef __CLM_UTILS_h_
#define __CLM_UTILS_h_

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <stdio.h>
#include <fstream>
#include <iostream>

#include "CLM.h"

// Used for face detection
//#include <dlib/image_processing/frontal_face_detector.h>
//#include <opencv.h>

using namespace std;
using namespace cv;

namespace CLMTracker
{
	//===========================================================================	
	// Defining a set of useful utility functions to be used within CLM


	//=============================================================================================
	// Helper functions for parsing the inputs
	//=============================================================================================
	void get_video_input_output_params(vector<string> &input_video_file, vector<string> &depth_dir,
		vector<string> &output_pose_file, vector<string> &output_video_file, vector<string> &output_features_file, bool& camera_plane_pose, vector<string> &arguments);

	void get_camera_params(int &device, float &fx, float &fy, float &cx, float &cy, vector<string> &arguments);

	void get_image_input_output_params(vector<string> &input_image_files, vector<string> &input_depth_files, vector<string> &output_feature_files, vector<string> &output_image_files,
		vector<Rect_<double> > &input_bounding_boxes, vector<string> &arguments);

	//===========================================================================
	// Fast patch expert response computation (linear model across a ROI) using normalised cross-correlation
	//===========================================================================
	// This is a modified version of openCV code that allows for precomputed dfts of templates and for precomputed dfts of an image
	// _img is the input img, _img_dft it's dft (optional), _integral_img the images integral image (optional), squared integral image (optional), 
	// templ is the template we are convolving with, templ_dfts it's dfts at varying windows sizes (optional),  _result - the output, method the type of convolution
	void matchTemplate_m( const Mat_<float>& input_img, Mat_<double>& img_dft, Mat& _integral_img, Mat& _integral_img_sq, const Mat_<float>& templ, map<int, Mat_<double> >& _templ_dfts, cv::Mat_<float>& result, int method );

	//===========================================================================
	// Point set and landmark manipulation functions
	//===========================================================================
	// Using Kabsch's algorithm for aligning shapes
	//This assumes that align_from and align_to are already mean normalised
	Matx22d AlignShapesKabsch2D(const Mat_<double>& align_from, const Mat_<double>& align_to );

	//=============================================================================
	// Basically Kabsch's algorithm but also allows the collection of points to be different in scale from each other
	Matx22d AlignShapesWithScale(cv::Mat_<double>& src, cv::Mat_<double> dst);

	//===========================================================================
	// Visualisation functions
	//===========================================================================
	void Project(Mat_<double>& dest, const Mat_<double>& mesh, double fx, double fy, double cx, double cy);
	void ProjectRahman(Mat_<double>& dest, const Mat_<double>& mesh, double fx, double fy, double cx, double cy);
	void DrawBox(Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy);
	double DrawBoxRahman(Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy);
	double DrawBoxRahman3D(Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy);


	void Draw(cv::Mat img, const Mat_<double>& shape2D, Mat_<int>& visibilities);
	void Draw(cv::Mat img, const Mat_<double>& shape2D);
	void Draw(cv::Mat img, CLM& clm_model);
	std::pair<double, double> DrawRahman(cv::Mat img, const Mat_<double>& shape2D, Mat_<int>& visibilities);  
	std::pair<double, double> DrawAndGiveFeedback(cv::Mat img, CLM& clm_model);

	//===========================================================================
	// Angle representation conversion helpers
	//===========================================================================
	Matx33d Euler2RotationMatrix(const Vec3d& eulerAngles);

	// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
	Vec3d RotationMatrix2Euler(const Matx33d& rotation_matrix);

	Vec3d Euler2AxisAngle(const Vec3d& euler);

	Vec3d AxisAngle2Euler(const Vec3d& axis_angle);

	Matx33d AxisAngle2RotationMatrix(const Vec3d& axis_angle);

	Vec3d RotationMatrix2AxisAngle(const Matx33d& rotation_matrix);

	//============================================================================
	// Face detection helpers
	//============================================================================

	// Face detection using Haar cascade classifier
	bool DetectFaces(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity);
	bool DetectFaces(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, CascadeClassifier& classifier);
	// The preference point allows for disambiguation if multiple faces are present (pick the closest one), if it is not set the biggest face is chosen
	bool DetectSingleFace(Rect_<double>& o_region, const Mat_<uchar>& intensity, CascadeClassifier& classifier, const cv::Point preference = Point(-1,-1));

	// Face detection using HOG-SVM classifier
	//bool DetectFacesHOG(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, std::vector<double>& confidences);
	//bool DetectFacesHOG(vector<Rect_<double> >& o_regions, const Mat_<uchar>& intensity, dlib::frontal_face_detector& classifier, std::vector<double>& confidences);
	// The preference point allows for disambiguation if multiple faces are present (pick the closest one), if it is not set the biggest face is chosen
	//bool DetectSingleFaceHOG(Rect_<double>& o_region, const Mat_<uchar>& intensity, dlib::frontal_face_detector& classifier, double& confidence, const cv::Point preference = Point(-1,-1));

	//============================================================================
	// Matrix reading functionality
	//============================================================================

	// Reading a matrix written in a binary format
	void ReadMatBin(std::ifstream& stream, Mat &output_mat);

	// Reading in a matrix from a stream
	void ReadMat(std::ifstream& stream, Mat& output_matrix);

	// Skipping comments (lines starting with # symbol)
	void SkipComments(std::ifstream& stream);

}
#endif
