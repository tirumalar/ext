#ifndef IRISSELECTOR_H_
#define IRISSELECTOR_H_

#include <vector>
#include <utility>
#include <set>
#include <string.h>
#include <deque>
#include <map>
#include <utility> 
#include <bitset>
//namespace IrisSelectorNS
 
#define IRIS_CODE_LENGTH 2560

typedef struct
{
	float x;
	float y;
	float r;
} IrisSelectorCircle;

typedef struct
{
	IrisSelectorCircle IrisCircle;
	IrisSelectorCircle PupilCircle;

} IrisSelectorCircles;


class IrisCode {
public:
	IrisCode(unsigned char *irisCode) {
		SetCode(irisCode);
	}
	IrisCode():m_pCode(NULL),m_pMask(NULL) {
	}
	~IrisCode() {}
	unsigned char *GetCode() {
		return m_pCode;
	}
	unsigned char *GetMask() {
		return m_pMask;
	}
	void SetCode(unsigned char *irisCode){
		m_pCode = irisCode;
		m_pMask = irisCode+(IRIS_CODE_LENGTH>>1);
	}
protected:
	unsigned char *m_pCode;
	unsigned char *m_pMask;
};
class Iris{
public:
	Iris(unsigned char *pData, unsigned char *irisCode):m_Code(irisCode) {
		Init();
		m_pImage = pData;
		m_HasCode = true;
		m_allocated = false;
	}

	Iris():m_Code(NULL){
		unsigned char *code = new unsigned char [2560];
		m_Code.SetCode(code);
		Init();
		m_pImage = NULL;
		m_HasCode = true;
		m_allocated = true;
	}
	//Bk Added Copy constructor
	Iris(const Iris & rhsIris):m_halo(rhsIris.m_halo),m_corruptBit_Mask(rhsIris.m_corruptBit_Mask),m_iid(rhsIris.m_iid){
		unsigned char *code = new unsigned char [2560];
		memcpy(code,const_cast<Iris *>(&rhsIris)->GetCode(),2560);
		m_Code.SetCode(code);
		Init();
		m_allocated = true;
		for(int i=0;i<8;i++)
			m_FeatureVariance[i]=rhsIris.m_FeatureVariance[i];

		m_iframeId = rhsIris.m_iframeId;

	}

	~Iris() {
		if(m_allocated){
			delete m_Code.GetCode();
		}
	}

