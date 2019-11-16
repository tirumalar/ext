/*
 * MatchIrisDB.cpp
 *
 *  Created on: 26-Feb-2010
 *      Author: akhil
 */

#include "HDMatcher.h"
#include "EyeSegmentationInterface.h"
#include "BiOmega.h"
#include "IrisSelectServer.h"
#include "ReaderWriter.h"
#include "MatchResultHistory.h"
#include "IrisData.h"
#include "DBAdapter.h"
#include "UtilityFunctions.h"

#ifdef __BFIN__
#include <bfin_sram.h>
#endif
#include <fstream>

#include "logging.h"
const char logger[30] = "HDMatcher";

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
	void ScaleCodeLow(unsigned char *inp,unsigned char *out,int cnt);
	void ScaleCodeHigh(unsigned char *inp,unsigned char *out,int cnt);
}

HDMatcher::HDMatcher(int byteSize,int id,bool useCoarseFine,int featuremask)
:m_numIris(0),m_pMatchManagerInterface(0),m_startIndx(0),m_GreedyMatch(false),m_CoarseFineMatch(false),m_Threshold(-1.0f),
m_Shift(0),m_logResults(0),m_maxCorruptBitsPercAllowed(70),m_Lut(0),m_maskCode(0xFFFFFFFF),m_irisData(0),m_assigned(0),m_compressedMatching(false)
,m_pupilzz(18.0f),m_OutdoorMatching(false),m_OutdoorMatchThresh(0.25f)
{
	m_size = byteSize;
	m_ID = id;
	m_numIris = 0;
	m_CoarseFineMatch=useCoarseFine;
	m_CoarseIrisSize=0;
	m_nominalCommonBits=0;
	m_minCommonBitsFine=0;
	m_minCommonBitsCoarse=0;
	m_matchResultHistory=0;
	m_debug= false;
	m_Mask = NULL;
	m_Iris = NULL;
	m_pIrisMatchInterface = NULL;
	m_pIrisMatchInterfaceCoarse = NULL;
	m_CoarseBuff = NULL;
	m_Coarsemask = NULL;
	m_LowerNibble = true;
	m_irisData = new IrisData;
	m_FeatureMask = featuremask;
	memset(m_cardMatchName, 0, 100);
}

void HDMatcher::StartMatchInterface(int shift, bool greedymatch, float threshold, float coarseThreshold, float OutdoorMatchThresh,int irissz){
	printf("MatcherFeatureMask %#0x \n",m_maskCode&0xFF);
	m_GreedyMatch = greedymatch;
	m_Threshold = threshold;
	m_CoarseThreshold=coarseThreshold;
	m_Shift=shift;
	m_OutdoorMatchThresh=OutdoorMatchThresh;
	Init(irissz);
}

HDMatcher::~HDMatcher() {
	if(m_pIrisMatchInterface){
		m_pIrisMatchInterface->term();
		delete m_pIrisMatchInterface; m_pIrisMatchInterface = 0;
	}
	if(m_pIrisMatchInterfaceCoarse){
		m_pIrisMatchInterfaceCoarse->term();
		delete m_pIrisMatchInterfaceCoarse; m_pIrisMatchInterfaceCoarse=0;
	}
	if(m_CoarseBuff){
		free(m_CoarseBuff);
		m_CoarseBuff=0;
	}
	if(m_Coarsemask){
		free(m_Coarsemask);
		m_Coarsemask=0;
	}
	if(m_Lut){
		free(m_Lut);
		m_Lut=0;
	}
#ifdef __BFIN__
	if(m_Mask)
		sram_free(m_Mask);

	if(m_Iris)
		sram_free(m_Iris);
#else
	if(m_Mask)
		free(m_Mask);

	if(m_Iris)
		free(m_Iris);
#endif
}

void HDMatcher::Init(int irissz){
#ifdef __BFIN__
	if(m_Iris == NULL)
		m_Iris=(unsigned char *)sram_alloc(irissz,L1_DATA_SRAM);
	if(m_Mask == NULL)
		m_Mask=(unsigned char *)sram_alloc(irissz,L1_DATA_SRAM);
#else
	if(m_Iris == NULL)
		m_Iris = (unsigned char *)malloc(irissz);
	if(m_Mask == NULL)
		m_Mask = (unsigned char *)malloc(irissz);
#endif

	if(m_pIrisMatchInterface == NULL){
		m_pIrisMatchInterface = new IrisMatchInterface(irissz,8,1,m_Shift);
		m_pIrisMatchInterface->init();
		m_pIrisMatchInterface->SetNominalCommonBits(m_nominalCommonBits);
		m_pIrisMatchInterface->SetMinCommonBits(m_minCommonBitsFine);
	}

	if(m_CoarseFineMatch){
		m_CoarseIrisSize=irissz>>2;
		if(m_CoarseBuff == NULL)
			m_CoarseBuff=(unsigned char *)malloc(m_CoarseIrisSize);
		if(m_Coarsemask == NULL)
			m_Coarsemask=(unsigned char *)malloc(m_CoarseIrisSize);

		if(m_pIrisMatchInterfaceCoarse == NULL){
			m_pIrisMatchInterfaceCoarse=new IrisMatchInterface(m_CoarseIrisSize,8,1,(m_Shift+3)>>2);
			m_pIrisMatchInterfaceCoarse->init();
			m_pIrisMatchInterfaceCoarse->SetMinCommonBits(m_minCommonBitsCoarse);
		}
	}
}

