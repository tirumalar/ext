#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "EyeDetection.h"
#include "trainingset.h"


#ifdef __BFIN__
	extern "C"{
	#include "file_manip.h"
	}
#else
#define TIME_OP(m,o) \
	o;
#endif

#define max(x,y) ((x > y)? x:y)

enum {TRAIN, CLASSIFY};

void QuickSort(FEATURE_VEC* values,const int lo,const int hi)
{
    int i=lo, j=hi;
	FEATURE_VEC v;
    REAL x = values[(lo+hi)/2].f;

    do
    {
        while (values[i].f<x) i++;
        while (values[j].f>x) j--;
        if (i<=j)
        {
            v=values[i]; values[i]=values[j]; values[j]=v;
            i++; j--;
        }
    } while (i<=j);

    if (lo<j) QuickSort(values,lo, j);
    if (i<hi) QuickSort(values,i, hi);
}

EyeDetectionServer::EyeDetectionServer(void *scr): scratch(scr)
{
	m_classifiers = NULL;
	m_trainset = NULL;
	m_weights = NULL;
	m_features = NULL;

	m_sx = m_sy = 24;
	m_alphas = NULL;
	m_thresh = 0;
	m_count = 0;
	m_scs = NULL;

	m_ssclassifiers = NULL;
	m_ssalphas = NULL;
	m_ssthresh = 0;
	m_ssscs = NULL;
	m_sstrainset = NULL;
}

EyeDetectionServer::~EyeDetectionServer()
{
	if(m_classifiers)
		delete [] m_classifiers;
	m_classifiers = NULL;

	if(m_features)
		delete [] m_features;
	m_features = NULL;

	if(m_trainset)
		delete m_trainset;
	m_trainset = NULL;

	if(m_weights)
		delete [] m_weights;
	m_weights = NULL;

	if(m_alphas)
		delete [] m_alphas;
	m_alphas = NULL;

	if(m_scs)
		delete [] m_scs;
	m_scs = NULL;

	if(m_used)
		delete [] m_used;
	m_used = NULL;

	//Maddy Spoof
	if(m_ssclassifiers)
		delete [] m_ssclassifiers;
	m_ssclassifiers = NULL;

	if(m_ssalphas)
		delete [] m_ssalphas;
	m_ssalphas = NULL;

	if(m_ssscs)
		delete [] m_ssscs;
	m_ssscs = NULL;

	if(m_sstrainset)
		delete [] m_sstrainset;
	m_sstrainset = NULL;
}

int EyeDetectionServer::CountHaarFeatures()
{
	int haarFeatures = 0;
	for(int i = 0; i < m_totalFeatures; i++)
	{
		haarFeatures += m_scs[i].GetNumberOfHaarFeatures();
	}
	return haarFeatures;
}

bool EyeDetectionServer::SetNumFeatures(int numFeatures)
{
	m_totalFeatures = numFeatures;
	if(m_classifiers)
		delete [] m_classifiers;
	m_classifiers = NULL;

	m_classifiers = new SimpleClassifier[m_totalFeatures];
	if(!m_classifiers )
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_used = new bool[m_totalFeatures];
	if(!m_used)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	for(int i=0;i<m_totalFeatures;i++)
		m_used[i]=false;

	m_alphas = new REAL[MAX_FEATURE_SELECTIONS];
	if(!m_alphas)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_scs = new SimpleClassifier[MAX_FEATURE_SELECTIONS];
	if(!m_scs)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}
	return true;

}

bool EyeDetectionServer::SetNumSSFeatures(int numFeatures){
	//Maddy For Spoof
	m_sstotalFeatures = numFeatures;
	if(m_ssclassifiers)
		delete [] m_ssclassifiers;
	m_ssclassifiers = NULL;

	m_ssclassifiers = new SimpleClassifier[m_sstotalFeatures];
	if(!m_ssclassifiers )
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_ssalphas = new REAL[MAX_FEATURE_SELECTIONS];
	if(!m_ssalphas)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_ssscs = new SimpleClassifier[MAX_FEATURE_SELECTIONS];
	if(!m_ssscs)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_sstrainset = new TrainingSet[2];

	if(!m_sstrainset)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}
	return true;
}


