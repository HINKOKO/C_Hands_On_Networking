#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * main - entry point
 * Pretty funny program that will actually output
 * as if you were running 'ipconfig' on Windows machine
 * or 'ifconfig' on ***Nix machines
 */

int main()
{
	struct ifaddrs *addrs, *addr = NULL;
	int family = 0;
	char ap[100];

	if (getifaddrs(&addrs) == -1)
	{
		printf("geifaddrs call failed\n");
		return (-1);
	}
	addr = addrs;
	while (addr)
	{
		family = addr->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6)
		{
			printf("%s\t", addr->ifa_name);
			printf("%s\t", family == AF_INET ? "Ipv4" : "IPv6");
			const int family_size = family == AF_INET
										? sizeof(struct sockaddr_in)
										: sizeof(struct sockaddr_in6);
			getnameinfo(addr->ifa_addr, (socklen_t)family_size, ap,
						sizeof(ap), 0, 0, NI_NUMERICHOST);
			printf("\t%s\n", ap);
		}
		addr = addr->ifa_next;
	}

	freeifaddrs(addrs);
	return (0);
}
