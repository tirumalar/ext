/* ------------------------------------------------------------------------------
  File: chr6dm_usart.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Functions and interrupt handlers for USART communication  
------------------------------------------------------------------------------ */ 
#include "stm32f10x.h"
#include "chr6dm_usart.h"
#include "chr6dm_config.h"

// Buffer, buffer index, and TX status flag for USART transmit
volatile char gTXBuf[TX_BUF_SIZE];
volatile int32_t gTXBufPtr = 0;
volatile char gTXBusy = 0;

// USART RX buffer and associated index and flags
volatile char gRXBuf[RX_BUF_SIZE];
volatile int32_t gRXBufPtr = 0;
volatile char gRXPacketReceived = 0;
volatile char gRXBufOverrun = 0;

// Queue for packets to be transmitted over the USART.  This is a FIFO circular buffer.
volatile USARTPacket gTXPacketBuffer[TX_PACKET_BUFFER_SIZE];
volatile uint8_t gTXPacketBufferStart = 0;
volatile uint8_t gTXPacketBufferEnd = 0;

// Flags used to ensure that packet data isn't corrupted if
// multiple sections of code try to access the buffer "simultaneously"
// (can happen when interrupt-driven code accesses the buffer)
volatile char TXPacketBufferReady = 1;
volatile char gCopyingTXPacketToBuffer = 0;

// Queue for packets received by the USART.  This is a FIFO circular buffer.
volatile USARTPacket gRXPacketBuffer[RX_PACKET_BUFFER_SIZE];
volatile uint8_t gRXPacketBufferStart = 0;
volatile uint8_t gRXPacketBufferEnd = 0;

/*******************************************************************************
* Function Name  : USART1_transmit
* Input          : chat* txdata, int32_t length
* Output         : Transmits the given character sequence out of the UART
* Return         : 1 if successful, 0 otherwise
* Description    : Copies the data pointed to by 'txdata' to the USART1 transmit
					    buffer.  USART1_TX_start() is then called, which copies the
						 first character in the TX buffer to the USART tx register to
						 start transmission.  Subsequent characters in the TX buffer 
						 are transmitted one at a time after the first character is
						 transmitted; this is handled in the USART interrupt handler.
*******************************************************************************/
int32_t USART1_transmit( char* txdata, int32_t length )
{
    int32_t index;
    
    if( length + gTXBufPtr > TX_BUF_SIZE )
    {
        return 0;
    }
    
    for( index = 0; index < length; index++ )
    {
        TXBufPush( txdata[index] );
    }

    USART1_TX_start( );
	 
	 return 1;
}

/*******************************************************************************
* Function Name  : TXBufPush
* Input          : char txdata
* Output         : None
* Return         : 1 if success, 0 otherwise
* Description    : Pushes the given character onto the TX Buffer.  The TX buffer
						  is a FIFO buffer used to store data that will be copied, one
						  character at a time, into the TX register to be transmitted
						  by the USART.
*******************************************************************************/
int32_t TXBufPush( char txdata )
{
    if( gTXBufPtr == TX_BUF_SIZE )
    {
        return 0;
    }

    gTXBuf[gTXBufPtr++] = txdata;

    return 1;
}

/*******************************************************************************
* Function Name  : TXBufPop
* Input          : None
* Output         : None
* Return         : char
* Description    : Pops the next character off of the TX buffer.  This is typically
						  called when the next character needs to be transmitted by the
						  USART.
*******************************************************************************/
char TXBufPop( void )
{
    char buffer_data;
    int32_t index;

    if( gTXBufPtr == 0 )
    {
        return 0;
    }

    buffer_data = gTXBuf[0];

    for( index = 0; index < (gTXBufPtr-1); index++ )
    {
        gTXBuf[index] = gTXBuf[index+1];
    }

    gTXBufPtr--;

    return buffer_data;
}

/*******************************************************************************
* Function Name  : USART1_TX_start
* Input          : None
* Output         : None
* Return         : None
* Description    : If the TX buffer is not empty, and if the USART transmitter is
						  not already busy, then USART1_TX_start pops the next character
						  to be transmitted off the TX buffer, and then copies that character
						  into the USART1 TX register to be transmitted.
*******************************************************************************/
void USART1_TX_start( )
{
    if( gTXBusy )
    {
        return;
    }

    if( gTXBufPtr == 0 )
    {
        return;
    }

    USART_SendData(USART1, TXBufPop() );

    gTXBusy = 1;    
}

