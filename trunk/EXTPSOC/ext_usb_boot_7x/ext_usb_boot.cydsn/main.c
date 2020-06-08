/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

int main(void)
{
  uint8 BootFlag;
  CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
  EEPROM_Start();
  CySetTemp();
  BootFlag=EEPROM_ReadByte(0);
  if (BootFlag==0x55)
    {
    EEPROM_EraseSector(0);
    ST_LED_0_Write(0);
    ST_LED_1_Write(0);
    ST_LED_2_Write(1);
    Bootloader_1_RESET_SR0_REG=Bootloader_1_SCHEDULE_BTLDR;
    }
  Bootloader_1_Start();
}

/* [] END OF FILE */
