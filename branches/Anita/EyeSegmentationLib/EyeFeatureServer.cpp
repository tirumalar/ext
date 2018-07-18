#include "EyeFeatureServer.h"
#include "EyeMatchServer.h"
#include <cv.h>
#include <stdio.h>
#include <map>
#include <list>


extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}
#ifdef __BFIN__
#include <bfin_sram.h>
#endif

int compare0 (const void * a, const void * b)
{
  return ( *(int*)a  - *(int*)b );
}
int compare1 (const void * a, const void * b)
{
  return ( *((int*)a + 1)  - *((int*)b + 1) );
}
int compare2 (const void * a, const void * b)
{
  return ( *((int*)a + 2)  - *((int*)b + 2) );
}
int compare3 (const void * a, const void * b)
{
  return ( *((int*)a + 3)  - *((int*)b + 3) );
}
int compare4 (const void * a, const void * b)
{
  return ( *((int*)a + 4)  - *((int*)b + 4) );
}
int compare5 (const void * a, const void * b)
{
  return ( *((int*)a + 5)  - *((int*)b + 5) );
}
int compare6 (const void * a, const void * b)
{
  return ( *((int*)a + 6)  - *((int*)b + 6) );
}
int compare7 (const void * a, const void * b)
{
  return ( *((int*)a + 7)  - *((int*)b + 7) );
}

int comparef0 (const void * a, const void * b)
{
	return ( *((float*)a + 0)  > *((float*)b + 0) )? 1:-1;
}
int comparef1 (const void * a, const void * b)
{
	return ( *((float*)a + 1)  > *((float*)b + 1) )? 1:-1;
}
int comparef2 (const void * a, const void * b)
{
	return ( *((float*)a + 2)  > *((float*)b + 2) )? 1:-1;
}
int comparef3 (const void * a, const void * b)
{
	return ( *((float*)a + 3)  > *((float*)b + 3) )? 1:-1;
}
int comparef4 (const void * a, const void * b)
{
	return ( *((float*)a + 4)  > *((float*)b + 4) )? 1:-1;
}
int comparef5 (const void * a, const void * b)
{
	return ( *((float*)a + 5)  > *((float*)b + 5) )? 1:-1;
}
int comparef6 (const void * a, const void * b)
{
	return ( *((float*)a + 6)  > *((float*)b + 6) )? 1:-1;
}
int comparef7 (const void * a, const void * b)
{
	return ( *((float*)a + 7)  > *((float*)b + 7) )? 1:-1;
}


int EyeFeatureServer::GetFeatureLength() const
{
	return (GetFeatureRowLength() * GetFeatureNumRows());
}
int EyeFeatureServer::GetFeatureByteSize() const { return 1; }
int EyeFeatureServer::GetFeatureRowLength() const { return (m_imageWidth * GetFeatureByteSize()/m_hSample); }
int EyeFeatureServer::GetFeatureNumRows() const { return (m_imageHeight/m_vSample); }

int EyeFeatureServer::checkBitCorruption(unsigned char* tag)
{
	int BitCount = 0;

	int len = GetFeatureLength();
	for(int ii = 0;ii<len;ii++)
		BitCount += (8 - m_lut[tag[ii]]);

	return(BitCount);
}

static int number_of_ones(int num)
{
	int cnt = 0;
	while(num)
	{
		cnt += num%2;
		num /= 2;
	}
	return cnt;
}

