#ifdef UNITTEST

#include <tut/tut.hpp>

#include <stdio.h>
//#include <stdlib.h>
#include "NanoFocusSpecularityMeasure.h"
#include <cxcore.h>
#include <cv.h>
//#include <direct.h>
#include "highgui.h"

// Threshold Values used in the UT's
//#define HALOMINCOUNT_SPECVAL230 25
//#define HALOMAXCOUNT_SPECVAL230 110
#define HALOTHRESHOLD_SPECVAL230 180
//#define HALOMINCOUNT_SPECVAL255 20
//#define HALOMAXCOUNT_SPECVAL255 150
#define HALOTHRESHOLD_SPECVAL255 175

using namespace std;

//#define DRAWROITEST

namespace tut
{
	struct NanoFocusSpecularityMeasureData
	{
		
		NanoFocusSpecularityMeasure *m_nanoFocusSpecularityMeasure;
		
		NanoFocusSpecularityMeasureData(): m_nanoFocusSpecularityMeasure(0)
		{ 
			init();
		}
		~NanoFocusSpecularityMeasureData() { term(); }

		void init()
		{
			m_nanoFocusSpecularityMeasure = new NanoFocusSpecularityMeasure();
		}

		void term()
		{
			if(m_nanoFocusSpecularityMeasure)	delete m_nanoFocusSpecularityMeasure;  m_nanoFocusSpecularityMeasure = 0;

		}
		
		void checkHalo(char directory[], char* imageListFile, int specVal, int imageType)
		{   //imageType (0=RealEye, 1=FakeEye, 2=Dont_Care during ensure)
			m_nanoFocusSpecularityMeasure->SetSpecularityValue(specVal);
			size_t offset = strlen(directory);
			FILE *fp = fopen(imageListFile, "rt");
			if(fp!=NULL)
			{
				while(fscanf(fp, "%s", directory+offset)==1)
				{	
					float hscorePrecomputed;
					ensure("StoredHaloScore value must exist in .txt file.",fscanf(fp, "%f", &hscorePrecomputed) );
					if(strstr(directory, ".pgm") != 0 )
					{
						IplImage *imgOrig = cvLoadImage(directory, CV_LOAD_IMAGE_GRAYSCALE);
						ensure("Image not found at specified location ",imgOrig!=NULL );
						float hscore = m_nanoFocusSpecularityMeasure->ComputeHaloScore(imgOrig);
						int countScore = m_nanoFocusSpecularityMeasure->GetHaloPixelCount();
						printf("\n%-37s Halo=%5.3f StoredHalo=%5.2f Count=%d",directory+offset,hscore,hscorePrecomputed,countScore);
						
						ensure("<<Hard Test on Halo Score should pass. Absolute diff between precomputedScore and current score should be <= 0.1 >>",
							(fabs(hscore-hscorePrecomputed)<=0.1)?1:0);
						if(imageType==0)
							ensure("<<Real Good Image Should Pass the Halo Criteria. Halo Threshold Issue.>>",
							(hscore<=HALOTHRESHOLD_SPECVAL230)?1:0);	
						else if(imageType==1)
							ensure("<<Fake Image Should not Pass the Halo Criteria. Halo Threshold Issue.>>",
							(hscore>HALOTHRESHOLD_SPECVAL230)?1:0);	
						cvReleaseImage(&imgOrig);
					}
				}
			}
			else ensure(" File containing Images List not found.",0);				
			fclose(fp);	
		}

