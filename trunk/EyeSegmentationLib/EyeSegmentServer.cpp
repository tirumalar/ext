#include <stdio.h>


#include "EyeSegmentServer.h"
#include "DistanceMetrics.h"
#include <opencv/cv.h>
#include <stdlib.h>
#include <opencv/cxcore.h>
#include "EyeFeatureServer.h"
#include "EyeMatchServer.h"
#include <time.h>
#include <map>
#include "useful.h"
#include <time.h>
#include "area_concom.h"
#include <algorithm>
#include <opencv/highgui.h>
#include "EdgeImage_private.h"
#include <opencv2/highgui/highgui.hpp>


using std::max;
using std::min;



// #define SEG



extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}

#if 1 //Anita for OPencv
#define cvGetHistValue_1D( hist, idx0 ) \
    ((float*)cvPtr1D( (hist)->bins, (idx0), 0))
#endif

void draw( IplImage* img1, CvPoint3D32f pt, CvScalar color )
{
	CvPoint center = {(int)(pt.x), (int)(pt.y)};
	int radius = (int)(pt.z);
	cvCircle( img1, center, radius, color, 1, 8, 0 );
}

bool EyeSegmentServer::Getiseye()
{
	return m_iseye;
}

void EyeSegmentServer::FindBoundary(CvPoint3D32f Point, int level, bool pupil)
{
	float startRadius = 0;
	float endRadius = 0;

	Point.x /= (1 << (1+level));
	Point.y /= (1 << (1+level));
	Point.z /= (1 << (1+level));

	CvRect cloc;

	startRadius = (Point.z-1)*(1 << (1+level));
	endRadius = (Point.z+1)*(1 << (1+level));


	cloc.x = (int) floor((Point.x - 1)*(1 << (1+level)) + 0.5);
	cloc.y = (int) floor((Point.y - 1)*(1 << (1+level)) + 0.5);
	cloc.height = 2*(1 << (1+level));
	cloc.width = 2*(1 << (1+level));
/*
	cloc.x = cvRound((Point.x - 0.5)*(1 << (1+level)));
	cloc.y = cvRound((Point.y - 0.5)*(1 << (1+level)));
	cloc.height = 1*(1 << (1+level));
	cloc.width = 1*(1 << (1+level));
*/

	CvPoint3D32f bestCircle;
	CircleParameters bestparams;

	for(int i=level;i>0;i--)
	{
		CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};

		float scaling = 1.0f;
		float sigma = 0.5f;

		startRadius /= (1 << i);
		endRadius /= (1 << i);

		cloc.x = (cloc.x >> i);
		cloc.y = (cloc.y >> i);
		cloc.width = (cloc.width >> i);
		cloc.height = (cloc.height >> i);

		CvRect maskRect;

		maskRect.x = (m_irisRect.x >> i);
		maskRect.y = (m_irisRect.y >> i);
		maskRect.height = (m_irisRect.height >> i);
		maskRect.width = (m_irisRect.width >> i);

//		if(!pupil)
		
		MaskSpecularities(m_imgPyr[i],maskRect,m_maskPyr[i],i); //turn on for non CASIA ver 1
	

		std::list<CircleParameters> circList;
		if(!pupil)
		{
			
			circList = findcircle(m_imgPyr[i], m_maskPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 0.0f, 3.0f, m_minIrisAngle, m_maxIrisAngle, pupil, i);
			
		}
		else
		{
			//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
			
			{
			cvDilate(m_maskPyr[i], m_smoothImage[i]);
			circList = findcircle(m_imgPyr[i], m_smoothImage[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 2.0f, m_minPupilAngle, m_maxPupilAngle, pupil, i);
			}
			
		}

		bestCircle = circList.begin()->cp;
		bestparams.ic = circList.begin()->ic;

		startRadius = (bestCircle.z - 1)*(1 << i);
		endRadius =   (bestCircle.z + 1)*(1 << i);

		cloc.x = (int) floor((bestCircle.x - 1)*(1 << i) + 0.5);
		cloc.y = (int) floor((bestCircle.y - 1)*(1 << i) + 0.5);
		cloc.height = 2*(1 << i);
		cloc.width = 2*(1 << i);

		bestCircle.z *= (1 << i);
		bestCircle.x *= (1 << i);
		bestCircle.y *= (1 << i);
	}

	if(!pupil)
	{
		/*FILE *fp = fopen("c:/Iris_Outputs/integral_cost.txt","a");
		fprintf(fp,"%d %f\n",1, (bestparams.ic) );
		fclose(fp);*/

		m_Iris.x = bestCircle.x;
		m_Iris.y = bestCircle.y;
		m_Iris.z = bestCircle.z;
	}
	else if(pupil)
	{
		/*FILE *fp = fopen("c:/Iris_Outputs/integral_cost.txt","a");
		fprintf(fp,"%d %f\n",0, (bestparams.ic) );
		fclose(fp);*/

		m_Pupil.x = bestCircle.x;
		m_Pupil.y = bestCircle.y;
		m_Pupil.z = bestCircle.z;
	}

}

void EyeSegmentServer::FindIrisAndPupil(std::list<CircleParameters> irisCircleList, std::list<CircleParameters> &pupilCircleList)
{
	CvPoint2D32f BestPtIris = {-1};
	CvPoint2D32f BestPtPupil = {-1};

	float maxCost = 0;
	float BestRadIris = 0;
	float BestRadPupil = 0;

	std::list<CircleParameters>::iterator icit = irisCircleList.begin();

	int IrisCnt = 0, PupilCnt = 0;
	int BestIrisCnt, BestPupilCnt;
	//float ihc, phc;
	for(;icit != irisCircleList.end(); icit++,IrisCnt++)
	{
		CircleParameters *icirc = &(*icit);

		float XcorI = icirc->cp.x;
		float YcorI = icirc->cp.y;
		float RadI = icirc->cp.z;
		float integralCostI = icirc->ic;
		float histCostI = icirc->hc;
		float simMeasureI = icirc->sm;
		float estimate_pupil_center_y = icirc->eph;


		XcorI *= (1 << m_levels);
		YcorI *= (1 << m_levels);
		RadI  *= (1 << m_levels);
		estimate_pupil_center_y *=  (1 << m_levels);

		std::list<CircleParameters>::iterator pcit =  pupilCircleList.begin();

		PupilCnt = 0;
		for(;pcit != pupilCircleList.end(); pcit++, PupilCnt++)
		{
			CircleParameters *pcirc = &(*pcit);

			float XcorP = pcirc->cp.x;
			float YcorP = pcirc->cp.y;
			float RadP = pcirc->cp.z;
			float integralCostP = pcirc->ic;
			float histCostP = pcirc->hc;
			float simMeasureP = pcirc->sm;

			XcorP *= (1 << (m_levels-1));
			YcorP *= (1 << (m_levels-1));
			RadP  *= (1 << (m_levels-1));

			float cdist = sqrt((XcorI - XcorP)*(XcorI - XcorP) + (YcorI - YcorP)*(YcorI - YcorP));

			// ASSUMPTION on the minimal iris area between pupil and sclera

//			if((RadP + xdist) > RadI - 20 || (RadP + ydist) > RadI - 10 ||  cdist > 30 || YcorP + RadP < estimate_pupil_center_y)
			if((RadP + cdist) > RadI - 10 || cdist > 30) // || YcorP + RadP < estimate_pupil_center_y)
				continue;

			float distCost = 3*histCostI + 2*histCostP;

			float costTotal = integralCostI + integralCostP + distCost -3*simMeasureI - 5*simMeasureP - cdist/4.0f;

			if(costTotal > maxCost)
			{
				BestPtIris = cvPoint2D32f(XcorI, YcorI);
				BestPtPupil = cvPoint2D32f(XcorP, YcorP);
				BestRadIris = RadI;
				BestRadPupil = RadP;
				maxCost = costTotal;
				BestIrisCnt = IrisCnt;
				BestPupilCnt = PupilCnt;
				/*ihc = histCostI;
				phc = histCostP;*/
			}
		}
	}

	/*FILE *fp = fopen("c:/hc.txt","a");
	fprintf(fp,"%f %f\n", ihc, phc );
	fclose(fp);*/

	/*FILE *fp = fopen("c:/IrisPupilMatch.txt","a");
	fprintf(fp,"%d %d\n",BestIrisCnt,BestPupilCnt);
	fclose(fp);*/

	/*FILE *fp1 = fopen( "c:/verify.txt", "w" );
	fprintf(fp1,"%f %f %f %f %f %f\n", BestPtIris.x/(1<<3), BestPtIris.y/(1<<3), BestRadIris/(1<<3),
		BestPtPupil.x/(1<<2), BestPtPupil.y/(1<<2), BestRadPupil/(1<<2) );
	fclose(fp1);*/

	m_Iris.x = BestPtIris.x;
	m_Iris.y = BestPtIris.y;
	m_Iris.z = BestRadIris;

	m_Pupil.x = BestPtPupil.x;
	m_Pupil.y = BestPtPupil.y;
	m_Pupil.z = BestRadPupil;

}
void EyeSegmentServer::ComputeAnnularSectionHistograms(IplImage *img, IplImage *mask, CvPoint3D32f iris, int level)
{
	float scale = 1.0f/(1 << level);
	iris.x *= scale;
	iris.y *= scale;
	iris.z *= scale;

	/*cvNamedWindow("test");
	cvShowImage("test",img);
	cvSaveImage("c:/test.bmp",img);
	cvNamedWindow("test1");
	cvShowImage("test1",mask);
	cvSaveImage("c:/test1.bmp",mask);*/
	int subsample = 3;
	float **o_hist = (float **) malloc((m_angleSamples+1) * sizeof(float **));

	float **hist = o_hist + 1;

	for(int i=0;i<m_angleSamples;i++)
	{
		cvSet(m_annularHist[i]->bins, cvRealScalar(0.01));
		hist[i] = cvGetHistValue_1D(m_annularHist[i], 0);
	}
	hist[-1] = hist[m_angleSamples-1];

	cvSet(m_irisHistogram->bins, cvRealScalar(0));

	float *full_hist = cvGetHistValue_1D(m_irisHistogram, 0);

	float rad = (iris.z > 12)?	iris.z - 12: 0;
	float inrad = rad*rad;
	float outrad = (iris.z - 1) * (iris.z - 1);
	//float outrad2 = (iris.z - 6) * (iris.z - 6);

	unsigned char *inptr = 0, *maskptr = 0;
	int inStep = 0, maskStep = 0;

	cvGetRawData(img, &inptr, &inStep);
	cvGetRawData(mask, &maskptr, &maskStep);

	CvRect roi = {max(cvFloor(iris.x - iris.z), 0), max(cvFloor(iris.y - iris.z), 0), cvRound(2*iris.z + 2), cvRound(2*iris.z + 2)};
	float angleScale = m_angleSamples/360.0f;


	for(int i=roi.y; i<roi.y+roi.height;i++)
	{
		unsigned char *iptr = inptr + i*inStep;
		unsigned char *mptr = maskptr + i*maskStep;

		float dy = i - iris.y;

//		if(dy*dy <= outrad)
		{
			for(int j=roi.x;j<roi.x+roi.width;j++)
			{
				float dx = j - iris.x;
				float dist = dy*dy + dx*dx;
				if(!mptr[j])
				{
					if(dist >= inrad && dist < outrad)
					{
						int index = cvRound(cvFastArctan(dx, dy) * angleScale) % m_angleSamples;
						hist[index][(iptr[j] >> 3)]++;
					}

					//// BUG???
				}

				if(dist < outrad)
				{
					//int index = (iptr[j] >> 2);
					//if( iptr[j-inStep] >> 2 == index && iptr[j+inStep] >> 2 == index && iptr[j+1] >> 2 == index && iptr[j-1] >> 2 == index )
						full_hist[(iptr[j] >> subsample)]++;
				}
					//full_hist[(iptr[j] >> 3)]++;
			}
		}
	}
	for(int i=0;i<m_angleSamples;i++)
		cvNormalizeHist(m_annularHist[i], 1.0f);

	free(o_hist);

}
void EyeSegmentServer::EstimatePupilCenter(IplImage *image, CvPoint3D32f iris, CvHistogram *histogram)
{
	float *full_hist = cvGetHistValue_1D(histogram, 0);
	std::map<int, float> peakMap;
	float scale = 1.0f/(1 << (m_levels-1));
	float min_pupil_area = (float) CV_PI * m_startPupilRadius *m_startPupilRadius*scale*scale;
	float total = 0;

	int bins = 31;
	int subsample = 3;
	for(int i=1;i<bins;i++)
	{
		total += full_hist[i];
		if(full_hist[i] > full_hist[i-1] && full_hist[i] > full_hist[i+1] && total > min_pupil_area)
		{
			peakMap[i] = full_hist[i];
			break;
		}
		/*if( total > min_pupil_area*2 )
		{
			peakMap[i] = full_hist[i];
			break;
		}*/
	}

	if(full_hist[0] > full_hist[1] && full_hist[0] > min_pupil_area)	peakMap[0] = full_hist[0];
	if(full_hist[bins] > full_hist[bins-1])	peakMap[bins] = full_hist[bins-1];

	//peakMap[2] = full_hist[1];
	if( !peakMap.size() )
	{
		/// Returning a Maximum value so that this particular pupil does is eliminated
		m_EstimatedPupil.first = (float) image->width;
		m_EstimatedPupil.second = (float) image->height;
		return;
	}
	int index = peakMap.begin()->first;

	CvRect roi = {max(cvFloor(iris.x - iris.z), 0), max(cvFloor(iris.y - iris.z), 0), cvRound(2*iris.z + 2), cvRound(2*iris.z + 2)};
	unsigned char *inptr = 0;
	int inStep = 0;
	float outrad = iris.z * iris.z;
	//float outrad2 = (iris.z-6) * (iris.z-6);

	cvGetRawData(image, &inptr, &inStep);

	float mx=0, my=0;
	int m=0;
	for(int i=roi.y; i<roi.y+roi.height;i++)
	{
		unsigned char *iptr = inptr + i*inStep;
		float dy = i - iris.y;

		if(dy*dy <= outrad)
		{
			for(int j=roi.x;j<roi.x+roi.width;j++)
			{
				float dx = j - iris.x;
				float dist = dy*dy + dx*dx;
				if(dist < outrad)
				{
					if(iptr[j]>>subsample <= index)
					{
						mx += j;
						my += i;
						m++;
					}
				}
			}
		}
	}

	m_EstimatedPupil.first =  mx/m;
	m_EstimatedPupil.second = my/m;

	return;

}

