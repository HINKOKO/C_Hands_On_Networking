#include "smtp.h"
#include <ctype.h>
#include <stdarg.h>


/**
 * get_input - Prompt the user for input
 * @prompt: any
 * @buffer: any
 */

void get_input(const char *prompt, char *buffer)
{
	printf("%s", prompt);

	buffer[0] = 0;
	fgets(buffer, MAXINPUT, stdin);
	const int read = strlen(buffer);
	if (read > 0)
		buffer[read - 1] = 0;
	/* fgets() do not remove the newline character , we overwrite it */
}

/**
 * send_format - send formatted strings directly over the wires (network)
 * @server: socket descriptor
 * @text: formatted string
 */

void send_format(SOCKET server, const char *text, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	va_end(args);

	send(server, buffer, strlen(buffer), 0);

	/* 'C:' for 'client' */
	printf("C: %s", buffer);
}

/**
 * parse_response - parse and interpret the SMTP response, which start by 3 digits code
 * especially the multispanned lines from SMTP server.
 * @response: SMTP response
 * Return: 0 on failure or response code (converted to int)
 */

int parse_response(const char *response)
{
	const char *k = response;
	if (!k[0] || !k[1] || !k[2])
		return (0);

	for (; k[3]; ++k)
	{
		if (k == response || k[-1] == '\n')
		{
			if (isdigit(k[0]) && isdigit(k[1]) && isdigit(k[2]))
			{
				if (k[3] != '-') /* multiline SMTP convention indicated by dash '-' */
				{
					if (strstr(k, "\r\n"))
						return (strtol(k, 0, 10));
				}
			}
		}
	}
	return (0);
}

/**
 * wait_on_response - waits until a particular response code is received
 *
 */

void wait_on_response(SOCKET server, int expected)
{
	char response[MAXRESPONSE + 1]; /* For storing SMTP's response */
	char *p = response;
	char *end = response + MAXRESPONSE; /* End-pointer, useful to ensure no attemps to write past end of buffer */

	int code = 0;

	do {
		int bytes_received = recv(server, p, end - p, 0);
		if (bytes_received < 1) {
			fprintf(stderr, "Connection dropped\n");
			exit(1);
		}

		p += bytes_received;
		*p = 0; /* p incremented to data received + Null terminator settled */

		if (p == end) {
			fprintf(stderr, "Server response too large:\n");
			fprintf(stderr, "%s", response);
			exit(1);
		}

		code = parse_response(response);
	} while (code == 0);

	if (code != expected) {
		fprintf(stderr, "Error from server side.\n");
		fprintf(stderr, "%s", response);
		exit(1);
	}
	/* hence, 'S' for 'server' */
	printf("S: %s", response);
}
