/*
 * (C) Copyright 2000
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
#include "bootp.h"
#include "tftp.h"
#include "arp.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define TIMEOUT		5		/* Seconds before trying ARP again */
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT)
#endif

static void ArpHandler(uchar *pkt, unsigned dest, unsigned src, unsigned len);
static void ArpTimeout(void);

int	ArpTry = 0;

/*
 *	Handle a ARP received packet.
 */
static void
ArpHandler(uchar *pkt, unsigned dest, unsigned src, unsigned len)
{
	/* Check if the frame is really an ARP reply */
	if (memcmp (NetServerEther, NetBcastAddr, 6) != 0) {
#ifdef	DEBUG
		printf("Got good ARP - start TFTP\n");
#endif
		TftpStart ();
	}
}


/*
 *	Timeout on ARP request.
 */
static void
ArpTimeout(void)
{
	if (ArpTry >= TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		NetSetTimeout (TIMEOUT * CFG_HZ, ArpTimeout);
		ArpRequest ();
	}
}


void
ArpRequest (void)
{
	int i;
	volatile uchar *pkt;
	ARP_t *	arp;

	printf("ARP broadcast %d\n", ++ArpTry);
	pkt = NetTxPacket;

	NetSetEther(pkt, NetBcastAddr, PROT_ARP);
	pkt += ETHER_HDR_SIZE;

	arp = (ARP_t *)pkt;

	arp->ar_hrd = htons(ARP_ETHER);
	arp->ar_pro = htons(PROT_IP);
	arp->ar_hln = 6;
	arp->ar_pln = 4;
	arp->ar_op  = htons(ARPOP_REQUEST);

	memcpy (&arp->ar_data[0], NetOurEther, 6);	/* source ET addr	*/
	NetWriteIP((uchar*)&arp->ar_data[6], NetOurIP);	/* source IP addr	*/
	for (i=10; i<16; ++i) {
		arp->ar_data[i] = 0;			/* dest ET addr = 0	*/
	}

	if((NetServerIP & NetOurSubnetMask) != (NetOurIP & NetOurSubnetMask)) {
	    if (NetOurGatewayIP == 0) {
		puts ("## Warning: gatewayip needed but not set\n");
	    }
	    NetWriteIP((uchar*)&arp->ar_data[16], NetOurGatewayIP);
	} else {
	    NetWriteIP((uchar*)&arp->ar_data[16], NetServerIP);
	}


	NetSendPacket(NetTxPacket, ETHER_HDR_SIZE + ARP_HDR_SIZE);

	NetSetTimeout(TIMEOUT * CFG_HZ, ArpTimeout);
	NetSetHandler(ArpHandler);
}

#endif /* CFG_CMD_NET */
