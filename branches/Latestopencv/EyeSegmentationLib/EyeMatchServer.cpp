#include "EyeFeatureServer.h"
#include "EyeMatchServer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "IrisMacros.h"

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}
#ifdef __ARM__
#include <arm_neon.h>
#endif

using namespace std;

#define NOMINAL_COMMON_BITS 4100.0


static int number_of_ones(int num)
{
	int cnt = 0;
	while(num)
	{
		cnt += num%2;
		num /= 2;
	}
	return cnt;
}


EyeMatchServer::EyeMatchServer(int featureLength, int numRows, int byteSize, int shift): m_featureLength(featureLength),m_scratch(NULL)
, m_numRows(numRows)
, m_byteSize(byteSize)
, m_ShiftLeft(-shift)
, m_ShiftRight(shift)
, m_nominalCommonBits(NOMINAL_COMMON_BITS)
, m_minCommonBits(0)
, m_DoRawScore(0)
, m_ShiftIncrement(1)
, m_lut16(0)
, m_maskArray(NULL)
, m_codeArray(NULL)
, m_maskTemp(NULL)
{
	int length = ((m_ShiftRight - m_ShiftLeft +1)+3)&(~3);
	printf("Shifts from %d for %d\n",-m_ShiftRight,length);

	for(int i=0;i<2;i++)
		m_temp[i] = new unsigned char[2*m_featureLength];

	for( int i = 0 ; i < 256 ; i++ )
		m_lut[i] = number_of_ones(i);


	for(int shift=m_ShiftLeft,i=0;shift<=m_ShiftRight;shift+=m_ShiftIncrement,i++){
		m_shiftedCode[i] = new unsigned char[m_featureLength];
		m_shiftedMask[i] = new unsigned char[m_featureLength];
	}
#ifndef __ARM__
	m_lut16 = (unsigned char *) malloc(256*256);
	for( int i = 0 ; i < 256*256 ; i++ ){
		m_lut16[i] = (unsigned char) number_of_ones(i);
	}
#endif

	int arrsz = m_ShiftRight-m_ShiftLeft +1;

	m_maskArray = (unsigned char **)calloc(arrsz,(sizeof(unsigned char *)));
	m_codeArray = (unsigned char **)calloc(arrsz,(sizeof(unsigned char *)));
	m_maskTemp = new unsigned char[m_featureLength];

	for(int i =0;i< arrsz;i++){
		m_maskArray[i] = new unsigned char[m_featureLength];
		m_codeArray[i] = new unsigned char[m_featureLength];
	}

	m_scratch=malloc(m_featureLength*(m_ShiftRight-m_ShiftLeft+2));
}

EyeMatchServer::~EyeMatchServer(void)
{

	for(int i=0;i<2;i++)
		delete [] m_temp[i];
	int arrsz = m_ShiftRight-m_ShiftLeft +1;

	for(int i=0;i<arrsz;i++){
		delete m_shiftedCode[i];
		delete m_shiftedMask[i];
	}
	free(m_lut16);

	for(int i =0;i< arrsz;i++){
		delete m_maskArray[i];
		delete m_codeArray[i];
	}
	delete m_maskTemp;
	free(m_maskArray);
	free(m_codeArray);
	free(m_scratch);
}





CvPoint EyeMatchServer::GetHammingDistance1(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval,int step)
{
	int bits = 0;
	int dist = 0;

	unsigned int distarr[1024];
	unsigned int bitarr[1024];
	unsigned char *distptr,*bitptr;

	distptr = (unsigned char *)distarr;
	bitptr = (unsigned char *)bitarr;

	int len = m_featureLength/m_numRows;

	int k=0;
//	unsigned int *m2=(unsigned int *)m2ptr,*f2=(unsigned int *)f2ptr;
//	for(int i=0;i<m_numRows;i++){
//		unsigned int *m1 = (unsigned int *)(m1ptr + step*i);
//		unsigned int *f1 = (unsigned int *)(f1ptr + step*i);
//		printf("%#x %#x %#x %#x \n",m1,f1,m2,f2);
//		for(int zz=0;zz<len>>2;zz++){
//			unsigned int temp = (m1[zz]) & (m2[k]) & (maskval);
//			bitarr[k] = temp;
//			distarr[k] = ((f1[zz]^f2[k]) & temp);
//			k++;
//		}
//	}
	unsigned char *bitarrc = (unsigned char *)bitarr;
	unsigned char *distarrc = (unsigned char *)distarr;

	unsigned char *m2=(unsigned char *)m2ptr,*f2=(unsigned char *)f2ptr;
	for(int i=0;i<m_numRows;i++){
		unsigned char *m1 = (unsigned char *)(m1ptr + step*i);
		unsigned char *f1 = (unsigned char *)(f1ptr + step*i);
//		printf("%#x %#x %#x %#x \n",m1,f1,m2,f2);
		for(int zz=0;zz<len;zz++){
			unsigned char temp = (m1[zz]) & (m2[k]) & (maskval);
			bitarrc[k] = temp;
			distarrc[k] = ((f1[zz]^f2[k]) & temp);
			k++;
		}
	}

	for(int zz=0;zz<m_featureLength;zz++){
		dist += m_lut[*distptr++];
		bits += m_lut[*bitptr++];
	}

	return cvPoint(dist, bits);
}

