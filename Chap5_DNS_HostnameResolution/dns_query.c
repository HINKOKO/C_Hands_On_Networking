#include "socket_macro.h"

/**
 * Utiliy program to send DNS queries to a DNS server and receive DNS reponse.
 * Shouldn't be normally needed in the field of planet Earth, However, it is a
 * good opportunity to better understand the DNS protocol, and get experience of sending
 * binary UDP packet !
 *
 * DNS Encoding is particular:
 * - Each label indicated by its length, followed by its text, A number of labels
 * can be repeated, and then the name is terminated with a single 0 byte.
 * - If  a length has its two highest bits set (that is , 0xc0) then it and the next byte
 * should be interpreted as a POINTER instead.
 * --> Must be aware too, at any time our DNS response could be ill-formed or corrupted,
 * try to program in such a way it won't crash if it receives bad messages. (easier said than done)
 * ==== const unsigned char *print_name(const unsigned char *msg,
 * 	const unsigned char *p, const unsigned char *end)
 * @msg: pointer to message's beginning
 * @p: Pointer to the name to print
 * @end: pointer to one past the end of the message (required to check we're not
 * reading past the end of the message)
 */

const unsigned char *print_name(const unsigned char *msg,
								const unsigned char *p, const unsigned char *end)
{
	/* Proper name is even possible ? because a name should consist: */
	/* at least a length and some text, we can return Error if 'p' is already */
	/* within 2 characters of the end */
	if (p + 2 > end)
	{
		fprintf(stderr, "End of message.\n");
		exit(1);
	}
	/* Check to see if 'p' points to a name pointer */
	/* Interpret the pointer and call 'print_name' recursively to print the name it points to */
	/* Binary for 0xC0 == 0b11000000 */
	if ((*p & 0xC0) == 0xC0)
	{
		const int k = ((*p & 0x3F) << 8) + p[1];
		p += 2;
		printf(" (pointer %d) ", k);
		print_name(msg, msg + k, end);
		return (p);
	}
	else
	{
		/* if not a pointer, print one label at a time */
		const int len = *p++;
		if (p + len + 1 > end)
		{
			fprintf(stderr, "End of message.\n");
			exit(1);
		}
		printf("%.*s", len, p);
		p += len;
		if (*p)
		{
			/* print a dot to sep the labels */
			printf(".");
			return (print_name(msg, p, end));
		}
		else
		{
			return (p + 1);
		}
	}
}

/**
 * Printing a DNS message to the screen
 * DNS shares same format for both request and response, so our function
 * is able to print either
 * @message: Pointer to the start of message
 * @msg_len: message len for sure
 */

