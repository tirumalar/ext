/*
 * MamigoFrameGrabber.c
 *
 *  Created on: 27 Sep, 2008
 *      Author: akhil
 */


#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include "Configuration.h"

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "bfin_ppi.h"

#ifdef __BFIN__
#include "pflags.h"
#include "ppifcd_i2c.h"
#endif

#include "MamigoFrameGrabber.h"
#include "ShiftAndDcOffset.h"


extern "C"{
#include "i2c.h"
#include "ppifcd_i2c_include.h"
#include "EdgeImage_private.h"
#include "file_manip.h"
}


#define I2C_DEVICE      "/dev/i2c-0"
#define PPI_DEVICE      "/dev/ppi0"
#define VERSION         "0.4"


#define MASTERCLOCK     48	//MHz
#define AVERAGE         100

void mydelay_inSecs(unsigned long delay){
	clock_t goal = delay * CLOCKS_PER_SEC + clock();
	while (goal > clock()) ;
}

void mydelay(unsigned long delay){
	clock_t goal = delay * CLOCKS_PER_SEC / 1000 + clock();
	while (goal > clock()) ;
}

#ifdef __BF537__
struct flagState{
	int pf33,pf34,pf36;
};
#endif

struct user_val{
	int binning;
	int skip;
	int verify;
	int width, height;
	int col_off, row_off;
	int board;			// 0 for BF537
	u_char addr;
//	u_short value, usetrigger, sendi2c; unused
	char *filename;
//	struct ioctl_pass_args icpa;
};

struct init_camera {
	int fd_standby, fd_led, fd_fs3;
};

struct ioctl_vals {
	int width, height,ppiControl,trigger_strobe;
};



struct camconfig getcamconfig(){

	struct camconfig config;
	config.row_start_val = 0x0036; //. Legal values: [0, 2004], even.

	config.column_start_val = 0x0010;

	config.row_size_val = 0x0797; // Legal values: [1, 2005], odd.

	config.column_size_val = 0x0A1F;// Legal values: [1, 2751], odd.

	config.horizontal_blank_val = 0x0000; // Legal values: [0, 4095].

	config.vertical_blank_val = 0x0019; // Legal values: [8, 2047].

	config.shutter_width_upper_val = 0x0000;

	config.shutter_width_lower_val = 0x0797;

	config.shutter_delay=0x0;

	config.restart_val = 0x0000;

	config.reset_val = 0x0000;

	config.read_mode1_val = 0x4006;

	config.read_mode2_val = 0x0040;

	config.row_address_mode_val = 0x0000;

	config.column_address_mode_val = 0x0000;

	config.green1_gain_val = 0x0008;

	config.red_gain_val = 0x0008;

	config.blue_gain_val = 0x0008;

	config.green2_gain_val = 0x0008;

	config.mirror_row = 0; // Should be 0 or 1

	config.mirror_column = 0;	// Should be 0 or 1

	config.row_bin = 0;  //Legal values 0 or 3

	config.row_skip = 0; //Legal values [0,7]

	config.column_bin = 0; //Legal values {0,1,3}

	config.column_skip = 0; // Legal values [0,6]

	config.green_digital_gain = 0 ; //Legal values [0,120]

	config.green_analog_gain = 8; // Legal values [8,63]

	config.green_analog_multiplier = 0; //Legal values {0,1}

	config.blue_digital_gain = 0 ; //Legal values [0,120]

	config.blue_analog_gain = 8; // Legal values [8,63]

	config.blue_analog_multiplier = 0; //Legal values {0,1}

	config.red_digital_gain = 0 ; //Legal values [0,120]

	config.red_analog_gain = 8; // Legal values [8,63]

	config.red_analog_multiplier = 0; //Legal values {0,1}

	config.enable_blc=1;
	config.black_level_target=0;
	config.row_black_default_offset=0;

	config.test_pattern_type=0;        //Legal values {0,4095}
	config.test_pattern_green=0;       //Legal values {0,4095}
	config.test_pattern_red=0;         //Legal values {0,4095}
	config.test_pattern_blue=0;        //Legal values {0,4095}
	config.test_pattern_bar_width=255; //Legal values {0,4095}, odd

	config.pixel_clock_control = 0;

