#include "EyeTracker.h"
#include "munkres.h"
#include <stdlib.h>

EyeTracker::EyeTracker(int threshold, bool peakDetection, int blockSize):
m_focusThreshold(threshold), m_peakDetection(peakDetection), m_blockThreshold(blockSize>0), m_blockSize(blockSize),
m_blockReset(!peakDetection), m_blockIndex(0)
{
	m_dist = (int **) malloc(100*sizeof(int *));
	for(int i=0;i<100;i++)
		m_dist[i] = (int *) malloc(100*sizeof(int));
}

EyeTracker::~EyeTracker(void)
{
	for(int i=0;i<100;i++)
		free(m_dist[i]);
	free(m_dist);
}

void EyeTracker::Reset()
{
	m_lastTrackMap.clear();
	m_lastEyeMap.clear();
	m_newTrackMap.clear();
	m_returnedGoodIDs.clear();
}

bool check_map_val(std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> > &trackMap, std::pair<int, EyeInfo> val)
{
	std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> >::iterator nit;
	for(nit=trackMap.begin(); nit != trackMap.end(); nit++)
	{
		if(nit->second == val)
			return true;
	}

	return false;
}

std::map<std::pair<int, int>, void * > EyeTracker::Track(int frameID, std::map<int, EyeInfo> eyeMap)
{
	if(m_peakDetection || m_blockThreshold)
	{
		if(m_lastEyeMap.empty() && !eyeMap.empty() && m_blockReset)
			m_blockIndex = (frameID - 1 + m_blockSize)%m_blockSize;

		std::map<int, EyeInfo>::iterator rit;
		int i=0;
		for(rit = m_lastEyeMap.begin(), i=0;rit!=m_lastEyeMap.end();rit++, i++)
		{
			int j=0;
			std::map<int, EyeInfo>::iterator iit;
			for(iit = eyeMap.begin(), j=0;iit!=eyeMap.end();iit++, j++)
				m_dist[i][j] = std::max(abs(rit->second.x - iit->second.x), abs(rit->second.y - iit->second.y));
		}

		int nrows = m_lastEyeMap.size();
		int ncols = eyeMap.size();

		m_lastTrackMap = m_newTrackMap;
		m_newTrackMap.clear();

		if(nrows * ncols)
		{
			Munkres m;

			m.solve(m_dist, nrows, ncols);

			// Display solved matrix.
			int i=0;
			for(rit = m_lastEyeMap.begin(), i=0;rit!=m_lastEyeMap.end();rit++, i++)
			{
				int j=0;
				std::map<int, EyeInfo>::iterator iit;
				for(iit = eyeMap.begin(), j=0;iit!=eyeMap.end();iit++, j++)
					if(m_dist[i][j]==0)
						m_newTrackMap[std::make_pair(iit->first, iit->second) ] = std::make_pair(rit->first, rit->second);
			}
		}

		// pairing of frame index, eye index, and the prior handle
		std::map<std::pair<int, int>, void * > goodID;

		// map of (pairing of current eye index, score) and (pairing of previous eye index, score)

		if(m_blockThreshold)
		{
			if(m_blockSize == 2)
			{
				std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> >::iterator nit;
				for(nit = m_newTrackMap.begin();nit != m_newTrackMap.end(); nit++)
				{
					if(nit->first.second > nit->second.second && nit->second.second < m_focusThreshold)
						goodID.insert(std::make_pair(std::make_pair(frameID-1, nit->second.first), nit->second.second.handle));			
					else if(nit->first.second < m_focusThreshold)
						goodID.insert(std::make_pair(std::make_pair(frameID, nit->first.first), nit->first.second.handle));
				}
			}
			else if(m_blockSize == 3)
			{
				std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> >::iterator nit;
				for(nit = m_newTrackMap.begin();nit != m_newTrackMap.end(); nit++)
				{
					if(nit->first.second > nit->second.second)
					{
						if(m_lastTrackMap.find(nit->second) != m_lastTrackMap.end())
						{
							if(m_lastTrackMap[nit->second].second > nit->second.second && nit->second.second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID-1, nit->second.first), nit->second.second.handle));
							else if(m_lastTrackMap[nit->second].second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID-2, m_lastTrackMap[nit->second].first), m_lastTrackMap[nit->second].second.handle));
						} 
						else if(frameID%m_blockSize == m_blockIndex && m_blockReset)
						{
							goodID.insert(std::make_pair(std::make_pair(frameID-1, nit->second.first), nit->second.second.handle));
						}
					}
					else
					{
						if(m_lastTrackMap.find(nit->second) != m_lastTrackMap.end())
						{
							if(m_lastTrackMap[nit->second].second > nit->first.second && nit->first.second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID, nit->first.first), nit->first.second.handle));
							else if(m_lastTrackMap[nit->second].second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID-2, m_lastTrackMap[nit->second].first), m_lastTrackMap[nit->second].second.handle));
						}
						else if(frameID%m_blockSize == m_blockIndex && m_blockReset)
						{
							goodID.insert(std::make_pair(std::make_pair(frameID, nit->first.first), nit->first.second.handle));
						}

					}
				}

				if(m_blockReset && frameID%m_blockSize == m_blockIndex)
				{
					std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> >::iterator nit;

					for(nit = m_lastTrackMap.begin();nit != m_lastTrackMap.end(); nit++)
					{
						if(!check_map_val(m_newTrackMap, nit->first))
						{
							if(nit->first.second > nit->second.second && nit->second.second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID-2, nit->second.first), nit->second.second.handle));			
							else if(nit->first.second < m_focusThreshold)
								goodID.insert(std::make_pair(std::make_pair(frameID-1, nit->first.first), nit->first.second.handle));
						}
					}
				}
			}
		}
		else
		{
			std::map<std::pair<int, EyeInfo>, std::pair<int, EyeInfo> >::iterator nit;
			for(nit = m_newTrackMap.begin();nit != m_newTrackMap.end(); nit++)
				if(nit->first.second > nit->second.second && nit->second.second < m_focusThreshold)
					if(m_lastTrackMap.find(nit->second) != m_lastTrackMap.end())
						if(m_lastTrackMap[nit->second].second > nit->second.second)
							goodID.insert(std::make_pair(std::make_pair(frameID-1, nit->second.first), m_lastEyeMap[nit->second.first].handle));
		}

		m_lastEyeMap = eyeMap;

		for(std::map<std::pair<int, int>, void * >::iterator mit = goodID.begin(); mit != goodID.end();)
		{
			if(m_returnedGoodIDs.find(mit->first) != m_returnedGoodIDs.end())
			{
				goodID.erase(mit++);
			}
			else
			{
				m_returnedGoodIDs[mit->first] = mit->second;
				mit++;
			}
		}

		m_returnedGoodIDs.erase(m_returnedGoodIDs.begin(), m_returnedGoodIDs.lower_bound(std::make_pair(frameID-4, 0)));

		if(goodID.size() && m_blockReset)
			Reset();

		return goodID;
	}
	else
	{
		std::map<std::pair<int, int>, void * > goodID;

		for(std::map<int, EyeInfo>::iterator rit = eyeMap.begin();rit!=eyeMap.end();rit++)
			if(rit->second.score < m_focusThreshold)
				goodID.insert(std::make_pair(std::make_pair(frameID, rit->first), rit->second.handle));

		return goodID;
	}
}