bool EyeDetectionServer::LoadTrainingData(char *ptInfoFile, char *ntInfoFile)
{
	std::ifstream fp, fn;
	int i;

	fp.open(ptInfoFile);
	fp >> m_positiveCount;

	fn.open(ntInfoFile);
	fn >> m_negativeCount;

	m_totalCount = m_positiveCount + m_negativeCount;

	m_trainset = new TrainingSet[m_totalCount];

	if(!m_trainset)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	m_weights = new REAL[m_totalCount];

	if(!m_weights)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	REAL pwt = (REAL) (1.0/(2*m_positiveCount));
	for(i=0;i<m_positiveCount;i++)
	{
		int resp = m_trainset[i].ReadFromFile(fp, i, 1);
		if(resp < 0)
		{
			printf("Unable to load all the image data\n");
			return false;
		}

		m_weights[i] = pwt;
	}
	fp.close();

	REAL nwt = (REAL) (1.0/(2*m_negativeCount));

	for(;i<m_totalCount;i++)
	{
		int resp = m_trainset[i].ReadFromFile(fn, i, 0);
		if(resp < 0)
		{
			printf("Unable to load all the image data\n");
			return false;
		}
		m_weights[i] = nwt;
	}
	fn.close();

	// This occupies the maximum amount of memory
	m_features = new FEATURE_VEC[m_totalCount*m_totalFeatures];

	if(m_features == NULL)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	return true;


}
void EyeDetectionServer::LoadClassifiers(char *classifier_filename)
{
	std::ifstream f;
	int i;

	f.open(classifier_filename);
	for(i=0;i<m_totalFeatures;i++)
		m_classifiers[i].ReadFromFile(f);
	f.close();
}

void EyeDetectionServer::LoadFinalClassifiers(char *classifier_filename)
{
	std::ifstream f;
	int i;

	f.open(classifier_filename);
	f >> m_totalFeatures >> m_thresh;

	for(i=0;i<m_totalFeatures;i++)
	{
		f >> m_alphas[i];
		m_scs[i].ReadFromFile(f);
	}

	f.close();
}

