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
  //===========================================================================

//      The Constrained Local Model landmark detector, the implementation is mainly based off of the following three papers:

//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild
//       IEEE Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    

//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    

//       J. M. Saragih, S. Lucey, and J. F. Cohn. Face Alignment through 
//       Subspace Constrained Mean-Shifts. International Conference of Computer 
//       Vision (ICCV), September, 2009.

#ifndef __CLM_h_
#define __CLM_h_

#include "PDM.h"
#include "Patch_experts.h"
#include "DetectionValidator.h"
#include "CLMParameters.h"

#include <vector>
#include <opencv/cv.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//#include <dlib/image_processing/frontal_face_detector.h>

using namespace std;
using namespace cv;

namespace CLMTracker
{

class CLM{

public:

	//===========================================================================
	// Member variables that contain the model description

	// The linear 3D Point Distribution Model
    PDM					pdm;
	// The set of patch experts
	Patch_experts		patch_experts;

	// The local and global parameters describing the current model instance (current landmark detections)

	// Local parameters describing the non-rigid shape
	Mat_<double>    params_local;

	// Global parameters describing the rigid shape [scale, euler_x, euler_y, euler_z, tx, ty]
    Vec6d           params_global;


	//==================== Helpers for face detection and landmark detection validation =========================================

	// Haar cascade classifier for face detection
	CascadeClassifier face_detector_HAAR;
	string			  face_detector_location;

	// A HOG SVM-struct based face detector
	//dlib::frontal_face_detector face_detector_HOG;


	// Validate if the detected landmarks are correct using an SVR regressor
	DetectionValidator	landmark_validator; 

	// Indicating if landmark detection succeeded (based on SVR validator)
	bool				detection_success; 

	// Indicating if the tracking has been initialised (for video based CLM)
	bool				tracking_initialised;

	// The actual output of the regressor (-1 is perfect detection 1 is worst detection)
	double				detection_certainty; 

	// the triangulation per each view (for drawing purposes only)
	vector<Mat_<int> >	triangulations;
	
	//===========================================================================
	// Member variables that retain the state of the tracking (reflecting the state of the lastly tracked (detected) image

	// Lastly detect 2D model shape [x1,x2,...xn,y1,...yn]
    Mat_<double>		detected_landmarks;
	
	// The landmark detection likelihoods (combined and per patch expert)
	double				model_likelihood;
	Mat_<double>		landmark_likelihoods;
	
	// Keeping track of how many frames the tracker has failed in so far when tracking in videos
	// This is useful for knowing when to initialise and reinitialise tracking
	int failures_in_a_row;

	// A template of a face that last succeeded with tracking (useful for large motions in video)
	Mat_<uchar> face_template;

	// Useful when resetting or initialising the model closer to a specific location (when multiple faces are present)
	cv::Point_<double> preference_det;

	// A default constructor
	CLM();

	// Constructor from a model file
	CLM(string fname);
	
	// Copy constructor (makes a deep copy of CLM)
	CLM(const CLM& other);

	// Assignment operator for lvalues (makes a deep copy of CLM)
	CLM & operator= (const CLM& other);

	// Empty Destructor	as the memory of every object will be managed by the corresponding libraries (no pointers)
	~CLM(){}

#if 0
	// Move constructor
	CLM(const CLM&& other);

	// Assignment operator for rvalues
	CLM & operator= (const CLM&& other);
#endif

	// Does the actual work - landmark detection
	bool DetectLandmarks(const Mat_<uchar> &image, const Mat_<float> &depth, CLMParameters& params);
	
	// Gets the shape of the current detected landmarks in camera space (given camera calibration)
	// Can only be called after a call to DetectLandmarksInVideo or DetectLandmarksInImage
	Mat_<double> GetShape(double fx, double fy, double cx, double cy);

	// A utility bounding box function
	Rect_<double> GetBoundingBox() const;

	// Reset the model (useful if we want to completelly reinitialise, or we want to track another video)
	void Reset();

	// Reset the model, choosing the face nearest (x,y) where x and y are between 0 and 1.
	void Reset(double x, double y);

	// Reading the model in
	void Read(string name);

	// Helper reading function
	void Read_CLM(string clm_location);
	
private:

	// the speedup of RLMS using precalculated KDE responses (described in Saragih 2011 RLMS paper)
	map<int, Mat_<double> >		kde_resp_precalc; 

	// The model fitting: patch response computation and optimisation steps
    bool Fit(const Mat_<uchar>& intensity_image, const Mat_<float>& depth_image, const std::vector<int>& window_sizes, const CLMParameters& parameters);

	// Mean shift computation that uses precalculated kernel density estimators (the one actually used)
	void NonVectorisedMeanShift_precalc_kde(Mat_<double>& out_mean_shifts, const vector<Mat_<double> >& patch_expert_responses, const Mat_<double> &dxs, const Mat_<double> &dys, int resp_size, double a, int scale, int view_id, map<int, Mat_<double> >& mean_shifts);

	// The actual model optimisation (update step), returns the model likelihood
    double NU_RLMS(Vec6d& final_global, Mat_<double>& final_local, const vector<Mat_<double> >& patch_expert_responses, const Vec6d& initial_global, const Mat_<double>& initial_local,
		          const Mat_<double>& base_shape, const Matx22d& sim_img_to_ref, const Matx22d& sim_ref_to_img, int resp_size, int view_idx, bool rigid, int scale, Mat_<double>& landmark_lhoods, const CLMParameters& parameters);

	// Removing background image from the depth
	bool RemoveBackground(Mat_<float>& out_depth_image, const Mat_<float>& depth_image);

	// Generating the weight matrix for the Weighted least squares
	void GetWeightMatrix(Mat_<double>& WeightMatrix, int scale, int view_id, const CLMParameters& parameters);

	//=======================================================
	// Legacy functions that are not used at the moment
	//=======================================================

	// Mean shift computation	
	void NonVectorisedMeanShift(Mat_<double>& out_mean_shifts, const vector<Mat_<double> >& patch_expert_responses, const Mat_<double> &dxs, const Mat_<double> &dys, int resp_size, double a, int scale, int view_id);

	// A vectorised version of mean shift (Not actually used)
	void VectorisedMeanShift(Mat_<double>& meanShifts, const vector<Mat_<double> >& patch_expert_responses, const Mat_<double> &iis, const Mat_<double> &jjs, const Mat_<double> &dxs, const Mat_<double> &dys, const Size patchSize, double sigma, int scale, int view_id);		
  };
  //===========================================================================
}
#endif
