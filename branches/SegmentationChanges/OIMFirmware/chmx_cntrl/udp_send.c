/*
 * udp_send.c
 *
 *  Created on: Aug 13, 2016
 *      Author: PTG
 */

/* Nichestack definitions */
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "bsdsock.h"

#include "system.h"
#include "udp_payload_inserter.h"
#include "altera_msgdma.h"
#include "cam.h"
short data_send[800];
alt_u8 the_dest_mac[] = {0xf8, 0x32,0xe4,0x9b,0x39,0xa2};


alt_msgdma_dev *dev;
alt_msgdma_standard_descriptor descriptor;
alt_msgdma_standard_descriptor descriptor_head;

#define HEAD_BUFFER_SIZE 1400
unsigned short head_buffer[HEAD_BUFFER_SIZE];
int head_buffer_ptr=0;


#define MY_IP_LSB 172
void send_udp_test(char *data, int send_len)
{
	UDP_INS_STATS insert_stat;
	//alt_u8 the_dest_mac[6];
	alt_u8 my_src_mac[6];
	ip_addr the_dest_ip;
	ip_addr first_hop;
	ip_addr my_src_ip;
	alt_u16 my_source_port;
	alt_u16 the_destination_port;
	int x;
	int len;
	int bytes_to_send;
	short *l_ptr;
	int idx=2;
	int send_head=1;


	stop_udp_payload_inserter(UDP_PAYLOAD_INSERTER_0_BASE);
	my_src_ip = nets[0]->n_ipaddr;																// we dig our IP address out of the stack variables

	// SOURCE MAC ADDRESS
	memmove(&my_src_mac, nets[0]->n_mib->ifPhysAddress, 6);					// we dig our own MAC address out of the stack variables

	my_source_port = htons(0x0020);
	the_destination_port = htons(0x0020);
	// we need the IP addresses and UDP ports in Network Byte
	the_dest_ip = htonl(my_src_ip)-162;										// order for the UDP Payload Insertion peripheral.
	my_src_ip = htonl(my_src_ip);
/*	the_dest_mac[0] = 0xf8;
	the_dest_mac[1] = 0x32;
	the_dest_mac[2] = 0xe4;
	the_dest_mac[3] = 0x9b;
	the_dest_mac[4] = 0x39;
	the_dest_mac[5] = 0xa2;
*/
		// start the udp payload inserter
	insert_stat.udp_dst = the_destination_port;								// we fill out this insert_stat struct to pass into the
	insert_stat.udp_src = my_source_port;									// payload inserter utility function
	insert_stat.ip_dst = the_dest_ip;
	insert_stat.ip_src = my_src_ip;
	insert_stat.mac_dst_hi = (the_dest_mac[0] << 24) | (the_dest_mac[1] << 16) | (the_dest_mac[2] << 8) | (the_dest_mac[3]);
	insert_stat.mac_dst_lo = (the_dest_mac[4] << 8) | (the_dest_mac[5]);
	insert_stat.mac_src_hi = (my_src_mac[0] << 24) | (my_src_mac[1] << 16) | (my_src_mac[2] << 8) | (my_src_mac[3]);
	insert_stat.mac_src_lo = (my_src_mac[4] << 8) | (my_src_mac[5]);

	start_udp_payload_inserter(UDP_PAYLOAD_INSERTER_0_BASE, &insert_stat);
	dev =  alt_msgdma_open (MSGDMA_0_CSR_NAME);

	//return;
#define CHUNK 1400
#define MIN_CHUNK 64

	while (send_len>0)
	{
	bytes_to_send = min(send_len,CHUNK);

	if(send_len>CHUNK)
		if (send_len<(MIN_CHUNK+CHUNK))
			bytes_to_send=bytes_to_send/2;

	bytes_to_send = max(bytes_to_send,MIN_CHUNK);
	send_len = send_len - bytes_to_send;

	l_ptr = &data[idx-2];
	*l_ptr=((bytes_to_send&0xff)<<8) | (bytes_to_send>>8) ;//| 0x40 ;
	// can use an iowr
	alt_dcache_flush(l_ptr,2);
	if(send_head)
	{
		l_ptr[1]=0x5555;
		alt_dcache_flush(&l_ptr[1],2);
		send_head=0;

	}

	alt_msgdma_construct_standard_mm_to_st_descriptor (
		dev,
		&descriptor,
		&data[idx-2],
		bytes_to_send+2,
		ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK|ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK);

	alt_msgdma_standard_descriptor_sync_transfer(
		dev,
		&descriptor);
	idx = idx+ bytes_to_send;
	}
	//printf("Last is %d\n",bytes_to_send);
}

//alt_u8 the_dest_mac[6]={ 0xf8, 0x32,0xe4, 0x9b,0x39,0xa2};
ip_addr the_dest_ip;

/* Simple Socket Server definitions */
#include "simple_socket_server.h"
void dma_callback (void *a)
    {
    //printf(".");
    //OSTaskResume(B_TIMER_PRIORITY);
    }