CvPoint EyeMatchServer::GetHammingDistance(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval)
{
	int bits = 0;
	int dist = 0;

	#define DATA_TYPE unsigned int

	DATA_TYPE distarr[1024];
	DATA_TYPE bitarr[1024];

	DATA_TYPE *m1,*m2,*f1,*f2;
	unsigned char *distptr,*bitptr;
	m1 = (DATA_TYPE *)m1ptr;
	m2 = (DATA_TYPE *)m2ptr;
	f1 = (DATA_TYPE *)f1ptr;
	f2 = (DATA_TYPE *)f2ptr;
	distptr = (unsigned char *)distarr;
	bitptr = (unsigned char *)bitarr;

	for(int zz=0;zz<m_featureLength>>2;zz++)
	{
		DATA_TYPE temp = (m1[zz]) & (m2[zz]) & (maskval);
		bitarr[zz] = temp;
		distarr[zz] = ((f1[zz]^f2[zz]) & temp);
	}


	for(int zz=0;zz<m_featureLength;zz++){
		dist += m_lut[*distptr++];
		bits += m_lut[*bitptr++];
	}
	return cvPoint(dist, bits);
}


void EyeMatchServer::LeftShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift)
{
	int jump = m_featureLength/m_numRows;
	for(int kk = 0;kk< m_numRows;kk++)
	{
		//printf("%d",kk);
		memcpy(dest + jump*(kk+1) - m_byteSize*abs(shift), src + jump*kk, m_byteSize*abs(shift));
		memcpy(dest + jump*kk, src + jump*kk + m_byteSize*abs(shift), jump - m_byteSize*abs(shift));
	}

}

void EyeMatchServer::RightShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift)
{
	int jump = m_featureLength/m_numRows;

	for(int kk = 0;kk< m_numRows;kk++)
	{
		memcpy(dest+jump*kk, src+jump*(kk+1)-m_byteSize*shift, m_byteSize*shift);
		memcpy(dest+jump*kk+m_byteSize*shift, src+jump*kk, jump - m_byteSize*shift);
	}

}


double EyeMatchServer::Match(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval)
{
	double min_score = 10.0;

	std::pair<int,int> dist = MatchNumDen(f1ptr, m1ptr, f2ptr, m2ptr, maskval);
	min_score = (dist.second > m_minCommonBits)? 1.0*dist.first/dist.second : 1.0;

	// Subtract penalty if no computing raw scores
	if(!m_DoRawScore)
		min_score = 0.5-(0.5-min_score)*sqrt((double)dist.second/m_nominalCommonBits);

	return min_score;
}


int EyeMatchServer::UpdateWith32bytesCushion(unsigned char *f1ptr, unsigned char *m1ptr){
    //Write other logic
    int jump = m_featureLength / m_numRows;
    int dststep = jump + 32 + 32;
    unsigned char *fs1ptr = m_temp[0];
    unsigned char *ms1ptr = m_temp[1];
    for(int kk = 0;kk < m_numRows;kk++){
        memcpy(fs1ptr + dststep * kk, f1ptr + jump * (kk + 1) - 32, 32); //copy last 32 to first
        memcpy(fs1ptr + dststep * kk + 32, f1ptr + jump * kk, jump);
        memcpy(fs1ptr + dststep * (kk + 1) - 32, f1ptr + jump * kk, 32); //copy first 32 to last
        memcpy(ms1ptr + dststep * kk, m1ptr + jump * (kk + 1) - 32, 32); //copy last 32 to first
        memcpy(ms1ptr + dststep * kk + 32, m1ptr + jump * kk, jump);
        memcpy(ms1ptr + dststep * (kk + 1) - 32, m1ptr + jump * kk, 32); //copy first 32 to last
    }
    return dststep;
}


