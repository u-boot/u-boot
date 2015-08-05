/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 *	SPDX-License-Identifier:	GPL-2.0
 */

#include "ping.h"
#include "arp.h"

static ushort ping_seq_number;

/* The ip address to ping */
struct in_addr net_ping_ip;

static void set_icmp_header(uchar *pkt, struct in_addr dest)
{
	/*
	 *	Construct an IP and ICMP header.
	 */
	struct ip_hdr *ip = (struct ip_hdr *)pkt;
	struct icmp_hdr *icmp = (struct icmp_hdr *)(pkt + IP_HDR_SIZE);

	net_set_ip_header(pkt, dest, net_ip);

	ip->ip_len   = htons(IP_ICMP_HDR_SIZE);
	ip->ip_p     = IPPROTO_ICMP;
	ip->ip_sum   = compute_ip_checksum(ip, IP_HDR_SIZE);

	icmp->type = ICMP_ECHO_REQUEST;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = htons(ping_seq_number++);
	icmp->checksum = compute_ip_checksum(icmp, ICMP_HDR_SIZE);
}

static int ping_send(void)
{
	uchar *pkt;
	int eth_hdr_size;

	/* XXX always send arp request */

	debug_cond(DEBUG_DEV_PKT, "sending ARP for %pI4\n", &net_ping_ip);

	net_arp_wait_packet_ip = net_ping_ip;

	eth_hdr_size = net_set_ether(net_tx_packet, net_null_ethaddr, PROT_IP);
	pkt = (uchar *)net_tx_packet + eth_hdr_size;

	set_icmp_header(pkt, net_ping_ip);

	/* size of the waiting packet */
	arp_wait_tx_packet_size = eth_hdr_size + IP_ICMP_HDR_SIZE;

	/* and do the ARP request */
	arp_wait_try = 1;
	arp_wait_timer_start = get_timer(0);
	arp_request();
	return 1;	/* waiting */
}

static void ping_timeout_handler(void)
{
	eth_halt();
	net_set_state(NETLOOP_FAIL);	/* we did not get the reply */
}

void ping_start(void)
{
	printf("Using %s device\n", eth_get_name());
	net_set_timeout_handler(10000UL, ping_timeout_handler);

	ping_send();
}

void ping_receive(struct ethernet_hdr *et, struct ip_udp_hdr *ip, int len)
{
	struct icmp_hdr *icmph = (struct icmp_hdr *)&ip->udp_src;
	struct in_addr src_ip;
	int eth_hdr_size;

	switch (icmph->type) {
	case ICMP_ECHO_REPLY:
		src_ip = net_read_ip((void *)&ip->ip_src);
		if (src_ip.s_addr == net_ping_ip.s_addr)
			net_set_state(NETLOOP_SUCCESS);
		return;
	case ICMP_ECHO_REQUEST:
		eth_hdr_size = net_update_ether(et, et->et_src, PROT_IP);

		debug_cond(DEBUG_DEV_PKT,
			   "Got ICMP ECHO REQUEST, return %d bytes\n",
			   eth_hdr_size + len);

		ip->ip_sum = 0;
		ip->ip_off = 0;
		net_copy_ip((void *)&ip->ip_dst, &ip->ip_src);
		net_copy_ip((void *)&ip->ip_src, &net_ip);
		ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);

		icmph->type = ICMP_ECHO_REPLY;
		icmph->checksum = 0;
		icmph->checksum = compute_ip_checksum(icmph, len - IP_HDR_SIZE);
		net_send_packet((uchar *)et, eth_hdr_size + len);
		return;
/*	default:
		return;*/
	}
}
