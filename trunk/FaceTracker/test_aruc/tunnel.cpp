#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT    48
#define MAXMSG  512

int
read_from_client (int filedes)
{
	printf("Read_from_client entering \n");
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
  {
    /* End-of-file. */
	  printf("Read_from_client exit 1\n");
    return 0;
  }
  else
    {
      /* Data read. */
	  buffer[nbytes]=0;
      fprintf (stderr, "Server: got message: `%s'\n", buffer);
      {
    	  void port_com_send(char *cmd);
    	  port_com_send(buffer);
      }
      printf("Read_from_client exit 2\n");
      return 0;
    }
}


int
make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM , 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  unlink(sock);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
	  printf("Port %d ",port);
      perror ("bind");
      exit (EXIT_FAILURE);
    }

  return sock;
}

static int sock;
static  fd_set active_fd_set, read_fd_set;

void *init_tunnel(void * arg)
{
//  extern int make_socket (uint16_t port);
	  int i;
	  struct sockaddr_in clientname;
	  size_t size;

  /* Create the socket and set it up to accept connections. */

	  printf("starting server \n");
  sock = make_socket (PORT);
  if (listen (sock, 1) < 0)
    {
      perror ("listen");
      exit (EXIT_FAILURE);
    }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  while (1)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == sock)
              {
                /* Connection request on original socket. */
                int new1;
                size = sizeof (clientname);
                new1 = accept (sock,(struct sockaddr *) &clientname, (socklen_t*)&size);
                if (new1 < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                printf (                         "Server: connect from host (in tunnel)\n");//s, port %hd.\n",
                         //inet_ntoa (clientname.sin_addr),
                         //ntohs (clientname.sin_port));
                FD_SET (new1, &active_fd_set);
              }
            else
              {
                /* Data arriving on an already-connected socket. */
                if (read_from_client (i) < 0)
                  {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                  }
              }
          }
    }
}
