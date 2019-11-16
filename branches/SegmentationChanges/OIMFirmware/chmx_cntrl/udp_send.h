/*
 * udp_send.h
 *
 *  Created on: Aug 13, 2016
 *      Author: PTG
 */

#ifndef UDP_SEND_H_
#define UDP_SEND_H_


void send_udp_test();

int send_udp_cam_data_avail(CAM_STRUCT * cam);
void send_udp_init(void);

#endif /* UDP_SEND_H_ */
