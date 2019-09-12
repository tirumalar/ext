/*
 * ptg_dma.c
 *
 *  Created on: Aug 14, 2016
 *      Author: PTG
 */

#include "io.h"
#include "ptg_dma.h"


int ptg_dma_get_pix_count(int base)
{
	return IORD(base,PTG_DMA_PIX_COUNT_REG);
}

void ptg_dma_init(int base, int width , int height, void * img_ptr)
{
	//address
	IOWR(base,PTG_DMA_DEST_REG,img_ptr);
	// height and width
	IOWR(base,PTG_DMA_HEIGHT_WIDTH_REG, height<<16 | width);
	// total
	IOWR(base,PTG_DMA_TOTAL_REG, height*width);
	//enable
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_ENABLE);
}
void  ptg_dma_enable(int base, int enable)
{
   if (enable)
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_ENABLE);
   else
		IOWR(base,PTG_DMA_CONTROL_REG, 0);
}

void ptg_dma_reset(int base)
{
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_RESET);
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_ENABLE);

}
void ptg_dma_set_ptr(int base, void * img_ptr)
{
	IOWR(base,PTG_DMA_DEST_REG,img_ptr);
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_RESET);
	IOWR(base,PTG_DMA_CONTROL_REG, PTG_DMA_CONTROL_ENABLE);
}


