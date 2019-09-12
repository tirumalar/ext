/*
 * cam.h
 *
 *  Created on: Aug 12, 2016
 *      Author: PTG
 */

#ifndef CAM_H_
#define CAM_H_

#define AR135_HEIGHT_REG 0x3006
#define AR135_WIDTH_REG 0x3008
#define AR135_GPIO_EN 0x100
#define AR135_DRIVE_PINS 0x40
#define AR135_EXPOSURE_REG 0x30ac

#define CAM_LEFT  1
#define CAM_RIGHT 2
#define CAM_FACE  4
#define CAM_LEFT_ALT 8
#define CAM_RIGHT_ALT 0x10

#define CAM_LEFT_IDX  0
#define CAM_RIGHT_IDX 1
#define CAM_FACE_IDX  2

#define CHIPID_LEFT   0x18
#define CHIPID_RIGHT  0x10
#define CHIPID_FACE  0x10

unsigned int cam_write16(int cam, int reg, int val);
void CamDoSnap(int cam);
unsigned int cam_read16(int cam, int reg);
void CamTrig(int cam);
void CamSetTrig(int val);

typedef struct
{
	char camId;
	unsigned long dma_base;
	unsigned long width;
	unsigned long bpp;
	unsigned long height;
	unsigned long bytes_transfered;
	unsigned long byte_to_transfer;
	char state;
	unsigned char *img;
	char udp_port_offset;
	int  header_data;
	unsigned short syndrome;
	unsigned short seed;
}CAM_STRUCT;

extern CAM_STRUCT cams[4];

void DoCamInit(CAM_STRUCT *cam, int camId, int width, int height, void * buffer, int dma_base, int udp_offset);
void CamInitFrame(CAM_STRUCT *cam);
void CamSetupGrab(CAM_STRUCT *cam, int enable);

#endif /* CAM_H_ */