static int number_of_ones(int num){
	int cnt = 0;
	while(num){
		cnt += num%2;
		num /= 2;
	}
	return cnt;
}

float  HDMatcher::CheckBitCorruptionPercentage(unsigned char* tag, int len, int* m_lut){
	int BitCount = 0;
	for(int ii = 0;ii<len;ii++){
		BitCount += (8 - m_lut[tag[ii]]);
	}
	//printf("BitCnt %d %5.2f%% \n",BitCount,(float)(BitCount*100.0f/(len<<3)));
	return(BitCount*100.0f/(len<<3));
}

void HDMatcher::SetCommonBits(int nominalCommonBits, int minCommonBitsFine, int minCommonBitsCoarse){
	m_nominalCommonBits=nominalCommonBits;
	m_minCommonBitsFine=minCommonBitsFine;
	m_minCommonBitsCoarse=minCommonBitsCoarse;

	m_Lut = (int*)malloc(sizeof(int)*256);
	for( int i = 0 ; i < 256 ; i++ ){
		m_Lut[i] = number_of_ones(i);
	}
}

int  HDMatcher::GetNumEyesPossibleInBuffer(){
	int numeye=0;
	if(m_compressedMatching){
		numeye = 2*GetSize()/(COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	}else{
		numeye= 2*GetSize()/(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	}
	return numeye;
}

unsigned char* HDMatcher::GetIris(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	key += IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}

unsigned char* HDMatcher::GetCompressIris(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	key += COMPRESS_IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}
unsigned char* HDMatcher::GetCoarseCompressIris(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON);
	key += COARSE_IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}


unsigned char* HDMatcher::GetMask(unsigned char *DB,int eyenum){
	unsigned char *key = DB + (eyenum>>1)*(IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON)+IRIS_SIZE;
	key += IRIS_SIZE_INCLUDING_MASK*(eyenum&0x1);
	return key;
}

unsigned char* HDMatcher::GetCoarseIris(unsigned char *DB,int eyenum){
	return DB+eyenum*(m_CoarseIrisSize<<1);
}

unsigned char* HDMatcher::GetCoarseMask(unsigned char *DB,int eyenum){
	return DB+eyenum*(m_CoarseIrisSize<<1)+m_CoarseIrisSize;
}



std::pair<int,float> HDMatcher::MatchIrisCodeExhaustiveCoarseFineDBCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB, CvPoint3D32f pupil){

	int FeatureMask = m_FeatureMask;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
	}

	EyelockLog(logger, DEBUG, "MatchIrisCodeExhaustiveCoarseFineDBCompressed.....pupilz %f FeatureMask %d\n", pupil.z, FeatureMask);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> best_scoreCoarse= std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	std::pair<int, float> scoreCoarse= std::make_pair(-1, 1);
	int irissz = 1280;
	int irisszby2 = irissz>>1;

	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + irissz;
	unsigned char *shiftedCode,*shiftedMask;
	m_pIrisMatchInterface->MakeShiftsForIris(refIrisCode,refMaskCode,&shiftedCode,&shiftedMask);

	XTIME_OP("COARSE",
		IrisMatchInterface::GetCoarseIrisCode(refIrisCode,refMaskCode,irissz,m_CoarseBuff,m_Coarsemask);
	);
	m_pIrisMatchInterfaceCoarse->MakeShifts(m_CoarseBuff,m_Coarsemask,0xFF);

	int k=0;
	if(m_debug)printf("(Coarse Score,Fine Score)\n");
	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			//Coarse
			unsigned char *coarseIrisCode = GetCoarseCompressIris(coarseDB,i);
			unsigned char *coarseMaskCode = coarseIrisCode +COARSE_IRIS_SIZE;
			scoreCoarse = m_pIrisMatchInterfaceCoarse->Match(coarseIrisCode, coarseMaskCode,FeatureMask);
			if(scoreCoarse.second < best_scoreCoarse.second){
				best_scoreCoarse = scoreCoarse;
				best_scoreCoarse.first = i;
			}
			if(scoreCoarse.second>m_CoarseThreshold) continue;
			k++;
			//Fine
			unsigned char *insIrisCode = GetCompressIris(DB,i);
			score = m_pIrisMatchInterface->MatchDBNewCompress(insIrisCode,1,m_GreedyMatch,1.0,FeatureMask,shiftedCode,shiftedMask);
			if(m_debug)printf("(%8.6f,%8.6f)\n",scoreCoarse.second,score.second);
			if(score.second < best_score.second){
				best_score = score;
				best_score.first = i;
			}
		}
	}
	if(m_debug)printf("Passed coarse score count %d\n",k);
	return best_score;
}

