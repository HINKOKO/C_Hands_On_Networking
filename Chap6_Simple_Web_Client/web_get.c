#include "socket.h"
#include <time.h>

#define TIMEOUT 5.0
#define COL1 "\033[31m"
#define COL2 "\033[32m"
#define RES "\033[0m"

void parse_url(char *url, char **hostname, char **port,
			   char **path)
{
	printf("URL: %s\n", url);

	/* Attempts to find '://' in the URL, and parse the protocol */
	char *p;
	p = strstr(url, "://");

	char *protocol = 0;
	if (p)
	{
		protocol = url;
		printf("%sWhat in protocol then ? -> %s\n%s", COL1, protocol, RES);
		*p = 0;
		/* Move p one char after the '://' */
		p += 3;
	}
	else
	{
		p = url;
	}

	printf("%swhat is recorded in url ? -> %s\n%s", COL2, url, RES);
	if (protocol)
	{
		printf("%sWhat in protocol then ? -> %s\n%s", COL1, protocol, RES);

		if (strcmp(protocol, "http"))
		{
			fprintf(stderr, "Unknow protocol '%s'. Only 'http' is supported.\n", protocol);
			exit(1);
		}
	}
	/* p now points to the beginning of the hostname, save it */
	*hostname = p;
	while (*p && *p != ':' && *p != '/' && *p != '#')
		++p;

	*port = "80";
	if (*p == ':')
	{
		*p++ = 0;
		*port = p;
	}
	while (*p && *p != '/' && *p != '#')
		++p;
	/* After port number, 'p' points to the document path */
	*path = p;
	if (*p == '/')
	{
		*path = p + 1;
	}
	*p = 0;
	/* Find a hash and overwrite it (never sent to server) */
	while (*p && *p != '#')
		++p;
	if (*p == '#')
		*p = 0;
	/* At this point, we parsed hostname, port number, document path */
	printf("hostname: %s\n", *hostname);
	printf("port: %s\n", *port);
	printf("path: %s\n", *path);
}

/**
 * send_request - Helper function to format and send the HTTP request
 *
 */

void send_request(SOCKET s, char *hostname, char *port, char *path)
{
	char buff[2048];

	sprintf(buff, "GET /%s HTTP/1.1\r\n", path);
	sprintf(buff + strlen(buff), "Host: %s:%s\r\n", hostname, port);
	sprintf(buff + strlen(buff), "Connection: close\r\n");
	sprintf(buff + strlen(buff), "User-Agent: honpwc web_get 1.0 \r\n");
	sprintf(buff + strlen(buff), "\r\n");

	/* Send the buffer through an open Socket */
	send(s, buff, strlen(buff), 0);
	printf("Sent headers: %s\n", buff);
}

/**
 * connect_to_host - helper function, takes in a hostname and port number,
 * establish a TCP socket connection to it.
 */

