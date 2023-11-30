#include "socket_macro.h"

/**
 * On Windows, conio header needed for the `_kbhit()` function
 * WHich help us to indicate whether terminal input is waiting
 */
#if defined(_WIN32)
#include <conio.h>
#endif

int main(int argc, char *argv[])
{

#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
		return (fprintf(stderr, "Failed to initialize\n"), 1);
#endif

	/* Program take hostname & port number of server as cmd-line-arguments */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: tcp_client hostname port\n");
		return 1;
	}

	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM; /* we want a connection-oriented TCP */
	struct addrinfo *peer_address;
	if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
	{
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* getaddrinfo() succeeded , we print as debugging measure the remote address */
	printf("Remote address is: ");
	char addr_buff[100], service_buff[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
				addr_buff, sizeof(addr_buff),
				service_buff, sizeof(service_buff),
				NI_NUMERICHOST);
	printf("%s %s\n", addr_buff, service_buff);

	/* Create our socket */
	printf("Creating socket...\n");
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family,
						 peer_address->ai_socktype, peer_address->ai_protocol);
	/* here we used peer_address to set proper socket family & protocol */
	/* Keep our program very flexible, as socket() call creates on the fly IPv4 || IPv6 as needed */
	if (!ISVALIDSOCKET(socket_peer))
	{
		fprintf(stderr, "socket() failed. (%d \n)", GETSOCKETERRNO());
		return 1;
	}
	/**
	 * Now connecting
	 * bind() associates a socket with a local address
	 * connect() associates a socket with a remote address & initiate the TCP conn.
	 */

	printf("Connecting...\n");
	if (connect(socket_peer,
				peer_address->ai_addr, peer_address->ai_addrlen))
	{
		fprintf(stderr, "connect() failed (%d).\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(peer_address);
	/* If we reached this far, TCP conn. is established */
	printf("Connected.\n");
	printf("To send data, enter text followed by enter.\n");

	while (98)
	{
		fd_set reads; /* To store our socket set */
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads); /* zeroed out it, and add our only socket */
#if !defined(_WIN32)
		FD_SET(fileno(stdin), &reads);
#endif
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
		{
			fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
			return 1;
		}
		/* After select() returns, we check to see whether our socket is set in `reads` */
		/* If it is, then we knows to call `recv()` to read the new data inputed */
		if (FD_ISSET(socket_peer, &reads))
		{
			char read[4096];
			int bytes_recv = recv(socket_peer, read, 4096, 0);
			if (bytes_recv < 1)
			{
				printf("Connection closed by peer\n");
				break;
			}
			printf("Received (%d bytes): %.*s", bytes_recv, bytes_recv, read);
		}
		/* After check for new TCP data, check for terminal input */
#if defined(_WIN32)
		if (_kbhit()) {
#else
		if (FD_ISSET(fileno(stdin), &reads))
		{
#endif
			char read[4096];
			if (!fgets(read, 4096, stdin))
				break;
			printf("Sending: %s", read);
			int bytes_sent = send(socket_peer, read, strlen(read), 0);
			/* ignore return value of send because a closed socket makes `select` returns immediately */
			/* Common paradigm in TCP socket prog to ignore errors on `send()` while detect & handle them on `recv()` */
			/* ==> Keeping our connection closing logic all in one place */
			printf("Sent %d bytes.\n", bytes_sent);
		}
	} /* end while(98) */
	printf("Closing socket...\n");
	CLOSESOCKET(socket_peer);

#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished.\n");
	return 0;
}