void EyeSegmentServer::compute_LUT_sqrtminmax(int gradientpupilmin,int gradientpupilmax,int gradientcirclemin,int gradientcirclemax){
	compute_LUT_sqrtminmax_C(SQRT_LUT_0_64,SQRT_LUT_0_256,gradientpupilmin,gradientpupilmax,gradientcirclemin,gradientcirclemax);
}

float EyeSegmentServer::ComputeLeftRightSimilarity(IplImage *img, IplImage *mask, CvPoint3D32f iris, int level, bool pupil)
{
	float **o_hist = (float **) malloc((m_angleSamples+1) * sizeof(float **));

	float **hist = o_hist + 1;

	for(int i=0;i<m_angleSamples;i++)
		hist[i] = cvGetHistValue_1D(m_annularHist[i], 0);

	hist[-1] = hist[m_angleSamples-1];

	float **rightDist = hist + m_angleSamples/4;
	float **leftDist = hist + 3*m_angleSamples/4;
	float **bottomDist = hist;

	int sz = 32;

	CvPoint off = {0,0};
	cvSetZero(m_emdFlow);

	float measure1 = findBestEMDDistance(leftDist, rightDist, sz, &off);
	float measure2 = findBestEMDDistance(leftDist, bottomDist, sz, &off);
	float measure3 = findBestEMDDistance(rightDist, bottomDist, sz, &off);

	free(o_hist);

	return min(min(measure1, measure2), measure3);

//	return (pupil)? min(min(measure1, measure2), measure3) : measure1;
}
int EyeSegmentServer::checkForObstruction(bool pupil, float maxCost, float maxHistCost)
{
	int theekHai = 0;

	float similar = 0;

	if(pupil)
	{
		similar = (m_eyelidSimilarityMeasure[1]>m_eyelidSimilarityMeasure[3])? m_eyelidSimilarityMeasure[1]: m_eyelidSimilarityMeasure[3];

		if(similar>25)
			theekHai = 1;
	}
	else if(!pupil)
	{
		similar = (m_eyelidSimilarityMeasure[2]>m_eyelidSimilarityMeasure[4])? m_eyelidSimilarityMeasure[2]: m_eyelidSimilarityMeasure[4];

		if(similar>25)
			theekHai = 1;
	}

	return(theekHai);
}
IplImage* EyeSegmentServer::GenerateMask()
{
	cvOr( m_upperEyelidMask, m_flatIrisMask, m_flatIrisMask);
	cvOr(m_flatIrisMask, m_lowerEyelidMask, m_flatIrisMask);

	return(m_flatIrisMask);
}
void EyeSegmentServer::generateSinCosTable(float angleSampleTable, float min_angleTemp,float max_angleTemp, float rad, float deltaRad, CirclePoints* sinCosTable)
{
	int ct = 0;

	for(float ang = min_angleTemp; ang <= max_angleTemp; ang += angleSampleTable)
	{
		float cs = cos(ang);
		float sn = sin(ang);

		sinCosTable[ct].on.x = rad*cs;
		sinCosTable[ct].on.y = -rad*cs;
		sinCosTable[ct].on.z = rad*sn;

		sinCosTable[ct].in.x = (rad - deltaRad)*cs;
		sinCosTable[ct].in.y = -(rad - deltaRad)*cs;
		sinCosTable[ct].in.z = (rad - deltaRad)*sn;

		sinCosTable[ct].out.x = (rad + deltaRad)*cs;
		sinCosTable[ct].out.y = -(rad + deltaRad)*cs;
		sinCosTable[ct].out.z = (rad + deltaRad)*sn;


		ct++;
	}
}

//#include <intrin.h>
//inline int FloatToInt_SSE(float x)	//this will do rounding
//{
//	return _mm_cvt_ss2si( _mm_load_ss(&x) );
//}


bool EyeSegmentServer::ComputeIntegralContribution(IplImage *gradImage, IplImage *img, IplImage *out, CvPoint2D32f ptf, CvPoint2D32f ptin, CvPoint2D32f ptout, CvSize searchArea)
{
	double dx = ptf.x - floor(ptf.x);
	double dy = ptf.y - floor(ptf.y);
	unsigned short C00 = min(65535, cvRound((1.0-dx) * (1.0-dy) * 65536));
	unsigned short C01 = min(65535, cvRound(dx * (1.0-dy) * 65536));
	unsigned short C10 = min(65535, cvRound((1.0-dx) * dy * 65536));
	unsigned short C11 = min(65535, cvRound(dx * dy * 65536));

	int sx = cvFloor(ptf.x);
	int sy = cvFloor(ptf.y);

	int on = sx +  (sy * img->widthStep);
	int ro = cvRound(ptout.x) + (cvRound(ptout.y) * img->widthStep) - on;
	int lo = cvRound(ptin.x) + (cvRound(ptin.y) * img->widthStep) - on;

	int ex = searchArea.width;
	int ey = sy + searchArea.height;

	for(int i=sy;i<ey;i++)
	{
		unsigned short *gptr = (unsigned short *) (gradImage->imageData + i*gradImage->widthStep) + sx;
		unsigned short *gwptr = (unsigned short *) (gradImage->imageData + (i+1)*gradImage->widthStep) + sx;
		unsigned char *iptr = (unsigned char *) (img->imageData + i*img->widthStep) + sx;
		unsigned int *optr = (unsigned int *) (out->imageData + (i-sy)*out->widthStep);

		for(int j=0;j<ex;j++, iptr++, gptr++, gwptr++, optr++)
		{
			if(iptr[lo] < iptr[ro])
				*optr += (C00 * gptr[0] + C01 * gptr[1] + C10 * gwptr[0] + C11 * gwptr[1] + 128)>>8;
		}
	}
	return true;
}


