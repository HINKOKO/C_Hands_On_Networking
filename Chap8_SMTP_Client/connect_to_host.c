#include "smtp.h"

/**
 * connect_to_host - open a TCP connection
 * @hostname: the hostname to connect to
 * @port: Port to connect on
 * Return: a socket descriptor
*/

SOCKET connect_to_host(const char *hostname, const char *port)
{
	printf("Configuring remote address...\n");
	struct addrinfo hints, *peer;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, port, &hints, &peer)) {
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Remote address is: ");
	char addr[100], service[100];

	getnameinfo(peer->ai_addr, peer->ai_addrlen, addr, sizeof(addr),
		service, sizeof(service), NI_NUMERICHOST);
	
	printf("%s %s\nCreating socket...\n", addr, service);
	SOCKET server;
	server = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);
	if (!ISVALIDSOCKET(server)) {
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Connecting...\n");
	if (connect(server, peer->ai_addr, peer->ai_addrlen)) {
		fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	freeaddrinfo(peer);
	printf("Connected.\n\n");

	return (server);
}