	config.pll_control= 0x51;
	config.pll_config1 = 0x1403;
	config.pll_config2 = 3;
	config.output_control = 0x1F82;

	return config;
}


int MamigoFrameGrabber::update(unsigned char regis , unsigned short value){
return i2cWrite2ByteToByteAddress(I2C_DEVICE, DEVID,regis,value);
}

int MamigoFrameGrabber::reset_cam(void){
return i2cWrite2ByteToByteAddress(I2C_DEVICE, DEVID,RESET,1);
}

int MamigoFrameGrabber::restart_cam(void){
return i2cWrite2ByteToByteAddress(I2C_DEVICE, DEVID,RESTART,1);
}

int MamigoFrameGrabber::update_all(struct camconfig newconfig)
{
	//Check if everything validates...Else fail
	if(newconfig.row_start_val > 2004){printf("Wrong row start value"); exit(1);}
	if(newconfig.column_start_val > 2750){printf("Wrong column start value"); exit(1);}
	if(newconfig.row_size_val < 1){printf("Wrong row size value (too low)"); exit(1);}
	if(newconfig.row_size_val > 2005){printf("Wrong row size value (too high)"); exit(1);}
	if(newconfig.column_size_val < 1){printf("Wrong column size value (too low)"); exit(1);}
	if(newconfig.column_size_val > 2751){printf("Wrong column size value(too high)"); exit(1);}
	if(newconfig.horizontal_blank_val > 4095){printf("Wrong horizontal blank value (too high)"); exit(1);}
	if(newconfig.vertical_blank_val < 8){printf("Wrong vertical blank value (too low)"); exit(1);}
	if(newconfig.vertical_blank_val > 2047){printf("Wrong vertical blank value (too high)"); exit(1);}
	if(newconfig.mirror_row > 1){printf("Value of mirror_row should be either 0 or 1"); exit(1);}
	if(newconfig.mirror_column > 1){printf("Value mirror_column should be either 0 or 1"); exit(1);}
	if((newconfig.row_bin > 3) | (newconfig.row_bin==2)){printf("Value row_bin should be {0,1,3} "); exit(1);}
	if(newconfig.row_skip > 7){printf("Value row_skip should be less than 8"); exit(1);}
	if((newconfig.column_bin > 3) | (newconfig.column_bin==2)){printf("Value column_bin should be {0,1,3} "); exit(1);}
	if(newconfig.column_skip > 6){printf("Value column_skip should be less than 7"); exit(1);}
	if(newconfig.green_digital_gain > 120){printf("Value green_digital_gain should be less than 121"); exit(1);}
	if(newconfig.green_analog_multiplier >2){printf("Value green_analog_multiplier should be either 0 or 1"); exit(1);}
	if(((unsigned short)newconfig.green_analog_gain < 8) | ((unsigned short )newconfig.green_analog_gain >63)){printf("Value green_analog_gain should be in the range [8,63] "); printf("green analog gain : %i\n",newconfig.green_analog_gain); exit(1);  }
	if(newconfig.blue_digital_gain > 120){printf("Value blue_digital_gain should be less than 121"); exit(1);}
	if(newconfig.blue_analog_multiplier >2){printf("Value blue_analog_multiplier should be either 0 or 1"); exit(1);}
	if((newconfig.blue_analog_gain < 8) | (newconfig.blue_analog_gain >63)){printf("Value blue_analog_gain should be in the range [8,63] "); exit(1);}
	if(newconfig.red_digital_gain > 120){printf("Value red_digital_gain should be less than 121"); exit(1);}
	if(newconfig.red_analog_multiplier >2){printf("Value green_analog_multiplier should be either 0 or 1"); exit(1);}
	if((newconfig.red_analog_gain < 8) | (newconfig.red_analog_gain >63)){printf("Value red_analog_gain should be in the range [8,63] "); exit(1);}
	// ROW_START
	update(ROW_START,newconfig.row_start_val);
	// COLUMN_START
	if (newconfig.mirror_column){
		if(newconfig.column_bin==0) update(COLUMN_START,4*((newconfig.column_start_val)/4)+2);
		if(newconfig.column_bin==1) update(COLUMN_START,8*((newconfig.column_start_val)/8)+4);
		if(newconfig.column_bin==2) update(COLUMN_START,16*((newconfig.column_start_val)/16)+8);
	}else {
		if(newconfig.column_bin==0) update(COLUMN_START,4*((newconfig.column_start_val)/4));
		if(newconfig.column_bin==1) update(COLUMN_START,8*((newconfig.column_start_val)/8));
		if(newconfig.column_bin==2) update(COLUMN_START,16*((newconfig.column_start_val)/16));
	}

	//ROW_SIZE
	update(ROW_SIZE,newconfig.row_size_val);
	//COLUMN_SIZE
	update(COLUMN_SIZE,newconfig.column_size_val);
	//HORIZONTAL_BLANK
	update(HORIZONTAL_BLANK,newconfig.horizontal_blank_val);
	//VERTICAL_BLANK
	update(VERTICAL_BLANK,newconfig.vertical_blank_val);
	//PIX_CLK_CTRL
	update(PIX_CLK_CTRL,newconfig.pixel_clock_control);
	//SHUTTER_WIDTH_UPPER
	update(SHUTTER_WIDTH_UPPER,newconfig.shutter_width_upper_val);
	//SHUTTER_WIDTH_LOWER
	update(SHUTTER_WIDTH_LOWER,newconfig.shutter_width_lower_val);

	int blc=0;
	if(newconfig.enable_blc)blc=1;

	//READ_MODE2
	update(READ_MODE2,(newconfig.mirror_row<<15)+(newconfig.mirror_column<<14)+(blc<<6));
	//ROW_ADDRESS_MODE
	update(ROW_ADDRESS_MODE,(newconfig.row_bin<<4)+(newconfig.row_skip));
	//COLUMN_ADDRESS_MODE
	update(COLUMN_ADDRESS_MODE,(newconfig.column_bin<<4)+(newconfig.column_skip));
	// GREEN GAIN
	update(GREEN1_GAIN,(newconfig.green_digital_gain<<8)+(newconfig.green_analog_multiplier << 6) + (newconfig.green_analog_gain));
	update(GREEN2_GAIN,(newconfig.green_digital_gain<<8)+(newconfig.green_analog_multiplier << 6) + (newconfig.green_analog_gain));
	//BLUE GAIN
	update(BLUE_GAIN,(newconfig.blue_digital_gain<<8)+(newconfig.blue_analog_multiplier << 6) + (newconfig.blue_analog_gain));
		//RED GAIN
	update(RED_GAIN,(newconfig.red_digital_gain<<8)+(newconfig.red_analog_multiplier << 6) + (newconfig.red_analog_gain));
	//GLOBAL GAIN
	update(GLOBAL_GAIN,newconfig.global_gain_val);
	update(ROW_BLACK_DEF_OFFSET,newconfig.row_black_default_offset);//Row black default offset should always be zero
	update(ROW_BLACK_TARGET,newconfig.black_level_target);//Row black target
	update(BLC_CALIBRATION,blc);//blc
	// test Pattern related values
	update(TEST_PATTERN_TYPE,newconfig.test_pattern_type);

	if(newconfig.test_pattern_type){
		update(TEST_PATTERN_GREEN,newconfig.test_pattern_green);
		update(TEST_PATTERN_RED,newconfig.test_pattern_red);
		update(TEST_PATTERN_BLUE,newconfig.test_pattern_blue);
		update(TEST_PATTERN_BAR_WIDTH,newconfig.test_pattern_bar_width);
	}

	if(m_pllEnable)	{
		update(OUTPUT_CONTROL,newconfig.output_control&0xFFFB);
		update(PLL_CONTROL,newconfig.pll_control&0xFFFD);
		update(PLL_CONFIG_1,newconfig.pll_config1);
		update(PLL_CONFIG_2,newconfig.pll_config2);
		printf("Sleep %d usec\n",m_pllDelay);
		usleep(m_pllDelay);
		update(PLL_CONTROL,(newconfig.pll_control&0xFFFD)|2);
		update(OUTPUT_CONTROL,(newconfig.output_control&0xFFFB)|0x4);
	}
}