float EyeSegmentServer::GetIntegral(IplImage *costField, IplImage *img, CvPoint2D32f point, float rad, float min_angle, float max_angle, float max_grad, CirclePoints* sinCosTable, float angleSample, int numSamples, bool pupil)
{
	int count = 0;

	CvPoint3D32f *ptlist = (CvPoint3D32f *) malloc(numSamples*sizeof(CvPoint3D32f));
	CvPoint3D32f *ptlistin = (CvPoint3D32f *) malloc(numSamples*sizeof(CvPoint3D32f));
	CvPoint3D32f *ptlistout = (CvPoint3D32f *) malloc(numSamples*sizeof(CvPoint3D32f));

	for(float ang = min_angle; ang <= max_angle; ang += angleSample)
	{
		CvPoint3D32f pt;

		pt.x = point.x + sinCosTable[count].on.x;
		pt.z = point.x + sinCosTable[count].on.y;
		pt.y = point.y + sinCosTable[count].on.z;

		ptlist[count] = pt;

		ptlistin[count].x = point.x + sinCosTable[count].in.x;
		ptlistin[count].z = point.x + sinCosTable[count].in.y;
		ptlistin[count].y = point.y + sinCosTable[count].in.z;

		ptlistout[count].x = point.x + sinCosTable[count].out.x;
		ptlistout[count].z = point.x + sinCosTable[count].out.y;
		ptlistout[count].y = point.y + sinCosTable[count].out.z;

		count++;
	}

	unsigned char *iptr = 0;
	float *cptr = 0;
	int cstep, istep;

	cvGetRawData(img, (unsigned char **) &iptr, &istep);
	cvGetRawData(costField, (unsigned char **) &cptr, &cstep);
	cstep /= sizeof(float);


	float acc = 0;
	int items = 0;

	for(int i=0;i<count;i++)
	{
		CvPoint3D32f pt = ptlist[i];

		int y = cvFloor(pt.y);
		float dy = pt.y - y;

		int x = cvFloor(pt.x);
		float dx = pt.x - x;

		float *ptr = cptr + y*cstep + x;

		float cost = (1.0f-dx)*(ptr[0]*(1.0f-dy) + ptr[cstep]*dy) + dx*(ptr[1]*(1.0f-dy) + ptr[1+cstep]*dy);

		int yin = cvRound(ptlistin[i].y);
		int xin = cvRound(ptlistin[i].x);
		int yout = cvRound(ptlistout[i].y);
		int xout = cvRound(ptlistout[i].x);

		bool gradSign = iptr[yin*istep + xin] < iptr[yout*istep + xout];

		if(gradSign )//&& (!pupil || iptr[yin*istep + xin] < 75 )) // MAGIC NUMBER
		{
			acc += cost;
		}
		items++;

		//if(!pupil || iptr[yin*istep + xin] < 75)

		x = cvFloor(pt.z);
		dx = pt.z - x;
		ptr = cptr + y*cstep + x;
		cost = (1.0f-dx)*(ptr[0]*(1.0f-dy) + ptr[cstep]*dy) + dx*(ptr[1]*(1.0f-dy) + ptr[1+cstep]*dy);

		xin = cvRound(ptlistin[i].z);
		xout = cvRound(ptlistout[i].z);

		gradSign = iptr[yin*istep + xin] < iptr[yout*istep + xout];

		if(gradSign)
			acc += cost;

		items++;

	}
	free(ptlist);
	free(ptlistin);
	free(ptlistout);

	return acc/items;
}
int getNumSamples(float s, float e, float sp)
{
	int index = 0;

	for(float x = s; x < e; x += sp)
		index++;

	return index;
}

int getNumSamplesInclusive(float s, float e, float sp)
{
	int index = 0;

	for(float x = s; x <= e; x += sp)
		index++;

	return index;
}

float EyeSegmentServer::RegionHistogramDistance(IplImage *img, IplImage *specularityMask, CvPoint2D32f pt, float radIn, float rad, float radOut)
{
	//PROFILE_START(HISTCOST)
	int iradIn = cvRound(radIn);
	int iradOut = cvRound(radOut);
	int irad = cvRound(rad);

	int radin2 = iradIn*iradIn;
	int radOut2 = iradOut*iradOut;
	int radm2 = irad*irad;

	int ptx = cvRound(pt.x);
	int pty = cvRound(pt.y);

	int yo = cvRound(pty - radOut);
	int yb = cvRound(pty + radOut + 1);
	int xo = cvRound(ptx - radOut);
	int xb = cvRound(ptx + radOut + 1);

	cvClearHist(m_outerRingHist);
	cvClearHist(m_innerRingHist);

	float *ohist = cvGetHistValue_1D(m_outerRingHist, 0);
	float *ihist = cvGetHistValue_1D(m_innerRingHist, 0);

	unsigned char *pMaskPtr = 0, *pImagePtr = 0;
	int maskStep = 0, imageStep = 0;

	cvGetRawData(specularityMask, &pMaskPtr, &maskStep);
	cvGetRawData(img, &pImagePtr, &imageStep);

	for(int y=yo;y<yb;y++)
	{
		unsigned char *iptr = pImagePtr + y*imageStep;
		unsigned char *mptr = pMaskPtr + y*maskStep + xo;
		int dx = (xo-ptx);
		int dy = (y-pty);
		int y2 = dy*dy + dx*dx;

		for(int x=xo;x<xb;x++, y2 += 2*(dx++)+1, mptr++)
		{
//			float angle = cvFastArctan(dy, dx);

//			if(angle > 180)
//				printf("hello\n");
			if(y2 > radin2 && y2 <= radOut2 && !mptr[0]) // && angle < 180)
			{
				if(y2 <= radm2 )
					ihist[iptr[x] >> 3]++;
				else // if(y2 > radm2)
					ohist[iptr[x] >> 3]++;
			}
		}
	}

	cvNormalizeHist(m_outerRingHist, 1.0);
	cvNormalizeHist(m_innerRingHist, 1.0);

	//PROFILE_END(HISTCOST)
	return kullBackDistance(ihist, ohist, 32);
}
EyeSegmentServer::EyeSegmentServer(int w, int h):
m_w(w), m_h(h),
m_startIrisRadius(70), m_endIrisRadius(145), m_levels(3),f_levels(2), // m_levels is 3 radii are not divided by two
m_startPupilRadius(16), m_endPupilRadius(85),  //radii are not divided by two
m_minIrisAngle((float) (-30*CV_PI/180) ), m_maxIrisAngle((float) (60*CV_PI/180) ), // 0 to 60
//m_minIrisAngle((float) (-90*CV_PI/180) ), m_maxIrisAngle((float) (90*CV_PI/180) ),
m_minPupilAngle((float) (-90*CV_PI/180) ), m_maxPupilAngle( (float) (90*CV_PI/180) ),	/// MAGIC NUMBER, was -45
//  m_minPupilAngle((float) (-90*CV_PI/180) ), m_maxPupilAngle( (float) (45*CV_PI/180) ),	/// MAGIC NUMBER, was -45
m_tmpHist(0), m_innerRingHist(0), m_outerRingHist(0),
m_flatIris(0), m_annularHist(0), m_angleSamples(20),m_measureIris(0),m_measurePupil(0),m_nbhdCheckIris(true),m_nbhdCheckPupil(true),
m_emdFlow(0), m_transposedEMDFlow(0),
m_upperEyelidMask(0), m_lowerEyelidMask(0),
m_startEyelidRadiusRight(80), // radius of the circle
m_endEyelidRadiusRight(150), // changed to 180
m_startEyelidRadiusLeft(80),
m_endEyelidRadiusLeft(150),
m_minLidAngle( (float) (-90*CV_PI/180) ),
m_maxLidAngle( (float) (0*CV_PI/180) ),
m_enableEyelidSegmentation(true),
m_enableEyeQualityAssessment(false),
//m_eyelidSearchSampling(1.0),
m_enableHistogramCost(true),
m_defaultUpperEyelidAngle((float) (-45*CV_PI/180)),
m_defaultLowerEyelidAngle((float) (-60*CV_PI/180)),
m_eyelidDetected(true)
{

	m_centerptUpperEyelid.x = 360;
	m_centerptUpperEyelid.y = 190;
	m_radiusUpperEyelid = 140.0;

	m_centerptLowerEyelid.x = 130;
	m_centerptLowerEyelid.y = 190;
	m_radiusLowerEyelid = 135.0;

	mask_cnt = 0;
	m_centerRect = cvRect(w/2 - 60, h/2 - 40, 120, 80);
//	m_centerRect = cvRect(w/2 - 40, h/2 - 20, 80, 60);
	m_irisRect = cvRect(cvRound(w/2 - m_endIrisRadius), cvRound(h/2 - m_endIrisRadius), cvRound(2*m_endIrisRadius), cvRound(2*m_endIrisRadius)); //radiis are not divided by two
	m_LidRightRect = cvRect(320, 90, 60, 140);
	m_LidLeftRect = cvRect(80, 80, 60, 120);

	m_flatIris = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);
	m_flatIrisMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);

	int wid = 480;
	int hig = 64;

	m_reusableImage = cvCreateImage(cvSize(w, h),  IPL_DEPTH_8U, 1);
#ifdef __EIGENREUSABLE__
	m_eigenvvReusableImage = cvCreateImage(cvSize(4*w, h),  IPL_DEPTH_32F, 1); // was 6
#endif
	for(int i=0;i<=m_levels;i++)
	{
		m_smoothImage[i] = cvCreateImageHeader(cvSize(w, h),  IPL_DEPTH_8U, 1);
		cvSetData(m_smoothImage[i],m_reusableImage->imageData,m_smoothImage[i]->widthStep);
		m_imgPyr[i] = cvCreateImage(cvSize(w, h),  IPL_DEPTH_8U, 1);
		m_gradPyr[i] = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 1);
		m_maskPyr[i] = cvCreateImage(cvSize(w, h),  IPL_DEPTH_8U, 1);
#ifdef __EIGENREUSABLE__
		m_eigenvv[i] = cvCreateImageHeader(cvSize(6*w, h),  IPL_DEPTH_32F, 1);
		cvSetData(m_eigenvv[i],m_eigenvvReusableImage->imageData,m_eigenvv[i]->widthStep);
