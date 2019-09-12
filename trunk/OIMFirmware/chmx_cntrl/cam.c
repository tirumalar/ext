/*
 * cam.c
 *
 *  Created on: Aug 12, 2016
 *      Author: PTG
 */



#include <stdio.h>
#include <alt_types.h>
#include <io.h>
#include <system.h>
#include <string.h>
#include <stdlib.h>

#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "i2c_opencores.h"
#include "ptg_dma.h"
#include "cam.h"

#include "pio_bits.h"
/*
 * i2c alt k15 h15
left and right  alt cameras


I2c_scl sda  a6     sda_a   open_cores_0
left right cameras
psoc  tof


i2c_f  t3     sda_b        open_cores_1
face camera accellorometer



i2c_out h11          open_cores_2 scl_pb
out to fixed board
 */


CAM_STRUCT cams[4];

int get_cam_i2c_address(int cam)
{
	switch (cam)
	{
	case CAM_LEFT: return CHIPID_LEFT;
	case CAM_RIGHT: return CHIPID_RIGHT;
	case CAM_FACE: return CHIPID_FACE;
	case CAM_LEFT_ALT: return CHIPID_LEFT;
	case CAM_RIGHT_ALT: return CHIPID_RIGHT;
	}
}
int get_cam_i2c_base(int cam)
{
	switch (cam)
	{
	case CAM_LEFT: return I2C_OPENCORES_0_BASE;
	case CAM_RIGHT: return I2C_OPENCORES_0_BASE;
	case CAM_FACE: return I2C_OPENCORES_1_BASE;
	case CAM_LEFT_ALT: return I2C_OPENCORES_3_BASE;
	case CAM_RIGHT_ALT: return I2C_OPENCORES_3_BASE;

	}

}

void set_cam_port_num(int cam, int port)
    {
    switch (cam)
    	{
    	case CAM_LEFT_IDX: cams[CAM_LEFT_IDX].udp_port_offset=port;break;
    	case CAM_RIGHT_IDX: cams[CAM_RIGHT_IDX].udp_port_offset=port;break;
  	case CAM_FACE_IDX: cams[CAM_FACE_IDX].udp_port_offset=port;break;
    	}
    }
unsigned int cam_read16(int cam, int reg)
{
	int address = get_cam_i2c_address(cam);
	int base = get_cam_i2c_base(cam);
	int val;
	int val2;
	I2C_start(base,address,0);
	I2C_write(base,reg>>8,0);  // upper 16 bits
	I2C_write(base,reg&0xff,0);  // write to register 3 command; no stop
	I2C_start(base,address,1);
	val = I2C_read(base,0);
	val2 = I2C_read(base,1);
	return (val<<8 | (val2&0xff));
}
unsigned int cam_write16(int cam, int reg, int val)
{
	int address = get_cam_i2c_address(cam);
	int base = get_cam_i2c_base(cam);

	I2C_start(base,address,0);
	I2C_write(base,reg>>8,0);  // upper 16 bits
	I2C_write(base,reg&0xff,0);  // write to register 3 command; no stop
	I2C_write(base,val>>8,0);  // upper 16 bits
	I2C_write(base,val&0xff,1);  // write to register 3 command; no stop
	return 1;
}


#define drive_pins
void DoCamInit(CAM_STRUCT *cam, int camId, int width, int height, void * buffer, int dma_base, int udp_offset)
{
int base = get_cam_i2c_base(camId);

cam->width = width;
cam->height = height;
cam->udp_port_offset =udp_offset;
cam->dma_base = dma_base;
cam->bytes_transfered=1;
cam->img = buffer;
cam->camId = camId;
cam->bpp=1;
cam->seed=0;
I2C_init(base,25000000,10000);
printf("Cam %d Read pll %x  base = %x add = %x",camId,cam_read16(camId,0x30B0), get_cam_i2c_base(camId), get_cam_i2c_address(camId));

cam_write16(camId,0x302e,0x1);  	// dont div by 2 pre_pll_clk_div
cam_write16(camId,0x30B0,0x4080); 	//disable pll
cam_write16(camId,0x3070,0); 		//walk a 1
printf("     %x  \n",camId,cam_read16(camId,0x30B0));

cam_write16(camId,AR135_HEIGHT_REG,height-1); //height
cam_write16(camId,AR135_WIDTH_REG,width-1); //width


//IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE,0x0);

cam_write16(camId,0x301a, (0x10D8 | AR135_GPIO_EN) & (~AR135_DRIVE_PINS));
//exp coarse
cam_write16(camId,0x3012,0x100);
printf("Cam %d GPIO Read %x\n",camId,cam_read16(camId,0x3026));


// init the dma
if (cam->bpp==2)
	ptg_dma_init(cam->dma_base,  width ,height, buffer);
else
	ptg_dma_init(cam->dma_base,  width/2 ,height, buffer);

ptg_dma_reset(cam->dma_base);
printf("width %d height %d\n",width,height);

// enable the flash strobe
cam_write16(camId,0x3046,0x100);


}

int g_trig_us = 500;


void CamTrig(int cam)
{



        IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_0_BASE,CAM_TRIG_A | CAM_TRIG_B|CAM_TRIG_C|SHORT_MODE);
	usleep(g_trig_us*3);
	//printf("Trig Cam %d",cam);
     IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_0_BASE,CAM_TRIG_A | CAM_TRIG_B|CAM_TRIG_C);
}

void CamSetTrig(int val)
{
	g_trig_us = val;

}

void CamDoSnap(int cam)
{
	int ch;
//    cam_write16(cam,0x301a,0x10D8 |4);
	printf("pix count %x\n",ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE));
//    printf("RIGHT pix count %x at %d\n",ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_1_BASE),clock());

	// reset the dma
	//ptg_dma_reset(PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE);
	//ptg_dma_reset(PTG_AVALON_VIDEO_DMA_CONTROLLER_1_BASE);

	//toggle the trigger
    CamTrig(cam);

	while (ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE)<(640*480/2))
	{
	   // printf("pix count %x at %d\n",ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE),clock());
	    //printf("RIGHT pix count %x at %d\n",ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_1_BASE),clock());
	    usleep(1000);

	}
    printf("Done at count %x at %d\n",ptg_dma_get_pix_count(PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE),clock());

}

void CamSetupGrab(CAM_STRUCT *cam, int enable)
{
	//printf("Cam %d enable %d\n",cam->camId,enable);
	if (enable==0)
		{
		ptg_dma_enable(cam->dma_base,0);
		return;
		}
	ptg_dma_reset(cam->dma_base);
	cam->bytes_transfered=2;

}

