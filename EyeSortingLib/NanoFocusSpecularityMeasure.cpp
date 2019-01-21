#include "NanoFocusSpecularityMeasure.h"
#include <opencv/cxcore.h>
#include <stdio.h>
#include "opencv/highgui.h"
#include <vector>
#include <algorithm>

NanoFocusSpecularityMeasure::NanoFocusSpecularityMeasure():m_expectedMaxValue(255),m_oldHaloScore(-1),m_HaloPixelCount(-1)
{
}

NanoFocusSpecularityMeasure::~NanoFocusSpecularityMeasure(void)
{
}

int NanoFocusSpecularityMeasure::ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius)
{
	// Do small search for max specularity:
	unsigned char maxValue = 0, peakThreshold = 0;
	int nx = cvRound(center.x), ny = cvRound(center.y);
	int oy, ox;

	for(int i = ny-8; i < ny+10; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-8; j < nx+10; j++)
		{
			unsigned char value = iptr[j]; 
			if(value > maxValue)
			{
				maxValue = value;
				oy = i;
				ox = j;
			}
		}
	}
	
	if(maxValue < m_expectedMaxValue)
	//if(maxValue != m_expectedMaxValue)	
	{
		for(int i = ny-radius; i < ny+radius; i++)
		{
			unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
			for(int j = nx-radius; j < nx+radius; j++)
			{
				unsigned char value = iptr[j]; 
				if(value > maxValue)
				{
					maxValue = value;
					oy = i;
					ox = j;
				}
			}
		}
	}

	if(maxValue < m_expectedMaxValue)
	//if(maxValue != m_expectedMaxValue)
	{
		for(int i = ny-2*radius; i < ny+2*radius; i++)
		{
			unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
			for(int j = nx-2*radius; j < nx+2*radius; j++)
			{
				unsigned char value = iptr[j]; 
				if(value > maxValue)
				{
					maxValue = value;
					oy = i;
					ox = j;
				}
			}
		}
	}

	if(maxValue < m_expectedMaxValue)
		return 0;

	nx = ox; ny = oy;

	// computing the mean value 
	int meanValueX = 0, meanValueY = 0;
	int varX = 0, varY = 0;
	int count = 0;
	
	//int peak_threshold = (31*maxValue) >> 5;
	int peak_threshold = (31*m_expectedMaxValue) >> 5;

	for(int i = ny-radius; i <= ny+radius; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-radius; j < nx+radius; j++)
		{
			int value = iptr[j];

			if(value > peak_threshold)
			{
				meanValueX += j;
				meanValueY += i;
				varX += j*j;
				varY += i*i;
				count ++;
			}
		}
	}
	
	ny = (meanValueY + (count>>1))/count;
	nx = (meanValueX + (count>>1))/count;

	meanValueX = 0; meanValueY = 0;
	varX = 0; varY = 0;
	count = 0;

	for(int i = ny-radius; i <= ny+radius; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep;
		for(int j = nx-radius; j < nx+radius; j++)
		{
			int value = iptr[j];

			if(value > peak_threshold)
			{
				meanValueX += j;
				meanValueY += i;
				varX += j*j;
				varY += i*i;
				count++;
			}
		}
	}

	center = cvPoint2D32f((float) meanValueX / count, (float) meanValueY / count);
	var = cvPoint2D32f((float) varX/count - center.x * center.x, (float) varY/count - center.y * center.y);

	return count;
}

// implementation assumes m = 1;
int NanoFocusSpecularityMeasure::check_image(IplImage *input)
{
	m_specularityCentroid.x = m_specularityCentroid.y = 0.0f;
	m_specularityROI.x = m_specularityROI.y = m_specularityROI.width = m_specularityROI.height = -1;

	CvPoint2D32f pt={0}, specvar={0};

	pt.x = input->width/2.0f; pt.y = input->height/2.0f;
	
	int count = ComputeSpecularityMetrics(input, pt, specvar, 28);

	m_specularityCentroid = pt;

	specvar.x = 3.0*sqrt(specvar.x);
	specvar.y = 3.0*sqrt(specvar.y);

	int tempWidth = 2*cvCeil(specvar.x)+1;
	int tempHeight = 2*cvCeil(specvar.y)+1;

	m_specularityROI = cvRect(pt.x-tempWidth/2,pt.y-tempHeight/2,tempWidth,tempHeight);

	return count;
}


