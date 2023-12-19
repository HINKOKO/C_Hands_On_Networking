#include "socket_macro.h"

int main() {
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif

	printf("Config local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_addr;
	getaddrinfo(0, "8080", &hints, &bind_addr);

	printf("Creating socket...\n");
	SOCKET socket_listen;
	/* Call to socket uses our address info from getaddrinfo() to create proper socket type */
	socket_listen = socket(bind_addr->ai_family,
		bind_addr->ai_socktype, bind_addr->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Binding socket to local addres...\n");
	if (bind(socket_listen, bind_addr->ai_addr, bind_addr->ai_addrlen)) {
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* Here Mitch , we gonna DIVERGE from TCP server implementation */
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	/* two previous declarations keep our code RObust if we swithc to deal IPv6 */
	char read[1024];
	int bytes_recv = recvfrom(socket_listen,
		read, 1024, 0,
		(struct sockaddr *)&client_address, &client_len);
	/* Think of `recvfrom` as a TCP combination version of `accept + recv` */
	/* data received may not be null terminated, safely print with the printf specifier as follow */
	printf("Received (%d) bytes: %.*s\n", bytes_recv, bytes_recv, read);
	/* print also sender address and port number */
	printf("Remote address is: ");
	char host_buff[100];
	char service_buff[100];
	getnameinfo((struct sockaddr *)&client_address,
		client_len, host_buff, sizeof(host_buff),
		service_buff, sizeof(service_buff),
		NI_NUMERICHOST | NI_NUMERICSERV);
	/* last argument of getnameinfo -> we want both client addr and port number in numeric form */
	printf("%s %s\n", host_buff, service_buff);

	/* Cleaning */
	CLOSESOCKET(socket_listen);
#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished\n");
	return (0);
}
