// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

/* Neighbour Discovery for IPv6 */

#include <common.h>
#include <net.h>
#include <net6.h>
#include <ndisc.h>

/* IPv6 destination address of packet waiting for ND */
struct in6_addr net_nd_sol_packet_ip6 = ZERO_IPV6_ADDR;
/* IPv6 address we are expecting ND advert from */
static struct in6_addr net_nd_rep_packet_ip6 = ZERO_IPV6_ADDR;
/* MAC destination address of packet waiting for ND */
uchar *net_nd_packet_mac;
/* pointer to packet waiting to be transmitted after ND is resolved */
uchar *net_nd_tx_packet;
static uchar net_nd_packet_buf[PKTSIZE_ALIGN + PKTALIGN];
/* size of packet waiting to be transmitted */
int net_nd_tx_packet_size;
/* the timer for ND resolution */
ulong net_nd_timer_start;
/* the number of requests we have sent so far */
int net_nd_try;

#define IP6_NDISC_OPT_SPACE(len) (((len) + 2 + 7) & ~7)

/**
 * ndisc_insert_option() - Insert an option into a neighbor discovery packet
 *
 * @ndisc:	pointer to ND packet
 * @type:	option type to insert
 * @data:	option data to insert
 * @len:	data length
 * Return: the number of bytes inserted (which may be >= len)
 */
static int
ndisc_insert_option(struct nd_msg *ndisc, int type, u8 *data, int len)
{
	int space = IP6_NDISC_OPT_SPACE(len);

	ndisc->opt[0] = type;
	ndisc->opt[1] = space >> 3;
	memcpy(&ndisc->opt[2], data, len);
	len += 2;

	/* fill the remainder with 0 */
	if (space - len > 0)
		memset(&ndisc->opt[len], '\0', space - len);

	return space;
}

/**
 * ndisc_extract_enetaddr() - Extract the Ethernet address from a ND packet
 *
 * Note that the link layer address could be anything but the only networking
 * media that u-boot supports is Ethernet so we assume we're extracting a 6
 * byte Ethernet MAC address.
 *
 * @ndisc:	pointer to ND packet
 * @enetaddr:	extracted MAC addr
 */
static void ndisc_extract_enetaddr(struct nd_msg *ndisc, uchar enetaddr[6])
{
	memcpy(enetaddr, &ndisc->opt[2], 6);
}

/**
 * ndisc_has_option() - Check if the ND packet has the specified option set
 *
 * @ip6:	pointer to IPv6 header
 * @type:	option type to check
 * Return: 1 if ND has that option, 0 therwise
 */
static int ndisc_has_option(struct ip6_hdr *ip6, __u8 type)
{
	struct nd_msg *ndisc = (struct nd_msg *)(((uchar *)ip6) + IP6_HDR_SIZE);

	if (ip6->payload_len <= sizeof(struct icmp6hdr))
		return 0;

	return ndisc->opt[0] == type;
}

