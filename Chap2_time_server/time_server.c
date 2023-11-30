#if defined(_WIN32)
#ifnef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws-32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

/* All above will detects if the compiler is running on Windows or not*/
/* And includes the proper headers for the platform it is running on */

/**********************
 * Invalid sockets => Windows retunrs INVALID_SOCKET
 * on Unix => Invalid socket returns negative num on failure
 * !! => pb  as Windows SOCKET is unsigned
 * There again , check this macro
 */

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

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

	/* Figure out the local address that our web server should bin to */
	printf("Configuring local address...\n");
	struct addrinfo hints;
	/* Init hints memory places */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 /* Ipv4 addresses in that family */
	hints.ai_socktype = SOCK_STREAM; /* Meaning using TCP */
	hints.ai_flags = AI_PASSIVE;	 /* So we are telling getaddrinfo() to set up and listen on any available network interface */
	/* If the AI_PASSIVE flag is specified in hints.ai_flags, and node is NULL (1st param of getaddrinfo is 0 (NULL)) */
	/* then the returned socket addresses will be suitable for bind(2)ing a socket that will accept(2)  connections. */

	struct addrinfo *bind_address;
	/* getaddrinfo fills struct addrinfo with needed infos */
	/* hints parameters tells it what we are looking for */
	getaddrinfo(0, "8080", &hints, &bind_address);

	/* getaddrinfo advantage => protocol independant Only need to change the field AF_INET to AF_INET6 */
	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
						   bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* Socket created successfully, we call bind() to associate it with our address from getaddrinfo() */
	printf("Now binding socket to local address...\n");
	/* Bind returns 0 on success*/
	if (bind(socket_listen,
			 bind_address->ai_addr, bind_address->ai_addrlen))
	{
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);
	/* start listening then */
	printf("Listening...\n");
	if (listen(socket_listen, 10) < 0)
	{
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/* We now attempt to accept() */
	printf("Waiting for connection on port 8080...\n");
	struct sockaddr_storage client;
	socklen_t client_len = sizeof(client);
	SOCKET socket_client = accept(socket_listen,
								  (struct sockaddr *)&client, &client_len);
	if (!ISVALIDSOCKET(socket_client))
	{
		fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	/**
	 * accept() => block the program until a new connection is made
	 * In other word, program 'sleep' until conn is made to the listenin socket
	 * whenn conn occurs, accept() create a newx socket for it. The original socket
	 * continues to listen for new connections (remember the 10 as 2nd param of 'listen' ? ) logic uh?
	 * But the new socket returned by accept() can be used to send/receive data over the newly established connection.
	 * Accetp() also fills in address info of the client that connected
	 */
	/* => At this point, a TCP connection has been established to a remote client */
	printf("Client is connected...\n");
	char address_buff[100];
	getnameinfo((struct sockaddr *)&client, client_len,
				address_buff, sizeof(address_buff), 0, 0, NI_NUMERICHOST);
	/* NI_NUMERICHOST => specifies we wanna see the hostname as an IP address */
	printf("%s\n", address_buff);
	/* Step optional, but always good practice to log network connections somewhere */

	/* As we are programming a web server, we expect the client (brower) to send us an HTTP request */
	printf("Reading request...\n");
	char req[1024];
	int bytes_received = recv(socket_client, req, 1024, 0);
	/* printf("Received %d bytes\n", bytes_received); */
	/* or if we wanna see the browser's request to console */
	/* a real web server would need to parse the request and look at which resource the browser is requesting */
	printf("%.*s", bytes_received, req);

	/* SEnd our response back */
	/* PArt of response coming after double \r\n\r\n treated by the browsers as plain text */
	printf("Sending response...\n");
	const char *res =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Local time is: ";
	int bytes_sent = send(socket_client, res, strlen(res), 0);
	printf("Send %d of %d bytes.\n", bytes_sent, (int)strlen(res));

	/* After HTTP header and beginning of our message is sent, we can send the actual time */
	time_t timer;
	time(&timer);
	char *time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
	printf("Send %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

	/* Close the client connection to indicate the browser that we've sent all of our data */
	printf("Closing connection...\n");
	CLOSESOCKET(socket_client);

	/* At this point, we could call accept() on 'socket_listen' to accept additional connections */
	/* That's what a "real world server" would do, here it is juste a quick demo program */
	printf("CLosing listening socket...\n");
	CLOSESOCKET(socket_listen);

#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished.\n");
	return 0;
}
