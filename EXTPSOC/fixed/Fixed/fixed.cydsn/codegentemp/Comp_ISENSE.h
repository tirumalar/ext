/*******************************************************************************
* File Name: Comp_ISENSE.h
* Version 1.20
*
* Description:
*  This file contains the function prototypes and constants used in
*  the Analog Comparator User Module.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_COMPARATOR_Comp_ISENSE_H)
#define CY_COMPARATOR_Comp_ISENSE_H

#include "cytypes.h"
#include "CyLib.h"
#include "cyfitter.h"


/***************************************
*       Type Definitions
***************************************/

/* Structure to save state before go to sleep */
typedef struct
{
    uint8  enableState;
} Comp_ISENSE_BACKUP_STRUCT;


/**************************************
*        Function Prototypes
**************************************/
void    Comp_ISENSE_Init(void);
void    Comp_ISENSE_Enable(void);
void    Comp_ISENSE_Start(void);
void    Comp_ISENSE_Stop(void);
void    Comp_ISENSE_SetSpeed(uint32 speed);
uint32  Comp_ISENSE_ZeroCal(void);
void    Comp_ISENSE_LoadTrim(uint32 trimVal);
void    Comp_ISENSE_Sleep(void);
void    Comp_ISENSE_Wakeup(void);
void    Comp_ISENSE_SaveConfig(void);
void    Comp_ISENSE_RestoreConfig(void);
uint32  Comp_ISENSE_GetCompare(void);
uint32  Comp_ISENSE_GetInterruptSource(void);
void    Comp_ISENSE_ClearInterrupt(uint32 interruptMask);
void    Comp_ISENSE_SetInterrupt(uint32 interruptMask);
void    Comp_ISENSE_SetInterruptMask(uint32 interruptMask);
uint32  Comp_ISENSE_GetInterruptMask(void);
uint32  Comp_ISENSE_GetInterruptSourceMasked(void);
void    Comp_ISENSE_SetInterruptMode(uint32 mode);
void    Comp_ISENSE_DisableInterruptOutput(void);

#define Comp_ISENSE_EnableInterruptOutput(void)     Comp_ISENSE_SetInterruptMask(Comp_ISENSE_GetInterruptMask() | Comp_ISENSE_INTR_MASK)
#define Comp_ISENSE_GetInterruptOutputStatus(void)  ((0u == (Comp_ISENSE_GetInterruptSourceMasked() & Comp_ISENSE_INTR_MASKED)) ? 0u : 1u)
#define Comp_ISENSE_ClearInterruptOutput(void)      Comp_ISENSE_ClearInterrupt(Comp_ISENSE_INTR)
#define Comp_ISENSE_SetInterruptOutput(void)        Comp_ISENSE_SetInterrupt(Comp_ISENSE_INTR_MASK)

/**************************************
*           API Constants
**************************************/

/* Power constants for SetSpeed() function */
#define Comp_ISENSE_SLOW_SPEED     (0x01u)
#define Comp_ISENSE_MED_SPEED      (0x02u)
#define Comp_ISENSE_HIGH_SPEED     (0x03u)

/* Trim defines for ZeroCal() function */
#define Comp_ISENSE_COMP_TRIM_SIGN_SHIFT   (5u)
#define Comp_ISENSE_COMP_TRIM_MAX_VALUE    (32u)
#define Comp_ISENSE_COMP_CALIBRATION_DELAY (10u)

/* Constants for Comp_ISENSE_SetInterruptMode(), mode parameter */
#define Comp_ISENSE_INTR_DISABLE       (0x00u)
#define Comp_ISENSE_INTR_RISING        (0x01u)
#define Comp_ISENSE_INTR_FALLING       (0x02u)
#define Comp_ISENSE_INTR_BOTH          (0x03u)

/* Constants for Comp_ISENSE_GetInterruptSource() and 
* Comp_ISENSE_ClearInterrupt(), interruptMask parameter 
*/
#define Comp_ISENSE_INTR_SHIFT         (Comp_ISENSE_cy_psoc4_abuf__INTR_SHIFT)
#define Comp_ISENSE_INTR               ((uint32)0x01u << Comp_ISENSE_INTR_SHIFT)

/* Constants for Comp_ISENSE_SetInterrupt(), interruptMask parameter */
#define Comp_ISENSE_INTR_SET_SHIFT     (Comp_ISENSE_cy_psoc4_abuf__INTR_SET_SHIFT)
#define Comp_ISENSE_INTR_SET           ((uint32)0x01u << Comp_ISENSE_INTR_SHIFT)

/* Constants for Comp_ISENSE_GetInterruptMask() and 
* Comp_ISENSE_SetInterruptMask(), interruptMask parameter 
*/
#define Comp_ISENSE_INTR_MASK_SHIFT    (Comp_ISENSE_cy_psoc4_abuf__INTR_MASK_SHIFT)
#define Comp_ISENSE_INTR_MASK          ((uint32)0x01u << Comp_ISENSE_INTR_MASK_SHIFT)

