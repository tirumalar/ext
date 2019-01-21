#ifdef UNITTEST

#include <tut/tut.hpp>

#include <stdio.h>
#include <stdlib.h>
#include "ImageQualityChecker.h"
#include "EyeSegmentationInterface.h"
#include <cxcore.h>
#include <cv.h>
//#include "ippi.h"
//#include "ippsr.h"
//#include "ipps.h"
//#include "ippcv.h"
#include "highgui.h"

//#include <vld.h>

using namespace std;

namespace tut
{
	struct ImageQualityCheckerData
	{
		
		ImageQualityChecker *m_imageQualityChecker;
		
		ImageQualityCheckerData(): m_imageQualityChecker(0) 
		{ 
			init();
		}
		~ImageQualityCheckerData() { term(); }

		void init()
		{
			m_imageQualityChecker = new ImageQualityChecker(640,480);
			
		}

		void term()
		{
			if(m_imageQualityChecker)	delete m_imageQualityChecker;  m_imageQualityChecker = 0;
		}

		void testImageQualityChecker(char directory[], char* imageListFile, int specVal, bool imageType)
		{   //imageType (false=Squinted/Bad Image, true=Good Image)
						
			m_imageQualityChecker->SetSpecularityValue(specVal);
			size_t offset = strlen(directory);
			FILE *fp = fopen(imageListFile, "rt");
			if(fp!=NULL)
			{
				while(fscanf(fp, "%s", directory+offset)==1)
				{	
					int f_IrisX;
					int f_IrisY;
					int f_IrisRad;
					int f_StartPixel;
					int f_EndPixel;
					ensure("Stored IrisX must exist in .txt file.",fscanf(fp, "%d", &f_IrisX) );
					ensure("Stored IrisY must exist in .txt file.",fscanf(fp, "%d", &f_IrisY) );
					ensure("Stored IrisRad must exist in .txt file.",fscanf(fp, "%d", &f_IrisRad) );
					ensure("Stored StartPixel must exist in .txt file.",fscanf(fp, "%d", &f_StartPixel) );
					ensure("Stored EndPixel must exist in .txt file.",fscanf(fp, "%d", &f_EndPixel) );

					if(strstr(directory, ".pgm") != 0 )
					{
						IplImage *imgOrig = cvLoadImage(directory, CV_LOAD_IMAGE_GRAYSCALE);
						ensure("Image not found at specified location ",imgOrig!=NULL );
						//printf("\n%s",directory+offset);

						IrisParameters irisParameters;
						irisParameters.x = f_IrisX;
						irisParameters.y = f_IrisY;
						irisParameters.r = f_IrisRad;
						
						int result = m_imageQualityChecker->checkQualityNano2((unsigned char *)imgOrig->imageData,imgOrig->width,imgOrig->height,imgOrig->widthStep,irisParameters);
						// result (0=Allowed, 1=Blocked)
						printf("\n%-25s I.x=%3d I.y=%3d I.r=%3d Result=%d",directory+offset,f_IrisX,f_IrisY,f_IrisRad,result);
									
						if(imageType==false)
							ensure("<<Squinted/Bad Images should not pass Image Quality Constraints.>>",result);	
						else if(imageType==true)
							ensure("<<Good Images should pass Image Quality Constraints.>>",!result);	
						
						cvReleaseImage(&imgOrig);
					}
				}
			}
			else ensure(" File containing Images List not found.",0);				
			fclose(fp);	
		}


	};

	typedef	test_group<ImageQualityCheckerData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("Image Quality Checker");
}

namespace tut 
{
	template<>
	template<>
	void testobject::test<1>() 
	{
		set_test_name("GoodImages_SpecVal(255)");
		char directory[200];   
		strcpy(directory, "data/ImageQualityCheckerData/Good_255/");
		char *imageListFile = "data/ImageQualityCheckerData/Good_255/Good_255.txt";
		testImageQualityChecker(directory,imageListFile,255,true);
	} 

	template<>
	template<>
	void testobject::test<2>() 
	{
		set_test_name("Squinted/BadImages_SpecVal(255)");
		char directory[200];   
		strcpy(directory, "data/ImageQualityCheckerData/Bad_255/");
		char *imageListFile = "data/ImageQualityCheckerData/Bad_255/Bad_255.txt";
		testImageQualityChecker(directory,imageListFile,255,false);
	} 

	template<>
	template<>
	void testobject::test<3>() 
	{
		set_test_name("GoodImages_SpecVal(230)");
		char directory[200];   
		strcpy(directory, "data/ImageQualityCheckerData/Good_230/");
		char *imageListFile = "data/ImageQualityCheckerData/Good_230/Good_230.txt";
		testImageQualityChecker(directory,imageListFile,230,true);
	} 

	template<>
	template<>
	void testobject::test<4>() 
	{
		set_test_name("Squinted/BadImages_SpecVal(230)");
		char directory[200];   
		strcpy(directory, "data/ImageQualityCheckerData/Bad_230/");
		char *imageListFile = "data/ImageQualityCheckerData/Bad_230/Bad_230.txt";
		testImageQualityChecker(directory,imageListFile,230,false);
	} 
	
} 
#endif