#else
		m_eigenvv[i] = cvCreateImage(cvSize(6*w, h),  IPL_DEPTH_32F, 1);
#endif
//Madhav		m_eigenvf[i] = cvCreateImage(cvSize(6*wid, hig),  IPL_DEPTH_32F, 1);
		m_imgFlatPyr[i] = cvCreateImage(cvSize(wid, hig),  IPL_DEPTH_8U, 1);
		m_maskFlatPyr[i] = cvCreateImage(cvSize(wid, hig),  IPL_DEPTH_8U, 1);

		cvSetZero(m_maskPyr[i]);
		cvSetZero(m_imgFlatPyr[i]);
		cvSetZero(m_maskFlatPyr[i]);

		unsigned char *pMaskPtr = 0;
		int maskStep = 0;

		cvGetRawData(m_maskFlatPyr[i], &pMaskPtr, &maskStep);

		memset(pMaskPtr, 255, m_maskFlatPyr[i]->widthStep);
		memset(pMaskPtr + (m_maskFlatPyr[i]->widthStep)*(m_maskFlatPyr[i]->height-1), 255, m_maskFlatPyr[i]->widthStep);

		w = (w+1)/2;
		h = (h+1)/2;
		wid = wid/2;
		hig = hig/2;
	}

	int hist_size[] = {32}; // LBP_CHANGE: note for raw Image = 32
	int full_hist_size[] = {32};
	float g[] = {0, 255}; // LBP_CHANGE: note for raw Image = 255
	float* ranges[] = {g};

	m_outerRingHist = cvCreateHist( 1, hist_size, CV_HIST_ARRAY, ranges);
	m_innerRingHist = cvCreateHist( 1, hist_size, CV_HIST_ARRAY, ranges);
	m_tmpHist = cvCreateHist( 1, hist_size, CV_HIST_ARRAY, ranges);

	m_irisHistogram = cvCreateHist( 1, full_hist_size, CV_HIST_ARRAY, ranges);

	m_maskImage = cvCreateImage(cvSize(m_w, m_h),  IPL_DEPTH_8U, 1);

	m_annularHist = (CvHistogram **) malloc(m_angleSamples*sizeof(CvHistogram *));

	for(int i=0;i<m_angleSamples;i++)
		m_annularHist[i] = cvCreateHist(1, hist_size, CV_HIST_ARRAY, ranges);

	m_emdFlow = cvCreateMat(hist_size[0], hist_size[0], CV_32FC1);
	cvSetZero(m_emdFlow);

	m_transposedEMDFlow = cvCreateMat(hist_size[0], hist_size[0], CV_32FC1);
	cvSetZero(m_transposedEMDFlow);

	m_upperEyelidMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);
	m_lowerEyelidMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);



	m_EyeLidCosTable = new float[480];
	m_EyeLidSinTable = new float[480];

	float angle = 0;
	float increment = (float) ((2 * CV_PI)/ 480);
	for( int loopi = 0 ; loopi < 480 ; loopi++, angle+= increment )
	{
		m_EyeLidCosTable[loopi] = cos(angle);
		m_EyeLidSinTable[loopi] = sin(angle);
	}

	m_dxTable = cvCreateImage(cvSize(480, 64), IPL_DEPTH_32F, 1);
	m_dyTable = cvCreateImage(cvSize(480, 64), IPL_DEPTH_32F, 1);


	m_ccompServer = new CCOMP_STATE();
	ccomp_alloc(m_ccompServer, 480, 64, 1000);

	m_eyelashMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);

	cvSetZero(m_eyelashMask);


	m_pPeak = (CvPoint3D32f *) malloc(sizeof(CvPoint3D32f)*1000);

	int numMatrices = 100;	// assuming this is all we would need
	m_costMatrixFloat = (IplImage **) calloc(numMatrices, sizeof(IplImage *));
	m_costMatrixFloatMax = (IplImage **) calloc(numMatrices, sizeof(IplImage *));
	for(int i=0;i<100;i++)
	{
		m_costMatrixFloat[i] = (IplImage *) malloc(sizeof(IplImage));
		m_costMatrixFloatMax[i] = (IplImage *) malloc(sizeof(IplImage));
	}

	m_costMatrix = cvCreateImage(cvSize(100,100), IPL_DEPTH_32S, 1);
	m_countMatrix = cvCreateImage(cvSize(100, 100), IPL_DEPTH_8U, 1);

	m_flatGradImage = cvCreateImage(cvSize(m_imgFlatPyr[f_levels]->width, m_imgFlatPyr[f_levels]->height+1), IPL_DEPTH_16U, 1);
	m_flatGradImage->height -= 1;

	m_coarseSearchSampling=1.0;
	m_fineSearchSampling=0.5;
	m_fullCostData = (float *) malloc(100*100*3*sizeof(float));
	m_fullCostDataMax = (float *) malloc(100*100*3*sizeof(float));
	m_pScore = (float *) malloc(sizeof(float)*1000);
	m_eyelidSearchSampling = 1.0;


}
EyeSegmentServer::~EyeSegmentServer()
{
	for(int i=0;i<=m_levels;i++)
	{
		cvReleaseImageHeader(m_smoothImage + i);
		cvReleaseImage(m_imgPyr + i);
		cvReleaseImage(m_gradPyr + i);
		cvReleaseImage(m_imgFlatPyr + i);
#ifdef __EIGENREUSABLE__
		cvReleaseImageHeader(m_eigenvv + i);
#else
		cvReleaseImage(m_eigenvv + i);
#endif
//Madhav		cvReleaseImage(m_eigenvf+i);
		cvReleaseImage(m_maskPyr + i);
		cvReleaseImage(m_maskFlatPyr + i);
		m_smoothImage[i] = 0;
		m_imgPyr[i] = 0;
		m_gradPyr[i] = 0;
		m_eigenvv[i] = 0;
//Madhav		m_eigenvf[i] = 0;
		m_maskPyr[i] = 0;
		m_imgFlatPyr[i] = 0;
		m_maskFlatPyr[i] = 0;
	}
	cvReleaseImage(&m_reusableImage); m_reusableImage=0;
#ifdef __EIGENREUSABLE__
	cvReleaseImage(&m_eigenvvReusableImage); m_eigenvvReusableImage=0;
#endif

	cvReleaseHist(&m_outerRingHist);
	m_outerRingHist = 0;
	cvReleaseHist(&m_innerRingHist);
	m_innerRingHist = 0;
	cvReleaseHist(&m_tmpHist);
	m_tmpHist = 0;

	cvReleaseHist(&m_irisHistogram);
	m_irisHistogram = 0;

	cvReleaseImage(&m_maskImage);
	m_maskImage = 0;

	cvReleaseImage(&m_flatIris);
	m_flatIris = 0;

	cvReleaseImage(&m_flatIrisMask);
	m_flatIrisMask = 0;

	for(int i=0;i<m_angleSamples;i++)
		cvReleaseHist(m_annularHist+i);

	free(m_annularHist);
	m_annularHist = 0;

	cvReleaseMat(&m_emdFlow);
	cvReleaseMat(&m_transposedEMDFlow);

	if(m_upperEyelidMask)	cvReleaseImage(&m_upperEyelidMask);
	if(m_lowerEyelidMask)	cvReleaseImage(&m_lowerEyelidMask);


	delete [] m_EyeLidCosTable;
	delete [] m_EyeLidSinTable;
	cvReleaseImage(&m_dxTable);
	cvReleaseImage(&m_dyTable);


	ccomp_free(m_ccompServer);

	delete m_ccompServer;

	cvReleaseImage(&m_eyelashMask);

	free(m_fullCostData);
	free(m_fullCostDataMax);

	free(m_pPeak);
	free(m_pScore);

	for(int i=0;i<100;i++)
	{
		free(m_costMatrixFloat[i]);
		free(m_costMatrixFloatMax[i]);
	}

	free(m_costMatrixFloat);
	free(m_costMatrixFloatMax);

	cvReleaseImage(&m_costMatrix);
	cvReleaseImage(&m_countMatrix);

	cvReleaseImage(&m_flatGradImage);

}

void EyeSegmentServer::find_common_points(IplImage *ref, IplImage *ins, CvPoint3D32f *peaks, float *score, int *peakCount, CvPoint3D32f off, float threshold)
{
	int pc = 0;
	for(int i=0;i<ref->height; i++)
	{
		float *rptr = (float *) (ref->imageData + i*ref->widthStep);
		float *iptr = (float *) (ins->imageData + i*ins->widthStep);

		for(int j=0;j<ref->width;j++)
		{
			if(*rptr++ == *iptr++)
			{
				if(rptr[-1] > threshold)
				{
					peaks[pc].x = j + off.x;
					peaks[pc].y = i + off.y;
					peaks[pc].z = off.z;
					score[pc] = rptr[-1];
					pc++;
				}
			}
		}
	}

	*peakCount = pc;

	return;
}
extern void PrintImg(IplImage * gradMatrix);
/*{
	int h,w,ws;

	h = gradMatrix->height;
	w = gradMatrix->width;
	ws = gradMatrix->widthStep;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	printf("\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		unsigned char *ptr = (unsigned char *)gradMatrix->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			unsigned char t = *(ptr+colctr);
			printf("%4d ",t);
		}
		printf("\n");
	}
	printf("\n");
}
*/

void PrintCost(IplImage * costMatrixFloat,int scale)
{
	int h,w,ws;

	h = costMatrixFloat->height;
	w = costMatrixFloat->width;
	ws = costMatrixFloat->widthStep>>2;

//	printf("%#8x \n",(int)costMatrixFloat->imageData);
	printf("\n");
	for( int rowctr = 0; rowctr<h;rowctr++)
	{
		float *ptr = (float *)costMatrixFloat->imageData + rowctr*ws;
		for(int colctr = 0;colctr<w;colctr++)
		{
			float t = *(ptr+colctr);
			printf("%5.2f ",t/scale);
		}
		printf("\n");
	}
	printf("\n");
}



