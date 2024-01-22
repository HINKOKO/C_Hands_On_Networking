/* separate header file with reusable macros */
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws-32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

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
#include <stdlib.h>
#include <string.h>

#define MAX_REQUEST_SIZE 2047

/**
 * struct client_info_s - To store infos about each connected client
 * @addr_len: Len of connected address
 * @addr: Client's address
 * @socket: Socket used
 * @req: All data received so far from client stored in that array
 * @received: Number of bytes stored in array 'req'
 * @next: Pointer to next client info struct
 */

typedef struct client_info_s
{
	socklen_t addr_len;
	struct sockaddr_storage addr;
	SOCKET socket;
	char req[MAX_REQUEST_SIZE + 1];
	int received;
	struct client_info_s *next;
} client_info_t;

/* Helper functions that works on client_info_t structures and clients linked list */
const char *get_content_type(const char *path);
client_info_t *get_client(client_info_t **client, SOCKET s);
void drop_client(client_info_t **client_list, client_info_t *client);
const char *get_client_address(client_info_t *ci);
fd_set wait_on_client(client_info_t **client_list, SOCKET server);
void send_400(client_info_t **client_list, client_info_t *client);
void send_404(client_info_t **client_list, client_info_t *client);
/* The actual 'server' of resource function */
void serve_resource(client_info_t **client_list, client_info_t *client, const char *path);
