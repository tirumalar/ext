/*
 * F2FSignal.cpp
 *
 *  Created on: 16-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "F2FSignal.h"
#include "Configuration.h"
#include "I2CBus.h"
#include "Synchronization.h"
#include "logging.h"
#include <iostream>
extern "C" {
#include "BobListener.h"
}
const char logger[30] = "F2FSignal";

F2FSignal::F2FSignal(Configuration& conf) {
	float freq = conf.getValue("GRITrigger.F2FFreqHz",20000.0f);
	m_Timep = (int)((1000000/freq)+0.5);
	m_flag = (char*)conf.getValue("GRITrigger.F2FOutput","PIN30-CONJ10");	//Enable F2F Signal

	m_endstate = conf.getValue("GRITrigger.F2FEndState",0);
	m_debug = conf.getValue("GRITrigger.F2FDebug",false);
	// getValueIndex(const char *key, short min, short max, short defValue, ...);
	m_BoardType = (BoardType)conf.getValueIndex("GRITrigger.BoardType", (int)EyeswipeMini, (int)Eyelock, (int)EyeswipeMini, "EyeswipeMini","EyeswipeNano", "Eyelock");
	EyelockLog(logger, DEBUG, "ClkTime %d",m_Timep);
}


void F2FSignal::RestartSignal(){
	EyelockLog(logger, DEBUG, "Restart F2FSignal");
}

void F2FSignal::CloseSignal(){
}


F2FSignal::~F2FSignal() {
	CloseSignal();
}

int F2FSignal::Config(void){
}

void F2FSignal::Send(const char *data){

	std::string name = "F2FSignal::Send";

	m_bitcount = (int)((((unsigned int) data[1])<<8) + (unsigned char) data[0]);
	//Initialise the Bit count and buffer to be zero
	m_sleeptime = (int)(m_Timep*2*(m_bitcount+1));
	if(m_debug){
		EyelockLog(logger, DEBUG, "%s Num Bits %d", name.c_str(), m_bitcount);
		EyelockLog(logger, DEBUG, "%s Sleep Time %d", name.c_str(), m_sleeptime);
	}

	if(m_bitcount)
	{
		if(m_BoardType == EyeswipeNano)
		{
			int result = 0;
			const char * string = data + 2;
			int bytes = (m_bitcount + 7) / 8;

			if(m_debug){
				EyelockLog(logger, DEBUG, " %s Writing F2F data to I2C @ BobSetData ", name.c_str());
				//PrintForDebug(string);
			}


#if 0
			// Removed per Yiqing's request 11/2/2016
			// All keep alive things are handled by ICM code locally

			// 0xB 0xC 0x1 0xF 0x9
			// 00,D1,E1,F9,80
			char f2fData[5];
			f2fData[0] = 0x00;
			f2fData[1] = 0xD1;
			f2fData[2] = 0xE1;
			f2fData[3] = 0xF9;
			f2fData[4] = 0x80;
			if (m_debug) {
			printf("F2F fixed data: ");
			for (int x=0; x < 5; x++)
			{
				printf(" 0x%x", f2fData[x]);
			}
			printf("\n");
			}
			BobSetData((void *)f2fData, 5);
			result = BobSetDataLength(5) || BobSetCommand(BOB_COMMAND_SEND);

			sleep(1);
			//usleep(500000);
#endif
			result = BobSetData((void *)string, bytes);
		    if(result != 0) {
		    	EyelockLog(logger, ERROR, "%s I2CBus F2F Data Write failed @ BobSetData ", name.c_str());
		    	fprintf(stderr, "%s I2CBus F2F Data Write failed @ BobSetData\n", name.c_str()); fflush(stdout); return;
		    }

		    // Set F2F data length as 11 per Balu's request 1/29/15
		    result = BobSetDataLength(11) || BobSetCommand(BOB_COMMAND_SEND);
		    if(result != 0) {
		    	EyelockLog(logger, ERROR, "%s I2CBus Command Write failed @ BobSetDataLength or BobSetCommand ", name.c_str());
		    	fprintf(stderr, "%s I2CBus Command Write failed @ BobSetDataLength or BobSetCommand\n", name.c_str()); fflush(stdout); return;
			}

			int count = 10;
			while (count--) {
				usleep(100000);
				if (BoBGetCommand() == 0)
					break;
			}

			// set back the length 96 bits temporarily until ICM update data length correctly after read
			// result = BobSetDataLength(96);

			if (m_debug)
			{
				printf("F2F output data: ");
				for (int x=0; x < bytes; x++)
				{
					printf(" 0x%x", string[x]);
				}
				printf("\n");
			}
		}
		usleep(m_sleeptime);
	}
}




