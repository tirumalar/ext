#include "NanoFocusSpecularityBasedSpoofDetector.h"
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <algorithm>
//#include "ippi.h"
//#include "ippsr.h"
//#include "ipps.h"
//#include "ippcv.h"
#include <stdio.h>
using namespace std;

NanoFocusSpecularityBasedSpoofDetector::NanoFocusSpecularityBasedSpoofDetector(int width, int height): m_width(width), m_height(height),
m_expectedMaxValue(255)
{
	m_hist = (Ipp32f *) malloc(1000*sizeof(float));
	low_threshold = std::make_pair(0.75f, 0.85f);
	high_threshold = std::make_pair(1.25f, 1.15f);
}

NanoFocusSpecularityBasedSpoofDetector::~NanoFocusSpecularityBasedSpoofDetector(void)
{
	if(m_hist)			free(m_hist);	m_hist = 0;
}

int NanoFocusSpecularityBasedSpoofDetector::ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius)
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
	
	if(maxValue != m_expectedMaxValue)
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

	if(maxValue != m_expectedMaxValue)
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
	
	int peak_threshold = (31*maxValue) >> 5;

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
NanoFocusSpecularityBasedSpoofDetector::MapType NanoFocusSpecularityBasedSpoofDetector::peak_detector(float *hist, float *temp, int len, int m, int l)
{
	MapType sortMap;

	for(int i=0; i<2*l+1; i++)
		temp[l] += hist[i];

	// running average. Since we are using for only detecting peak we don't need to divide by 2*l+1;
	for(int i=l+1;i<len-l;i++)
		temp[i] = temp[i-1] - hist[i-l-1] + hist[i+l];

	// local 3 pixel window peak detection (i.e assumes m = 1)

	for(int i=l+m; i<len-l-m;i++)
		if(temp[i] > temp[i-1] && temp[i] > temp[i+1])
			sortMap.insert(std::make_pair(hist[i], i-l-m));

	return sortMap;
}

float NanoFocusSpecularityBasedSpoofDetector::strip_focus3(IplImage *img, CvPoint pt, CvPoint2D32f specvar, CvSize sz, float *hist)
{
	m_irisPupilIntensities = std::make_pair(-1, -1);
	
	/* The following hist is actually a vertical strip average */
	memset(hist, 0, sizeof(float)*256);

	sz.height /= 2;

	for(int i=0;i<sz.height;i++)
	{
		unsigned char *iptr = (unsigned char *) (img->imageData + img->widthStep * (pt.y-sz.height/4+i) + pt.x-sz.height/2);
		for(int j=0;j<sz.width;j++, iptr++)
			hist[j] += *iptr;
	}

	int minloc = 0;

	float scale = 1.0f/sz.height;
	for(int i=0;i<sz.width;i++)
	{
		hist[i] *= scale;
		if( hist[minloc] > hist[i] )
			minloc = i;
	}

	m_irisPupilIntensities.first = hist[minloc];

	return 0.0f;
}

