/*
 * EyelockWrap.h
 *
 *  Created on: Apr 20, 2011
 *      Author: developer1
 */

#ifndef EYELOCKWRAP_H_
#define EYELOCKWRAP_H_

#ifdef __cplusplus
	extern "C" {
#endif

	//API for enrollment application
	void EyelockInit(char *iniFileFullPath);
	int GetIrisCode(unsigned char *imageBuffer, char *Iriscode, float *robustFeatures);
	float MatchPair(unsigned char *ins, unsigned char *ref);

	void SetDoSaveDetections(int save);
	void EnableEyelidMask(int value);
	void GetLEDSpot(int *xy);
	int GetConfParam(const char *name);

	//following functions are under testing
	void EyelockProcess(char *img, int *indx, float *score);
	void EyelockExit();
	int DoMatch(float *score, char **person);
	int DoTestMatch(unsigned char *inscode, float *score, char **person);
	int AppendDB(char *ptr1,char* ptr2,char* fname, char* file);
	int StoreBestPairOfEyes(const char *personName);
	int SaveBestPairOfEyes(const char *personName);
	unsigned char *GetEnrollmentEye(int index);
	int FindEyes(char *ptr);
	void SaveEyes(int idx);
	void FlushAllEyes();
	char *LoadFile(char *name, int size, int idx);
	void EyelockDbUpdate(char* dbptr,char* key);
	void EyelockReloadDB();
	int DecodeJPEG(unsigned char *out_luma, int *width, int *height,unsigned char *compBuffer, int compLen);


	void DecodeInit();
	void DecodeExit();
	int DecodeJpeg(unsigned char *jpegBuffer,int length ,unsigned char *outBuffer,int output_format, int *width, int *height);
	char* GetString(char* key);
	int GetInt(char* key);
	void ConfInit(char *str);
	void ConfExit();
#ifdef __cplusplus
	}
#endif

#endif /* EYELOCKWRAP_H_ */
