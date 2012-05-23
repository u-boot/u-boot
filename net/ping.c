/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

#include "ping.h"
#include "arp.h"

static ushort PingSeqNo;

/* The ip address to ping */
IPaddr_t NetPingIP;

static void set_icmp_header(uchar *pkt, IPaddr_t dest)
{
	/*
	 *	Construct an IP and ICMP header.
	 */
	struct ip_hdr *ip = (struct ip_hdr *)pkt;
	struct icmp_hdr *icmp = (struct icmp_hdr *)(pkt + IP_HDR_SIZE);

	net_set_ip_header(pkt, dest, NetOurIP);

	ip->ip_len   = htons(IP_ICMP_HDR_SIZE);
	ip->ip_p     = IPPROTO_ICMP;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE >> 1);

	icmp->type = ICMP_ECHO_REQUEST;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = htons(PingSeqNo++);
	icmp->checksum = ~NetCksum((uchar *)icmp, ICMP_HDR_SIZE	>> 1);
}

static int ping_send(void)
{
	static uchar mac[6];
	uchar *pkt;
	int eth_hdr_size;

	/* XXX always send arp request */

	memcpy(mac, NetEtherNullAddr, 6);

	debug("sending ARP for %pI4\n", &NetPingIP);

	NetArpWaitPacketIP = NetPingIP;
	NetArpWaitPacketMAC = mac;

	pkt = NetArpWaitTxPacket;
	eth_hdr_size = NetSetEther(pkt, mac, PROT_IP);
	pkt += eth_hdr_size;

	set_icmp_header(pkt, NetPingIP);

	/* size of the waiting packet */
	NetArpWaitTxPacketSize = eth_hdr_size + IP_ICMP_HDR_SIZE;

	/* and do the ARP request */
	NetArpWaitTry = 1;
	NetArpWaitTimerStart = get_timer(0);
	ArpRequest();
	return 1;	/* waiting */
}

static void ping_timeout(void)
{
	eth_halt();
	NetState = NETLOOP_FAIL;	/* we did not get the reply */
}

void ping_start(void)
{
	printf("Using %s device\n", eth_get_name());
	NetSetTimeout(10000UL, ping_timeout);

	ping_send();
}

void ping_receive(struct ethernet_hdr *et, struct ip_udp_hdr *ip, int len)
{
	struct icmp_hdr *icmph = (struct icmp_hdr *)&ip->udp_src;
	IPaddr_t src_ip;

	switch (icmph->type) {
	case ICMP_ECHO_REPLY:
		src_ip = NetReadIP((void *)&ip->ip_src);
		if (src_ip == NetPingIP)
			NetState = NETLOOP_SUCCESS;
		return;
	case ICMP_ECHO_REQUEST:
		debug("Got ICMP ECHO REQUEST, return "
			"%d bytes\n", ETHER_HDR_SIZE + len);

		memcpy(&et->et_dest[0], &et->et_src[0], 6);
		memcpy(&et->et_src[0], NetOurEther, 6);

		ip->ip_sum = 0;
		ip->ip_off = 0;
		NetCopyIP((void *)&ip->ip_dst, &ip->ip_src);
		NetCopyIP((void *)&ip->ip_src, &NetOurIP);
		ip->ip_sum = ~NetCksum((uchar *)ip,
				       IP_HDR_SIZE >> 1);

		icmph->type = ICMP_ECHO_REPLY;
		icmph->checksum = 0;
		icmph->checksum = ~NetCksum((uchar *)icmph,
			(len - IP_HDR_SIZE) >> 1);
		NetSendPacket((uchar *)et, ETHER_HDR_SIZE + len);
		return;
/*	default:
		return;*/
	}
}
