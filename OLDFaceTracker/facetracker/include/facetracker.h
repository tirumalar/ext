#ifndef __FACETRACKER__
#define __FACETRACKER__

#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>
#include <fstream>
#include <sstream>
#include <opencv/cv.h>

class FaceTracker
{
		// By default try webcam 0
	int device; 

	// cx and cy aren't necessarilly in the image center, so need to be able to override it (start with unit vals and init them if none specified)
	
	double distance3D;
	std::pair <double, double> facePosition; 
	bool cx_undefined;
	int frame_count;
	float fx, fy;
	float cx, cy;
	CLMTracker::CLMParameters clm_parameters;
	CLMTracker::CLM clm_model;
	double certaintyAllowed;
	double certaintyVisualization;

public:
	
	FaceTracker( vector<string> arguments, float cert = 0.2, float certViz = 0.3,int dev = 0, float fxx = 500.0, float fyy = 500.0, float cxx = 0.0, float cyy = 0.0)
	{
		device = dev;
		fx = fxx;
		fy = fyy;
		cx = cxx;
		cy = cyy;
		cx_undefined = true;
		distance3D = -99.0;
		facePosition = std::pair <double, double>(0.0, 0.0);
		clm_parameters = CLMTracker::CLMParameters(arguments);
		clm_model = CLMTracker::CLM(clm_parameters.model_location);
		CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);  
		frame_count  = 0;
		certaintyAllowed = cert;
		certaintyVisualization = certViz;
	}

	void setCxCy(float width, float height);
	static vector<string> get_arguments(int argc, char **argv);
	//std::pair <double, std::pair <double, double> > getDistance (Mat captured_image_fullSize, bool webcam, bool showTracking);  //Old
	std::pair <double, std::pair <double, double> > getDistance (Mat captured_image_fullSize, bool webcam, bool showTracking, double FPS);
	double getDistance();
	std::pair <double, double> getPosition();

};
#endif