/*******************************************************************************
* Function Name  : HandleUSART1Reception
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Checks to see if there are any unprocessed characters in the USART1 RX buffer.
If there are, the function calls ProcessNextCharacter(), which takes the next
unprocessed character in the buffer and handles it.

The RX buffer is filled by the DMA controller as data arrives over the UART.
gRXBufPtr points to the next character that needs to be processed.

*******************************************************************************/
void HandleUSART1Reception( )
{
	 // As long as there are unprocessed characters in the RX buffer, retrieve and
	 // process them.  The function call DMA_GetCurrDataCounter(DMA1_Channel5)
	 // returns the number of characters that must be received to fill the RX buffer.
	 // If gRXBufPtr != (RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5))
	 // then there are new characters that haven't been processed yet.
	 while( gRXBufPtr != (RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5)) )
	 {
		  ProcessNextCharacter();
		  
		  // Increment RXBufPtr.  We use the RX buffer as a circular buffer (since that
		  // is how the DMA controller is configured to treat it).  Therefore, if gRXBufPtr
		  // becomes larger than the buffer size, reset it to zero.  The DMA controller should
		  // be doing the same.
		  gRXBufPtr++;
		  
		  if( gRXBufPtr == RX_BUF_SIZE )
		  {
				gRXBufPtr = 0;
		  }
	 }
}