void print_dns_message(const char *message, int msg_len)
{
	/* Recall => DNS header is 12 bytes long */
	if (msg_len < 12)
	{
		fprintf(stderr, "Message too short to be valid\n");
		exit(1);
	}
	/* Copy the message pointer into new variable, (unsigned char pointer -> certain calculation easier)*/
	const unsigned char *msg = (const unsigned char *)message;
	/* Raw response full: for curious people, you can uncomment the following code (many lines will be printed, can be annoying)*/
	/*int i;
	for (i = 0; i < msg_len; ++i) {
		unsigned char r = msg[i];
		printf("%02d:	%02X   %03d  '%c'\n, i, r, r, r");
	}
	printf("\n") */

	/* Message ID : first 2 bytes */
	printf("ID = %0X %0X\n", msg[0], msg[1]);
	/* Determine with bitmask if msg is a response or a query */
	const int qr = (msg[2] & 0x80) >> 7;
	printf("QR = %d %s\n", qr, qr ? "response" : "query");
	/* Much same way for OPCODE, AA, TC, and RD fields */
	const int opcode = (msg[2] & 0x78) >> 3;
	printf("OPCODE = %d", opcode);
	switch (opcode)
	{
	case 0:
		printf("standard\n");
		break;
	case 1:
		printf("reverse\n");
		break;
	case 2:
		printf("status\n");
		break;
	default:
		printf("?\n");
		break;
	}

	const int aa = (msg[2] & 0x04) >> 2;
	printf("AA = %d %s\n", aa, aa ? "authorative" : "");

	const int tc = (msg[2] & 0x02) >> 1;
	printf("TC = %d %s\n", tc, tc ? "message truncated" : "");

	const int rd = (msg[2] & 0x01);
	printf("RD = %d %s\n", rd, rd ? "recursion desired" : "");
	/* Read in RCODE for response-type message */
	if (qr)
	{
		const int rcode = msg[3] & 0x07;
		printf("RCODE = %d", rcode);
		switch (rcode)
		{
		case 0:
			printf("success\n");
			break;
		case 1:
			printf("format error\n");
			break;
		case 2:
			printf("server failure\n");
			break;
		case 3:
			printf("name error\n");
			break;
		case 4:
			printf("not implemented\n");
			break;
		case 5:
			printf("refused\n");
			break;
		default:
			printf("?\n");
			break;
		}
		if (rcode != 0)
			return;
	}
	/* Next 4 fields in the header --> Question count, answer count, name server count, additional count */
	const int qdcount = (msg[4] << 8) + msg[5];
	const int ancount = (msg[6] << 8) + msg[7];
	const int nscount = (msg[8] << 8) + msg[9];
	const int arcount = (msg[10] << 8) + msg[11];

	printf("QDCOUNT = %d\n", qdcount);
	printf("ANCOUNT = %d\n", ancount);
	printf("NSCOUNT = %d\n", nscount);
	printf("ARCOUNT = %d\n", arcount);

	/* HERE -- That concludes Reading the first 12 bytes HEADER */
	/* =======================================================  */

	const unsigned char *p = msg + 12;
	/* set `end` variable one past the end of message */
	const unsigned char *end = msg + msg_len;
	if (qdcount)
	{
		int i;
		for (i = 0; i < qdcount; i++)
		{
			if (p >= end)
			{
				fprintf(stderr, "End of message.\n");
				exit(1);
			}

			printf("Query %2d\n", i + 1);
			printf("  name:  ");
			p = print_name(msg, p, end);
			printf("\n");

			if (p + 4 > end)
			{
				fprintf(stderr, "End of message.\n");
				exit(1);
			}
			const int type = (p[0] << 8) + p[1];
			printf("  type: %d\n", type);
			p += 2;

			const int qclass = (p[0] << 8) + p[1];
			printf("  class: %d\n", qclass);
			p += 2;
		}
	}

	/* Reading name, type and class for answer, authority and adds sections */
	if (ancount || nscount || arcount)
	{
		int i;
		for (i = 0; i < ancount + nscount + arcount; i++)
		{
			if (p >= end)
				fprintf(stderr, "End of message.\n"); exit(1);
			printf("Answer %2d\n", i + 1);
			printf("  name: ");
			p = print_name(msg, p, end); printf("\n");
			if (p + 10  > end)
			{
				fprintf(stderr, "End of message.\n");
				exit(1);
			}
			const int type = (p[0] << 8) + p[1];
			printf("  type: %d\n", type);
			p += 2;
			const int qclass = (p[0] << 8) + p[1];
			printf("  class: %d\n", qclass);
			p += 2;
			/* TTL field -> tells us how many seconds we are allowed to cache an answer for */
			const unsigned int ttl = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
			printf("   ttl: %u\n", ttl);
			p += 4;

			const int rdlen = (p[0] << 8) + p[1];
			printf(" rdlen: %d\n", rdlen);
			p += 2;
			if (p + rdlen > end)
			{
				fprintf(stderr, "End of message.\n");
				exit(1);
			}
			/* After this sanity check , interpret data */
			if (rdlen == 4 && type == 1) /* A record */
			{
				printf("Address: ");
				printf("%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
			}
			else if (type == 15 && rdlen > 3) /* MX record */
			{
				const int preference = (p[0] << 8) + p[1];
				printf("  pref: %d\n", preference);
				printf("MX: ");
				print_name(msg, p + 2, end); printf("\n");
			}
			else if (rdlen == 16 && type == 28) /* AAAA record */
			{
				printf("Address ");
				int j;
				for (j = 0; j < rdlen; j += 2)
				{
					printf("%02x%02x", p[j], p[j + 1]);
					if (j + 2 < rdlen) printf(":");
				}
				printf("\n");
			}
			else if (type == 16) /* TXT record */
				printf("TXT: '%.*s'\n", rdlen - 1, p + 1);
			else if (type == 5) /* CNAME record */
			{
				printf("CNAME: ");
				print_name(msg, p, end); printf("\n");
			}
			p += rdlen;
		}
	}
	/* Check if all data was read properly */
	if (p != end)
		printf("There is some unread data left over.\n");
	
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage:\n\t[dns_query] [hostname] [type]\n");
		fprintf(stderr, "Example:\n\tdns_query example.com aaaa\n");
		exit(0);
	}

	if (strlen(argv[1]) > 255)
	{
		fprintf(stderr, "Hostname too long");
		exit(1);
	}
	unsigned char type;
	if (!strcmp(argv[2], "a"))
		type = 1;
	else if (!strcmp(argv[2], "mx"))
		type = 15;
	else if (!strcmp(argv[2], "txt"))
		type = 16;
	else if (!strcmp(argv[2], "aaaa"))
		type = 28;
	else if (!strcmp(argv[2], "any"))
		type = 255;
	else 
	{
		fprintf(stderr, "Unknow type '%s', Use a, aaaa, txt, mx or any.", argv[2]);
		exit(1);
	}

#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		fprintf(stderr, "Failed to initialize\n");
		return 1;
	}
