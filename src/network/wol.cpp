/*****************************************************************************
 *
 * wol.c - Wake-On-LAN utility to wake a networked PC
 *
 * by R. Edwards (bob@cs.anu.edu.au), January 2000
 * (in_ether routine adapted from net-tools-1.51/lib/ether.c by
 * Fred N. van Kempen)
 * 
 * This utility allows a PC with WOL configured to be powered on by
 * sending a "Magic Packet" to it's network adaptor (see:
 * http://www.amd.com/products/npd/overview/20212.html).
 * Only the ethernet MAC address needs to be given to make this work.
 *
 * Current version uses a UDP broadcast to send out the Magic Packet.
 * A future version will use the raw packet interface to unicast the
 * packet only to the target machine.
 *
 * compile with: gcc -Wall -o wol wol.c
 * 
 * usage: wol <MAC address>
 * where <MAC address> is in xx:xx:xx:xx:xx:xx format.
 *
 * Released under GNU Public License January, 2000.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <netinet/in.h>

/* Input an Ethernet address and convert to binary. */
static int in_ether (const char *bufp, unsigned char *addr)
{
	char c;
	int i;
	unsigned char *ptr = addr;
	unsigned val;

	i = 0;
	
#ifdef DEBUG
	const char* orig = bufp;
#endif
	while ((*bufp != '\0') && (i < ETH_ALEN)) {
		val = 0;
		c = *bufp++;
		if (isdigit(c)) {
			val = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			val = c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			val = c - 'A' + 10;
		} else {
		#ifdef DEBUG
			fprintf(stderr, "in_ether(%s): invalid ether address!\n", orig);
		#endif
			errno = EINVAL;
			return (-1);
		}
		
		val <<= 4;
		c = *bufp;
		
		if (isdigit(c)) {
			val |= c - '0';
		} else if (c >= 'a' && c <= 'f') {
			val |= c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			val |= c - 'A' + 10;
		} else if (c == ':' || c == 0) {
			val >>= 4;
		} else {
			#ifdef DEBUG
				fprintf(stderr, "in_ether(%s): invalid ether address!\n", orig);
			#endif
				errno = EINVAL;
				return (-1);
		}
		
		if (c != 0) {
			bufp++;
		}
		
		*ptr++ = (unsigned char) (val & 0377);
		i++;

		/* We might get a semicolon here - not required. */
		if (*bufp == ':') {
			if (i == ETH_ALEN) {
				/* nothing */
			}
			bufp++;
		}
	}
	return (0);
}


int sendWol(const char* addr)
{
	int i, j;
	int packet;
	struct sockaddr_in sap;
	unsigned char ethaddr[8];
	unsigned char *ptr;
	unsigned char buf [128];
	int optval = 1;


	/* Fetch the hardware address. */
	if (in_ether (addr, ethaddr) < 0) {
		fprintf (stderr, "wol.c: invalid hardware address\n");
		return (-1);
	}

        /* setup the packet socket */
//      if ((packet = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_802_3))) < 0)
//{
	if ((packet = socket (PF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf (stderr, "wol.c: socket failed\n");
		return (-1);
	}

	/* Set socket options */
	if (setsockopt (packet, SOL_SOCKET, SO_BROADCAST, (caddr_t) &optval, sizeof (optval)) < 0) {
		fprintf (stderr, "wol.c: setsocket failed %s\n", strerror (errno));
		close (packet);
		return (-1);
	}

	/* Set up broadcast address */
	sap.sin_family = AF_INET;
	sap.sin_addr.s_addr = htonl(0xffffffff);        /* broadcast address */
	sap.sin_port = htons(60000);

	/* Build the message to send - 6 x 0xff then 16 x MAC address */
	ptr = buf;
	for (i = 0; i < 6; i++)
		*ptr++ = 0xff;
	for (j = 0; j < 16; j++)
		for (i = 0; i < ETH_ALEN; i++)
			*ptr++ = ethaddr [i];

	/* Send the packet out */
	if (sendto (packet, buf, 102, 0, (struct sockaddr *) &sap, sizeof (sap)) < 0) {
		fprintf (stderr, "wol.c: sendto failed, %s\n", strerror(errno));
		close (packet);
	return (-1);
	}

	close (packet);     
	return (0);
}