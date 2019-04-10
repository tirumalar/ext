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
#include <project.h>
#include "softuart.h"




void SU_Start(SOFT_UART_STRUCT *s, int baud, int  parity, int stop)
{
	s->baud = baud;
	s->parity = parity;
	s->rx_bits = 0;
	s->tx_bits = 0;
	s->flags = TX_READY;
	s->timer_tx_ctr=0;
	s->timer_rx_ctr=0;
	s->stop_bits =stop;
	s->bits=8;

}



void SU_Isr(SOFT_UART_STRUCT *s)
{
	uint8 rb;
	
	// this is where we tx
	if ( s->tx_bits)
	{
		if ( --s->timer_tx_ctr<=0 )
		{
			if ((s->tx_byte & 0x01) == 0x01)
			{
                OptSelect_Write(2);
			}
			else
			{
                OptSelect_Write(3);
			}
			s->tx_byte = s->tx_byte >>1;
			s->tx_bits--;
			if (s->tx_bits)
			{
				s->flags &=~TX_READY;
				s->timer_tx_ctr = 3;
		    }
			else
			   s->flags |=TX_READY;
		}	
	}

    rb = Pin_WD0_Read();
	if ( s->rx_bits==0)
	{
		if ( rb==0) // we got a start bit
		{
			s->timer_rx_ctr = 4;
			s->rx_bits = s->bits+s->stop_bits+s->parity;
		}
	}
	else
	{
		if ( --s->timer_rx_ctr<=0 )
		{				// rcv
			if ( rb )
			{	s->rx_byte |= 0x8000;	}
            
			s->rx_byte >>= 1;
			s->rx_bits--;
					
			if ( s->rx_bits==0)
			{
				s->rx_recv =  (s->rx_byte>>(8-1-s->stop_bits-(s->parity?1:0)))&0xff;
				if (s->flags & RX_BYTE_READY)
					 s->flags |=RX_OVERFLOW;
				else
					 s->flags |=RX_BYTE_READY;
			}
			else
				s->timer_rx_ctr = 3;
		}
	}
}

#define START_BIT 0

unsigned char getOddParity(unsigned char p)
  {
      p = p ^ (p >> 4 | p << 4);
      p = p ^ (p >> 2);
      p = p ^ (p >> 1);
      return p & 1;
  }

int SU_GetStat( SOFT_UART_STRUCT *s)
{
	return s->flags;
}

int SU_GetChar( SOFT_UART_STRUCT *s)
{
	uint8 b;
	b = s->rx_recv;
	CyGlobalIntDisable;
	s->flags &=~RX_BYTE_READY;
	CyGlobalIntEnable;
	return b;
}

void SU_PutChar( SOFT_UART_STRUCT *s, int c)
{
	unsigned short data_tosend;
	int parity = 0;
	data_tosend = (c<<1) | START_BIT;
	if (s->parity==PARITY_ODD)
	  {
        parity = getOddParity(c);
		data_tosend |= parity ? (0x300 << s->stop_bits) : (0x200 << s->stop_bits);
	  }
	else if (s->parity==PARITY_EVEN)
	  {
         parity = getOddParity(c);
		data_tosend |= parity ?  (0x200 << s->stop_bits) : (0x300 << s->stop_bits);
	  }
	else
		data_tosend |=  1<<(s->bits + s->stop_bits);
	// wait for it to be ready
	while ((s->flags&TX_READY)==0);

	CyGlobalIntDisable;
	s->flags &=~TX_READY;
	CyGlobalIntEnable;
	s->tx_byte = data_tosend;
	s->timer_tx_ctr = 3;
    if(s->parity != PARITY_NONE)
    	s->tx_bits = s->bits+s->stop_bits+ 2;
    else
    	s->tx_bits = s->bits+s->stop_bits+ 1;
	
}





/* [] END OF FILE */
