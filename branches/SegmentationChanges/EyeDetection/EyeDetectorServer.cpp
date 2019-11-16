#include "EyeDetectorServer.h"
#include "HRect.h"
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "assert.h"
#define USE_MMX 0
#define DO_DY_OUTSIDE 0
using std::max;
extern "C"{
	#include "file_manip.h"
}

#ifdef __ANDRIOD__
	#include "logging.h"
	#define printf LOGI
#endif


#define GET_IMAGE_PIXEL(image, index) ((unsigned char) image->imageData[index])

#define TEST_BOUNDS 0

#if TEST_BOUNDS
#define ALWAYS_OFF(x) 0
#define ALWAYS_ON(x) 1
#else
#define ALWAYS_OFF(x) x
#define ALWAYS_ON(x) x
#endif

EyeDetectorServer::EyeDetectorServer(void)
{
	m_pEyeCenterMask = 0;
}

EyeDetectorServer::EyeDetectorServer(int width, int height)
{
	Alloc(width, height);
}

EyeDetectorServer::~EyeDetectorServer(void)
{
	Free();
}

void EyeDetectorServer::Resize(int width, int height)
{
	Free();
	Alloc(width, height);
}

void EyeDetectorServer::Alloc(int width, int height)
{
	m_ImageWidth = width;
	m_ImageHeight = height;
	m_pEyeCenterMask = new Image8u(width, height);
	m_pBlur = new Image8u(width, height);
}

void EyeDetectorServer::Free()
{
	if(m_pEyeCenterMask)
	{
		delete m_pEyeCenterMask;
		m_pEyeCenterMask = 0;
	}

	if(m_pBlur)
	{
		delete m_pBlur;
		m_pBlur = 0;
	}
}

HU_MOMENTS EyeDetectorServer::ComputeSpecularityMetrics(Image8u* frame, int x, int y, int level, int radius)
{
	HU_MOMENTS mts;

	double mx0 = 0, my0 = 0, tval = 0, mx2 = 0, my2 = 0, mxy = 0;

	//printf("testing specularity: (%d, %d)\n", y, x);

	// Do small search for max specularity:
	int maxValue = 0, peakThreshold = 0;
	int nx = x, ny = y;

	HRect frameRoi(0, 0, frame->GetWidth(), frame->GetHeight());
	int searchWidth = 4 >> level;
	HRect maxRoi(x - searchWidth, y - searchWidth, (2 * searchWidth + 1), (2 * searchWidth + 1));
	maxRoi = maxRoi.Intersect(frameRoi);

	for(int i = maxRoi.GetTop(); i < maxRoi.GetBottom(); i++)
	{
		for(int j = maxRoi.GetLeft(); j < maxRoi.GetRight(); j++)
		{
			int value = frame->At(i, j);
			if(value > maxValue)
			{
				maxValue = value;
				ny = i;
				nx = j;
			}
		}
	}

	// computing the mean value
	double meanValue = 0;
	double var = 0;
	int count = 0;

	HRect searchRoi( nx - radius, ny - radius, radius * 2 + 1, radius * 2 + 1 );
	searchRoi = searchRoi.Intersect(frameRoi);

	for(int i = searchRoi.GetTop(); i < searchRoi.GetBottom(); i++)
	{
		for(int j = searchRoi.GetLeft(); j < searchRoi.GetRight(); j++)
		{
			int value = frame->At(i, j);
			meanValue += value;
			var += value*value;
			count++;
		}
	}

	meanValue /= count;
	var = var/count - meanValue*meanValue;

	//maxValue  = (int) (( maxValue - meanValue) * 0.5);
	peakThreshold  = (int) (maxValue * 0.75);

	// Find peak threshold as center of two bins:


	// first order moments
	for(int i = searchRoi.GetTop(); i < searchRoi.GetBottom(); i++)
	{
		for(int j = searchRoi.GetLeft(); j < searchRoi.GetRight(); j++)
		{
			int value = frame->At(i, j);
			if(value > peakThreshold)
			{
				mx0 += j * value;
				my0 += i * value;
				tval += value;
			}
		}
	}

	tval = 1.0/tval;

	mx0 *= tval;
	my0 *= tval;

	// second order moments
	for(int i = searchRoi.GetTop(); i < searchRoi.GetBottom(); i++)
	{
		for(int j = searchRoi.GetLeft(); j < searchRoi.GetRight(); j++)
		{
			int value = frame->At(i, j);
			if(value > peakThreshold)
			{
				mx2 += (j - mx0)*(j - mx0) * value;
				my2 += (i - my0)*(i - my0) * value;
				mxy += (j - mx0)*(i - my0) * value;
			}
		}
	}

	mx2 *= (tval);
	my2 *= (tval);
	mxy *= (tval);


	// Computer eigenvalues (normalized for level)
	// Added: David Hirvonen 4/18
	double a11 = mx2 * (1 << level);
	double a12 = mxy * (1 << level);
	double a21 = mxy * (1 << level);
	double a22 = my2 * (1 << level);
	double term1 = (a11 + a22);
	double term2 = sqrt(4 * a12 * a21 + ((a11 - a22) * (a11 - a22)));
	mts.lambda1 = term1 + term2;
	mts.lambda2 = term1 - term2;

// mx and my are at full res, while i1 and i2 are the set levels

	mts.mx = mx0 * (1 << level);
	mts.my = my0 * (1 << level);

	mts.i1 = mx2 + my2;
	mts.i2 = (mx2 - my2)*(mx2 - my2) + 4*mxy*mxy;
	mts.angle = atan2(2*mxy, (mx2 - my2));

	mts.mean = meanValue;
	mts.var = var;

	//printf("(%d,\t%d)\t max %d\t%f\t%f\n", ny, nx, maxValue, mts.i1, mts.i2);

	return mts;
}