std::list<CircleParameters> EyeSegmentServer::findcircle(IplImage *image, IplImage *mask, CvPoint3D32f irsc, CvRect loc,
														 float lradius, float uradius, float scaling, float sigma,
														 float vert, float horz, float lambda, float min_angle,
														 float max_angle, bool pupil, int level)
{
#ifdef PROFILE
	QueryPerformanceFrequency(&profiler_freq);
	//TimeInfo = fopen("c:/profile_detail4.txt","a");
#endif

	//PROFILE_START(BS)

	unsigned short *gradptr=0;
	unsigned char *rsptr=0;
	int gradStep=0, reStep=0;

	IplImage *gradImage = m_gradPyr[level];

	cvGetRawData(image, &rsptr, &reStep);
	cvGetRawData(gradImage, (unsigned char **) &gradptr, &gradStep);
	gradStep /= sizeof(short);

	unsigned char *maskPtr = 0;
	int maskStep = 0;
	if(mask)
	{
		cvGetRawData(mask, &maskPtr, &maskStep);
	}
	else
	{
		maskPtr = (unsigned char *) calloc(gradImage->width, sizeof(char));
		maskStep = 0;
	}

	cvSetZero(gradImage);

	CvRect maskRect;// = {0,0,nw,nh};

	maskRect.x = cvRound(max(loc.x - uradius - 5, 1.0f));
	maskRect.y = cvRound(max(loc.y - uradius - 5, 1.0f));
	maskRect.height = cvRound(2*uradius + loc.height + 10);
	maskRect.width = cvRound(2*uradius + loc.width + 10);

	if(maskRect.x + maskRect.width > image->width-1) maskRect.width -= (maskRect.x + maskRect.width) - image->width + 1;
	if(maskRect.y + maskRect.height > image->height-1) maskRect.height -= (maskRect.y + maskRect.height) - image->height + 1;

	bool atCoarsestLevel = level == m_levels || (pupil && level == m_levels-1);

	short hzi = (short) cvRound(horz);
	short vzi = (short) cvRound(vert);

	unsigned short t1 = pupil ? 5*(1<<8) : 2*(1<<8); /* DJH via Manoj  11/06/02 */// error
	//unsigned short t1 = 5*(1 << 8);
	unsigned short t2 = pupil? 64*(1 << 8): 0xFFFF;

	for(int i=maskRect.y;i<maskRect.height+maskRect.y;i++)
	{
		unsigned char *rptr = rsptr + i * reStep + maskRect.x;
		unsigned char *rptr_p1 = rsptr + (i+1) * reStep + maskRect.x;
		unsigned char *rptr_m1 = rsptr + (i-1) * reStep + maskRect.x;
		unsigned short *gptr = gradptr + i * gradStep + maskRect.x;
		unsigned char *mptr = maskPtr + i *maskStep + maskRect.x;

		for(int j=0;j<maskRect.width;j++, rptr++, rptr_p1++, rptr_m1++)
		{
			short v = rptr[1] - rptr[-1];
			short h = rptr_p1[0] -  rptr_m1[0];
			unsigned short val = (*mptr++ )? 0: cvRound(cvSqrt((float) (h*h*hzi + v*v*vzi))*256);
			if(val < t1) val = 0;
			if(val > t2) val = t2;
			*gptr++ = val;
		}
	}

#if 0
	IppiSize gradRoi; gradRoi.width = maskRect.width, gradRoi.height = maskRect.height;

	IppStatus status = ippiSqrt_16u_C1IRSfs(gradptr + maskRect.y * gradStep + maskRect.x, m_gradPyr[level]->widthStep, gradRoi, -8);
	if(pupil)
	{
		cvSetImageROI(gradImage, maskRect);
		cvThreshold(gradImage, gradImage, 5*(1<<8), 0, CV_THRESH_TOZERO);
		cvThreshold(gradImage, gradImage, 64*(1<<8), 64*(1<<8), CV_THRESH_TRUNC);
		cvResetImageROI(gradImage);
	}
	else
	{
		cvSetImageROI(gradImage, maskRect);

		cvThreshold(gradImage, gradImage, 5*(1<<8), 0, CV_THRESH_TOZERO);
		cvResetImageROI(gradImage);

	}
#endif

	if(!mask) free(maskPtr);

	float sx = loc.x * scaling, sy = loc.y * scaling;
	float ex = (loc.x + loc.width)*scaling;
	float ey = (loc.y + loc.height)*scaling;

	float lradsc = (lradius*scaling);
	float uradsc = (uradius*scaling);

	float maxCostSearch = 0;

	double bias = atCoarsestLevel? m_coarseSearchSampling:m_fineSearchSampling;


	float deltaRad = 1.5f;

	//PROFILE_END(BS)
		//clock_t start = clock();
	//PROFILE_START(STEP1)
	int idx = 0;

	CvSize searchArea = cvSize(ceil(ex-sx), ceil(ey - sy));

	IplImage **costMatrixFloat = m_costMatrixFloat;
	IplImage **costMatrixFloatMax = m_costMatrixFloatMax;

	IplImage costMatrixHeader;
	cvInitImageHeader(&costMatrixHeader, searchArea, IPL_DEPTH_32S, 1);

	cvSetData(&costMatrixHeader, m_costMatrix->imageData, (searchArea.width << 2));

	IplImage *costMatrix = &costMatrixHeader;

	memset(m_fullCostData, 0, (searchArea.height+2)*(searchArea.width+2)*3*sizeof(int));

	m_pScore[0] = 0;

	int peakCountTotal = 0;

	CvSize sz; sz.width = searchArea.width; sz.height = searchArea.height;

	int offset = sizeof(int) * (sz.width + 2 + 1);

	std::list<CircleParameters>	circleList;

	CvPoint3D32f off = cvPoint3D32f(sx, sy, lradsc-bias);

	for(double rad = lradsc; rad <=uradsc; rad+=bias, idx++, off.z += bias)
	{
		double angleSample = 1.0/rad;

		costMatrixFloat[idx] = cvInitImageHeader(costMatrixFloat[idx], searchArea, IPL_DEPTH_32F, 1);
		costMatrixFloatMax[idx] = cvInitImageHeader(costMatrixFloatMax[idx], searchArea, IPL_DEPTH_32F, 1);

		cvSetData(costMatrixFloat[idx], m_fullCostData + (idx%3)*(sz.height+2)*(sz.width+2), (sz.width + 2)* sizeof(float));
		cvSetData(costMatrixFloatMax[idx], m_fullCostDataMax + (idx%3)*(sz.height+2)*(sz.width+2), (sz.width + 2)* sizeof(float));

		costMatrixFloat[idx]->imageData += offset;
		costMatrixFloatMax[idx]->imageData += offset;

		for(double bsy = 0; bsy < 1.0; bsy += bias)
		{
			for(double bsx = 0; bsx < 1.0; bsx += bias)
			{
				int nang = 0;
				cvSetZero(costMatrix);
				for(double ang = min_angle; ang <= max_angle; ang += angleSample, nang++)
				{
					double cs = cos(ang);
					double sn = sin(ang);
					CvPoint2D32f pt, ptin, ptout;

					pt.x = sx + bsx + cs*rad;
					pt.y = sy + bsy + sn*rad;

					ptin.x = sx + bsx + cs * (rad - deltaRad);
					ptin.y = sy + bsy + sn * (rad - deltaRad);

					ptout.x = sx + bsx + cs * (rad + deltaRad);
					ptout.y = sy + bsy + sn * (rad + deltaRad);

					ComputeIntegralContribution(m_gradPyr[level], image, costMatrix, pt, ptin, ptout, searchArea);

					pt.x = sx + bsx - cs*rad;
					ptin.x = sx + bsx - cs * (rad - deltaRad);
					ptout.x = sx + bsx - cs * (rad + deltaRad);

					ComputeIntegralContribution(m_gradPyr[level], image, costMatrix, pt, ptin, ptout, searchArea);
				}

				cvConvertScale(costMatrix, costMatrixFloat[idx], 0.5/(65536*nang));
#ifdef PRINT_DEBUG
				printf("\nIdx = %d",idx%3);
				PrintCost_float(costMatrixFloat[idx],65536*2);
#endif
				if(atCoarsestLevel)
				{
					cvDilate(costMatrixFloat[idx], costMatrixFloatMax[idx]);

					int peakCount = 0;

					if(idx==1)
					{
						cvMax(costMatrixFloatMax[idx], costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-1]);

						find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-1], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);

					}
					else if(idx > 1)
					{
						cvMax(costMatrixFloatMax[idx], costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-1]);
						cvMax(costMatrixFloatMax[idx-1], costMatrixFloatMax[idx-2], costMatrixFloatMax[idx-2]);

						find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-2], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);
					}

#ifdef PRINT_DEBUG
				    printf("\nPeakCount = %03d \n",peakCount);
				    printf("Radius = %5.2f \n",off.z);
				    printf(" x    y   score \n");
				    for(int i = peakCountTotal;i<peakCountTotal+peakCount;i++)
				    {
				    	printf("%03d  ",(int)m_pPeak[i].x);
				    	printf("%03d  ",(int)m_pPeak[i].y);
				    	printf("%4.2f \n",(m_pScore[i]/(65536.0*2.0)));
				    }
				    printf("\n");
#endif

					peakCountTotal += peakCount;
				}
				else
				{
					CvPoint minLoc, maxLoc;
					double maxVal = 0, minVal = 0;
					peakCountTotal = 1;
//					PrintCost(costMatrixFloat[idx],1);
					cvMinMaxLoc(costMatrixFloat[idx], &minVal, &maxVal, &minLoc, &maxLoc);
#ifdef PRINT_DEBUG
					printf("Radius = %5.2f \n",rad);
					printf("%4.2f  ",bsx);
					printf("%4.2f  ",bsy);

					printf("%4.2f ",off.x);
					printf("%4.2f  ",off.y);

				    printf("%03d  ",maxLoc.x);
				    printf("%03d  ",maxLoc.y);
				    printf("%4.2f \n",maxVal);
#endif

					if((float) maxVal > m_pScore[0])
					{
						m_pScore[0] = (float) maxVal;
						m_pPeak[0].x = maxLoc.x + off.x + bsx;
						m_pPeak[0].y = maxLoc.y + off.y + bsy;
						m_pPeak[0].z = rad;
					}
				}
			}
		}
	}




	int bidx = 0;
	if(atCoarsestLevel)
	{
		int peakCount = 0;
		find_common_points(costMatrixFloat[idx-1], costMatrixFloatMax[idx-2], m_pPeak + peakCountTotal, m_pScore + peakCountTotal, &peakCount, off);
		peakCountTotal += peakCount;

		for(int i=0;i<peakCountTotal; i++)
			if(m_pScore[i] > m_pScore[bidx])
				bidx = i;
	}
