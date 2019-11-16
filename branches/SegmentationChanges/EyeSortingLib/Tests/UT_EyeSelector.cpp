#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include "BiOmega.h"
#include "highgui.h"
#include "IrisSelector.h"
#include "EyeSegmentationInterface.h"
using namespace std;
namespace tut
{
	struct EyeSelectorData
	{		
		IrisMatchInterface *m_pIrisMatchInterface;
		EyeSegmentationInterface *m_pSegmentationInterface;
		IrisSelector *m_pIrisSelector;
		std::vector<Iris*> m_EnrollmentEyes;

		EyeSelectorData() : m_pIrisMatchInterface(0), m_pSegmentationInterface(0), m_pIrisSelector(0)
		{
#ifdef NANO_SDK
			m_pIrisMatchInterface = new IrisMatchInterface(1280,8,1,12);
#else
			m_pSegmentationInterface = new EyeSegmentationInterface();
			m_pSegmentationInterface->init();
			m_pIrisMatchInterface = new IrisMatchInterface(1280,8,1);
#endif
			m_pIrisMatchInterface->init(0x0f0f0f0f, NULL);
			m_pIrisMatchInterface->SetFeatureScale(6, 0.5);
#ifdef NANO_SDK
			m_pIrisMatchInterface->init();
			m_pIrisMatchInterface->SetFeatureScale(1);
			m_pIrisMatchInterface->SetNominalCommonBits(2050);
			//m_pIrisMatchInterface->SetNominalCommonBits(commonBitScale);
#else
			m_pIrisMatchInterface->init(0x0f0f0f0f, NULL);
			m_pIrisMatchInterface->SetFeatureScale(6, 0.5);
#endif
			m_pIrisSelector = new IrisSelector(m_pIrisMatchInterface);
			m_pIrisSelector->SetHDThreshold(0.2f);
		}
		~EyeSelectorData() 
		{ 
//			for (int i = 0; i < m_EnrollmentEyes.size(); i++)
//			{
//				delete[] m_EnrollmentEyes[i]->GetCode();
//				delete m_EnrollmentEyes[i];
//			}
			m_EnrollmentEyes.clear();

			if (m_pIrisSelector) { m_pIrisSelector->Clear(); }
			if(m_pIrisMatchInterface) 
			{
				m_pIrisMatchInterface->term();
				delete m_pIrisMatchInterface; m_pIrisMatchInterface = 0;
			}
			if (m_pIrisSelector) { delete m_pIrisSelector; m_pIrisSelector = 0;	}
			if(m_pSegmentationInterface) { delete m_pSegmentationInterface; m_pSegmentationInterface = 0;}
		}

		void clearIrisData() {
			for (int i = 0; i < m_EnrollmentEyes.size(); i++) 
			{
				delete[] m_EnrollmentEyes[i]->GetCode();
				delete m_EnrollmentEyes[i];
			}
			m_EnrollmentEyes.clear();

			if (m_pIrisSelector) { m_pIrisSelector->ClearAll(); }
		}

		void draw( IplImage* img1, CircleParameters1 pt, CvScalar color )
		{
			CvPoint center = {cvRound(pt.x), cvRound(pt.y)};
			int radius = cvRound(pt.r);
			cvCircle( img1, center, radius, color, 1, 8, 0 );
		}

		void readIris(char *directory, char *irisFile)
		{
			int count = 1;
			unsigned char *irisCodeRead = (unsigned char *) malloc(2560);

			float halo;

			FILE *fp;
			char fname[200];
			strcpy(fname, directory);
			fp=fopen(irisFile, "rb");
			do
			{
				int ret = fscanf(fp, "%s%f", fname+strlen(directory), &halo);
				if(ret == -1) break;
				if(strstr(fname, ".pgm") != 0)
				{
					IrisPupilCircles circles;
					CircleParameters1 pp, ip;

					IplImage *image = cvLoadImage( fname, CV_LOAD_IMAGE_GRAYSCALE );
					bool SegResult = m_pSegmentationInterface->GetIrisCode( (unsigned char *) image->imageData, image->width, image->height, image->widthStep, irisCodeRead, irisCodeRead+1280, &circles );
					if(SegResult)
					{
						m_pSegmentationInterface->GetPupilPoint( &pp );
						m_pSegmentationInterface->GetIrisPoint( &ip );

						Iris *p_Iris = new Iris(NULL,irisCodeRead);
						p_Iris->SetBestScore(0);
						//p_Iris->setFeatureVariances(0);
						p_Iris->SetHasCode(true);
						p_Iris->SetId(count++);
						p_Iris->SetHaloScore(halo);

						//Add Iris vector
						m_EnrollmentEyes.push_back(p_Iris);
						draw( image, pp, CV_RGB(255,0,0) );
						draw( image, ip, CV_RGB(255,0,0) );

						strcpy(fname+strlen(fname)-4, "-c.bmp");
						cvSaveImage(fname, image);
					}
					else{
						cout << "failed segmentation for "<<fname << endl;
					}
					cvReleaseImage(&image);
				}
				else
					break;
			}
			while(1);
			free(irisCodeRead);
			fclose(fp);
		}

		std::pair<int,int> process(char irisFile[])
		{
			std::pair<Iris *, Iris *> result(NULL,NULL);
			m_pIrisSelector->Clear();
			result = m_pIrisSelector->Select(m_EnrollmentEyes);
			//outputDisplay(result);
			std::pair<int,int> output(-1,-1);
			if(result.first) output.first = result.first->GetId();
			if(result.second) output.second = result.second->GetId();
			return output;
		}

		void outputDisplay(std::pair<Iris *, Iris *> result)
		{
			if(result.first)		
				std::cout<<"\nFirst->Score" << result.first->GetBestScore()<< " Halo" << 
				result.first->GetHaloScore()<< " ID = "<< result.first->GetId();
			if(result.second)
				std::cout<<"\n Second->Score" << result.second->GetBestScore()<< " Halo" << 
				result.second->GetHaloScore()<< " ID = "<< result.second->GetId() << "\n";
		}

	};

	typedef	test_group<EyeSelectorData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("Iris Selector with Segmentation");
}

namespace tut 
{
	template<>
	template<>
	void testobject::test<1>() 
	{
		set_test_name("IrisSelector_BatchModeVersion");
		std::pair<int,int> output;
		int expectedData[1][2] = { {5,15} };
		char directory[200]; 
		strcpy(directory, "data/hbox/");
		char *imageListFile = "data/hbox/hbox.txt";
		int count=1;

		{
			readIris(directory, imageListFile);
			output = process(directory);		
			ensure("Output Image ID's should match the expectedData",
				output.first==expectedData[count][0] && output.second==expectedData[count][1]);	
		}

	}	

}