std::pair<int,int> EyeMatchServer::MatchNumDen(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr, unsigned int maskval)
{
	double min_score = 10.0;
	m_bestMatchScore = cvPoint(0, m_featureLength);
	std::pair<int,int> ret(1,1);

	if(m_featureLength<640){
		for(int shift=m_ShiftLeft;shift<=m_ShiftRight;shift++)
		{
			CvPoint dist;

			unsigned char *fs1ptr = m_temp[0];
			unsigned char *ms1ptr = m_temp[1];

			if(shift<0) // left shift
			{
				LeftShiftFeatureVector(f1ptr, fs1ptr, shift);
				LeftShiftFeatureVector(m1ptr, ms1ptr, shift);
			}
			else if(shift>0)		// right shiftsGetHammingDistance
			{
				RightShiftFeatureVector(f1ptr, fs1ptr, shift);
				RightShiftFeatureVector(m1ptr, ms1ptr, shift);
			}
			else
			{
				fs1ptr = f1ptr;
				ms1ptr = m1ptr;
			}

			dist = GetHammingDistance(fs1ptr, ms1ptr, f2ptr, m2ptr,maskval);
			//printf("Dist Bits %d %d\n",dist.x,dist.y);
			double score = (dist.y > m_minCommonBits)? 1.0*dist.x/dist.y : 1.0;
			m_HammingDistances[shift - m_ShiftLeft] = score;
			if(score < min_score)
			{
				min_score = score;
				m_bestMatchScore = dist;

				ret = std::pair<int,int>(dist.x, dist.y); // return best result
			}
		}
	}
	else
	{
		int dststep;

		dststep = UpdateWith32bytesCushion(f1ptr,m1ptr);

		unsigned char *fs1ptr = m_temp[0];
		unsigned char *ms1ptr = m_temp[1];

		unsigned char *startfptr,*startmptr;

		startfptr = fs1ptr + 32 + abs(m_ShiftLeft);
		startmptr = ms1ptr + 32 + abs(m_ShiftLeft);

		for(int shift=m_ShiftLeft;shift<=m_ShiftRight;shift++)
		{
			CvPoint dist;

			dist = GetHammingDistance1(startfptr, startmptr, f2ptr, m2ptr,maskval,dststep);
			//printf("Dist Bits %d %d\n",dist.x,dist.y);
			startfptr +=-1;
			startmptr +=-1;

			double score = (dist.y > m_minCommonBits)? 1.0*dist.x/dist.y : 1.0;
			m_HammingDistances[shift - m_ShiftLeft] = score;
			if(score < min_score)
			{
				min_score = score;
				m_bestMatchScore = dist;

				ret = std::pair<int,int>(dist.x, dist.y); // return best result
			}
		}
	}
	return ret;
}


void EyeMatchServer::MakeShifts(unsigned char *f1ptr, unsigned char *m1ptr){
	for(int shift=m_ShiftLeft,i=0;shift<=m_ShiftRight;shift+=m_ShiftIncrement,i++){
		if(shift<0){
			LeftShiftFeatureVector(f1ptr, m_shiftedCode[i], shift);
			LeftShiftFeatureVector(m1ptr, m_shiftedMask[i], shift);
		}else if(shift>0){
			RightShiftFeatureVector(f1ptr, m_shiftedCode[i], shift);
			RightShiftFeatureVector(m1ptr, m_shiftedMask[i], shift);
		}
		else{
			memcpy(m_shiftedCode[i],f1ptr,m_featureLength);
			memcpy(m_shiftedMask[i],m1ptr,m_featureLength);
		}
	}
}
double EyeMatchServer::MatchWithShifts(unsigned char *f2ptr, unsigned char *m2ptr, unsigned int maskval)
{
	double min_score = 10.0;
	m_bestMatchScore = cvPoint(0, m_featureLength);

	for(int shift=m_ShiftLeft,i=0;shift<=m_ShiftRight;shift+=m_ShiftIncrement,i++)
	{
		CvPoint dist;
		dist = GetHammingDistanceNew(m_shiftedCode[i], m_shiftedMask[i], f2ptr, m2ptr,maskval,m_featureLength>>3);
		double score = (dist.y > m_minCommonBits)? 1.0*dist.x/dist.y : 1.0;
		m_HammingDistances[shift - m_ShiftLeft] = (float) score;
		if(score < min_score)
		{
			min_score = score;
			m_bestMatchScore = dist;
			m_bestShiftIndex = shift;
		}
	}
	min_score = 0.5-(0.5-min_score)*sqrt((double)m_bestMatchScore.y/m_nominalCommonBits);
	return min_score;
}