EyeFeatureServer::EyeFeatureServer(int maskPerCent, int baseScale, int hSample):
m_maskPerCent(maskPerCent), m_hSample(hSample), m_vSample(8), m_baseScale(baseScale),
m_imageHeight(64), m_imageWidth(480), m_featureNormalize(false)
{
	m_maxFiltSize = 4 * m_baseScale * 4;

	imgInt = cvCreateImage( cvSize(m_imageWidth+m_maxFiltSize+1, m_imageHeight+1), IPL_DEPTH_32S, 1 ); // for Integral Image: Iris
	imgWrap = cvCreateImage( cvSize(m_imageWidth+m_maxFiltSize, m_imageHeight),IPL_DEPTH_8U, 1 );		// for wrapped Iris Image

	maskWrap = cvCreateImage( cvSize(m_imageWidth+m_maxFiltSize, m_imageHeight),IPL_DEPTH_8U, 1 );		  // For wrapped Mask Image
	maskInt = cvCreateImage( cvSize(m_imageWidth+m_maxFiltSize+1, m_imageHeight+1), IPL_DEPTH_32S, 1 ); // for Integral Image:Masks

	m_fullFeature = (int *) malloc(8*GetFeatureLength()*sizeof(int));
	m_tempFullFeature = (int *) malloc(8*GetFeatureLength()*sizeof(int));

	float extraFactor = 2.0f*m_baseScale/4;

/*
	m_coreThreshold[0] = cvRound(30*extraFactor);			// 6
	m_coreThreshold[1] = cvRound(20*extraFactor);
	m_coreThreshold[2] = cvRound(88*extraFactor);			// 12
	m_coreThreshold[3] = cvRound(48*extraFactor);
	m_coreThreshold[4] = cvRound(145*extraFactor);			// 18
	m_coreThreshold[5] = cvRound(83*extraFactor);
	m_coreThreshold[6] = cvRound(237*extraFactor);			// 24
	m_coreThreshold[7] = cvRound(118*extraFactor);
*/
/*
	m_coreThreshold[0] = cvRound(40*extraFactor);			// 6
	m_coreThreshold[1] = cvRound(20*extraFactor);
	m_coreThreshold[2] = cvRound(94*extraFactor);			// 12
	m_coreThreshold[3] = cvRound(48*extraFactor);
	m_coreThreshold[4] = cvRound(144*extraFactor);			// 18
	m_coreThreshold[5] = cvRound(83*extraFactor);
	m_coreThreshold[6] = cvRound(224*extraFactor);			// 24
	m_coreThreshold[7] = cvRound(118*extraFactor);
*/
	m_coreThreshold[0] = cvRound(11*extraFactor);			// 6
	m_coreThreshold[1] = cvRound(14*extraFactor);
	m_coreThreshold[2] = cvRound(27*extraFactor);			// 12
	m_coreThreshold[3] = cvRound(33*extraFactor);
	m_coreThreshold[4] = cvRound(48*extraFactor);			// 18
	m_coreThreshold[5] = cvRound(65*extraFactor);
	m_coreThreshold[6] = cvRound(74*extraFactor);			// 24
	m_coreThreshold[7] = cvRound(86*extraFactor);

	m_sortFunction[0] = comparef0;
	m_sortFunction[1] = comparef1;
	m_sortFunction[2] = comparef2;
	m_sortFunction[3] = comparef3;
	m_sortFunction[4] = comparef4;
	m_sortFunction[5] = comparef5;
	m_sortFunction[6] = comparef6;
	m_sortFunction[7] = comparef7;

	for( int i = 0 ; i < 256 ; i++ )
		m_lut[i] = number_of_ones(i);

}

EyeFeatureServer::~EyeFeatureServer(void)
{
	cvReleaseImage(&imgInt);
	cvReleaseImage(&imgWrap);
	cvReleaseImage(&maskWrap);
	cvReleaseImage(&maskInt);

	free(m_fullFeature);
	free(m_tempFullFeature);

}

void EyeFeatureServer::horizontal_border_wrap(IplImage *in, IplImage *out)
{
	unsigned char *inptr=0;
	unsigned char *outptr = 0;
	int inStep = 0, outStep = 0;
	CvSize inRoi, outRoi;

	cvGetRawData(in, &inptr, &inStep, &inRoi);
	cvGetRawData(out, &outptr, &outStep, &outRoi);

	int len = m_maxFiltSize/2;

	for(int ii=0;ii<inRoi.height;ii++)
	{
		unsigned char *iptr = inptr + ii*inStep;
		unsigned char *optr = outptr + ii*outStep;

		memcpy(optr, iptr + inRoi.width-len, len);
		memcpy(optr+len, iptr, inRoi.width);
		memcpy(optr + inRoi.width + len, iptr, len);
	}
}

int EyeFeatureServer::getThreshold(int *feature, int idx)
{
	return m_coreThreshold[idx];

	int featureLength = GetFeatureRowLength()*GetFeatureNumRows()/GetFeatureByteSize();
	qsort(feature, featureLength, GetFeatureByteSize()*sizeof(int), m_sortFunction[idx]);

	return feature[ GetFeatureByteSize()*(featureLength >> 2) + idx ];
}
float EyeFeatureServer::GetRobustFeatureVariance(int idx)
{
	//printf("GetRobustFeatureVariance...\n");
	float *normalizedFeature = (float *) m_fullFeature;
	//printf("GetRobustFeatureVariance...%f\n", *normalizedFeature );
	int featureLength = GetFeatureRowLength()*GetFeatureNumRows()/GetFeatureByteSize();
	//printf("GetRobustFeatureVariance...featureLength..%d\n", featureLength);
	qsort(normalizedFeature, featureLength, 8*sizeof(float), m_sortFunction[idx]);
	//printf("After qsort GetRobustFeatureVariance...\n");
	return normalizedFeature[ 8*(featureLength >> 1) + idx ];

}
int EyeFeatureServer::FeatureCoring(unsigned char *feature, unsigned char *tag)
{
	int featureRowLength = GetFeatureRowLength();
	int bytesPerSample = GetFeatureByteSize();

	memcpy(m_tempFullFeature, m_fullFeature, GetFeatureLength()*sizeof(int));
	printf("\n");

	for(int idx=0;idx<bytesPerSample;idx++)	// over all scales for a sample point
	{
		int threshold = getThreshold(m_tempFullFeature, idx);
		printf("%d ", threshold);
		for(int j=0; j<8; j++)	// over all rows
		{
			int *fr = m_fullFeature + j*featureRowLength;
			unsigned char *tr = tag + j*featureRowLength;

			for(int i=0; i<featureRowLength; i+=bytesPerSample)
				if( abs(fr[i + idx]) < threshold)
					tr[i+idx] = '1';
		}
	}
	printf("\n");
	return 1;
}

