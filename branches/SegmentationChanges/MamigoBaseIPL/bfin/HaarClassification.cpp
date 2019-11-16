/*
 * HaarClassification.cpp
 *
 *  Created on: 11 Mar, 2009
 *      Author: akhil
 *
 *  Mamigo Implementation of various Haar classification related modules
 */

#ifdef __BFIN__
#include "HaarClassification.h"
#include "GaussPyr.h"
extern "C"{
	#include "haar_private.h"
}

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_haar_scaled_variance(
/*inputs->*/	IplImage *in,char * scratch, MAMIGO_Rect *rect,int shift,
/*outputs->*/	IplImage *intImg, IplImage *sqIntImg,IplImage *varImg)
{
	//1. calc the integrala nd squared integral images
	mipl_integral_with_square_image(in,intImg,sqIntImg,scratch);
	//2. use them to calc the variance image

	int w = varImg->width;
	int h = varImg->height;
	int extra_output_width_step= varImg->widthStep -(varImg->width<<2);
	int weight_variance = (rect->width * rect->height)>>shift;
	int dstParam[13];
	int skip=2;
	int extra_input_width_step=(intImg->width - skip*w)*4 + (intImg->width*4*(skip-1));
	dstParam[0] = w;
	dstParam[1] = h;
	dstParam[2] = extra_output_width_step;	// extra widthstep
	dstParam[3] = shift;
	dstParam[4] = weight_variance;
	dstParam[5] = (int) sqIntImg->imageData - (int) intImg->imageData;
	dstParam[6] = 0;
	dstParam[7] = 0;
	dstParam[8] = 0;
	dstParam[9] = 0;
	dstParam[10] = extra_input_width_step;
	dstParam[11] = 4*skip;

	set_roi_planar_ptr((int *)(intImg->imageData), intImg->widthStep, *rect, dstParam+6);
	compute_haar_variance_scaled_nsquare_planar(dstParam, (int *)(varImg->imageData));

	return APR_SUCCESS;
}


HaarClassification::~HaarClassification(){
	if(params){
		for(int i=0;i<max_features;i++) delete params[i];
		free(params);
	}
	if(intImg) 		cvReleaseImageHeader(&intImg);
	if(sqintImg) 	cvReleaseImageHeader(&sqintImg);
	if(varImg) 		cvReleaseImageHeader(&varImg);
	if(featureImg)	cvReleaseImageHeader(&featureImg);
	if(scoreImg)	cvReleaseImageHeader(&scoreImg);
}
apr_status_t HaarClassification::init(char * scratch, int max_scratch, CvSize img, CvSize featureSize, const char *classifier_fileName){

	intImg=cvCreateImageHeader(cvSize(img.width+1, img.height+1),IPL_DEPTH_32S,1);
	sqintImg=cvCreateImageHeader(cvSize(img.width+1, img.height+1),IPL_DEPTH_32S,1);
	varImg=cvCreateImageHeader(featureSize,IPL_DEPTH_32S,1);
	featureImg=cvCreateImageHeader(featureSize,IPL_DEPTH_32S,1);
	scoreImg=cvCreateImageHeader(featureSize,IPL_DEPTH_32S,1);

	char * scrPos=scratch;
	intImg->imageData=scratch; 		scrPos+=intImg->imageSize;
	varImg->imageData=scrPos;		scrPos+=varImg->imageSize;
	sqintImg->imageData=scrPos;
	scratch_for_variance=scrPos+sqintImg->imageSize;

	int max_scratch_need=(scratch_for_variance-scratch) + ((1+img.width)<<3);
	assert(max_scratch>max_scratch_need);

	/// we can safely reuse the area used by squint->imagedata
	featureImg->imageData=scrPos;	scrPos+=featureImg->imageSize;
	scoreImg->imageData=scrPos; 	scrPos+=scoreImg->imageSize;
	max_scratch_need=scrPos-scratch;
	assert(max_scratch>max_scratch_need);

	// load the parameters
	params=MIPLClassificationParams::loadFromFile(classifier_fileName,max_features,m_thresh);

	for(int i=0;i<max_features;i++){
		params[i]->initializeWith(intImg,featureImg);
	}
	rect.x=rect.y=0;
	rect.width=rect.height=24;
	return APR_SUCCESS;
}
bool HaarClassification::isHaarEye(IplImage *eyeImg){

	mipl_haar_scaled_variance(eyeImg,scratch_for_variance,&rect,4,intImg,sqintImg,varImg);
	cvSetZero(scoreImg);
	unsigned int best_score=0;
	for(int i=0;i<max_features;i++){
		best_score=params[i]->haarDecision(varImg,4,scoreImg);
		if(best_score>m_thresh){
			return true;
		}
	}
	return false;
//	}
//	double scale=(1<<24);
//	double score=best_score/scale;
//	printf("best_score %f\n",score);
//
//	return best_score>m_thresh;

}

#endif //__BFIN__
