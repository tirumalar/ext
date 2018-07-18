#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h> 
#include "cvblob.h"
#include "blob_detect.h"

using namespace cvb;

DetectGlass::DetectGlass(){

}
DetectGlass::~DetectGlass(){

}

bool DetectGlass::detect_glass(IplImage* m_eyeCropIn, IplImage *oframe, double Ratio_check,double Ratio1_check,double edge_thresh,int scale)
{
	IplImage *m_eyeCrop =  cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,0);
	 cvCopy(m_eyeCropIn, m_eyeCrop);
	//cvResize(m_eyeCropIn, m_eyeCrop);
	double Ratio;
	IplImage *m_eyeCrop1;

	if(scale == 0)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(1),m_eyeCrop->height/(1)),IPL_DEPTH_8U,0);
	else if(scale == 1)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(2),m_eyeCrop->height/(2)),IPL_DEPTH_8U,0);
	else if(scale == 2)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(4),m_eyeCrop->height/(4)),IPL_DEPTH_8U,0);
	

	cvResize(m_eyeCrop,m_eyeCrop1);
	IplImage *labelImg=cvCreateImage(cvSize(m_eyeCrop1->width,m_eyeCrop1->height),IPL_DEPTH_LABEL,0);
	IplImage *threshImg=cvCreateImage(cvSize(m_eyeCrop1->width,m_eyeCrop1->height),IPL_DEPTH_8U,0);

	cvThreshold(m_eyeCrop1,threshImg,250,255,CV_THRESH_BINARY);
	cvSmooth(threshImg,threshImg,CV_MEDIAN,7,7);

			//cvShowImage("Smmoth", threshImg);
			//	cvWaitKey(5);
			//	getchar();

	CvBlobs blobs;
	unsigned int result=cvLabel(threshImg,labelImg,blobs);
	int scale1;
	if(scale==0)
		scale1=1;
	else if(scale==1)
		scale1=2;
	else if(scale==2)
		scale1=4;

	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		if((it->second->area > (100/(scale1*scale1))) && (it->second->area < (6000*2/(scale1*scale1))) && (it->second->miny>(70/scale1)) && (it->second->maxy<(410*2/scale1)) && (it->second->minx>(70/scale1)) && (it->second->maxx<(570*2/scale1)))
		{
			double Tx=it->second->centroid.x - it->second->minx;double Ty=it->second->centroid.y - it->second->miny;
			Ratio=10;
			if(Tx>Ty)
				Ratio=Tx/Ty;
			else
				Ratio=Ty/Tx;
			double Area = it->second->area;
			double Ratio1=(4*Tx*Ty)/Area;
			if(Ratio<Ratio_check && Ratio1<Ratio1_check)     
			{
				//Checing the edges of specularity 
				//Level 0
				CvRect cropRect = cvRect((it->second->minx*scale1)-10,(it->second->miny*scale1)-10, (it->second->maxx*scale1)-(it->second->minx*scale1)+20, (it->second->maxy*scale1)-(it->second->miny*scale1)+20); // ROI in source image
				if (cropRect.x < 1)
					cropRect.x = 1;
				if (cropRect.y < 1)
					cropRect.y = 1;

				
				cropRect.width = cropRect.x + cropRect.width > 638 ?  638 - cropRect.x : cropRect.width;
				cropRect.height = cropRect.y + cropRect.height > 478 ?  478 - cropRect.y : cropRect.height;


				    
				
				cvSetImageROI(m_eyeCrop, cropRect);
				//Level accourding to input
				//CvRect cropRect = cvRect((it->second->minx)-(10/scale),(it->second->miny)-(10/scale), (it->second->maxx)-(it->second->minx)+(20/scale), (it->second->maxy)-(it->second->miny)+(20/scale)); // ROI in source image
				//cvSetImageROI(m_eyeCrop1, cropRect);
				IplImage* cropImg=cvCreateImage(cvSize(cropRect.width,cropRect.height),IPL_DEPTH_8U,0); // Destination cropped image
				IplImage* cropImg1=cvCreateImage(cvSize(cropRect.width,cropRect.height),IPL_DEPTH_64F,0); // Destination cropped image
				IplImage* cropImg2=cvCreateImage(cvSize(cropRect.width,cropRect.height),IPL_DEPTH_64F,0); // Destination cropped image
				IplImage* cropImg3=cvCreateImage(cvSize(cropRect.width,cropRect.height),IPL_DEPTH_64F,0); // Destination cropped image
				printf("\nIn GlassDetect:%d %d %d %d, %d, %d, %d %.3f, %.3f\n", cropRect.x, cropRect.y , cropRect.width , cropRect.height, cropImg->width, cropImg->height, cropImg->depth, Ratio, Ratio1 );
				
				cvCopy(m_eyeCrop, cropImg, NULL); // Copies only crop region
				cvResetImageROI(m_eyeCrop);




				
				

				double scale1 = (double)1/255;
				cvConvertScale(cropImg,cropImg1,scale1,0);  

				float x_mask[]  = { 0.125,  0, -0.125,
					0.25,  0, -.25,
					0.125,  0, -0.125};

				float y_mask[]  = { 0.125,  0.25, 0.125,
					0,  0, 0,
					-0.125,  -0.25, -0.125};

				CvMat kernel_x=cvMat(3,3,CV_32FC1,x_mask);
				CvMat kernel_y=cvMat(3,3,CV_32FC1,y_mask);

				cvFilter2D(cropImg1, cropImg2, &kernel_x, cvPoint(-1,-1));
				cvFilter2D(cropImg1, cropImg3, &kernel_y, cvPoint(-1,-1));
				cvMul(cropImg2,cropImg2,cropImg2,1);
				cvMul(cropImg3,cropImg3,cropImg3,1);
				cvAdd(cropImg3,cropImg2,cropImg1);

				double cutoff = edge_thresh*edge_thresh;

				for(int i=0;i<cropImg1->height;i++)
				{
					for(int j=0;j<cropImg1->width;j++)
					{
						if(CV_IMAGE_ELEM(cropImg1, double, i, j)>=cutoff)
							cropImg->imageData[i*cropImg->widthStep+j] = 255;
						else
							cropImg->imageData[i*cropImg->widthStep+j] = 0;
					}
				}

				//Checking four sides for presence of edges

				//First Horizontal
				int edge_count_side=0,edge_count=0;
				for(int i=0;i<(cropImg->width)/2;i++)
				{
					for(int j=(int)((cropImg->height)/2)-1;j<(int)((cropImg->height)/2)+2;j++)
					{
						if(CV_IMAGE_ELEM(cropImg, unsigned char,j,i)==255)
							edge_count_side++;
					}
				}
				if(edge_count_side>0)
					edge_count++;
				edge_count_side=0;

				for(int i=(cropImg->width)/2;i<(cropImg->width);i++)
				{
					for(int j=(int)((cropImg->height)/2)-1;j<(int)((cropImg->height)/2)+2;j++)
					{
						if(CV_IMAGE_ELEM(cropImg, unsigned char,j,i)==255)
							edge_count_side++;
					}
				}
				if(edge_count_side>0)
					edge_count++;
				edge_count_side=0;

				//Vertical
				edge_count_side=0;
				for(int j=0;j<(cropImg->height)/2;j++)
				{
					for(int i=(int)((cropImg->width)/2)-1;i<(int)((cropImg->width)/2)+2;i++)
					{
						if(CV_IMAGE_ELEM(cropImg, unsigned char,j,i)==255)
							edge_count_side++;
					}
				}
				if(edge_count_side>0)
					edge_count++;
				edge_count_side=0;

				for(int j=(cropImg->height)/2;j<(cropImg->height);j++)
				{
					for(int i=(int)((cropImg->width)/2)-1;i<(int)((cropImg->width)/2)+2;i++)
					{
						if(CV_IMAGE_ELEM(cropImg, unsigned char,j,i)==255)
							edge_count_side++;
					}
				}
				if(edge_count_side>0)
					edge_count++;
				edge_count_side=0;


				if(edge_count> 0)
				{
					/*cvShowImage("Crop",cropImg);
					cvWaitKey(1);*/
					cvReleaseImage(&m_eyeCrop);
					cvReleaseImage(&cropImg);
					cvReleaseImage(&cropImg1);
					cvReleaseImage(&cropImg2);
					cvReleaseImage(&cropImg3);
					cvReleaseImage(&labelImg);
					cvReleaseImage(&threshImg);
					cvReleaseImage(&m_eyeCrop1);
					cvReleaseBlobs(blobs);
					return true;

				}
				//cvSaveImage("crop.pgm",cropImg);
				/*cvShowImage("Crop",cropImg);
				cvWaitKey(1);*/
				cvReleaseImage(&cropImg);
				cvReleaseImage(&cropImg1);
				cvReleaseImage(&cropImg2);
				cvReleaseImage(&cropImg3);
			}
		}
	}

	cvReleaseImage(&m_eyeCrop);
	cvReleaseBlobs(blobs);
	cvReleaseImage(&labelImg);
	cvReleaseImage(&threshImg);
	cvReleaseImage(&m_eyeCrop1);
	return false;
}