int EyeFeatureServer::ExtractFeatures(IplImage *img, IplImage *mask, unsigned char *feature, unsigned char *tag)
{
	int width = mask->width;
	int maskPerCent = m_maskPerCent * 255;
	int numEvenFeatures[4] = {0, 0, 0, 0}, numOddFeatures[4] = {0, 0, 0, 0};

	//Image wrapping for Original Iris Image
	horizontal_border_wrap(img, imgWrap);	horizontal_border_wrap(mask, maskWrap);	

	cvIntegral(imgWrap, imgInt);cvIntegral(maskWrap, maskInt);	
	int start = 0;

	for(int i=0;i<4;i++)
		m_evenVariance[i] = m_oddVariance[i] = 0.0f;


	// computing features imgInt is the wrapped Iris maskInt is the wrapperd mask

	int* imgIntPtr = 0;
	int* maskIntPtr = 0;
	int inStep = 0, maskStep = 0;

	cvGetRawData(imgInt, (unsigned char **)&imgIntPtr, &inStep);
	inStep /= sizeof(int);

	cvGetRawData(maskInt,(unsigned char **)&maskIntPtr, &maskStep);
	maskStep /= sizeof(int);

	int border_size = m_maxFiltSize/2;

	float *normalizedFeature = (float *) m_fullFeature;


	for(int rr=0;rr<m_imageHeight/m_vSample;rr++)
	{
		int *iptr  = imgIntPtr + m_vSample*rr*inStep + border_size;
		int *iptr8 = iptr + m_vSample*inStep;

		int *mptr  = maskIntPtr + m_vSample*rr*maskStep + border_size;
		int *mptr8 = mptr + m_vSample*maskStep;


		for(int jj=0; jj< width; jj+=m_hSample, iptr+=m_hSample, iptr8+=m_hSample, mptr+=m_hSample, mptr8+=m_hSample)
		{
			unsigned char enc_feat = 0;
			unsigned char enc_mask = 0;

			char a1 = 0x1, a2 = 0x2;

			for(int i=0;i<4;i++, a1 <<= 2, a2 <<= 2)
			{
				int D, M;
				int sc = m_baseScale * (i+1);
				int sc2 = sc << 1;

				//Even Filter 1

				D = 3*(iptr[-sc] - 2*iptr[0] + iptr[sc] - iptr8[-sc] + 2*iptr8[0] - iptr8[sc]);
				D += (iptr8[sc2] + iptr[sc] - iptr8[sc] - iptr[sc2]) - (iptr[-sc2] + iptr8[-sc] - iptr8[-sc2] - iptr[-sc]);
				D = D >> 1;

				if(D > 0)enc_feat |= a1 ;
				M = mptr[-sc2] - mptr[sc2] - mptr8[-sc2] + mptr8[sc2];
				if(M > maskPerCent || abs(D) < m_coreThreshold[i*2]) enc_mask |= a1;

				float mean = m_featureNormalize? iptr8[sc2] + iptr[-sc2] - iptr[sc2] - iptr8[-sc2] : 1.0f;
				mean = 1.0f/(mean*mean);

				float featureVal = (float) D * (float) D *  mean;
				*normalizedFeature++ = featureVal;

				if(M == 0)
				{
					m_evenVariance[i] += featureVal;
					numEvenFeatures[i]++;
				}

				// ODD Filter 1

				D = -iptr[-sc2] + 2*iptr[-sc] - 2*iptr[sc] + iptr[sc2] + iptr8[-sc2] - 2*iptr8[-sc] + 2*iptr8[sc] - iptr8[sc2];
				if(D > 0)enc_feat |= a2 ;

				M = mptr[-sc2] - mptr[sc2] - mptr8[-sc2] + mptr8[sc2];
				if(M > maskPerCent || abs(D) < m_coreThreshold[i*2+1]) enc_mask |= a2;

				featureVal = (float) D * (float) D *  mean;
				*normalizedFeature++ = featureVal;


				if(M == 0)
				{
					m_oddVariance[i] += featureVal;
					numOddFeatures[i]++;
				}
			}

			feature[start] = enc_feat;
			tag[start++] = ~enc_mask;	//

		}
	}

	for(int i=0;i<4;i++)
	{
		m_evenVariance[i] /= numEvenFeatures[i];
		m_oddVariance[i] /= numOddFeatures[i];
	}

//	FeatureNonMaximalSuppression(feature, tag);
//	FeatureCoring(feature, tag);
	return(0);
}