bool EyeDetectorServer::AddDetection(int x1, int y1, int x2, int y2, float confidence, EyeCenterPointList *list)
{
	bool status = false;
	int xcenter = (x1 + x2) / 2;
	int ycenter = (y1 + y2) / 2;
	if (ALWAYS_ON( (m_pEyeCenterMask->At(ycenter, xcenter) == 0)) )
	{ /* No eye found here before */

		//float confidence =  (iavg==0) ? 300 : (float)(imax-imin) / (float)(iavg);
		//float confidence2 = (iavg2==0) ? 300 : (float)(imax2-imin2) / (float)(iavg2);

		status = true;

		CEyeCenterPoint eyeCenterPoint(xcenter, ycenter, (float) ycenter);
		eyeCenterPoint.m_nLeftSpecularityX = x1;
		eyeCenterPoint.m_nLeftSpecularityY = y1;
		eyeCenterPoint.m_nRightSpecularityX = x2;
		eyeCenterPoint.m_nRightSpecularityY = y2;

		eyeCenterPoint.m_nLeftSpecularityXL2 = x1;
		eyeCenterPoint.m_nLeftSpecularityYL2 = y1;
		eyeCenterPoint.m_nRightSpecularityXL2 = x2;
		eyeCenterPoint.m_nRightSpecularityYL2 = y2;

		eyeCenterPoint.m_fConfidence = confidence;
		eyeCenterPoint.m_index = m_Index;	//saves the frame number to which the center point belongs

		eyeCenterPoint.SetIsSpecularityEye( true );

		list->push_back(eyeCenterPoint);

		//printf("separation: %d (%d %d %d) (%d %d %d)\n", x2 - x, dx1, dyn1, dys1, dx2, dyn2, dys2);
		//DebugOutput("Eye found with xcenter, ycenter, confidence : %d, %d, %f\n", xcenter, ycenter, (float)ycenter);
		//printf("eye: (%d, %d)  (%d, %d)\n", x1,y1,x2,y2);

		for (int yyh=ycenter-m_MaskRadius;yyh<=ycenter+m_MaskRadius;yyh++)
		{
			for (int xxh=xcenter-m_MaskRadius;xxh<=xcenter+m_MaskRadius;xxh++)
			{
				if (xxh>=0 && xxh< m_ImageWidth && yyh>=0 && yyh<m_ImageHeight)
				{
					m_pEyeCenterMask->At(yyh, xxh) = 1;
				}
			}
		}
	}

	return status;
}

