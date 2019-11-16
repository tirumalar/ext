#pragma once

#include <opencv/cxcore.h>
#include <map>
#include <list>

typedef int (*SortFunctionPtr)(const void *, const void *);

class EyeFeatureServer
{
public:
	EyeFeatureServer(int m_maskPerCent=0, int baseScale=6, int hSample=3);
public:
	~EyeFeatureServer(void);
	int ExtractFeatures(IplImage *img, IplImage *mask, unsigned char *feature, unsigned char *tag);
	int GetFeatureLength() const;
	int GetFeatureByteSize() const;
	int GetFeatureRowLength() const;
	int GetFeatureNumRows() const;
	int checkBitCorruption(unsigned char* tag);

	int GetFeatureVariances(float *var)
	{
		for(int i=0,j=0;i<4;i++)
		{
			var[j++] = sqrt(m_evenVariance[i]);
			var[j++] = sqrt(m_oddVariance[i]);
		}

		return 1;
	}

	float GetRobustFeatureVariance(int index);

	void SetFeatureNormalization(bool normalize) { m_featureNormalize = normalize; }

protected:
	void horizontal_border_wrap(IplImage *in, IplImage *out);
	int FeatureCoring(unsigned char *feature, unsigned char *tag);
	int m_maskPerCent;

	IplImage *maskInt, *maskWrap, *imgInt, *imgWrap;
	int *m_fullFeature, *m_tempFullFeature;
	int m_hSample, m_vSample;
	int m_baseScale;
	int m_maxFiltSize;
	int m_imageHeight, m_imageWidth;
	int m_coreThreshold[8];
	SortFunctionPtr m_sortFunction[8];
	int getThreshold(int *feature, int idx);
	int m_lut[256];
	float m_evenVariance[4], m_oddVariance[4];
	bool m_featureNormalize;

};