int EyeDetectionServer::WriteSimpleClassifiers(char *filename, int pickup)
{
	int x1,x2,x3,x4,y1,y2,y3,y4;
	SimpleClassifier sc;
	int index;
	std::ofstream f;
	int buf = 6;

	f.open(filename);
	index = 0;

	for(x1=0;x1<m_sx/2+buf;x1+=1)
	{
		int x3start = max(m_sx/2 - buf, x1+2);
		for(x3=x3start;x3<=m_sx;x3+=2)
		{
			for(y1=0;y1<m_sy/2+buf;y1+=1)
			{
				int y3start = max(m_sy/2 - buf, y1+1);
				for(y3=y3start;y3<=m_sy;y3+=1)
				{
					x2 = (x1+x3)/2;
					y2 = y4 = x4 = -1;
					sc.type = 0; sc.error = 0.0;
					sc.x1 = x1; sc.x2 = x2; sc.x3 = x3; sc.x4 = x4;
					sc.y1 = y1; sc.y2 = y2; sc.y3 = y3; sc.y4 = y4;
					sc.parity = 0;
					sc.thresh = 0.0;
					if(rand()%pickup == 0)
					{
						sc.WriteToFile(f);
						index++;
					}
				}
			}
		}
	}

	for(x1=0;x1<m_sx/2+buf;x1+=1)
	{
		int x3start = max(m_sx/2 - buf, x1+1);
		for(x3=x3start;x3<=m_sx;x3+=1)
		{
			for(y1=0;y1<m_sy/2+buf;y1+=1)
			{
				int y3start = max(m_sy/2 - buf, y1+2);

				for(y3=y3start;y3<=m_sy;y3+=2)
				{
					y2 = (y1+y3)/2;
					x2 = x4 = y4 = -1;
					sc.type = 1; sc.error = 0.0;
					sc.x1 = x1; sc.x2 = x2; sc.x3 = x3; sc.x4 = x4;
					sc.y1 = y1; sc.y2 = y2; sc.y3 = y3; sc.y4 = y4;
					sc.parity = 0;
					sc.thresh = 0.0;
					if(rand()%pickup == 0)
					{
						sc.WriteToFile(f);
						index++;
					}
				}
			}
		}
	}

	for(x1=0;x1<m_sx/2+buf;x1++)
	{
		int x4start = max(m_sx/2 - buf, x1+3);
		for(x4=x4start;x4<=m_sx;x4+=3)
		{
			for(y1=0;y1<m_sy/2+buf;y1+=1)
			{
				int y3start = max(m_sy/2 - buf, y1+1);
				for(y3=y3start;y3<=m_sy;y3+=1)
				{
					x2 = x1 + (x4-x1)/3;
					x3 = x2 + (x4-x1)/3;
					y2 = y4 = -1;
					sc.type = 2; sc.error = 0.0;
					sc.x1 = x1; sc.x2 = x2; sc.x3 = x3; sc.x4 = x4;
					sc.y1 = y1; sc.y2 = y2; sc.y3 = y3; sc.y4 = y4;
					sc.parity = 0;
					sc.thresh = 0.0;
					if(rand()%pickup == 0)
					{
						sc.WriteToFile(f);
						index++;
					}
				}
			}
		}
	}

	for(x1=0;x1<m_sx/2+buf;x1++)
	{

		for(x3=x1+1;x3<=m_sx;x3+=1)
		{
			for(y1=0;y1<m_sy/2+buf;y1++)
			{
				int y4start = max(m_sy/2 - buf, y1+3);
				for(y4=y4start;y4<=m_sy;y4+=3)
				{
					y2 = y1 + (y4-y1)/3;
					y3 = y2 + (y4-y1)/3;
					x2 = x4 = -1;
					sc.type = 3; sc.error = 0.0;
					sc.x1 = x1; sc.x2 = x2; sc.x3 = x3; sc.x4 = x4;
					sc.y1 = y1; sc.y2 = y2; sc.y3 = y3; sc.y4 = y4;
					sc.parity = 0;
					sc.thresh = 0.0;
					if(rand()%pickup == 0)
					{
						sc.WriteToFile(f);
						index++;
					}
				}
			}
		}
	}

	for(x1=0;x1<m_sx/2+buf;x1+=1)
	{
		int x3start = max(m_sx/2 - buf, x1+2);
		for(x3=x3start;x3<=m_sx;x3+=2)
		{
			for(y1=0;y1<m_sy/2+buf;y1+=1)
			{
				int y3start = max(m_sy/2 - buf, y1+2);
				for(y3=y3start;y3<=m_sy;y3+=2)
				{
					x2 = (x1+x3)/2;
					y2 = (y1+y3)/2;
					x4 = y4 = -1;
					sc.type = 4; sc.error = 0.0;
					sc.x1 = x1; sc.x2 = x2; sc.x3 = x3; sc.x4 = x4;
					sc.y1 = y1; sc.y2 = y2; sc.y3 = y3; sc.y4 = y4;
					sc.parity = 0;
					sc.thresh = 0.0;
					if(rand()%pickup == 0)
					{
						sc.WriteToFile(f);
						index++;
					}
				}
			}
		}
	}

	f.close();

	return index;
}

