#pragma once

#include <map>
#include <list>

class EyeInfo 
{
public:
	bool operator < (const EyeInfo &other) const
	{
		return score < other.score;
	}

//	friend bool operator < (const EyeInfo &source, const EyeInfo &comp)
//	{
//		return source.score < comp.score;
//	}

	friend bool operator > (const EyeInfo &source, const EyeInfo &comp)
	{
		return source.score > comp.score;
	}

	friend bool operator == (const EyeInfo &source, const EyeInfo &comp)
	{
		return source.score == comp.score;
	}

	friend bool operator < (const EyeInfo &source, const int &comp)
	{
		return source.score < comp;
	}

	friend bool operator > (const EyeInfo &source, const int &comp)
	{
		return source.score > comp;
	}

	int score;
	int x;
	int y;
	void *handle;
};

class EyeTracker
{
public:
	EyeTracker(int threshold = 100000, bool peakDetection=true, int blockSize=0);
	virtual ~EyeTracker(void);
	std::map<std::pair<int, int>, void * > Track(int frameID, std::map<int, EyeInfo> eyeMap);
	std::list<std::pair<void *, void *> > generate_pairings(int frameID, std::map<int, EyeInfo> eyeMap);
	void Reset();
protected:
	std::map<int, EyeInfo> m_lastEyeMap;
	int **m_dist;
	std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> > m_lastTrackMap, m_newTrackMap;
	int m_focusThreshold;
	bool m_peakDetection;
	bool m_blockThreshold;
	int m_blockSize;
	std::map<std::pair<int, int>, void * > m_returnedGoodIDs;
	bool m_blockReset;
	int m_blockIndex;
};
