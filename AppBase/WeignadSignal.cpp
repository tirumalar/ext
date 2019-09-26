/*
 * WeignadSignal.cpp
 *
 *  Created on: 16-Apr-2010
 *      Author: madhav.shanbhag@mamigo.us
 */

#include "WeignadSignal.h"
#include "Configuration.h"
#include <iostream>
#include "I2CBus.h"
#include "Synchronization.h"
#include "F2FDispatcher.h"
#include "logging.h"
extern "C" {
#include "BobListener.h"
}
const char logger[30] = "WeignadSignal";

WeignadSignal::WeignadSignal(Configuration& conf) {
	m_clk1 = (char*)conf.getValue("GRITrigger.WeigandHiOutput", "PIN23-CONJ10");
	m_clk2 = (char*)conf.getValue("GRITrigger.WeigandLoOutput", "PIN3-CONJ23");

//	m_bitcount = conf.getValue("GRITrigger.WeigandNumBits",0);
	m_endstate = conf.getValue("GRITrigger.WeigandEndState",1);
	m_lowtime = conf.getValue("GRITrigger.WeigandLowTimeuSec",50);
	m_hightime = conf.getValue("GRITrigger.WeigandHighTimeuSec",1950);
	m_debug = conf.getValue("GRITrigger.WeigandDebug",false);
	m_bdebug= conf.getValue("GRITrigger.WeigandbDebug",false);
	m_oldWeigandSupport = conf.getValue("GRITrigger.OldWeigandSupport",false);
	// getValueIndex(const char *key, short min, short max, short defValue, ...);
	m_BoardType = (BoardType)conf.getValueIndex("GRITrigger.BoardType", (int)EyeswipeMini, (int)Eyelock, (int)EyeswipeMini, "EyeswipeMini","EyeswipeNano", "Eyelock");
	m_pac = conf.getValue("GRITrigger.PACEnable",false);
	m_wighid = conf.getValue("GRITrigger.WeigandHidEnable",false);
	m_WaitBobTime = conf.getValue("GRITrigger.WaitBobReplyTime",500);	// default 500ms
	m_wiegandLedIn = conf.getValue("GRITrigger.LedInEnable",false);
	m_transTOC = conf.getValue("GRITrigger.TransTOCMode", false);

	EyelockLog(logger, DEBUG, "WeigandSignal::WeigandSignal => %d ", (int)m_BoardType); fflush(stdout);

	EyelockLog(logger, DEBUG, "ClkLowTime %d ",m_lowtime);
	EyelockLog(logger, DEBUG, "ClkHighTime %d ",m_hightime);

}

void WeignadSignal::RestartSignal(){
	EyelockLog(logger, INFO, "Restart WeigandSignal ");
}

void WeignadSignal::CloseSignal(){
}

WeignadSignal::~WeignadSignal()
{
	CloseSignal();
}

int WeignadSignal::Config(void)
{
	return 0;
}

void WeignadSignal::Send(const char *data)
{
	std::string name = "WeignadSignal::Send()";
	unsigned int sendData = 1;
	m_bitcount = (int)((((unsigned int) data[1])<<8) + (unsigned char) data[0]);
	m_sleeptime = (int)((m_lowtime+m_hightime)*(m_bitcount+1));
	if(m_debug)
	{
		EyelockLog(logger, DEBUG, "Number of Bits %d ",m_bitcount);
		EyelockLog(logger, DEBUG, "Sleep Time %d ",m_sleeptime);
	}

	if(m_bitcount)
	{
		if(m_BoardType == EyeswipeNano)
		{
			int result = 0;
			char * string = (char *)data + 2;
			int bytes = (m_bitcount + 7) / 8;

			if(m_debug){
				EyelockLog(logger, DEBUG, " %s Writing data to I2C @ BobSetData ", name.c_str());
				PrintForDebug(string);
			}

			result = BobSendAcsData((void *)string, m_bitcount);
		    if(result != 0) {
		    	EyelockLog(logger, ERROR, "%s I2CBus Data Write failed @ BobSendAcsData ", name.c_str());
		    	fprintf(stderr, "%s I2CBus Data Write failed @ BobSendAcsData\n", name.c_str()); fflush(stdout); return;
		    }

		    if (m_transTOC)
		    	result = BobSetACSTypeBits(m_bitcount);

// old code
/*			result = BobSetData((void *)string, bytes);
		    if(result != 0) {
		    	EyelockLog(logger, ERROR, "%s I2CBus Data Write failed @ BobSetData ", name.c_str());
		    	fprintf(stderr, "%s I2CBus Data Write failed @ BobSetData\n", name.c_str()); fflush(stdout); return;
		    }

		    if (m_transTOC)
		    	result = BobSetACSTypeBits(m_bitcount);

		    result = BobSetDataLength(m_bitcount) || BobSetCommand(BOB_COMMAND_SEND);
		    if(result != 0) {
		    	EyelockLog(logger, ERROR, "%s I2CBus Command Write failed @ BobSetDataLength or BobSetCommand ", name.c_str());
		    	fprintf(stderr, "%s I2CBus Command Write failed @ BobSetDataLength or BobSetCommand\n", name.c_str()); fflush(stdout); return;
			}
*/
// end old code

		    if(m_debug)
		    	EyelockLog(logger, DEBUG, " %s Writing data to I2C Length %d", name.c_str(), m_bitcount);

			// wait max 1s
			int count = 10;
			while (count--) {
				usleep(100000);
				if (BoBGetCommand() == 0){
					//printf("\n$$$$$$$$$$ Command 0 at %d ms in wiegand $$$$$$$$$$\n ", ((10-count)*100));
					break;
				}
			}

			usleep(m_sleeptime);

		}
	}
}