std::list<std::pair<void *, void *> > EyeTracker::generate_pairings(int frameID, std::map<int, EyeInfo> eyeMap)
{
	std::map<int, EyeInfo>::iterator rit;
	int i=0;
	for(rit = m_lastEyeMap.begin(), i=0;rit!=m_lastEyeMap.end();rit++, i++)
	{
		int j=0;
		std::map<int, EyeInfo>::iterator iit;
		for(iit = eyeMap.begin(), j=0;iit!=eyeMap.end();iit++, j++)
			m_dist[i][j] = std::max(abs(rit->second.x - iit->second.x), abs(rit->second.y - iit->second.y));
	}

	int nrows = m_lastEyeMap.size();
	int ncols = eyeMap.size();

	std::list<std::pair<void *, void *> > outputList;

	if(nrows * ncols)
	{
		Munkres m;

		m.solve(m_dist, nrows, ncols);

		// Display solved matrix.
		int i=0;
		for(rit = m_lastEyeMap.begin(), i=0;rit!=m_lastEyeMap.end();rit++, i++)
		{
			int j=0;
			std::map<int, EyeInfo>::iterator iit;
			for(iit = eyeMap.begin(), j=0;iit!=eyeMap.end();iit++, j++)
				if(m_dist[i][j]==0)
					outputList.push_back(std::make_pair(rit->second.handle, iit->second.handle));
		}
	}

	m_lastEyeMap = eyeMap;

	return outputList;
}
