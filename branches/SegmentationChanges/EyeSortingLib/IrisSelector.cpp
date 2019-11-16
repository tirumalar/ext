#include "IrisSelector.h"
#include "EyeSegmentationInterface.h"
#include <stdio.h>
#include <algorithm>
#include <map>
#include <utility>
#include <iostream>
#include <string>
#include <opencv/cxcore.h>
#include <fstream>
#include <iterator>
#include<deque>
#define IRIS_IMAGE_WIDTH 640
#define IRIS_IMAGE_HEIGHT 480
#define IRIS_NOMINAL_COMMON_BITS 4100

#ifdef __ANDROID__
#define printf LOGI
#endif

using namespace std;


bool IrisFocusPredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetFeatureVariance(0) < rhs->GetFeatureVariance(0));
}

bool IrisModifiedHaloPredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetHaloScore() < rhs->GetHaloScore());
}

bool LaplacianPredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetLaplacianScore() < rhs->GetLaplacianScore());
}

bool IrisOldHaloPredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetOldHaloScore() < rhs->GetOldHaloScore());
}

bool QualityScorePredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetQualityScore() > rhs->GetQualityScore());
}

bool BitCountMaskPredicate(const Iris * lhs, const Iris * rhs)
{
	return (lhs->GetCorruptBit_Mask() < rhs->GetCorruptBit_Mask());
}

class FocusPredicate  : public std::binary_function<const Iris *, const Iris *,bool>
{
public:
	FocusPredicate(int index) : m_Index(index) {}
	bool operator()(const Iris * lhs, const Iris * rhs) { return (lhs->GetFeatureVariance(m_Index) < rhs->GetFeatureVariance(m_Index)); }
	int m_Index;
};


bool IrisListSizePredicate(const std::vector<Iris *> *lhs, const std::vector<Iris *> *rhs)
{
	return lhs->size() > rhs->size();
}

bool IrisListFocusPredicate(const std::vector<Iris *> *lhs, const std::vector<Iris *> *rhs)
{
	return lhs->back()->GetFeatureVariance(0) > rhs->back()->GetFeatureVariance(0);
}

void AdjacencyMatrix::Connect(int y, int x) {
		m_Connected.insert(std::make_pair(y,x));
}
void AdjacencyMatrix::disConnect(int y, int x) {
	m_Connected.erase(std::make_pair(y,x));
}

const bool AdjacencyMatrix::Connected(int y, int x) const {
	std::set<std::pair<unsigned int ,unsigned int> >::const_iterator it = m_Connected.find(std::make_pair(y,x));
	return it != m_Connected.end();
}

void AdjacencyMatrix::ClearGraph() { 
	m_Connected.clear(); 
	m_Size = 0; 
}

ConnectedComponents::ConnectedComponents(AdjacencyMatrix &graph) : m_Graph(graph), m_Label(0) 
{
  m_Labels.resize(graph.GetSize());
  m_Visited.resize(graph.GetSize());
  std::fill(m_Visited.begin(), m_Visited.end(), false);
  Label();
}

// Connected Components LOGIC:
//   for all v in V do visited(v) = 0
//   c = 0  (a global variable)
//   for all v in V do {
//       if (visited(v)==0) {
//           DFS(v)
//           c = c+1
//       }
//   }
//
void ConnectedComponents::Label() {
	m_Label = 0;
	for (int v = 0; v < m_Graph.GetSize(); v++) {
		if (Visited(v) == 0) {
			DFS(v);
			m_Label++;
		}
	}
}

// DFS LOGIC:
//   DFS(v) {
//      visited(v) = 1
//      comp(v) = c
//      for all w in adj(v) do if (visited(w)==0) DFS(w)
//   }
void ConnectedComponents::DFS(int v) {
	m_Visited[v] = true;
	m_Labels[v] = m_Label;
	for (int w = 0; w < m_Graph.GetSize(); w++) {
		if(w!=v){
			if ((m_Graph.Connected(v, w) || m_Graph.Connected(w, v)) && !Visited(w)) {
				DFS(w);
			}
		}
	}
}

