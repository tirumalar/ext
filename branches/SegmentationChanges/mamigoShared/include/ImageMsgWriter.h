/*
 * ImageMsgWriter.h
 *
 *  Created on: 19 Feb, 2009
 *      Author: akhil
 */

#ifndef IMAGEMSGWRITER_H_
#define IMAGEMSGWRITER_H_

#include <opencv/cv.h>
#include <stdint.h>
#include "Configurable.h"
enum FORMATTYPE {eHTTP=0,eXML};
class ImageMsgWriter: public Configurable  {
public:
	ImageMsgWriter(Configuration *pConf);
	virtual ~ImageMsgWriter(){}
	virtual int write(char *outBuff,int maxSize, IplImage *img, int id, int cameraId, int frameId, int imageId, int nImages, int x, int y, float scale,int score= -1,uint64_t ts=0,
			int ill0=0,int spoof=0,float ascore=0.0f,float fscore=0.0f,float blc= 0.0f,float irx=0.0f,float iry=0.0f,int prev=-1,float halo= -1.0f,int numbits=8);
	// special version
	virtual int write_bitalign(char *outBuff,int maxSize,  IplImage *img, int id, int frameId, int imageId, int nImages, int x, int y, float scale);
	virtual int writeHB(char *outBuff,int maxSize);
	void SetHostIP(const char*inp ){svrAddr = inp;}
protected:
	const char *svrAddr;
	const char *url;
	const char *boundary;
	int footerLen;
	char footer[100];
	const char *cameraID;
	char hbMsg[100];
	int hbMsgLen;
};

#endif /* IMAGEMSGWRITER_H_ */
