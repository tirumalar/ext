/*
 * NetworkUtilities.h
 *
 *  Created on: Aug 31, 2011
 *      Author: dhirvonen
 */

#ifndef NETWORKUTILITIES_H_
#define NETWORKUTILITIES_H_

#include "socket.h"
int SendMessage(const char *ip, int port, const char *message);


class CrazySocketServer: public SocketServer
{
	public:
		CrazySocketServer(int port, ENetwork Network=eIPv4, int QLen=15);
		CrazySocketServer(HostAddress& Addr, int QLen=15);
		virtual ~CrazySocketServer(void);
};

#if 0
int yes=1;
// lose the pesky "Address already in use" error message
if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
}
#endif


#endif /* NETWORKUTILITIES_H_ */