std::pair<int,float> HDMatcher::MatchIrisCodeExhaustiveDBCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB, CvPoint3D32f pupil){
	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> best_scoreCoarse= std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	std::pair<int, float> scoreCoarse= std::make_pair(-1, 1);
	int irissz = 1280;
	int irisszby2 = irissz>>1;

	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + irissz;
	unsigned char *shiftedCode,*shiftedMask;
	m_pIrisMatchInterface->MakeShiftsForIris(refIrisCode,refMaskCode,&shiftedCode,&shiftedMask);

	int FeatureMask = m_FeatureMask;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
	}

	EyelockLog(logger, DEBUG, "MatchIrisCodeExhaustiveDBCompressed.....pupilz %f FeatureMask %d\n", pupil.z, FeatureMask);

	int k=0;
	if(m_debug)printf("(Coarse Score,Fine Score)\n");
	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			//Fine
			unsigned char *insIrisCode = GetCompressIris(DB,i);
			unsigned char *insMaskCode = insIrisCode + COMPRESS_IRIS_SIZE;
			k++;
			score = m_pIrisMatchInterface->MatchDBNewCompress(insIrisCode,1,m_GreedyMatch,1.0,FeatureMask,shiftedCode,shiftedMask);
			if(score.second < best_score.second){
				best_score = score;
				best_score.first = i;
			}
		}
	}
	return best_score;
}



std::pair<int,float> HDMatcher::MatchIrisCodeExhaustiveCoarseFineDBUnCompressed(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB, CvPoint3D32f pupil){

	int FeatureMask = m_FeatureMask;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
		FeatureMask = 255;
	}
	EyelockLog(logger, DEBUG, "MatchIrisCodeExhaustiveCoarseFineDBUnCompressed.....pupilz %f FeatureMask %d\n", pupil.z, FeatureMask);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> best_scoreCoarse= std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	std::pair<int, float> scoreCoarse= std::make_pair(-1, 1);
	int irissz = 1280;
	int irisszby2 = irissz>>1;

	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + irissz;
	unsigned char *shiftedCode,*shiftedMask;
	m_pIrisMatchInterface->MakeShiftsForIris(refIrisCode,refMaskCode,&shiftedCode,&shiftedMask);

	XTIME_OP("COARSE",
		IrisMatchInterface::GetCoarseIrisCode(refIrisCode,refMaskCode,irissz,m_CoarseBuff,m_Coarsemask);
	);

	m_pIrisMatchInterfaceCoarse->MakeShifts(m_CoarseBuff,m_Coarsemask,0xFF);
//	m_pIrisMatchInterface->MakeShifts(refIrisCode, refMaskCode,m_maskCode);

	int k=0;
	if(m_debug)printf("(Coarse Score,Fine Score)\n");
	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			//Fine
			unsigned char *insIrisCode = GetIris(DB,i)+ 0;
			unsigned char *insMaskCode = GetMask(DB,i)+ 0;

			//Coarse
			unsigned char *coarseIrisCode = GetCoarseIris(coarseDB,0);
			unsigned char *coarseMaskCode = GetCoarseMask(coarseDB,0);
			IrisMatchInterface::GetCoarseIrisCode(insIrisCode,insMaskCode,irissz,coarseIrisCode,coarseMaskCode);
			scoreCoarse = m_pIrisMatchInterfaceCoarse->Match(coarseIrisCode, coarseMaskCode,FeatureMask);
			if(scoreCoarse.second < best_scoreCoarse.second){
				best_scoreCoarse = scoreCoarse;
				best_scoreCoarse.first = i;
			}
			if(scoreCoarse.second>m_CoarseThreshold) continue;
			k++;
//			score = m_pIrisMatchInterface->Match(insIrisCode, insMaskCode);
			score = m_pIrisMatchInterface->MatchDBNewOpt(insIrisCode,1,m_GreedyMatch,1.0,0xF,shiftedCode,shiftedMask);

			if(m_debug)printf("(%8.6f,%8.6f)\n",scoreCoarse.second,score.second);
			if(score.second < best_score.second){
				best_score = score;
				best_score.first = i;
			}
		}
	}
	if(m_debug)printf("Passed coarse score count %d\n",k);
	return best_score;
}