bool EyeDetectorServer::AddDetection_Simple(int x1, int y1, EyeCenterPointList *list)
{
	CEyeCenterPoint eyeCenterPoint(x1, y1, (float) y1);
	eyeCenterPoint.m_nLeftSpecularityX = x1;
	eyeCenterPoint.m_nLeftSpecularityY = y1;
	eyeCenterPoint.m_nRightSpecularityX = x1;
	eyeCenterPoint.m_nRightSpecularityY = y1;

	eyeCenterPoint.m_nLeftSpecularityXL2 = x1;
	eyeCenterPoint.m_nLeftSpecularityYL2 = y1;
	eyeCenterPoint.m_nRightSpecularityXL2 = x1;
	eyeCenterPoint.m_nRightSpecularityYL2 = y1;

	eyeCenterPoint.m_fConfidence = 1.0;
	eyeCenterPoint.m_index = m_Index;	//saves the frame number to which the center point belongs

	eyeCenterPoint.SetIsSpecularityEye( true );

	list->push_back(eyeCenterPoint);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool EyeDetectorServer::IsFrameAnEyeSingleSpec(Image8u* frame, EyeCenterPointList *list, bool resetMask, bool doAvgThreshold, int halfWidth, int threshold)
{
	if(doAvgThreshold)
	{  // This is tuned for image where iris diameter is greather than 10 pixels
		int border = halfWidth;

		IppiSize maskSize;
		maskSize.height = 2 * border + 1;
		maskSize.width = 2 * border + 1;

		IppiPoint anchor;
		anchor.x = border;
		anchor.y = border;

		IppiSize dstSize;
		dstSize.width = m_pBlur->GetWidth()-(2*border);
		dstSize.height = m_pBlur->GetHeight()-(2*border);

		m_pBlur->Fill(0);

		cvSmooth(frame->GetData(), m_pBlur->GetData(), CV_BLUR, maskSize.width, maskSize.height);

		// ippiFilterBox_8u_C1R( &frame->At(border, border), frame->GetStride(), &m_pBlur->At(border, border), m_pBlur->GetStride(), dstSize, maskSize, anchor );

		//WritePGM5("c:/temp/blur.pgm", frame->GetData(), frame->GetWidth(), frame->GetHeight(), frame->GetStride());
	}

	if(resetMask)
	{
		m_pEyeCenterMask->Fill(0);
	}

	IplImage* image = frame->GetData();

	SingleSpectDetect(image, list,doAvgThreshold,m_SpecularityMagnitude,false);

	//if any center points were found, then we have found an eye
	if (list->size() > 0)
		return true;
	else
		return false;
}

void logResults(EyeCenterPointList *list, bool igNored, std::string logfile){
	FILE *logFile=fopen(logfile.c_str(),"a+");
	fprintf(logFile,"\n");
	for(unsigned int i=0;i<list->size();i++){
		CEyeCenterPoint& eye=list->at(i);
		fprintf(logFile,"%c(%d,%d) ",eye.GetIsHaarEye()?'H':'S', eye.m_nCenterPointX, eye.m_nCenterPointY);
	}
	if(igNored){
		fprintf(logFile,"Too many detections: ignored");
	}
	fprintf(logFile,"\n");
	fclose(logFile);
}

#ifdef __linux__
extern "C" {
	#include "spec_private.h"
};
int cntr=0;
void EyeDetectorServer::SingleSpectDetect(IplImage* image, EyeCenterPointList *list, bool doAvgThreshold, int threshold, bool simpleAdd){

	int rowStride=image->widthStep;
	int params[] ={ image->width,
					image->height- m_SearchY,
					rowStride,
					(m_SpecularityMagnitude<<16)|(m_SpecularityMagnitude),
					MAX_SPEC_POINTS,m_SpecularitySize};
	unsigned char *ptr=(unsigned char *)(image->imageData + m_SpecularitySize +m_SpecularitySize*image->widthStep);
	int nPoints=detect_single_specularity(ptr, (twoshorts *)m_specPoints, params);

	bool bIgnored=nPoints<0;

#ifdef MADHAV
	char Buff[1024];
	static int ctr =0;
	sprintf(Buff,"SingleSpectDetect%d.pgm",ctr++);
	cvSaveImage(Buff,image);
	printf("Number of Spec Points %d %d \n",m_SpecularitySize,nPoints);
	for(int i=0;i<nPoints;i++){
		twoshorts *ts= (twoshorts *)(&m_specPoints[i]);
		int x=ts->hi;
		int y=ts->lo;
		printf(" %d  [%d %d]\n",i,x,y);
	}
#endif

	for(int i=0; i<nPoints;i++){
		twoshorts *ts= (twoshorts *)(&m_specPoints[i]);
		int x=ts->hi;
		int y=ts->lo;
		int iavg=0, icnt=0, imax=-1, imin=999;
		int val1=0,bright1= GET_IMAGE_PIXEL(image,x +y*image->widthStep);

		//loop over left and right
		int yPtr = y * rowStride;
		int yPtrx = yPtr + x + m_BoxX;
		int yPtrx2 = yPtr + x - m_BoxX;
		for(int yt = -m_BoxY * rowStride; yt <= m_BoxY * rowStride; yt = yt + rowStride)
		{
			val1 = GET_IMAGE_PIXEL(image, yPtrx + yt);
			iavg += val1;
			icnt += 1;
			if (val1 < imin) { imin = val1; }
			if (val1 > imax) { imax = val1; }

			val1 = GET_IMAGE_PIXEL(image, yPtrx2 + yt);
			iavg += val1;
			icnt += 1;
			if (val1 < imin) { imin = val1; }
			if (val1 > imax) { imax = val1; }
		}

		yPtrx = yPtr + x - m_BoxY * rowStride;
		yPtrx2 = yPtrx + 2 * m_BoxY * rowStride;

		for (int xt = -m_BoxX; xt <= m_BoxX; xt++)
		{
			val1 = GET_IMAGE_PIXEL(image, yPtrx + xt);
			iavg += val1;
			icnt += 1;
			if (val1 < imin) { imin = val1; }
			if (val1 > imax) { imax = val1; }

			val1 = GET_IMAGE_PIXEL(image, yPtrx2 + xt);
			iavg += val1;
			icnt += 1;
			if (val1 < imin) { imin = val1; }
			if (val1 > imax) { imax = val1; }
		}

		/* Now let's check mean and variance */
		iavg = iavg / icnt;
		if ( ALWAYS_OFF((bright1-imin) > (bright1-iavg) * m_VarianceThresholdMin) ) { /* imin check */
			//printf("va(%d,%d) ",x,y);
			continue;
		}
		if ( ALWAYS_OFF((bright1-imax) < (bright1-iavg) * m_VarianceThresholdMax) ) { /* imax check */
			continue;
		}

		if(simpleAdd)  AddDetection_Simple(x,y,list);
		else AddDetection(x,y,x,y,1.0,list);
	}
	if(m_shouldLog) logResults(list,bIgnored,m_LogFile);
}

//// BFin impl: first find single specularities and then look for pairing them
bool EyeDetectorServer::IsFrameAnEye(Image8u* frame, EyeCenterPointList *list) {

	EyeCenterPointList sList;

	m_pEyeCenterMask->Fill(0);
	IplImage* image = frame->GetData();
	SingleSpectDetect(image, &sList,false,m_SpecularityMagnitude,true);

	if (sList.size()==0) return false;

	bool status=false;
	int min_xDist=max(0,m_Separation-m_SearchX);	// dont search to the left
	int max_xDist=m_Separation+m_SearchX;
	int min_yDist=-m_SearchY;
	int max_yDist=m_SearchY;
	EyeCenterPointListIterator it1,it2;
	for(it1 = sList.begin(); it1 != sList.end(); it1++){
		CEyeCenterPoint &left_eye = (*it1);

		bool removeLeftEye=false;
		//start from beginning and look for any point which meets the criterion
		for (it2 = sList.begin(); it2 != sList.end(); it2++) {
			CEyeCenterPoint &right_eye = (*it2);
			int ydist=right_eye.m_nCenterPointY-left_eye.m_nCenterPointY;

			if(ydist>max_yDist) {
				// already too far below
				//left_eye has no taker should be removed from list to save iterations
				removeLeftEye=true;
				break;
			}

			int xdist=right_eye.m_nCenterPointX-left_eye.m_nCenterPointX;

			if(ydist<min_yDist|| xdist<min_xDist || xdist>max_xDist) continue;	//look for more points

			//So we have a match: add a point of double spec into list
			status=true;
			int x1=left_eye.m_nCenterPointX;
			int y1=left_eye.m_nCenterPointY;
			int x2=right_eye.m_nCenterPointX;
			int y2=right_eye.m_nCenterPointY;
			AddDetection(x1,y1,x2,y2,1.0,list);

			// no need to continue searching we should remove the pair from sList
			//it2=sList.erase(it2);
			removeLeftEye=true;
			break;
		}

		//if(removeLeftEye) it1=sList.erase(it1);
		//else it1++;
	}
	sList.empty();
	return status;
}


#else
void EyeDetectorServer::SingleSpectDetect(IplImage* image, EyeCenterPointList *list, bool doAvgThreshold, int threshold, bool SimpleAdd)
{
	int xMin = max(m_BoxX,m_SpecularitySize);
	int xMax = image->width - m_Separation - m_SearchX - 1;
	int yMin = max(m_BoxY,m_SpecularitySize)+m_SearchY;
	int yMax = image->height - m_SearchY - max(m_BoxY, m_SpecularitySize) - 1;

	long yPtr, yPtrx, yPtrx2;
	int bright1;
	int iavg, icnt, imax, imin;
	int x,y;
	int val1;
	int dummy;
	int dyn1=0,dys1=0;

	int rowStride = image->widthStep;

	for (y = yMin; y < yMax; y = y + m_StepSize)
	{
		//printf("row: %d\n", y);

		yPtr = y * rowStride;

		for (x = xMin; x< xMax; x = x + m_StepSize	)
		{
			if(m_pEyeCenterMask->At(y, x))
			{
				continue;
			}

			yPtrx = yPtr + x;
			bright1	= GET_IMAGE_PIXEL(image,yPtrx);

			if(doAvgThreshold)
			{
				Ipp8u &value = m_pBlur->At(y, x);
				if ( ALWAYS_OFF( value < threshold ) )
				{
					goto GETOUT;
				}
			}

			int dx1;

			//check whether we have a bright point, with a darker point to its RIGHT
			dx1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx + m_SpecularitySize));
			if (ALWAYS_OFF(dx1 < m_SpecularityMagnitude) )
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point to its LEFT
			dx1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx - m_SpecularitySize));
			if (ALWAYS_OFF(dx1 < m_SpecularityMagnitude) )
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point above
			dyn1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx - rowStride * m_SpecularitySize));
			if (ALWAYS_OFF(dyn1 < m_SpecularityMagnitude) )
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point below
			dys1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx + rowStride * m_SpecularitySize));
			if (ALWAYS_OFF(dys1 < m_SpecularityMagnitude)) // CRASH HERE
			{
				goto GETOUT;
			}

			//we now have a single bright point � let�s check that in a region around the specularity
			//there is not even one point in the cluster that fails the brightness test
			iavg = 0;
			icnt = 0;
			imin = 999;
			imax = -1;

			yPtrx = yPtr + x + m_BoxX;
			yPtrx2 = yPtr + x - m_BoxX;

			/* Left specularity - looking for space to the left and right vertically */
			for (int yt = -m_BoxY * rowStride; yt <= m_BoxY * rowStride; yt = yt + rowStride)
			{
				val1 = GET_IMAGE_PIXEL(image, yPtrx + yt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }

				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

				val1 = GET_IMAGE_PIXEL(image, yPtrx2 + yt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }

				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

			}

			/* Left specularity - looking for space to the top and bottom horizontally */
			yPtrx = yPtr + x - m_BoxY * rowStride;
			yPtrx2 = yPtrx + 2 * m_BoxY * rowStride;

			for (int xt = -m_BoxX; xt <= m_BoxX; xt++)
			{
				val1 = GET_IMAGE_PIXEL(image, yPtrx + xt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }
				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

				val1 = GET_IMAGE_PIXEL(image, yPtrx2 + xt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }
				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}
			}

			/* Now let's check mean and variance */
			iavg = iavg / icnt;

			if ( ALWAYS_OFF((bright1-imin) > (bright1-iavg) * m_VarianceThresholdMin) ) { /* imin check */
				goto GETOUT;
			}

			if ( ALWAYS_OFF((bright1-imax) < (bright1-iavg) * m_VarianceThresholdMax) ) { /* imax check */
				goto GETOUT;
			}


			/* OK - we now think we have a good point on the left . Now let's search for a point om the right */
			{
				//((bright1-imin) > (bright1-iavg) * 1.5)
				//((bright1-imax) < (bright1-iavg) * 0.666)
				float conf1a = ((bright1-iavg) != 0) ? (float)(bright1-imin) / (bright1-iavg) : 1000.f; // 1 is good (less than 1.5 okay)
				float conf1b = ((bright1-imax) != 0) ? (float)(bright1-iavg) / (bright1-imax) : 1000.f; // 1 is good (less than 1.5 okay)

				float confidence = max(conf1a, conf1b);

				AddDetection(x, y, x, y, confidence, list);
			}