SOCKET connect_to_host(char *hostname, char *port)
{
	printf("Configuring remote address...\n");
	struct addrinfo hints, *peer;
	memset(&hints, 0, sizeof(hints));

	hints.ai_socktype = SOCK_STREAM;
	/* getaddrinfo used to resolve hostname */
	if (getaddrinfo(hostname, port, &hints, &peer))
	{
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	printf("Remote address is: ");
	char host[100], serv[100];
	getnameinfo(peer->ai_addr, peer->ai_addrlen,
				host, sizeof(host), serv, sizeof(serv), NI_NUMERICHOST);
	printf("%s %s\n", host, serv);

	printf("Creating socket...\n");
	SOCKET server;
	server = socket(peer->ai_family,
					peer->ai_socktype, peer->ai_protocol);
	if (!ISVALIDSOCKET(server))
	{
		fprintf(stderr, "socket() failed. (%d \n)", GETSOCKETERRNO());
		exit(1);
	}

	printf("Connecting...\n");
	if (connect(server, peer->ai_addr, peer->ai_addrlen))
	{
		fprintf(stderr, "connect() failed (%d).\n", GETSOCKETERRNO());
		exit(1);
	}

	freeaddrinfo(peer);
	printf("Connected.\n\n");

	return (server);
}

/**
 * Http client - takes as input a URL.
 * Attempts to connect to the host and retrieve the resource given by the URL.
 * Display HTTP headers that are sent and received, attempts to parse out the requested resource
 * from the HTTP response
 */

int main(int argc, char *argv[])
{

#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
		return (fprintf(stderr, "Failed to initialize\n"), 1);
#endif

	if (argc < 2)
	{
		fprintf(stderr, "Usage: web_get url\n");
		return (1);
	}
	char *url = argv[1];
	char *hostname, *port, *path;
	parse_url(url, &hostname, &port, &path);
	SOCKET server = connect_to_host(hostname, port);
	send_request(server, hostname, port, path);

	/* One feature is that it times out if a request takes too long to complete */
	/* In order to know how much time exactly, we record that time */
	const clock_t start_time = clock();

/* max size of HTTP response we reserve memory for */
/* If you want to take it further, it may be useful to think of using the heap ... (malloc) */
#define RESPONSE_SIZE 8192
	char response[RESPONSE_SIZE + 1];
	char *p = response, *q;
	char *end = response + RESPONSE_SIZE;
	char *body = 0;

	enum
	{
		length,
		chunked,
		connection
	};
	int encoding = 0, remaining = 0;

	while (98)
	{
		if ((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT)
		{
			fprintf(stderr, "Timeout after %.2f seconds\n", TIMEOUT);
			return (1);
		}
		if (p == end)
		{
			fprintf(stderr, "Out of buffer space\n");
			return (1);
		}
		/* Now let's code to receive data over TCP socket - uses `select()` */
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(server, &reads);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000;

		if (select(server + 1, &reads, 0, 0, &timeout) < 0)
		{
			fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
			return (1);
		}
		/* use FD_ISSET to check whether new data is available to be read */
		if (FD_ISSET(server, &reads))
		{
			int bytes_received = recv(server, p, end - p, 0);
			if (bytes_received < 1)
			{
				if (encoding == connection && body)
				{
					printf("%.*s", (int)(end - body), body);
				}
				printf("\nCOnnection closed by peer\n");
				break;
			}
			/* printf("received (%d) bytes: '%.*s'", bytes_received, bytes_received, p); */
			p += bytes_received;
			*p = 0;
			/* clever trick -> p advanced and set to '0' */
			/* Our received data always end with null terminator -> allows us to use std string fucntions */
		}
		if (!body && (body = strstr(response, "\r\n\r\n")))
		{
			*body = 0;
			body += 4;

			printf("Received headers:\n%s\n", response);

			q = strstr(response, "\nContent-Length: ");
			if (q)
			{
				encoding = length;
				q = strchr(q, ' ');
				q += 1;
				remaining = strtol(q, 0, 10);
			}
			else
			{
				q = strstr(response, "\nTransfer-Encoding: chunked");
				if (q)
				{
					encoding = chunked;
					remaining = 0;
				}
				else
				{
					encoding = connection;
				}
			}
			printf("\nReceived Body\n");
		}

		if (body)
		{
			if (encoding == length)
			{
				if (p - body >= remaining)
				{
					printf("%.*s", remaining, body);
					break;
				}
			}
			else if (encoding == chunked)
			{
				do
				{
					if (remaining == 0)
					{
						if ((q = strstr(body, "\r\n")))
						{
							remaining = strtol(body, 0, 16);
							if (!remaining)
								goto finish;
							body = q + 2;
						}
						else
						{
							break;
						}
					}
					if (remaining && p - body >= remaining)
					{
						printf("%.*s", remaining, body);
						body += remaining + 2;
						remaining = 0;
					}
				} while (!remaining);
			}
		}
	}

finish:
	printf("\nClosing socket...\n");
	CLOSESOCKET(server);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Job done\n");
	return (0);
}