	void Init() {
		m_pImage = 0;
		m_BestScore = 1.0;
		m_HasCode = false;
		m_iid = -1;
		m_icameraId = -1;
		m_iframeId = -1;
		m_iimageId = -1;
		m_inumberOfImages = -1;
		m_fscale = -1.0f;
		m_ix = -1;
		m_iy = -1;
		m_iscore = -1;
		m_imaxValue = -1;
		m_pfileName = 0;
		m_idiameter = -1;
		m_irx=0.0f;
		m_iry=0.0f;
		m_halo=-500.0f;
		m_prevEyeIdx=0;
		m_illumState=true;
		m_corruptBit_Mask=10240;
		m_corruptBit_Occlusion=10240;
		m_fvScore=999;
		m_hScore=999;
		m_cbMaskScore=999;
		m_qualityScore=999;
		m_OldHalo=300.0f;
		m_oldHScore=999;
		m_laplacian = -1000.0f;
		m_laplacianScore = -1000.0f;
		m_occlusion = 0;
	}
	float GetBestScore() const {
		return m_BestScore;
	}
	void SetBestScore(float score) {
		m_BestScore = score;
	}
	unsigned char *GetImage() {
		return m_pImage;
	}
	unsigned char *GetCode() {
		return m_Code.GetCode();
	}
	unsigned char *GetMask() {
		return m_Code.GetMask();
	}
	const IrisSelectorCircles &GetIrisSelectorCircles() const
	{
		return m_irisSelectorCircles;
	}
	IrisSelectorCircles &GetIrisSelectorCircles()
	{
		return m_irisSelectorCircles;
	}
	void SetIrisSelectorCircles(const IrisSelectorCircles &value)
	{
		m_irisSelectorCircles = value;
	}
	void SetHasCode(bool has) {
		m_HasCode = has;
	}
	bool GetHasCode() const {
		return m_HasCode;
	}
	const float & GetFeatureVariance(int i) const {
		return m_FeatureVariance[i];
	}
	float & GetFeatureVariance(int i) {
		return m_FeatureVariance[i];
	}
	void setFeatureVariances(float *f)
	{
		for(int i=0;i<8;i++)
			m_FeatureVariance[i]=f[i];
	}
	void SetId(int id){
		m_iid = id;
	}
	int GetId() const {
		return m_iid; 
	}
	bool GetHasId() const {
		return m_iid != -1;
	}
	void SetCameraId(int cameraId){
		m_icameraId = cameraId;
	}
	int GetCameraId() const {
		return m_icameraId; 
	}
	bool GetHasCameraId() const {
		return m_icameraId != -1;
	}
	void SetFrameId(int frameId){
		m_iframeId = frameId;
	}
	int GetFrameId() const {
		return m_iframeId; 
	}
	bool GetHasFrameId() const {
		return m_iframeId != -1;
	}
	void SetImageId(int imageId){
		m_iimageId = imageId;
	}
	int GetImageId() const {
		return m_iimageId; 
	}
	bool GetHasImageId() const {
		return m_iimageId != -1;
	}
	void SetNumberOfImages(int numberOfImages){
		m_inumberOfImages = numberOfImages;
	}
	int GetNumberOfImages() const {
		return m_inumberOfImages; 
	}
	bool GetHasNumberOfImages() const {
		return m_inumberOfImages != -1;
	}
	void SetScale(float scale){
		m_fscale = scale;
	}
	float GetScale() const {
		return m_fscale; 
	}
	void SetHaloScore(float score){
		m_halo=score;
	}
	float GetHaloScore() const {
		return m_halo; 
	}
	void SetOldHaloScore(float score){
		m_OldHalo=score;
	}
	float GetOldHaloScore() const {
		return m_OldHalo; 
	}
	bool GetIllumState() const {
		return m_illumState; 
	}
	void SetIllumState(bool score){
		m_illumState=score;
	}
	void SetPrevEyeIdx(int idx){
		m_prevEyeIdx=idx;
	}
	int GetPrevEyeIdx() const {
		return m_prevEyeIdx; 
	}
	void GetSpecCentroid(float& x, float& y) const {
		x=m_irx;
		y=m_iry;
	}
	float GetSpecCentroidX() const {
		return m_irx;
	}
	float GetSpecCentroidY() const {
		return m_iry;
	}

	void SetSpecCentroid(float x, float y) {
		m_irx=x;
		m_iry=y;
	}
	bool GetHasScale() const {
		return m_fscale != -1.0f;
	}
	void SetX(int x){
		m_ix = x;
	}
	int GetX() const {
		return m_ix; 
	}
	bool GetHasX() const {
		return m_ix != -1;
	}
	void SetY(int y){
		m_iy = y;
	}
	int GetY() const {
		return m_iy; 
	}
	bool GetHasY() const {
		return m_iy != -1;
	}
	void SetWidth(int width){
		m_iwidth = width;
	}
	int GetWidth() const {
		return m_iwidth; 
	}
	bool GetHasWidth() const {
		return m_iwidth != -1;
	}
	void SetHeight(int height){
		m_iheight = height;
	}
	int GetHeight() const {
		return m_iheight; 
	}
	bool GetHasHeight() const {
		return m_iheight != -1;
	}
	void SetScore(int score){
		m_iscore = score;
	}
	int GetScore() const {
		return m_iscore; 
	}
	bool GetHasScore() const {
		return m_iscore != -1;
	}
	void SetMaxValue(int maxValue){
		m_imaxValue = maxValue;
	}
	int GetMaxValue() const {
		return m_imaxValue; 
	}
	bool GetHasMaxValue() const {
		return m_imaxValue != -1;
	}
	void SetFileName(char *fileName){
		m_pfileName = fileName;
	}
	char *GetFileName() const {
		return m_pfileName; 
	}
	bool GetHasFileName() const {
		return m_pfileName != 0;
	}
	void SetDiameter(int diameter){
		m_idiameter = diameter;
	}
	int GetDiameter() const {
		return m_idiameter; 
	}
	bool GetHasDiameter() const {
		return m_idiameter != 0;
	}
	void SetIrisData(unsigned char *data){
		memcpy(m_Code.GetCode(),data,2560);
	}
	void SetCorruptBit_Mask(int corruptBit_Mask){
		m_corruptBit_Mask = corruptBit_Mask;
	}
	int GetCorruptBit_Mask() const {
		return m_corruptBit_Mask; 
	}
	void SetCorruptBit_Occlusion(int corruptBit_Occlusion){
		m_corruptBit_Occlusion = corruptBit_Occlusion;
	}
	int GetCorruptBit_Occlusion() const {
		return m_corruptBit_Occlusion; 
	}
	void SetQualityScore(float score){
		m_qualityScore = score;
	}
	float GetQualityScore() const {
		return m_qualityScore; 
	}
	void SetHScore(float score){
		m_hScore = score;
	}
	float GetHScore() const {
		return m_hScore; 
	}
	void SetOldHScore(float score){
		m_oldHScore = score;
	}
	float GetOldHScore() const {
		return m_oldHScore; 
	}
	void SetFVScore(float score){
		m_fvScore = score;
	}
	float GetFVScore() const {
		return m_fvScore; 
	}
	void SetCBMask_Score(float score){
		m_cbMaskScore = score;
	}
	float GetCBMask_Score() const {
		return m_cbMaskScore; 
	}
	void SetOldHalo(float score){
		m_OldHalo=score;
	}
	float GetOldHalo() const {
		return m_OldHalo; 
	}
	void SetLaplacian(float score){
		m_laplacian=score;
	}

