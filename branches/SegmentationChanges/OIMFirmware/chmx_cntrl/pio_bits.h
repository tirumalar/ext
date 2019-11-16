/*
 * pio_bits.h
 *
 *  Created on: Feb 22, 2018
 *      Author: PTG
 */

#ifndef PIO_BITS_H_
#define PIO_BITS_H_



#define IMGR_OE_ALT 0x10
#define IMGR_STDBY_ALT 0x20
#define IMGR_STDBY_MAIN 0x40
#define CAM_TRIG_A 1
#define CAM_TRIG_B 2
#define CAM_TRIG_C 4
#define SHORT_MODE 8

/*
assign cam_a_trig= pio[0];
	assign cam_b_trig= pio[1];
	assign cam_c_trig= pio[2];
	wire          mdio_oen_from_the_tse_mac;
	wire          mdio_out_from_the_tse_mac;


	assign  imgr_oe_alt=pio[4];
   assign  imgr_oe_main=~pio[4];   //n14
   assign  imgr_stdby_alt=pio[5];  //k11
   assign  imgr_stdby_main=pio[6];

*/

#endif /* PIO_BITS_H_ */