void EyeDetectionServer::GetFeatureValues0(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int y1,const int y3)
{
	int stride = m_trainset[0].m_stride / sizeof(Ipp32s);

	int a = x1*stride + y3;
	int b = x1*stride + y1;
	int c = x3*stride + y3;
	int d = x3*stride + y1;
	int e = x2*stride + y1;
	int f = x2*stride + y3;

//	FILE *fp = fopen("C:\\test.txt", "wt");

	for(int i=from;i<to;i++)
	{
		Ipp32s *data = (Ipp32s *) (m_trainset[i].m_data->imageData);
		REAL f1 = (REAL) (  data[a] - data[b] + data[c] - data[d]
			 + 2*(data[e] - data[f]));

		features[i].f = (REAL) (f1 * m_trainset[i].m_variance);
		features[i].id = i;
		features[i].cl = m_trainset[i].m_cl;
//		fprintf(fp, "%d %f\n", i, features[i].f);
	}
//	fclose(fp);
}

void EyeDetectionServer::GetFeatureValues1(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x3,const int y1,const int y2,const int y3)
{
	int stride = m_trainset[0].m_stride / sizeof(Ipp32s);
	int a = x3*stride + y1;
	int b =	x3*stride + y3;
	int c =	x1*stride + y1;
	int d =	x1*stride + y3;
	int e =	x1*stride + y2;
	int f =	x3*stride + y2;

	for(int i=from;i<to;i++)
	{
		Ipp32s *data = (Ipp32s *) (m_trainset[i].m_data->imageData);
		REAL f1 = (REAL) (  data[a] + data[b] - data[c] - data[d]
			 + 2*(data[e] - data[f]));
		features[i].f = (REAL) (f1 * m_trainset[i].m_variance);
		features[i].id = i;
		features[i].cl = m_trainset[i].m_cl;
	}
}

void EyeDetectionServer::GetFeatureValues2(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int x4,const int y1,const int y3)
{
	int stride = m_trainset[0].m_stride / sizeof(Ipp32s);

	int a = x1*stride+y1;
	int b = x1*stride+y3;
	int c = x4*stride+y3;
	int d = x4*stride+y1;
	int e = x2*stride+y3;
	int f = x2*stride+y1;
	int g = x3*stride+y1;
	int h = x3*stride+y3;

	for(int i=from;i<to;i++)
	{
		Ipp32s *data = (Ipp32s *) (m_trainset[i].m_data->imageData);
		REAL f1 =  (REAL) ( data[a] -data[b] + data[c] - data[d]
			 + 3*(data[e] - data[f] + data[g] - data[h]));
		features[i].f = (REAL) (f1 * m_trainset[i].m_variance);
		features[i].id = i;
		features[i].cl = m_trainset[i].m_cl;
	}
}

void EyeDetectionServer::GetFeatureValues3(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x3,const int y1,const int y2,const int y3,const int y4)
{
	int stride = m_trainset[0].m_stride / sizeof(Ipp32s);
	int a = x1*stride+y1;
	int b = x1*stride+y4;
	int c = x3*stride+y4;
	int d = x3*stride+y1;
	int e = x3*stride+y2;
	int f = x3*stride+y3;
	int g = x1*stride+y3;
	int h = x1*stride+y2;

	for(int i=from;i<to;i++)
	{
		Ipp32s *data = (Ipp32s *) (m_trainset[i].m_data->imageData);
		REAL f1 =  (REAL) ( data[a] - data[b] + data[c] - data[d]
			 + 3*(data[e] - data[f] + data[g] - data[h] ));
		features[i].f = (REAL) (f1 * m_trainset[i].m_variance);
		features[i].id = i;
		features[i].cl = m_trainset[i].m_cl;
	}
}

