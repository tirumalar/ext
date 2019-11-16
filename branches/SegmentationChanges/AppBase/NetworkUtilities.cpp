/*
 * NetworkUtilities.cpp
 *
 *  Created on: Aug 31, 2011
 *      Author: dhirvonen
 */

#include "NetworkUtilities.h"
#include "stdio.h"
#include "strings.h"
#include "MessageExt.h"
int SendMessage(const char *ip, int port, const char *str)
{
		printf("SendMessage(%s, %d, %s)\n", ip, port, str); fflush(stdout);
		HostAddress address("127.0.0.1");
		address.SetPort(port);
		TextMessage message(str, strlen(str)+1);
	    SocketClient client(address);
	    client.SendAll(address, message);
	    client.CloseOutput();
	    client.CloseInput();
	    return 0;
}
