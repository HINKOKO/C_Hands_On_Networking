#include "socket.h"

/**
 * get_client - Searches through the Linked list of connected clients
 * @s: Socket descriptor
 * Return: The relevant client_info_t for the given SOCKET
 */

client_info_t *get_client(client_info_t **client_list, SOCKET s)
{
	client_info_t *ci = *client_list;
	while (ci)
	{
		if (ci->socket == s)
			break;
		ci = ci->next;
	}
	if (ci)
		return (ci);

	/* calloc comes in handy, since it zero's out the alloce'd struct for us */
	client_info_t *new = (client_info_t *)calloc(1, sizeof(client_info_t));
	if (!new)
	{
		fprintf(stderr, "Out of memory, get buy some!\n");
		exit(1);
	}
	new->addr_len = sizeof(new->addr);
	new->next = *client_list;
	*client_list = new;
	/* new becomes the root */
	return (new);
}

/**
 * drop_client - Loop through the Linked list and delete a given client
 * @client: the client to remove
 */

void drop_client(client_info_t **client_list, client_info_t *client)
{
	CLOSESOCKET(client->socket);

	client_info_t **p = client_list;

	while (*p)
	{
		if (*p == client)
		{
			*p = client->next;
			free(client);
			return;
		}
		p = &(*p)->next;
	}
	fprintf(stderr, "Client to be dropped not found\n");
	exit(1);
}

/**
 * get_client_address - Helper to convert client's IP address into text
 * @ci: Struct of client to be textify
 * Return: Readable/text buffered client address
 */

const char *get_client_address(client_info_t *ci)
{
	/* static -> ensures its memory is available after the function returns */
	/* cons -> not re-entrant safe */
	static char addr_buff[100];
	getnameinfo((struct sockaddr *)&ci->addr, ci->addr_len,
				addr_buff, sizeof(addr_buff), 0, 0, NI_NUMERICHOST);

	return (addr_buff);
}

/**
 * wait_on_client - Helper which blocks until an existing client sends data.
 * Or, a new client attempts to connect. We use our good old friend `select()`
 * @server: socket descriptor
 * Return: set of monitored socket descriptors, so caller can see which socket is ready
 */

fd_set wait_on_client(client_info_t **client_list, SOCKET server)
{
	fd_set reads;
	FD_ZERO(&reads);
	/* add first our initial socket descriptor 'server' to the set */
	FD_SET(server, &reads);
	SOCKET max_socket = server;

	client_info_t *ci = *client_list;
	while (ci)
	{
		FD_SET(ci->socket, &reads);
		if (ci->socket > max_socket)
			max_socket = ci->socket;
		ci = ci->next;
	}

	if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
	{
		fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	return (reads);
}

/**
 * send_400 - Send 400 Error when request is not understood
 * @client: client struct
 */

void send_400(client_info_t **client_list, client_info_t *client)
{
	const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
					   "Connection: close\r\n"
					   "Content-Length: 11\r\n\r\nBad Request";
	send(client->socket, c400, strlen(c400), 0);
	drop_client(client_list, client);
}

/**
 * send_404 - Send 404 Error when request is not understood
 * @client: client struct
 */

void send_404(client_info_t **client_list, client_info_t *client)
{
	const char *c404 = "HTTP/1.1 404 Not Found\r\n"
					   "Connection: close\r\n"
					   "Content-Length: 9\r\nNot Found";
	send(client->socket, c404, strlen(c404), 0);
	drop_client(client_list, client);
}
