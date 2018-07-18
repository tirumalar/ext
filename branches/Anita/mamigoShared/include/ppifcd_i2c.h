#ifndef _PPIFCD_I2C_H_
#define _PPIFCD_I2C_H_
/*
 *
 *
 * I2C Command sets for *MT9P031*
 *
 *
 *
 */



/*********************************************
 *                                           *
 * Register addresses                        *
 *********************************************
*/

#define ROW_START		0x01
#define COLUMN_START		0x02
#define ROW_SIZE		0x03
#define COLUMN_SIZE		0x04
#define HORIZONTAL_BLANK	0x05
#define VERTICAL_BLANK		0x06
#define SHUTTER_WIDTH_UPPER	0x08
#define SHUTTER_WIDTH_LOWER	0x09
#define PIX_CLK_CTRL 		0x0A
#define RESTART			0x0B
#define SHUTTER_DELAY   0x0C
#define RESET			0x0D
#define READ_MODE1		0x1E
#define READ_MODE2		0X20
#define ROW_ADDRESS_MODE	0x22
#define COLUMN_ADDRESS_MODE	0x23
#define GREEN1_GAIN		0x2B
#define GREEN2_GAIN		0x2E
#define BLUE_GAIN		0x2C
#define RED_GAIN		0x2D
#define GLOBAL_GAIN		0x35

#define TEST_PATTERN_TYPE 0xA0
#define TEST_PATTERN_GREEN 0xA1
#define TEST_PATTERN_RED 0xA2
#define TEST_PATTERN_BLUE 0xA3
#define TEST_PATTERN_BAR_WIDTH 0xA4

#define ROW_BLACK_DEF_OFFSET 0x4B
#define ROW_BLACK_TARGET 0x49
#define BLC_CALIBRATION 0x62

//PLL REGISTER
#define OUTPUT_CONTROL 0x7
#define PLL_CONTROL 0x10
#define PLL_CONFIG_1 0x11
#define PLL_CONFIG_2 0x12

 struct camconfig
{

unsigned short row_start_val ; //. Legal values: [0, 2004], even.

unsigned short column_start_val; //Legal values : [0,2750], even.

unsigned short row_size_val ; // Legal values: [1, 2005], odd.

unsigned short column_size_val ;// Legal values: [1, 2751], odd.

unsigned short horizontal_blank_val ; // Legal values: [0, 4095].

unsigned short vertical_blank_val; // Legal values: [8, 2047].

unsigned short shutter_width_upper_val;

unsigned short shutter_width_lower_val;

unsigned short pixel_clock_control; // Legal Values: [0x0, 0x8000] to invert pixel clock

unsigned short restart_val;

unsigned short reset_val ;

unsigned short read_mode1_val;

unsigned short read_mode2_val ;

unsigned short row_address_mode_val;

unsigned short column_address_mode_val;

unsigned short global_gain_val;

unsigned short green1_gain_val;

unsigned short red_gain_val;

unsigned short blue_gain_val;

unsigned short green2_gain_val;

unsigned short shutter_delay;

int mirror_row; // Should be 0 or 1

int mirror_column;	// Should be 0 or 1

int row_bin;  //Legal values 0 or 3

int row_skip; //Legal values [0,7]

int column_bin; //Legal values {0,1,3}

int column_skip; // Legal values [0,6]

int green_digital_gain; //Legal values [0,120]

int green_analog_gain; // Legal values [8,63]

int green_analog_multiplier; //Legal values {0,1}

int blue_digital_gain; //Legal values [0,120]

int blue_analog_gain; // Legal values [8,63]

int blue_analog_multiplier; //Legal values {0,1}

int red_digital_gain; //Legal values [0,120]

int red_analog_gain; // Legal values [8,63]

int red_analog_multiplier; //Legal values {0,1}

int enable_blc; // Legal Values {0,1}
int black_level_target; //[0,4095]zzzzzzs
int row_black_default_offset;//[0,4095]

int test_pattern_type;
int test_pattern_green;
int test_pattern_red;
int test_pattern_blue;
int test_pattern_bar_width;

int pll_control;
int pll_config1;
int pll_config2;
int output_control;

int pll_enable;
int pll_delay;

};

#endif
