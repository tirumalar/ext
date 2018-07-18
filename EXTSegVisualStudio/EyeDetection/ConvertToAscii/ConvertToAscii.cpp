#include <highgui.h>
#include <stdio.h>

void main()
{
	IplImage *img = cvLoadImage("data/eye012_0.pgm", CV_LOAD_IMAGE_UNCHANGED);
	FILE *fp = fopen("data/eye012_0.txt", "wt");

	for(int i=0; i<img->height; i++)
	{
		unsigned char *iptr = (unsigned char *) img->imageData + i*img->width;
		for(int j=0;j<img->width; j++)
			fprintf(fp, "%d ", (int) iptr[j]);
		
		fprintf(fp, "\n");
	}

	fclose(fp);

	cvReleaseImage(&img);
}