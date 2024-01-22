#include "smtp.h"
#include <time.h>


int main()
{
/* Windows OS */
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif

	char hostname[MAXINPUT];
	get_input("mail server: ", hostname);
	printf("Connecting to host: %s: 25\n", hostname);

	SOCKET server = connect_to_host(hostname, "25");
	/* Connection is now established, our client must not issue any commands until server */
	/* fires back with a '220' code */
	wait_on_response(server, 220);

	/* We then politely say 'HELO' standard SMTP */
	send_format(server, "HELO HONPWC\r\n");
	wait_on_response(server, 250);

	char sender[MAXINPUT];
	get_input("from: ", sender);
	send_format(server, "MAIL FROM:<%s>\r\n", sender);
	wait_on_response(server, 250);

	char recipient[MAXINPUT];
	get_input("to: ", recipient);
	send_format(server, "RCPT TO<%s>\r\n", recipient);
	wait_on_response(server, 250);

	/* Next step -> issue the 'DATA' command, which instruct server to 'listen' for the core stuff: email */
	send_format(server, "DATA\r\n");
	wait_on_response(server, 354);

	/* Prompt the user for an email subject line */
	char subject[MAXINPUT];
	get_input("subject: ", subject);
	send_format(server, "From:<%s>\r\n", sender);
	send_format(server, "To:<%s>\r\n", recipient);
	send_format(server, "Subject:<%s>\r\n", subject);

	time_t timer;
	time(&timer);

	struct tm *timeinfo;
	timeinfo = gmtime(&timer);

	char date[128];
	strftime(date, 128, "%a, %d %b %Y %H:%M:%S +0000", timeinfo);

	send_format(server, "Date:%s\r\n", date);
	send_format(server, "\r\n"); /* To delineate header from body */

	/* Prompt the user for the actual 'body' of its email */
	printf("Enter your email text, end with \".\" on a line by itself \n");

	while (98) {
		char body[MAXINPUT];
		get_input("> ", body);
		send_format(server, "%s\r\n", body);
		if (!strcmp(body, "."))
			break;
	}

	wait_on_response(server, 250);
	send_format(server, "QUIT\r\n");
	wait_on_response(server, 221);
	
	printf("Closing socket...\n");
	CLOSESOCKET(server);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished\n");
	return (0);
}