CvPoint EyeMatchServer::GetHammingDistanceNew(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char *f2ptr, unsigned char *m2ptr,unsigned int maskval,int step)
{
	int bits = 0;
	int dist = 0;

	unsigned int distarr[1024];
	unsigned int bitarr[1024];
	unsigned char *distptr,*bitptr;

	distptr = (unsigned char *)distarr;
	bitptr = (unsigned char *)bitarr;

	int len = m_featureLength/m_numRows;
		
	int k=0;
	unsigned int *m2=(unsigned int *)m2ptr,*f2=(unsigned int *)f2ptr;
	for(int i=0;i<m_numRows;i++){
		unsigned int *m1 = (unsigned int *)(m1ptr + step*i);
		unsigned int *f1 = (unsigned int *)(f1ptr + step*i);
		for(int zz=0;zz<len>>2;zz++){
			unsigned int temp = (m1[zz]) & (m2[k]) & (maskval);
			bitarr[k] = temp;
			distarr[k] = ((f1[zz]^f2[k]) & temp);
			k++;
		}
	}

	for(int zz=0;zz<m_featureLength;zz++){
		dist += m_lut[*distptr++];
		bits += m_lut[*bitptr++];
	}
	return cvPoint(dist, bits);
}


void EyeMatchServer::MakeShifts(unsigned char *f1ptr, unsigned char *m1ptr,unsigned int uiMask){

	unsigned int *maskint= (unsigned int *)m1ptr;
	unsigned int *temp1,*temp = (unsigned int *)m_maskTemp;
	temp1 = temp;

	uiMask=(uiMask<<24)|(uiMask<<16)|(uiMask<<8)|(uiMask);

	for(int i=0;i<m_featureLength>>2;i++)
	{
		*temp++ = *maskint++&uiMask;
	}

	for(int shift=m_ShiftLeft,i=0;shift<=m_ShiftRight;shift+=m_ShiftIncrement,i++){
		unsigned char *code = m_codeArray[i];
		unsigned char *mask = m_maskArray[i];

		if(shift<0){
			LeftShiftFeatureVector(f1ptr, code, shift);
			LeftShiftFeatureVector((unsigned char*)temp1, mask, shift);
		}else if(shift>0){
			RightShiftFeatureVector(f1ptr, code, shift);
			RightShiftFeatureVector((unsigned char*)temp1, mask, shift);
		}
		else{
			memcpy(code,f1ptr,m_featureLength);
			memcpy(mask,temp1,m_featureLength);
		}
	}
}


void EyeMatchServer::LeftShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift, int jump, int numRows)
{
	for(int kk = 0;kk< numRows;kk++){
		memcpy(dest + jump*(kk+1) - m_byteSize*abs(shift), src + jump*kk, m_byteSize*abs(shift));
		memcpy(dest + jump*kk, src + jump*kk + m_byteSize*abs(shift), jump - m_byteSize*abs(shift));
	}
}

void EyeMatchServer::RightShiftFeatureVector(unsigned char *src, unsigned char *dest, int shift, int jump, int numRows)
{
	for(int kk = 0;kk< m_numRows;kk++){
		memcpy(dest+jump*kk, src+jump*(kk+1)-m_byteSize*shift, m_byteSize*shift);
		memcpy(dest+jump*kk+m_byteSize*shift, src+jump*kk, jump - m_byteSize*shift);
	}
}