void EyeDetectionServer::GetFeatureValues4(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int y1,const int y2,const int y3)
{
	int stride = m_trainset[0].m_stride / sizeof(Ipp32s);

	int a = x1*stride+y1;
	int b = x1*stride+y3;
	int c = x3*stride+y1;
	int d = x3*stride+y3;
	int e = x2*stride+y1;
	int f = x2*stride+y3;
	int g = x1*stride+y2;
	int h = x3*stride+y2;
	int j = x2*stride+y2;

	for(int i=from;i<to;i++)
	{
		Ipp32s *data = (Ipp32s *) (m_trainset[i].m_data->imageData);
		REAL f1 = (REAL) (  data[a] + data[b] + data[c] + data[d]
			 - 2*(data[e] + data[f] + data[g] + data[h])
			 + 4*data[j]);
		features[i].f = (REAL) (f1 * m_trainset[i].m_variance);
		features[i].id = i;
		features[i].cl = m_trainset[i].m_cl;
	}
}

void EyeDetectionServer::FillTheTable(int row, const SimpleClassifier& sc)
{
	int x1,x2,x3,x4,y1,y2,y3,y4;


	x1 = sc.x1;	y1 = sc.y1;
	x2 = sc.x2;	y2 = sc.y2;
	x3 = sc.x3; y3 = sc.y3;
	x4 = sc.x4;	y4 = sc.y4;

	FEATURE_VEC *features = m_features + row*m_totalCount;
	switch(sc.type)
	{
		case 0:GetFeatureValues0(features,0,m_totalCount,x1,x2,x3,y1,y3);break;
		case 1:GetFeatureValues1(features,0,m_totalCount,x1,x3,y1,y2,y3);break;
		case 2:GetFeatureValues2(features,0,m_totalCount,x1,x2,x3,x4,y1,y3);break;
		case 3:GetFeatureValues3(features,0,m_totalCount,x1,x3,y1,y2,y3,y4);break;
		case 4:GetFeatureValues4(features,0,m_totalCount,x1,x2,x3,y1,y2,y3);break;
	}

//	qsort(features, m_totalCount, sizeof(FEATURE_VEC), compare);
	QuickSort(features,0,m_totalCount-1);

#if 0
	features[0].sp = features[0].sm = 0;
	features[0].tp = features[0].tm = 0.5;

	for(int i=1;i<m_totalCount;i++)
	{
		features[i].tp = features[i].tm = 0.5;
		if(features[i].cl)
		{
			int idx = features[i-1].id;
			if(idx < m_positiveCount) features[i].sp += m_weights[idx];
			else features[i].sm += m_weights[idx];
		}
	}
#endif

	return;
}

void EyeDetectionServer::SingleFeatureClassifier(int row, SimpleClassifier& sc)
{
	FEATURE_VEC *features = m_features + row*m_totalCount;

	REAL e1 = 0.0;
	int pos1 = 0,pos2 = 0;

	// compute errors1, suppose parity is 1, that is f(x)<thresh ==> h(x) = 1
	// compute errors2, suppose parity is 0, that is f(x)>thresh ==> h(x) = 1
	for(int i=0;i<m_positiveCount;i++)
		e1+= m_weights[i];

	REAL e2 = REAL(1.0)-e1;

	REAL min1 = e1;
	REAL min2 = e2;

	for(int i=0;i<m_totalCount;i++)
	{
		int idx = features[i].id;

		e1 += (features[i].cl !=0)?  -m_weights[idx]:m_weights[idx];

		if(e1<min1) { min1=e1; pos1=i; }
		e2 = 1 - e1;
		if(e2<min2) { min2=e2; pos2=i; }
	}

	pos1++; if(pos1==m_totalCount) pos1--;
	pos2++; if(pos2==m_totalCount) pos2--;

	if(min1<min2)
	{
		sc.parity = 1;
		sc.error = min1;
		sc.thresh = features[pos1].f;
		sc.index = pos1;
	}
	else
	{
		sc.parity = 0;
		sc.error = min2;
		sc.thresh = features[pos2].f;
		sc.index = pos2;
	}

}

