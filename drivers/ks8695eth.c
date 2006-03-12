/*
 * ks8695eth.c -- KS8695 ethernet driver
 *
 * (C) Copyright 2004-2005, Greg Ungerer <greg.ungerer@opengear.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/****************************************************************************/

#include <common.h>

#ifdef	CONFIG_DRIVER_KS8695ETH
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

/****************************************************************************/

/*
 * Hardware register access to the KS8695 LAN ethernet port
 * (well, it is the 4 port switch really).
 */
#define	ks8695_read(a)    *((volatile unsigned long *) (KS8695_IO_BASE + (a)))
#define	ks8695_write(a,v) *((volatile unsigned long *) (KS8695_IO_BASE + (a))) = (v)

/****************************************************************************/

/*
 * Define the descriptor in-memory data structures.
 */
struct ks8695_txdesc {
	uint32_t	owner;
	uint32_t	ctrl;
	uint32_t	addr;
	uint32_t	next;
};

struct ks8695_rxdesc {
	uint32_t	status;
	uint32_t	ctrl;
	uint32_t	addr;
	uint32_t	next;
};

/****************************************************************************/

/*
 * Allocate local data structures to use for receiving and sending
 * packets. Just to keep it all nice and simple.
 */

#define	TXDESCS		4
#define	RXDESCS		4
#define	BUFSIZE		2048

volatile struct ks8695_txdesc ks8695_tx[TXDESCS] __attribute__((aligned(256)));
volatile struct ks8695_rxdesc ks8695_rx[RXDESCS] __attribute__((aligned(256)));
volatile uint8_t ks8695_bufs[BUFSIZE*(TXDESCS+RXDESCS)] __attribute__((aligned(2048)));;

/****************************************************************************/

/*
 *	Ideally we want to use the MAC address stored in flash.
 *	But we do some sanity checks in case they are not present
 *	first.
 */
unsigned char eth_mac[] = {
	0x00, 0x13, 0xc6, 0x00, 0x00, 0x00
};

void ks8695_getmac(void)
{
	unsigned char *fp;
	int i;

	/* Check if flash MAC is valid */
	fp = (unsigned char *) 0x0201c000;
	for (i = 0; (i < 6); i++) {
		if ((fp[i] != 0) && (fp[i] != 0xff))
			break;
	}

	/* If we found a valid looking MAC address then use it */
	if (i < 6)
		memcpy(&eth_mac[0], fp, 6);
}

/****************************************************************************/

void eth_reset(bd_t *bd)
{
	int i;

	debug ("%s(%d): eth_reset()\n", __FILE__, __LINE__);

	/* Reset the ethernet engines first */
	ks8695_write(KS8695_LAN_DMA_TX, 0x80000000);
	ks8695_write(KS8695_LAN_DMA_RX, 0x80000000);

	ks8695_getmac();

	/* Set MAC address */
	ks8695_write(KS8695_LAN_MAC_LOW, (eth_mac[5] | (eth_mac[4] << 8) |
		(eth_mac[3] << 16) | (eth_mac[2] << 24)));
	ks8695_write(KS8695_LAN_MAC_HIGH, (eth_mac[1] | (eth_mac[0] << 8)));

	/* Turn the 4 port switch on */
	i = ks8695_read(KS8695_SWITCH_CTRL0);
	ks8695_write(KS8695_SWITCH_CTRL0, (i | 0x1));
	/* ks8695_write(KS8695_WAN_CONTROL, 0x3f000066); */

	/* Initialize descriptor rings */
	for (i = 0; (i < TXDESCS); i++) {
		ks8695_tx[i].owner = 0;
		ks8695_tx[i].ctrl = 0;
		ks8695_tx[i].addr = (uint32_t) &ks8695_bufs[i*BUFSIZE];
		ks8695_tx[i].next = (uint32_t) &ks8695_tx[i+1];
	}
	ks8695_tx[TXDESCS-1].ctrl = 0x02000000;
	ks8695_tx[TXDESCS-1].next = (uint32_t) &ks8695_tx[0];

	for (i = 0; (i < RXDESCS); i++) {
		ks8695_rx[i].status = 0x80000000;
		ks8695_rx[i].ctrl = BUFSIZE - 4;
		ks8695_rx[i].addr = (uint32_t) &ks8695_bufs[(i+TXDESCS)*BUFSIZE];
		ks8695_rx[i].next = (uint32_t) &ks8695_rx[i+1];
	}
	ks8695_rx[RXDESCS-1].ctrl |= 0x00080000;
	ks8695_rx[RXDESCS-1].next = (uint32_t) &ks8695_rx[0];

	/* The KS8695 is pretty slow reseting the ethernets... */
	udelay(2000000);

	/* Enable the ethernet engine */
	ks8695_write(KS8695_LAN_TX_LIST, (uint32_t) &ks8695_tx[0]);
	ks8695_write(KS8695_LAN_RX_LIST, (uint32_t) &ks8695_rx[0]);
	ks8695_write(KS8695_LAN_DMA_TX, 0x3);
	ks8695_write(KS8695_LAN_DMA_RX, 0x71);
	ks8695_write(KS8695_LAN_DMA_RX_START, 0x1);

	printf("KS8695 ETHERNET: ");
	for (i = 0; (i < 5); i++) {
		bd->bi_enetaddr[i] = eth_mac[i];
		printf("%02x:", eth_mac[i]);
	}
	bd->bi_enetaddr[i] = eth_mac[i];
	printf("%02x\n", eth_mac[i]);
}

/****************************************************************************/

int eth_init(bd_t *bd)
{
	debug ("%s(%d): eth_init()\n", __FILE__, __LINE__);

	eth_reset(bd);
	return 0;
}

/****************************************************************************/

void eth_halt(void)
{
	debug ("%s(%d): eth_halt()\n", __FILE__, __LINE__);

	/* Reset the ethernet engines */
	ks8695_write(KS8695_LAN_DMA_TX, 0x80000000);
	ks8695_write(KS8695_LAN_DMA_RX, 0x80000000);
}

/****************************************************************************/

int eth_rx(void)
{
	volatile struct ks8695_rxdesc *dp;
	int i, len = 0;

	debug ("%s(%d): eth_rx()\n", __FILE__, __LINE__);

	for (i = 0; (i < RXDESCS); i++) {
		dp= &ks8695_rx[i];
		if ((dp->status & 0x80000000) == 0) {
			len = (dp->status & 0x7ff) - 4;
			NetReceive((void *) dp->addr, len);
			dp->status = 0x80000000;
			ks8695_write(KS8695_LAN_DMA_RX_START, 0x1);
			break;
		}
	}

	return len;
}

/****************************************************************************/

int eth_send(volatile void *packet, int len)
{
	volatile struct ks8695_txdesc *dp;
	static int next = 0;

	debug ("%s(%d): eth_send(packet=%x,len=%d)\n", __FILE__, __LINE__,
		packet, len);

	dp = &ks8695_tx[next];
	memcpy((void *) dp->addr, (void *) packet, len);

	if (len < 64) {
		memset((void *) (dp->addr + len), 0, 64-len);
		len = 64;
	}

	dp->ctrl = len | 0xe0000000;
	dp->owner = 0x80000000;

	ks8695_write(KS8695_LAN_DMA_TX, 0x3);
	ks8695_write(KS8695_LAN_DMA_TX_START, 0x1);

	if (++next >= TXDESCS)
		next = 0;

	return len;
}

#endif	/* CONFIG_DRIVER_KS8695ETH */
