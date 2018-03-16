#include "Image.h"
#include "SimpleClassifier.h"
#include "trainingset.h"

#define MAX_FEATURE_SELECTIONS 500

typedef struct feature_st
{
	REAL f;
	short id;
	char cl;
} FEATURE_VEC;

class EyeDetectionServer
{
public:
	EyeDetectionServer(void *scr);
	~EyeDetectionServer();

	bool InitValidationMode();
	bool Classify(Image8u &image, float *pScore = 0);
	bool Classify(Image8u &image, IppiPoint *offsets, int numOffsets, float *pScore, IppiPoint *haarCenter);

	void ExtractFeatures(Image8u *img);
	int WriteSimpleClassifiers(char *filename, int pickup);
	bool SetNumFeatures(int numFeatures);
	bool SetNumSSFeatures(int numFeatures);
	int GetNumFeatures() { return m_totalFeatures; }
	void LoadClassifiers(char *classifier_filename);
	void FillTheTable(int row, const SimpleClassifier& sc);
	bool LoadTrainingData(char *ptInfoFile, char *ntInfoFile);
	void TrainAdaBoost(int numFeatureSelections, char *classificationResultsFile);
	void LoadFinalClassifiers(char *classifier_filename);
	void Classify(char *resultsFile);
	REAL GetThreshold() { return m_thresh; }

	int CountHaarFeatures();
	SimpleClassifier &GetClassifier(int i) { return m_scs[i]; }
	REAL GetAlpha(int i) { return m_alphas[i]; }

protected:
	void GetFeatureValues0(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int y1,const int y3);
	void GetFeatureValues1(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x3,const int y1,const int y2,const int y3);
	void GetFeatureValues2(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int x4,const int y1,const int y3);
	void GetFeatureValues3(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x3,const int y1,const int y2,const int y3,const int y4);
	void GetFeatureValues4(FEATURE_VEC* const features,const int from,const int to,const int x1,const int x2,const int x3,const int y1,const int y2,const int y3);

	void NormalizeWeights();
	void SingleFeatureClassifier(int row, SimpleClassifier& sc);
	void SelectOneSimpleClassifier();

	SimpleClassifier *m_classifiers;
	int m_totalFeatures;
	int m_sx, m_sy;
	FEATURE_VEC *m_features;
	int m_positiveCount;
	int m_negativeCount;
	int m_totalCount;
	TrainingSet *m_trainset;
	REAL *m_weights;
	bool* m_used;
	SimpleClassifier *m_scs;
	REAL *m_alphas;
	REAL m_thresh;
	int m_count;
	void *scratch;	// used for Bfin
//Maddy added for Spoof
	SimpleClassifier *m_ssclassifiers;
	int m_sstotalFeatures;
	SimpleClassifier *m_ssscs;
	REAL *m_ssalphas;
	REAL m_ssthresh;
	TrainingSet *m_sstrainset;
};