std::pair<int,float> HDMatcher::MatchIrisCode(unsigned char *IrisCode, unsigned char* DB, CvPoint3D32f pupil, unsigned char *coarseDB){
//#ifdef HBOX_PG 
	//PrintDB(DB); 
// #endif
	if(m_compressedMatching){
		//return m_pIrisMatchInterface->MatchDBRotation(IrisCode,IrisCode+1280,DB,m_numIris,m_GreedyMatch,1.0,0xF);
		if(m_CoarseFineMatch)
			return MatchIrisCodeExhaustiveCoarseFineDBCompressed(IrisCode,DB,coarseDB, pupil);
		else
			return MatchIrisCodeExhaustiveDBCompressed(IrisCode,DB,coarseDB, pupil);
	}
	if(m_CoarseFineMatch){
		if(m_GreedyMatch){
			return MatchIrisCodeGreedyCoarseFine(IrisCode,DB,coarseDB, pupil);
		}else{
			return MatchIrisCodeExhaustiveCoarseFine(IrisCode,DB,coarseDB, pupil);
		}
	}
	else{
		if(m_GreedyMatch){
			return MatchIrisCodeGreedy(IrisCode,DB, pupil);
		}else{
			return MatchIrisCodeExhaustive(IrisCode,DB, pupil);
		}
	}
}

void HDMatcher::PrintDB(unsigned char* DB){
#ifndef HBOX_PG
	FILE *filew = fopen("/mnt/mmc/DB.txt","w");
#else
	FILE *filew = fopen("DB.txt","w");
#endif 	
	for(int i=0;i<m_numIris;i++){
		unsigned char *insIrisCode = GetIris(DB,i);
		unsigned char *insMaskCode = GetMask(DB,i);

		fprintf(filew,"EyeIndex %d \n",i);
		fprintf(filew,"IRIS:\n",i);
		for(int cnt=0;cnt<1280;cnt++){
			if((cnt%64) == 0) fprintf(filew,"\n");
			fprintf(filew,"%#02x ",insIrisCode[cnt]);
		}
		fprintf(filew,"\n");

		fprintf(filew,"MASK:\n",i);
		for(int cnt=0;cnt<1280;cnt++){
			if((cnt%64) == 0) fprintf(filew,"\n");
			fprintf(filew,"%#02x ",insMaskCode[cnt]);
		}
		fprintf(filew,"\n");
	}
	fclose(filew);
}

static void SaveCode1( char *code) {
	char filename[512];
	static int index=0;
	sprintf(filename,"/mnt/sdcard/Eyelock/DBEye_%03d.bin",index++);
    std::ofstream output(filename, std::ios::binary);
        if (output.good()) {
                output.write(code, 2560);
        }
}


std::pair<int,float> HDMatcher::MatchIrisCodeExhaustive(unsigned char *IrisCode, unsigned char* DB, CvPoint3D32f pupil)
{
	int FeatureMask = m_FeatureMask;
	float matchThresh = m_Threshold;
	unsigned int maskCode = m_maskCode;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
		if(FeatureMask == 255){
			matchThresh = m_OutdoorMatchThresh;
			maskCode = (FeatureMask<<24)|(FeatureMask<<16)|(FeatureMask<<8)|(FeatureMask);
		}
	}

	EyelockLog(logger, DEBUG, "pupilzz %f FeatureMask %d matchThresh %f m_numIris %d", pupil.z, FeatureMask, matchThresh, m_numIris);
	// EyelockLog(logger, DEBUG, "MatchIrisCodeExhaustive.....pupilz %f FeatureMask %d  matchThresh %f m_numIris %d \n", pupil.z, FeatureMask, matchThresh, m_numIris);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
#ifdef MADHAV
	int irissz = m_IrisDBHeader->GetIrisSizeForMatcher();
#endif
	int irissz = 1280;

	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + irissz;

	FrameMatchResult *mrh = GetMatcherHistory();
	irissz= irissz>>1;
	m_pIrisMatchInterface->MakeShifts(refIrisCode, refMaskCode,maskCode);

	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			unsigned char *insIrisCode = GetIris(DB,i)+ 0;
			unsigned char *insMaskCode = GetMask(DB,i)+ 0;
			//SaveCode1((char *)insIrisCode);
			score = m_pIrisMatchInterface->Match(insIrisCode, insMaskCode,FeatureMask);
//			printf("Score %f \n",score.second);
			if(mrh){
				if(score.second < matchThresh){
					mrh->Append(i+m_startIndx,score.second);
					mrh->SetType(eMATCH);
				}
			}
			if(score.second < best_score.second){
				best_score = score;
				best_score.first = i;
			}
		}
	}
	if(mrh){
		if(mrh->GetType()==eNONE){
			mrh->Append(best_score.first,best_score.second);
			mrh->SetType(eNOMATCH);
		}
	}
	return best_score;
}