/* Constants for Comp_ISENSE_GetInterruptSourceMasked() */ 
#define Comp_ISENSE_INTR_MASKED_SHIFT  (Comp_ISENSE_cy_psoc4_abuf__INTR_MASKED_SHIFT)
#define Comp_ISENSE_INTR_MASKED        ((uint32)0x01u << Comp_ISENSE_INTR_MASKED_SHIFT)


/***************************************
*   Initial Parameter Constants
****************************************/

#define Comp_ISENSE_HYSTERESIS         (0u)
#define Comp_ISENSE_POWER              (2u)
#define Comp_ISENSE_DEEPSLEEP_SUPPORT  (0u)
#define Comp_ISENSE_INTERRUPT_EN       (0u)
#define Comp_ISENSE_INTERRUPT          (1u)


/***************************************
*    Variables with External Linkage
***************************************/

extern uint8  Comp_ISENSE_initVar;


/**************************************
*             Registers
**************************************/

#ifdef CYIPBLOCK_m0s8pass4b_VERSION
    #define Comp_ISENSE_CTB_CTRL_REG       (*(reg32 *) Comp_ISENSE_cy_psoc4_abuf__CTB_CTB_CTRL)
    #define Comp_ISENSE_CTB_CTRL_PTR       ( (reg32 *) Comp_ISENSE_cy_psoc4_abuf__CTB_CTB_CTRL)
#else
    #define Comp_ISENSE_CTB_CTRL_REG       (*(reg32 *) Comp_ISENSE_cy_psoc4_abuf__CTBM_CTB_CTRL)
    #define Comp_ISENSE_CTB_CTRL_PTR       ( (reg32 *) Comp_ISENSE_cy_psoc4_abuf__CTBM_CTB_CTRL)
#endif /* CYIPBLOCK_m0s8pass4b_VERSION */

#define Comp_ISENSE_OA_RES_CTRL_REG            (* (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_RES_CTRL)
#define Comp_ISENSE_OA_RES_CTRL_PTR            (  (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_RES_CTRL)

#define Comp_ISENSE_OA_COMP_STAT_REG           (* (reg32 *) Comp_ISENSE_cy_psoc4_abuf__COMP_STAT)
#define Comp_ISENSE_OA_COMP_STAT_PTR           (  (reg32 *) Comp_ISENSE_cy_psoc4_abuf__COMP_STAT)

#define Comp_ISENSE_OA_OFFSET_TRIM_REG         (* (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_OFFSET_TRIM)
#define Comp_ISENSE_OA_OFFSET_TRIM_PTR         (  (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_OFFSET_TRIM)

#define Comp_ISENSE_OA_SLOPE_OFFSET_TRIM_REG   (* (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_SLOPE_OFFSET_TRIM)
#define Comp_ISENSE_OA_SLOPE_OFFSET_TRIM_PTR   (  (reg32 *) Comp_ISENSE_cy_psoc4_abuf__OA_SLOPE_OFFSET_TRIM)

#define Comp_ISENSE_OA_COMP_SHIFT              (Comp_ISENSE_cy_psoc4_abuf__COMP_STAT_SHIFT)

#define Comp_ISENSE_INTR_REG       (*(reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR)
#define Comp_ISENSE_INTR_PTR       ( (reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR)

#define Comp_ISENSE_INTR_SET_REG   (*(reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_SET)
#define Comp_ISENSE_INTR_SET_PTR   ( (reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_SET)

#define Comp_ISENSE_INTR_MASK_REG    (*(reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_MASK) 
#define Comp_ISENSE_INTR_MASK_PTR    ( (reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_MASK) 

#define Comp_ISENSE_INTR_MASKED_REG  (*(reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_MASKED) 
#define Comp_ISENSE_INTR_MASKED_PTR  ( (reg32 *)Comp_ISENSE_cy_psoc4_abuf__INTR_MASKED)

/***************************************
*        Registers Constants
***************************************/

/* Comp_ISENSE_CTB_CTRL_REG */
#define Comp_ISENSE_CTB_CTRL_DEEPSLEEP_ON_SHIFT    (30u)   /* [30] Selects behavior CTB IP in the DeepSleep power mode */
#define Comp_ISENSE_CTB_CTRL_ENABLED_SHIFT         (31u)   /* [31] Enable of the CTB IP */

#define Comp_ISENSE_CTB_CTRL_DEEPSLEEP_ON          ((uint32) 0x01u << Comp_ISENSE_CTB_CTRL_DEEPSLEEP_ON_SHIFT)
#define Comp_ISENSE_CTB_CTRL_ENABLED               ((uint32) 0x01u << Comp_ISENSE_CTB_CTRL_ENABLED_SHIFT)

/* Comp_ISENSE_OA_RES_CTRL_REG */
#define Comp_ISENSE_OA_PWR_MODE_SHIFT          (0u)    /* [1:0]    Power level */
#define Comp_ISENSE_OA_COMP_EN_SHIFT           (4u)    /* [4]      Comparator enable */
#define Comp_ISENSE_OA_HYST_EN_SHIFT           (5u)    /* [5]      Hysteresis enable (10mV) */
#define Comp_ISENSE_OA_BYPASS_DSI_SYNC_SHIFT   (6u)    /* [6]      Bypass comparator output synchronization for DSI (trigger) output */
#define Comp_ISENSE_OA_COMPINT_SHIFT           (8u)    /* [9:8]    Sets Interrupt mode */
#define Comp_ISENSE_OA_PUMP_EN_SHIFT           (11u)   /* [11]     Pump enable */