		void checkModifiedHalo(char directory[], char* imageListFile, int specVal, int imageType)
		{   //imageType (0=RealEye, 1=FakeEye, 2=Dont_Care during ensure)
			m_nanoFocusSpecularityMeasure->SetSpecularityValue(specVal);
			size_t offset = strlen(directory);
			FILE *fp = fopen(imageListFile, "rt");
			if(fp!=NULL)
			{
				while(fscanf(fp, "%s", directory+offset)==1)
				{	
					float hscorePrecomputed;
					ensure("StoredHaloScore value must exist in .txt file.",fscanf(fp, "%f", &hscorePrecomputed) );
					if(strstr(directory, ".pgm") != 0 )
					{
						IplImage *imgOrig = cvLoadImage(directory, CV_LOAD_IMAGE_GRAYSCALE);
						ensure("Image not found at specified location ",imgOrig!=NULL );
						//float hscore = m_nanoFocusSpecularityMeasure->ComputeHaloScoreTopPoints(imgOrig,255,6,25.0f,90,180);
						float hscore = m_nanoFocusSpecularityMeasure->ComputeHaloScoreTopPointsNano(imgOrig,255,6,25.0f,90,190);
						int countScore = m_nanoFocusSpecularityMeasure->GetHaloPixelCount();
						printf("\n%-37s Halo=%5.3f StoredHalo=%5.2f Count=%d",directory+offset,hscore,hscorePrecomputed,countScore);
						
						ensure("<<Hard Test on Halo Score should pass. Absolute diff between precomputedScore and current score should be <= 0.1 >>",
							(fabs(hscore-hscorePrecomputed)<=0.1)?1:0);
						
						cvReleaseImage(&imgOrig);
					}
				}
			}
			else ensure(" File containing Images List not found.",0);				
			fclose(fp);	
		}

	};

	typedef	test_group<NanoFocusSpecularityMeasureData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("Nano Focus Specularity Measure");
}

namespace tut 
{

	template<>
	template<>
	void testobject::test<1>() 
	{
		/*****************************************************************
		* HaloScore Constraint on FAKE IMAGES with Specval=230.
		******************************************************************/
		set_test_name("HaloConstraint_FakeImages_SpecVal(230)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/Fake_230/");
		char *imageListFile = "data/NanoFocusData/Fake_230/Fake_230.txt";
		checkHalo(directory,imageListFile,230,1);
	} 

	template<>
	template<>
	void testobject::test<2>() 
	{
		/*****************************************************************
		* HaloScore Constraint on FAKE IMAGES with Specval=255.
		******************************************************************/
		set_test_name("HaloConstraint_FakeImages_SpecVal(255)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/Fake_255/");
		char *imageListFile = "data/NanoFocusData/Fake_255/Fake_255.txt";
		checkHalo(directory,imageListFile,255,1);
	} 

	template<>
	template<>
	void testobject::test<3>() 
	{
		/*****************************************************************
		* HaloScore Constraint on REAL GOOD IMAGES with Specval=230.
		******************************************************************/
		set_test_name("HaloConstraint_RealGoodImages_SpecVal(230)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/RealGood_230/");
		char *imageListFile = "data/NanoFocusData/RealGood_230/RealGood_230.txt";
		checkHalo(directory,imageListFile,230,0);
	} 

	template<>
	template<>
	void testobject::test<4>() 
	{
		/*****************************************************************
		* HaloScore Constraint on REAL GOOD IMAGES with Specval=255.
		******************************************************************/
		set_test_name("HaloConstraint_RealGoodImages_SpecVal(255)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/RealGood_255/");
		char *imageListFile = "data/NanoFocusData/RealGood_255/RealGood_255.txt";
		checkHalo(directory,imageListFile,255,0);
	} 

	template<>
	template<>
	void testobject::test<5>() 
	{
		/*****************************************************************
		* HaloScore Constraint on REAL BAD IMAGES with Specval=230.(Only Hard Constraint on Halo. No Threshold Constraint.)
		******************************************************************/
		set_test_name("HaloConstraint_RealBadImages_SpecVal(230)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/RealBad_230/");
		char *imageListFile = "data/NanoFocusData/RealBad_230/RealBad_230.txt";
		checkHalo(directory,imageListFile,230,2);
	} 

	template<>
	template<>
	void testobject::test<6>() 
	{
		/*****************************************************************
		* HaloScore Constraint on REAL BAD IMAGES with Specval=230.(Only Hard Constraint on Halo. No Threshold Constraint.)
		******************************************************************/
		set_test_name("HaloConstraint_RealBadImages_SpecVal(255)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/RealBad_255/");
		char *imageListFile = "data/NanoFocusData/RealBad_255/RealBad_255.txt";
		checkHalo(directory,imageListFile,255,2);
	} 

