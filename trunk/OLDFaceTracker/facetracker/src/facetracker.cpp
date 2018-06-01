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


// SimpleCLM.cpp : Defines the entry point for the console application.

#include "facetracker.h"

#define INFO_STREAM( stream ) \
	std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
	std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
	std::cout << "Error: " << stream << std::endl



static void printErrorAndAbort( const std::string & error )
{
	std::cout << error << std::endl;
	abort();
}

#define FATAL_STREAM( stream ) \
	printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;
using namespace cv;

vector<string> FaceTracker::get_arguments(int argc, char **argv)
{
	
	vector<string> arguments;

	for(int i = 1; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;

}


void FaceTracker::setCxCy(float width, float height){
	if(cx_undefined)
		{
			cx =width/ 4.0f;
			cy = height / 4.0f;
		}
}
std::pair <double, std::pair <double, double> >  FaceTracker::getDistance (Mat captured_image_fullSize, bool webcam, bool showTracking, double FPS)
{

   std::pair <double, std::pair <double, double> >  returnValue;
   returnValue.first = -1;
   returnValue.second.first = -1;
   returnValue.second.second = -1;

   bool use_camera_plane_pose = false;
   //Mat captured_image_fullSize = Mat(inputImage, true);
   Mat captured_image = captured_image_fullSize;
   if(!webcam)
		pyrDown(captured_image_fullSize, captured_image, Size(captured_image_fullSize.cols/2, captured_image_fullSize.rows/2));
	if(!captured_image.empty())
		{		

			Mat_<float> depth_image;
			Mat_<uchar> grayscale_image;

			if(captured_image.channels() == 3)
			{
				cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);				
			}
			else
			{
				grayscale_image = captured_image.clone();				
			}

			
			// The actual facial landmark detection / tracking
			bool detection_success = CLMTracker::DetectLandmarksInVideo(grayscale_image, depth_image, clm_model, clm_parameters);
			//bool detection_success = 0;
			// Work out the pose of the head from the tracked model
			Vec6d pose_estimate_CLM;
			if(use_camera_plane_pose)
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);
			}
			else
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCamera(clm_model, fx, fy, cx, cy, clm_parameters);
			}

			// Visualising the results
			// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
			double detection_certainty = clm_model.detection_certainty;
			double detection_certaintySave = 0;
			distance3D = -1;
			if(detection_certainty < certaintyVisualization)
			{
				
				detection_certaintySave = detection_certainty;
				facePosition = CLMTracker::DrawAndGiveFeedback(captured_image, clm_model); //CLMTracker::Draw(captured_image, clm_model);
				returnValue.second = facePosition;
				if(detection_certainty > 1)
					detection_certainty = 1;
				if(detection_certainty < -1)
					detection_certainty = -1;

				detection_certainty = (detection_certainty + 1)/(certaintyVisualization +1);

				// A rough heuristic for box around the face width
				int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);

				Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);
				if(detection_certaintySave < certaintyAllowed)
							{
								distance3D = CLMTracker::DrawBoxRahman3D(captured_image, pose_estimate_to_draw, Scalar((1-detection_certainty)*255.0,0, detection_certainty*255), thickness, fx, fy, cx, cy);
				                returnValue.first = 8200.67/(distance3D);
							}
				//CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1-detection_certainty)*255.0,0, detection_certainty*255), thickness, fx, fy, cx, cy);
			}

			
			// Write out the framerate on the image before displaying it
			char fpsC[255];
			float tempD = 8200.67/(distance3D);
			//sprintf(fpsC, "Frame: %d, Distance 3D: %0.4f, Position: x: %0.2f, y: %0.2f", (int)frame_count, 82.67*exp(-0.004*distance3D), facePosition.first, facePosition.second);
			if (distance3D > 0)
				sprintf(fpsC, "Distance: %0.3f,  FPS = %0.2f", /*detection_certaintySave,  , wncInfo,  detection_certaintySave,*/ (0.00756*tempD *tempD  + 0.187* tempD  + 5.067), FPS); //(int)frame_count, 82.67*exp(-0.004*distance3D), facePosition.first, facePosition.second,
			else
				sprintf(fpsC, "Distance: -1.00, FPS = %0.2f", FPS);
			//string fpsSt("Frame:  ");
			string fpsSt = fpsC;
			//string fpsSt(fgr.getcurrentPersonName());
			cv::putText(captured_image, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 255, 0));		

			if(showTracking)
			{
				namedWindow("Tracker",1);
				imshow("Tracker", captured_image);
			}

			
			char character_press = cv::waitKey(1);

			// restart the tracker
			if(character_press == 'r')
			{
				clm_model.Reset();
			}
			
			// Update the frame count
			frame_count++;

		}
	return returnValue;
}

	double FaceTracker::getDistance()
	{
		return distance3D;
	}
	std::pair <double, double> FaceTracker::getPosition()
	{
		return facePosition;
	}



