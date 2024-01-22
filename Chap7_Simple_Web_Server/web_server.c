#include "socket.h"

/**
 * get_content_type - Determine the proper media type
 * Not a perfect solution, but we keep it simple here
 * @path: filename to check the extension of
 * Return: the MIME type of the file
 */

const char *get_content_type(const char *path)
{
	const char *last_dot = strchr(path, '.');
	if (last_dot)
	{
		if (strcmp(last_dot, ".css") == 0)
			return "text/css";
		if (strcmp(last_dot, ".csv") == 0)
			return "text/csv";
		if (strcmp(last_dot, ".gif") == 0)
			return "image/gif";
		if (strcmp(last_dot, ".htm") == 0)
			return "text/html";
		if (strcmp(last_dot, ".html") == 0)
			return "text/html";
		if (strcmp(last_dot, ".ico") == 0)
			return "image/x-icon";
		if (strcmp(last_dot, ".jpeg") == 0)
			return "image/jpeg";
		if (strcmp(last_dot, ".jpg") == 0)
			return "image/jpeg";
		if (strcmp(last_dot, ".js") == 0)
			return "application/javascript";
		if (strcmp(last_dot, ".json") == 0)
			return "application/json";
		if (strcmp(last_dot, ".png") == 0)
			return "image/png";
		if (strcmp(last_dot, ".pdf") == 0)
			return "application/pdf";
		if (strcmp(last_dot, ".svg") == 0)
			return "image/svg+xml";
		if (strcmp(last_dot, ".txt") == 0)
			return "text/plain";
	}

	/* Default case */
	return "application/octect-stream";
}

/**
 * create_socket - Our HTTP server, like all server
 * needs to create a listening socket to accept new conns.
 * @host: Hostname that will listen
 * @port: Port to listen on
 * Return: a socket fd
 */

SOCKET create_socket(const char *host, const char *port)
{
	printf("Configuring local address, and creating socket...\n");

	struct addrinfo hints, *bind_addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; /* to be suitable for binding-accepting */

	getaddrinfo(host, port, &hints, &bind_addr);
	SOCKET listener;
	listener = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);

	if (!ISVALIDSOCKET(listener))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Binding socket to local address...\n");
	if (bind(listener, bind_addr->ai_addr, bind_addr->ai_addrlen))
	{
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	freeaddrinfo(bind_addr);

	printf("Listening...\n");
	if (listen(listener, 10) < 0)
	{
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	return (listener);
}

/**
 * main - Entry point of our server
 * Return: 1 on Failure, 0 for Success
 */

int main()
{

/* Windows Compatibility */
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif

	SOCKET server = create_socket(0, "8080");
	client_info_t *client_list = NULL;

	/* Endless loop to detect client's connection */
	while (98)
	{
		fd_set reads;
		reads = wait_on_client(&client_list, server);

		if (FD_ISSET(server, &reads))
		{
			/* Calling get_client() with invalid socket (-1), will cause the function to creates a new struct client_info_t */
			client_info_t *client = get_client(&client_list, -1);
			client->socket = accept(server, (struct sockaddr *)&(client->addr),
									&(client->addr_len));

			if (!ISVALIDSOCKET(client->socket))
			{
				fprintf(stderr, "accept() failed (%d)\n", GETSOCKETERRNO());
				return (1);
			}
			printf("New connection from %s\n", get_client_address(client));
		}

		/* Loop through linked list of connected clients, scanning which client has data to be sent */
		client_info_t *client = client_list;
		while (client)
		{
			client_info_t *next = client->next;
			if (FD_ISSET(client->socket, &reads))
			{
				if (MAX_REQUEST_SIZE == client->received)
				{
					send_400(&client_list, client);
					continue;
				}
				int r = recv(client->socket, client->req + client->received,
							 MAX_REQUEST_SIZE - client->received, 0);
				if (r < 1)
				{
					printf("Unexpected disconnect from %s.\n", get_client_address(client));
					drop_client(&client_list, client);
				}
				else
				{
					client->received += r;
					client->req[client->received] = '\0';

					char *q = strstr(client->req, "\r\n\r\n");
					if (q)
					{
						if (strncmp("GET /", client->req, 5))
						{
							send_400(&client_list, client);
						}
						else
						{
							char *path = client->req + 4; /* Fifth character located at +4 obviously */
							char *end_path = strstr(path, " ");
							if (!end_path)
							{
								send_400(&client_list, client);
							}
							else
							{
								*end_path = 0;
								serve_resource(&client_list, client, path);
							}
						}
					} /* if (q) */
				}
			}
			client = next;
		}
	} /* while (98) */

	printf("\nClosing socket, goodnight...\n");
	CLOSESOCKET(server);

#if defined(_WIN32)
	WSACleanup();
#endif
	printf("Finished\n");
	return (0);
}