void EyeMatchServer::MakeShiftsInternal(unsigned char *codeTemp, unsigned char *maskTemp, int featureLength, int numRows, int shiftScale)
{
	int leftShift = abs(m_ShiftLeft/shiftScale);
	int rightShift = abs(m_ShiftRight/shiftScale);

	int maxShift = leftShift > rightShift ? leftShift : rightShift;
	for(int shift=1, j=1;shift<=maxShift;shift+=m_ShiftIncrement)
	{
		if(shift <= leftShift)
		{
			unsigned char *code = codeTemp + j*featureLength;
			unsigned char *mask = maskTemp + j*featureLength;

			LeftShiftFeatureVector((unsigned char*) codeTemp, code, -shift, featureLength/numRows, numRows);
			LeftShiftFeatureVector((unsigned char*) maskTemp, mask, -shift, featureLength/numRows, numRows);

			j++;
		}

		if(shift <= rightShift)
		{
			unsigned char *code = codeTemp + j*featureLength;
			unsigned char *mask = maskTemp + j*featureLength;

			RightShiftFeatureVector((unsigned char*) codeTemp, code, shift, featureLength/numRows, numRows);
			RightShiftFeatureVector((unsigned char*) maskTemp, mask, shift, featureLength/numRows, numRows);

			j++;
		}
	}
}

void EyeMatchServer::Compress_Shift(unsigned char *input, unsigned char *output, int rows, int cols)
{
	int inSize=rows*cols;
	int outSize=inSize>>1;	// 2 outputs of that size
	unsigned char *outPtr=output;

	//compressed
	{
		unsigned char *inPtr=(unsigned char *)input;
		for(int i=0;i<inSize;i=i+2)
		{
			unsigned char a= *(inPtr++);
			unsigned char b= *(inPtr++);
			*outPtr++ = ((a&0x0F)<<4) | ((b&0x0F));
		}
	}
	//4 bit rotated
	unsigned char *inPtr=(unsigned char *)output;
	cols=cols>>1;
	for(int r=0;r<rows;r++)
	{
		unsigned char row_carry=(*inPtr)>>4;
		unsigned char shifted=(*inPtr++)<<4;
		for(int c=1;c<cols;c++)
		{
			*(outPtr++)=(*inPtr)>>4 | shifted;
			shifted=(*inPtr++)<<4;

		}
		*(outPtr++)=shifted|row_carry;
	}
}

void printTempMask(unsigned char *templ,unsigned char *mask,int len){
	printf("CODE: \n");
	for(int m=0;m<len;m++){
		if((m)%80==0) printf("\n");
		printf("%02x",templ[m]);
	}
	printf("\n");
	printf("MASK: \n");
	for(int m=0;m<len;m++){
		if((m)%80==0) printf("\n");
		printf("%02x",mask[m]);
	}
	printf("\n");
}

void EyeMatchServer::MakeShifts_x0F(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask,void *scratch )
{
	*shiftedCode = (unsigned char *) scratch;
	*shiftedMask = *shiftedCode + m_featureLength*(m_ShiftRight/2 - m_ShiftLeft/2 + 1);

	// generates two half length iris codes, 1/2 byte shifted.
	Compress_Shift(f1ptr, *shiftedCode,m_numRows,m_featureLength/m_numRows);
	Compress_Shift(m1ptr, *shiftedMask,m_numRows,m_featureLength/m_numRows);
//	printf("Generate SHIFTS\n");
//	printTempMask(*shiftedCode,*shiftedMask,640);

	return MakeShiftsInternal(*shiftedCode, *shiftedMask, m_featureLength, 2*m_numRows, 2);
}

unsigned char* EyeMatchServer::GetIris(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	key += IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}

unsigned char* EyeMatchServer::GetMask(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON)+IRIS_SIZE;
	key += IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}


