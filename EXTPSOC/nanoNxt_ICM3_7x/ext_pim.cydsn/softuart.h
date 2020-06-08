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

typedef struct
{
	int  baud;
	int  parity;
	int  stop_bits;
	int  bits;
	unsigned short rx_byte;
	volatile unsigned short tx_byte;
	volatile unsigned short tx_bits;
	unsigned short rx_bits;
	volatile int  flags;
	volatile int timer_tx_ctr;
	volatile int timer_rx_ctr;
	volatile int rx_recv;
}SOFT_UART_STRUCT;
/* [] END OF FILE */

#define RX_BYTE_READY 1
#define TX_READY      2
#define RX_OVERFLOW   4

#define PARITY_ODD  1
#define PARITY_EVEN 2
#define PARITY_NONE 0

void SU_Isr(SOFT_UART_STRUCT *s);
void SU_PutChar(SOFT_UART_STRUCT *s, int c);
void SU_Start(SOFT_UART_STRUCT *s, int baud, int  parity, int stop);
int SU_GetStat( SOFT_UART_STRUCT *s);
int SU_GetChar( SOFT_UART_STRUCT *s);


