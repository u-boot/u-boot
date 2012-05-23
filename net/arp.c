/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
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

IPaddr_t	NetArpWaitPacketIP;
IPaddr_t	NetArpWaitReplyIP;
/* MAC address of waiting packet's destination */
uchar	       *NetArpWaitPacketMAC;
/* THE transmit packet */
uchar	       *NetArpWaitTxPacket;
int		NetArpWaitTxPacketSize;
uchar		NetArpWaitPacketBuf[PKTSIZE_ALIGN + PKTALIGN];
ulong		NetArpWaitTimerStart;
int		NetArpWaitTry;

void ArpInit(void)
{
	/* XXX problem with bss workaround */
	NetArpWaitPacketMAC = NULL;
	NetArpWaitPacketIP = 0;
	NetArpWaitReplyIP = 0;
	NetArpWaitTxPacket = &NetArpWaitPacketBuf[0] + (PKTALIGN - 1);
	NetArpWaitTxPacket -= (ulong)NetArpWaitTxPacket % PKTALIGN;
	NetArpWaitTxPacketSize = 0;
}

void ArpRequest(void)
{
	uchar *pkt;
	struct arp_hdr *arp;

	debug("ARP broadcast %d\n", NetArpWaitTry);

	pkt = NetTxPacket;

	pkt += NetSetEther(pkt, NetBcastAddr, PROT_ARP);

	arp = (struct arp_hdr *) pkt;

	arp->ar_hrd = htons(ARP_ETHER);
	arp->ar_pro = htons(PROT_IP);
	arp->ar_hln = 6;
	arp->ar_pln = 4;
	arp->ar_op = htons(ARPOP_REQUEST);

	/* source ET addr */
	memcpy(&arp->ar_data[0], NetOurEther, 6);
	/* source IP addr */
	NetWriteIP((uchar *) &arp->ar_data[6], NetOurIP);
	/* dest ET addr = 0 */
	memset(&arp->ar_data[10], '\0', 6);
	if ((NetArpWaitPacketIP & NetOurSubnetMask) !=
	    (NetOurIP & NetOurSubnetMask)) {
		if (NetOurGatewayIP == 0) {
			puts("## Warning: gatewayip needed but not set\n");
			NetArpWaitReplyIP = NetArpWaitPacketIP;
		} else {
			NetArpWaitReplyIP = NetOurGatewayIP;
		}
	} else {
		NetArpWaitReplyIP = NetArpWaitPacketIP;
	}

	NetWriteIP((uchar *) &arp->ar_data[16], NetArpWaitReplyIP);
	(void) eth_send(NetTxPacket, (pkt - NetTxPacket) + ARP_HDR_SIZE);
}

void ArpTimeoutCheck(void)
{
	ulong t;

	if (!NetArpWaitPacketIP)
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
	IPaddr_t tmp;
	uchar *pkt;

	/*
	 * We have to deal with two types of ARP packets:
	 * - REQUEST packets will be answered by sending  our
	 *   IP address - if we know it.
	 * - REPLY packates are expected only after we asked
	 *   for the TFTP server's or the gateway's ethernet
	 *   address; so if we receive such a packet, we set
	 *   the server ethernet address
	 */
	debug("Got ARP\n");

	arp = (struct arp_hdr *)ip;
	if (len < ARP_HDR_SIZE) {
		printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
		return;
	}
	if (ntohs(arp->ar_hrd) != ARP_ETHER)
		return;
	if (ntohs(arp->ar_pro) != PROT_IP)
		return;
	if (arp->ar_hln != 6)
		return;
	if (arp->ar_pln != 4)
		return;

	if (NetOurIP == 0)
		return;

	if (NetReadIP(&arp->ar_data[16]) != NetOurIP)
		return;

	switch (ntohs(arp->ar_op)) {
	case ARPOP_REQUEST:
		/* reply with our IP address */
		debug("Got ARP REQUEST, return our IP\n");
		pkt = (uchar *)et;
		pkt += NetSetEther(pkt, et->et_src, PROT_ARP);
		arp->ar_op = htons(ARPOP_REPLY);
		memcpy(&arp->ar_data[10], &arp->ar_data[0], 6);
		NetCopyIP(&arp->ar_data[16], &arp->ar_data[6]);
		memcpy(&arp->ar_data[0], NetOurEther, 6);
		NetCopyIP(&arp->ar_data[6], &NetOurIP);
		(void) eth_send((uchar *)et,
				(pkt - (uchar *)et) + ARP_HDR_SIZE);
		return;

	case ARPOP_REPLY:		/* arp reply */
		/* are we waiting for a reply */
		if (!NetArpWaitPacketIP || !NetArpWaitPacketMAC)
			break;

#ifdef CONFIG_KEEP_SERVERADDR
		if (NetServerIP == NetArpWaitPacketIP) {
			char buf[20];
			sprintf(buf, "%pM", arp->ar_data);
			setenv("serveraddr", buf);
		}
#endif

		tmp = NetReadIP(&arp->ar_data[6]);

		/* matched waiting packet's address */
		if (tmp == NetArpWaitReplyIP) {
			debug("Got ARP REPLY, set eth addr (%pM)\n",
				arp->ar_data);

			/* save address for later use */
			memcpy(NetArpWaitPacketMAC,
			       &arp->ar_data[0], 6);

#ifdef CONFIG_NETCONSOLE
			NetGetHandler()(0, 0, 0, 0, 0);
#endif
			/* modify header, and transmit it */
			memcpy(((struct ethernet_hdr *)NetArpWaitTxPacket)->
				et_dest, NetArpWaitPacketMAC, 6);
			(void) eth_send(NetArpWaitTxPacket,
					NetArpWaitTxPacketSize);

			/* no arp request pending now */
			NetArpWaitPacketIP = 0;
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