void send_udp_init(void)
{
	UDP_INS_STATS insert_stat;
	alt_u8 my_src_mac[6];
	ip_addr first_hop;
	ip_addr my_src_ip;
	alt_u16 my_source_port;
	alt_u16 the_destination_port;
	int x;
	int len;
	int bytes_to_send;
	short *l_ptr;
	int idx=2;
	int send_head=1;


	stop_udp_payload_inserter(UDP_PAYLOAD_INSERTER_0_BASE);
	my_src_ip = nets[0]->n_ipaddr;																// we dig our IP address out of the stack variables

	// SOURCE MAC ADDRESS
	memmove(&my_src_mac, nets[0]->n_mib->ifPhysAddress, 6);					// we dig our own MAC address out of the stack variables

	my_source_port = htons(0x0020);
	the_destination_port = htons(0x0020);
	// we need the IP addresses and UDP ports in Network Byte

	//the_dest_ip = htonl(my_src_ip)-162;										// order for the UDP Payload Insertion peripheral.
	my_src_ip = htonl(my_src_ip);

		// start the udp payload inserter
	insert_stat.udp_dst = the_destination_port;								// we fill out this insert_stat struct to pass into the
	insert_stat.udp_src = my_source_port;									// payload inserter utility function
	insert_stat.ip_dst = the_dest_ip;
	insert_stat.ip_src = my_src_ip;
	insert_stat.mac_dst_hi = (the_dest_mac[0] << 24) | (the_dest_mac[1] << 16) | (the_dest_mac[2] << 8) | (the_dest_mac[3]);
	insert_stat.mac_dst_lo = (the_dest_mac[4] << 8) | (the_dest_mac[5]);
	insert_stat.mac_src_hi = (my_src_mac[0] << 24) | (my_src_mac[1] << 16) | (my_src_mac[2] << 8) | (my_src_mac[3]);
	insert_stat.mac_src_lo = (my_src_mac[4] << 8) | (my_src_mac[5]);

	start_udp_payload_inserter(UDP_PAYLOAD_INSERTER_0_BASE, &insert_stat);
	dev =  alt_msgdma_open (MSGDMA_0_CSR_NAME);

	alt_msgdma_register_callback(
		dev,
		dma_callback,
		0,//alt_u32 control,
		0);
	//dev->descriptor_fifo_depth=128;
}


// simple hashing functions
unsigned short calc_syndrome(unsigned short syndrome,unsigned short in)
    {
    return    syndrome ^= ((syndrome << 5) + (in) +(syndrome >> 2));
    return    syndrome ^= ((syndrome >> 5) + (in) );
    return syndrome + in+1;
    }

int send_udp_cam_data_avail(CAM_STRUCT * cam)
{
	short *l_ptr;
	int bytes_to_send;
	int rval;
	unsigned int set_int;
	l_ptr = &cam->img[cam->bytes_transfered-2];

	alt_u32 fifo_read_fill_level = (
			IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(dev->csr_base) &
			ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK) >>
			ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;



	if (cam->bytes_transfered>=(cam->width*cam->height*cam->bpp))
			{
			ptg_dma_enable(cam->dma_base,0);
			//printf("Done sending");
			return 0;
			}



	bytes_to_send = ptg_dma_get_pix_count(cam->dma_base)*4-cam->bytes_transfered;

	// first lets see if there is anything to transfer
	if ((bytes_to_send <CHUNK)  && ((cam->bytes_transfered +bytes_to_send)<(cam->width*cam->height*cam->bpp)))
			return 2;
	if ((dev->descriptor_fifo_depth-2) <= fifo_read_fill_level)

			{
			//printf("f");
			return 2;
			}
		// is this the first transfer  we start with 2 because of the header
	if (cam->bytes_transfered==2)
		{
		extern int g_fcount;
		//l_ptr[1]=0x5555;
	       //alt_dcache_flush(&l_ptr[1],2);
		head_buffer[head_buffer_ptr+1]=0x5555;
		l_ptr[2]= ((cam->header_data) &0xff) | ( (g_fcount&0xff)<<8);
		alt_dcache_flush(&l_ptr[2],2);
		cam->syndrome=cam->seed;
		}
	else
	    {
	    head_buffer[head_buffer_ptr+1]=IORD_16DIRECT(l_ptr,0);
	    // dont do this on the last packet
	    if ((cam->bytes_transfered+CHUNK)<(cam->width*cam->height*cam->bpp))
		cam->syndrome=calc_syndrome(cam->syndrome,(unsigned short)IORD_16DIRECT(l_ptr,0));

	    }
		    //(l_ptr|0x80000000)[1]; // move the first pixel into the head buffer
	// flush the cash

	// send the syndrome
	// is this the last transfer
	if ((cam->bytes_transfered+CHUNK)>=(cam->width*cam->height*cam->bpp))
	    head_buffer[head_buffer_ptr+1]=cam->syndrome;

	bytes_to_send = min(bytes_to_send,CHUNK);

#if 0
	 // prepend the send length and the udp port
	 *l_ptr=((bytes_to_send&0xff)<<8) | (bytes_to_send>>8) | (cam->udp_port_offset<<6) ;
	 // can use an iowr
	 alt_dcache_flush(l_ptr,2);
#endif
	 head_buffer[head_buffer_ptr]=((bytes_to_send&0xff)<<8) | (bytes_to_send>>8) | (cam->udp_port_offset<<6) ;
	 alt_dcache_flush(&head_buffer[head_buffer_ptr],4);



	//printf("Sending %d\n",bytes_to_send);
	//if(send_len>CHUNK)
	//		if (send_len<(MIN_CHUNK+CHUNK))
	//			bytes_to_send=bytes_to_send/2;


	//if ((dev->descriptor_fifo_depth-3) == fifo_read_fill_level)
	 //   set_int=ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK;
	//else
	 //   set_int =0;

// send the header
	 alt_msgdma_construct_standard_mm_to_st_descriptor (
				dev,
				&descriptor,
				&head_buffer[head_buffer_ptr],
				4,
				set_int | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK /*|ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK*/);

	 rval=alt_msgdma_standard_descriptor_async_transfer(
				dev,
				&descriptor);

	 // move the head buffer pointer
	 head_buffer_ptr+=2;
	 if (head_buffer_ptr>=(HEAD_BUFFER_SIZE-1))
	     head_buffer_ptr=0;

	 //send the data
	alt_msgdma_construct_standard_mm_to_st_descriptor (
			dev,
			&descriptor,
			&cam->img[cam->bytes_transfered],
			(bytes_to_send-2)>2?bytes_to_send-2:2,
			set_int | /*ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK|*/ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK);

	// wait for previous to finish
       // while ( (IORD_ALTERA_MSGDMA_CSR_STATUS(dev->csr_base) & ALTERA_MSGDMA_CSR_BUSY_MASK));


         rval=alt_msgdma_standard_descriptor_async_transfer(
			dev,
			&descriptor);
	if (rval==0)
		cam->bytes_transfered += bytes_to_send;
	else
		printf("Dma to \n");

	return 1;
}

