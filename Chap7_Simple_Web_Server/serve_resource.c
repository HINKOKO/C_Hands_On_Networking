#include "socket.h"

/**
 * serve_resource - sends a connected client the requested resource
 * @client: Connected client's infos
 * @path: Path to the resource requested
 * ==> Our server expects all hosted files to be under './public' directory
 * ==> Most 'real-world' server, or more advanced ones, would log the date, time, request method
 * client-user agent's tring, and response code as a minimum
 */

void serve_resource(client_info_t **client_list, client_info_t *client, const char *path)
{
	printf("Serving resource %s %s\n", get_client_address(client), path);

	/* Redirect root request, prevent too long or malicious request too (such as '../') */
	if (!strcmp(path, "/"))
		path = "/index.html";

	if (strlen(path) > 100)
	{
		send_400(client_list, client);
		return;
	}

	if (strstr(path, ".."))
	{
		send_404(client_list, client);
		return;
	}

	/* After those sanity checks, convert the path to refer to files in the public dir */
	char full_path[128];
	sprintf(full_path, "public%s", path);

/* Slash conversion for windows systems which are not doing automatic slash forward-Backward switch */
#if defined(_WIN32)
	char *p = full_path;
	while (*p)
	{
		if (*p == '/')
			*p = '\\';
		++p
	}
#endif

	FILE *fp = fopen(full_path, "rb");
	if (!fp)
	{
		send_404(client_list, client);
		return;
	}
	fseek(fp, 0L, SEEK_END);
	size_t cl = ftell(fp);
	rewind(fp);
	/* Get the type */
	const char *ct = get_content_type(full_path);

#define BSIZE 1024
	char buffer[BSIZE];

	/* Using sprintf() and send() in turn to print relevant header for HTTP response */
	sprintf(buffer, "HTTP/1.1 200 OK\r\n");
	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Connection: close\r\n");
	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Content-Length: %lu\r\n", cl);
	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Content-Type: %s\r\n", ct);
	send(client->socket, buffer, strlen(buffer), 0);

	/* Blank line to delineate the HTTP header from the beginning of Body */
	sprintf(buffer, "\r\n");
	send(client->socket, buffer, strlen(buffer), 0);

	int r = fread(buffer, 1, BSIZE, fp);
	while (r)
	{
		send(client->socket, buffer, r, 0);
		r = fread(buffer, 1, BSIZE, fp);
	}

	fclose(fp);
	drop_client(client_list, client);
}
