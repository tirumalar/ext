#ifndef PUPILSEGMENTATION_H 
#define PUPILSEGMENTATION_H 
int segmentation(const char *filename);
int segmentation(const char *inputImage, const char *inputCircle,  const char *label);
int segmentation(unsigned char *data, int w, int h, float pupilX, float pupilY, float pupilR, float irisX, float irisY, float irisR, float Threshold);
#endif