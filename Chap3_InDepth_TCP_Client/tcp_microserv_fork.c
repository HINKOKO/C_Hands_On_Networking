/* WARNING -> This version is not portable as `tcp_microserv_toupper.c*/
/* Here we use the `fork` simple Unix feature, therefore we do not used `select()` to iterate over the `fd_set`*/

#if defined(_WIN32)
#error This program does not support Windows
#endif

#include "socket_macro.h"
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <netdb.h>

/**
 * main - entry point
 * Note -> While at Holberton we used struct sockaddr_in directly -> specially deals with IPv4
 * addresses & ports.
 * here -> struct sockaddrinfo is more "comprehensive", helps in performing address resolution
 * (both IPv4 & IPv6) != socket types, protocol options... make it more flexible for handling various network scenarios
 */

int main(void)
{
	printf("Configuring local address...\n");
	struct addrinfo hints;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *res;
	getaddrinfo(0, "8080", &hints, &res);

	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(res->ai_family,
						   res->ai_socktype, res->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	printf("Let's bind to local address...\n");
	if (bind(socket_listen, res->ai_addr, res->ai_addrlen))
	{
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(res);

	printf("Now listening...\n");
	if (listen(socket_listen, 10) < 0)
	{
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	printf("Ready for connections...\n");
	while (98)
	{
		struct sockaddr_storage client;
		socklen_t client_len = sizeof(client);
		SOCKET socket_client = accept(socket_listen,
									  (struct sockaddr *)&client, &client_len);
		if (!ISVALIDSOCKET(socket_client))
		{
			fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
			return 1;
		}
		char addr_buff[100];
		/* getnameinfo | inverse of | getaddrinfo */
		/* Converts a socket address to a corresponding host and service */
		getnameinfo((struct sockaddr *)&client,
					client_len, addr_buff, sizeof(addr_buff),
					0, 0, NI_NUMERICHOST);
		printf("New connection from %s\n", addr_buff);
		int pid = fork();
		if (pid == 0)
		{
			/* Child process */
			CLOSESOCKET(socket_listen);
			while (65)
			{
				char read[1024];
				int bytes_recv = recv(socket_client, read, 1024, 0);
				if (bytes_recv < 1)
				{
					CLOSESOCKET(socket_client);
					exit(0);
				}
				int j;
				for (j = 0; j < bytes_recv; j++)
					read[j] = toupper(read[j]);
				send(socket_client, read, bytes_recv, 0);
			}
		}
		CLOSESOCKET(socket_client);
	} /* while(98) is over*/
	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.Gutten APPetit!\n");
	return 0;
}
