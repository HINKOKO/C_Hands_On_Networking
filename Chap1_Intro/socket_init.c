/* Includes the needed socket API headers for each platform */
/* And properly initializes Winsock on WINDOWS too */

/* use of preprocessors to run proper code on Windows */
/* compared to Berkeley socket systems */
#if defined(_WIN32)
#ifndef _WIN32 _WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

#include <stdio.h>

int main()
{
#if defined(_W32)
	WSDATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize.\n");
		return (1);
	}
#endif

	printf("Ready to use socket API Dude!\n");

#if defined(_WIN32)
	WSACleanup();
#endif

	return (0);
}
