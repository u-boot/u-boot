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

#include <common.h>

#include "arp.h"

#ifndef	CONFIG_ARP_TIMEOUT
/* Milliseconds before trying ARP again */
# define ARP_TIMEOUT		5000UL
#else
# define ARP_TIMEOUT		CONFIG_ARP_TIMEOUT
#endif


#ifndef	CONFIG_NET_RETRY_COUNT
# define ARP_TIMEOUT_COUNT	5	/* # of timeouts before giving up  */
#else
# define ARP_TIMEOUT_COUNT	CONFIG_NET_RETRY_COUNT
#endif

struct in_addr net_arp_wait_packet_ip;
static struct in_addr net_arp_wait_reply_ip;
/* MAC address of waiting packet's destination */
uchar	       *NetArpWaitPacketMAC;
int		NetArpWaitTxPacketSize;
ulong		NetArpWaitTimerStart;
int		NetArpWaitTry;

static uchar   *NetArpTxPacket;	/* THE ARP transmit packet */
static uchar	NetArpPacketBuf[PKTSIZE_ALIGN + PKTALIGN];

void ArpInit(void)
{
	/* XXX problem with bss workaround */
	NetArpWaitPacketMAC = NULL;
	net_arp_wait_packet_ip.s_addr = 0;
	net_arp_wait_reply_ip.s_addr = 0;
	NetArpWaitTxPacketSize = 0;
	NetArpTxPacket = &NetArpPacketBuf[0] + (PKTALIGN - 1);
	NetArpTxPacket -= (ulong)NetArpTxPacket % PKTALIGN;
}

void arp_raw_request(struct in_addr source_ip, const uchar *targetEther,
	struct in_addr target_ip)
{
	uchar *pkt;
	struct arp_hdr *arp;
	int eth_hdr_size;

	debug_cond(DEBUG_DEV_PKT, "ARP broadcast %d\n", NetArpWaitTry);

	pkt = NetArpTxPacket;

	eth_hdr_size = NetSetEther(pkt, NetBcastAddr, PROT_ARP);
	pkt += eth_hdr_size;

	arp = (struct arp_hdr *) pkt;

	arp->ar_hrd = htons(ARP_ETHER);
	arp->ar_pro = htons(PROT_IP);
	arp->ar_hln = ARP_HLEN;
	arp->ar_pln = ARP_PLEN;
	arp->ar_op = htons(ARPOP_REQUEST);

	memcpy(&arp->ar_sha, NetOurEther, ARP_HLEN);	/* source ET addr */
	net_write_ip(&arp->ar_spa, source_ip);		/* source IP addr */
	memcpy(&arp->ar_tha, targetEther, ARP_HLEN);	/* target ET addr */
	net_write_ip(&arp->ar_tpa, target_ip);		/* target IP addr */

	NetSendPacket(NetArpTxPacket, eth_hdr_size + ARP_HDR_SIZE);
}

void ArpRequest(void)
{
	if ((net_arp_wait_packet_ip.s_addr & net_netmask.s_addr) !=
	    (net_ip.s_addr & net_netmask.s_addr)) {
		if (net_gateway.s_addr == 0) {
			puts("## Warning: gatewayip needed but not set\n");
			net_arp_wait_reply_ip = net_arp_wait_packet_ip;
		} else {
			net_arp_wait_reply_ip = net_gateway;
		}
	} else {
		net_arp_wait_reply_ip = net_arp_wait_packet_ip;
	}

	arp_raw_request(net_ip, NetEtherNullAddr, net_arp_wait_reply_ip);
}

void ArpTimeoutCheck(void)
{
	ulong t;

	if (!net_arp_wait_packet_ip.s_addr)
		return;

	t = get_timer(0);

	/* check for arp timeout */
	if ((t - NetArpWaitTimerStart) > ARP_TIMEOUT) {
		NetArpWaitTry++;

		if (NetArpWaitTry >= ARP_TIMEOUT_COUNT) {
			puts("\nARP Retry count exceeded; starting again\n");
			NetArpWaitTry = 0;
			NetStartAgain();
		} else {
			NetArpWaitTimerStart = t;
			ArpRequest();
		}
	}
}

