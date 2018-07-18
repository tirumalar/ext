/***************************************************************************
 *   Copyright (C) 2008 by Akshay Mathur,,,   				   *
 *   akshay.mathur@mamigo.in   						   *
 *                                                                         *
 *   Mamigo Copyright protects the code.				   *
 ***************************************************************************/

#ifndef _MAMIGO_DMA_
#define _MAMIGO_DMA_ 1

#ifndef MAMIGO_VERIFY
#include <asm-blackfin/mach/dma.h>
#endif

#include "dmaparams.h"

void set_wdsize16(void);
void set_wdsize8(void);
void copy_via_dma_mem(void * dest,const void *src );
void write_dest_dmasetup_ch1( dma_config dmcfg);
void write_source_dmasetup_ch1( dma_config dmcfg);
void do_dmasetup(dma_config_output dmaco, int wordSize);
#endif
