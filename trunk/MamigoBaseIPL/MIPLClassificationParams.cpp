/*
 * MIPLClassificationParams.cpp
 *
 *  Created on: 12 Mar, 2009
 *      Author: akhil
 */

#include "MIPLClassificationParams.h"
#include "math.h"
extern "C"{
#include "haar_private.h"
}

#ifdef __BFIN__

//used for positive numbers only
void MIPLClassificationParams::representDouble(double mag, char& exponent, unsigned int& mantissa){
	if(mag<=0.0){	// special case
		mantissa=0;
		exponent=31;
		return;
	}

	exponent=30-ilogb(mag);

	mantissa = (int)(mag*pow(2,exponent));
}

void MIPLClassificationParams::representThresh(double d, char& exponent, unsigned int& mantissa, bool& isNeg){
	isNeg=signbit(d);
	double mag=d*d;
	representDouble(mag,exponent,mantissa);
}

MIPLClassificationParams *fromStream(std::ifstream& f, char tScale){

	double good_score,thresh;
	int scaled_good_score;
	int parity, type, x1, x2, x3, x4, y1, y2, y3, y4;
	f>>good_score;
	f>>thresh>>parity>>type;
	f>>x1>>x2>>x3>>x4>>y1>>y2>>y3>>y4;
	f.ignore(256,'\n');
	assert(parity == 0 || parity == 1);
	assert(type>=0 && type<=4);
	scaled_good_score=(int)(good_score*(1<<tScale));
	return new MIPLClassificationParams(scaled_good_score,type,parity,thresh,x1, x2, x3, x4, y1, y2, y3, y4);
}

MIPLClassificationParams **MIPLClassificationParams::loadFromFile(const char* classifier_filename, int& max_features, unsigned int& thresh){
	std::ifstream f;
	int i;
	char tScale;
	double  dThresh;

	f.open(classifier_filename);
	f >> max_features >> dThresh;

	// find a scale suitable for thresh: assumption 4*m_thresh>sum(all thresh)
	representDouble(fabs(4*dThresh),tScale, thresh);
	thresh=thresh>>2;
	MIPLClassificationParams **features= (MIPLClassificationParams **)calloc(max_features,sizeof(MIPLClassificationParams *));

	for(i=0;i<max_features;i++)
	{
		features[i]=fromStream(f, tScale);
	}
	f.close();
	return features;
}

int MIPLClassificationParams::doComputation(IplImage *integralImage,
		IplImage *featureImg) {
	initializeWith(integralImage, featureImg);
	int *out= (int *)(featureImg->imageData);
	int ret=-1;
	switch (type) {
		case 0:
			ret=compute_haar_feature_with_skip_type_0(computeParam, out);
			break;
		case 1:
			ret=compute_haar_feature_with_skip_type_1(computeParam, out);
			break;
		case 2:
			ret=compute_haar_feature_with_skip_type_2(computeParam, out);
			break;
		case 3:
			ret=compute_haar_feature_with_skip_type_3(computeParam, out);
			break;
		case 4:
			ret=compute_haar_feature_with_skip_type_4(computeParam, out);
			break;
		default:
			bool knownType = false;
			assert(knownType);
			throw("unknown type");
		}
	return getfeatureshift(ret);
}
void MIPLClassificationParams::initializeWith(IplImage *integralImage,
		IplImage *featureImage) {
	if (integralImg == integralImage && featureImg == featureImage) {
		// no need to init we are all set
		return;
	}
	integralImg = integralImage;
	featureImg = featureImage;

	assert(featureImg->depth==IPL_DEPTH_32S);
	assert(integralImg->depth==IPL_DEPTH_32S);

	int w = featureImg->width;
	int h = featureImg->height;
	int skip=getSkip();
	setWidth(w);
	setHeight(h);
	setExtra_output_widthstep(featureImg->widthStep - (w << 2));

	int extra_input_width_step =(integralImg->width - skip*(w+1))*4
			+ (integralImg->width)*4*(skip-1);

	setExtra_input_widthstep(extra_input_width_step);

	int *img = (int *) (integralImg->imageData);
	int widthStep = integralImg->widthStep;
	int *op = computeParam + eX1;
	switch (type) {
	case 0:
		set_roi_ptr_type_0(img, widthStep, XYs, op);
		break;
	case 1:
		set_roi_ptr_type_1(img, widthStep, XYs, op);
		break;
	case 2:
		set_roi_ptr_type_2(img, widthStep, XYs, op);
		break;
	case 3:
		set_roi_ptr_type_3(img, widthStep, XYs, op);
		break;
	case 4:
		set_roi_ptr_type_4(img, widthStep, XYs, op);
		break;
	default:
		bool knownType = false;
		assert(knownType);
		throw("unknown type");
	}
}
unsigned int MIPLClassificationParams::haarDecision(IplImage *var_img, int shift, IplImage *cumm_out_score)
{
	int feature_shift=doComputation(integralImg,featureImg);

	decParam[eScore_out]=(int)(cumm_out_score->imageData);
	decParam[eThreshExponent]=thresh_exponent + (12 - shift) + feature_shift;
	decParam[eFeatureShift]=-feature_shift;

	int *feature_out=(int *)(featureImg->imageData);
	int *variances=(int *)(var_img->imageData);

	unsigned int ret=0;
	if(thresh_isNeg){
		ret=HaarDecisionNegativeThreshold(feature_out,variances,decParam);
	}
	else{
		ret=HaarDecisionPositiveThreshold(feature_out,variances,decParam);
	}
	return ret;
}
#endif