void udp_wait_done( )
{
    while(1)
    {
	alt_u32 fifo_read_fill_level = (
   			IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(dev->csr_base) &
   			ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK) >>
   			ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;
    if (fifo_read_fill_level==0)
	break;
    if (fifo_read_fill_level==256)
 	break;
    }

    while ( (IORD_ALTERA_MSGDMA_CSR_STATUS(dev->csr_base) & ALTERA_MSGDMA_CSR_BUSY_MASK));
}
void dest_mac(ip_addr incomming)
{
	ip_addr my_src_ip;
	ip_addr first_hop;
	struct arptabent *arpent;
	//alt_u8 the_dest_mac[6];
	int x;
	my_src_ip = nets[0]->n_ipaddr;																// we dig our IP address out of the stack variables

	// we need the IP addresses and UDP ports in Network Byte
	the_dest_ip = incomming; //my_src_ip&0xffffff | (DEST_IP_LSB>>24);										// order for the UDP Payload Insertion peripheral.
	iproute(the_dest_ip, &first_hop);						// a client on the other side of a router, or many routers,
	arpent = find_oldest_arp(first_hop);									// we want the MAC address for the first hop that our
	if (arpent->t_pro_addr == first_hop) {									// outbound packets should take
		// DEST MAC ADDRESS
		memmove(&the_dest_mac, arpent->t_phy_addr, 6);
		for (x=0;x <6;x++)
			printf("%2x:",the_dest_mac[x]);
		printf("\n");
	  }
	the_dest_ip = htonl(the_dest_ip);


}
/*
 *
 * // lookup the first hop route for this destination
						    OS_ENTER_CRITICAL();													// since we could be in a routed network sending data to
							result = (int)(iproute(the_dest_ip, &first_hop));						// a client on the other side of a router, or many routers,
						    OS_EXIT_CRITICAL();														// we ask the stack to give us the IP address of the first
							if(result == 0) {														// hop that our packets should be sent to for the
								result = tx_command(my_fd, DENY_STR, DENY_STR_SIZE, 0);				// destination IP address that we are sending to.
								if (result == -1)
									perror("tx_command sht 5.1");
								continue;
							}

							// get the MAC address from the arp table
						    OS_ENTER_CRITICAL();
							arpent = find_oldest_arp(first_hop);									// we want the MAC address for the first hop that our
							if (arpent->t_pro_addr == first_hop) {									// outbound packets should take
								// DEST MAC ADDRESS
								memmove(&the_dest_mac, arpent->t_phy_addr, 6);
							} else {																// if we can't locate the first hop MAC address, then we
							    OS_EXIT_CRITICAL();													// deny the request
								result = tx_command(my_fd, DENY_STR, DENY_STR_SIZE, 0);
								if (result == -1)
									perror("tx_command sht 6");
								continue;
							}
						    OS_EXIT_CRITICAL();
 */