std::pair<int, float> EyeMatchServer::MatchDBNew(unsigned char *database, int numCodes,
		bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask) {
	unsigned int codeSize = m_featureLength << 1;
	double bestScore = 1.0;
	int bestIdx = -1;
	int jump = m_featureLength;
	unsigned char *f2ptr=NULL; // = templ;
	unsigned char *m2ptr=NULL; //= mask;

	f2ptr = (unsigned char*) malloc(codeSize); //=templ; //db
	m2ptr = (unsigned char*) malloc(codeSize); //=mask; //db

	jump = m_featureLength >> 1;

	int j = 0, k = 0;
	for(int shift = m_ShiftLeft, i = 0; shift <= m_ShiftRight; shift +=m_ShiftIncrement, i++) {
		double rScore = 1.0;
		int rindex = 0;
		double min_score = 10.0;
		CvPoint bestMatchScore = cvPoint(0, m_featureLength);
		if ((i & 0x1) == 0) {
			j = abs(m_ShiftLeft / m_ShiftIncrement) - k;
			k = k + 1;
		} else {
			j = abs(m_ShiftLeft / m_ShiftIncrement) + k;
		}
		unsigned char *icode = shiftedCode + j * jump; //insp
		unsigned char *imask = shiftedMask + j * jump; //insp

		for (int r = 0; r < numCodes;r++) {
			unsigned char *mask = GetMask(database,r);
			unsigned char *templ = GetIris(database,r);
			Compress_Shift(templ, f2ptr, m_numRows,	m_featureLength / m_numRows);
			Compress_Shift(mask, m2ptr, m_numRows, m_featureLength / m_numRows);
			CvPoint dist;
			dist = GetHammingDistance_globalOpt(icode, imask, f2ptr, m2ptr);
			double score =(dist.y > m_minCommonBits) ? 1.0 * dist.x / dist.y : 1.0;
			if (score < min_score) {
				min_score = score;
				bestMatchScore = dist;
				rindex = r;
			}
		}
		min_score = 0.5	- (0.5 - min_score)	* sqrt((double) bestMatchScore.y / m_nominalCommonBits);
		rScore = min_score;
		if ((rScore < hammingscore)) {
			bestScore = rScore;
			bestIdx = rindex;
			hammingscore = rScore;
//			printf("%d %f\n",bestIdx,bestScore);
			if(greedy)
				break;
		}
	}

	free(f2ptr);
	free(m2ptr);

	std::pair<int, float> result(-1, 1.0);
	result.first = bestIdx;
	result.second = bestScore;

	return result;
}

unsigned char* EyeMatchServer::GetCompressIris(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	key += COMPRESS_IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}

unsigned char* EyeMatchServer::GetCompressMask(unsigned char *DB,int eyenum){
	return GetCompressIris(DB,eyenum)+COMPRESS_IRIS_SIZE;
}


std::pair<int, float> EyeMatchServer::MatchDBNewCompress(unsigned char *database, int numCodes,
		bool greedy,float hammingscore,int featureMask,unsigned char *shiftedCode,unsigned char *shiftedMask) {
	double bestScore = 1.0;
	int bestIdx = -1;
	int jump = m_featureLength;
	jump = m_featureLength >> 1;

	int j = 0, k = 0;
	for(int shift = m_ShiftLeft, i = 0; shift <= m_ShiftRight; shift +=m_ShiftIncrement, i++) {
		double rScore = 1.0;
		int rindex = 0;
		double min_score = 10.0;
		CvPoint bestMatchScore = cvPoint(0, m_featureLength);
		if ((i & 0x1) == 0) {
			j = abs(m_ShiftLeft / m_ShiftIncrement) - k;
			k = k + 1;
		} else {
			j = abs(m_ShiftLeft / m_ShiftIncrement) + k;
		}
		unsigned char *icode = shiftedCode + j * jump; //insp
		unsigned char *imask = shiftedMask + j * jump; //insp
//		printf("%d Code amd Mask\n",i);
//		printTempMask(icode,imask,640);

		for (int r = 0; r < numCodes;r++) {
			unsigned char *mask = GetCompressMask(database,r);
			unsigned char *templ = GetCompressIris(database,r);
			CvPoint dist;
//			printf("%d DB Code amd Mask\n",r);
//			printTempMask(templ,mask,640);

			dist = GetHammingDistance_globalOpt(icode, imask, templ, mask);
			double score =(dist.y > m_minCommonBits) ? 1.0 * dist.x / dist.y : 1.0;
//			printf("%d %f\n",r,score);
			if (score < min_score) {
				min_score = score;
				bestMatchScore = dist;
				rindex = r;
			}
		}
		min_score = 0.5	- (0.5 - min_score)	* sqrt((double) bestMatchScore.y / m_nominalCommonBits);
//		printf("%d Score %f\n",i,min_score);
		rScore = min_score;
		if ((rScore < hammingscore)) {
			bestScore = rScore;
			bestIdx = rindex;
			hammingscore = rScore;
//			printf("%d %f\n",bestIdx,bestScore);
			if(greedy)
				break;
		}
	}

	std::pair<int, float> result(-1, 1.0);
	result.first = bestIdx;
	result.second = bestScore;

	return result;
}