/*
 * Configure a MT9P031 sensor using ini file
 */
#define CAM_CONFIG( N, P ) camCfg->P = pCfg->getValue( #N , camCfg->P)
#define CAM_CONFIG_MT9P031(P) CAM_CONFIG( MT9P031.P, P )
void MamigoFrameGrabber::configure_MT9P031(Configuration *pCfg, struct camconfig *camCfg){
	CAM_CONFIG_MT9P031(row_start_val); //. Legal values: [0, 2004], even.
	CAM_CONFIG_MT9P031(column_start_val); //Legal values : [0,2750], even.
//	CAM_CONFIG_MT9P031(row_size_val); // Legal values: [1, 2005], odd.
//	CAM_CONFIG_MT9P031(column_size_val);// Legal values: [1, 2751], odd.
	CAM_CONFIG_MT9P031(horizontal_blank_val); // Legal values: [0, 4095].
	CAM_CONFIG_MT9P031(vertical_blank_val); // Legal values: [8, 2047].
	CAM_CONFIG_MT9P031(shutter_width_upper_val);
	CAM_CONFIG_MT9P031(shutter_width_lower_val);
	CAM_CONFIG_MT9P031(read_mode1_val);
	CAM_CONFIG_MT9P031(read_mode2_val);
	CAM_CONFIG_MT9P031(row_address_mode_val);
	CAM_CONFIG_MT9P031(column_address_mode_val);
	CAM_CONFIG_MT9P031(global_gain_val);
	CAM_CONFIG_MT9P031(green1_gain_val);
	CAM_CONFIG_MT9P031(red_gain_val);
	CAM_CONFIG_MT9P031(blue_gain_val);
	CAM_CONFIG_MT9P031(green2_gain_val);
	CAM_CONFIG_MT9P031(mirror_row);     // Should be 0 or 1
	CAM_CONFIG_MT9P031(mirror_column);	// Should be 0 or 1
//	CAM_CONFIG_MT9P031(row_bin);  //Legal values 0 or 3
//	CAM_CONFIG_MT9P031(row_skip); //Legal values [0,7]
//	CAM_CONFIG_MT9P031(column_bin); //Legal values {0,1,3}
//	CAM_CONFIG_MT9P031(column_skip); // Legal values [0,6]
	CAM_CONFIG_MT9P031(green_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT9P031(green_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT9P031(green_analog_multiplier); //Legal values {0,1}
	CAM_CONFIG_MT9P031(blue_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT9P031(blue_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT9P031(blue_analog_multiplier); //Legal values {0,1}
	CAM_CONFIG_MT9P031(red_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT9P031(red_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT9P031(red_analog_multiplier); //Legal values {0,1}

	CAM_CONFIG_MT9P031(enable_blc); // Legal Values {0,1}
	CAM_CONFIG_MT9P031(black_level_target); //[0,4095]
	CAM_CONFIG_MT9P031(row_black_default_offset); //[0,4095]
	CAM_CONFIG_MT9P031(test_pattern_type);
	CAM_CONFIG_MT9P031(test_pattern_green);
	CAM_CONFIG_MT9P031(test_pattern_red);
	CAM_CONFIG_MT9P031(test_pattern_blue);
	CAM_CONFIG_MT9P031(test_pattern_bar_width);

	//Madhav - Need to check what has to be done for eval board
	CAM_CONFIG_MT9P031(pixel_clock_control);
	//bit 15 - Invert pixel clk-> Should be 1 in Mamigo board and 0 in Eval
	//bit 0 - Clk Div factor /2  for face grab on mamigo board.
	camCfg->pixel_clock_control=(isEvalBoard)?(camCfg->pixel_clock_control&0x7FFE):(camCfg->pixel_clock_control|0x8000);

	m_pllEnable = pCfg->getValue( "MT9P031.pll_enable", m_pllEnable);
	m_pllDelay = pCfg->getValue( "MT9P031.pll_delay", m_pllDelay);

	CAM_CONFIG_MT9P031(pll_control);
	CAM_CONFIG_MT9P031(pll_config1);
	CAM_CONFIG_MT9P031(pll_config2);
	CAM_CONFIG_MT9P031(output_control);

}

void MamigoFrameGrabber::map_uservals_to_registers_MT9P031(struct user_val *uv, struct ioctl_vals *iv, Configuration *pCfg){

	struct camconfig newconfig = getcamconfig();

	newconfig.read_mode1_val = 0x8000;
	newconfig.global_gain_val = 0x8;

	newconfig.row_start_val=uv->row_off;
	newconfig.row_size_val= uv->height -1;
	newconfig.column_start_val = uv->col_off;
	newconfig.column_size_val= uv->width -1;
/*
	switch (uv->binning) {
	case 0:
		newconfig.row_bin = 0;
		newconfig.column_bin = 0;
		newconfig.row_skip = 0;
		newconfig.column_skip = 0;
		break;
	case 1:
		newconfig.row_bin = 1;
		newconfig.column_bin = 1;
		newconfig.row_skip = 1;
		newconfig.column_skip = 1;
		(*iv).height = uv->height / 2;
		(*iv).width = uv->width / 2;
		break;
	case 2:
		newconfig.row_bin = 3;
		newconfig.column_bin = 3;
		newconfig.row_skip = 3;
		newconfig.column_skip = 3;
		(*iv).height = uv->height / 4;
		(*iv).width = uv->width / 4;
		break;
	default:
		newconfig.row_bin = 0;
		newconfig.column_bin = 0;
		newconfig.row_skip = 0;
		newconfig.column_skip = 0;
		break;
	}
*/

	//Madhav Let me try writing the Binning Logic...
	//row_skip legal values [0,7] row_bin [0 3] , For full binning these values have to be identical
	//column_skip legal values [0,6] column_bin [0 1 3] ,For full binning these values have to be identical

	struct camconfig *camCfg = &newconfig;

	CAM_CONFIG_MT9P031(row_bin);  //Legal values 0 or 3
	CAM_CONFIG_MT9P031(row_skip); //Legal values [0,7]
	CAM_CONFIG_MT9P031(column_bin); //Legal values {0,1,3}
	CAM_CONFIG_MT9P031(column_skip); // Legal values [0,6]

	if(newconfig.row_bin){
		if(newconfig.row_skip < newconfig.row_bin){
			newconfig.row_skip = newconfig.row_bin;
			}
		}

	if(newconfig.column_bin){
		if(newconfig.column_skip < newconfig.column_bin){
			newconfig.column_skip = newconfig.column_bin;
		}
	}

	int divisorh = 1;
	switch (newconfig.row_skip) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		divisorh = 1<< newconfig.row_skip;
		break;
	default:
		newconfig.row_bin = 0;
		newconfig.row_skip = 0;
		break;
	}

	(*iv).height = uv->height / divisorh;

	int divisorw = 1;
	switch (newconfig.column_skip) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		divisorw = 1<< newconfig.column_skip;
		break;
	default:
		newconfig.column_bin = 0;
		newconfig.column_skip = 0;
		break;
	}

	(*iv).width = uv->width / divisorw;

	//Lets Update the Height and Width..

	(*iv).width = ((*iv).width>>4)<<4;//make multiple of 16
	(*iv).height = ((*iv).height>>4)<<4;//make multiple of 16

	int roiWidth = (*iv).width*divisorw;
	int roiHeight = (*iv).height*divisorh;

	uv->width = (*iv).width;
	uv->height = (*iv).height;


	newconfig.row_size_val= roiHeight -1;
	newconfig.column_size_val= roiWidth -1;

	printf("(X,Y,W,H)::(%d,%d,%d,%d)\n",newconfig.column_start_val,newconfig.row_start_val,newconfig.column_size_val,newconfig.row_size_val);

	printf("Row Bin %#x Skip %#x\n",newconfig.row_bin,newconfig.row_skip);
	printf("Col Bin %#x Skip %#x\n",newconfig.column_bin,newconfig.column_skip);
	printf("Width::User %d IntVec %d\n",uv->width,(*iv).width);
	printf("Height::User %d IntVec %d\n",uv->height,(*iv).height);


	if((newconfig.row_size_val+1+newconfig.row_start_val > (1944+54)) ||
    	(newconfig.column_size_val+1+newconfig.column_start_val > (2592+16))){
		printf("ROI in the Config is wrong...\n");
	}

	newconfig.shutter_width_lower_val = (*iv).height -1;

	//Allow a user modification
	if(pCfg) configure_MT9P031(pCfg, &newconfig);

// write this one as update_all does not write it. ppifcd_test.c is another user of the same function
	i2cWrite2ByteToByteAddress(I2C_DEVICE, DEVID, READ_MODE1, newconfig.read_mode1_val);

	update_all(newconfig);

	update_extra_regs((char *)pCfg->getValue( "MT9P031.extra_registers",""));
}


MamigoFrameGrabber::MamigoFrameGrabber(short sType):m_ImagePtr(NULL),m_shiftAndDcOffset(NULL),m_hw16to8(false){
	skipBytes=0;
	uv=new user_val;
	ic= new init_camera;
	ic->fd_standby=ic->fd_led=ic->fd_fs3=0;
	iv= new ioctl_vals;
	req=new frameRead;
//	flagstate = new flagState;
	m_pllEnable = 0;
	m_pllDelay =10000; //microsec
	
	SetImageBits(sType>8?16:8);
	printf("MT9P031 Configured %d \n",GetImageBits());
}
MamigoFrameGrabber::~MamigoFrameGrabber(){
	stop();
	term();
	delete uv;
	delete ic;
	delete iv;
	delete req;
	if(m_ImagePtr)
		free(m_ImagePtr);
	if(m_shiftAndDcOffset)
		delete m_shiftAndDcOffset;
}

void MamigoFrameGrabber::term()
{
}


void MamigoFrameGrabber::setDefaults(Configuration *pCfg){
	uv->binning =0; uv->skip =0;

	getPPIParams(uv->width,uv->height,iv->ppiControl);
	if(pCfg){
		uv->height=pCfg->getValue("MT9P031.row_size_val",uv->height-1);
		uv->width=pCfg->getValue("MT9P031.column_size_val",uv->width-1);
		uv->height++;
		uv->width++;
		printf("height %d\n",uv->height);
		printf("width %d\n",uv->width);
	}

	if((m_datatype != CFG_PPI_DATALEN_8)&&(m_ImagePtr == NULL))
		m_ImagePtr = (unsigned char *)malloc(uv->height*uv->width);


	uv->row_off=pCfg->getValue("MT9P031.row_start_val",54);
	uv->col_off=pCfg->getValue("MT9P031.column_start_val",16);
	uv->filename= '\0';
	uv->verify=-1;
	
	iv->width = uv->width;
	iv->height = uv->height;
}

void MamigoFrameGrabber::init(Configuration *pCfg)
{
	int ppibits = pCfg->getValue("MT9P031.Bits",8);
	m_DcOffset = (unsigned short)pCfg->getValue("GRI.dc", (int)0);
	m_ShiftRight = (unsigned short)pCfg->getValue("GRI.shiftRight", (int)0);
	int numbits = pCfg->getValue("Eyelock.NumBits",8);
	m_numbits = numbits > 8?16:numbits;

	printf("MT9P031 Configured %d \n",ppibits);
	m_hw16to8 = pCfg->getValue("MT9P031.Hw16To8",false);
	if(m_hw16to8){
		if(!m_shiftAndDcOffset) m_shiftAndDcOffset = new ShiftAndDcOffset();
		m_shiftAndDcOffset->SetShiftAndOffsetValue(m_ShiftRight,m_DcOffset);
	}

	if(ppibits == 8)
		m_datatype = CFG_PPI_DATALEN_8;
	else if(ppibits == 12)
		m_datatype = CFG_PPI_DATALEN_12;
	else if(ppibits == 16)
		m_datatype = CFG_PPI_DATALEN_16;
	else
		m_datatype = CFG_PPI_DATALEN_8;

	isEvalBoard=pCfg->getValue("test.IsEvalBoard",false);
	isEyelockBoard = pCfg->getValue("test.IsEyelockBoard",false);
	initialI2CAddress=(isEvalBoard)?(0x73):(0x72>>1);
	initialI2CValue=(isEvalBoard)?(0x1):(0x0);

	unsigned char devID = pCfg->getValue("MT9P031.Address",0x90);
	printf("Micron DevID %#x \n",devID);
	DEVID=(devID>>1); //0x90>>1

	setDefaults(pCfg);
	map_uservals_to_registers_MT9P031(uv, iv,pCfg);
}

void MamigoFrameGrabber::exec_ioctls(struct ioctl_vals *iv, int fd){
	ioctl(fd,CMD_PPI_SKIPPING, CFG_PPI_SKIP_DISABLE);
	ioctl(fd,CMD_PPI_PACKING,  CFG_PPI_PACK_ENABLE);
	ioctl(fd,CMD_PPI_DATALEN,  m_datatype);

	ioctl(fd,CMD_PPI_FSACTIVE_SELECT,0);
	ioctl(fd,CMD_PPI_PORT_CFG, CFG_PPI_PORT_CFG_XSYNC23);
	ioctl(fd,CMD_PPI_XFR_TYPE, CFG_PPI_XFR_TYPE_NON646);
	ioctl(fd,CMD_PPI_CLK_EDGE,CFG_PPI_CLK_EDGE_FALL);//POLC try all 00-11   POLS11 and POLC 00 gave error

	ioctl(fd,CMD_PPI_TRIG_EDGE, CFG_PPI_TRIG_EDGE_RISE);
	ioctl(fd,CMD_PPI_SET_DIMS, CFG_PPI_DIMS_2D);
	ioctl(fd,CMD_PPI_LINELEN, iv->width );
	ioctl(fd,CMD_PPI_NUMLINES, iv->height);
	ioctl(fd,CMD_PPI_FIFOUWM, 0x00);
	ioctl(fd,CMD_PPI_FIFORWM, 0x00);
	ioctl(fd,CMD_PPI_DELAY, 0);
}


bool MamigoFrameGrabber::start(bool bStillFrames)
{
	this->bStillFrames=bStillFrames;
	/* Open /dev/ppi */
	fd = open(PPI_DEVICE, O_RDONLY, 0);
	if (fd == -1) {
		printf("Could not open %s : %d \n", PPI_DEVICE,errno);
		return false;
	}
	//Execute IOCTLs
	exec_ioctls(iv, fd);

	//prepare read request
	req->isBlocking=1;
	req->isLatest=1;
	req->minIdx=0;
	DMA();
	return true;
}

void MamigoFrameGrabber::DMA(void){
	ioctl(fd,CMD_SET_PIXEL_PER_LINE, iv->width);
	ioctl(fd,CMD_SET_LINES_PER_FRAME, iv->height);
	ioctl(fd,CMD_SETUP_VIDEO_STREAM, 0x6000000);
}


char* MamigoFrameGrabber::getLatestFrame_raw(){
	int rc=ioctl(fd, CMD_READFRAME_VIDEO_STREAM, (unsigned long)(req));
	CURR_TV_AS_USEC(t);
	m_ts = t;
	if(rc) return 0; // rc should be zero normally
	m_frameIndex = req->actualIdx;
	req->minIdx=req->actualIdx+1;	// for next fetch
	if(m_numbits == 8){
		if((m_datatype != CFG_PPI_DATALEN_8)&&(!m_hw16to8)) {
			if(m_ImagePtr){
				XTIME_OP("16To8",
					Convert16to8_ASM((unsigned short* )(req->start_addr+skipBytes),m_ImagePtr,iv->width*iv->height,	m_DcOffset,m_ShiftRight)
				);
			}else{
				printf("Check the ini for hw16to8 and image bits\n");
			}
			return (char*)m_ImagePtr;
		}
	}
	return (char* )(req->start_addr+skipBytes);
}

bool MamigoFrameGrabber::stop()
{
	if(fd){
		// teardown the video stream
		if(!bStillFrames)
			ioctl(fd, CMD_TEARDOWN_VIDEO_STREAM, 0);
		/* Close PPI */
		close(fd);
	}
	fd=0;
	return true;
}

void MamigoFrameGrabber::getDims(int& width, int& height) const{
		width=uv->width;
		height=uv->height;
}

void MamigoFrameGrabber::getPPIParams(int& pixels_per_line, int& lines_per_frame, int& ppiControl) const{
	pixels_per_line=2592;
	lines_per_frame=1944;
}

bool MamigoFrameGrabber::isBayer() const {
	return (true);
}
bool MamigoFrameGrabber::isITU656Source() const {
	return (false);
}

size_t MamigoFrameGrabber::readFrame(void *buf, size_t count){
	if(!fd){
		printf("device not opened yet!\n");
		return 0;
	}
	return read(fd, (void*)(0x6000000) , uv->height * uv->width);
}
