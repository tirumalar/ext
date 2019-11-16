/*******************************************************************************
* File Name: AUDIO_DAC.h
* Version 1.10
*
* Description:
*  This file provides constants and parameter values for the IDAC_P4
*  component.
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_IDAC_AUDIO_DAC_H)
#define CY_IDAC_AUDIO_DAC_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"


/***************************************
* Internal Type defines
***************************************/

/* Structure to save state before go to sleep */
typedef struct
{
    uint8  enableState;
} AUDIO_DAC_BACKUP_STRUCT;


extern uint32 AUDIO_DAC_initVar;

/**************************************
*    Enumerated Types and Parameters
**************************************/

/* IDAC polarity setting constants */
#define AUDIO_DAC__SOURCE 0
#define AUDIO_DAC__SINK 1


/* IDAC range setting constants */
#define AUDIO_DAC__LOWRANGE 0
#define AUDIO_DAC__HIGHRANGE 1


/* IDAC polarity setting definitions */
#define AUDIO_DAC_MODE_SOURCE    (AUDIO_DAC__SOURCE)
#define AUDIO_DAC_MODE_SINK      (AUDIO_DAC__SINK)

/* IDAC RANGE setting definitions */
#define AUDIO_DAC_MODE_LOWRANGE  (AUDIO_DAC__LOWRANGE)
#define AUDIO_DAC_MODE_HIGHRANGE (AUDIO_DAC__HIGHRANGE)

/***************************************
*   Conditional Compilation Parameters
****************************************/

#define AUDIO_DAC_IDAC_RESOLUTION    (7u)
#define AUDIO_DAC_IDAC_RANGE         (1u)
#define AUDIO_DAC_IDAC_POLARITY      (0u)


/***************************************
*    Initial Parameter Constants
***************************************/
#define AUDIO_DAC_IDAC_INIT_VALUE    (120u)




/***************************************
*        Function Prototypes
***************************************/

void AUDIO_DAC_Init(void);
void AUDIO_DAC_Enable(void);
void AUDIO_DAC_Start(void);
void AUDIO_DAC_Stop(void);
void AUDIO_DAC_SetValue(uint32  value);
void AUDIO_DAC_SaveConfig(void);
void AUDIO_DAC_Sleep(void);
void AUDIO_DAC_RestoreConfig(void);
void AUDIO_DAC_Wakeup(void);


/***************************************
*            API Constants
***************************************/

#define AUDIO_DAC_IDAC_EN_MODE           (3u)
#define AUDIO_DAC_IDAC_CSD_EN            (1u)
#define AUDIO_DAC_IDAC_CSD_EN_POSITION   (31u)

#define AUDIO_DAC_IDAC_VALUE_POSITION    (AUDIO_DAC_cy_psoc4_idac__CSD_IDAC_SHIFT)

#define AUDIO_DAC_IDAC_MODE_SHIFT        (8u)
#define AUDIO_DAC_IDAC_MODE_POSITION     ((uint32)AUDIO_DAC_cy_psoc4_idac__CSD_IDAC_SHIFT +\
                                                 AUDIO_DAC_IDAC_MODE_SHIFT)

#define AUDIO_DAC_IDAC_RANGE_SHIFT       (10u)
#define AUDIO_DAC_IDAC_RANGE_POSITION    ((uint32)AUDIO_DAC_cy_psoc4_idac__CSD_IDAC_SHIFT +\
                                                 AUDIO_DAC_IDAC_RANGE_SHIFT)

#define AUDIO_DAC_IDAC_POLARITY_POSITION ((uint32)AUDIO_DAC_cy_psoc4_idac__POLARITY_SHIFT)

#define AUDIO_DAC_IDAC_TRIM1_POSITION    ((uint32)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM1_SHIFT)
#define AUDIO_DAC_IDAC_TRIM2_POSITION    ((uint32)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM2_SHIFT)

#define AUDIO_DAC_IDAC_CDS_EN_MASK       (0x80000000u)

#if(AUDIO_DAC_IDAC_RESOLUTION == 8u)
  #define AUDIO_DAC_IDAC_VALUE_MASK      (0xFFu)
#else
  #define AUDIO_DAC_IDAC_VALUE_MASK      (0x7Fu)
