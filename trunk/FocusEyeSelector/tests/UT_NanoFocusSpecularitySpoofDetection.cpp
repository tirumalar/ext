#ifdef UNITTEST

#include <tut/tut.hpp>

#include <stdio.h>
#include <stdlib.h>
#include "NanoFocusSpecularityBasedSpoofDetector.h"
#include <cxcore.h>
#include <highgui.h>


namespace tut
{
	struct NanoFocusSpecularityBasedSpoofDetectorData
	{
		NanoFocusSpecularityBasedSpoofDetector *m_detector;


		NanoFocusSpecularityBasedSpoofDetectorData(): m_detector(0)
		{ 
			init();
		}
		~NanoFocusSpecularityBasedSpoofDetectorData() { term(); }

		void init()
		{
			m_detector = new NanoFocusSpecularityBasedSpoofDetector();
			m_detector->SetSpecularityValue(230);
		}

		void term()
		{
			if(m_detector)	delete m_detector; m_detector = 0;

		}

	};

	typedef	test_group<NanoFocusSpecularityBasedSpoofDetectorData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("Nano Focus Specularity Spoof TESTS");
}

namespace tut 
{

	template<>
	template<>
	void testobject::test<1>() 
	{
		set_test_name("Nano Focus Specularity Spoof Detection");

		const char *filenames[3] = {"data/nano/real/Image_75_0_0_1.pgm", "data/nano/real/Image_76_0_1_0.pgm", "data/nano/real/Image_77_0_0_1.pgm"};

		for(int i=0;i<3;i++)
		{
			IplImage *img1 = cvLoadImage(filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
			ensure("Image must exist", img1 != NULL);

			std::pair<float, float> score = m_detector->check_image(img1->imageData, img1->width, img1->height, img1->widthStep);
	
			cvReleaseImage(&img1);
		}

	}


}

#endif