// Note: this class doesn't own any of the member variables, so no memory management is required
IrisSelector::IrisSelector(IrisMatchInterface *pIrisMatchInterface, bool myris_Enroll) :
	m_pIrisMatchInterface(pIrisMatchInterface), m_HDThreshold(0.20), m_FeatureScaleIndex(0)
	,m_EnableSpoof(false),m_SpoofDispThreshX(10), m_SpoofDispThreshY(1), m_SpoofPairDepth(3)
	,m_haloRankWeight(0.0),m_fvRankWeight(0.25),m_cbMaskRankWeight(0.25),m_oldHaloRankWeight(0.0)
	,m_laplacianRankWeight(0.5f) ,m_minEyesInCluster(2),m_eyeSortingLogEnable(false),m_isRefineGraph(true),m_RefineGPtr(new CiRefineGraph){
	G = AdjacencyMatrix();
	m_Myris_Enroll = myris_Enroll;
}
IrisSelector::~IrisSelector() {
	if(m_RefineGPtr) 
		delete m_RefineGPtr;
	m_RefineGPtr=0;
}

void IrisSelector::Clear() {
	for(int i = 0; i < m_EyeClusters.size(); i++ )
	{
		m_EyeClusters[i].clear();
	}
	m_EyeClusters.clear();
	m_RankedEyeClusters.clear();
	m_RefineGPtr->clear();
}

void IrisSelector::ClearAll() {
	Clear();
	G.ClearGraph();
	m_RefineGPtr->clear();
}

void IrisSelector::SetFeatureVarianceScaleIndex(int scale) {
	m_FeatureScaleIndex = scale;
}

void IrisSelector::SetHDThreshold(float threshold) {
	m_HDThreshold = threshold;
}
float IrisSelector::GetHDThreshold() const {
	return m_HDThreshold;
}
float IrisSelector::Match(Iris *pEye1, Iris *pEye2) {
	std::pair<int, float> result = m_pIrisMatchInterface->match_pair( pEye1->GetCode(), pEye1->GetMask(), pEye2->GetCode(), pEye2->GetMask());
	return result.second;
}


