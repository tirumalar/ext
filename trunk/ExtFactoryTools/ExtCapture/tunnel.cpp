#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "logging.h"
#include "portcom.h"

#define PORT    48
#define MAXMSG  512

const char logger[30] = "tunnel";

int read_from_client(int filedes) {
	EyelockLog(logger, TRACE, "read_from_client");
	// printf("Read_from_client entering \n");
	char buffer[MAXMSG];
	int nbytes;

	nbytes = read(filedes, buffer, MAXMSG);
	if (nbytes < 0) {
		EyelockLog(logger, ERROR, "read_from_client socket read error %d",
				nbytes);
		/* Read error. */
		perror("read");
		exit (EXIT_FAILURE);
	} else if (nbytes == 0) {
		EyelockLog(logger, ERROR, "read_from_client socket read end of file %d",
				nbytes);
		/* End-of-file. */
		// printf("Read_from_client exit 1\n");
		return 0;
	} else {
		/* Data read. */
		buffer[nbytes] = 0;
		fprintf(stderr, "Server: got message: `%s'\n", buffer);
		{

			port_com_send(buffer);
		}
		EyelockLog(logger, ERROR, "read_from_client exit");
		// printf("Read_from_client exit 2\n");
		return 0;
	}
}

int make_socket(uint16_t port) {
	EyelockLog(logger, TRACE, "make_socket");
	int sock;
	struct sockaddr_in name;

	/* Create the socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		EyelockLog(logger, ERROR, "make_socket socket error");
		perror("socket");
		exit (EXIT_FAILURE);
	}

	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int)) < 0)	{
		perror("socket: failed to set SO_REUSEADDR");
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	unlink(sock);
	if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
		// printf("Port %d ",port);
		EyelockLog(logger, ERROR, "make_socket socket bind error");
		perror("bind");
		exit (EXIT_FAILURE);
	}

	return sock;
}

static int sock;
static fd_set active_fd_set, read_fd_set;

void *init_tunnel(void * arg) {
	EyelockLog(logger, TRACE, "init_tunnel");
//  extern int make_socket (uint16_t port);
	int i;
	struct sockaddr_in clientname;
	size_t size;

	/* Create the socket and set it up to accept connections. */

	printf("starting server \n");
	sock = make_socket(PORT);
	if (listen(sock, 1) < 0) {
		EyelockLog(logger, ERROR, "init_tunnel socket listen error");
		perror("listen");
		exit (EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	while (1) {
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			EyelockLog(logger, ERROR, "init_tunnel socket select error");
			perror("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET(i, &read_fd_set)) {
				if (i == sock) {
					/* Connection request on original socket. */
					int new1;
					size = sizeof(clientname);
					new1 = accept(sock, (struct sockaddr *) &clientname,
							(socklen_t*) &size);
					if (new1 < 0) {
						EyelockLog(logger, ERROR,
								"init_tunnel socket accept error");
						perror("accept");
						exit (EXIT_FAILURE);
					}
					EyelockLog(logger, DEBUG,
							"Server: connect from host (in tunnel)\n"); //s, port %hd.\n",
					//inet_ntoa (clientname.sin_addr),
					//ntohs (clientname.sin_port));
					FD_SET(new1, &active_fd_set);
				} else {
					/* Data arriving on an already-connected socket. */
					if (read_from_client(i) < 0) {
						close(i);
						FD_CLR(i, &active_fd_set);
					}
				}
			}
	}
}
