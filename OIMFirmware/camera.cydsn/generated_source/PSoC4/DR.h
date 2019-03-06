/*******************************************************************************
* File Name: DR.h  
* Version 2.10
*
* Description:
*  This file containts Control Register function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_DR_H) /* Pins DR_H */
#define CY_PINS_DR_H

#include "cytypes.h"
#include "cyfitter.h"
#include "DR_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    DR_Write(uint8 value) ;
void    DR_SetDriveMode(uint8 mode) ;
uint8   DR_ReadDataReg(void) ;
uint8   DR_Read(void) ;
uint8   DR_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define DR_DRIVE_MODE_BITS        (3)
#define DR_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - DR_DRIVE_MODE_BITS))

#define DR_DM_ALG_HIZ         (0x00u)
#define DR_DM_DIG_HIZ         (0x01u)
#define DR_DM_RES_UP          (0x02u)
#define DR_DM_RES_DWN         (0x03u)
#define DR_DM_OD_LO           (0x04u)
#define DR_DM_OD_HI           (0x05u)
#define DR_DM_STRONG          (0x06u)
#define DR_DM_RES_UPDWN       (0x07u)

/* Digital Port Constants */
#define DR_MASK               DR__MASK
#define DR_SHIFT              DR__SHIFT
#define DR_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define DR_PS                     (* (reg32 *) DR__PS)
/* Port Configuration */
#define DR_PC                     (* (reg32 *) DR__PC)
/* Data Register */
#define DR_DR                     (* (reg32 *) DR__DR)
/* Input Buffer Disable Override */
#define DR_INP_DIS                (* (reg32 *) DR__PC2)


#if defined(DR__INTSTAT)  /* Interrupt Registers */

    #define DR_INTSTAT                (* (reg32 *) DR__INTSTAT)

#endif /* Interrupt Registers */


/***************************************
* The following code is DEPRECATED and 
* must not be used.
***************************************/

#define DR_DRIVE_MODE_SHIFT       (0x00u)
#define DR_DRIVE_MODE_MASK        (0x07u << DR_DRIVE_MODE_SHIFT)


#endif /* End Pins DR_H */


/* [] END OF FILE */