#endif /* (AUDIO_DAC_IDAC_RESOLUTION == 8u) */

#define AUDIO_DAC_IDAC_MODE_MASK         (3u)
#define AUDIO_DAC_IDAC_RANGE_MASK        (1u)
#define AUDIO_DAC_IDAC_POLARITY_MASK     (1u)

#define AUDIO_DAC_CSD_IDAC_TRIM1_MASK    (0x0000000FuL << AUDIO_DAC_IDAC_TRIM1_POSITION)
#define AUDIO_DAC_CSD_IDAC_TRIM2_MASK    (0x0000000FuL << AUDIO_DAC_IDAC_TRIM2_POSITION)


/***************************************
*        Registers
***************************************/

#define AUDIO_DAC_IDAC_CONTROL_REG    (*(reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_IDAC)
#define AUDIO_DAC_IDAC_CONTROL_PTR    ( (reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_IDAC)

#define AUDIO_DAC_IDAC_POLARITY_CONTROL_REG    (*(reg32 *)AUDIO_DAC_cy_psoc4_idac__CONTROL)
#define AUDIO_DAC_IDAC_POLARITY_CONTROL_PTR    ( (reg32 *)AUDIO_DAC_cy_psoc4_idac__CONTROL)

#define AUDIO_DAC_CSD_TRIM1_REG       (*(reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM1)
#define AUDIO_DAC_CSD_TRIM1_PTR       ( (reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM1)

#define AUDIO_DAC_CSD_TRIM2_REG       (*(reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM2)
#define AUDIO_DAC_CSD_TRIM2_PTR       ( (reg32 *)AUDIO_DAC_cy_psoc4_idac__CSD_TRIM2)

#if (CY_PSOC4_4100M || CY_PSOC4_4200M)
    #if(AUDIO_DAC_cy_psoc4_idac__IDAC_NUMBER > 2u)
        #define AUDIO_DAC_SFLASH_TRIM1_REG       (*(reg8 *)CYREG_SFLASH_CSD1_TRIM1_HVIDAC)
        #define AUDIO_DAC_SFLASH_TRIM1_PTR       ( (reg8 *)CYREG_SFLASH_CSD1_TRIM1_HVIDAC)
        
        #define AUDIO_DAC_SFLASH_TRIM2_REG       (*(reg8 *)CYREG_SFLASH_CSD1_TRIM2_HVIDAC)
        #define AUDIO_DAC_SFLASH_TRIM2_PTR       ( (reg8 *)CYREG_SFLASH_CSD1_TRIM2_HVIDAC)
    #else
        #define AUDIO_DAC_SFLASH_TRIM1_REG       (*(reg8 *)CYREG_SFLASH_CSD_TRIM1_HVIDAC)
        #define AUDIO_DAC_SFLASH_TRIM1_PTR       ( (reg8 *)CYREG_SFLASH_CSD_TRIM1_HVIDAC)
        
        #define AUDIO_DAC_SFLASH_TRIM2_REG       (*(reg8 *)CYREG_SFLASH_CSD_TRIM2_HVIDAC)
        #define AUDIO_DAC_SFLASH_TRIM2_PTR       ( (reg8 *)CYREG_SFLASH_CSD_TRIM2_HVIDAC)
    #endif /* (AUDIO_DAC_cy_psoc4_idac__IDAC_NUMBER > 2u) */
#else
    #define AUDIO_DAC_SFLASH_TRIM1_REG       (*(reg8 *)CYREG_SFLASH_CSD_TRIM1_HVIDAC)
    #define AUDIO_DAC_SFLASH_TRIM1_PTR       ( (reg8 *)CYREG_SFLASH_CSD_TRIM1_HVIDAC)
    
    #define AUDIO_DAC_SFLASH_TRIM2_REG       (*(reg8 *)CYREG_SFLASH_CSD_TRIM2_HVIDAC)
    #define AUDIO_DAC_SFLASH_TRIM2_PTR       ( (reg8 *)CYREG_SFLASH_CSD_TRIM2_HVIDAC)
#endif /* (CY_PSOC4_4100M || CY_PSOC4_4200M) */

#endif /* CY_IDAC_AUDIO_DAC_H */

/* [] END OF FILE */
