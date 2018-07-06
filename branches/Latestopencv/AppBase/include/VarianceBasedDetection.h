/*
 * VarianceBasedDetection.h
 *
 *  Created on: Jun 1, 2011
 *      Author: developer1
 */

#ifndef VARIANCEBASEDDETECTION_H_
#define VARIANCEBASEDDETECTION_H_
#include <stdint.h>

enum VARIANCESTATE{VS_INITIAL=0,VS_SEARCHING};

class VarianceBasedDetection {
public:
	VarianceBasedDetection(float threshold,unsigned long timeout, int indx,unsigned long silenttime, int debug);
	virtual ~VarianceBasedDetection();
	bool Process(uint64_t tn,bool matched,float *var);
	void SetInitialState();
	void SetSearchingState();
	int GetState(){ return m_fVarState;}
	int GetVarienceIndx(){ return m_fVarIndex;}
	float GetVarianceMaxValue(){ return m_VarienceMaxValue;}
protected:
	float m_fVarThreshold;
	uint64_t m_fVarTimestamp,m_fVarTimeout,m_fVarSilentPeriod,m_fVarSilentTimeOut;
	int m_fVarState, m_fVarIndex;
	bool m_debug;
	float m_VarienceMaxValue;
};

#endif /* VARIANCEBASEDDETECTION_H_ */
