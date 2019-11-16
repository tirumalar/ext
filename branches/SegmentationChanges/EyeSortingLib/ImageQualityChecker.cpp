#include "ImageQualityChecker.h"
#include <opencv/cxcore.h>
#pragma managed(push, off)
#include <opencv/cv.h>
#pragma managed(pop)
//#include "ippi.h"
//#include "ippsr.h"
//#include "ipps.h"
//#include "ippcv.h"
#include <stdio.h>
//#include <direct.h>
#include "opencv/highgui.h"


using namespace std;
#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

//#define _DEBUG
//#define _SAVELOG
#define _ENABLEVERTICALSTRIP false



ImageQualityChecker::ImageQualityChecker(int width, int height, int maxGrayScaleSpec,float threshScore,float threshRatio): m_width(width), m_height(height),m_MaxGrayScaleSpec(maxGrayScaleSpec),
m_threshScore(threshScore),m_threshRatio(threshRatio),
m_expectedMaxValue(maxGrayScaleSpec), m_scratchBuffer(0), m_smallImage128x128(0), m_smallImage256x256(0), m_direction(VERTICAL)
{
	//	m_scratchBuffer = (unsigned char *) malloc(bufSize);
	m_histSizeBytes = 1000*sizeof(float);
	m_hist = (float *) malloc(m_histSizeBytes);

	low_threshold = std::make_pair(0.75f, 0.85f);
	high_threshold = std::make_pair(1.25f, 1.15f);

	m_smallImage128x128 = cvCreateImage(cvSize(128, 128), IPL_DEPTH_8U, 1);
	m_smallImageExpand128x128 = cvCreateImage(cvSize(128, 128), IPL_DEPTH_8U, 1);
	m_smallImage256x256 = cvCreateImage(cvSize(256, 256), IPL_DEPTH_16U, 1);
	cvZero(m_smallImage128x128);
	cvZero(m_smallImageExpand128x128);
}


ImageQualityChecker::~ImageQualityChecker(void)
{
	if(m_hist)			free(m_hist);	m_hist = 0;
	if(m_smallImage128x128)	cvReleaseImage(&m_smallImage128x128);	m_smallImage128x128 = 0;
	if(m_smallImageExpand128x128)	cvReleaseImage(&m_smallImageExpand128x128);	m_smallImageExpand128x128 = 0;
	if(m_smallImage256x256)	cvReleaseImage(&m_smallImage256x256);	m_smallImage256x256 = 0;
//	if(m_scratchBuffer)	free(m_scratchBuffer);	m_scratchBuffer = 0;
}

 

int calculateRadius(int length){ 
	return (0.87*length)/6; 
}

bool isIrisIntersectingRegionBoundary(IrisParameters irisParameters, int startPixel, int endPixel) {
	if( (irisParameters.x - irisParameters.r) > startPixel  && (irisParameters.x + irisParameters.r) < endPixel )
		return false; // Means no Intersection.
	else 
		return true; // Means Intersection Detected.
}