float NanoFocusSpecularityBasedSpoofDetector::strip_focus2(IplImage *img, CvPoint pt, CvPoint2D32f specvar, CvSize sz, float *hist)
{
	float mean = 0.0f;
	float var = 0.0f;
	int count=0;

	m_irisPupilIntensities = std::make_pair(-1, -1);

	specvar.x = 3.0*sqrt(specvar.x);
	specvar.y = 3.0*sqrt(specvar.y);

	int xo = pt.x - cvCeil(specvar.x) - (pt.x-sz.width/4);
	int yo = pt.y - cvCeil(specvar.y) - (pt.y-sz.height/4);
	int xb = pt.x + cvCeil(specvar.x) - (pt.x-sz.width/4) + 1;
	int yb = pt.y + cvCeil(specvar.y) - (pt.y-sz.height/4) + 1;

	//Setting value of Specularity ROI
	//m_specularityROI = cvRect(pt.x+xo,pt.y+yo,xb-xo,yb-yo);
	int tempWidth = xb-xo;
	int tempHeight = yb-yo;
	m_specularityROI = cvRect(pt.x-tempWidth/2,pt.y-tempHeight/2,tempWidth,tempHeight);

	unsigned char threshold = (1*m_expectedMaxValue) >> 1;
	float max_score = 0;

	for(int i=0;i<sz.height;i++)
	{
		unsigned char *iptr = (unsigned char *) (img->imageData + img->widthStep * (pt.y-sz.height/4+i) + pt.x-sz.width/4);
		for(int j=0;j<sz.width;j++, iptr++)
		{
			hist[*iptr+3]++;	// intensity histogram
			if(!(j > xo && j < xb && i > yo && i < yb && *iptr > threshold) && *iptr != m_expectedMaxValue)
			{
				mean += (float) *iptr;
				var += (float) (*iptr) * (float) (*iptr);
				count++;
			}
			if(max_score < hist[*iptr+3] && *iptr != m_expectedMaxValue)
				max_score = hist[*iptr+3];
		}
	}

	mean /= count;
	var = sqrt(var/count - (mean * mean));
	
	hist[2] = std::min(hist[3]*0.75f, hist[4]);
	hist[1] = std::min(hist[3]*0.75f*0.75f, hist[5]);
	hist[0] = std::min(hist[3]*0.75f*0.75f*0.75f, hist[6]);

	Ipp8u lpeaks[259]={0};
	Ipp8u *peaks = lpeaks;
#ifdef __linux__

	float temp[259]={0};
	MapType sortMap =  peak_detector(hist,temp,259,1,1);
//	printf("SortMap size %d \n",sortMap.size());
	for(MapType::iterator sit=sortMap.begin(); sit != sortMap.end(); sit++){
			lpeaks[sit->second+2] = 1;
//			printf("%f , %d\n",sit->first,sit->second);
	}


#else
	ippsFindPeaks_32f8u(hist, peaks, 259, 1, 1);

#endif

	hist +=3;
	peaks += 3;

	typedef std::map<int, int > MapType;

	float scoreThreshold = max_score/8.0;
	count = 0;

	for(int i=0;i<256;i++)
	{
		if(hist[i] < scoreThreshold)
			peaks[i] = 0;

		count += peaks[i];
		if(peaks[i])	peaks[i] = (unsigned char) std::min(255, cvRound(hist[i]*255/max_score));
	}

	max_score = 255.0;

	if(count < 2)	return 0.0f;

	/* The following hist is actually a vertical strip average */
	memset(hist, 0, sizeof(float)*256);

	sz.height /= 2;

	for(int i=0;i<sz.height;i++)
	{
		unsigned char *iptr = (unsigned char *) (img->imageData + img->widthStep * (pt.y-sz.height/4+i) + pt.x-sz.height/2);
		for(int j=0;j<sz.width;j++, iptr++)
			hist[j] += *iptr;
	}

	int minloc = 0;

	// darkest point
	float scale = 1.0f/sz.height;
	for(int i=0;i<sz.width;i++)
	{
		hist[i] *= scale;
		if( hist[minloc] > hist[i] )
			minloc = i;
	}

	std::map<int, int> distMap;

	int minVal = cvRound(hist[minloc]);
	int lastSmallestVal =cvFloor(hist[minloc])-5;
	int lastLoc = minloc;

	for(int idx =0; idx < 4; idx++)
	{
		std::map<int, int> ldistMap;
		for(int i=sz.width-1;i>=lastLoc;i--)
		{
			int val = cvFloor(hist[i]);
			if( peaks[val] > 0 && val > lastSmallestVal)	// if the intensity value is a peak
				ldistMap[val] = i;
			else if( peaks[val+1] > 0 && val+1 > lastSmallestVal)	// if the intensity value is a peak
				ldistMap[val+1] = i;
			else if( peaks[val+2] > 0 && val+2 > lastSmallestVal)	// if the intensity value is a peak
				ldistMap[val+2] = i;
			else if( val > 0 && peaks[val-1] > 0 && val-1 > lastSmallestVal)	// if the intensity value is a peak
				ldistMap[val-1] = i;
			else if( val > 1 && peaks[val-2] > 0 && val-2 > lastSmallestVal)	// if the intensity value is a peak
				ldistMap[val-2] = i;
		}

		if(!ldistMap.empty())
		{
			lastSmallestVal = ldistMap.begin()->first;
			lastLoc = ldistMap.begin()->second;

			distMap[lastSmallestVal] = lastLoc;

		}
	}

	// distmap is a mapping of peak intensity and its location

	
	double best_cost = 2.0;
	std::pair<int, int> best_pair = std::make_pair(0,0);

	double dvar = var * 0.5;

	double psigma = 0.5/(dvar*dvar);	
	double isigma = 0.5/(dvar*dvar);	
	double sigma2 = 0.5/2.0;		// high variance for the difference in peak value
	double sigma3 = 0.5/0.25;		// tight variance for one of the peaks close to max.

	for(MapType::iterator sit=distMap.begin(); sit != distMap.end(); sit++)
	{
		for(MapType::iterator pit=sit; ++pit != distMap.end(); )
		{
			if(sit->first < mean && pit->first > 0.5*mean && pit->first - sit->first > 0.5*var && pit->second - sit->second < 100 && sit->first < 100)
			{
				double cost = 0;
				// smaller intensity values of pupil are preferred
				double dx = (sit->first - minVal)/256.0;
				dx *= dx*sigma2;
				cost += dx;


				// at least one of them must have a large peak
				dx = 1.0 - std::max(peaks[sit->first], peaks[pit->first])/max_score;
				dx *= dx*sigma3;
				cost += dx;


				if(cost < best_cost)
				{
					best_pair = std::make_pair(sit->first, pit->first);
					best_cost = cost;
				}
			}
		}

	}

	int pupil_peak = best_pair.first;
	int iris_peak = best_pair.second;
	
	if(pupil_peak == 0 && iris_peak == 0)	return 0.0f;

	m_irisPupilIntensities = best_pair;

	float avg_min = pupil_peak, avg_max = iris_peak;

	float avg_val = 0.5*(pupil_peak + iris_peak);

	int maxloc = minloc;

	for(int i=minloc;i<sz.width;i++)
	{
		if( hist[maxloc] < hist[i] )
			maxloc = i;
	}

	if(maxloc <= minloc)	return 0.0f;


	int loc = 0;

	for(int i=minloc;i<maxloc;i++)
	{
		if(hist[i] > avg_val)
		{
			loc = i;
			break;
		}
	}
	
	if(loc < 3 || loc > sz.width-3)	
		return 0.0;

	float average_grad = (hist[loc+1] - hist[loc-2]);

	for(int i=loc-3;i<=loc+3;i++)
	{
		float val = hist[i+1] - hist[i-2];
		if(average_grad < val)
			average_grad = val;
	}

	average_grad /= 3.0f;

	float contrast = avg_max - avg_min;

	return sz.height*average_grad / contrast;
}