std::pair<Iris *, Iris *> IrisSelector::Select(std::vector<Iris *> &eyes) {
	// Perform all pairs matching; create adjacency matrix
	//printf("\n Eyes List Size = %d",eyes.size());
	std::pair<Iris *, Iris *> result((Iris *)NULL, (Iris *)NULL);

	if (m_RankedEyeClusters.size() > 0)
		if((m_RankedEyeClusters[0]->size()) > (m_minEyesInCluster - 1)) result.first = m_RankedEyeClusters[0]->back();
	if (m_RankedEyeClusters.size() > 1)
		if((m_RankedEyeClusters[1]->size()) > (m_minEyesInCluster - 1)) result.second = m_RankedEyeClusters[1]->back();

	int oldSize = G.GetSize();
	G.SetSize(eyes.size());
	Clear();
	for(int new_size = oldSize+1; new_size<=eyes.size();new_size++)
	{   
		int j=new_size-1;
		for (int i = 0 ; i < j; i++) {

			float hd = Match(eyes[i], eyes[j]);
			
			if (hd < eyes[i]->GetBestScore()) {
				eyes[i]->SetBestScore(hd);
			}

			if (hd < eyes[j]->GetBestScore()) {
				eyes[j]->SetBestScore(hd);
			}

			if (hd < m_HDThreshold) {
				G.Connect(i, j);


			}
		}
		
	}

	result=Select(eyes,eyes.size());

	
	return result;
}

 void CiRefineGraph::initRefineGraph(AdjacencyMatrix &G){
	
	gSize = G.GetSize();
	for(int v=0;v < gSize; ++v){

		for(int i=v +1  ;i < gSize; ++i){

			if(G.Connected(v,i)){
					GmapIrisPtrVertxs[v].push_back(i);
					GmapIrisPtrVertxs[i].push_back(v);
			}
		}
	}
}
	
	
void CiRefineGraph::refineGraph(AdjacencyMatrix &G){
	initRefineGraph(G);
	std::pair<int,int> setResults;
		//Getting the nodes sorted so that we can apply set operations on them
	for (int k=0; k< gSize; ++k){
		std::sort(GmapIrisPtrVertxs[k].begin(),GmapIrisPtrVertxs[k].end());
	}

	for (int i=0; i< gSize; ++i){
			
		for (int j=i+1; j< gSize ; ++j){
								
			if( G.Connected(i,j) || G.Connected(j,i) ) {
				setResults.first=0;
				setResults.second=0;
				
				count_set_intersection_Union(GmapIrisPtrVertxs[i].begin(),GmapIrisPtrVertxs[i].end(),GmapIrisPtrVertxs[j].begin(),GmapIrisPtrVertxs[j].end(),setResults);//using std set_intersection would be slow
				 				
				if(setResults.second <= 2)
					continue;

				if( setResults.first < (CorrelationFactor * (setResults.second/2)) ){ // Node pair having connection and having less than 50% intersection count would be disconncetd.
				
					if(G.Connected(i,j)){
						G.disConnect(i,j);
						GmapIrisPtrVertxs[i].erase(std::find(GmapIrisPtrVertxs[i].begin(),GmapIrisPtrVertxs[i].end(),j));
						GmapIrisPtrVertxs[j].erase(std::find(GmapIrisPtrVertxs[j].begin(),GmapIrisPtrVertxs[j].end(),i));
					}

					if(G.Connected(j,i)){
						G.disConnect(j,i);
						GmapIrisPtrVertxs[i].erase(std::find(GmapIrisPtrVertxs[i].begin(),GmapIrisPtrVertxs[i].end(),j));
						GmapIrisPtrVertxs[j].erase(std::find(GmapIrisPtrVertxs[j].begin(),GmapIrisPtrVertxs[j].end(),i));
					}
				}
			}
		}
		
	}
}

void CiRefineGraph::count_set_intersection_Union(std::deque<int>::iterator first1, std::deque<int>::iterator last1,std::deque<int>::iterator first2, std::deque<int>::iterator last2,std::pair<int,int> &result){
	//Custom 
	//First intersection count
	//second union count
	
	while (first1!=last1 && first2!=last2)
	{
		if (*first1<*first2){
			++first1;
			++result.second;
		}
		else if (*first2<*first1) {
			++first2;
			++result.second;
		}
		else {
			++result.second;
			++result.first;

			++first1; 
			++first2;
		}
	}

	if(first1!=last1)
		result.second += distance(first1,last1);
	else if(first2!=last2)
		result.second += distance(first2,last2);
}