void HDMatcher::MatchIrisCodeExhaustiveNumDen(unsigned char *IrisCode, unsigned char* DB,int *buffer)
{
	std::pair<int, int> numden;
	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + 1280;

	for(int i=0;i<m_numIris;i++){
		numden.first = 1;
		numden.second = 1;
		if(CheckFromCorruptList(i)){
			unsigned char *insIrisCode = GetIris(DB,i);
			unsigned char *insMaskCode = GetMask(DB,i);
			numden = m_pIrisMatchInterface->match_pairNumDen(refIrisCode, refMaskCode, insIrisCode, insMaskCode);
		}
		int a = (numden.first);
		a |= (numden.second)<<16;
		buffer[i] = a;
		if(m_debug)printf("%d ,[%d,%d],%#08x\n",i,numden.first,numden.second,a);
	}
}

std::pair<int,float> HDMatcher::MatchIrisCodeGreedy(unsigned char *IrisCode, unsigned char* DB, CvPoint3D32f pupil)
{
	int FeatureMask = m_FeatureMask;
	float matchThresh = m_Threshold;
	unsigned int maskCode = m_maskCode;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
		if(FeatureMask == 255){
			matchThresh = m_OutdoorMatchThresh;
			maskCode = (FeatureMask<<24)|(FeatureMask<<16)|(FeatureMask<<8)|(FeatureMask);
		}
	}


	EyelockLog(logger, DEBUG, "MatchIrisCodeGreedy.....pupilz %f FeatureMask %d  matchThresh %f \n", pupil.z, FeatureMask, matchThresh);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + 1280;
	m_pIrisMatchInterface->MakeShifts(refIrisCode, refMaskCode,maskCode);

	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			unsigned char *insIrisCode = GetIris(DB,i);
			unsigned char *insMaskCode = GetMask(DB,i);
			score = m_pIrisMatchInterface->Match(insIrisCode, insMaskCode,FeatureMask);

			if(score.second < matchThresh){
				best_score = score;
				best_score.first = i;
				break;
			}
		}
	}
	return best_score;
}

std::pair<int,float> HDMatcher::MatchIrisCodeExhaustiveCoarseFine(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB, CvPoint3D32f pupil)
{
	int FeatureMask = m_FeatureMask;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
	}
	EyelockLog(logger, DEBUG, "MatchIrisCodeExhaustiveCoarseFine.....pupilz %f FeatureMask %d\n", pupil.z, FeatureMask);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> best_scoreCoarse= std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	std::pair<int, float> scoreCoarse= std::make_pair(-1, 1);
	int irissz = 1280;
	int irisszby2 = irissz>>1;

	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + irissz;
//	printf("%#x \n",m_maskCode);
	XTIME_OP("COARSE",
	IrisMatchInterface::GetCoarseIrisCode(refIrisCode,refMaskCode,irissz,m_CoarseBuff,m_Coarsemask);
	);

	m_pIrisMatchInterfaceCoarse->MakeShifts(m_CoarseBuff,m_Coarsemask,0xFF);
	m_pIrisMatchInterface->MakeShifts(refIrisCode, refMaskCode,m_maskCode);

	int k=0;
	if(m_debug)printf("(Coarse Score,Fine Score)\n");
	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			//Fine
			unsigned char *insIrisCode = GetIris(DB,i)+ 0;
			unsigned char *insMaskCode = GetMask(DB,i)+ 0;

			//Coarse
			unsigned char *coarseIrisCode = GetCoarseIris(coarseDB,0);
			unsigned char *coarseMaskCode = GetCoarseMask(coarseDB,0);
			IrisMatchInterface::GetCoarseIrisCode(insIrisCode,insMaskCode,irissz,coarseIrisCode,coarseMaskCode);

			//std::pair<int, float> scoreCoarse1 = m_pIrisMatchInterfaceCoarse->match_pair(m_CoarseBuff, m_Coarsemask, coarseIrisCode, coarseMaskCode);
			scoreCoarse = m_pIrisMatchInterfaceCoarse->Match(coarseIrisCode, coarseMaskCode,FeatureMask);
			//printf("%f %f\n",scoreCoarse1.second,scoreCoarse.second);

			if(scoreCoarse.second < best_scoreCoarse.second){
				best_scoreCoarse = scoreCoarse;
				best_scoreCoarse.first = i;
			}

			if(scoreCoarse.second>m_CoarseThreshold) continue;
			k++;

			//std::pair<int, float>score1 = m_pIrisMatchInterface->match_pair(refIrisCode, refMaskCode, insIrisCode, insMaskCode,m_maskCode);
			score = m_pIrisMatchInterface->Match(insIrisCode, insMaskCode,FeatureMask);
			
			//printf("%f %f\n",score1.second,score.second);

			if(m_debug)printf("(%8.6f,%8.6f)\n",scoreCoarse.second,score.second);
			if(score.second < best_score.second){
				best_score = score;
				best_score.first = i;
			}
		}
	}
	if(m_debug)printf("Passed coarse score count %d\n",k);
	return best_score;
}