bool DetectGlass::detect_glass_window(int decision)
{

	if (decision)
		G[0] = 1;
	
	G<<=1;
	if (G.count() >= 1)
		return true;
	else return  false;
}

CvRect rect_intersect(CvRect a, CvRect b) 
{ 
    CvRect r; 
    r.x = (a.x > b.x) ? a.x : b.x;
    r.y = (a.y > b.y) ? a.y : b.y;
    r.width = (a.x + a.width < b.x + b.width) ? 
        a.x + a.width - r.x : b.x + b.width - r.x; 
    r.height = (a.y + a.height < b.y + b.height) ? 
        a.y + a.height - r.y : b.y + b.height - r.y; 
    if(r.width <= 0 || r.height <= 0) 
        r = cvRect(0, 0, 0, 0); 

    return r; 
}

bool DetectGlass::detect_glass_Effect(IplImage* m_eyeCropIn, IplImage *oframe, double Ratio_check,double Ratio1_check,double edge_thresh,int scale, int maxIrisRadius)
{
	maxIrisRadius = maxIrisRadius/(scale*2);
	IplImage *m_eyeCrop =  cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,0);
	cvCopy(m_eyeCropIn, m_eyeCrop);
	//cvResize(m_eyeCropIn, m_eyeCrop);
	double Ratio;
	IplImage *m_eyeCrop1;

	if(scale == 0)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(1),m_eyeCrop->height/(1)),IPL_DEPTH_8U,0);
	else if(scale == 1)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(2),m_eyeCrop->height/(2)),IPL_DEPTH_8U,0);
	else if(scale == 2)
		m_eyeCrop1=cvCreateImage(cvSize(m_eyeCrop->width/(4),m_eyeCrop->height/(4)),IPL_DEPTH_8U,0);
	

	cvResize(m_eyeCrop,m_eyeCrop1);
	//IplImage *labelImg=cvCreateImage(cvSize(m_eyeCrop1->width,m_eyeCrop1->height),IPL_DEPTH_LABEL,0);
	//IplImage *threshImg=cvCreateImage(cvSize(m_eyeCrop1->width,m_eyeCrop1->height),IPL_DEPTH_8U,0);
	int padWidth = 12, padHeight = 12;
    CvPoint offset;
	offset.x = padWidth;
	offset.y = padHeight;

	
	IplImage *paddedImg=cvCreateImage(cvSize(m_eyeCrop1->width + 2*padWidth,m_eyeCrop1->height + 2*padHeight),IPL_DEPTH_8U,0);
	IplImage *labelImg=cvCreateImage(cvSize(paddedImg->width,paddedImg->height),IPL_DEPTH_LABEL,0);
	cvCopyMakeBorder(m_eyeCrop1, paddedImg, offset, 0, cvScalarAll(0));



	IplImage *threshImg=cvCreateImage(cvSize(paddedImg->width,paddedImg->height),IPL_DEPTH_8U,0);
	
	cvThreshold(paddedImg,threshImg,250,255,CV_THRESH_BINARY);


	
	cvErode(threshImg, threshImg, 0, 4);//erosion
	//cvSmooth(threshImg,threshImg,CV_MEDIAN,7,7);
	cvShowImage("Erosion", threshImg);
	cvWaitKey(5);
	//getchar();

	
	

	//cvThreshold(m_eyeCrop1,threshImg,250,255,CV_THRESH_BINARY);
	//cvSmooth(threshImg,threshImg,CV_MEDIAN,7,7);

			//cvShowImage("Smmoth", threshImg);
			//	cvWaitKey(5);
			//	getchar();

	CvBlobs blobs;
	unsigned int result=cvLabel(threshImg,labelImg,blobs);
	int scale1;
	if(scale==0)
		scale1=1;
	else if(scale==1)
		scale1=2;
	else if(scale==2)
		scale1=4;

	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		if (it->second->area > 10)	
		{
		//Checing the edges of specularity 
				//Level 0
				int X,Y, Width, Height;
				X = (it->second->minx)-5;
				Y = (it->second->miny)-5;
				Width = (it->second->maxx)-(it->second->minx)+10;
				Height = (it->second->maxy)-(it->second->miny)+10;

				//printf("rectBlobImpact: %d, %d, %d, %d, area = %d\n", X, Y, Width, Height, (it->second->area));

				CvRect cropRect = cvRect(X,Y, Width, Height); // ROI in source image
				if (cropRect.x < 1)
					cropRect.x = 1;
				if (cropRect.y < 1)
					cropRect.y = 1;

				
				cropRect.width = cropRect.x + cropRect.width > paddedImg->width-1 ?   paddedImg->width-1 - cropRect.x : cropRect.width;
				cropRect.height = cropRect.y + cropRect.height > paddedImg->height-1 ?  paddedImg->height-1 - cropRect.y : cropRect.height;


				    
				
				//cvSetImageROI(m_eyeCrop, cropRect);
				cvSetImageROI(paddedImg, cropRect);
				

				IplImage* cropImg=cvCreateImage(cvSize(cropRect.width,cropRect.height),IPL_DEPTH_8U,0); // Destination cropped image
				//cvCopy(m_eyeCrop, cropImg, NULL); // Copies only crop region
				cvCopy(paddedImg, cropImg, NULL); // Copies only crop region
				cvResetImageROI(paddedImg);

				
				int  eyeX= paddedImg->width/2 - maxIrisRadius;
				int eyeY = paddedImg->height/2 - maxIrisRadius;
				int eyeWidth = 2*maxIrisRadius;
				int eyeHeight = 2*maxIrisRadius;

				CvRect eyeRegion = cvRect(eyeX, eyeY, eyeWidth, eyeHeight);
				CvRect inter_Section = rect_intersect(cropRect, eyeRegion);
				int area = inter_Section.height*inter_Section.width;

				cvRectangle(paddedImg, cvPoint(eyeX, eyeY), cvPoint(eyeX+ eyeWidth, eyeY + eyeHeight), cvScalar(127,127,127),1);
				cvRectangle(paddedImg, cvPoint(X, Y), cvPoint(X+ Width, Y + Height), cvScalar(200,127,127),1);

				
				int tempX = (cropRect.x-padWidth);
				int tempY = cropRect.y-padHeight;
				if (tempX < 0) tempX = 1;
				if (tempY < 0)  tempY = 1;

				CvRect eyeRegionOrig = cvRect((tempX)*scale*2, (tempY)*scale*2, cropRect.width*scale*2, cropRect.height*scale*2);
				
				eyeRegionOrig.width = eyeRegionOrig.x + eyeRegionOrig.width > m_eyeCropIn->width-1 ?   m_eyeCropIn->width-1 - eyeRegionOrig.x : eyeRegionOrig.width;
				eyeRegionOrig.height = eyeRegionOrig.y + eyeRegionOrig.height > m_eyeCropIn->height-1 ?  m_eyeCropIn->height-1 - eyeRegionOrig.y : eyeRegionOrig.height;



				printf("rectBlobInImage: %d, %d, %d, %d,\n", eyeRegionOrig.x, eyeRegionOrig.y, eyeRegionOrig.x + eyeRegionOrig.width, eyeRegionOrig.y + eyeRegionOrig.height);
			for(int i = 0; i < eyeRegionOrig.height; i++)
					{
					//unsigned char* iptr =  (unsigned char*)(cropImg->imageData + i*cropImg->widthStep);
						unsigned char*  optr = (unsigned char*)(m_eyeCropIn->imageData + (i + eyeRegionOrig.y) *m_eyeCropIn->widthStep + eyeRegionOrig.x);
						for(int j = 0; j < eyeRegionOrig.width; j++)
							optr[j] = 0;

					}


 			cvCopy(m_eyeCropIn, oframe);
	
				cvShowImage("PaddedThreshold", paddedImg);
				cvShowImage("Masked EyeCrop", oframe);
				//cvWaitKey(5);
				//getchar();
				
				if (area > 5)
				{
					cvReleaseImage(&cropImg);
					cvReleaseImage(&paddedImg);
					cvReleaseImage(&m_eyeCrop);
					cvReleaseImage(&cropImg);
					cvReleaseImage(&labelImg);
					cvReleaseImage(&threshImg);
					cvReleaseImage(&m_eyeCrop1);
					cvReleaseBlobs(blobs);
					//return false;
				}

			}

	}
				
	cvReleaseImage(&paddedImg);
	cvReleaseImage(&m_eyeCrop);
	cvReleaseBlobs(blobs);
	cvReleaseImage(&labelImg);
	cvReleaseImage(&threshImg);
	cvReleaseImage(&m_eyeCrop1);
	return true;
}