float NanoFocusSpecularityMeasure::ComputeHaloScore(IplImage *img1) 
{
 //result.x = sum, result.y = count, result.z = haloScore
	float result = -1.0f;
	m_HaloPixelCount = -1;
	if(check_image(img1) == 0) return result;

	CvPoint2D32f specCentroid = GetSpecularityCentroid();
	CvRect specROI = GetSpecularityROI();

	if (specROI.width<=1 || specROI.width>640 || specROI.height<=1 || specROI.height>480){
	//	printf("\n\n<<<Invalid values of specROI>>>");
	}
	else
	{
		int sum=0;
		int count = 0;
				
		for(int i=specROI.y;i<specROI.y + specROI.height;i++)
		{
			unsigned char *iptr = (unsigned char *) (img1->imageData + img1->widthStep*i + specROI.x);
			for(int j=0;j<specROI.width;j++,iptr++)
			{
				int max=0;
				for(int k=-1;k<2;k++)
				{
					unsigned char *_iptr = iptr + img1->widthStep*k -1;
					for(int l=0; l<3; l++)
					{
						if (*(_iptr+l) > max)
							max = *(_iptr+l);

					}
				}
				if (max > *iptr && max == m_expectedMaxValue) {
					sum += *iptr;
					count += 1;
				}
			}
		}
		float val = -1.0;
		if(count>0){
			val = sum*1.0/count;
		}
		m_HaloPixelCount = count;
		result = val;
	}
	return result;
}

float NanoFocusSpecularityMeasure::ComputeHaloScoreTopPointsNano(IplImage *img1, int specValue, int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh) 
{
	float result = -400.0f;
	m_HaloPixelCount = -1;
	if(check_image(img1) == 0) return result;

	CvPoint2D32f specCentroid = GetSpecularityCentroid();
	CvRect specROI = GetSpecularityROI();

	if (specROI.width<=1 || specROI.width>640 || specROI.height<=1 || specROI.height>480){
		printf("\n\n<<<Invalid values of specROI>>>");
	}
	else
	{
		int sum=0;
		int count = 0;
		float sumOfDiffs = 0;
		int diff = 0;
		std::vector<int> sdBasedHalo;
		for(int i=specROI.y;i<specROI.y + specROI.height;i++)
		{
			unsigned char *iptr = (unsigned char *) (img1->imageData + img1->widthStep*i + specROI.x);
			for(int j=0;j<specROI.width;j++,iptr++)
			{
				int max=0;
				for(int k=-1;k<2;k++)
				{
					unsigned char *_iptr = iptr + img1->widthStep*k -1;
					for(int l=0; l<3; l++)
					{
						if (*(_iptr+l) > max)
							max = *(_iptr+l);
						
					}
				}
				if (max > *iptr && max >= m_expectedMaxValue && *iptr<m_expectedMaxValue) {
				//if (max > *iptr && max == m_expectedMaxValue) {
					sum += *iptr;
					count += 1;
					sdBasedHalo.push_back(*iptr);
				}
			}
		}
		float val = -400.0f;
		if(noOfPixelsToConsider>0) noOfPixelsToConsider = count/noOfPixelsToConsider;		
		if(count>0 && noOfPixelsToConsider>0){
			val = sum*1.0/count;
			if(val > HaloThresh || val < 0) return -400.0f; // Marking it Bad Image
			std::sort(sdBasedHalo.begin(), sdBasedHalo.end());
			for(int i=0; i<noOfPixelsToConsider; i++)			
			{
				diff = (specValue - sdBasedHalo[i]);
				sumOfDiffs += diff;
			}
			m_modifiedBottomPointsDiff = (sumOfDiffs/(noOfPixelsToConsider));

			sumOfDiffs = 0;
			diff = 0;

			int noOfBottomPoints = 0;
			int noOfTopPoints = 0;
			float intensitySumTopPoints = 0;
			int topthreshold = (((100-topPixelsPercentage)/100.0)*specValue);
			for(int i=0; i<count; i++)
			{
				if(sdBasedHalo[i]<intensityThreshBP)
					++noOfBottomPoints;
				if(sdBasedHalo[i]>topthreshold)
				{
					intensitySumTopPoints += sdBasedHalo[i];
					++noOfTopPoints;
				}
			}
			m_noOfBottomPoints = noOfBottomPoints;
			m_noOfTopPoints = noOfTopPoints;	
			m_avgIntensityTP = intensitySumTopPoints/count;
			sdBasedHalo.clear();
			result = m_modifiedBottomPointsDiff - m_avgIntensityTP;
		}
		m_HaloPixelCount = count;
		m_oldHaloScore = val;
		if ( count<=0 )
			result = -400.0f; // Marking it Bad Image
	}
	return result;
}

