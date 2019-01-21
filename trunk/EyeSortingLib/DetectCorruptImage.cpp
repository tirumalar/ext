
#include "detectCorrupt.h"
bool detectCorruptImage(IplImage *img, double threshold, int lineLengthThresh, int borderX, int borderY)
{

	IplImage *sobelX;
	IplImage *sobelY;
	IplImage *tempX;
	IplImage *tempY;

	sobelX =cvCreateImage(cvSize(img->width,img->height),8,1);
	sobelY =cvCreateImage(cvSize(img->width,img->height),8,1);
	// Create Temp Images
	tempX =cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_16S,1);
	tempY =cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_16S,1);

	cvSobel(img,tempX,1,0,3);
	cvSobel(img,tempY,0,1,3);

	// Convert to the Absolute Value
	cvConvertScaleAbs(tempX,sobelX,1,0);
	cvConvertScaleAbs(tempY,sobelY,1,0);

	
	unsigned char *base = (unsigned char *)sobelY->imageData;

	cvThreshold(sobelY, sobelY, 180.0, 255, 0);
	bool corrupted = false;
	for (int rowId = borderY; rowId < sobelY->height -  borderY; rowId++)
	{
		for (int colId = borderX; colId < sobelY->width -  borderX; colId++)
		{
			int lineLength = 0;
			unsigned char tmp= base[rowId*sobelY->width + colId];
			if ( tmp == 255 && colId + lineLengthThresh < sobelY->width)
			{
				for (int i = colId; i < colId + lineLengthThresh ; i++)
						if (base[rowId*sobelY->width + i] == 255) 	lineLength++;

			}
			if (lineLength > lineLengthThresh - 2)
			{
				corrupted = true;
				break;
			}

		}
		if (corrupted) break;
	}

	
	
	cvReleaseImage(&sobelX);
	cvReleaseImage(&sobelY);

	cvReleaseImage(&tempX);
	cvReleaseImage(&tempY);

	return corrupted;

}
