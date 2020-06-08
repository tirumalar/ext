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

/* [] END OF FILE */


#include <project.h>

#define EXT_POWER_LIMIT 100

//int IsExtPower()
//{
//	int pow;
//	ADC_POWER_IsEndConversion(ADC_POWER_WAIT_FOR_RESULT);
//	pow = ADC_POWER_GetResult16(0);
//	//return 0;
//	if (pow > EXT_POWER_LIMIT)
//	   return 1;
//	else
//	   return 0;
//}
//
//void SetLocalPower()
//{
// EXT_POWER_EN_Write(0);
// LOC_POWER_EN_Write(1);
//}
//void SetExtPower()
//{
// EXT_POWER_EN_Write(1);
// LOC_POWER_EN_Write(0);
//}
//
//void DoPower(void)
//{
//	if (IsExtPower())
//	  SetExtPower();
//	else
//	  SetLocalPower();
//}