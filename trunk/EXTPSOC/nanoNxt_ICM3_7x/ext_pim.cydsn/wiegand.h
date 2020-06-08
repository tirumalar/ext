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
#include <isr_50us.h>

CY_ISR_PROTO(Pin_50us_Reset); /**< 50 us reset ineterrupt  */
void initWiegand(); /**< Initialization procedure for wiegand protocol*/
void send_D0_50us_pulse(); //debug remove
void SendWeigand(unsigned char *, int nbits); /**< Send wiegand data to panel procedure*/
int ReadWeigand(int, int); /**< Read wiegand dada from card reader, used new interrupt algorithm procedure */
void clearWiegandBuffer(); /**< Clear wiegand dada buffer. Used when data from wiegand card reader was read and buffer shoul to clear*/
/* [] END OF FILE */
