/******************************************************************************
* File Name: DMA_OSDP_dma.h  
* Version 1.70
*
*  Description:
*   Provides the function definitions for the DMA Controller.
*
*
********************************************************************************
* Copyright 2008-2010, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/
#if !defined(CY_DMA_DMA_OSDP_DMA_H__)
#define CY_DMA_DMA_OSDP_DMA_H__



#include <CYDMAC.H>
#include <CYFITTER.H>

#define DMA_OSDP__TD_TERMOUT_EN (((0 != DMA_OSDP__TERMOUT0_EN) ? TD_TERMOUT0_EN : 0) | \
    (DMA_OSDP__TERMOUT1_EN ? TD_TERMOUT1_EN : 0))

/* Zero based index of DMA_OSDP dma channel */
extern uint8 DMA_OSDP_DmaHandle;


uint8 DMA_OSDP_DmaInitialize(uint8 BurstCount, uint8 ReqestPerBurst, unsigned short UpperSrcAddress, unsigned short UpperDestAddress) ;
void  DMA_OSDP_DmaRelease(void) ;


/* CY_DMA_DMA_OSDP_DMA_H__ */
#endif