void ArpReceive(struct ethernet_hdr *et, struct ip_udp_hdr *ip, int len)
{
	struct arp_hdr *arp;
	struct in_addr reply_ip_addr;
	uchar *pkt;
	int eth_hdr_size;

	/*
	 * We have to deal with two types of ARP packets:
	 * - REQUEST packets will be answered by sending  our
	 *   IP address - if we know it.
	 * - REPLY packates are expected only after we asked
	 *   for the TFTP server's or the gateway's ethernet
	 *   address; so if we receive such a packet, we set
	 *   the server ethernet address
	 */
	debug_cond(DEBUG_NET_PKT, "Got ARP\n");

	arp = (struct arp_hdr *)ip;
	if (len < ARP_HDR_SIZE) {
		printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
		return;
	}
	if (ntohs(arp->ar_hrd) != ARP_ETHER)
		return;
	if (ntohs(arp->ar_pro) != PROT_IP)
		return;
	if (arp->ar_hln != ARP_HLEN)
		return;
	if (arp->ar_pln != ARP_PLEN)
		return;

	if (net_ip.s_addr == 0)
		return;

	if (net_read_ip(&arp->ar_tpa).s_addr != net_ip.s_addr)
		return;

	switch (ntohs(arp->ar_op)) {
	case ARPOP_REQUEST:
		/* reply with our IP address */
		debug_cond(DEBUG_DEV_PKT, "Got ARP REQUEST, return our IP\n");
		pkt = (uchar *)et;
		eth_hdr_size = net_update_ether(et, et->et_src, PROT_ARP);
		pkt += eth_hdr_size;
		arp->ar_op = htons(ARPOP_REPLY);
		memcpy(&arp->ar_tha, &arp->ar_sha, ARP_HLEN);
		net_copy_ip(&arp->ar_tpa, &arp->ar_spa);
		memcpy(&arp->ar_sha, NetOurEther, ARP_HLEN);
		net_copy_ip(&arp->ar_spa, &net_ip);

#ifdef CONFIG_CMD_LINK_LOCAL
		/*
		 * Work-around for brain-damaged Cisco equipment with
		 *   arp-proxy enabled.
		 *
		 *   If the requesting IP is not on our subnet, wait 5ms to
		 *   reply to ARP request so that our reply will overwrite
		 *   the arp-proxy's instead of the other way around.
		 */
		if ((net_read_ip(&arp->ar_tpa).s_addr & net_netmask.s_addr) !=
		    (net_read_ip(&arp->ar_spa).s_addr & net_netmask.s_addr))
			udelay(5000);
#endif
		NetSendPacket((uchar *)et, eth_hdr_size + ARP_HDR_SIZE);
		return;

	case ARPOP_REPLY:		/* arp reply */
		/* are we waiting for a reply */
		if (!net_arp_wait_packet_ip.s_addr)
			break;

#ifdef CONFIG_KEEP_SERVERADDR
		if (net_server_ip.s_addr == net_arp_wait_packet_ip.s_addr) {
			char buf[20];
			sprintf(buf, "%pM", &arp->ar_sha);
			setenv("serveraddr", buf);
		}
#endif

		reply_ip_addr = net_read_ip(&arp->ar_spa);

		/* matched waiting packet's address */
		if (reply_ip_addr.s_addr == net_arp_wait_reply_ip.s_addr) {
			debug_cond(DEBUG_DEV_PKT,
				"Got ARP REPLY, set eth addr (%pM)\n",
				arp->ar_data);

			/* save address for later use */
			if (NetArpWaitPacketMAC != NULL)
				memcpy(NetArpWaitPacketMAC,
				       &arp->ar_sha, ARP_HLEN);

			net_get_arp_handler()((uchar *)arp, 0, reply_ip_addr,
				0, len);

			/* set the mac address in the waiting packet's header
			   and transmit it */
			memcpy(((struct ethernet_hdr *)NetTxPacket)->et_dest,
				&arp->ar_sha, ARP_HLEN);
			NetSendPacket(NetTxPacket, NetArpWaitTxPacketSize);

			/* no arp request pending now */
			net_arp_wait_packet_ip.s_addr = 0;
			NetArpWaitTxPacketSize = 0;
			NetArpWaitPacketMAC = NULL;

		}
		return;
	default:
		debug("Unexpected ARP opcode 0x%x\n",
		      ntohs(arp->ar_op));
		return;
	}
}