void EyeDetectionServer::NormalizeWeights()
{
	REAL sum = 0;

	for(int i=0;i<m_totalCount;i++)
		sum += m_weights[i];

	sum = 1.0f/sum;

	for(int i=0;i<m_totalCount;i++)
		m_weights[i] *= sum;

}

void EyeDetectionServer::SelectOneSimpleClassifier()
{
	int i;
	REAL alpha,beta;
	SimpleClassifier *minsc;
	REAL minerror;
	int minindex;

	minerror = REAL(1.01); minindex  = -1;
	NormalizeWeights();

	for(i=0;i<m_totalFeatures;i++)
	{
		if(m_used[i]) continue;
		SingleFeatureClassifier(i,m_classifiers[i]);

		if(m_classifiers[i].error<minerror)
		{
			minerror = m_classifiers[i].error;
			minsc = m_classifiers+i;
			minindex = i;
		}
	}

	m_used[minindex] = true;
	beta = minsc->error / (REAL(1.0)-minsc->error);
	FEATURE_VEC *features = m_features + m_totalCount*minindex;

	int index = minsc->index;
	char parity = minsc->parity;

	for(i=0;i<index;i++)
		if(features[i].cl == parity)
			m_weights[features[i].id] *= beta;

	for(;i<m_totalCount;i++)
		if(features[i].cl != parity)
			m_weights[features[i].id] *= beta;

	if(beta<REAL(1e-8))
		beta = REAL(1e-8);

	alpha = -log(beta);

	m_scs[m_count] = *minsc;
	m_alphas[m_count] = alpha;
	m_thresh += (REAL(0.5)*alpha);
	m_count++;

}

void EyeDetectionServer::TrainAdaBoost(int numFeatureSelections, char *classificationResultsFile)
{
//	ofstream f;
//	REAL* curresult;
//	int fn,fp;

	printf("Computing features ...\n");
	for(int i=0;i<m_totalFeatures;i++)
	{
		FillTheTable(i,m_classifiers[i]);
		printf("\r%d%c Complete", (i*100/m_totalFeatures), 37);
	}

	{
		FILE *fp = fopen("C:\\featureData.dat", "wb");
		fwrite(m_features, sizeof(FEATURE_VEC), (m_totalCount*m_totalFeatures), fp);
		fclose(fp);
	}

	REAL *curresult = new REAL[m_totalCount];
	memset(curresult, 0, m_totalCount*sizeof(REAL));

	printf("\nSelecting Classifiers ...\n");


	for(int i=0;i<numFeatureSelections;i++)
	{
		SelectOneSimpleClassifier();

#if 1
		for(int j=0;j<m_totalCount;j++)
			curresult[j] += (m_alphas[i] * m_scs[i].Apply(m_trainset[j].GetIntegralImage(), m_trainset[j].m_variance));

		int err = 0;
		for(int j=0;j<m_totalCount;j++)
			if( (curresult[j]<m_thresh && m_trainset[j].m_cl==1) || (curresult[j]>=m_thresh && m_trainset[j].m_cl==0) )
				err++;

		printf("Completed %d/%d - Error = %d\n", i+1, numFeatureSelections, err);

#endif

	}

	FILE *fp = fopen(classificationResultsFile, "wt");
	fprintf(fp, "%d %f\n", numFeatureSelections, m_thresh);
	for(int i=0;i<numFeatureSelections;i++)
	{
		fprintf(fp, "%f\t%f %d %d %d %d %d %d %d %d %d %d\n", m_alphas[i], m_scs[i].thresh, m_scs[i].parity, m_scs[i].type,
			m_scs[i].x1, m_scs[i].x2, m_scs[i].x3, m_scs[i].x4, m_scs[i].y1, m_scs[i].y2, m_scs[i].y3, m_scs[i].y4);
	}

	fclose(fp);
}