// this assumes that Database has not been compressed.
// MA: We should create a version that works with compressed database
std::pair<int, float> EyeMatchServer::MatchDBRotation(unsigned char *f1ptr,unsigned char *m1ptr, unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask) {
	unsigned char *shiftedCode;
	unsigned char *shiftedMask;
	MakeShiftsForIris(f1ptr, m1ptr, &shiftedCode, &shiftedMask);
	return MatchDBNew(database,numCodes,greedy,hammingscore,featureMask,shiftedCode,shiftedMask);
}

void EyeMatchServer::MakeShiftsForIris(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask){
	MakeShifts_x0F(f1ptr, m1ptr,shiftedCode,shiftedMask,m_scratch);
}

std::pair<int, float> EyeMatchServer::MatchDBNewOpt(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask, unsigned char *shiftedCode,unsigned char *shiftedMask){
	return MatchDBNew(database,numCodes,greedy,hammingscore,featureMask,shiftedCode,shiftedMask);
}


void EyeMatchServer::PrintAllShift(){
	for(int shift=m_ShiftLeft,i=0;shift<=m_ShiftRight;shift+=m_ShiftIncrement,i++){
		CvPoint dist;
		printf("CODE: %d\n",shift);
		unsigned char *icode = m_codeArray[i]; //insp
		unsigned char *imask = m_maskArray[i]; //insp
		printf("CODE: %d\n",shift);
		for(int k=0;k<m_featureLength;k++){
			if((k+1)%160==0) printf("\n");
			printf("%02x",icode[k]);
		}
		printf("\n");
		printf("MASK: %d\n",shift);
		for(int k=0;k<m_featureLength;k++){
			if((k+1)%160==0) printf("\n");
			printf("%02x",imask[k]);
		}
	}
}

double EyeMatchServer::Match(unsigned char* f2ptr, unsigned char* m2ptr){
	double min_score = 10.0;
	CvPoint bestMatchScore = cvPoint(0, m_featureLength);
	int arrsz = m_ShiftRight-m_ShiftLeft +1;
	for(int i=0;i<arrsz;i++){
		CvPoint dist;
		unsigned char *icode = m_codeArray[i]; //insp
		unsigned char *imask = m_maskArray[i]; //insp

		dist = GetHammingDistance_global(icode,imask,f2ptr,m2ptr);
		//printf("%d[%d] -> total=>%d %d\n",i,m_featureLength,dist.x,dist.y);
		double score = (dist.y > m_minCommonBits)? 1.0*dist.x/dist.y : 1.0;
		if(score < min_score){
			min_score = score;
			bestMatchScore = dist;
		}
	}
	//printf(" min_score:%f  matchedBits: %d, m_nominalCommonBits: %f\n",min_score,bestMatchScore.y,m_nominalCommonBits);
	min_score = 0.5-(0.5-min_score)*sqrt((double)bestMatchScore.y/m_nominalCommonBits);
	return min_score;
}

CvPoint EyeMatchServer::GetHammingDistance_global(unsigned char *f1, unsigned char *m1, unsigned char *f2, unsigned char *m2)
{
	unsigned int bits = 0;
	unsigned int dist = 0;

#ifdef __ARM__
	uint8x16_t tmp,tmp1;
	uint8x16_t d,b;

	uint32x4_t bitsT = vmovq_n_u32(0);
	uint32x4_t ditsT = vmovq_n_u32(0);
	unsigned char test[16];
	for(int zz=0;zz<m_featureLength>>4;zz++,m1+=16,m2+=16,f1+=16,f2+=16){
		tmp = vld1q_u8((const uint8_t *)m1);
		tmp1 = vld1q_u8((const uint8_t *)m2);
		b = vandq_u8(tmp,tmp1);
		uint8x16_t cnt = vcntq_u8 (b);

		uint16x8_t bitSet8 = vpaddlq_u8 (cnt);
		uint32x4_t bitSet4 = vpaddlq_u16 (bitSet8);
		bitsT = vaddq_u32(bitsT, bitSet4);

		tmp = vld1q_u8((const uint8_t *)f1);
		tmp1 = vld1q_u8((const uint8_t *)f2);
		d = veorq_u8(tmp,tmp1);
		d = vandq_u8(d,b);
		cnt = vcntq_u8 (d);

		uint16x8_t bitSet8_1 = vpaddlq_u8 (cnt);
		uint32x4_t bitSet4_1 = vpaddlq_u16 (bitSet8_1);
		ditsT = vaddq_u32(ditsT, bitSet4_1);
	}

	int result = 0;
    uint64x2_t bitSet2 = vpaddlq_u32 (bitsT);
    result = vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2),0);
    result += vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2),2);
    bits = result;

    uint64x2_t bitSet2_1 = vpaddlq_u32 (ditsT);
    result = vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2_1),0);
    result += vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2_1),2);
    dist = result;
    return cvPoint(dist, bits);
