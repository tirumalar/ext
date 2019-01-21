#include <tut/tut.hpp>
#include <stdio.h>
//#include <stdlib.h>
#include "NanoFocusSpecularityMeasure.h"
#include "getdata_bwCPP.h"
#include <cxcore.h>
#include <cv.h>
//#include <direct.h>
#include "highgui.h"

using namespace std;

//#define DRAWROITEST
namespace tut
{
	// Data used by each test
	struct checkBoundary_Test
	{


		int testLength;
		bool batch;
		checkBoundary_Test(): testLength(5), batch(false){}
		virtual ~checkBoundary_Test() { }
	};
	// Test group registration
	typedef test_group<checkBoundary_Test> factory;
	typedef factory::object object;
}
namespace
{
	tut::factory tf("Check Boundary Test");
}


namespace tut
{
	template<>
	template<>
	void object::test<1>()
	{
		char directory[200]="./data/checkBoundaryData/";
		char* imageListFile = "./data/checkBoundaryData/inputRealOnly.csv";
	
		//imageType (0=RealEye, 1=FakeEye, 2=Dont_Care during ensure)
		NanoFocusSpecularityMeasure *m_nanoFocusSpecularityMeasure = new NanoFocusSpecularityMeasure();
		m_nanoFocusSpecularityMeasure->SetSpecularityValue(255);
		size_t offset = strlen(directory);
		FILE *fp = fopen(imageListFile, "r");
		if(fp!=NULL)
		{
			while(fscanf(fp, "%s", directory+offset)==1)
			{	
				
				if(strstr(directory, ".bmp") != 0 )
				{
					IplImage *imgOrig = cvLoadImage(directory, CV_LOAD_IMAGE_GRAYSCALE);
					ensure("Image not found at specified location ",imgOrig!=NULL );
					bool isBoundaryInValid = checkBoundary(imgOrig);
					cvReleaseImage(&imgOrig);
				}
			}
		}
		else ensure(" File containing Images List not found.",0);				
		fclose(fp);	
	}
} 
