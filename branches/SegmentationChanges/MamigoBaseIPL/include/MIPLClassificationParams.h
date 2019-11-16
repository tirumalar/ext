/*
 * MIPLClassificationParams.h
 *
 *  Created on: 12 Mar, 2009
 *      Author: akhil
 * Represents the parameters passed to the to the assembly classification routines
 * They follow the flyweight pattern: created initially and reused over and over
 */

#ifndef MIPLCLASSIFICATIONPARAMS_H_
#define MIPLCLASSIFICATIONPARAMS_H_
#include "MamigoBaseIPL_def.h"
#include <opencv/cxcore.h>
#include <fstream>

class MIPLClassificationParams
{
public:

	static void representDouble(double mag, char& exponent, unsigned int& mantissa);

	static void representThresh(double d, char& exponent, unsigned int& mantissa, bool& isNeg);

	static MIPLClassificationParams **loadFromFile(const char* classifier_filename, int& max_features, unsigned int& thresh);

    MIPLClassificationParams(int good_score, int type, int parity, double thresh, int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4)
    {
    	setGood_score(good_score);
    	setType(type);
		//make sure Thresh is set before parity
    	setThresh(thresh);
    	setParity(parity);

    	int i=0;
        XYs[i++] = x1;
        XYs[i++] = x2;
        XYs[i++] = x3;
        XYs[i++] = x4;
		XYs[i++] = y1;
		XYs[i++] = y2;
		XYs[i++] = y3;
		XYs[i++] = y4;

		setSkip(2);	// default

		// remember the sign and exponent for future, initialize mantissa
		unsigned int mantissa;
		representThresh(this->thresh,thresh_exponent,mantissa,thresh_isNeg);
		decParam[eThreshold]=mantissa;
		decParam[eBase_score]=0;
		decParam[eConstt]=81;
		integralImg=0;
		featureImg=0;
    }

    virtual ~MIPLClassificationParams()
    {
    }

    //test only
    int doComputation(IplImage *integralImage, IplImage *featureImg);

    void initializeWith(IplImage *integralImage, IplImage *featureImage);

    unsigned int haarDecision(IplImage *var_img, int shift, IplImage *cumm_out_score);

    inline int getType() const
    {
        return type;
    }
    inline int getParity() const
    {
        return parity;
    }
    inline double getThresh() const
    {
        return thresh;
    }

//test only
    int *getParams() { return computeParam;}

protected:
    inline void setWidth(int width)
    {
    	decParam[eWidth]=computeParam[eWidth] = width;

    }

    inline void setHeight(int height)
    {
    	decParam[eHeight]=computeParam[eHeight] = height;
    }

    inline void setExtra_output_widthstep(int gap)
    {
        computeParam[eExtra_outout_widthstep] = gap;
    }
    inline void setExtra_input_widthstep(int gap)
    {
        computeParam[eExtra_input_widthstep] = gap;
    }
    inline void setType(int type)
    {
        this->type = type;
    }
    inline void setParity(int parity)
    {
        this->parity = parity;
        if(parity==1) {
        	setWeight(-1);
        	thresh=-thresh;
        }
        else setWeight(1);
    }
    inline void setThresh(double thresh)
    {
        this->thresh = thresh;
    }
    inline void setGood_score(int good_score)
    {
         decParam[eGood_score]=good_score;
    }
    inline void setWeight(int w)
    {
        computeParam[eWeight] = w;
    }

    inline void setSkip(int s)
    {
        computeParam[eSkip] = s<<2;
    }

    inline int getSkip()
    {
        return computeParam[eSkip]>>2;
    }
    int computeParam[14];
    int decParam[10];
    int XYs[8];
    enum computeParamPos{ eWidth = 0, eHeight, eExtra_outout_widthstep, eWeight, eX1, eX2, eX3, eX4, eY1, eY2, eY3, eY4, eExtra_input_widthstep,eSkip};
    enum decParamPos{eScore_out=2,eThreshold,eThreshExponent,eBase_score,eGood_score,eFeatureShift,eConstt};
    int type;
    int parity;

    double thresh;
    bool thresh_isNeg;
    char thresh_exponent;
    IplImage *integralImg, *featureImg;
};

#endif /* MIPLCLASSIFICATIONPARAMS_H_ */