	template<>
	template<>
	void testobject::test<7>() 
	{
		/*****************************************************************
		* Modified HaloScore Good Images. Higher value means better focus image.
		******************************************************************/
		set_test_name("ModifiedHalo(255)");
		char directory[200];   
		strcpy(directory, "data/NanoFocusData/RealGood_255/");
		char *imageListFile = "data/NanoFocusData/RealGood_255/RealGood_255_ModifiedHalo_linux.txt";
		checkModifiedHalo(directory,imageListFile,255,0);
	} 


#ifdef DRAWROITEST	
	template<>
	template<>
	void testobject::test<20>() 
	{
		/*****************************************************************
		* Draw SpecROI on Images, save them and Display HaloScore as well as pixelCount.
		******************************************************************/
		set_test_name("DrawSave_SpecRoi_OnImages_SpecVal(255)");
		m_nanoFocusSpecularityMeasure->SetSpecularityValue(255);
		char imageFile[200];

		const char *filename = "data/NanoFocusData/DoubleIlluminator_255/DoubleIlluminator_255.txt";
		strcpy(imageFile, "data/NanoFocusData/DoubleIlluminator_255/");

		size_t offset = strlen(imageFile);
		FILE *fp = fopen(filename, "rt");
	
		if(fp!=NULL)
		{
			while(fscanf(fp, "%s", imageFile+offset)==1)
			{
				if(strstr(imageFile, ".pgm") != 0)
				{
					IplImage *imgOrig = cvLoadImage(imageFile, CV_LOAD_IMAGE_GRAYSCALE);
					ensure("Image not found at specified location ",imgOrig!=NULL );	
					
					float hscore = m_nanoFocusSpecularityMeasure->ComputeHaloScore(imgOrig);
					int countScore = m_nanoFocusSpecularityMeasure->GetHaloPixelCount();
					CvPoint2D32f specCentroid = m_nanoFocusSpecularityMeasure->GetSpecularityCentroid();
					CvRect specROI = m_nanoFocusSpecularityMeasure->GetSpecularityROI();
					static int imageCount=0;
					printf("\nImage_%02d_%-35s %-5s=%6.2f Count=%3d",++imageCount,imageFile+offset,"Halo",hscore,countScore);

					//Plotting over the Image
					cvNamedWindow( "Output", 1 );
					cvCircle(imgOrig,cvPoint((int)specCentroid.x,(int)specCentroid.y),1,   
						cvScalar(150, 150, 100, 1),3, 8, 0);     
					cvRectangle(imgOrig, 
						cvPoint(specROI.x,specROI.y), cvPoint(specROI.x + specROI.width, specROI.y + specROI.height), 
						cvScalar(150, 100, 150, 1), 
						1, 8, 0);
					CvFont font;
					cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.35, 0.35, 0, 1, CV_AA);
					
					char buffer1[100],buffer2[200],buffer3[100],buffer4[200];
					
					mkdir("d:/NanoFocusHaloData");
					mkdir("d:/NanoFocusHaloData/Output");
					FILE *fDump = fopen("D:\\NanoFocusHaloData\\Output\\Dump.txt","a");
					sprintf(buffer1,"Centroid.x = %f Centroid.y= %f",specCentroid.x,specCentroid.y);
					sprintf(buffer2,"specROI.x = %d specROI.y = %d specROI.w = %d specROI.h = %d",specROI.x,specROI.y,specROI.width,specROI.height);
					sprintf(buffer3,"HaloScore=%f Count=%d",hscore,countScore);
					sprintf(buffer4,"D:\\NanoFocusHaloData\\Output\\Image_%u.pgm",imageCount);
					fprintf(fDump,"Image_%u_%s;%d;%f;\n",imageCount,imageFile+offset,countScore,hscore);
					fclose(fDump);
					cvPutText(imgOrig,buffer1, cvPoint(1,10), &font, cvScalar(255, 255, 255, 0));
					cvPutText(imgOrig,buffer2, cvPoint(1,20), &font, cvScalar(255, 255, 255, 0));
					cvPutText(imgOrig,buffer3, cvPoint(1,30), &font, cvScalar(255, 255, 255, 0));
					cvShowImage( "Output", imgOrig );
					cvSaveImage(buffer4,imgOrig);
					cvWaitKey(0);
					cvReleaseImage(&imgOrig);
				}
			}
		}
		else ensure(" File containing Images List not found.",0);				

	    fclose(fp);				
	} 
#endif

} //end of namespace tut

#endif