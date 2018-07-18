/*
 * CamConfig.cpp
 *
 *  Created on: Jan 10, 2011
 *      Author: developer1
 */

#include "CamConfig.h"

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

#include "ppifcd_i2c.h"
#include "FrameGrabber.h"
#include "Configuration.h"



CameraConfig::CameraConfig() {
	CameraParam = NULL;
}

CameraConfig::~CameraConfig() {
	if(CameraParam){
		delete CameraParam;
		CameraParam =0;
	}
}

camconfig* CameraConfig::Getnewcamconfig(){

	camconfig *config = new camconfig;

	config->row_start_val = 0x0036; //. Legal values: [0, 2004], even.

	config->column_start_val = 0x0010;

	config->row_size_val = 0x0797; // Legal values: [1, 2005], odd.

	config->column_size_val = 0x0A1F;// Legal values: [1, 2751], odd.

	config->horizontal_blank_val = 0x0000; // Legal values: [0, 4095].

	config->vertical_blank_val = 0x0019; // Legal values: [8, 2047].

	config->shutter_width_upper_val = 0x0000;

	config->shutter_width_lower_val = 0x0797;

	config->restart_val = 0x0000;

	config->reset_val = 0x0000;

	config->read_mode1_val = 0x4006;

	config->read_mode2_val = 0x0040;

	config->row_address_mode_val = 0x0000;

	config->column_address_mode_val = 0x0000;

	config->green1_gain_val = 0x0008;

	config->red_gain_val = 0x0008;

	config->blue_gain_val = 0x0008;

	config->green2_gain_val = 0x0008;

	config->mirror_row = 0; // Should be 0 or 1

	config->mirror_column = 0;	// Should be 0 or 1

	config->row_bin = 0;  //Legal values 0 or 3

	config->row_skip = 0; //Legal values [0,7]

	config->column_bin = 0; //Legal values {0,1,3}

	config->column_skip = 0; // Legal values [0,6]

	config->green_digital_gain = 0 ; //Legal values [0,120]

	config->green_analog_gain = 8; // Legal values [8,63]

	config->green_analog_multiplier = 0; //Legal values {0,1}

	config->blue_digital_gain = 0 ; //Legal values [0,120]

	config->blue_analog_gain = 8; // Legal values [8,63]

	config->blue_analog_multiplier = 0; //Legal values {0,1}

	config->red_digital_gain = 0 ; //Legal values [0,120]

	config->red_analog_gain = 8; // Legal values [8,63]

	config->red_analog_multiplier = 0; //Legal values {0,1}

	config->enable_blc=1;
	config->black_level_target=0;
	config->row_black_default_offset=0;

	config->test_pattern_type=0;        //Legal values {0,4095}
	config->test_pattern_green=0;       //Legal values {0,4095}
	config->test_pattern_red=0;         //Legal values {0,4095}
	config->test_pattern_blue=0;        //Legal values {0,4095}
	config->test_pattern_bar_width=255; //Legal values {0,4095}, odd

	config->pixel_clock_control = 0;

	config->pll_control= 0x51;
	config->pll_config1 = 0x1403;
	config->pll_config2 = 3;
	config->output_control = 0x1F82;

	config->pll_enable = 0;
	config->pll_delay = 0;

	config->shutter_delay =0;
	return config;
}


#define CAM_CONFIG( N, P ) camCfg->P = pCfg->getValue( #N , camCfg->P)
#define CAM_CONFIG_MT(suf,P) CAM_CONFIG(suf.P, P )

#ifdef 	__BFIN__
	#define Suf MT9P031
#else
	#define Suf MT9P001
#endif

void CameraConfig::configure(Configuration *pCfg, camconfig *camCfg){
#ifndef __BFIN__
	CAM_CONFIG_MT(Suf,row_start_val); //. Legal values: [0, 2004], even.
	CAM_CONFIG_MT(Suf,column_start_val); //Legal values : [0,2750], even.
#endif

//	CAM_CONFIG_MT(Suf,row_size_val); // Legal values: [1, 2005], odd.
//	CAM_CONFIG_MT(Suf,column_size_val);// Legal values: [1, 2751], odd.

	//Make it multiple of 16 so that it helps in Making Pyramid.
//	camCfg->row_size_val = (((camCfg->row_size_val+1)>>4)<<4) -1;
//	camCfg->column_size_val = (((camCfg->column_size_val+1)>>4)<<4) -1;

	CAM_CONFIG_MT(Suf,horizontal_blank_val); // Legal values: [0, 4095].
	CAM_CONFIG_MT(Suf,vertical_blank_val); // Legal values: [8, 2047].
	CAM_CONFIG_MT(Suf,shutter_width_upper_val);
	CAM_CONFIG_MT(Suf,shutter_width_lower_val);
	CAM_CONFIG_MT(Suf,read_mode1_val);
	CAM_CONFIG_MT(Suf,read_mode2_val);
	CAM_CONFIG_MT(Suf,row_address_mode_val);
	CAM_CONFIG_MT(Suf,column_address_mode_val);
	CAM_CONFIG_MT(Suf,global_gain_val);
	CAM_CONFIG_MT(Suf,green1_gain_val);
	CAM_CONFIG_MT(Suf,red_gain_val);
	CAM_CONFIG_MT(Suf,blue_gain_val);
	CAM_CONFIG_MT(Suf,green2_gain_val);
	CAM_CONFIG_MT(Suf,mirror_row);     // Should be 0 or 1
	CAM_CONFIG_MT(Suf,mirror_column);	// Should be 0 or 1

//	CAM_CONFIG_MT(Suf,row_bin);  //Legal values 0 or 3
//	CAM_CONFIG_MT(Suf,row_skip); //Legal values [0,7]
//	CAM_CONFIG_MT(Suf,column_bin); //Legal values {0,1,3}
//	CAM_CONFIG_MT(Suf,column_skip); // Legal values [0,6]

	CAM_CONFIG_MT(Suf,green_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT(Suf,green_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT(Suf,green_analog_multiplier); //Legal values {0,1}
	CAM_CONFIG_MT(Suf,blue_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT(Suf,blue_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT(Suf,blue_analog_multiplier); //Legal values {0,1}
	CAM_CONFIG_MT(Suf,red_digital_gain); //Legal values [0,120]
	CAM_CONFIG_MT(Suf,red_analog_gain); // Legal values [8,63]
	CAM_CONFIG_MT(Suf,red_analog_multiplier); //Legal values {0,1}

	CAM_CONFIG_MT(Suf,enable_blc); // Legal Values {0,1}
	CAM_CONFIG_MT(Suf,black_level_target); //[0,4095]
	CAM_CONFIG_MT(Suf,row_black_default_offset); //[0,4095]
	CAM_CONFIG_MT(Suf,test_pattern_type);
	CAM_CONFIG_MT(Suf,test_pattern_green);
	CAM_CONFIG_MT(Suf,test_pattern_red);
	CAM_CONFIG_MT(Suf,test_pattern_blue);
	CAM_CONFIG_MT(Suf,test_pattern_bar_width);

	//Madhav - Need to check what has to be done for eval board
	CAM_CONFIG_MT(Suf,pixel_clock_control);

	CAM_CONFIG_MT(Suf,pll_enable);
	CAM_CONFIG_MT(Suf,pll_delay);

	CAM_CONFIG_MT(Suf,pll_control);
	CAM_CONFIG_MT(Suf,pll_config1);
	CAM_CONFIG_MT(Suf,pll_config2);
	CAM_CONFIG_MT(Suf,output_control);

}