/*******************************************************************************
* Function Name  : ProcessNextCharacter
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Takes the character in the RX buffer pointed to by gRXBufPtr and processes it.  This
function should only be called if the buffer pointer points to a character that has not
yet been handled.  This function is designed to be called by the function 
HandleUSART1Reception()

This function does NOT increment gRXBufPtr after the character has been handled.  This
should be done by the calling function.

The RX handler automatically parses input characters and formats strings of
characters into packets.  The behavior of the RX handler is based on the current
"state" of the packet handler.  There are 5 states:

USART_STATE_WAIT
	 In this state, the RX handler is waiting to receive the packet start sequence
	 's' 'n' 'p'.  When the sequence is received, the state transitions to 
	 USART_STATE_TYPE

USART_STATE_TYPE
	 In this state, the RX handler expects the next byte received to indicate the
	 packet type.  Once the byte is received, the state transitions to USART_STATE_LENGTH

USART_STATE_LENGTH
	 In this state, the RX handler expects the next byte received to indicate the number
	 of data bytes in the packet. Once the byte is received, the state transitions to
	 USART_STATE_DATA, unless the number of data bytes is zero.  In that case, the state
	 transitions to USART_STATE_CHECKSUM

USART_STATE_DATA
	 In this state, the RX handler expects to receive the number of data bytes indicated
	 by the byte received in the USART_STATE_LENGTH state.  Once all the data bytes have
	 been received, the state transitions to the USART_STATE_CHECKSUM state

USART_STATE_CHECKSUM
	 In this state, the RX handler expects to receive two bytes containing the sum
	 of all other bytes transmitted in the packet.  After the two bytes have been
	 received, the checksum is evaluated.  If it is valid, then the entire packet is
	 copied into the RX packet buffer.  The state then transitions back to the 
	 USART_STATE_WAIT state.  The packet will be handled later from within the
	 main program loop. 

*******************************************************************************/
void ProcessNextCharacter( )
{
	 static uint8_t USART_State = USART_STATE_WAIT;
	 static uint8_t data_counter = 0;
	 static USARTPacket new_packet;
	 
	 // The next action should depend on the USART state.
	 switch( USART_State )
	 {
		  // USART in the WAIT state.  In this state, the USART is waiting to see the sequence of bytes
		  // that signals a new incoming packet.
		  case USART_STATE_WAIT:
				if( data_counter == 0 )		// Waiting on 's' character
				{
					 if( gRXBuf[gRXBufPtr] == 's' )
					 {
						  data_counter++;
					 }
					 else
					 {
						  data_counter = 0;
					 }
				}
				else if( data_counter == 1 )		// Waiting on 'n' character
				{
					 if( gRXBuf[gRXBufPtr] == 'n' )
					 {
						  data_counter++;
					 }
					 else
					 {
						  data_counter = 0;
					 }
				}
				else if( data_counter == 2 )		// Waiting on 'p' character
				{
					 if( gRXBuf[gRXBufPtr] == 'p' )
					 {
						  // The full 'snp' sequence was received.  Reset data_counter (it will be used again
						  // later) and transition to the next state.
						  data_counter = 0;
						  USART_State = USART_STATE_TYPE;
					 }
					 else
					 {
						  data_counter = 0;
					 }
				}
		  break;
		  
		  // USART in the TYPE state.  In this state, the USART has just received the sequence of bytes that
		  // indicates a new packet is about to arrive.  Now, the USART expects to see the packet type.
		  case USART_STATE_TYPE:
				new_packet.PT = gRXBuf[gRXBufPtr];
				USART_State = USART_STATE_LENGTH;
		  break;
		  
		  // USART in the LENGTH state.  In this state, the USART expects to receive a single byte indicating
		  // the number of bytes in the data section of the packet.
		  case USART_STATE_LENGTH:
				new_packet.length = gRXBuf[gRXBufPtr];
				
				if( new_packet.length > 0 )
				{
					 USART_State = USART_STATE_DATA;
				}
				else
				{
					 USART_State = USART_STATE_CHECKSUM;
				}
		  
				// If the packet size is larger than the highest allowable packet data section size, then
				// restore USART state to the WAIT state.  Packet is invalid.
				if( new_packet.length > MAX_PACKET_DATA )
				{
					 USART_State = USART_STATE_WAIT;
					 
					 // Send a BAD_DATA_LENGTH message
					 new_packet.PT = BAD_DATA_LENGTH;
					 new_packet.packet_data[0] = new_packet.length;
					 new_packet.length = 1;
					 new_packet.checksum = ComputeChecksum( &new_packet );
					 
					 SendTXPacketSafe( &new_packet );
				}
				
		  break;
		  
		  // USART in the DATA state.  In this state, the USART expects to receive new_packet.length bytes of
		  // data.
		  case USART_STATE_DATA:
				new_packet.packet_data[data_counter] =  gRXBuf[gRXBufPtr];
				data_counter++;
		  
				// If the expected number of bytes has been received, transition to the CHECKSUM state.
				if( data_counter == new_packet.length )
				{
					 // Reset data_counter, since it will be used in the CHECKSUM state.
					 data_counter = 0;
					 USART_State = USART_STATE_CHECKSUM;
				}
				
		  break;
		  
		  // USART in CHECKSUM state.  In this state, the entire packet has been received, with the exception
		  // of the 16-bit checksum.
		  case USART_STATE_CHECKSUM:
				
				// Get the highest-order byte
				if( data_counter == 0 )
				{
					 new_packet.checksum = ((uint16_t)gRXBuf[gRXBufPtr] << 8);
					 
					 data_counter++;
				}
				else // ( data_counter == 1 )
				{
					 // Get lower-order byte
					 new_packet.checksum = new_packet.checksum | ((uint16_t)gRXBuf[gRXBufPtr] & 0x0FF);
					 
					 // Both checksum bytes have been received.  Make sure that the checksum is valid.
					 uint16_t checksum = ComputeChecksum( &new_packet );
					 
					 // If checksum does not match, send a BAD_CHECKSUM packet
					 if( checksum != new_packet.checksum )
					 {
						  // Send bad checksum packet
						  new_packet.PT = BAD_CHECKSUM;
						  new_packet.length = 0;
						  new_packet.checksum = ComputeChecksum( &new_packet );
						  
						  SendTXPacketSafe( &new_packet );
					 }
					 else
					 {
						  // Packet was received correctly.  Add the packet to the RX packet buffer and
						  // set a flag indicating that a new packet has been received.  
						  AddRXPacket( &new_packet );
						  gRXPacketReceived = 1;
					 }
					 
					 // A full packet has been received.
					 // Put the USART back into the WAIT state and reset 
					 // the data_counter variable so that it can be used to receive the next packet.
					 data_counter = 0;
						  
					 USART_State = USART_STATE_WAIT;					  
				}
				
		  break;
	 }
		  
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Handles interrupt requests generated by USART1. Only TX interrupts are handled here.
RX interrupts are disabled, because RX data is piped directly to memory via the DMA
controller.

The TX interrupt handler ensures that as long as there is data remaining in the TX
buffer, or as long as there is a packet waiting to be copied into the TX buffer,
the USART transmitter will keep sending the data.  A TX interrupt occurs when a
byte has been transmitted successfully.  On an interrupt, if another byte is waiting
in the buffer, it is transmitted.  If the TX buffer is empty, and a packet is waiting
to be transmitted, the packet is copied into the TX buffer and transmission resumes.
If there are no packets waiting, and if the buffer is empty, transmission stops.

*******************************************************************************/
void USART1_IRQHandler(void)
{
	 // Handle TX interrupts
    if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
		  // If there is data in the USART that needs to be transmitted, 
		  // send the next byte
        if( gTXBufPtr > 0 )
        {
            USART_SendData(USART1, TXBufPop() );
        }
        else
        {
				// TX buffer is empty.  
				gTXBusy = 0;
				
				// If there is a packet waiting to be transmitted,
				// copy the packet data to the TX buffer and start the transmission.
				if( gTXPacketBufferStart != gTXPacketBufferEnd )
				{
					 SendNextPacket( );
				}
				
        }

        USART_ClearFlag( USART1, USART_FLAG_TC );
    }


}