void logging(int flagPassOverall, IplImage *imgOrig, float bestValueMax, float ratioMax, IrisParameters irisParameters, int bestIndex[], int bestIndexMax){

	static int discardImageCount = 0;
	static int goodImageCount = 0;
	char buffer[100];

#ifdef _SAVELOG
	FILE *fDump = fopen("D:\\ImageQualityChecker\\Output\\Dump.txt","a");
#endif
		
	switch(flagPassOverall)
	{
		case 1: 
		case 3:
				++goodImageCount;
#ifdef _DEBUG
				printf("\nGoodImage_%u.pgm  width = %d  height = %d",(goodImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-ALLOWED-NO-VERTICAL-ARTIFACT>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\GoodImage\\GoodImage_%u.pgm",(goodImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nGoodImage_%u.pgm  width = %d  height = %d",(goodImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-ALLOWED-NO-VERTICAL-ARTIFACT>>\n",bestValueMax, ratioMax);
				}
#endif
				break;

		case 2:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-VERTICALSTRIPSPLICE>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_VerticalStripSplice\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-VERTICALSTRIP>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

		case 4:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-INTERSECTION>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_IrisIntersection\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-INTERSECTION>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

		case 5:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-X>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_IrisX\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-X>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	
		
		case 6:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-RADIUS>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_IrisRad\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-RADIUS>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

		case 7:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-Y>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_IrisY\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-IRIS-Y>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

		case 8:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-PROXIMITY-THRESH>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_Proximity\\Discarded_%u.pgm",(discardImageCount));
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-PROXIMITY-THRESH>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

		case 9:
				++discardImageCount;
#ifdef _DEBUG
				printf("\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
				printf("\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-INSUFFICIENT-EFFECTIVE-REGION>>\n",bestValueMax, ratioMax);
#endif
#ifdef _SAVELOG
				sprintf(buffer,"D:\\ImageQualityChecker\\Output\\Discarded_Insufficient_Region\\Discarded_%u.pgm",(discardImageCount));	
				if(!cvSaveImage(buffer,imgOrig)) printf("\n Unable to save Image");
				else {
					fprintf(fDump,"\nDiscarded_%u.pgm  width = %d  height = %d",(discardImageCount), imgOrig->width, imgOrig->height);
					fprintf(fDump,"\nbestValueMax = %lf  ratioMax = %lf <<IMAGE-BLOCKED-INSUFFICIENT-EFFECTIVE-REGION>>\n",bestValueMax, ratioMax);
				}
#endif
				break;	

	} // end of switch
		
#ifdef _DEBUG
	printf("I.x = %d I.y = %d I.r = %d",irisParameters.x ,irisParameters.y, irisParameters.r);
	//printf("\nbestIndexMax = %d  Index1 = %d Index2 = %d Index3 = %d",bestIndexMax,bestIndex[0],bestIndex[1],bestIndex[2]);
#endif

#ifdef _SAVELOG
				fprintf(fDump,"I.x = %d I.y = %d I.r = %d",irisParameters.x ,irisParameters.y,irisParameters.r);
				//fprintf(fDump,"\nbestIndexMax = %d  Index1 = %d Index2 = %d Index3 = %d",bestIndexMax,bestIndex[0],bestIndex[1],bestIndex[2]);
				fclose(fDump);
#endif
		
}

int ImageQualityChecker::ComputeSpecularityMetrics(IplImage* frame, CvPoint2D32f &center, CvPoint2D32f &var, int radius)
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
////
// implementation assumes m = 1;
ImageQualityChecker::MapType ImageQualityChecker::peak_detector(float *hist, float *temp, int len, int m, int l)
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

float ImageQualityChecker::strip_focus3(IplImage *img, CvPoint pt, CvPoint2D32f specvar, CvSize sz, float *hist)
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

/*
float ImageQualityChecker::strip_focus2(IplImage *img, CvPoint pt, CvPoint2D32f specvar, CvSize sz, float *hist)
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

	Ipp8u *lpeaks = (Ipp8u *) calloc(259, sizeof(Ipp8u));
	Ipp8u *peaks = lpeaks;

	ippsFindPeaks_32f8u(hist, peaks, 259, 1, 1);

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

	free(lpeaks);

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
*/
float ImageQualityChecker::strip_focus(IplImage *img, CvPoint pt, CvSize sz, float *hist)
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

	for(int i=MAX(0, minloc-3);i<MIN(minloc+3, sz.width-1);i++)
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

std::pair<float, float> ImageQualityChecker::check_image(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
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

	if(count == 0 || MAX(var.x, var.y)/MIN(var.x, var.y) > 3.0f)
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

		sz = cvSize(10, cvRound(MIN(MAX(60.0f, len), 120.0f)));
	}

	measure.first = (float) count;
	
	// A  measure.second = strip_focus2(&input, cvPoint(cvRound(pt.x), cvRound(pt.y)), var, sz, m_hist);

	return measure;
}

std::pair<float, float> ImageQualityChecker::check_image2(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
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

	if(count == 0 ||MAX(var.x, var.y)/MIN(var.x, var.y) > 3.0f)
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

		sz = cvSize(10, cvRound(MIN(MAX(60.0f, len), 120.0f)));
	}

	measure.first = (float) count;
	
	measure.second = strip_focus3(&input, cvPoint(cvRound(pt.x), cvRound(pt.y)), var, sz, m_hist);

	return measure;
}

int ImageQualityChecker::check(char *img, int w, int h, int widthStep, CvPoint3D32f ip, CvPoint3D32f pp, float fmeas)
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

void ImageQualityChecker::reset()
{
	score_list.clear();
}

int ImageQualityChecker::CheckVerticalBlurring(IplImage* frame, CvPoint center, int radius, CvPoint3D64f *of, unsigned char specVal)
{
	int nx = center.x, ny = center.y;

	double hmean =0, vmean = 0, count = 0;
	int bufferSizeBytes = 4*radius*sizeof(float);

	if( m_histSizeBytes < bufferSizeBytes )
	{
		//printf("Error: m_hist size %d is less than bufferSizeBytes %d\n", m_histSizeBytes, bufferSizeBytes);
		free(m_hist);
		m_hist = (float*)malloc(bufferSizeBytes);
		m_histSizeBytes = bufferSizeBytes ;
	}

	memset(m_hist, 0, bufferSizeBytes);

	float *hval = m_hist;
	float *hcount = hval + 2*radius;

	for(int i = ny-2*radius; i < ny+2*radius; i++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep + nx - radius;
		unsigned char *_iptr = iptr - frame->widthStep;

		for(int j = 0; j < 2*radius; j++, iptr++, _iptr++)
		{
			if(iptr[0] != specVal || iptr[-1] != specVal || _iptr[0] != specVal) 
			{
				int h = abs((int) iptr[0] - (int) iptr[-1]); 
				int v = abs((int) *_iptr - (int) *iptr);

				hmean += h;
				vmean += v;
				count++;

				hval[j] += h;
				hcount[j]++;
			}
			//else
				//printf("h");
		}
	}

	if(count > 0)
	{
		hmean /= count;
		vmean /= count;
	}

	float bestVal = 0, bestConf = 0, bestMinVal = 1000, bestMinConf = 0, bestValIndex=0;

	for(int j = 0; j < 2*radius; j++)
	{
		if(hcount[j] > 0)
			hval[j] /= hcount[j];

		if(hval[j] > bestVal && hcount[j] > radius)
		{
			bestVal = hval[j];
			bestValIndex = j;
			bestConf = hcount[j]/(4*radius);
		}

		if(hval[j] < bestMinVal && hcount[j] > radius)
		{
			bestMinVal = hval[j];
			bestMinConf = hcount[j]/(4*radius);
		}
	}


#if 0

	FILE *fp = fopen("histogram.txt", "wt");
	for(int i=0;i<m_smallImage256x256->height;i++)
	{
		unsigned short *lhist = (unsigned short *) (m_smallImage256x256->imageData + i*m_smallImage256x256->widthStep);

		for(int j=0;j<m_smallImage256x256->width;j++)
			fprintf(fp, "%d ", *lhist++);

		fprintf(fp, "\n");
	}

	fclose(fp);
#endif

	*of++ = cvPoint3D64f(bestVal, 0, bestConf);
	*of++ = cvPoint3D64f(bestMinVal, 0, bestMinConf);
	*of++ = cvPoint3D64f(hmean, vmean, count/(8*radius*radius));

	//return 3;
	return bestValIndex;
} // end of CheckVerticalBlurring


int ImageQualityChecker::CheckHorizontalBlurring(IplImage* frame, CvPoint center, int radius, CvPoint3D64f *of, unsigned char specVal)
{
	//return 3;
	int nx = center.x, ny = center.y;

	double hmean =0, vmean = 0, count = 0;
	int bufferSizeBytes = 4*radius*sizeof(float);

	if( m_histSizeBytes < bufferSizeBytes )
	{
		//printf("Error: m_hist size %d is less than bufferSizeBytes %d\n", m_histSizeBytes, bufferSizeBytes);
		free(m_hist);
		m_hist = (float*)malloc(bufferSizeBytes);
		m_histSizeBytes = bufferSizeBytes ;
	}

	memset(m_hist, 0, bufferSizeBytes);

	float *hval = m_hist;
	float *hcount = hval + 2*radius;

	int k = 0;
	for(int i = ny-radius; i < ny+radius; i++, k++)
	{
		unsigned char *iptr = (unsigned char *) frame->imageData + i*frame->widthStep + nx - 2*radius;
		unsigned char *_iptr = iptr - frame->widthStep;

		for(int j = 0; j < 4*radius; j++, iptr++, _iptr++)
		{
			if(iptr[0] != specVal || iptr[-1] != specVal || _iptr[0] != specVal) 
			{
				int h = abs((int) iptr[0] - (int) iptr[-1]); 
				int v = abs((int) *_iptr - (int) *iptr);

				hmean += h;
				vmean += v;
				count++;

				hval[k] += v;
				hcount[k]++;
			}
			//else
				//printf("h");
		}
	}

	if(count > 0)
	{
		hmean /= count;
		vmean /= count;
	}

	float bestVal = 0, bestConf = 0, bestMinVal = 1000, bestMinConf = 0;

	for(int j = 0; j < 2*radius; j++)
	{
		if(hcount[j] > 0)
			hval[j] /= hcount[j];

		if(hval[j] > bestVal && hcount[j] > radius)
		{
			bestVal = hval[j];
			bestConf = hcount[j]/(4*radius);
		}

		if(hval[j] < bestMinVal && hcount[j] > radius)
		{
			bestMinVal = hval[j];
			bestMinConf = hcount[j]/(4*radius);
		}
	}


#if 0

	FILE *fp = fopen("histogram.txt", "wt");
	for(int i=0;i<m_smallImage256x256->height;i++)
	{
		unsigned short *lhist = (unsigned short *) (m_smallImage256x256->imageData + i*m_smallImage256x256->widthStep);

		for(int j=0;j<m_smallImage256x256->width;j++)
			fprintf(fp, "%d ", *lhist++);

		fprintf(fp, "\n");
	}

	fclose(fp);
#endif

	*of++ = cvPoint3D64f(bestVal, 0, bestConf);
	*of++ = cvPoint3D64f(bestMinVal, 0, bestMinConf);
	*of++ = cvPoint3D64f(vmean, hmean, count/(8*radius*radius));
	
	//frame = NULL;
	return 3;
} // end of CheckHorizontalBlurring



int ImageQualityChecker::ConsolidateResultsVerticalBlurring(IplImage* imgOrig)
{
	/*************************************************************************************
	* Objective: Quality Evaluation of the 5MP landscape image.
	* Input:  5MP Landscape Image.
	* Output: Pass/Fail status. Note - Here Pass means Image should be discarded.
	* Thresholds: THRESH_BEST_SCORE  and  THRESH_RATIO are required.
	**************************************************************************************/
		double ratio1 = 0.0, ratio2 = 0.0, ratio3 = 0.0, ratioMax=0.0;
		double bestValueMax = 0.0, best1 = 0.0, best2 = 0.0, best3 = 0.0;
		int radius = 300, centerX = 968, centerY = 1296;
		int flagPassOverall = 0;
		CvPoint3D64f ret[3];
		
		//cvSetImageROI(imgOrig, cvRect(centerX-radius-2, centerY-2*radius-2, centerX+radius+2, centerY+2*radius+2));
		//cvSetImageROI(imgOrig, cvRect(68-2, centerY-2*radius-2, 1868+2, centerY+2*radius+2)); // For Portrait Image, no need of Transpose
		cvSetImageROI(imgOrig, cvRect(centerY-2*radius-2, 68-2, centerY+2*radius+2, 1868+2)); // For Landscape Image
		IplImage *imgRoi1 = cvCreateImage(cvGetSize(imgOrig), imgOrig->depth, imgOrig->nChannels);
		IplImage *imgRoiTranspose = cvCreateImage(cvSize(imgRoi1->height,imgRoi1->width), imgOrig->depth, imgOrig->nChannels);
		cvCopy(imgOrig, imgRoi1, NULL); /* copy subimage */
		cvResetImageROI(imgOrig);
		cvTranspose(imgRoi1,imgRoiTranspose);
				
		CheckVerticalBlurring(imgRoiTranspose, cvPoint(centerX-2*radius, centerY), radius, ret);
		ratio1 = (ret[2].x / ret[2].y);
		best1 = ret[0].x;
		//printf("(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) (%lf)\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z, ratio1);
		//fprintf(fDump,"OUTPUT1;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z, ratio1);
				
		CheckVerticalBlurring(imgRoiTranspose, cvPoint(centerX, centerY), radius, ret);
		ratio2 = (ret[2].x / ret[2].y);
		best2 = ret[0].x;
		//printf("(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) (%lf)\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z), ratio2;
		//fprintf(fDump,"OUTPUT2;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z, ratio2);

		CheckVerticalBlurring(imgRoiTranspose, cvPoint(centerX+2*radius, centerY), radius, ret);
		ratio3 = (ret[2].x / ret[2].y);
		best3 = ret[0].x;
		//printf("(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) (%lf)\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z, ratio3);
		//fprintf(fDump,"OUTPUT3;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;\n", ret[0].x, ret[0].y, ret[0].z, ret[1].x, ret[1].y, ret[1].z, ret[2].x, ret[2].y, ret[2].z, ratio3);

		ratioMax = MAX(MAX(ratio1,ratio2),ratio3);
		bestValueMax = MAX(MAX(best1,best2),best3);
		if(bestValueMax>=m_threshScore || ratioMax>=m_threshRatio) flagPassOverall = 1;
		else flagPassOverall = 0;
		
		cvReleaseImage(&imgRoi1);
		cvReleaseImage(&imgRoiTranspose);
		
return flagPassOverall;
} //end of function

#define cvSetImageData cvSetData

int ImageQualityChecker::checkQualityNano2(unsigned char *imgBuff,int width,int height,int widthStep,IrisParameters irisParams)
{
	IplImage img;
	cvInitImageHeader(&img,cvSize(width,height),IPL_DEPTH_8U,1);
	cvSetImageData(&img,imgBuff,widthStep);
	return ConsolidateResultsVerticalBlurringGeneralized2(&img,(StripDirection) 0,60,irisParams);
}



int ImageQualityChecker::ConsolidateResultsVerticalBlurringGeneralized2(IplImage* imgOrig,StripDirection direction,int radius,IrisParameters irisParameters)
{
	/*************************************************************************************
	* Objective: Quality Evaluation of the 640*480p images.
	* Input:  640*480p image.
	* Output: Pass/Fail status. Note - Here Pass means Image should be discarded.
	* Note:   Return Value 1 means Image should be discarded.
	*		  Return Value 0 means Image should be allowed for Enrollment etc.	
			  Return Value -1 means Invalid Image Size (error).		
	* Thresholds: THRESH_BEST_SCORE is required.
	**************************************************************************************/
	
	int flagPassOverall = -999; // Denotes uninitialized case.
	int result;
	int bestIndex[3]={-1,-1,-1}; 
	int bestIndexMax=-1;
    if(!imgOrig || imgOrig->width==0 || imgOrig->height == 0 ){
		printf("\nError:Image not found or Invalid Image Size");
		flagPassOverall = -1;
		return flagPassOverall;
	}

	double ratio1 = 0.0, ratio2 = 0.0, ratio3 = 0.0, ratioMax=0.0;
	double bestValueMax = 0.0, best1 = 0.0, best2 = 0.0, best3 = 0.0;
	int centerX = imgOrig->width/2, centerY = imgOrig->height/2;
	int specVal = m_MaxGrayScaleSpec;
	CvPoint3D64f ret[3];

	int flagX, flagY;
	int offset;

	setDirection(direction);
	flagPassOverall = findEffectiveImageParameters2(imgOrig, direction, irisParameters);
	if(_ENABLEVERTICALSTRIP==false && flagPassOverall==10) flagPassOverall = 1; //1 means image should be allowed. 

	if( flagPassOverall == 10 && _ENABLEVERTICALSTRIP==true)
	{
		if (direction==VERTICAL){
			centerX = m_effectiveRegionCenter.x;
			centerY = m_effectiveRegionCenter.y;
			radius = calculateRadius(m_effectiveRegionLength);
			
			flagX = 1;
			flagY = 0;
		}
		else{
			flagX = 0;
			flagY = 1;			
		}

		offset = 2*radius;
		if(getDirection()==VERTICAL) ptrblurringFunc = &ImageQualityChecker::CheckVerticalBlurring;
		else if(getDirection()==HORIZONTAL) ptrblurringFunc = &ImageQualityChecker::CheckHorizontalBlurring;
		else return -1;
						
		bestIndex[0]=(this->*ptrblurringFunc)(imgOrig, cvPoint(centerX-(flagX*offset), centerY-(flagY*offset)), radius, ret, specVal);
		bestIndex[0] = (centerX-(flagX*offset) - radius) + bestIndex[0];
		ratio1 = (ret[2].x / ret[2].y);
		best1 = ret[0].x;
				
		bestIndex[1]=(this->*ptrblurringFunc)(imgOrig, cvPoint(centerX, centerY), radius, ret, specVal);
		bestIndex[1] = (centerX - radius) + bestIndex[1];
		ratio2 = (ret[2].x / ret[2].y);
		best2 = ret[0].x;

		bestIndex[2]=(this->*ptrblurringFunc)(imgOrig, cvPoint(centerX+(flagX*offset), centerY+(flagY*offset)), radius, ret, specVal);
		bestIndex[2] = (centerX+(flagX*offset) - radius) + bestIndex[2];
		ratio3 = (ret[2].x / ret[2].y);
		best3 = ret[0].x;

		ratioMax = MAX(MAX(ratio1,ratio2),ratio3);
		bestValueMax = MAX(MAX(best1,best2),best3);
		//bestIndexMax = MAX(MAX(bestIndex[0],bestIndex[1]),bestIndex[2]);
		if(bestValueMax>=m_threshScore) flagPassOverall = 2;
		else flagPassOverall = 1;
	}
						
#if defined(_DEBUG) || defined(_SAVELOG) 
	logging(flagPassOverall,imgOrig,bestValueMax,ratioMax,irisParameters,bestIndex,bestIndexMax);
#endif

	if (flagPassOverall==1 || flagPassOverall==3) result = 0;
	else result = 1;

return result;
} //end of function


int ImageQualityChecker::findEffectiveImageParameters(IplImage* frame, StripDirection direction)
{
	/***************************************************************************************
	* Objective: To find Effective Region parameters (Image - black border area)
	* Input:     Image and StripDirection
	* Output:    EffectiveRegionLength(m_effectiveRegionLength), EffectiveRegionCenter(m_effectiveRegionCenter)
	* Note:      Return value is 0 for following failure cases:
	*			 1.) Complete Black Image
	*			 2.) Effective Region is less than Iris Size(200 here).
	****************************************************************************************/

	int startPixel, endPixel;	

  // Traversing Middle Row of Image Horizontally for finding EffectiveRegion startPixel
	unsigned char *iptr = (unsigned char *) frame->imageData + (frame->height/2)*frame->widthStep;
	for(int j = 0; j < frame->width; j++)
	{
			if(iptr[j] != 0) 
			{
				startPixel = j;
				break;
			}
	}

  // Traversing Middle Row of Image Horizontally for finding EffectiveRegion endPixel
	iptr = (unsigned char *) frame->imageData + (frame->height/2)*frame->widthStep + frame->width;
	for(int j = frame->width-1; j >= 0; j--)
	{
			if(iptr[j] != 0) 
			{
				endPixel = j;
				break;
			}
	}

	m_effectiveRegionLength = endPixel - startPixel;
	m_effectiveRegionCenter.x = (startPixel + endPixel)/2;
	m_effectiveRegionCenter.y = frame->height/2;

#ifdef _DEBUG
	printf("\n startPixel = %d  endPixel = %d  CenterX = %d", startPixel, endPixel, m_effectiveRegionCenter.x );
#endif
	return (m_effectiveRegionLength>200)?1:0;

} // end of function


int ImageQualityChecker::findEffectiveImageParameters2(IplImage* frame, StripDirection direction, IrisParameters irisParameters)
{
	/***************************************************************************************
	* Objective: To find Effective Region parameters (Image - black border area)
	* Input:     Image and StripDirection
	* Output:    Set EffectiveRegionLength(m_effectiveRegionLength), EffectiveRegionCenter(m_effectiveRegionCenter)
	* Note:      Return Value 4 to 9 denotes image should be discarded.
	*			 Return Value of 10 denotes image is candidate for vertical artifacts evaluation.
	****************************************************************************************/

	int startPixel, endPixel;	
	
 
  // Traversing Middle Row of Image Horizontally for finding EffectiveRegion startPixel
	unsigned char *iptr = (unsigned char *) frame->imageData + (frame->height/2)*frame->widthStep;
	for(int j = 0; j < frame->width; j++)
	{
			if(iptr[j] != 0) 
			{
				startPixel = j;
				break;
			}
	}

  // Traversing Middle Row of Image Horizontally for finding EffectiveRegion endPixel
	iptr = (unsigned char *) frame->imageData + (frame->height/2)*frame->widthStep + frame->width;
	for(int j = frame->width-1; j >= 0; j--)
	{
			if(iptr[j] != 0) 
			{
				endPixel = j;
				break;
			}
	}

	m_effectiveRegionLength = endPixel - startPixel;
	m_effectiveRegionCenter.x = (startPixel + endPixel)/2;
	m_effectiveRegionCenter.y = frame->height/2;

#ifdef _DEBUG
	printf("\n\n startPixel = %d  endPixel = %d  CenterX = %d", startPixel, endPixel, m_effectiveRegionCenter.x );
#endif

#ifdef _SAVELOG
	mkdir("d:/ImageQualityChecker");
	mkdir("d:/ImageQualityChecker/Output");
	mkdir("d:/ImageQualityChecker/Output/GoodImage");
	mkdir("d:/ImageQualityChecker/Output/Discarded_VerticalStripSplice");
	mkdir("d:/ImageQualityChecker/Output/Discarded_Proximity");
	mkdir("d:/ImageQualityChecker/Output/Discarded_IrisY");
	mkdir("d:/ImageQualityChecker/Output/Discarded_IrisX");
	mkdir("d:/ImageQualityChecker/Output/Discarded_IrisRad");
	mkdir("d:/ImageQualityChecker/Output/Discarded_IrisIntersection");
	mkdir("d:/ImageQualityChecker/Output/Discarded_Insufficient_Region");

	FILE *fDump = fopen("D:\\ImageQualityChecker\\Output\\Dump.txt","a");
	fprintf(fDump,"\n\n startPixel = %d  endPixel = %d  CenterX = %d", startPixel, endPixel, m_effectiveRegionCenter.x );
	fclose(fDump);
#endif


// Discarded due to Insufficient Region Boundary.
	if(m_effectiveRegionLength<180) return 9;	

// Discarded due to Iris Intersecting Region Boundary.
	if( isIrisIntersectingRegionBoundary(irisParameters,startPixel,endPixel) ) return 4;

// Discarded due to proximity threshold.
	if( startPixel >=(frame->width/2 - 110) || 
		endPixel <=(frame->width/2 + 110) ) return 8;

// Discarded due to Iris Radius.
	if( irisParameters.r < 82 
		|| irisParameters.r > 145 ) return 6;

// Discarded due to Iris X.
	if( irisParameters.x < 280 || 
		irisParameters.x > 360) return 5;

// Discarded due to Iris Y.
	if( irisParameters.y < 220 || 
		irisParameters.y > 260 ) return 7;

	return 10;  //10 Denotes candidate for vertical artifacts evaluation.
} // end of function