#ifdef SEG
	printf("\nPeakCountTotal = %03d \n",peakCountTotal);
#endif
#ifdef PRINT_DEBUG
	printf(" x    y   score \n");
	for(int i = 0;i<peakCountTotal;i++)
	{
		printf("%03d  ",(int)m_pPeak[i].x);
		printf("%03d  ",(int)m_pPeak[i].y);
		printf("%4.2f  ",m_pPeak[i].z);
		printf("%4.2f \n",m_pScore[i]);
	}
#endif
	// printf("\n");
	// NEED to handle situation where peakCountTotal = 0

	//PROFILE_START(TOTREFINE)

	if(!atCoarsestLevel)
	{
		CircleParameters circ;
		memset(&circ, 0, sizeof(CircleParameters));

		circ.cp = m_pPeak[bidx];
		circleList.push_back(circ);
	}
	else
	{

		int processedPeaks = 0;
		for(int i=0;i<peakCountTotal;i++)
		{
			float neck = pupil ? 0:0.25f;

			if(m_pScore[i] < neck * m_pScore[bidx])
				continue;

			processedPeaks++;

//			printf("%f %f %f %f\n", m_pPeak[i].x, m_pPeak[i].y, m_pPeak[i].z, m_pScore[i]);

			CvPoint3D32f pt = m_pPeak[i];

			maxCostSearch = 0;

			//PROFILE_START(REFINEMENT)

			float radbest;
			CvPoint2D32f pt2d;

			pt2d.x = pt.x;
			pt2d.y = pt.y;
			radbest = pt.z;

#if 0//Manoj-Madhav Telecon //6May2009
			//PROFILE_START(HISTCOST)
			float histCost = RegionHistogramDistance(image, mask, pt2d, (pupil? 0.0f: radbest - 5), radbest, radbest + 5);
			//PROFILE_END(HISTCOST)
#else
			float histCost = 0;
#endif
			float measure = 0;
			float pupil_height = 0, pupil_width = 0;

#if 0 //Manoj-Madhav Telecon //6May2009
			if(!pupil)
			{
				cvResize(mask, m_maskPyr[m_levels-1], CV_INTER_NN);
				ComputeAnnularSectionHistograms(m_imgPyr[m_levels-1], m_maskPyr[m_levels-1], cvPoint3D32f(2*pt2d.x, 2*pt2d.y, 2*radbest), 0);
				//measure = ComputeLeftRightSimilarity(m_imgPyr[m_levels-1], m_maskPyr[m_levels-1], cvPoint3D32f(2*pt2d.x, 2*pt2d.y, 2*radbest), 0, pupil); // For Iris
				EstimatePupilCenter(m_imgPyr[m_levels-1], cvPoint3D32f(2*pt2d.x, 2*pt2d.y, 2*radbest), m_irisHistogram);
				pupil_height = m_EstimatedPupil.second*0.5f;
				pupil_width = m_EstimatedPupil.first*0.5f;
				measure = 0;
			}
#endif
			CircleParameters circ;
			circ.cp = cvPoint3D32f(pt2d.x, pt2d.y, radbest);
			circ.ic = m_pScore[i];
			circ.hc = histCost;
			circ.sm = measure;
			circ.eph = pupil_height;
			circ.epw = pupil_width;

			circleList.push_back(circ);

		}
#ifdef SEG
		printf("Num maximas = %d, pupil = %d\n", processedPeaks, (int) pupil);
#endif
	}
	//PROFILE_END(TOTREFINE);
#ifdef SEG
	printf("Number of circles  = %3d\n",circleList.size());
#endif
	std::list<CircleParameters>::iterator i;
#ifdef PRINT_DEBUG
	for(i=circleList.begin(); i != circleList.end();i++)
	{	CircleParameters circ;
		circ = *i;
    	printf("%4.2f  ",circ.cp.x);
    	printf("%4.2f  ",circ.cp.y);
    	printf("%4.2f ",circ.cp.z);
    	printf("%4.2f ",circ.ic);
    	printf("%4.2f ",circ.hc);
    	printf("%4.2f ",circ.sm);
    	printf("%4.2f ",circ.eph);
    	printf("%4.2f \n",circ.epw);
	}
#endif
	return circleList;
}




void EyeSegmentServer::RefineCircle1( IplImage *image, int level, bool pupil, CvPoint3D32f pt, CvPoint3D32f & ptnew )
{
	float *gradptr=0;
	unsigned char *rsptr=0;
	int gradStep=0, reStep=0;

	IplImage *mask = m_maskPyr[level];
	//cvSaveImage("c:/mask.bmp",mask);

	float lambda, lradius, uradius;
	if( pupil )
	{
		lradius = m_startPupilRadius/(1 << level);
		uradius = m_endPupilRadius/(1 << level);
		lambda = 2.0f;
	}
	else
	{
		lradius = m_startIrisRadius/(1 << level);
		uradius = m_endIrisRadius/(1 << level);
		lambda = 3.0f;
	}

	CvRect loc;
	if( pupil )
	{
		loc = cvRect(m_centerRect.x-8, m_centerRect.y-8, m_centerRect.width+16, m_centerRect.height+16);
	}
	else
	{
		loc = m_centerRect;
	}
	loc.x = (loc.x >> level);
	loc.y = (loc.y >> level);
	loc.width = (loc.width >> level);
	loc.height = (loc.height >> level);

	CvRect maskRect;// = {0,0,nw,nh};
	maskRect.x = cvRound(max(loc.x - uradius - 5, 1.0f));
	maskRect.y = cvRound(max(loc.y - uradius - 5, 1.0f));
	maskRect.height = cvRound(2*uradius + loc.height + 10);
	maskRect.width = cvRound(2*uradius + loc.width + 10);

	IplImage *gradImage = m_gradPyr[level];

//	IplImage *gradImage = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_32F, 1);

	cvGetRawData(image, &rsptr, &reStep);
	cvGetRawData(gradImage, (unsigned char **) &gradptr, &gradStep);
	gradStep /= sizeof(float);

	unsigned char *maskPtr = 0;
	int maskStep = 0;
	cvGetRawData(mask, &maskPtr, &maskStep);

	if(maskRect.x + maskRect.width > image->width-1) maskRect.width -= (maskRect.x + maskRect.width) - image->width + 1;
	if(maskRect.y + maskRect.height > image->height-1) maskRect.height -= (maskRect.y + maskRect.height) - image->height + 1;

	float horz = 1.0f;
	float vert = 1.0f;
	for(int i=maskRect.y;i <maskRect.height+maskRect.y; i++ )
	{
		unsigned char *rptr = rsptr + i * reStep + maskRect.x;
		unsigned char *rptr_p1 = rsptr + (i+1) * reStep + maskRect.x;
		unsigned char *rptr_m1 = rsptr + (i-1) * reStep + maskRect.x;
		float *gptr = gradptr + i * gradStep + maskRect.x;
		unsigned char *mptr = maskPtr + i *maskStep + maskRect.x;

		for(int j=0;j<maskRect.width;j++, rptr++, rptr_p1++, rptr_m1++)
		{
			int v = rptr[1] - rptr[-1];
			int h = rptr_p1[0] -  rptr_m1[0];
			float gradVal = cvSqrt(h*h*horz + v*v*vert);
			*gptr++ = (*mptr++ || gradVal < 10)? 0:gradVal;
		}
	}

	float scaling = 1.0f; /// Hard Coding
	float sx = loc.x * scaling, sy = loc.y * scaling;
	float ex = (loc.x + loc.width)*scaling;
	float ey = (loc.y + loc.height)*scaling;
	//lradius = lradius*scaling;
	//uradius = uradius*scaling;

	CvPoint2D32f pt2d;
	float bestRad;
	if( pupil )
	{
		RefineCircle( gradImage, image, mask, sx, ex, sy, ey, lradius, uradius, m_minPupilAngle, m_maxPupilAngle,
					lambda, level, pupil, pt, pt2d, bestRad );
	}
	else
	{
		//float irisminAngle = 0.0*CV_PI/180.0;
		RefineCircle( gradImage, image, mask, sx, ex, sy, ey, lradius, uradius, /*irisminAngle*/m_minIrisAngle, m_maxIrisAngle,
					lambda, level, pupil, pt, pt2d, bestRad );
	}
	ptnew.x = pt2d.x;
	ptnew.y = pt2d.y;
	ptnew.z = bestRad;
//	cvReleaseImage(&gradImage);
}

void EyeSegmentServer::RefineCircle( IplImage *gradImage, IplImage *image, IplImage *mask, float sx, float ex, float sy, float ey,
									float lradsc, float uradsc, float min_angle, float max_angle, float lambda, int level,
									bool pupil, CvPoint3D32f pt, CvPoint2D32f & pt2d, float & bestRad )
{
	float deltaRad = 1.5f;
	float maximaBiasr, maximaBiasy;
	maximaBiasr = 0.5f;
	maximaBiasy = 0.5f;
	/*if( pupil )
	{
		maximaBiasr = 0.5f;
		maximaBiasy = 0.5f;
	}
	else
	{
		maximaBiasr = 3.0f;
		maximaBiasy = 3.0f;
	}*/
	float maximaBiasx = 0.5f;
	float maxCostSearch = 0;
	float max_grad = 0;

	CvPoint2D32f bestptSearch = {(sx+ex)/2, (sy+ey)/2};
	float bestRadSearch = (lradsc + uradsc)/2;;

	for(float rad = pt.x-maximaBiasr; rad <= pt.x+maximaBiasr; rad+=0.5f)
	{
		if( rad <= lradsc || rad >= uradsc )
			continue;

		float angleSampleTable = 0.5f/rad;

		int numSampleTable = cvFloor((max_angle - min_angle + 2)/angleSampleTable);

		CirclePoints *sinCosTable = (CirclePoints *) malloc(numSampleTable*sizeof(CirclePoints));

		generateSinCosTable(angleSampleTable,min_angle,max_angle,rad, deltaRad, sinCosTable);

		for(float x = pt.y-maximaBiasx; x <= pt.y+maximaBiasx; x+=0.5f)
		{
			if(x <= sx || x >= ex-0.5f)	continue;

			for(float y = pt.z-maximaBiasy; y <= pt.z+maximaBiasy; y+=0.5f)
			{
				if(y <= sy || y >= ey-0.5f)	continue;

				CvPoint2D32f Pt = {x , y};

				float cost = GetIntegral(gradImage, image, Pt, rad, min_angle, max_angle, max_grad, sinCosTable, angleSampleTable,numSampleTable,pupil);

				/*FILE *fp = fopen("c:/cost.txt","a");
				fprintf(fp,"%f\n",cost);
				fclose(fp);
				*/float distCost = 0;

				if(lambda > 0.0f)
				{
					distCost = RegionHistogramDistance(image, mask, Pt, (pupil? 0.0f: rad - 5), rad, rad + 5);
				}
				cost += lambda * distCost;

				if(cost > maxCostSearch)
				{
					maxCostSearch = cost;
					bestptSearch = Pt;
					bestRadSearch = rad;
				}
			}
		}
		free(sinCosTable);
	}

	pt2d = bestptSearch;
	bestRad = bestRadSearch;
}


