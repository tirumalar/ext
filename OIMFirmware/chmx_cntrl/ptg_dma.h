/*
 * ptg_dma.h
 *
 *  Created on: Aug 14, 2016
 *      Author: PTG
 */

#ifndef PTG_DMA_H_
#define PTG_DMA_H_


#define PTG_DMA_DEST_REG 			0x1
#define PTG_DMA_HEIGHT_WIDTH_REG 	0x2
#define PTG_DMA_TOTAL_REG 			0x4
#define PTG_DMA_CONTROL_REG 		0x3
#define PTG_DMA_PIX_COUNT_REG 		0x5

#define PTG_DMA_CONTROL_ENABLE      0x4
#define PTG_DMA_CONTROL_RESET       0x1


int ptg_dma_get_pix_count(int base);
void ptg_dma_init(int base, int width , int height, void * img_ptr);
void ptg_dma_reset(int base);
void ptg_dma_enable(int base, int enable);
void ptg_dma_set_ptr(int base, void * img_ptr);



#endif /* PTG_DMA_H_ */