void EyeDetectionServer::Classify(char *resultsFile)
{
	int numFeatures = m_totalFeatures;
	int errNeg = 0, errPos = 0;

	REAL *curresult = new REAL[m_totalCount];
	memset(curresult, 0, m_totalCount*sizeof(REAL));

	for(int i=0;i<numFeatures;i++)
		for(int j=0;j<m_totalCount;j++)
			curresult[j] += (m_alphas[i] * m_scs[i].Apply(m_trainset[j].GetIntegralImage(), m_trainset[j].m_variance));

	FILE *fp = fopen(resultsFile, "wt");

	for(int j=0;j<m_totalCount;j++)
	{
		if( curresult[j]<m_thresh && m_trainset[j].m_cl==1)
		{
			fputs(m_trainset[j].m_fileName.c_str(), fp);
			fprintf(fp, "\n");
			errNeg++;
		}
		else if(curresult[j]>=m_thresh && m_trainset[j].m_cl==0)
		{
			fputs(m_trainset[j].m_fileName.c_str(), fp);
			fprintf(fp, "\n");
			errPos++;
		}
	}

	fclose(fp);

	printf("Total False Positives = %d/%d\n", errPos, m_negativeCount);
	printf("Total False Negatives = %d/%d\n", errNeg, m_positiveCount);

	delete [] curresult;
}


/**************************************************************************************

   REAL TIME VERSIONS OF ALGORITHM

***************************************************************************************/

// must also call these methods to initialize
//
//resp = eyeServer->SetNumFeatures(MAX_FEATURE_SELECTIONS);
//if(resp)
//{
//	eyeServer->LoadFinalClassifiers(classifiersResultsFile);
//}

bool EyeDetectionServer::InitValidationMode()
{
	m_positiveCount = 0;
	m_negativeCount = 0;

	m_totalCount = 1;
	m_trainset = new TrainingSet(scratch);

	if(!m_trainset)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	// This occupies the maximum amount of memory
	m_features = new FEATURE_VEC[m_totalCount*m_totalFeatures];

	if(m_features == NULL)
	{
		printf("!!! Out of Memory !!!\n");
		printf("Either increase classifier sampling or get more memory\n");
		return false;
	}

	return true;
}

bool EyeDetectionServer::Classify(Image8u &image, float *pScore)
{
	bool isAnEye = false;

	int numFeatures = m_totalFeatures;

	// load in single image here:
	m_trainset[0].ComputeIntegralImage(&image);

	REAL curresult = 0;

	for(int i=0;i<numFeatures;i++)
	{
		curresult += (m_alphas[i] * m_scs[i].Apply(m_trainset[0].GetIntegralImage(), m_trainset[0].m_variance));
	}

	if( curresult>=m_thresh )
	{
		isAnEye = true;
	}

	if(pScore)
	{
		pScore[0] = curresult;;
	}

	return isAnEye;
}

bool EyeDetectionServer::Classify(Image8u &image, IppiPoint *offsets, int numOffsets, float *pScore, IppiPoint *haarCenter)
{
	bool isAnEye = false;

	int numFeatures = m_totalFeatures;

	// load in single image here:
	m_trainset[0].ComputeIntegralImage(&image);

	REAL bestScore = 0;

	for(int j=0;j<numOffsets;j++)
	{
		REAL curresult = 0;
		Ipp64f v =0;
		v = m_trainset[0].GetInvVariance(&image, cvRect(offsets[j].x, offsets[j].y, 24, 24));

		for(int i=0;i<numFeatures;i++)
		{
			curresult += (m_alphas[i] * m_scs[i].Apply(m_trainset[0].GetIntegralImage(), offsets[j], v));
		}

		if( curresult>=m_thresh )
		{
			isAnEye = true;
			if(curresult > bestScore)
			{
				bestScore = curresult;
				*haarCenter = offsets[j];
			}
		}
	}
	if(pScore)
		pScore[0] = bestScore;;

	return isAnEye;
}