#if 0 // Anita
std::list<CircleParameters> EyeSegmentServer::FindIrisMaximas(IplImage *image,int tag)
{
#ifdef FINDIRIS
	FILE *fp = fopen("SegmentPluginOutPut_EyeSegmentServer.txt", "at");
	fprintf(fp,"I am here");
#endif
	CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::FindIrisMaximas:1, m_availablePyramidLevel = %d,ih=%d, iw=%d,ph=%d, pw=%d,m_startIrisRadius=%d,m_endIrisRadius=%d",m_availablePyramidLevel,image->height,image->width,m_imgPyr[m_availablePyramidLevel]->height,m_imgPyr[m_availablePyramidLevel]->width,m_startIrisRadius,m_endIrisRadius);
#endif
	cvCopy(image, m_imgPyr[m_availablePyramidLevel]);
	
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::FindIrisMaximas:2");
#endif
	float startRadius = m_startIrisRadius;
	float endRadius = m_endIrisRadius;

	CvRect cloc = m_centerRect;

	// compute the image pyramid
	for(int loopi=m_availablePyramidLevel;loopi<m_levels;loopi++){
		cvPyrDown(m_imgPyr[loopi], m_imgPyr[loopi+1]);
	}
	
	// compute the image gradient pyramid
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::FindIrisMaximas:3");
#endif
	int i=m_levels;

	float scaling = 1.0f;
	float sigma = 0.5f;

	startRadius /= (1 << i);
	endRadius /= (1 << i);
	cloc.x = (cloc.x >> i);
	cloc.y = (cloc.y >> i);
	cloc.width = (cloc.width >> i);
	cloc.height = (cloc.height >> i);

	CvRect maskRect;

	maskRect.x = cvRound(max(cloc.x - endRadius - 5, 0.0f));
	maskRect.y = cvRound(max(cloc.y - endRadius - 5, 0.0f));
	maskRect.height = cvRound(min(2*endRadius + cloc.height + 10, (float)m_imgPyr[i]->height));
	maskRect.width = cvRound(min(2*endRadius + cloc.width + 10, (float)m_imgPyr[i]->width));

	
	MaskSpecularities(m_imgPyr[i],maskRect,m_maskPyr[i],i); //turn back on for non CASIA ver 1
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	std::list<CircleParameters> rv;
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::FindIrisMaximas:4");
#endif
	rv=findcircle(m_imgPyr[i], m_maskPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 3.0f, m_minIrisAngle, m_maxIrisAngle, false, i);
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::FindIrisMaximas:5");
	fclose(fp);
#endif
	return rv;

}
#else
std::list<CircleParameters> EyeSegmentServer::FindIrisMaximas(IplImage *image,int tag)
{
	CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};
	XTIME_OP("CopyImage",
	cvCopy(image, m_imgPyr[0])
	);

	float startRadius = m_startIrisRadius;
	float endRadius = m_endIrisRadius;

	CvRect cloc = m_centerRect;

	// compute the image pyramid
	XTIME_OP("cvPyrDownLoop",
	for(int loopi=0;loopi<m_levels;loopi++){
		cvPyrDown(m_imgPyr[loopi], m_imgPyr[loopi+1]);
	}
	);

	// compute the image gradient pyramid

	int i=m_levels;

	float scaling = 1.0f;
	float sigma = 0.5f;

	startRadius /= (1 << i);
	endRadius /= (1 << i);
	cloc.x = (cloc.x >> i);
	cloc.y = (cloc.y >> i);
	cloc.width = (cloc.width >> i);
	cloc.height = (cloc.height >> i);

	CvRect maskRect;

	maskRect.x = cvRound(max(cloc.x - endRadius - 5, 0.0f));
	maskRect.y = cvRound(max(cloc.y - endRadius - 5, 0.0f));
	maskRect.height = cvRound(min(2*endRadius + cloc.height + 10, (float)m_imgPyr[i]->height));
	maskRect.width = cvRound(min(2*endRadius + cloc.width + 10, (float)m_imgPyr[i]->width));

	XTIME_OP("MaskSpecularities",
	MaskSpecularities(m_imgPyr[i],maskRect,m_maskPyr[i],i) //turn back on for non CASIA ver 1
	);

	std::list<CircleParameters> rv;

	XTIME_OP("FindIrisMaximas-findcircle",
	rv=findcircle(m_imgPyr[i], m_maskPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 3.0f, m_minIrisAngle, m_maxIrisAngle, false, i)
	);
	return rv;

}
#endif

#if 0
void EyeSegmentServer::MaskSpecularities(IplImage *image, CvRect rect, IplImage *mask, int level, bool pupil)
{
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	// MaskSpecularities_Fix(image,rect,mask, m_eigenvv,level, pupil); // ,m_ccompServer->scr);
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
#if 0
	char fName[100];
	static int test =0;
	sprintf(fName,"Mask_%d.pgm",test);
	cvSaveImage(fName,mask );

	sprintf(fName,"Inp_%d.pgm",test);
	cvSaveImage(fName,image );
	test++;
#endif

}

#else

// To be used as on 28th March 2018 - EXT
void EyeSegmentServer::  MaskSpecularities(IplImage *image, CvRect rect, IplImage *mask, int level, bool pupil)
{
	cvSetImageROI(image,rect);
/*
	printf("\n rect %d %d %d %d \n",rect.x,rect.y,rect.width,rect.height);
	printf("\n IMAGE %d %d %d \n",image->width,image->widthStep,image->height);
	for(int i =0 ;i<image->height;i++)
	{
		for(int j = 0;j<image->width;j++)
		{
			printf("%3d,",(unsigned char *)*(image->imageData+i*image->widthStep+j));
		}
		printf("\n");
	}
*/
	cvSetImageROI(m_eigenvv[level],cvRect(rect.x*6,rect.y,rect.width*6,rect.height));

	//PROFILE_START(EIG)
	cvCornerEigenValsAndVecs(image, m_eigenvv[level], 5, 3);
	//PROFILE_END(EIG)

	cvResetImageROI(image);
	cvResetImageROI(m_eigenvv[level]);

	cvSetZero(mask);

	float *t_pEigenVV;
	int t_eigStep;

	cvGetRawData(m_eigenvv[level], (unsigned char **) &t_pEigenVV, &t_eigStep);
	t_eigStep /= sizeof(float);

	//int *maxHist = (int *) calloc(1000, sizeof(int));
	//int *minHist = (int *) calloc(1000, sizeof(int));

	float t_maxMinEigValue = 0.0f;
/*
	printf("\n EIGEN \n");
	for(int i =0 ;i<image->height;i++)
	{
		for(int j = 0;j<image->width;j++)
		{
			float *ptr = t_pEigenVV +i* t_eigStep + j*6;
			printf("%10.7f, %10.7f,",*ptr,*(ptr+1));
		}
		printf("\n");
	}
*/
	for(int loopi=rect.y;loopi<rect.height+rect.y;loopi++)
	{
		float *t_eig = t_pEigenVV + loopi * t_eigStep + 6*rect.x;

		for(int loopj=0;loopj<rect.width; loopj++, t_eig+=6)
		{
			float t_mineig = min(t_eig[0], t_eig[1]);
			//float maxeig =  max(eig[0], eig[1]);

			if( t_mineig > t_maxMinEigValue )
				t_maxMinEigValue = t_mineig;
		}
	}
//	printf("\n Min %10.7f \n",t_maxMinEigValue);

	unsigned char *t_pMaskPtr = 0;
	int t_maskStep = 0;

	cvGetRawData(mask, (unsigned char **) &t_pMaskPtr, &t_maskStep);

	float t_eigenTH = ( t_maxMinEigValue > 0.01 )? 0.2f:0.25f;

	for(int loopi=rect.y;loopi<rect.height+rect.y;loopi++)
	{
		float *t_eig = t_pEigenVV + loopi * t_eigStep + 6*rect.x;
		unsigned char *t_mptr = t_pMaskPtr + loopi*t_maskStep + rect.x;

		for(int loopj=0;loopj<rect.width; loopj++, t_eig+=6)
		{
			float t_mineig = min(t_eig[0], t_eig[1]);
			//// MAGIC NUMBER ( WAS 0.25 before )
			*t_mptr++ = (t_mineig > t_eigenTH*t_maxMinEigValue)? 255:0;
		}
	}
/*
	printf("\n EIGEN \n");
	for(int i =0 ;i<mask->height;i++)
	{
		for(int j = 0;j<mask->width;j++)
		{
			printf("%3d,",(unsigned char *)*(mask->imageData+i*mask->widthStep+j));
		}
		printf("\n");
	}
*/
/*
	char fName[100];
	sprintf(fName,"Mask_%d.pgm",test);
	cvSaveImage(fName,mask );

	sprintf(fName,"Inp_%d.pgm",test);
	cvSaveImage(fName,image );
	test++;
*/
	return;

}
#endif

