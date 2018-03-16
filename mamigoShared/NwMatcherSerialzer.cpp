/*
 * NwMatcherSerialzer.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: developer1
 */

#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include <stdio.h>


NwMatcherSerialzer::NwMatcherSerialzer() {

}

NwMatcherSerialzer::~NwMatcherSerialzer() {

}

int NwMatcherSerialzer::writeInt(char *dst,int* var){
	int ret = sizeof(int);
//	printf("NwMatcherSerialzer::writeInt %#x %#x %#x \n",dst,var,ret);
	memcpy(dst,var,ret);
	return ret;
}
int NwMatcherSerialzer::writeLong64(char *dst,uint64* var){
	int ret = sizeof(uint64_t);
	memcpy(dst,var,ret>>1);
	memcpy(dst+4,((char*)var)+4,ret>>1);
	return ret;
}

int NwMatcherSerialzer::writeFloat(char *dst,float* var){
	int ret = sizeof(float);
//	printf("NwMatcherSerialzer::writeFloat %#x %#x %#x \n",dst,var,ret);
	memcpy(dst,var,ret);
	return ret;
}

int NwMatcherSerialzer::writeBuffer(char *dst,char *src,int size){
//	printf("NwMatcherSerialzer::writeBuffer %#x %#x %#x \n",dst,src,size);
	memcpy(dst,src,size);
	return size;
}

void NwMatcherSerialzer::Append(char *ptr,void *inp){

}

int NwMatcherSerialzer::getBitStreamSize(const char* buff){

	const char *buff1 = strstr(buff,"IRISDATA");
	const char *temp = strstr(buff,";");
	if(temp == NULL){
		printf("Error In finding ;\n");
		return 0;
	}
	int offset = temp-(buff1)+1;
	int val = offset&(ALIGN_BYTES -1);
	if(val){
		offset = offset + ALIGN_BYTES - val;
	}

	int totalsz = *((int*)(buff1+offset));
	return totalsz;
}

bool NwMatcherSerialzer::ExtractNwMsg(IrisData* data,char* buff,char*hdr){
	char *temp = buff;
	int offset = 0;
	int hdrlen = strlen(hdr);
//	printf("Header %d -> %s \n",hdrlen,hdr);

	temp = strstr(buff,hdr);
//	printf("Data -> %s\n",temp);
	if(temp == NULL){
		printf("Error In finding %s\n",hdr);
		return false;
	}
	offset = temp - buff + strlen(hdr);

	temp = strstr(buff+offset,";");
	if(temp == NULL){
		printf("Error In finding ;\n");
		return false;
	}
//	printf("Data1 -> %s\n",temp);
	offset += temp-(buff+offset)+1;
//	printf("Offset %d\n",offset);

	int val = offset&(ALIGN_BYTES -1);
	if(val){
		offset = offset + ALIGN_BYTES - val;
	}

	int totalsz;
	offset+= writeInt((char*)&totalsz,(int*)(buff+offset));
//	printf("Total Sz %d\n ",totalsz);
	// TBD Put some check here for validity of data::::

//	temp = strstr(buff+offset,";");
//	if(temp == NULL){
//		printf("Error In finding ;\n");
//		return false;
//	}
//	offset += temp-(buff+offset)+1;


	// Read IRIS
	int ret = writeBuffer((char*)data->getIris(),buff+offset,2560);
	offset+=ret;
//	printf("Offset After IRIS %d\n",offset);

	uint64_t ts;
	ret = writeLong64((char*)&ts,(uint64_t*)(buff+offset));
//	printf("TimeStamp %d %#llx \n",ret,ts);
	offset+=ret;
	data->setTimeStamp(ts);
//	printf("Offset After TS %d\n",offset);
	//Read Centroid
	CvPoint2D32f centroid;
	ret = writeFloat((char*)&centroid.x,(float*)(buff+offset));
	offset+=ret;
	ret = writeFloat((char*)&centroid.y,(float*)(buff+offset));
	offset+=ret;
	data->setSpecCentroid(centroid.x,centroid.y);
//	printf("Offset After SC %d\n",offset);
	//Write IRIS CIRCLE
	CvPoint3D32f cp;
	ret = writeFloat((char*)&cp.x,(float*)(buff+offset));
	offset+=ret;
	ret = writeFloat((char*)&cp.y,(float*)(buff+offset));
	offset+=ret;
	ret = writeFloat((char*)&cp.z,(float*)(buff+offset));
	offset+=ret;

	data->setIrisCircle(cp.x,cp.y,cp.z);
//	printf("Offset After IC %d\n",offset);
	//Write PUPIL CIRCLE
	ret = writeFloat((char*)&cp.x,(float*)(buff+offset));
	offset+=ret;
	ret = writeFloat((char*)&cp.y,(float*)(buff+offset));
	offset+=ret;
	ret = writeFloat((char*)&cp.z,(float*)(buff+offset));
	offset+=ret;
	data->setPupilCircle(cp.x,cp.y,cp.z);
//	printf("Offset After PC %d\n",offset);
	//Write Score
	float score ;
	ret = writeFloat((char*)&score,(float*)(buff+offset));
	offset+=ret;
	data->setAreaScore(score);

	ret = writeFloat((char*)&score,(float*)(buff+offset));
	offset+=ret;
	data->setFocusScore(score);

	ret = writeFloat((char*)&score,(float*)(buff+offset));
	offset+=ret;
	data->setHalo(score);

	ret = writeFloat((char*)&score,(float*)(buff+offset));
	offset+=ret;
	data->setBLC(score);
//	printf("Offset After BLC %d\n",offset);
//	int val ;
	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setIlluminatorState(val);

	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setFrameIndex(val);

	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setEyeIndex(val);

	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setPrevIndex(val);

	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setSegmentation(val?1:0);

	ret = writeInt((char*)&val,(int*)(buff+offset));
	offset+=ret;
	data->setIrisRadiusCheck(val?1:0);

//	printf("Offset After RC %d\n",offset);

	temp = strstr(buff+offset,";");
	if(temp == NULL){
		printf("Error In finding End;\n");
		return false;
	}
	int len= temp - (buff + offset);
	std::string cam(buff+offset,len);
	data->setCamID((char*)cam.c_str());

	CURR_TV_AS_USEC(ts1);
	data->setEnqueTimeStamp(ts1);

	return true;
}


