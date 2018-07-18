/*
 * ImageXMLMsgWriter.h
 *
 *  Created on: 20 Feb, 2009
 *      Author: akhil
 */

#ifndef IMAGEXMLMSGWRITER_H_
#define IMAGEXMLMSGWRITER_H_

#include "ImageMsgWriter.h"

class ImageXMLMsgWriter: public ImageMsgWriter {
public:
	ImageXMLMsgWriter(Configuration *pConf):ImageMsgWriter(pConf){}
	virtual ~ImageXMLMsgWriter(){}
	virtual int write(char *outBuff, int maxSize, IplImage *img, int id, int frameId, int imageId, int nImages, int x, int y, float scale);
};

#endif /* IMAGEXMLMSGWRITER_H_ */
