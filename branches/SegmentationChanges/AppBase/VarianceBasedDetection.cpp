/*
 * VarianceBasedDetection.cpp
 *
 *  Created on: Jun 1, 2011
 *      Author: developer1
 */

#include "VarianceBasedDetection.h"
#include <stdio.h>

VarianceBasedDetection::VarianceBasedDetection(float threshold,unsigned long vartimeout, int indx,unsigned long silenttimeout, int debug )
:m_fVarThreshold(threshold),m_fVarTimeout(vartimeout),m_fVarState(VS_INITIAL),m_fVarTimestamp(0),m_fVarIndex(indx)
,m_fVarSilentTimeOut(0),m_VarienceMaxValue(0.0f){
	m_fVarSilentPeriod = silenttimeout;
	m_debug=debug;
	if(m_debug){
		printf("Var Threshold %f\n",m_fVarThreshold);
		printf("Var Indx %d\n",m_fVarIndex);
		printf("State: %s \n",m_fVarState?"VS_SEARCHING":"VS_INITIAL");
		printf("Var Timeout %llu\n",m_fVarTimeout);
		printf("Var Silent Timeout %llu\n",m_fVarSilentPeriod);
	}
}

VarianceBasedDetection::~VarianceBasedDetection() {
	// TODO Auto-generated destructor stub
}

void VarianceBasedDetection::SetInitialState(){
	m_fVarState = VS_INITIAL;
	m_fVarTimestamp = 0;
}

void VarianceBasedDetection::SetSearchingState(){
	m_fVarState = VS_SEARCHING;
}

bool VarianceBasedDetection::Process(uint64_t tn,bool matched,float *var){
	bool retval = false;
	bool check = true;
	if(matched){
		SetInitialState();
	}else{
		if(!m_fVarSilentTimeOut)
			m_fVarSilentTimeOut = tn + m_fVarSilentPeriod;

		if(!m_fVarTimestamp)
			m_fVarTimestamp = tn + m_fVarTimeout;

		if(m_debug)	printf("%llu State: %s \n",tn,m_fVarState?"VS_SEARCHING":"VS_INITIAL");
		if(m_fVarState == VS_INITIAL){
			if(m_debug)	printf("var[m_fVarIndex] < m_fVarThreshold %f < %f \n",var[m_fVarIndex],m_fVarThreshold);
			if(var[m_fVarIndex] < m_fVarThreshold){
				SetInitialState();
				check = false;
			}
			//Doesn't matter as it will come when the state in initial
			m_VarienceMaxValue = var[m_fVarIndex];
		}

		if(m_debug)	printf("m_VarienceMaxValue < var[m_fVarIndex] %f > %f \n",m_VarienceMaxValue ,var[m_fVarIndex]);
		if(m_VarienceMaxValue < var[m_fVarIndex])
			m_VarienceMaxValue = var[m_fVarIndex];


		if(m_debug)	printf("tn > m_fVarSilentTimeOut %llu > %llu \n",tn,m_fVarSilentTimeOut);
		if((check)&&(tn > m_fVarSilentTimeOut)){
			SetInitialState();
			//m_fVarTimestamp = tn+m_fVarTimeout;
			check = false;
		}

		if(m_debug)	printf("tn < m_fVarTimestamp %lu < %lu \n",tn,m_fVarTimestamp);
		if((check)&&(tn < m_fVarTimestamp)){
			SetSearchingState();
			check = false;
		}

		if(check){
			SetInitialState();
			retval = true;
		}
	}
	m_fVarSilentTimeOut = tn + m_fVarSilentPeriod;
	if(m_debug){
		printf("m_fVarSilentTimeOut %llu\n",m_fVarSilentTimeOut);
		printf("%s\n",retval?"TRUE":"FALSE");
	}

	return retval;
}