GETOUT:

			dummy = 1;

		}  /* For y */
	}  /* For x */
}

bool EyeDetectorServer::IsFrameAnEye(Image8u* frame, EyeCenterPointList *list)
{
	IplImage* image = frame->GetData();

	int eyecnt;
	int xMin = max(m_BoxX,m_SpecularitySize);
	int xMax = frame->GetWidth() - m_Separation - m_SearchX - 1;
	int yMin = max(m_BoxY,m_SpecularitySize)+m_SearchY;
	int yMax = frame->GetHeight() - m_SearchY - max(m_BoxY, m_SpecularitySize) - 1;

	long yPtr, yPtr2, yPtrx, yPtrx2;
	int bright1, bright2;
	int iavg, icnt, imax, imin;
	int iavg2, icnt2, imax2, imin2;
	int x,y,x2,y2;
	int val1, val2;
	int dummy;

	eyecnt = 0;

	int rowStride = frame->GetStride();
	int dyn1=0,dys1=0,dyn2=0,dys2=0;
	float conf1a=0.0f,conf1b=0.0f, conf2a=0.0f, conf2b=0.0f, confidence=0.0f;
	m_pEyeCenterMask->Fill(0);

	//SaveIPLImage(frame, "junk.pgm");

#if USE_MMX
	__int64 threshold = 0;
	for(int i = 0; i < 8; i++)
	{
		((unsigned char *)(&threshold))[i] = m_SpecularityMagnitude;
	}
	int blocks = (xMax - xMin) / 8;
	unsigned char *diff = new unsigned char[imageWidth * imageHeight];
	//printf("blocks: %d\n", blocks);

#if DO_DY_OUTSIDE
	for (int y = yMin; y < yMax; y = y + m_StepSize)
	{
		yPtr = y * rowStride;
		RowDiffLT(&image[yPtr+xMin], &image[yPtr+xMin+m_SpecularitySize], blocks, &diff[yPtr+xMin], threshold);
		unsigned char *pDiff = &diff[yPtr+xMin];
	}
	_asm { emms }
#endif
#endif

	for (y = yMin; y < yMax; y = y + m_StepSize)
	{
		//printf("row: %d\n", y);

		yPtr = y * rowStride;

#if USE_MMX
#if !DO_DY_OUTSIDE
		RowDiffLT(&image[yPtr+xMin], &image[yPtr+xMin+m_SpecularitySize], blocks, &diff[yPtr+xMin], threshold);
		int end = blocks * 8;
		for(int i = end; i < xMax; i++)
		{
			int a = image[yPtr+i];
			int b = image[yPtr+i+m_SpecularitySize];
			int dx = a - b;
			diff[yPtr + i] = dx < m_SpecularityMagnitude ? 1 : 0;
		}
		_asm { emms }
#endif
		unsigned char *pDiff = &diff[yPtr+xMin];
#endif

		for (x = xMin; x< xMax; x = x + m_StepSize
#if USE_MMX
			, pDiff += m_StepSize
#endif
			)
		{
			yPtrx = yPtr + x;
			bright1	= GET_IMAGE_PIXEL(image,yPtrx);

			//check whether we have a bright point, with a darker point to its RIGHT
			int dx1;

#if USE_MMX
			dx1 = *pDiff;
			if(dx1)
#else
			dx1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx + m_SpecularitySize)); // Purify: UMR
			if (ALWAYS_OFF(dx1 < m_SpecularityMagnitude) )
#endif
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point to its LEFT
			dx1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx - m_SpecularitySize));
			if (ALWAYS_OFF(dx1 < m_SpecularityMagnitude) )
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point above
			dyn1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx - rowStride * m_SpecularitySize)); // Purify: UMR
			if (ALWAYS_OFF(dyn1 < m_SpecularityMagnitude) )
			{
				goto GETOUT;
			}

			//check whether we have a bright point, with a darker point below
			dys1 = (bright1 - GET_IMAGE_PIXEL(image, yPtrx + rowStride * m_SpecularitySize));
			if (ALWAYS_OFF(dys1 < m_SpecularityMagnitude)) // CRASH HERE
			{
				goto GETOUT;
			}

			// KH: Vertical dy threshold

			//we now have a single bright point � let�s check that in a region around the specularity
			//there is not even one point in the cluster that fails the brightness test
			iavg = 0;
			icnt = 0;
			imin = 999;
			imax = -1;

			yPtrx = yPtr + x + m_BoxX;
			yPtrx2 = yPtr + x - m_BoxX;

			/* Left specularity - looking for space to the left and right vertically */
			for (int yt = -m_BoxY * rowStride; yt <= m_BoxY * rowStride; yt = yt + rowStride)
			{
				val1 = GET_IMAGE_PIXEL(image, yPtrx + yt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }

				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

				val1 = GET_IMAGE_PIXEL(image, yPtrx2 + yt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }

				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

			}

			/* Left specularity - looking for space to the top and bottom horizontally */
			yPtrx = yPtr + x - m_BoxY * rowStride;
			yPtrx2 = yPtrx + 2 * m_BoxY * rowStride;

			for (int xt = -m_BoxX; xt <= m_BoxX; xt++)
			{
				val1 = GET_IMAGE_PIXEL(image, yPtrx + xt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }
				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}

				val1 = GET_IMAGE_PIXEL(image, yPtrx2 + xt);
				iavg += val1;
				icnt += 1;
				if (val1 < imin) { imin = val1; }
				if (val1 > imax) { imax = val1; }
				if (ALWAYS_OFF((bright1 - val1) < m_SpecularityMagnitude) )
				{
					goto GETOUT;
				}
			}

			// KH: horizontal scanline delta threshold test

			/* Now let's check mean and variance */
			iavg = iavg / icnt;

			if ( ALWAYS_OFF((bright1-imin) > (bright1-iavg) * m_VarianceThresholdMin) ) { /* imin check */
				goto GETOUT;
			}

			if ( ALWAYS_OFF((bright1-imax) < (bright1-iavg) * m_VarianceThresholdMax) ) { /* imax check */
				goto GETOUT;
			}

			// KH: variance tests for point 1