	float GetLaplacian() const {
		return m_laplacian; 
	}

	void SetLaplacianScore(float score){
		m_laplacianScore = score;
	}

	float GetLaplacianScore() const {
		return m_laplacianScore; 
	}

	void setSide(int side)
	{
		m_Side = side;
	}
	int getSide() const
	{
	
		return m_Side;
	
	}
	void SetOcclusion(double val){
		m_occlusion=val;
	}
	double GetOcclusion(){
		return m_occlusion;
	}

	void SetCalib(double val1,double val2){
		m_calibAverage=val1;
		m_calibPercentile=val2;
	}
	
	std::pair <double,double> GetCalib(){
		std::pair <double,double> calibPara;
		calibPara.first = m_calibAverage;
		calibPara.second = m_calibPercentile;
		return calibPara;
	}

protected:
	float m_BestScore;
	float m_FeatureVariance[8];
	unsigned char *m_pImage;
	IrisCode m_Code;
	IrisSelectorCircles m_irisSelectorCircles;
	bool m_HasCode;
	int m_iid;
	int m_icameraId;
	int m_iframeId;
	int m_iimageId;
	int m_inumberOfImages;
	float m_fscale;
	float m_irx, m_iry, m_halo;
	int m_prevEyeIdx;
	int m_ix;
	int m_iy;
	int m_iwidth;
	int m_iheight;
	int m_iscore;
	int m_imaxValue;
	char *m_pfileName;
	int m_idiameter;
	bool m_illumState;
	bool m_allocated;
	int m_corruptBit_Mask;
	int m_corruptBit_Occlusion;
	float m_hScore;
	float m_fvScore;
	float m_cbMaskScore;
	float m_qualityScore;
	float m_OldHalo;
	float m_oldHScore;
	float m_laplacian;
	float m_laplacianScore;
	int m_Side; //Eye Side //Rahman
	double m_occlusion;
	double m_calibAverage;
	double m_calibPercentile;
};

/*
* Simple undirected graph via adjacency matrix.
* Could use more abstracted Graph base class akin to boost.
* Let's not reinvent the wheel for now.
*/
class AdjacencyMatrix {
public:
	AdjacencyMatrix() : m_Size(0) { }
	AdjacencyMatrix(int n) : m_Size(n) { } 
	void Connect(int y, int x);
	const bool Connected(int y, int x) const; 
	int GetSize() { return m_Size; }
	void SetSize(int n) { m_Size = n; }
	void ClearGraph();
	void disConnect(int x, int y);
protected:
	int m_Size;
	std::set<std::pair<unsigned int, unsigned int> > m_Connected;
};

class ConnectedComponents {
public:
	ConnectedComponents(AdjacencyMatrix &graph); 
	bool Visited(int v) const { return m_Visited[v]; }
	const std::vector<int> & GetLabels() const { return m_Labels; }
	int GetNumberOfLabels() const { return m_Label; }
protected:
	void Label();
	void DFS(int v); 
	AdjacencyMatrix &m_Graph;
	int m_Label;
	std::vector<bool> m_Visited;
	std::vector<int> m_Labels;
};

class IrisMatchInterface;
class CiRefineGraph;
class IrisSelector
{
public:

	IrisSelector( IrisMatchInterface *pIrisMatchInterface, bool myris_Enroll);
	~IrisSelector();
	void Clear();
	void SetHDThreshold( float threshold );
	float GetHDThreshold( ) const;
	std::pair< Iris *, Iris *> Select();
	float Match(Iris *pEye1, Iris *pEye2);
	std::pair< Iris *, Iris *> Select( std::vector<Iris *> &eyes );
	std::vector< std::vector<Iris *> * >& GetRankedEyeClusters();
	void SetFeatureVarianceScaleIndex(int scale);
	bool CheckSpoof();
	void SetSpoofParams(bool enableSpoof, float threshX, float threshY, int SpoofPairDepth)
	{
		m_SpoofDispThreshX=threshX;
		m_SpoofDispThreshY=threshY;
		m_EnableSpoof=enableSpoof;
		m_SpoofPairDepth=SpoofPairDepth;
	}
	void ClearAll();
	void SetEyeQualityClassifierWeights(float haloRankWeight, float fvRankWeight, float cbMaskRankWeight)
	{
		m_haloRankWeight = haloRankWeight;
		m_fvRankWeight = fvRankWeight;
		m_cbMaskRankWeight = cbMaskRankWeight;
	}
	void SetOldHaloRankWeight(float oldHaloRankWeight) { m_oldHaloRankWeight = oldHaloRankWeight; }
void SetLaplacianRankWeight(float laplacianRankWeight) { m_laplacianRankWeight = laplacianRankWeight; }
	void SetMinEyesInCluster(int minEyesInCluster) { m_minEyesInCluster = minEyesInCluster; }
	void SetEyeSortingLogEnable(bool enable) { m_eyeSortingLogEnable = enable; }
	int GetMinEyesInCluster(void) {return m_minEyesInCluster;}
	float GetHaloRankWeight(void) {return m_haloRankWeight;}
	float GetOldHaloRankWeight(void) {return m_oldHaloRankWeight;}
	float GetFVRankWeight(void) {return m_fvRankWeight;}
	float GetCBMaskRankWeight(void) {return m_cbMaskRankWeight;}
	void SetisRefineGraph(bool val){m_isRefineGraph = val;}
float GetLaplacianRankWeight(void) {return m_laplacianRankWeight;}
	void SetLoggingBasePath(const char *thePath) { strcpy(m_szBaseLoggingPath, thePath); }
	char *GetLoggingBasePath() { return m_szBaseLoggingPath; }

protected:
	std::pair<Iris *, Iris *> Select(std::vector<Iris *> &eyes, int new_size);

	enum SpoofRetType{
		SR_failed=0,
		SR_passed=1,
		SR_nopair=2,
	};

	IrisMatchInterface *m_pIrisMatchInterface;
	bool m_Myris_Enroll; //Myris Rahman

	int m_FeatureScaleIndex;
	float m_HDThreshold;
	float m_SpoofDispThreshX, m_SpoofDispThreshY;
	bool m_EnableSpoof;
	int m_SpoofPairDepth;
	std::vector< std::vector<Iris *> > m_EyeClusters;
	std::vector< std::vector<Iris *> * > m_RankedEyeClusters;
	bool checkSpoof(int clusterIdx);
	SpoofRetType checkSpoof(int clusterIdx, int position);
	bool checkSpecularityDisparity(Iris* prev, Iris* curr);
	float m_haloRankWeight;
	float m_fvRankWeight;
	float m_cbMaskRankWeight;
	float m_oldHaloRankWeight;
	float m_laplacianRankWeight;
	int m_minEyesInCluster;
	bool m_eyeSortingLogEnable;
	bool m_isRefineGraph;
	char m_szBaseLoggingPath[1024];

#ifdef UNITTEST
public:
#endif
	AdjacencyMatrix G;
	CiRefineGraph* m_RefineGPtr;
};

class CiRefineGraph{
public:
	CiRefineGraph():CorrelationFactor(0.5){}
	void initRefineGraph(AdjacencyMatrix &G);
	void count_set_intersection_Union(std::deque<int>::iterator first1, std::deque<int>::iterator last1,std::deque<int>::iterator first2, std::deque<int>::iterator last2,std::pair<int,int> &result);
	void refineGraph(AdjacencyMatrix &G);
	void clear(){ GmapIrisPtrVertxs.clear();}
	void setCorrelationFactor(float cf){CorrelationFactor=cf;}

private:
	std::map<int , std::deque<int> > GmapIrisPtrVertxs;
	int gSize;
	float CorrelationFactor;
};


#endif /* IRISSELECTOR_H_ */