std::pair<Iris *, Iris *> IrisSelector::Select(std::vector<Iris *> &eyes, int new_size) {
	std::pair<int,int> setResults;
    if(m_isRefineGraph)
		m_RefineGPtr->refineGraph(G);
	ConnectedComponents cc(G);
	const std::vector<int> &labels = cc.GetLabels();

	// Extract lists of connected irises
	m_EyeClusters.resize(cc.GetNumberOfLabels());
	for (int i = 0; i < labels.size(); i++) {
		m_EyeClusters[labels[i]].push_back(eyes[i]);
	}

	m_RankedEyeClusters.resize(m_EyeClusters.size());

	if(m_Myris_Enroll)
	{
		m_haloRankWeight = 0.0;
		m_fvRankWeight = 0.25;
		m_cbMaskRankWeight = 0.25;
		m_oldHaloRankWeight =0.0;
		m_laplacianRankWeight = 0.5f;
	}

	for (int i = 0; i < m_EyeClusters.size(); i++) {
					
		/* 
		* Reminder: STL will sort:
		*
		*		<= ascending order (1, 2, ..., 10)
		*		>= descending order (10, ..., 2, 1)
		*
		* Note: High feature variance is assumed to be proportional to sharpness
		* Sorting with <= using feature variance will order images from blurry -> sharp
		*/

		std::vector<Iris *>::iterator iter = (std::min_element(m_EyeClusters[i].begin(), m_EyeClusters[i].end(),IrisOldHaloPredicate));
		float bestOldHalo = (*iter)->GetOldHaloScore();
		iter = (std::max_element(m_EyeClusters[i].begin(), m_EyeClusters[i].end(),IrisModifiedHaloPredicate));
		float bestHalo = (*iter)->GetHaloScore();
		iter = (std::max_element(m_EyeClusters[i].begin(), m_EyeClusters[i].end(),FocusPredicate(0)));
		float bestFV = (*iter)->GetFeatureVariance(0);
		iter = (std::max_element(m_EyeClusters[i].begin(), m_EyeClusters[i].end(),LaplacianPredicate));
		float bestLaplacian = (*iter)->GetLaplacianScore();
		iter = (std::min_element(m_EyeClusters[i].begin(), m_EyeClusters[i].end(),BitCountMaskPredicate));
		int bestBC_Mask = (*iter)->GetCorruptBit_Mask();
		
		for(iter=m_EyeClusters[i].begin();iter!=m_EyeClusters[i].end();iter++)
		{
			float oldHScore = fabs( (((*iter)->GetOldHaloScore() - bestOldHalo)*1.0f)/((*iter)->GetOldHaloScore()) );
			(*iter)->SetOldHScore(oldHScore);
			float hScore = fabs((bestHalo - ((*iter)->GetHaloScore()))/bestHalo);
			(*iter)->SetHScore(hScore);
			float fvScore = (bestFV - ((*iter)->GetFeatureVariance(0)))/bestFV;
			(*iter)->SetFVScore(fvScore); 
			float cbMaskScore = ( ((*iter)->GetCorruptBit_Mask() - bestBC_Mask)*1.0f)/((*iter)->GetCorruptBit_Mask());
			(*iter)->SetCBMask_Score(cbMaskScore); 
			float laplacianScore = (bestLaplacian - ((*iter)->GetLaplacianScore()))/bestLaplacian;
			(*iter)->SetLaplacian(laplacianScore);
			float qualityScore = (((*iter)->GetHScore())*GetHaloRankWeight() + ((*iter)->GetFVScore())*GetFVRankWeight() + 
				((*iter)->GetCBMask_Score())*GetCBMaskRankWeight() + ((*iter)->GetOldHScore())*GetOldHaloRankWeight()	+ 
				((*iter)->GetLaplacian())*GetLaplacianRankWeight());
			(*iter)->SetQualityScore(qualityScore);
		}
		
		std::sort(m_EyeClusters[i].begin(), m_EyeClusters[i].end(), QualityScorePredicate);
		m_RankedEyeClusters[i] = &m_EyeClusters[i]; // get pointers to lists for sorting
	}

	// Sort the lists so that real eye lists occur before lists of blurry eyes or false detections
	std::sort(m_RankedEyeClusters.begin(), m_RankedEyeClusters.end(), IrisListSizePredicate);

	if (m_eyeSortingLogEnable)
	{
		char szFullPath[256];
		char szFullFilename[256];

#ifdef _WIN32
		sprintf(szFullPath, "%sClusters\\", GetLoggingBasePath());
#else
		sprintf(szFullPath, "%sClusters/", GetLoggingBasePath());
#endif


		if (m_RankedEyeClusters.size() > 0)
		{
			std::vector<Iris *>::iterator iterCluster;

			sprintf(szFullFilename, "%sCluster1.txt", szFullPath);
			FILE *fDumpTemp = fopen(szFullFilename,"a");

			fprintf(fDumpTemp,"\n");
			for(iterCluster=m_RankedEyeClusters[0]->begin();iterCluster!=m_RankedEyeClusters[0]->end();iterCluster++)
			{			
	fprintf(fDumpTemp,"\nImage_%u_%f_%f.pgm QS=%4.2f HRankWeight:%4.2f HRankS=%4.2f FVRankWeight:%4.2f FVRankS=%4.2f CBRankWeight:%4.2f CBRankS=%4.2f OldHRankWeight: %4.2f OldHRankS=%4.2f LaplacianScore=%4.2f MinEyesInCluster=%d",(*iterCluster)->GetId(),(*iterCluster)->GetLaplacianScore(), (*iterCluster)->GetHaloScore(),
					(*iterCluster)->GetQualityScore(),GetHaloRankWeight(),(*iterCluster)->GetHScore(),GetFVRankWeight(),(*iterCluster)->GetFVScore(),
					GetCBMaskRankWeight(), (*iterCluster)->GetCBMask_Score(), GetOldHaloRankWeight(), (*iterCluster)->GetOldHScore(), (*iterCluster)->GetLaplacian(), m_minEyesInCluster);
			}
			fprintf(fDumpTemp, "\nCBRankWeight = %4.2f, FV RankWeight %4.2f,  Laplacian RankWeight: %4.2f",  GetCBMaskRankWeight(), GetFVRankWeight(), GetLaplacianRankWeight());

			fclose(fDumpTemp);							
		}	
		if (m_RankedEyeClusters.size() > 1)
		{
			std::vector<Iris *>::iterator iterCluster;

			sprintf(szFullFilename, "%sCluster2.txt", szFullPath);
			FILE *fDumpTemp = fopen(szFullFilename,"a");
			fprintf(fDumpTemp,"\n");
			for(iterCluster=m_RankedEyeClusters[1]->begin();iterCluster!=m_RankedEyeClusters[1]->end();iterCluster++)
			{			
	fprintf(fDumpTemp,"\nImage_%u_%f_%f.pgm QS=%4.2f HRankS=%4.2f FVRankS=%4.2f CBRankS=%4.2f OldHRankS=%4.2f LaplacianScore=%4.2f",(*iterCluster)->GetId(),(*iterCluster)->GetLaplacianScore(), (*iterCluster)->GetHaloScore(),
					(*iterCluster)->GetQualityScore(),(*iterCluster)->GetHScore(),(*iterCluster)->GetFVScore(),
					(*iterCluster)->GetCBMask_Score(), (*iterCluster)->GetOldHScore(), (*iterCluster)->GetLaplacian() );

			}
			fclose(fDumpTemp);							
		}	
	}

	std::pair<Iris *, Iris *> result((Iris *)NULL, (Iris *)NULL);

	if(!CheckSpoof())
		return result;

	if (m_RankedEyeClusters.size() > 0)
		if((m_RankedEyeClusters[0]->size()) > (m_minEyesInCluster - 1)) result.first = m_RankedEyeClusters[0]->back();
	if (m_RankedEyeClusters.size() > 1)
		if((m_RankedEyeClusters[1]->size()) > (m_minEyesInCluster - 1)) result.second = m_RankedEyeClusters[1]->back();

	return result;
}