std::pair<int,float> HDMatcher::MatchIrisCodeGreedyCoarseFine(unsigned char *IrisCode, unsigned char* DB,unsigned char *coarseDB, CvPoint3D32f pupil)
{
	int FeatureMask = m_FeatureMask;
	float matchThresh = m_Threshold;
	unsigned int maskCode = m_maskCode;
	if(m_OutdoorMatching){
		if(pupil.z <= m_pupilzz)
			FeatureMask = 255;
		if(FeatureMask == 255){
			matchThresh = m_OutdoorMatchThresh;
			maskCode = FeatureMask;
			maskCode = (FeatureMask<<24)|(FeatureMask<<16)|(FeatureMask<<8)|(FeatureMask);
		}
	}

	EyelockLog(logger, DEBUG, "MatchIrisCodeGreedyCoarseFine.....pupilz %f FeatureMask %d  matchThresh %f \n", pupil.z, FeatureMask, matchThresh);

	std::pair<int, float> best_score = std::make_pair(-1, 1);
	std::pair<int, float> score= std::make_pair(-1, 1);
	unsigned char *refIrisCode = IrisCode;
	unsigned char *refMaskCode = refIrisCode + 1280;

	XTIME_OP("COARSEG",
	IrisMatchInterface::GetCoarseIrisCode(refIrisCode,refMaskCode,1280,m_CoarseBuff,m_Coarsemask);
	);
	m_pIrisMatchInterfaceCoarse->MakeShifts(m_CoarseBuff,m_Coarsemask,0xFF);
	m_pIrisMatchInterface->MakeShifts(refIrisCode, refMaskCode,maskCode);

	for(int i=0;i<m_numIris;i++){
		if(CheckFromCorruptList(i)){
			//Fine
			unsigned char *insIrisCode = GetIris(DB,i);
			unsigned char *insMaskCode = GetMask(DB,i);
			//Coarse
			unsigned char *coarseIrisCode = GetCoarseIris(coarseDB,0);
			unsigned char *coarseMaskCode = GetCoarseMask(coarseDB,0);
			IrisMatchInterface::GetCoarseIrisCode(insIrisCode,insMaskCode,1280,coarseIrisCode,coarseMaskCode);

			score = m_pIrisMatchInterfaceCoarse->Match(coarseIrisCode, coarseMaskCode, FeatureMask);

			if(score.second>m_CoarseThreshold) continue;

			insIrisCode = GetIris(DB,i);
			insMaskCode = GetMask(DB,i);
			score = m_pIrisMatchInterface->Match(insIrisCode, insMaskCode, FeatureMask);

			if(score.second < matchThresh){
				best_score = score;
				best_score.first = i;
				break;
			}
		}
	}
	return best_score;
}

unsigned char* HDMatcher::GetF2FAndIDKey(unsigned char* DB,int indx){
	if(indx == -1) indx = m_startIndx;
	indx -= m_startIndx;
	indx = indx>>1;
#ifdef MADHAV
	unsigned char *key = DB + 4 + m_IrisDBHeader->GetIrisSize()*4 + indx*(m_IrisDBHeader->GetOneRecSizeinDB());
	return key;
#endif
}

bool HDMatcher::UpdateSingleUser(unsigned char* perid,unsigned char* leftiris,unsigned char* rightiris,unsigned char *DB,unsigned char*coarsedb){
	int found = -1;
	for(int i=0;i<GetNumEyes();i+=2){
		if(0 == memcmp(GetMatchGUID(DB,i).c_str(),perid,GUID_SIZE)){
			found = i;
			break;
		}
	}
	if(found != -1){
		UpdateBuffer(DB,coarsedb,found>>1,leftiris,rightiris,perid,false);
		return true;
	}
	return false;
}

unsigned char* HDMatcher::GetIrisFromDB(unsigned char *DB,int eyeindx){
	if(m_compressedMatching){
		return GetCompressIris(DB,eyeindx);
	}else{
		return GetIris(DB,eyeindx);
	}
}

unsigned char* HDMatcher::GetCoarseIrisFromDB(unsigned char *DB,int eyeindx){
	if((m_compressedMatching)&&(m_CoarseFineMatch)){
		return GetCoarseCompressIris(DB,eyeindx);
	}else{
		return NULL;
	}
}

void HDMatcher::PrintDBGUID(unsigned char *DB){
	for(int i=0;i<m_numIris;i+=2){
		printf("%d -> %s \n",i,GetMatchGUID(DB,i).c_str());
	}
}