float NanoFocusSpecularityMeasure::ComputeHaloScoreTopPointsPico(IplImage *img1, int specValue, int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh) 
{
	float result = -400.0f;
	m_HaloPixelCount = -1;
	if(check_image(img1) == 0) return result;

	CvPoint2D32f specCentroid = GetSpecularityCentroid();
	CvRect specROI = GetSpecularityROI();

	if (specROI.width<=1 || specROI.width>640 || specROI.height<=1 || specROI.height>480){
		printf("\n\n<<<Invalid values of specROI>>>");
	}
	else
	{
		int sum=0;
		int count = 0;
		float sumOfDiffs = 0;
		int diff = 0;
		std::vector<int> sdBasedHalo;
		for(int i=specROI.y;i<specROI.y + specROI.height;i++)
		{
			unsigned char *iptr = (unsigned char *) (img1->imageData + img1->widthStep*i + specROI.x);
			for(int j=0;j<specROI.width;j++,iptr++)
			{
				int max=0;
				for(int k=-1;k<2;k++)
				{
					unsigned char *_iptr = iptr + img1->widthStep*k -1;
					for(int l=0; l<3; l++)
					{
						if (*(_iptr+l) > max)
							max = *(_iptr+l);
						
					}
				}
				if (max > *iptr && max == m_expectedMaxValue) {
					sum += *iptr;
					count += 1;
					sdBasedHalo.push_back(*iptr);
				}
			}
		}
		float val = -400.0;
		if(count>0 && noOfPixelsToConsider<count){
			val = sum*1.0/count;
			if(val > HaloThresh || val < 0) return -400.0f; // Marking it Bad Image
			std::sort(sdBasedHalo.begin(), sdBasedHalo.end());
			for(int i=0; i<noOfPixelsToConsider; i++)			
			{
				diff = (specValue - sdBasedHalo[i]);
				sumOfDiffs += diff;
			}
			m_modifiedBottomPointsDiff = (sumOfDiffs/(noOfPixelsToConsider));

			sumOfDiffs = 0;
			diff = 0;

			int noOfBottomPoints = 0;
			int noOfTopPoints = 0;
			float intensitySumTopPoints = 0;
			int topthreshold = (((100-topPixelsPercentage)/100.0)*specValue);
			for(int i=0; i<count; i++)
			{
				if(sdBasedHalo[i]<intensityThreshBP)
					++noOfBottomPoints;
				if(sdBasedHalo[i]>topthreshold)
				{
					intensitySumTopPoints += sdBasedHalo[i];
					++noOfTopPoints;
				}
			}
			m_noOfBottomPoints = noOfBottomPoints;
			m_noOfTopPoints = noOfTopPoints;	
			m_avgIntensityTP = intensitySumTopPoints/count;
			sdBasedHalo.clear();
			result = m_modifiedBottomPointsDiff + m_noOfBottomPoints - m_avgIntensityTP;
		}
		m_HaloPixelCount = count;
		m_oldHaloScore = val;
		if ( count<=0 )
			result = -400.0f; // Marking it Bad Image
	}
	return result;
}
