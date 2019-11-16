/*
 * ShiftAndDcOffset.cpp
 *
 *  Created on: Feb 18, 2013
 *      Author: mamigo
 */

#include "ShiftAndDcOffset.h"
#include "GPIODriver.h"
#include <stdio.h>
//DSP_CPLD_GPIO_0 = PI8 -> GPIO 136
#define DSP_GPIO_FOR_SHIFT_3	139
#define DSP_GPIO_FOR_SHIFT_2	138
#define DSP_GPIO_FOR_SHIFT_1	137
#define DSP_GPIO_FOR_SHIFT_0	136

//PF8/PPI0D8 -> gpio 88
#define DSP_GPIO_FOR_SUB_D8  	88
#define DSP_GPIO_FOR_SUB_D9  	89
#define DSP_GPIO_FOR_SUB_D10	90
#define DSP_GPIO_FOR_SUB_D11	91

/*
Logic D11 D10 D9 D8 [0000 - 1111]
parameter SUB0 = 0;
parameter SUB1 = 200;
parameter SUB2 = 328;
parameter SUB3 = 456;
parameter SUB4 = 584;
parameter SUB5 = 712;
parameter SUB6 = 840;
parameter SUB7 = 968;
parameter SUB8 = 1096;
parameter SUB9 = 1224;
parameter SUB10 = 1352;
parameter SUB11 = 1480;
parameter SUB12 = 1608;
parameter SUB13 = 1736;
parameter SUB14 = 1864;
parameter SUB15 = 1992;

SHIFT_3 SHIFT_2 SHIFT_1 SHIFT_0 0000 - 1111
0 -> shift by 0
1 -> shift by 1
2 -> shift by 2
3 -> shift by 3
4 -> shift by 4

*/


ShiftAndDcOffset::ShiftAndDcOffset() {
	m_shiftValue=0;
	m_dcOffset=0;

	m_gpioShift[0] = new GPIODriver(DSP_GPIO_FOR_SHIFT_0);
	m_gpioShift[1] = new GPIODriver(DSP_GPIO_FOR_SHIFT_1);
	m_gpioShift[2] = new GPIODriver(DSP_GPIO_FOR_SHIFT_2);
	m_gpioShift[3] = new GPIODriver(DSP_GPIO_FOR_SHIFT_3);
	m_gpioOffset[0] =  new GPIODriver(DSP_GPIO_FOR_SUB_D8);
	m_gpioOffset[1] =  new GPIODriver(DSP_GPIO_FOR_SUB_D9);
	m_gpioOffset[2] =  new GPIODriver(DSP_GPIO_FOR_SUB_D10);
	m_gpioOffset[3] =  new GPIODriver(DSP_GPIO_FOR_SUB_D11);

	for(int i=0;i<4;i++){
		m_gpioShift[i]->SetGPIODirOut();
		m_gpioOffset[i]->SetGPIODirOut();
	}
}

ShiftAndDcOffset::~ShiftAndDcOffset() {
	for(int i=0;i<4;i++){
		if(m_gpioShift[i]) delete m_gpioShift[i];
		if(m_gpioOffset[i]) delete m_gpioOffset[i];
	}
}

void ShiftAndDcOffset::SetShiftAndOffsetValue(int shift,int dcoffset){
	m_shiftValue = shift;
	m_dcOffset = dcoffset;

	int sub = 0;
	if(m_dcOffset <= 200){
		sub=0;
	}else if((m_dcOffset > 200) && (m_dcOffset <= 328)){
		sub=1;
	}else if((m_dcOffset > 328) && (m_dcOffset <= 456)){
		sub=2;
	}else if((m_dcOffset > 456) && (m_dcOffset <= 584)){
		sub=3;
	}else if((m_dcOffset > 584) && (m_dcOffset <= 712)){
		sub=4;
	}else if((m_dcOffset > 712) && (m_dcOffset <= 840)){
		sub=5;
	}else if((m_dcOffset > 840) && (m_dcOffset <= 968)){
		sub=6;
	}else if((m_dcOffset > 968) && (m_dcOffset <= 1096)){
		sub=7;
	}else if((m_dcOffset > 1096) && (m_dcOffset <= 1224)){
		sub=8;
	}else if((m_dcOffset > 1224) && (m_dcOffset <= 1352)){
		sub=9;
	}else if((m_dcOffset > 1352) && (m_dcOffset <= 1480)){
		sub=10;
	}else if((m_dcOffset > 1480) && (m_dcOffset <= 1608)){
		sub=11;
	}else if((m_dcOffset > 1608) && (m_dcOffset <= 1736)){
		sub=12;
	}else if((m_dcOffset > 1736) && (m_dcOffset <= 1864)){
		sub=13;
	}else if((m_dcOffset > 1864) && (m_dcOffset <= 1992)){
		sub=14;
	}else if((m_dcOffset > 1992) ){
		sub=15;
	}
	m_sub = sub;
	SetToGPIOShiftAndOffset();
}

void ShiftAndDcOffset::SetToGPIOShiftAndOffset(){

	printf("Shift %d DcOffset %d \n",m_shiftValue,m_dcOffset);
	printf("Set shift ");
	for(int i=0;i<4;i++){
		int val = m_shiftValue&(1<<i);
		val = (val>>i);
		m_gpioShift[i]->SetGPIOValue(val);
		printf(" %d ",val);
	}
	printf("\n");

	printf("Set dc Offset ");
	for(int i=0;i<4;i++){
		int val = m_sub&(1<<i);
		val = (val>>i);
		m_gpioOffset[i]->SetGPIOValue(val);
		printf(" %d ",val);
	}
	printf("\n");
}