bool HDMatcher::DeleteSingleUser(unsigned char *guid,unsigned char *DB, unsigned char *coarsedb){
//	printf("Before :: \n");PrintDBGUID(DB);
	int found = -1;
	for(int i=0;i<m_numIris;i+=2){
		if(0 == memcmp(GetMatchGUID(DB,i).c_str(),guid,GUID_SIZE)){
			found = i;
			break;
		}
	}
	if(found != -1){
		unsigned char *data = GetIrisFromDB(DB,found);
		unsigned char *cdata = GetCoarseIrisFromDB(coarsedb,found);
		int lastidx=m_numIris-2;

		if(lastidx<0) lastidx=0;
		unsigned char *last = GetIrisFromDB(DB,lastidx);
		unsigned char *clast = GetCoarseIrisFromDB(coarsedb,lastidx);

		if(m_compressedMatching){
			memcpy(data,last,COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
			memset(last,0,COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
		}else{
			memcpy(data,last,IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
			memset(last,0,IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
		}
		if((m_CoarseFineMatch) &&(clast) &&(cdata)){
			memcpy(cdata,clast,COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON);
			memset(clast,0,COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON);
		}
		m_numIris+=-2;
//		printf("After :: \n");PrintDBGUID(DB);
		return true;
	}
	return false;
}

void HDMatcher::UpdateBuffer(unsigned char *DB,unsigned char*coarsedb,int indx,unsigned char* left,unsigned char* right,unsigned char* guid,bool inc){
//	printf("Before :: \n");PrintDBGUID(DB);
	unsigned char buffer[IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON]={0};
	unsigned char cbuffer[COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON]={0};
	if((m_CoarseFineMatch)&& (coarsedb)){
		CreateCoarse(left,&cbuffer[0],IRIS_SIZE_INCLUDING_MASK);
		CreateCoarse(right,&cbuffer[COARSE_IRIS_SIZE_INCLUDING_MASK],IRIS_SIZE_INCLUDING_MASK);
	}
	if(m_compressedMatching){
		CompressIris(left,&buffer[0],IRIS_SIZE_INCLUDING_MASK);
		CompressIris(right,&buffer[COMPRESS_IRIS_SIZE_INCLUDING_MASK],IRIS_SIZE_INCLUDING_MASK);
		memcpy(&buffer[COMPRESS_IRIS_SIZE_INCLUDING_MASK_PER_PERSON],guid,GUID_SIZE);
	}else{
		memcpy(&buffer[0],left,IRIS_SIZE_INCLUDING_MASK);
		memcpy(&buffer[IRIS_SIZE_INCLUDING_MASK],right,IRIS_SIZE_INCLUDING_MASK);
		memcpy(&buffer[IRIS_SIZE_INCLUDING_MASK_PER_PERSON],guid,GUID_SIZE);
	}
	SetIrisAndGUID(DB,indx,buffer,coarsedb,cbuffer);
	if(inc){
		m_numIris+=2;
	}
	if(m_debug) {
		printf("After :: \n");PrintDBGUID(DB);
		PrintDB(DB);
	}

}



void HDMatcher::SetIrisAndGUID(unsigned char *DB,int indx,unsigned char* data,unsigned char *coarsedb,unsigned char*coarsedata){
	if(m_compressedMatching){
		memcpy(DB + COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON*indx,data,COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
	}else{
//		unsigned char *ptr= DB + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON*indx+IRIS_SIZE_INCLUDING_MASK_PER_PERSON;
		memcpy(DB + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON*indx,data,IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON);
//		printf("GUID %#08x-> %s\n",ptr,ptr);
	}
	if((m_CoarseFineMatch) &&(coarsedb) &&(coarsedata)){
		memcpy(coarsedb + COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON*indx,coarsedata,COARSE_IRIS_SIZE_INCLUDING_MASK_PER_PERSON);
	}
}

std::string HDMatcher::GetMatchGUID(unsigned char* DB,int indx){
	std::string test;
	test.resize(GUID_SIZE);
	if(indx == -1) return test;
	indx = indx>>1;
	if(m_compressedMatching){
		memcpy((void*)test.c_str(),DB + COMPRESS_IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON*indx + COMPRESS_IRIS_SIZE_INCLUDING_MASK_PER_PERSON,36);
	}else{
		memcpy((void*)test.c_str(),DB + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON*indx + IRIS_SIZE_INCLUDING_MASK_PER_PERSON,36);
	}
	return test;
}

char *HDMatcher::GetStatus()
{
	switch(m_Status)
	{
	case INIT:
		return "INIT";
	case REGISTERED:
		return "REGISTERED";
	case AVAILABLE:
		return "AVAILABLE";
	case BUSY:
		return "BUSY";
	case NOTAVAILABLE:
		return "NOTAVAILABLE";
	default:
		return "UNKNOWN";
	}
}

bool HDMatcher::isReadyForDB()
{
	switch(m_Status){
		case REGISTERED:
		case AVAILABLE:
			return true;
	}
	return false;

}

unsigned char * HDMatcher::extractCoarseDbRecord(unsigned char *personRec, unsigned char *coarseRec)
{
#ifdef MADHAV
	//left eye
	unsigned char *fineIrisCode = GetIris(personRec,0);
	unsigned char *fineMaskCode = GetMask(personRec,0);

	IrisMatchInterface::GetCoarseIrisCode(
			fineIrisCode,fineMaskCode,m_IrisDBHeader->GetIrisSize(),
			coarseRec,coarseRec+m_CoarseIrisSize);
	coarseRec+=(m_CoarseIrisSize<<1);

	//right eye
	fineIrisCode = GetIris(personRec,1);
	fineMaskCode = GetMask(personRec,1);
	IrisMatchInterface::GetCoarseIrisCode(
				fineIrisCode,fineMaskCode,m_IrisDBHeader->GetIrisSize(),
				coarseRec,coarseRec+m_CoarseIrisSize);

	coarseRec+=(m_CoarseIrisSize<<1);
#endif
	return coarseRec;
}

void HDMatcher::CheckCorruptBits(unsigned char *db,int i){
	int irissz = m_compressedMatching?COMPRESS_IRIS_SIZE:IRIS_SIZE;
	unsigned char *MaskCode = m_compressedMatching?GetCompressIris(db,i)+COMPRESS_IRIS_SIZE:GetIris(db,i)+IRIS_SIZE;
	float val = CheckBitCorruptionPercentage(MaskCode,irissz,m_Lut);
//	printf("%d -> %6.2f \n",i,val);
	if(val > m_maxCorruptBitsPercAllowed){
		m_CorruptedBitsList.push_back(i);
	}
}
bool HDMatcher::CheckFromCorruptList(int indx){
	for(int i=0;i< m_CorruptedBitsList.size();i++){
		if(indx == m_CorruptedBitsList.at(i))
			return false;
	}
	return true;
}

bool HDMatcher::InitializeDb(DBAdapter* dbAdapter,unsigned char *db, unsigned char *coarsedb){
	bool ret= false;
	if(dbAdapter){
		printf("Start Indx %d Num Eyes %d \n",GetStartIndx(),GetNumEyes());
		int r = -1;
		XTIME_OP("MakeMatchBuffer",
			r = dbAdapter->MakeMatchBuffer((char*)db,GetSize(),GetNumEyes()>>1,GetStartIndx()>>1,m_compressedMatching,(char*)coarsedb);
		);
		if(0 == r){
			m_Status = AVAILABLE;
		} else {
			printf("HDMLocal:: Unable to Read Data base \n");
			return false;
		}
		m_CorruptedBitsList.clear();
		for(int i=0;i<(GetNumEyes()>>1);i++){
			CheckCorruptBits(db,2*i);
			CheckCorruptBits(db,2*i+1);
		}
		printf("Corrupt List Size %d \n",m_CorruptedBitsList.size());
		if(m_debug){
			for(int i =0;i<m_CorruptedBitsList.size();i++){
				printf("%d ",m_CorruptedBitsList.at(i));
			}
			printf("\n");
		}
		ret = true;
	}
	return ret;
}

bool HDMatcher::InitializeDb(ReaderWriter* dbRdr,unsigned char *db, unsigned char *coarsedb){
//TODO: convert me to SQLITE3
#ifdef __MADHAV__
	GetIrisDBHeader()->PrintAll();
	dbRdr->Init(GetIrisDBHeader());
	//Skip by record no:
	int position= (GetStartIndx()>>1) * GetIrisDBHeader()->GetOneRecSizeinDBFile();
	printf("Start Indx %d Num Eyes %d \n",GetStartIndx(),GetNumEyes());

	//Clear if the list is there and start afresh..
	m_CorruptedBitsList.clear();

	int bytes = 0;
	for(int i=0;i<(GetNumEyes()>>1);i++)
	{
		bytes+=dbRdr->Read(db+i*GetIrisDBHeader()->GetOneRecSizeinDB(),
			GetIrisDBHeader()->GetOneRecSizeinDBFile(),position+i*GetIrisDBHeader()->GetOneRecSizeinDBFile());

		m_CorruptedBitsList.clear();
		CheckCorruptBits(db,2*i);
		CheckCorruptBits(db,2*i+1);
//		if(m_CoarseFineMatch)
//		{
//			coarsedb=extractCoarseDbRecord(db+i*GetIrisDBHeader()->GetOneRecSizeinDB(),coarsedb);
//		}
	}
//	if(m_debug) PrintDB(db);
	printf("Corrupt List Size %d \n",m_CorruptedBitsList.size());
	if(m_debug){
		for(int i =0;i<m_CorruptedBitsList.size();i++){
			printf("%d ",m_CorruptedBitsList.at(i));
		}
		printf("\n");
	}


	int tryRead = (GetNumEyes()>>1) * GetIrisDBHeader()->GetOneRecSizeinDBFile();
	if(bytes != tryRead){
		DeclareBad();
		return false;
	}
#endif

	return true;
}