float NanoFocusSpecularityBasedSpoofDetector::strip_focus(IplImage *img, CvPoint pt, CvSize sz, float *hist)
{
	float mean = 0.0f;
	float var = 0.0f;
	for(int i=0;i<sz.height;i++)
	{
		unsigned char *iptr = (unsigned char *) (img->imageData + img->widthStep * (pt.y-sz.height/4+i) + pt.x-sz.height/2);
		for(int j=0;j<sz.width;j++, iptr++)
		{
			hist[j] += *iptr;
			mean += *iptr;
			var += (*iptr) * (*iptr);
		}
	}

	mean /= (sz.height * sz.width);
	var = sqrt(var/(sz.height * sz.width) - mean * mean);
	int minloc = 0;

	for(int i=0;i<sz.width;i++)
	{
		if( hist[minloc] > hist[i] )
			minloc = i;
	}

	int maxloc = minloc;

	for(int i=minloc;i<sz.width;i++)
	{
		if( hist[maxloc] < hist[i] )
			maxloc = i;
	}

	if(maxloc <= minloc)	return 0.0f;

	int countMin = 0, countMax = 0;
	float avg_min = 0.0f, avg_max = 0.0f;

	for(int i=max(0, minloc-3);i<min(minloc+3, sz.width-1);i++)
	{
		avg_min += hist[i];
		countMin++;
	}
	
	avg_min /= (countMin);

	for(int i=max(0, maxloc-3);i<min(maxloc+3, sz.width-1);i++)
	{
		avg_max += hist[i];
		countMax++;
	}

	avg_max /= (countMax);

	if(avg_max < 1.25*avg_min)	return 0.0;

	float avg_val = (avg_max + avg_min)*0.5f;

	int loc = 0;

	for(int i=minloc;i<maxloc;i++)
	{
		if(hist[i] > avg_val)
		{
			loc = i;
			break;
		}
	}
	
	if(loc < 3 || loc > sz.width-3)	
		return 0.0;

	float average_grad = (hist[loc+1] - hist[loc-2]);

	for(int i=loc-3;i<=loc+3;i++)
	{
		float val = hist[i+1] - hist[i-2];
		if(average_grad < val)
			average_grad = val;
	}

	average_grad /= 3.0f;

	float contrast = avg_max - avg_min;

	return average_grad / contrast;
}

