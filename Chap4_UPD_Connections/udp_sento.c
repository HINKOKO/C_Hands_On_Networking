#include "socket_macro.h"

/**
 * Fun fact of UDP nature =>
 * This program runs even if no udp server listen
 * `udp_sendto` doesn't know that the packet was not delivered, and it doesn't care
*/

int main() {
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif

	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM; /* we want a connection-oriented TCP */
	struct addrinfo *peer_address;
	
	if (getaddrinfo("127.0.0.1", "8080", &hints, &peer_address)) {
		fprintf(stderr, "getaddrinfo() failed (%d).\n", GETSOCKETERRNO());
		return (1);
	}
	printf("Remote address is: ");
	char addr_buff[100], service_buff[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
				addr_buff, sizeof(addr_buff),
				service_buff, sizeof(service_buff),
				NI_NUMERICHOST | NI_NUMERICSERV);
	printf("%s %s\n", addr_buff, service_buff);

	/* Now create a socket, passing fields from `peer_address` to create appropriate */
	printf("Creating socket...\n");
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family,
						 peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_peer))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	/* socket creation OK. UDP world --> go straight to send data with `sendto()`*/
	const char *msg = "Hello DlroW !";
	printf("Sending message: %s\n", msg);
	int bytes_sent = sendto(socket_peer, msg, strlen(msg),
		0, peer_address->ai_addr, peer_address->ai_addrlen);
	printf("Sent %d bytes.\n", bytes_sent);

	/* if we were to call `recvfrom()` at this point, we could potentially get */
	/* data from anybody that sends to us - Not necessarily the server we just transmitted to */

	/* CleanUP */
	freeaddrinfo(peer_address);
	CLOSESOCKET(socket_peer);

#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished\n");
	return (0);
}