#define Comp_ISENSE_OA_PWR_MODE                ((uint32) 0x02u << Comp_ISENSE_OA_PWR_MODE_SHIFT)
#define Comp_ISENSE_OA_PWR_MODE_MASK           ((uint32) 0x03u << Comp_ISENSE_OA_PWR_MODE_SHIFT)
#define Comp_ISENSE_OA_COMP_EN                 ((uint32) 0x01u << Comp_ISENSE_OA_COMP_EN_SHIFT)
#define Comp_ISENSE_OA_HYST_EN                 ((uint32) 0x01u << Comp_ISENSE_OA_HYST_EN_SHIFT)
#define Comp_ISENSE_OA_BYPASS_DSI_SYNC         ((uint32) 0x01u << Comp_ISENSE_OA_BYPASS_DSI_SYNC_SHIFT)
#define Comp_ISENSE_OA_COMPINT_MASK            ((uint32) 0x03u << Comp_ISENSE_OA_COMPINT_SHIFT)
#define Comp_ISENSE_OA_PUMP_EN                 ((uint32) 0x01u << Comp_ISENSE_OA_PUMP_EN_SHIFT)


/***************************************
*       Init Macros Definitions
***************************************/

#define Comp_ISENSE_GET_DEEPSLEEP_ON(deepSleep)    ((0u != (deepSleep)) ? (Comp_ISENSE_CTB_CTRL_DEEPSLEEP_ON) : (0u))
#define Comp_ISENSE_GET_OA_HYST_EN(hyst)           ((0u != (hyst)) ? (Comp_ISENSE_OA_HYST_EN) : (0u))
#define Comp_ISENSE_GET_OA_PWR_MODE(mode)          ((mode) & Comp_ISENSE_OA_PWR_MODE_MASK)
#define Comp_ISENSE_CHECK_PWR_MODE_OFF             (0u != (Comp_ISENSE_OA_RES_CTRL_REG & \
                                                                Comp_ISENSE_OA_PWR_MODE_MASK))
#define Comp_ISENSE_GET_OA_COMPINT(intType)        ((uint32) ((((uint32)(intType) << Comp_ISENSE_OA_COMPINT_SHIFT)) & \
                                                        Comp_ISENSE_OA_COMPINT_MASK))

#define Comp_ISENSE_GET_INTR_MASK(mask)            ((0u != (mask)) ? (Comp_ISENSE_INTR_MASK) : (0u))

/* Returns true if component available in Deep Sleep power mode*/ 
#define Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT        (0u != Comp_ISENSE_DEEPSLEEP_SUPPORT) 

#define Comp_ISENSE_DEFAULT_CTB_CTRL (Comp_ISENSE_GET_DEEPSLEEP_ON(Comp_ISENSE_DEEPSLEEP_SUPPORT) | \
                                           Comp_ISENSE_CTB_CTRL_ENABLED)

#define Comp_ISENSE_DEFAULT_OA_RES_CTRL (Comp_ISENSE_OA_COMP_EN | \
                                              Comp_ISENSE_GET_OA_HYST_EN(Comp_ISENSE_HYSTERESIS)  | \
                                              Comp_ISENSE_GET_OA_COMPINT(Comp_ISENSE_INTERRUPT) |\
                                              Comp_ISENSE_OA_BYPASS_DSI_SYNC)

#define Comp_ISENSE_INTR_MASK_REG_DEFAULT  (Comp_ISENSE_GET_INTR_MASK(Comp_ISENSE_INTERRUPT_EN))


/***************************************
* The following code is DEPRECATED and 
* should not be used in new projects.
***************************************/

/* Power constants for SetSpeed() function */
#define Comp_ISENSE_SLOWSPEED      (Comp_ISENSE_SLOW_SPEED)
#define Comp_ISENSE_MEDSPEED       (Comp_ISENSE_MED_SPEED)
#define Comp_ISENSE_HIGHSPEED      (Comp_ISENSE_HIGH_SPEED)

#define Comp_ISENSE_OA_CTRL_REG        (Comp_ISENSE_OA_RES_CTRL_REG)
#define Comp_ISENSE_OA_COMPSTAT_REG    (Comp_ISENSE_OA_COMP_STAT_REG)
#define Comp_ISENSE_OA_COMPSHIFT       (Comp_ISENSE_OA_COMP_SHIFT)

#define Comp_ISENSE_OA_CTB_EN_SHIFT    (Comp_ISENSE_CTB_CTRL_ENABLED_SHIFT)
#define Comp_ISENSE_OA_PWR_MODE_HIGH   (Comp_ISENSE_HIGH_SPEED) 
#define Comp_ISENSE_OA_BYPASS_SHIFT    (Comp_ISENSE_OA_BYPASS_DSI_SYNC_SHIFT)

#endif /*  CY_COMPARATOR_Comp_ISENSE_H */


/* [] END OF FILE */
