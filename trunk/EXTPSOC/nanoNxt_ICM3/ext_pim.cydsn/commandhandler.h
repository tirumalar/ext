/** 
 * @file commandhandler.h
 * @brief Command handler
 *    
 * 
 */
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
void commandSendF2F(int len); /**< send F2F protocol command    */
void commandSendWiegand(); /**< send Wiegand data to panel */
void commandSendTOC(int len); /**< command to read Portable Template    */
void commandSendPAC(); /**< command to send TOC    */
void HandleCommand(); /**< handle the commands using handler defined in the commandhandler.c.    */
void commandReadTOC(); /**< command to read TOC    */

/* [] END OF FILE */