std::pair<float, float> NanoFocusSpecularityBasedSpoofDetector::check_image(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
{
	IplImage input;

	cvInitImageHeader(&input, cvSize(w,h), IPL_DEPTH_8U, 1);
	cvSetData(&input, img, widthStep);

	m_irisPupilIntensities = std::make_pair(-1, -1);

	CvPoint2D32f pt={0}, var={0};

	pt.x = input.width/2.0f; pt.y = input.height/2.0f;
	
	int count = ComputeSpecularityMetrics(&input, pt, var, 28);

	m_specularityCentroid = pt;

	std::pair<float, float> measure = std::make_pair(0.0f,0.0f);

	if(count == 0 || max(var.x, var.y)/min(var.x, var.y) > 3.0f)
		return measure;

	memset(m_hist, 0, sizeof(float)*1000);

	CvSize sz = cvSize(160, 20);

	if(ip.z * pp.z > 1.0f)
	{
		float min_dist  = pp.z - pt.x + pp.x;
		float max_dist  = ip.z - pt.x + ip.x;
		float len = (max_dist + min_dist)*0.5f;
		len *= 4.0f/3.0f;

		if(min_dist < 0.0f)	return measure;

		sz = cvSize(10, cvRound(min(max(60.0f, len), 120.0f)));
	}

	measure.first = (float) count;
	
	measure.second = strip_focus2(&input, cvPoint(cvRound(pt.x), cvRound(pt.y)), var, sz, m_hist);

	return measure;
}

std::pair<float, float> NanoFocusSpecularityBasedSpoofDetector::check_image2(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
{
	IplImage input;

	cvInitImageHeader(&input, cvSize(w,h), IPL_DEPTH_8U, 1);
	cvSetData(&input, img, widthStep);

	m_irisPupilIntensities = std::make_pair(-1, -1);

	CvPoint2D32f pt={0}, var={0};

	pt.x = input.width/2.0f; pt.y = input.height/2.0f;
	
	int count = ComputeSpecularityMetrics(&input, pt, var, 28);

	m_specularityCentroid = pt;

	std::pair<float, float> measure = std::make_pair(0.0f,0.0f);

	if(count == 0 || max(var.x, var.y)/min(var.x, var.y) > 3.0f)
		return measure;

	memset(m_hist, 0, sizeof(float)*1000);

	CvSize sz = cvSize(160, 20);

	if(ip.z * pp.z > 1.0f)
	{
		float min_dist  = pp.z - pt.x + pp.x;
		float max_dist  = ip.z - pt.x + ip.x;
		float len = (max_dist + min_dist)*0.5f;
		len *= 4.0f/3.0f;

		if(min_dist < 0.0f)	return measure;

		sz = cvSize(10, cvRound(min(max(60.0f, len), 120.0f)));
	}

	measure.first = (float) count;
	
	measure.second = strip_focus3(&input, cvPoint(cvRound(pt.x), cvRound(pt.y)), var, sz, m_hist);

	return measure;
}

int NanoFocusSpecularityBasedSpoofDetector::check(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
{
	std::pair<float, float> score = check_image(img, w, h, widthStep, ip, pp, fmeas);
	int isSpoof = 2;

	if(score.first > 0.0f && score.second > 0.0f)
	{
		for(std::list<std::pair<float, float> >::iterator lit = score_list.begin(); lit != score_list.end(); lit++)
		{
			std::pair<float, float> ref_score = *lit;
			std::pair<float, float> ratio = std::make_pair(ref_score.first / score.first, ref_score.second / score.second);

			if((ratio.first < low_threshold.first && ratio.second < low_threshold.second) ||
			   (ratio.first > high_threshold.first && ratio.second > high_threshold.second))
			{
				if(isSpoof == 0)
				{
					isSpoof = 3;
					break;
				}
				else
					isSpoof = 1;
			}

			if((ratio.first < low_threshold.first && ratio.second > high_threshold.second) ||
			   (ratio.first > high_threshold.first && ratio.second < low_threshold.second))
			{
				if(isSpoof == 1)
				{
					isSpoof = 3;
					break;
				}
				else
					isSpoof = 0;
			}
		}

		score_list.push_back(score);
	}

	return isSpoof;
}

void NanoFocusSpecularityBasedSpoofDetector::reset()
{
	score_list.clear();
}

CvPoint3D32f NanoFocusSpecularityBasedSpoofDetector::ComputeHaloScore(IplImage *img1, unsigned char specValue)
{
	CvPoint3D32f result = cvPoint3D32f(-1,-1,-1.0); //result.x = sum, result.y = count, result.z = haloScore
	CvPoint2D32f specCentroid = GetSpecularityCentroid();
	CvRect specROI = GetSpecularityROI();
	if (specROI.width<0 || specROI.width>640 || specROI.height<0 || specROI.height>480){
		printf("<<<Invalid values of specROI>>>");
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
				//printf("\nMax=%d",max);
				if (max > *iptr && max == specValue) {
					sum += *iptr;
					count += 1;
				}
			}
		}
		//printf("\nSum=%d Count=%d HaloScore=%f",sum,count,sum*1.0/count);
		float val = -1.0;
		if(count>0){
			val = sum*1.0/count;
		}
		result = cvPoint3D32f(sum,count,val);
		//printf("specROI.w = %d specROI.h = %d Halo %6.3f\n",specROI.width,specROI.height,result.z);
	}
	return result;
}

CvPoint3D32f NanoFocusSpecularityBasedSpoofDetector::ComputeTopPointsBasedHaloScore(IplImage *img1, unsigned char specValue, int pixelsToConsider,
		float TopPixelsPercentage, int BottomPixelsIntensityThresh, float HaloThreshold,
		bool EnableHaloThresh, int MHaloNegationThresh)
{
	CvPoint3D32f result = cvPoint3D32f(-1,-1,-1.0); //result.x = sum, result.y = count, result.z = haloScore
	CvPoint2D32f specCentroid = GetSpecularityCentroid();
	CvRect specROI = GetSpecularityROI();
	if (specROI.width<0 || specROI.width>640 || specROI.height<0 || specROI.height>480){
		printf("<<<Invalid values of specROI>>>");
	}
	else
	{
		int sum=0;
		int count = 0;
		float sumOfDiffs = 0;
		float diff = 0;
		float modifiedHalo;
		//HaloPoints.clear();
		std::vector<unsigned char> HaloPoints;
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
				//printf("\nMax=%d",max);
				if (max > *iptr && max == specValue) {
					sum += *iptr;
					count += 1;
					HaloPoints.push_back(*iptr);
				}
			}
		}
		//printf("\nSum=%d Count=%d HaloScore=%f",sum,count,sum*1.0/count);
		float val = -1.0;
		pixelsToConsider = count/pixelsToConsider;
		if(count>0){
			val = sum*1.0/count;
			std::sort(HaloPoints.begin(), HaloPoints.end());
			for(int i=0; i<pixelsToConsider; i++)
			{
				diff = (specValue - HaloPoints[i]);
				sumOfDiffs += diff;
			}
			SetBottomPointsDiff(sumOfDiffs*1.0f/pixelsToConsider);

			sumOfDiffs = 0;
			diff = 0;
			int noOfBottomPoints = 0;
			int noOfTopPoints = 0;
			float intensitySumTopPoints = 0;
			int topthreshold = (((100-TopPixelsPercentage)/100.0)*specValue);
			
			for(int i=0; i<count; i++)
			{
				if(HaloPoints[i]<BottomPixelsIntensityThresh)
					++noOfBottomPoints;
				if(HaloPoints[i] > topthreshold)
				{
					intensitySumTopPoints += HaloPoints[i];
					++noOfTopPoints;
				}
			}
			SetNoOfBottomPoints(noOfBottomPoints);
			SetNoOfTopPoints(noOfTopPoints);
			SetAvgIntensityTP(intensitySumTopPoints/count);
			modifiedHalo = (MHaloNegationThresh - (m_BottomPointsDiff-m_avgIntensityTP)); // Making it -ve going
			modifiedHalo = std::max(int (modifiedHalo),1);
		}
		if ((EnableHaloThresh==true) && (val > HaloThreshold) || (count==0))// check for m_enableHaloThreshold
			result = cvPoint3D32f(sum,count,-1.0);
		else
			result = cvPoint3D32f(sum,count,modifiedHalo);
		//printf("specROI.w = %d specROI.h = %d Halo %6.3f\n",specROI.width,specROI.height,result.z);
		//HaloPoints.clear();
	}
	return result;
}


