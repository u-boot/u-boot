/*
 * DNS support driver
 *
 * Copyright (c) 2008 Pieter Voorthuijsen <pieter.voorthuijsen@prodrive.nl>
 * Copyright (c) 2009 Robin Getz <rgetz@blackfin.uclinux.org>
 *
 * This is a simple DNS implementation for U-Boot. It will use the first IP
 * in the DNS response as NetServerIP. This can then be used for any other
 * network related activities.
 *
 * The packet handling is partly based on TADNS, original copyrights
 * follow below.
 *
 */

/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include <common.h>
#include <command.h>
#include <net.h>

#include "dns.h"

char *NetDNSResolve;	/* The host to resolve  */
char *NetDNSenvvar;	/* The envvar to store the answer in */

static int DnsOurPort;

static void
DnsSend(void)
{
	struct header *header;
	int n, name_len;
	uchar *p, *pkt;
	const char *s;
	const char *name;
	enum dns_query_type qtype = DNS_A_RECORD;

	name = NetDNSResolve;
	pkt = p = (uchar *)(NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE);

	/* Prepare DNS packet header */
	header           = (struct header *) pkt;
	header->tid      = 1;
	header->flags    = htons(0x100);	/* standard query */
	header->nqueries = htons(1);		/* Just one query */
	header->nanswers = 0;
	header->nauth    = 0;
	header->nother   = 0;

	/* Encode DNS name */
	name_len = strlen(name);
	p = (uchar *) &header->data;	/* For encoding host name into packet */

	do {
		s = strchr(name, '.');
		if (!s)
			s = name + name_len;

		n = s - name;			/* Chunk length */
		*p++ = n;			/* Copy length  */
		memcpy(p, name, n);		/* Copy chunk   */
		p += n;

		if (*s == '.')
			n++;

		name += n;
		name_len -= n;
	} while (*s != '\0');

	*p++ = 0;			/* Mark end of host name */
	*p++ = 0;			/* Some servers require double null */
	*p++ = (unsigned char) qtype;	/* Query Type */

	*p++ = 0;
	*p++ = 1;				/* Class: inet, 0x0001 */

	n = p - pkt;				/* Total packet length */
	debug("Packet size %d\n", n);

	DnsOurPort = random_port();

	NetSendUDPPacket(NetServerEther, NetOurDNSIP, DNS_SERVICE_PORT,
		DnsOurPort, n);
	debug("DNS packet sent\n");
}

static void
DnsTimeout(void)
{
	puts("Timeout\n");
	NetState = NETLOOP_FAIL;
}

static void
DnsHandler(uchar *pkt, unsigned dest, unsigned src, unsigned len)
{
	struct header *header;
	const unsigned char *p, *e, *s;
	u16 type, i;
	int found, stop, dlen;
	char IPStr[22];
	IPaddr_t IPAddress;
	short tmp;


	debug("%s\n", __func__);
	if (dest != DnsOurPort)
		return;

	for (i = 0; i < len; i += 4)
		debug("0x%p - 0x%.2x  0x%.2x  0x%.2x  0x%.2x\n",
			pkt+i, pkt[i], pkt[i+1], pkt[i+2], pkt[i+3]);

	/* We sent 1 query. We want to see more that 1 answer. */
	header = (struct header *) pkt;
	if (ntohs(header->nqueries) != 1)
		return;

	/* Received 0 answers */
	if (header->nanswers == 0) {
		puts("DNS server returned no answers\n");
		NetState = NETLOOP_SUCCESS;
		return;
	}

	/* Skip host name */
	s = &header->data[0];
	e = pkt + len;
	for (p = s; p < e && *p != '\0'; p++)
		continue;

	/* We sent query class 1, query type 1 */
	tmp = p[1] | (p[2] << 8);
	if (&p[5] > e || ntohs(tmp) != DNS_A_RECORD) {
		puts("DNS response was not A record\n");
		NetState = NETLOOP_SUCCESS;
		return;
	}

	/* Go to the first answer section */
	p += 5;

	/* Loop through the answers, we want A type answer */
	for (found = stop = 0; !stop && &p[12] < e; ) {

		/* Skip possible name in CNAME answer */
		if (*p != 0xc0) {
			while (*p && &p[12] < e)
				p++;
			p--;
		}
		debug("Name (Offset in header): %d\n", p[1]);

		tmp = p[2] | (p[3] << 8);
		type = ntohs(tmp);
		debug("type = %d\n", type);
		if (type == DNS_CNAME_RECORD) {
			/* CNAME answer. shift to the next section */
			debug("Found canonical name\n");
			tmp = p[10] | (p[11] << 8);
			dlen = ntohs(tmp);
			debug("dlen = %d\n", dlen);
			p += 12 + dlen;
		} else if (type == DNS_A_RECORD) {
			debug("Found A-record\n");
			found = stop = 1;
		} else {
			debug("Unknown type\n");
			stop = 1;
		}
	}

	if (found && &p[12] < e) {

		tmp = p[10] | (p[11] << 8);
		dlen = ntohs(tmp);
		p += 12;
		memcpy(&IPAddress, p, 4);

		if (p + dlen <= e) {
			ip_to_string(IPAddress, IPStr);
			printf("%s\n", IPStr);
			if (NetDNSenvvar)
				setenv(NetDNSenvvar, IPStr);
		} else
			puts("server responded with invalid IP number\n");
	}

	NetState = NETLOOP_SUCCESS;
}

void
DnsStart(void)
{
	debug("%s\n", __func__);

	NetSetTimeout(DNS_TIMEOUT, DnsTimeout);
	NetSetHandler(DnsHandler);

	DnsSend();
}