std::vector<std::vector<Iris *> *>& IrisSelector::GetRankedEyeClusters() {
	return m_RankedEyeClusters;
}

bool IrisSelector::checkSpecularityDisparity(Iris* prev, Iris* curr)
{
	CvPoint2D32f diff[2] = {0};
	// compute normalized separation of centroid from the iris center for image 0
	diff[0].x = (prev->GetSpecCentroidX() - prev->GetIrisSelectorCircles().IrisCircle.x) * 100 / prev->GetIrisSelectorCircles().IrisCircle.r;
	diff[0].y = (prev->GetSpecCentroidY() - prev->GetIrisSelectorCircles().IrisCircle.y) * 100 / prev->GetIrisSelectorCircles().IrisCircle.r;
	// compute normalized separation of centroid from the iris center for image 1
	diff[1].x = (curr->GetSpecCentroidX() - curr->GetIrisSelectorCircles().IrisCircle.x) * 100 / curr->GetIrisSelectorCircles().IrisCircle.r;
	diff[1].y = (curr->GetSpecCentroidY() - curr->GetIrisSelectorCircles().IrisCircle.y) * 100 / curr->GetIrisSelectorCircles().IrisCircle.r;
	// Compare the two normalized separations

	printf("disp(%d %d-%d-%d vs %d-%d-%d)=> (%f, %f), H(%f, %f)\n"
		,prev->GetCameraId()
		,prev->GetFrameId(),prev->GetImageId(), prev->GetIllumState()
		,curr->GetFrameId(),curr->GetImageId(), curr->GetIllumState()
		,fabs(diff[0].x - diff[1].x),fabs(diff[0].y - diff[1].y)
		,prev->GetHaloScore(),curr->GetHaloScore());

	if(fabs(diff[0].x - diff[1].x) > m_SpoofDispThreshX && fabs(diff[0].y - diff[1].y) > m_SpoofDispThreshY )
	{
		return true;
	}
	return false;
}