#endif
 
	/* Connect to public DNS server 8.8.8.8 */
	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	struct addrinfo *peer;
	if (getaddrinfo("8.8.8.8", "53", &hints, &peer)) {
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return (1);
	}
	printf("Creating socket...\n");
	SOCKET sock_peer;
	sock_peer = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);
	if (!ISVALIDSOCKET(sock_peer)) {
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return (1);
	}
	/* lets construct with C the data for the DNS query message ! */
	char query[1024] = {0xAB, 0xCD, /* ID */
						0x01, 0x00, /* Set recursion  */
						0x00, 0x01, /* QDCOUNT (1) */
						0x00, 0x00, /* ANCOUNT */
						0x00, 0x00, /* ARCOUNT */
	};

	/* Encode user desired hostname into the query */
	char *p = query + 12;
	char *h = argv[1];

	while (*h) {
		char *len = p;
		p++;
		if (h != argv[1]) ++h;

		while (*h && *h != '.') *p++ = *h++;
		*len = p - len - 1;
	}
	*p++ = 0;
	/* Add question type/class to query manually */
	*p++ = 0x00; *p++ = type;
	*p++ = 0x00; *p++ = 0x01;
	const int query_size = p - query;
	int bytes_sent = sendto(sock_peer, query, query_size,
		0, peer->ai_addr, peer->ai_addrlen);
	printf("Send %d bytes\n", bytes_sent);
	/*  for debugging purposes, print the query with our print_name function */
	print_dns_message(query, query_size);

	char read[1024];
	int bytes_received = recvfrom(sock_peer, read, 1024, 0, 0, 0);
	printf("Received %d bytes\n", bytes_received);
	print_dns_message(read, bytes_received);
	printf("\n");

	freeaddrinfo(peer);
	CLOSESOCKET(sock_peer);

#if defined(_WIN32)
	WSACleanup();
#endif
	return (0);
}
/**
 * dns_query - A 'handmade' DNs question for "example.com" in C
 * This data could be sent as is to a DNS Server over port 53
 */

// char dns_query[] = {
// 	0xAB, 0xCD,							  /* ID */
// 	0x01, 0x00,							  /* Recursion */
// 	0x00, 0x01,							  /* QDCOUNT */
// 	0x00, 0x00,							  /* ANCOUNT */
// 	0x00, 0x00,							  /* NSCOUNT */
// 	0x00, 0x00,							  /* ARCOUNT */
// 	7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', /* label */
// 	3, 'c', 'o', 'm'					  /* label */
// 	0,									  /* end of name */
// 	0x00, 0x01,							  /* QTYPE = A */
// 	0x00, 0x01							  /* QCLASS */
// };
