#include "socket_macro.h"
#include <ctype.h>
#include <netdb.h>
#include <sys/select.h>

/**
 * Here we make a small modification in the Loop of line 114
 * Where we used to simply "microservice" the uppercasing work,
 * Instead, we loop through all the sockets in master `set`, for each socket, we check
 * that it's not the listening socket && check that it's not the same socket that just sent data.
 * If not -> we call `send()` to echo the received data.
 */

int main(void)
{
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif
	/* GEt our local address, create our socket, and `bind()` */
	printf("Configuring local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 /* IPv4 */
	hints.ai_socktype = SOCK_STREAM; /* TCP */
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	getaddrinfo(0, "8080", &hints, &bind_address);

	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family,
						   bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* Bind our local socket to local address and have it enter listening state */
	printf("Binding socket to local address...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
	{
		fprintf(stderr, "bind() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);
	printf("Listening...");
	if (listen(socket_listen, 10) < 0)
	{
		fprintf(stderr, "listen() is deaf (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* Now we are going to diverge drasticaly from our previous programs */
	/* fd_set -> to store all our active sockets -- max_socket holds the largest fd of sockets */
	/* For now we add our only listening socket to the set and set it to the max of course */
	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;
	/* Later on , we'll add new conn. to master on the fly */
	printf("Waiting for connections...\n");

	while (98)
	{
		fd_set reads;
		reads = master;
		/* select modify the set given to it -> if we didn't copy master, we would lose its data */
		/* Notice the timeout to '0', so that select doesn't return until a socket in `master` set is ready to be read from */
		if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
		{
			fprintf(stderr, "select failed (%d)\n", GETSOCKETERRNO());
			return 1;
		}
		/* At first, our set 'master' contains socket_listen only, but as program runs, add each new conn. to master */
		SOCKET i;
		for (i = 1; i <= max_socket; ++i)
		{
			if (FD_ISSET(i, &reads))
			{
				/* Recall that FD_ISSET() only true for socket that are ready to be read.*/
				/* In case of socket_listen, it means a new conn. is ready to me set with `accept` */
				/* for all other sockets, it means data is ready to be read with `recv()` */
				if (i == socket_listen)
				{
					struct sockaddr_storage client_address;
					socklen_t client_len = sizeof(client_address);
					SOCKET socket_client = accept(socket_listen,
												  (struct sockaddr *)&client_address, &client_len);
					if (!ISVALIDSOCKET(socket_client))
					{
						fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
						return 1;
					}
					FD_SET(socket_client, &master);
					if (socket_client > max_socket)
						max_socket = socket_client;

					char addr_buff[1024];
					getnameinfo((struct sockaddr *)&client_address, client_len,
								addr_buff, sizeof(addr_buff), 0, 0, NI_NUMERICHOST);
					printf("New connection from %s\n", addr_buff);
				}
				else
				{
					/* if i is not socket_listen, it's instead a request for an established conn., in this case, we need to read it */
					char read[1024];
					int bytes_received = recv(i, read, 1024, 0);
					if (bytes_received < 1)
					{
						FD_CLR(i, &master);
						CLOSESOCKET(i);
						continue;
					}
					int j;
					for (j = 0; j <= max_socket; ++j)
					{
						if (FD_ISSET(j, &master))
						{
							if (j == socket_listen || j == i)
								continue;
							else
								send(j, read, bytes_received, 0);
						}
					}
				}
			}
		}
	} /* end of while */
	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished...\n");
	return 0;
}