/*******************************************************************************
* Function Name  : SendNextPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

If the TX packet buffer is not empty, and if the USART transmitter is not already 
operating, this function copies the next packet in the TX packet buffer into 
the TX buffer.  

Note the distinction between the TX Packet Buffer, and the TX Buffer.  The 
TX Packet Buffer is an array of structures containing data that needs to be
transmitted over the USART.  The TX Buffer is an array of bytes, representing
data that is currently being transmitted.  Packets are first copied into the TX Packet
Buffer, where they "wait" until the USART1 transmitter isn't busy.  Then, when
the transmitter becomes available, the packet is copied into the TX buffer.

*******************************************************************************/
void SendNextPacket( )
{
	 uint8_t PT;
	 uint8_t data_length;
	 uint16_t checksum;
	 int32_t i;
	 
	 // If there are no packets in the buffer that need to be transmitted, return
	 if( gTXPacketBufferStart == gTXPacketBufferEnd )
	 {
		  return;
	 }
	 
	 // If there is already data in the TX buffer, return - the next packet will be sent
	 // automatically when the buffer is empty
	 if( gTXBusy )
	 {
		  return;
	 }
	 
	 PT = gTXPacketBuffer[gTXPacketBufferStart].PT;
	 data_length = gTXPacketBuffer[gTXPacketBufferStart].length;
	 checksum = gTXPacketBuffer[gTXPacketBufferStart].checksum;
	 
	 TXBufPush( 's' );
	 TXBufPush( 'n' );
	 TXBufPush( 'p' );
	 TXBufPush( PT );
	 TXBufPush( data_length );
	 
	 for( i = 0; i < data_length; i++ )
	 {
		  TXBufPush( gTXPacketBuffer[gTXPacketBufferStart].packet_data[i] );
	 }
	 
	 TXBufPush( (char)((checksum >> 8) & 0x0FF) );
	 TXBufPush( (char)((checksum) & 0x0FF) );
	 
	 // Increment packet buffer start pointer.
	 gTXPacketBufferStart++;
	 if( gTXPacketBufferStart >= TX_PACKET_BUFFER_SIZE )
	 {
		  gTXPacketBufferStart = 0;
	 }
	 
	 // Start the transmission
	 USART1_TX_start();
}

/*******************************************************************************
* Function Name  : SendTXPacket
* Input          : USARTPacket* new_packet
* Output         : None
* Return         : None
* Description    : Copies the given packet into the TX Packet Buffer.  Then,
						 SendNextPacket() is called, which copies the packet into the
						 TX Buffer *IF* the transmitter is not already busy.
*******************************************************************************/
void SendTXPacket( USARTPacket* new_packet )
{
	 AddTXPacket( new_packet );
	 SendNextPacket();
}

/*******************************************************************************
* Function Name  : SendTXPacketSafe
* Input          : None
* Output         : None
* Return         : None
* Description    : Equivalent to SendTXPacket, except that it first checks to make
						 sure that a packet isn't currently being copied into the TX Packet
						 Buffer.  It is possible that an interrupt triggered a call to
						 SendTXPacketSafe, in which case the function might already be
						 in use.  This "safe" function call prevents packet data from
						 being corrupted.
*******************************************************************************/
void SendTXPacketSafe( USARTPacket* new_packet )
{
	 if( TXPacketBufferReady )
	 {
		  TXPacketBufferReady = 0;
		  SendTXPacket( new_packet );
		  TXPacketBufferReady = 1;
	 }
}

/*******************************************************************************
* Function Name  : AddTXPacket
* Input          : USARTPacket* new_packet
* Output         : None
* Return         : None
* Description    : Adds the specified packet to the TX Packet Buffer.
*******************************************************************************/
void AddTXPacket( USARTPacket* new_packet )
{
	 gTXPacketBuffer[gTXPacketBufferEnd] = *new_packet;
	 
	 gTXPacketBufferEnd++;
	 if( gTXPacketBufferEnd >= TX_PACKET_BUFFER_SIZE )
	 {
		  gTXPacketBufferEnd = 0;
	 }
}

/*******************************************************************************
* Function Name  : AddRXPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : Adds the specified packet to the RX Packet Buffer.
*******************************************************************************/
void AddRXPacket( USARTPacket* new_packet )
{
	 gRXPacketBuffer[gRXPacketBufferEnd] = *new_packet;
	 
	 gRXPacketBufferEnd++;
	 if( gRXPacketBufferEnd >= RX_PACKET_BUFFER_SIZE )
	 {
		  gRXPacketBufferEnd = 0;
	 }
}

/*******************************************************************************
* Function Name  : ComputeChecksum
* Input          : USARTPacket* new_packet
* Output         : None
* Return         : uint16_t
* Description    : Returns the two byte sum of all the individual bytes in the
						 given packet.
*******************************************************************************/
uint16_t ComputeChecksum( USARTPacket* new_packet )
{
	 int32_t index;

	 uint16_t checksum = 0x73 + 0x6E + 0x70 + new_packet->PT + new_packet->length;
	 
	 for( index = 0; index < new_packet->length; index++ )
	 {
		  checksum += new_packet->packet_data[index];
	 }
	 
	 return checksum;
}