#if DO_SINGLE_SPECULARITY
			/* OK - we now think we have a good point on the left . Now let's search for a point om the right */
			{
				AddDetection(x, y, x, y, list);
				goto GETOUT;
			}
#endif

			//	for (y2=y/*-1m_SearchY*/; y2<=y/*+m_SearchY*/; y2++) {
			for (y2=y-m_SearchY; y2<=y+m_SearchY; y2=y2+m_StepSize) {
				yPtr2 = y2 * rowStride; // DJH: was yPtr = ... which was screwing up code with dependencies above

				// for (x2=x+m_Separation/*-m_SearchX*/; x2<=x+m_Separation/*+m_SearchX*/; x2++) {
				for (x2=x+m_Separation-m_SearchX; x2<=x+m_Separation+m_SearchX; x2=x2+m_StepSize) {

					yPtrx = yPtr2 + x2;

					bright2 = GET_IMAGE_PIXEL(image, yPtrx);

					//check whether we have a bright point, with a darker point to its RIGHT
					int dx2 = (bright2 - GET_IMAGE_PIXEL(image, yPtrx + m_SpecularitySize)); // Purify: UMR
					if (ALWAYS_OFF(dx2 < m_SpecularityMagnitude) )
					{
						goto GETOUT2;
					}

					//check whether we have a bright point, with a darker point to its LEFT
					dx2 = (bright2 - GET_IMAGE_PIXEL(image, yPtrx - m_SpecularitySize)); // Purify: UMR
					if (ALWAYS_OFF(dx2 < m_SpecularityMagnitude) )
					{
						goto GETOUT2;
					}

					//check whether we have a bright point, with a darker point above
					dyn2 = (bright2 - GET_IMAGE_PIXEL(image, yPtrx - rowStride * m_SpecularitySize)); // Purify: UMR
					if (ALWAYS_OFF(dyn2 < m_SpecularityMagnitude) )
					{
						goto GETOUT2;
					}

					//check whether we have a bright point, with a darker point below
					dys2 = (bright2 - GET_IMAGE_PIXEL(image, yPtrx + rowStride * m_SpecularitySize));
					if (ALWAYS_OFF(dys2 < m_SpecularityMagnitude) ) // CRASH HERE
					{
						goto GETOUT2;
					}

					// KH: vertical dy threshold test (2)

					//we now have a single bright point � let�s check that in a region around the specularity
					//there is not even one point in the cluster that fails the brightness test

					iavg2 = 0;
					icnt2 = 0;
					imin2 = 999;
					imax2 = -1;

					yPtrx = yPtr2 + x2 + m_BoxX;
					yPtrx2 = yPtr2 + x2 - m_BoxX;

					/* Left specularity - looking for space to the LEFT and RIGHT vertically */
					for (int yt = -m_BoxY*rowStride; yt <= m_BoxY*rowStride; yt=yt+rowStride)
					{
						//	//	image2[yPtrx + yt] = 255;
						val2 = GET_IMAGE_PIXEL(image, yPtrx + yt);
						iavg2 += val2;
						icnt2 += 1;
						if (val2 < imin2) { imin2 = val2; }
						if (val2 > imax2) { imax2 = val2; }

						if (ALWAYS_OFF(((bright2 - val2) < m_SpecularityMagnitude)) )
						{
							goto GETOUT2;
						}

						val2 = GET_IMAGE_PIXEL(image, yPtrx2 + yt);
						iavg2 += val2;
						icnt2 += 1;
						if (val2 < imin2) { imin2 = val2; }
						if (val2 > imax2) { imax2 = val2; }

						if (ALWAYS_OFF(((bright2 - val2) < m_SpecularityMagnitude)) )
						{
							goto GETOUT2;
						}

					}

					// KH: Dx threshold test (2)

					/* Left specularity - looking for space to the top and bottom horizontally */
					yPtrx = yPtr2 + x2 - m_BoxY * rowStride;
					yPtrx2 = yPtrx + 2 * m_BoxY * rowStride;

					for (int xt = -m_BoxX; xt <= m_BoxX; xt++)
					{
						val2 = GET_IMAGE_PIXEL(image, yPtrx + xt);
						iavg2 += val2;
						icnt2 += 1;
						if (val2 < imin2) { imin2 = val2; }
						if (val2 > imax2) { imax2 = val2; }
						if (ALWAYS_OFF((bright2 - val2) < m_SpecularityMagnitude) )
						{
							goto GETOUT2;
						}

						val2 = GET_IMAGE_PIXEL(image, yPtrx2 + xt);
						iavg2 += val2;
						icnt2 += 1;
						if (val2 < imin2) { imin2 = val2; }
						if (val2 > imax2) { imax2 = val2; }
						if (ALWAYS_OFF(((bright2 - val2) < m_SpecularityMagnitude)) )
						{
							goto GETOUT2;
						}

					}

					// KH: horizontal scanline delta threshold test (2)

					/* Now let's check mean and variance */
					iavg2 = iavg2 / icnt2;

					if ( ALWAYS_OFF(((bright2-imin2) > ((bright2-iavg2) * m_VarianceThresholdMin))) )
					{ /* imin check */
						goto GETOUT2;
					}

					//DebugOutput("Confidence value is %f, %f\n",confidence,confidence2);

					if ( ALWAYS_OFF(((bright2-imax2) < ((bright2-iavg2) * m_VarianceThresholdMax)))  )
					{ /* imax check */
						goto GETOUT2;
					}

					// KH: variance tests for point 2

					/* OK - Here we will insert spoke */

					//((bright1-imin) > (bright1-iavg) * 1.5)
					//((bright1-imax) < (bright1-iavg) * 0.666)
					conf1a = ((bright1-iavg) != 0) ? (float)(bright1-imin) / (bright1-iavg) : 1000.f; // 1 is good (less than 1.5 okay)
					conf1b = ((bright1-imax) != 0) ? (float)(bright1-iavg) / (bright1-imax) : 1000.f; // 1 is good (less than 1.5 okay)

					//((bright2-imin2) > (bright2-iavg2) * 1.5)
					//((bright2-imax2) < (bright2-iavg2) * 0.666)
					conf2a = ((bright2-iavg2) != 0) ? (float)(bright2-imin2) / (bright2-iavg2) : 1000.f; // 1 is good (less than 1.5 okay)
					conf2b = ((bright2-imax2) != 0) ? (float)(bright2-iavg2) / (bright2-imax2) : 1000.f; // 1 is good (less than 1.5 okay)

					confidence = max(max(max(conf1a, conf1b), conf2a), conf2b);

					if(AddDetection(x, y, x2, y2, confidence, list))
					{
						eyecnt++;
						goto GETOUT; // DJH
					}

GETOUT2:
					dummy = 1;

				}  /* For y2 */
			}  /* FOr x2 */
GETOUT:

			dummy = 1;


		}  /* For y */
	}  /* For x */

	// Sort the vector using predicate and std::sort
	//frame->m_pEyeCenterPoints->sort( EyeConfidenceSortPredicate );

	//printf("eye count: %d\n", eyecnt);

#if USE_MMX
	delete [] diff;
#endif

#if 1
	if (eyecnt>0) {
		//DebugOutput("Ending here - eyecnt is %d\n",eyecnt);
	}
#endif

	//if any center points were found, then we have found an eye
	if (list->size() > 0)
		return true;
	else
		return false;
}



#endif
