/*
 * ImageXMLMsgWriter.cpp
 *
 *  Created on: 20 Feb, 2009
 *      Author: akhil
 */
#include <stdio.h>
#include "ImageXMLMsgWriter.h"
#include "base64.h"

int ImageXMLMsgWriter::write(char *outBuff,int maxSize, IplImage *img, int id, int frameId, int imageId, int nImages, int x, int y, float scale)
{
	static char hdrFmt[]="<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
	"<images><image id=\"%d\" cameraId=\"%s\" frameId=\"%d\" imageId=\"%d\" numberOfImages=\"%d\" scale=\"%3.2f\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" maxValue=\"255\">";

	static char ftrFmt[]="</image><iris diameter=\"120\"/></images>";
	static int ftrSize=strlen(ftrFmt);

	std::string encoded_img=base64_encode((unsigned char *)(img->imageData), img->imageSize);

	int count=sprintf(outBuff,hdrFmt,id,cameraID,frameId,imageId,nImages,scale,x,y,img->width,img->height);

	int neededBuffer=count+encoded_img.length()+ftrSize;

	if(neededBuffer>maxSize){
		printf("Too small sendBuffer needed:%d, actual:%d",neededBuffer,maxSize);
		bool tooSmallSendBuffer=false;
		assert(tooSmallSendBuffer);
	}

	outBuff+=count;
	memcpy(outBuff,encoded_img.c_str(),encoded_img.length());
	outBuff+=encoded_img.length();
	count+=encoded_img.length();
	count+=sprintf(outBuff,ftrFmt);
	return count;
}



