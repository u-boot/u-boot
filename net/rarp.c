/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "nfs.h"
#include "bootp.h"
#include "rarp.h"
#include "tftp.h"

#define TIMEOUT 5000UL /* Milliseconds before trying BOOTP again */
#ifndef	CONFIG_NET_RETRY_COUNT
#define TIMEOUT_COUNT 5 /* # of timeouts before giving up  */
#else
#define TIMEOUT_COUNT (CONFIG_NET_RETRY_COUNT)
#endif

int RarpTry;

/*
 *	Handle a RARP received packet.
 */
void rarp_receive(struct ip_udp_hdr *ip, unsigned len)
{
	struct arp_hdr *arp;

	debug_cond(DEBUG_NET_PKT, "Got RARP\n");
	arp = (struct arp_hdr *)ip;
	if (len < ARP_HDR_SIZE) {
		printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
		return;
	}

	if ((ntohs(arp->ar_op) != RARPOP_REPLY) ||
		(ntohs(arp->ar_hrd) != ARP_ETHER)   ||
		(ntohs(arp->ar_pro) != PROT_IP)     ||
		(arp->ar_hln != 6) || (arp->ar_pln != 4)) {

		puts("invalid RARP header\n");
	} else {
		NetCopyIP(&NetOurIP, &arp->ar_data[16]);
		if (NetServerIP == 0)
			NetCopyIP(&NetServerIP, &arp->ar_data[6]);
		memcpy(NetServerEther, &arp->ar_data[0], 6);
		debug_cond(DEBUG_DEV_PKT, "Got good RARP\n");
		net_auto_load();
	}
}


/*
 *	Timeout on BOOTP request.
 */
static void RarpTimeout(void)
{
	if (RarpTry >= TIMEOUT_COUNT) {
		puts("\nRetry count exceeded; starting again\n");
		NetStartAgain();
	} else {
		NetSetTimeout(TIMEOUT, RarpTimeout);
		RarpRequest();
	}
}


void RarpRequest(void)
{
	uchar *pkt;
	struct arp_hdr *rarp;
	int eth_hdr_size;

	printf("RARP broadcast %d\n", ++RarpTry);
	pkt = NetTxPacket;

	eth_hdr_size = NetSetEther(pkt, NetBcastAddr, PROT_RARP);
	pkt += eth_hdr_size;

	rarp = (struct arp_hdr *)pkt;

	rarp->ar_hrd = htons(ARP_ETHER);
	rarp->ar_pro = htons(PROT_IP);
	rarp->ar_hln = 6;
	rarp->ar_pln = 4;
	rarp->ar_op  = htons(RARPOP_REQUEST);
	memcpy(&rarp->ar_data[0],  NetOurEther, 6);	/* source ET addr */
	memcpy(&rarp->ar_data[6],  &NetOurIP,   4);	/* source IP addr */
	/* dest ET addr = source ET addr ??*/
	memcpy(&rarp->ar_data[10], NetOurEther, 6);
	/* dest IP addr set to broadcast */
	memset(&rarp->ar_data[16], 0xff,        4);

	NetSendPacket(NetTxPacket, eth_hdr_size + ARP_HDR_SIZE);

	NetSetTimeout(TIMEOUT, RarpTimeout);
}