bool IrisSelector::CheckSpoof()
{
	if(!m_EnableSpoof) return true;

	for(int i=0;i<1;i++)
	{
		if(!checkSpoof(i))
		{
			printf("spoof check failed for cluster %d\n",i);
			return false;
		}
	}
	printf("spoof check passed\n");
	return true;
}

bool IrisSelector::checkSpoof(int clusterIdx)
{
	SpoofRetType ret=SR_failed;
	for(int i=0;i<m_SpoofPairDepth;i++)
	{
		printf("trying to pair: cluster %d try %d\n",clusterIdx,i);	
		ret=checkSpoof(clusterIdx,i);
		if(ret!=SR_nopair)
			return ret==SR_passed;
	}
	return false;
}

IrisSelector::SpoofRetType IrisSelector::checkSpoof(int clusterIdx, int position)
{
	if(m_RankedEyeClusters.size()<=clusterIdx) 
		return SR_nopair;
	std::vector<Iris *>  *aCluster=m_RankedEyeClusters[clusterIdx];

	if(position>=aCluster->size()) 
		return SR_nopair;

	Iris* bestEye=aCluster->at(aCluster->size()-position-1);

	char buff[100];
	std::map<std::string,int> irisSet;
	SpoofRetType ret=SR_nopair;
	for(int i=0;i<aCluster->size();i++)
	{
		Iris* aIris=aCluster->at(i);
		if(aIris==bestEye) continue; //dont compare with myself

		sprintf(buff,"%d-%d",aIris->GetCameraId(),aIris->GetFrameId());

		//check 1: if we have >1 eyes in this cluster with same frame+camid => spoof
		std::map<std::string,int>::iterator it =irisSet.find(buff);
		if(it!=irisSet.end())
		{
			printf("Two eyes in 1 frame: C-F: %s I1=%d I2=%d\n",buff,aIris->GetImageId(),bestEye->GetImageId());
			return SR_failed;
		}
		else 
		{
			//remeber the combination
			irisSet[buff]=aIris->GetImageId();
		}

		//check 2: find previous eye in the same frame/camera id
		int frameDiff=bestEye->GetFrameId()-aIris->GetFrameId();
		if(bestEye->GetCameraId()==aIris->GetCameraId() 
			&& ((frameDiff==1)||(frameDiff==-1))
			&& (bestEye->GetIllumState()!=aIris->GetIllumState()))
		{

			return (checkSpecularityDisparity(bestEye, aIris))?SR_passed: SR_failed;
		}
	}
	if(ret==SR_nopair)
	{
		printf("No pair for cluster %d best eye %d %d-%d\n",clusterIdx
			,bestEye->GetCameraId(), bestEye->GetFrameId(), bestEye->GetImageId());
	}

	return ret;
}