int NwMatcherSerialzer::MakeNwMsg(char *dst,IrisData *data,char *hdr){
	int offset = 0;
	int hdrlen = strlen(hdr);
	memcpy(dst+offset,hdr,hdrlen);
	dst[hdrlen] = ';';
	offset+=1;
	offset += hdrlen;
	int val = offset&(ALIGN_BYTES -1);
	if(val){
		offset = offset + ALIGN_BYTES - val;
	}
	char* size = dst+offset;
	offset+=sizeof(int);

//	printf("Offset After sz %d\n",offset);
//	dst[offset]=';';
//	offset+=1;

	// Write IRIS
	int ret = writeBuffer(dst+offset,(char*)data->getIris(),2560);
	offset+=ret;
//	printf("Offset After IRIS %d\n",offset);
	//Write TimeStamp
	uint64_t ts = data->getTimeStamp();
	ret = writeLong64(dst+offset,&ts);
	offset+=ret;
//	printf("Offset After TS %d\n",offset);

	//write spec centroid
	CvPoint2D32f centroid = data->getSpecCentroid();
	ret = writeFloat(dst+offset,&centroid.x);
	offset+=ret;
	ret = writeFloat(dst+offset,&centroid.y);
	offset+=ret;

//	printf("Offset After SC %d\n",offset);

	//Write IRIS CIRCLE
	CvPoint3D32f cp = data->getIrisCircle();
	ret = writeFloat(dst+offset,&cp.x);
	offset+=ret;
	ret = writeFloat(dst+offset,&cp.y);
	offset+=ret;
	ret = writeFloat(dst+offset,&cp.z);
	offset+=ret;

//	printf("Offset After IC %d\n",offset);

	//Write PUPIL CIRCLE
	cp = data->getPupilCircle();
	ret = writeFloat(dst+offset,&cp.x);
	offset+=ret;
	ret = writeFloat(dst+offset,&cp.y);
	offset+=ret;
	ret = writeFloat(dst+offset,&cp.z);
	offset+=ret;

//	printf("Offset After PC %d\n",offset);

	//Write Score
	float score = data->getAreaScore();
	ret = writeFloat(dst+offset,&score);
	offset+=ret;

	score = data->getFocusScore();
	ret = writeFloat(dst+offset,&score);
	offset+=ret;

	score = data->getHalo();
	ret = writeFloat(dst+offset,&score);
	offset+=ret;

	score = data->getBLC();
	ret = writeFloat(dst+offset,&score);
	offset+=ret;

//	printf("Offset After BLC %d\n",offset);

	val = data->getIlluminatorState();
	ret = writeInt(dst+offset,&val);
	offset+=ret;
	val = data->getFrameIndex();
	ret = writeInt(dst+offset,&val);
	offset+=ret;
	val = data->getEyeIndex();
	ret = writeInt(dst+offset,&val);
	offset+=ret;
	val = data->getPrevIndex();
	ret = writeInt(dst+offset,&val);
	offset+=ret;

	bool seg = data->getSegmentation();
	val = seg?1:0;
	ret = writeInt(dst+offset,&val);
	offset+=ret;

	seg = data->getIrisRadiusCheck();
	val = seg?1:0;
	ret = writeInt(dst+offset,&val);
	offset+=ret;

//	printf("Offset After Radius check %d\n",offset);

	hdrlen = strlen(data->getCamID());
	memcpy(dst+offset,data->getCamID(),hdrlen);
	dst[offset+hdrlen]=';';
	offset+=hdrlen+1;

//	printf("Offset After CamID %d\n",offset);

	//Write the size;
	writeInt(size,&offset);

//	printf("Size %d\n",offset);

	return offset;

}

int NwMatcherSerialzer::GetSizeOfNwMsg(IrisData *data,char *hdr){
	int sz = strlen(hdr)+1;
	int val = sz&(ALIGN_BYTES -1);
	if(val){
		sz = sz + ALIGN_BYTES - val;
	}
	sz += sizeof(int);
	sz += data->getSizeofIrisData();
	return sz;
}



