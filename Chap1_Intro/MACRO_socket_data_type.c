/**
 ********************************************
 * Windows sockets were modeled on Berkeley sockets
 * Therefore, many similarities, however, you should be aware that
 * **NIX platforms, socket descriptor -> std file descriptor (non negative and smalls)
 * WIndows platforms, socket handle can be anything, returns a SOCKET
 * SOCKET is a typedef for an 'unsigned int' in Winsock headers
 * Useful to code a typedef int SOCKET on non-Windows --> Portability
 */

#if !defined(_WIN32)
#define SOCKET int
#endif

/**********************
 * Invalid sockets => Windows retunrs INVALID_SOCKET
 * on Unix => Invalid socket returns negative num on failure
 * !! => pb pb as Windows SOCKET is unsigned
 * There again , check this macro
 */

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#endif

/********
 * Closing sockets
 * Unix => close() // WIndows => closesocket()
 * Abstract out this difference with a macro
 */

#if defined(_WIN32)
#define CLOSESOCKET(s) closesocket(s)
#else
#define CLOSESOCKET(s) close(s)
#endif

/***********
 * Error handling
 * Unix stores error of socket functions (socket, bind, listen, accept...)
 * in the thread-global 'errno'
 * On Windows => calling WSAGetLastError() instead
 * There again, abstract the difference using the macro
 */

#if defined(_WIN32)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define GETSOCKETERRNO() (errno)
#endif
