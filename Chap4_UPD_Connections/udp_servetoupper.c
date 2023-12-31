#include "socket_macro.h"
#include <ctype.h>
#include <sys/select.h>


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
	printf("Configuring local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 /* IPv4 */
	hints.ai_socktype = SOCK_DGRAM; /* UDP */
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

	printf("Binding socket to local address...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
	{
		fprintf(stderr, "bind() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;

	printf("Waiting for connections...\n");
	/* main loop stuff, copies socket set into a new variable, `reads`*/
	/* Then uses `select` to wait until our socket is ready to read from */
	while (98) {
		fd_set reads;
		reads = master;
		if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
			fprintf(stderr, "select() failed (%d)\n", GETSOCKETERRNO());
			return 1;
		}

		if (FD_ISSET(socket_listen, &reads)) {
			struct sockaddr_storage client;
			socklen_t client_len = sizeof(client);
			char read[1024];
			int bytes_recv = recvfrom(socket_listen, read, 1024, 0,
				(struct sockaddr *)&client, &client_len);
			if (bytes_recv < 1) {
				fprintf(stderr, "Connection closed.(%d)\n", GETSOCKETERRNO());
				return (1);
			}
			int j;
			for (j = 0; j < bytes_recv; j++)
				read[j] = toupper(read[j]);
			sendto(socket_listen, read, bytes_recv, 0
				(struct sockaddr *)&client, client_len);
		} /* if FD_ISSET */
	} /* end of while (98) */
	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished...\n");
	return 0;
}
