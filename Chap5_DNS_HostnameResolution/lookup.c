#include "socket_macro.h"

#include <netdb.h>

#ifndef AI_ALL
#define AI_ALL 0x0100
#endif

/**
 * Little sample program to lookup for printing the IP addresses for a given host
 * provided as command line argument in argv[1]
*/

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage:\n\tlookup hostname\n");
		printf("Example:\n\tlookup example.com");
		exit(0);
	}

/* To init Winsock on Windows platform */
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif
	printf("Resolving hostname '%s'\n", argv[1]);
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	 /* We want all available address of any type, both Ipv4 & Ipv6 , even those we can't handle mwahaha !!! */
	 /* I want it all with 'AI_ALL' Freddy Mercury go I want it aallllll */
	hints.ai_flags = AI_ALL;
	struct addrinfo *peer_address;
	/* '0' for service arg -> we don't care about port number, just wanna resolve an address */
	if (getaddrinfo(argv[1], 0, &hints, &peer_address)) {
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	/* Now getaddrinfo() success, 'peer_address' holds the desired address */
	/* Let's use getnameinfo to convert them to text */
	printf("Remote address is...\n");
	struct addrinfo *address = peer_address;
	do {
		char addr_buff[100];
		/* NI_NUMERICHOST: we want to put IP address into buffer and not a hostname */
		getnameinfo(address->ai_addr, address->ai_addrlen,
			addr_buff, sizeof(addr_buff), 0, 0, NI_NUMERICHOST);
		printf("\t%s\n", addr_buff);
	} while (address = address->ai_next);

	freeaddrinfo(peer_address);

#if defined(_WIN32)
	WSACleanup();
#endif
	return (0);
}