#else
	int length = m_featureLength >> 2;
	unsigned int *f1ptr = (unsigned int *) f1;
	unsigned int *m1ptr = (unsigned int *) m1;
	unsigned int *f2ptr = (unsigned int *) f2;
	unsigned int *m2ptr = (unsigned int *) m2;

	for(int zz=0;zz<length;zz++)
	{
		unsigned int t1 = (*m1ptr++) & (*m2ptr++);
		unsigned int t2 = (*f1ptr++ ^ *f2ptr++) & t1;

		bits += m_lut16[t1 & 0xFFFF] + m_lut16[t1 >> 16];
		dist += m_lut16[t2 & 0xFFFF] + m_lut16[t2 >> 16];
	}
	return cvPoint(dist, bits);
#endif
}
CvPoint EyeMatchServer::GetHammingDistance_globalOpt(unsigned char *f1, unsigned char *m1, unsigned char *f2, unsigned char *m2)
{
	unsigned int bits = 0;
	unsigned int dist = 0;

#ifdef __ARM__
	uint8x16_t tmp,tmp1;
	uint8x16_t d,b;

	uint32x4_t bitsT = vmovq_n_u32(0);
	uint32x4_t ditsT = vmovq_n_u32(0);
	for(int zz=0;zz<m_featureLength>>5;zz++,m1+=16,m2+=16,f1+=16,f2+=16){
		tmp = vld1q_u8((const uint8_t *)m1);
		tmp1 = vld1q_u8((const uint8_t *)m2);
		b = vandq_u8(tmp,tmp1);
		uint8x16_t cnt = vcntq_u8 (b);

		uint16x8_t bitSet8 = vpaddlq_u8 (cnt);
		uint32x4_t bitSet4 = vpaddlq_u16 (bitSet8);
		bitsT = vaddq_u32(bitsT, bitSet4);

		tmp = vld1q_u8((const uint8_t *)f1);
		tmp1 = vld1q_u8((const uint8_t *)f2);
		d = veorq_u8(tmp,tmp1);
		d = vandq_u8(d,b);
		cnt = vcntq_u8 (d);

		uint16x8_t bitSet8_1 = vpaddlq_u8 (cnt);
		uint32x4_t bitSet4_1 = vpaddlq_u16 (bitSet8_1);
		ditsT = vaddq_u32(ditsT, bitSet4_1);
	}

	int result = 0;
    uint64x2_t bitSet2 = vpaddlq_u32 (bitsT);
    result = vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2),0);
    result += vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2),2);
    bits = result;

    uint64x2_t bitSet2_1 = vpaddlq_u32 (ditsT);
    result = vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2_1),0);
    result += vgetq_lane_s32 (vreinterpretq_s32_u64(bitSet2_1),2);
    dist = result;
    return cvPoint(dist, bits);
#else
	int length = m_featureLength >> 3;
	unsigned int *f1ptr = (unsigned int *) f1;
	unsigned int *m1ptr = (unsigned int *) m1;
	unsigned int *f2ptr = (unsigned int *) f2;
	unsigned int *m2ptr = (unsigned int *) m2;

	for(int zz=0;zz<length;zz++)
	{
		unsigned int t1 = (*m1ptr++) & (*m2ptr++);
		unsigned int t2 = (*f1ptr++ ^ *f2ptr++) & t1;

		bits += m_lut16[t1 & 0xFFFF] + m_lut16[t1 >> 16];
		dist += m_lut16[t2 & 0xFFFF] + m_lut16[t2 >> 16];
	}
	return cvPoint(dist, bits);
#endif
}
