/*
 * HaarClassification.h
 *
 *  Created on: 11 Mar, 2009
 *      Author: akhil
 *
 *  Mamigo Implementation of of various Haar classification related modules
 */

#ifndef HAARCLASSIFICATION_H_
#define HAARCLASSIFICATION_H_

#include "apr.h"
#include "apr_errno.h"
#include "MamigoBaseIPL_def.h"
#include "CvInterfaceDefs.h"
#include <opencv/cxcore.h>
#include "MIPLClassificationParams.h"

/*
 *  This method first Calculates the integral and squared integral and then
 *  calculates the scaled haar variance (sdv^2) over various offsets within rect.
 *  The number of offsets is determined based on the size of variance image.
 *
 *  The output is a variance, i.e. sdv^2, which is scaled by (rect.width * rect.height)*(rect.width * rect.height)>>shift
 *  this much scaling ensures that output doesn't overflow and maintains its sign
 *
 *  Assumptions:
 *  	a. scratch has to be 2*(in->width+1) integers = 8*(in->width+1) bytes
 *  	b. intImg->imageData < sqIntImg->imageData
 *  	c. intImg, sqIntImg, varianceImg are integer images (depth 4bytes)
 */

apr_status_t MAMIGO_BASE_IPL_DLL_EXPORT mipl_haar_scaled_variance(
/*inputs->*/	IplImage *in,char * scratch, MAMIGO_Rect *rect,int shift,
/*outputs->*/	IplImage *intImg, IplImage *sqIntImg,IplImage *varImg);


/*
 * Mamigo Implemetation of Haar Classification on Bfin
 */
class MAMIGO_BASE_IPL_DLL_EXPORT HaarClassification{
public:
	HaarClassification():intImg(0),sqintImg(0), varImg(0),featureImg(0),scoreImg(0),params(0){}
	~HaarClassification();
	/*
	 * Does all allocations etc. thorws exception if scratch is insufficient
	 * initializes parameters
	 */
	apr_status_t init(char * scratch, int max_scratch, CvSize eyeSize, CvSize featureSize, const char *classifier_fileName);
	bool isHaarEye(IplImage *eyeImg);
protected:
	IplImage *intImg;
	IplImage *sqintImg;
	IplImage *varImg;
	IplImage *featureImg;
	IplImage *scoreImg;
	MIPLClassificationParams **params;
	int max_features;
	unsigned int m_thresh;
	char * scratch_for_variance;
	MAMIGO_Rect rect;
};

#endif /* HAARCLASSIFICATION_H_ */
