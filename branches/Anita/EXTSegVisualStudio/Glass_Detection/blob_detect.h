#include <cv.h>
#include <iostream>
#include <bitset>

class DetectGlass
{

	std::bitset <5> G;
	
public:
	DetectGlass(); 
	~DetectGlass(); 
	static bool detect_glass(IplImage* m_eyeCrop, IplImage *oframe, double Ratio_check,double Ratio1_check,double edge_thresh,int scale);
	static bool detect_glass_Effect(IplImage* m_eyeCropIn, IplImage *oframe, double Ratio_check,double Ratio1_check,double edge_thresh,int scale, int maxirisRadius = 100);

	bool detect_glass_window(int decision);
};