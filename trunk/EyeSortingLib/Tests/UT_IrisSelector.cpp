#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "stdio.h"
#include "stdlib.h"
//#include "BiOmega.h"
#include "highgui.h"
#include "IrisSelector.h"
#include "EyeSegmentationInterface.h"

namespace tut
{
	struct IrisSelectorData
	{		
		IrisMatchInterface *m_pIrisMatchInterface;
		IrisSelector *m_pIrisSelector;
		std::vector<Iris*> m_EnrollmentEyes;

		IrisSelectorData() : m_pIrisMatchInterface(0), m_pIrisSelector(0)
		{ 	
#ifdef NANO_SDK
			m_pIrisMatchInterface = new IrisMatchInterface(1280,8,1,12);
#else
			m_pIrisMatchInterface = new IrisMatchInterface(1280,8,1);
#endif
			int weights[8] = {1,1,1,1,0,0,0,0};
			int scale_weights[32];
			for(int i = 0; i < 4; i++)
			{
				for(int j = 0; j < 8; j++)
				{
					scale_weights[i*8+j] = weights[j];
				}
			}

			int weightSum = 0;
			for(int i = 0; i < 8; i++)
			{
				weightSum += (weights[i]);
			}
			unsigned char featureMask = 0xffffffff;

			double commonBitScale = double(weightSum) / 8;
#ifdef __linux
			m_pIrisMatchInterface->init();
			m_pIrisMatchInterface->SetFeatureScale(1);
			m_pIrisMatchInterface->SetNominalCommonBits(2050);
			//m_pIrisMatchInterface->SetNominalCommonBits(commonBitScale);
#else
			m_pIrisMatchInterface->init(featureMask, scale_weights);
			m_pIrisMatchInterface->SetFeatureScale(1, commonBitScale);
#endif
			m_pIrisSelector = new IrisSelector(m_pIrisMatchInterface);
			m_pIrisSelector->SetHDThreshold(0.2f);
		}
		~IrisSelectorData() 
		{ 
			for (int i = 0; i < m_EnrollmentEyes.size(); i++) 
			{
				delete[] m_EnrollmentEyes[i]->GetCode();
				delete m_EnrollmentEyes[i];
			}
			m_EnrollmentEyes.clear();
			
			if (m_pIrisSelector) { m_pIrisSelector->Clear(); }
			if(m_pIrisMatchInterface) 
			{
				m_pIrisMatchInterface->term();
				delete m_pIrisMatchInterface; m_pIrisMatchInterface = 0;
			}
			if (m_pIrisSelector) { delete m_pIrisSelector; m_pIrisSelector = 0;	}
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

		void readIrisSimulated(char irisFile[], int id, float newHalo, float fv0, int cbMask)
		{
			static int count = 0;
			++count;			
			float variance[8];
			unsigned char *irisCodeRead = new unsigned char[2560];
			float bestScore;

			FILE *fDump;
			fDump=fopen(irisFile, "rb");
			if (fDump==NULL) fputs ("File error",stderr);
			fread(&bestScore, sizeof(float), 1, fDump);
			fread(&variance, sizeof(float), 8, fDump);
			fread(irisCodeRead, sizeof(unsigned char), 2560, fDump);
			fclose(fDump);

			Iris *p_Iris = new Iris(NULL,irisCodeRead);
			for(int i=0; i<8; i++)
				(i==0)?(variance[i]=fv0):(variance[i]=-1);					
			
			//Setting the stored values
			p_Iris->SetId(id);
			p_Iris->SetHaloScore(newHalo);
			p_Iris->setFeatureVariances(variance);
			p_Iris->SetCorruptBit_Mask(cbMask);
			//Add Iris vector
			m_EnrollmentEyes.push_back(p_Iris);			
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

	typedef	test_group<IrisSelectorData> tg;
	typedef tg::object testobject;
}

namespace {
	tut::tg test_group("Iris Selector Without Segmentation");
}

namespace tut 
{
	template<>
	template<>
	void testobject::test<1>() 
	{
		set_test_name("IrisSelector_UT_GetBestIrisPair");
		std::pair<int,int> output;
		int expectedData[5][2] =  { {-1,-1}, 
									{-1,-1}, 
									{2,-1},
									{2,5},
									{3,5},
								  };
		int inputData[5][4] =  { {1,107.103,134.340,3424}, //Cluster1
								 {4,107.997,182.565,4306}, //Cluster2
								 {2,134.194,229.090,3618}, //Cluster1
								 {5,136.327,267.713,2661}, //Cluster2
								 {3,138.755,299.377,2919}, //Cluster1
							   };
		char directory[200]; 
		strcpy(directory, "data/IrisSelectorData/GeneratedIris/");
		char *imageListFile = "data/IrisSelectorData/GeneratedIris/GeneratedIrisClassifier.txt";
		size_t offset = strlen(directory);
		FILE *fp = fopen(imageListFile, "rt");
		int count = 0;
		if(fp!=NULL)
		{
			while(fscanf(fp, "%s", directory+offset)==1)
			{	
				readIrisSimulated(directory,inputData[count][0],inputData[count][1],inputData[count][2],inputData[count][3]);
				output = process(directory);		
				ensure("Output Image ID's should match the expectedData",
					output.first==expectedData[count][0] && output.second==expectedData[count][1]);
				count++;	
			}
		}
		else 
			ensure(" File containing Images List not found.",0);				
		fclose(fp);	
	}	

	template<>
	template<>
	void testobject::test<2>() 
	{
		set_test_name("IrisSelectorNewHaloClassifier_BatchMode_ClearAll_Test");
		std::pair<int,int> output;
		int inputData[5][4] =  { {1,107.103,134.340,3424}, //Cluster1
								 {4,107.997,182.565,4306}, //Cluster2
								 {2,134.194,229.090,3618}, //Cluster1
								 {5,136.327,267.713,2661}, //Cluster2
								 {3,138.755,299.377,2919}, //Cluster1
							   };
		int expectedData[1][2] = { {3,5} };
		char directory[200]; 
		strcpy(directory, "data/IrisSelectorData/GeneratedIris/");
		char *imageListFile = "data/IrisSelectorData/GeneratedIris/GeneratedIrisClassifier.txt";
		size_t offset = strlen(directory);
		FILE *fp = fopen(imageListFile, "rt");
		int count = 0;
		if(fp!=NULL)
		{
			while(fscanf(fp, "%s", directory+offset)==1)
			{	
				readIrisSimulated(directory,inputData[count][0],inputData[count][1],inputData[count][2],inputData[count][3]);
				count++;
			}
			output = process(directory);		
			count=0;
			ensure("Output Image ID's should match the expectedData",
				output.first==expectedData[count][0] && output.second==expectedData[count][1]);	
		}
		else 
			ensure(" File containing Images List not found.",0);				
		fclose(fp);	

		ensure("Graph Size should match expectedData before calling clear",m_pIrisSelector->G.GetSize()==5);
		clearIrisData();
		//printf("\n Graph Size = %d\n",m_pIrisSelector->G.GetSize());
		ensure("Graph Size should match expectedData after calling clear",m_pIrisSelector->G.GetSize()==0);
		
		count=0;
		fp = fopen(imageListFile, "rt");
		if(fp!=NULL)
		{
			while(fscanf(fp, "%s", directory+offset)==1)
			{	
				readIrisSimulated(directory,inputData[count][0],inputData[count][1],inputData[count][2],inputData[count][3]);
				count++;
			}
			output = process(directory);	
			count=0;
			ensure("Output Image ID's should match the expectedData",
				output.first==expectedData[count][0] && output.second==expectedData[count][1]);	
		}
		else 
			ensure(" File containing Images List not found.",0);				
		fclose(fp);	
	}	

}