static void ip6_send_ns(struct in6_addr *neigh_addr)
{
	struct in6_addr dst_adr;
	unsigned char enetaddr[6];
	struct nd_msg *msg;
	__u16 len;
	uchar *pkt;
	unsigned short csum;
	unsigned int pcsum;

	debug("sending neighbor solicitation for %pI6c our address %pI6c\n",
	      neigh_addr, &net_link_local_ip6);

	/* calculate src, dest IPv6 addr and dest Eth addr */
	ip6_make_snma(&dst_adr, neigh_addr);
	ip6_make_mult_ethdstaddr(enetaddr, &dst_adr);
	len = sizeof(struct icmp6hdr) + IN6ADDRSZ +
	    IP6_NDISC_OPT_SPACE(INETHADDRSZ);

	pkt = (uchar *)net_tx_packet;
	pkt += net_set_ether(pkt, enetaddr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_link_local_ip6, &dst_adr, PROT_ICMPV6,
			   IPV6_NDISC_HOPLIMIT, len);

	/* ICMPv6 - NS */
	msg = (struct nd_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_NDISC_NEIGHBOUR_SOLICITATION;
	msg->icmph.icmp6_code = 0;
	memset(&msg->icmph.icmp6_cksum, 0, sizeof(__be16));
	memset(&msg->icmph.icmp6_unused, 0, sizeof(__be32));

	/* Set the target address and llsaddr option */
	net_copy_ip6(&msg->target, neigh_addr);
	ndisc_insert_option(msg, ND_OPT_SOURCE_LL_ADDR, net_ethaddr,
			    INETHADDRSZ);

	/* checksum */
	pcsum = csum_partial((__u8 *)msg, len, 0);
	csum = csum_ipv6_magic(&net_link_local_ip6, &dst_adr,
			       len, PROT_ICMPV6, pcsum);
	msg->icmph.icmp6_cksum = csum;
	pkt += len;

	/* send it! */
	net_send_packet(net_tx_packet, (pkt - net_tx_packet));
}

static void
ip6_send_na(uchar *eth_dst_addr, struct in6_addr *neigh_addr,
	    struct in6_addr *target)
{
	struct nd_msg *msg;
	__u16 len;
	uchar *pkt;
	unsigned short csum;

	debug("sending neighbor advertisement for %pI6c to %pI6c (%pM)\n",
	      target, neigh_addr, eth_dst_addr);

	len = sizeof(struct icmp6hdr) + IN6ADDRSZ +
	    IP6_NDISC_OPT_SPACE(INETHADDRSZ);

	pkt = (uchar *)net_tx_packet;
	pkt += net_set_ether(pkt, eth_dst_addr, PROT_IP6);
	pkt += ip6_add_hdr(pkt, &net_link_local_ip6, neigh_addr,
			   PROT_ICMPV6, IPV6_NDISC_HOPLIMIT, len);

	/* ICMPv6 - NA */
	msg = (struct nd_msg *)pkt;
	msg->icmph.icmp6_type = IPV6_NDISC_NEIGHBOUR_ADVERTISEMENT;
	msg->icmph.icmp6_code = 0;
	memset(&msg->icmph.icmp6_cksum, 0, sizeof(__be16));
	memset(&msg->icmph.icmp6_unused, 0, sizeof(__be32));
	msg->icmph.icmp6_dataun.u_nd_advt.solicited = 1;
	msg->icmph.icmp6_dataun.u_nd_advt.override = 1;
	/* Set the target address and lltargetaddr option */
	net_copy_ip6(&msg->target, target);
	ndisc_insert_option(msg, ND_OPT_TARGET_LL_ADDR, net_ethaddr,
			    INETHADDRSZ);

	/* checksum */
	csum = csum_ipv6_magic(&net_link_local_ip6,
			       neigh_addr, len, PROT_ICMPV6,
			       csum_partial((__u8 *)msg, len, 0));
	msg->icmph.icmp6_cksum = csum;
	pkt += len;

	/* send it! */
	net_send_packet(net_tx_packet, (pkt - net_tx_packet));
}

void ndisc_request(void)
{
	if (!ip6_addr_in_subnet(&net_ip6, &net_nd_sol_packet_ip6,
				net_prefix_length)) {
		if (ip6_is_unspecified_addr(&net_gateway6)) {
			puts("## Warning: gatewayip6 is needed but not set\n");
			net_nd_rep_packet_ip6 = net_nd_sol_packet_ip6;
		} else {
			net_nd_rep_packet_ip6 = net_gateway6;
		}
	} else {
		net_nd_rep_packet_ip6 = net_nd_sol_packet_ip6;
	}

	ip6_send_ns(&net_nd_rep_packet_ip6);
}

int ndisc_timeout_check(void)
{
	ulong t;

	if (ip6_is_unspecified_addr(&net_nd_sol_packet_ip6))
		return 0;

	t = get_timer(0);

	/* check for NDISC timeout */
	if ((t - net_nd_timer_start) > NDISC_TIMEOUT) {
		net_nd_try++;
		if (net_nd_try >= NDISC_TIMEOUT_COUNT) {
			puts("\nNeighbour discovery retry count exceeded; "
			     "starting again\n");
			net_nd_try = 0;
			net_set_state(NETLOOP_FAIL);
		} else {
			net_nd_timer_start = t;
			ndisc_request();
		}
	}
	return 1;
}

void ndisc_init(void)
{
	net_nd_packet_mac = NULL;
	net_nd_tx_packet = NULL;
	net_nd_sol_packet_ip6 = net_null_addr_ip6;
	net_nd_rep_packet_ip6 = net_null_addr_ip6;
	net_nd_tx_packet_size = 0;
	net_nd_tx_packet = &net_nd_packet_buf[0] + (PKTALIGN - 1);
	net_nd_tx_packet -= (ulong)net_nd_tx_packet % PKTALIGN;
}

int ndisc_receive(struct ethernet_hdr *et, struct ip6_hdr *ip6, int len)
{
	struct icmp6hdr *icmp =
	    (struct icmp6hdr *)(((uchar *)ip6) + IP6_HDR_SIZE);
	struct nd_msg *ndisc = (struct nd_msg *)icmp;
	uchar neigh_eth_addr[6];

	switch (icmp->icmp6_type) {
	case IPV6_NDISC_NEIGHBOUR_SOLICITATION:
		debug("received neighbor solicitation for %pI6c from %pI6c\n",
		      &ndisc->target, &ip6->saddr);
		if (ip6_is_our_addr(&ndisc->target) &&
		    ndisc_has_option(ip6, ND_OPT_SOURCE_LL_ADDR)) {
			ndisc_extract_enetaddr(ndisc, neigh_eth_addr);
			ip6_send_na(neigh_eth_addr, &ip6->saddr,
				    &ndisc->target);
		}
		break;

	case IPV6_NDISC_NEIGHBOUR_ADVERTISEMENT:
		/* are we waiting for a reply ? */
		if (ip6_is_unspecified_addr(&net_nd_sol_packet_ip6))
			break;

		if ((memcmp(&ndisc->target, &net_nd_rep_packet_ip6,
			    sizeof(struct in6_addr)) == 0) &&
		    ndisc_has_option(ip6, ND_OPT_TARGET_LL_ADDR)) {
			ndisc_extract_enetaddr(ndisc, neigh_eth_addr);

			/* save address for later use */
			if (!net_nd_packet_mac)
				net_nd_packet_mac = neigh_eth_addr;

			/* modify header, and transmit it */
			memcpy(((struct ethernet_hdr *)net_nd_tx_packet)->et_dest,
			       neigh_eth_addr, 6);

			net_send_packet(net_nd_tx_packet,
					net_nd_tx_packet_size);

			/* no ND request pending now */
			net_nd_sol_packet_ip6 = net_null_addr_ip6;
			net_nd_tx_packet_size = 0;
			net_nd_packet_mac = NULL;
		}
		break;
	default:
		debug("Unexpected ICMPv6 type 0x%x\n", icmp->icmp6_type);
		return -1;
	}

	return 0;
}
