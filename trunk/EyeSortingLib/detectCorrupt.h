#ifndef DETECTCORRUPT_H
#define DETECTCORRUPT_H

#include "opencv/cxcore.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"

bool detectCorruptImage(IplImage *img, double threshold, int lineLengthThresh, int borderX  = 50, int borderY = 90);

#endif