std::list<CircleParameters> EyeSegmentServer::FindPupilMaximas(IplImage *image,int tag)
{
	//#ifdef PROFILE
	//QueryPerformanceFrequency(&profiler_freq);
	////TimeInfo = fopen("c:/profile_detail4.txt","a");
	//#endif

	//PROFILE_START(BASICSTEPS)
	float startRadius = m_startPupilRadius;
	float endRadius = m_endPupilRadius;

	CvRect cloc = cvRect(m_centerRect.x-8, m_centerRect.y-8, m_centerRect.width+16, m_centerRect.height+16);

	CvPoint3D32f nullPt = {0.0f, 0.0f, 0.0f};

	CvRect maskRect = m_irisRect;

	int i=m_levels-1;

	float scaling = 1.0f;
	float sigma = 0.5f;

	startRadius /= (1 << i);
	endRadius /= (1 << i);

	cloc.x = (cloc.x >> i);
	cloc.y = (cloc.y >> i);
	cloc.width = (cloc.width >> i);
	cloc.height = (cloc.height >> i);

#if 0
	std::list<CircleParameters>::iterator st = m_irisCircleList.begin(), en = m_irisCircleList.end(), penend;
	float minx = (*st).epw, miny = (*st).eph, maxx = (*st).epw, maxy = (*st).eph;

	for(penend = st; st != en ; st++ )
	{
		minx = min( minx, (*st).epw );
		miny = min( miny, (*st).eph );
		maxx = max( maxx, (*st).epw );
		maxy = max( maxy, (*st).eph );
		if((*penend).ic < (*st).ic)
			penend = st;
	}

	minx = min( minx, (*penend).cp.x );
	miny = min( miny, (*penend).cp.y );
	maxx = max( maxx, (*penend).cp.x );
	maxy = max( maxy, (*penend).cp.y );

	cloc.x = cvRound(max((float)cloc.x, minx*2) - 5);
	cloc.y = cvRound(max((float)cloc.y, miny*2) - 5);
	cloc.width = cvRound(min((float)cloc.width,(maxx-minx)*2) + 10);
	cloc.height = cvRound(min((float)cloc.height, (maxy-miny)*2) + 10);
#endif

	maskRect.x = (m_irisRect.x >> i);
	maskRect.y = (m_irisRect.y >> i);
	maskRect.height = (m_irisRect.height >> i);
	maskRect.width = (m_irisRect.width >> i);
	//PROFILE_END(BASICSTEPS)

	//PROFILE_START(MASKSPECS)
	
	MaskSpecularities(m_imgPyr[i],maskRect,m_maskPyr[i],i); // turn back on for non CASIA ver 1
	
	//PROFILE_END(MASKSPECS)

	//PROFILE_START(FINDCIRCLE)
	//std::list<CircleParameters> test = findcircle(m_imgPyr[i], m_maskPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 2.0f, m_minPupilAngle, m_maxPupilAngle, true, i);
	std::list<CircleParameters> rv;
	
	rv= findcircle(m_imgPyr[i], m_maskPyr[i], nullPt, cloc, startRadius, endRadius, scaling, sigma, 1.0f, 1.0f, 2.0f, m_minPupilAngle, m_maxPupilAngle, true, i);
	
	return rv;
	//PROFILE_END(BASICSTEPS)
}

void EyeSegmentServer::ReturnBadSegmentation( )
{
	m_eso.ip.x = -1;
	m_eso.ip.y = -1;
	m_eso.ip.z = -1;

	m_eso.pp.x = -1;
	m_eso.pp.y = -1;
	m_eso.pp.z = -1;

	cvSet(m_flatIris,cvScalar(0));
	cvSet(m_flatIrisMask,cvScalar(0));
	m_eso.flatIris = m_flatIris;
	m_eso.flatMask = m_flatIrisMask;

	m_irisCircleList.empty();
	m_pupilCircleList.empty();

}
int labelindx;
EyeSegmentationOutput EyeSegmentServer::Process(IplImage* image, int index)
{
#ifdef FINDIRIS
	FILE *fp = fopen("SegmentServer.txt", "at");
	fprintf(fp,"EyeSegmentServer::Process:1\n");
#endif
	m_eso.ip.x = -1;
	labelindx = index;

	// DebugPrint( "--------------------------Image %d------------------------\n", index );
//	DebugPrint("Iris Boundary");

	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	m_irisCircleList.empty();
	m_pupilCircleList.empty();
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::Process:2\n");
#endif
	m_irisCircleList = FindIrisMaximas( image, 0 ); // tag 0 for Iris
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
#ifdef FINDIRIS
	fprintf(fp,"EyeSegmentServer::Process:3\n");
//	DebugPrint("...done\n");
	fclose(fp);
#endif
// 	DebugPrint("Pupil Boundary\n");


	if( !(m_irisCircleList.size()) )
	{
		ReturnBadSegmentation( );
		return m_eso;
	}
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	
	m_pupilCircleList = FindPupilMaximas(image,1); // tag 1 for pupil
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	if( !(m_pupilCircleList.size()) )
	{
		ReturnBadSegmentation( );
		return m_eso;
	}


	// DebugPrint("...done\n");

	// XTIME_OP("FindIrisAndPupil",
	FindIrisAndPupil(m_irisCircleList, m_pupilCircleList); 
	//);
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);

		char imageName[100];
		IplImage * tmp = cvCloneImage(image);
		sprintf(imageName,"aSegImage.pgm");
		CvScalar color = cvRealScalar(255);
		//CvPoint3D32f irisPnt={m_Iris.x/2,m_Iris.y/2,m_Iris.z/2};
		//CvPoint3D32f pupPnt={m_Pupil.x/2,m_Pupil.y/2,m_Pupil.z/2};

		CvPoint3D32f irisPnt={m_Iris.x,m_Iris.y,m_Iris.z};
		CvPoint3D32f pupPnt={m_Pupil.x,m_Pupil.y,m_Pupil.z};

		draw( tmp, irisPnt, color );
		draw( tmp,pupPnt, color );
		// cvSaveImage(imageName,tmp,0);

		cvRelease((void**)&tmp);



	if( m_Iris.x <= 0 )
	{
		printf("*********m_Iris & pupil CircleList bad seg *************\n");
		ReturnBadSegmentation( );
		return m_eso;
	}


#if 0
	CvPoint3D32f pt;
	pt.x = m_Iris.z/(1<<3);
	pt.y = m_Iris.x/(1<<3);
	pt.z = m_Iris.y/(1<<3);
	XTIME_OP("RefineCircle1",
	RefineCircle1( m_imgPyr[3], 3, false, pt, m_Iris )
	);
	m_Iris.x *= 1<<3;
	m_Iris.y *= 1<<3;
	m_Iris.z *= 1<<3;

	pt.x = m_Pupil.z/(1<<2);
	pt.y = m_Pupil.x/(1<<2);
	pt.z = m_Pupil.y/(1<<2);
	XTIME_OP("RefineCircle1",
	RefineCircle1( m_imgPyr[2], 2, true, pt, m_Pupil )
	);
	m_Pupil.x *= 1<<2;
	m_Pupil.y *= 1<<2;
	m_Pupil.z *= 1<<2;
#endif
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	
	FindBoundary(m_Pupil, 1, true);
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	
	FindBoundary(m_Iris, 2, false);
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);

	m_eso.ip.x = m_Iris.x;
	m_eso.ip.y = m_Iris.y;
	m_eso.ip.z = m_Iris.z;

	m_eso.pp.x = m_Pupil.x;
	m_eso.pp.y = m_Pupil.y;
	m_eso.pp.z = m_Pupil.z;

	m_eyelidDetected = true;
	m_iseye = true;


	
	GeneratePolarWarpTable(m_eso.ip, m_eso.pp);
	
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	IplImage *flatIris = m_eso.flatIris = FlattenIris(image, m_eso.ip, m_eso.pp);

	
	MaskSpecularitiesFlat(m_eso.ip, m_eso.pp);
	


	m_eso.specMask = m_maskPyr[0];

	cvCopy(flatIris, m_imgFlatPyr[0]);
	cvCopy(m_flatIrisMask,m_maskFlatPyr[0]);

	
	for(int i=0;i<f_levels;i++)
	{
		cvPyrDown(m_maskFlatPyr[i], m_maskFlatPyr[i+1]);
		cvPyrDown(m_imgFlatPyr[i], m_imgFlatPyr[i+1]);
	}
	

	//"Upper Eye Lid.........\n");
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	
	m_eso.UpperEyeLid = FindUpperEyelidBoundaries(m_imgFlatPyr[f_levels], m_maskFlatPyr[f_levels]);
	

	//DebugPrint("Done\n");
	//DebugPrint("Lower Eye Lid.........\n");
	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);

	
	m_eso.LowerEyeLid = FindLowerEyelidBoundaries(m_imgFlatPyr[f_levels], m_maskFlatPyr[f_levels]);
	

	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);
	//DebugPrint("Done\n");

	
	m_eso.flatMask = GenerateMask();
	

	//printHexBytes(__FILE__,__LINE__, m_ccompServer->scr);

	if(m_enableEyelidSegmentation){
		//;//TODO BK UnCOMMENT
		
		findEyeLashes(m_eso.flatIris,m_eso.flatMask);
		
	}

	if(m_eso.pp.z <= 22.0)
	{
		// char nameOrig[100];
		//char name[100];
		// static int count =0;

		// cv::Mat mateyeOrig = cv::cvarrToMat(m_eso.flatMask);
		// sprintf(nameOrig, "FlatMask_Orig_%d.pgm",count);
		//imwrite(nameOrig, mateyeOrig);

		cv::Mat mateye = cv::cvarrToMat(m_eso.flatMask);
		// printf("cols..%d row...%d\n", mateye.cols, mateye.rows);
		mateye.rowRange(16,48).setTo(cv::Scalar(255));
		// mateye.rowRange(48,63).setTo(cv::Scalar(255));
		IplImage test = mateye;
		cvCopy(&test, m_eso.flatMask);

		// sprintf(name, "FlatMask_%d.pgm",count);
		//imwrite(name, mateye);
		// count++;
	}

	m_irisCircleList.empty();
	m_pupilCircleList.empty();

	return m_eso;
}


