/*
Ported to U-Boot  by Christian Pellegrin <chri@ascensit.com>

Based on sources from the Linux kernel (pcnet_cs.c, 8390.h) and
eCOS(if_dp83902a.c, if_dp83902a.h). Both of these 2 wonderful world
are GPL, so this is, of course, GPL.


==========================================================================

dev/if_dp83902a.c

Ethernet device driver for NS DP83902a ethernet controller

==========================================================================
####ECOSGPLCOPYRIGHTBEGIN####
-------------------------------------------
This file is part of eCos, the Embedded Configurable Operating System.
Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

eCos is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 or (at your option) any later version.

eCos is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with eCos; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

As a special exception, if other files instantiate templates or use macros
or inline functions from this file, or you compile this file and link it
with other works to produce a work based on this file, this file does not
by itself cause the resulting work to be covered by the GNU General Public
License. However the source code for this file must still be made available
in accordance with section (3) of the GNU General Public License.

This exception does not invalidate any other reasons why a work based on
this file might be covered by the GNU General Public License.

Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
at http://sources.redhat.com/ecos/ecos-license/
-------------------------------------------
####ECOSGPLCOPYRIGHTEND####
####BSDCOPYRIGHTBEGIN####

-------------------------------------------

Portions of this software may have been derived from OpenBSD or other sources,
and are covered by the appropriate copyright disclaimers included herein.

-------------------------------------------

####BSDCOPYRIGHTEND####
==========================================================================
#####DESCRIPTIONBEGIN####

Author(s):    gthomas
Contributors: gthomas, jskov, rsandifo
Date:	      2001-06-13
Purpose:
Description:

FIXME:	      Will fail if pinged with large packets (1520 bytes)
Add promisc config
Add SNMP

####DESCRIPTIONEND####


==========================================================================

*/

#include <common.h>
#include <command.h>
#include <net.h>
#include <malloc.h>

#ifdef CONFIG_DRIVER_NE2000

/* wor around udelay resetting OCR */
static void my_udelay(long us) {
	long tmo;

	tmo = get_timer (0) + us * CFG_HZ / 1000000; /* will this be much greater than 0 ? */
	while (get_timer (0) < tmo);
}

#define mdelay(n)       my_udelay((n)*1000)

/* forward definition of function used for the uboot interface */
void uboot_push_packet_len(int len);
void uboot_push_tx_done(int key, int val);

/* timeout for tx/rx in s */
#define TOUT 5

#define ETHER_ADDR_LEN 6

/*
  ------------------------------------------------------------------------
  Debugging details

  Set to perms of:
  0 disables all debug output
  1 for process debug output
  2 for added data IO output: get_reg, put_reg
  4 for packet allocation/free output
  8 for only startup status, so we can tell we're installed OK
*/
/*#define DEBUG 0xf*/
#define DEBUG 0

#if DEBUG & 1
#define DEBUG_FUNCTION() do { printf("%s\n", __FUNCTION__); } while (0)
#define DEBUG_LINE() do { printf("%d\n", __LINE__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#define DEBUG_LINE() do {} while(0)
#endif

#include "ne2000.h"

#if DEBUG & 1
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif

static dp83902a_priv_data_t nic; /* just one instance of the card supported */

static bool
dp83902a_init(void)
{
	dp83902a_priv_data_t *dp = &nic;
	cyg_uint8* base;
	int i;

	DEBUG_FUNCTION();

	base = dp->base;
	if (!base) return false;  /* No device found */

	DEBUG_LINE();

	/* Prepare ESA */
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);  /* Select page 1 */
	/* Use the address from the serial EEPROM */
	for (i = 0; i < 6; i++)
		DP_IN(base, DP_P1_PAR0+i, dp->esa[i]);
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0);  /* Select page 0 */

	printf("NE2000 - %s ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       "eeprom",
	       dp->esa[0],
	       dp->esa[1],
	       dp->esa[2],
	       dp->esa[3],
	       dp->esa[4],
	       dp->esa[5] );

	return true;
}

static void
dp83902a_stop(void)
{
	dp83902a_priv_data_t *dp = &nic;
	cyg_uint8 *base = dp->base;

	DEBUG_FUNCTION();

	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_STOP);  /* Brutal */
	DP_OUT(base, DP_ISR, 0xFF);		/* Clear any pending interrupts */
	DP_OUT(base, DP_IMR, 0x00);		/* Disable all interrupts */

	dp->running = false;
}

/*
  This function is called to "start up" the interface.  It may be called
  multiple times, even when the hardware is already running.  It will be
  called whenever something "hardware oriented" changes and should leave
  the hardware ready to send/receive packets.
*/
static void
dp83902a_start(unsigned char * enaddr)
{
	dp83902a_priv_data_t *dp = &nic;
	cyg_uint8 *base = dp->base;
	int i;

	DEBUG_FUNCTION();

	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_STOP); /* Brutal */
	DP_OUT(base, DP_DCR, DP_DCR_INIT);
	DP_OUT(base, DP_RBCH, 0);		/* Remote byte count */
	DP_OUT(base, DP_RBCL, 0);
	DP_OUT(base, DP_RCR, DP_RCR_MON);	/* Accept no packets */
	DP_OUT(base, DP_TCR, DP_TCR_LOCAL);	/* Transmitter [virtually] off */
	DP_OUT(base, DP_TPSR, dp->tx_buf1);	/* Transmitter start page */
	dp->tx1 = dp->tx2 = 0;
	dp->tx_next = dp->tx_buf1;
	dp->tx_started = false;
	DP_OUT(base, DP_PSTART, dp->rx_buf_start); /* Receive ring start page */
	DP_OUT(base, DP_BNDRY, dp->rx_buf_end-1); /* Receive ring boundary */
	DP_OUT(base, DP_PSTOP, dp->rx_buf_end);	/* Receive ring end page */
	dp->rx_next = dp->rx_buf_start-1;
	DP_OUT(base, DP_ISR, 0xFF);		/* Clear any pending interrupts */
	DP_OUT(base, DP_IMR, DP_IMR_All);	/* Enable all interrupts */
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1 | DP_CR_STOP);  /* Select page 1 */
	DP_OUT(base, DP_P1_CURP, dp->rx_buf_start);   /* Current page - next free page for Rx */
	for (i = 0;  i < ETHER_ADDR_LEN;  i++) {
		DP_OUT(base, DP_P1_PAR0+i, enaddr[i]);
	}
	/* Enable and start device */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_TCR, DP_TCR_NORMAL); /* Normal transmit operations */
	DP_OUT(base, DP_RCR, DP_RCR_AB);  /* Accept broadcast, no errors, no multicast */
	dp->running = true;
}

/*
  This routine is called to start the transmitter.  It is split out from the
  data handling routine so it may be called either when data becomes first
  available or when an Tx interrupt occurs
*/

static void
dp83902a_start_xmit(int start_page, int len)
{
	dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *) &nic;
	cyg_uint8 *base = dp->base;

	DEBUG_FUNCTION();

#if DEBUG & 1
	printf("Tx pkt %d len %d\n", start_page, len);
	if (dp->tx_started)
		printf("TX already started?!?\n");
#endif

	DP_OUT(base, DP_ISR, (DP_ISR_TxP | DP_ISR_TxE));
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_TBCL, len & 0xFF);
	DP_OUT(base, DP_TBCH, len >> 8);
	DP_OUT(base, DP_TPSR, start_page);
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_TXPKT | DP_CR_START);

	dp->tx_started = true;
}

/*
  This routine is called to send data to the hardware.  It is known a-priori
  that there is free buffer space (dp->tx_next).
*/
static void
dp83902a_send(unsigned char *data, int total_len, unsigned long key)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	int len, start_page, pkt_len, i, isr;
#if DEBUG & 4
	int dx;
#endif

	DEBUG_FUNCTION();

	len = pkt_len = total_len;
	if (pkt_len < IEEE_8023_MIN_FRAME) pkt_len = IEEE_8023_MIN_FRAME;

	start_page = dp->tx_next;
	if (dp->tx_next == dp->tx_buf1) {
		dp->tx1 = start_page;
		dp->tx1_len = pkt_len;
		dp->tx1_key = key;
		dp->tx_next = dp->tx_buf2;
	} else {
		dp->tx2 = start_page;
		dp->tx2_len = pkt_len;
		dp->tx2_key = key;
		dp->tx_next = dp->tx_buf1;
	}

#if DEBUG & 5
	printf("TX prep page %d len %d\n", start_page, pkt_len);
#endif

	DP_OUT(base, DP_ISR, DP_ISR_RDC);  /* Clear end of DMA */
	{
		/* Dummy read. The manual sez something slightly different, */
		/* but the code is extended a bit to do what Hitachi's monitor */
		/* does (i.e., also read data). */

		cyg_uint16 tmp;
		int len = 1;

		DP_OUT(base, DP_RSAL, 0x100-len);
		DP_OUT(base, DP_RSAH, (start_page-1) & 0xff);
		DP_OUT(base, DP_RBCL, len);
		DP_OUT(base, DP_RBCH, 0);
		DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_RDMA | DP_CR_START);
		DP_IN_DATA(dp->data, tmp);
	}

#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA
	/* Stall for a bit before continuing to work around random data */
	/* corruption problems on some platforms. */
	CYGACC_CALL_IF_DELAY_US(1);
#endif

	/* Send data to device buffer(s) */
	DP_OUT(base, DP_RSAL, 0);
	DP_OUT(base, DP_RSAH, start_page);
	DP_OUT(base, DP_RBCL, pkt_len & 0xFF);
	DP_OUT(base, DP_RBCH, pkt_len >> 8);
	DP_OUT(base, DP_CR, DP_CR_WDMA | DP_CR_START);

	/* Put data into buffer */
#if DEBUG & 4
	printf(" sg buf %08lx len %08x\n ", (unsigned long) data, len);
	dx = 0;
#endif
	while (len > 0) {
#if DEBUG & 4
		printf(" %02x", *data);
		if (0 == (++dx % 16)) printf("\n ");
#endif
		DP_OUT_DATA(dp->data, *data++);
		len--;
	}
#if DEBUG & 4
	printf("\n");
#endif
	if (total_len < pkt_len) {
#if DEBUG & 4
		printf("  + %d bytes of padding\n", pkt_len - total_len);
#endif
		/* Padding to 802.3 length was required */
		for (i = total_len;  i < pkt_len;) {
			i++;
			DP_OUT_DATA(dp->data, 0);
		}
	}

#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA
	/* After last data write, delay for a bit before accessing the */
	/* device again, or we may get random data corruption in the last */
	/* datum (on some platforms). */
	CYGACC_CALL_IF_DELAY_US(1);
#endif

	/* Wait for DMA to complete */
	do {
		DP_IN(base, DP_ISR, isr);
	} while ((isr & DP_ISR_RDC) == 0);
	/* Then disable DMA */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);

	/* Start transmit if not already going */
	if (!dp->tx_started) {
		if (start_page == dp->tx1) {
			dp->tx_int = 1;  /* Expecting interrupt from BUF1 */
		} else {
			dp->tx_int = 2;  /* Expecting interrupt from BUF2 */
		}
		dp83902a_start_xmit(start_page, pkt_len);
	}
}

/*
  This function is called when a packet has been received.  It's job is
  to prepare to unload the packet from the hardware.  Once the length of
  the packet is known, the upper layer of the driver can be told.  When
  the upper layer is ready to unload the packet, the internal function
  'dp83902a_recv' will be called to actually fetch it from the hardware.
*/
static void
dp83902a_RxEvent(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	unsigned char rsr;
	unsigned char rcv_hdr[4];
	int i, len, pkt, cur;

	DEBUG_FUNCTION();

	DP_IN(base, DP_RSR, rsr);
	while (true) {
		/* Read incoming packet header */
		DP_OUT(base, DP_CR, DP_CR_PAGE1 | DP_CR_NODMA | DP_CR_START);
		DP_IN(base, DP_P1_CURP, cur);
		DP_OUT(base, DP_P1_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
		DP_IN(base, DP_BNDRY, pkt);

		pkt += 1;
		if (pkt == dp->rx_buf_end)
			pkt = dp->rx_buf_start;

		if (pkt == cur) {
			break;
		}
		DP_OUT(base, DP_RBCL, sizeof(rcv_hdr));
		DP_OUT(base, DP_RBCH, 0);
		DP_OUT(base, DP_RSAL, 0);
		DP_OUT(base, DP_RSAH, pkt);
		if (dp->rx_next == pkt) {
			if (cur == dp->rx_buf_start)
				DP_OUT(base, DP_BNDRY, dp->rx_buf_end-1);
			else
				DP_OUT(base, DP_BNDRY, cur-1); /* Update pointer */
			return;
		}
		dp->rx_next = pkt;
		DP_OUT(base, DP_ISR, DP_ISR_RDC); /* Clear end of DMA */
		DP_OUT(base, DP_CR, DP_CR_RDMA | DP_CR_START);
#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_RX_DMA
		CYGACC_CALL_IF_DELAY_US(10);
#endif

		for (i = 0;  i < sizeof(rcv_hdr);) {
			DP_IN_DATA(dp->data, rcv_hdr[i++]);
		}

#if DEBUG & 5
		printf("rx hdr %02x %02x %02x %02x\n",
		       rcv_hdr[0], rcv_hdr[1], rcv_hdr[2], rcv_hdr[3]);
#endif
		len = ((rcv_hdr[3] << 8) | rcv_hdr[2]) - sizeof(rcv_hdr);
		uboot_push_packet_len(len);
		if (rcv_hdr[1] == dp->rx_buf_start)
			DP_OUT(base, DP_BNDRY, dp->rx_buf_end-1);
		else
			DP_OUT(base, DP_BNDRY, rcv_hdr[1]-1); /* Update pointer */
	}
}

/*
  This function is called as a result of the "eth_drv_recv()" call above.
  It's job is to actually fetch data for a packet from the hardware once
  memory buffers have been allocated for the packet.  Note that the buffers
  may come in pieces, using a scatter-gather list.  This allows for more
  efficient processing in the upper layers of the stack.
*/
static void
dp83902a_recv(unsigned char *data, int len)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	int i, mlen;
	cyg_uint8 saved_char = 0;
	bool saved;
#if DEBUG & 4
	int dx;
#endif

	DEBUG_FUNCTION();

#if DEBUG & 5
	printf("Rx packet %d length %d\n", dp->rx_next, len);
#endif

	/* Read incoming packet data */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_RBCL, len & 0xFF);
	DP_OUT(base, DP_RBCH, len >> 8);
	DP_OUT(base, DP_RSAL, 4);		/* Past header */
	DP_OUT(base, DP_RSAH, dp->rx_next);
	DP_OUT(base, DP_ISR, DP_ISR_RDC); /* Clear end of DMA */
	DP_OUT(base, DP_CR, DP_CR_RDMA | DP_CR_START);
#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_RX_DMA
	CYGACC_CALL_IF_DELAY_US(10);
#endif

	saved = false;
	for (i = 0;  i < 1;  i++) {
		if (data) {
			mlen = len;
#if DEBUG & 4
			printf(" sg buf %08lx len %08x \n", (unsigned long) data, mlen);
			dx = 0;
#endif
			while (0 < mlen) {
				/* Saved byte from previous loop? */
				if (saved) {
					*data++ = saved_char;
					mlen--;
					saved = false;
					continue;
				}

				{
					cyg_uint8 tmp;
					DP_IN_DATA(dp->data, tmp);
#if DEBUG & 4
					printf(" %02x", tmp);
					if (0 == (++dx % 16)) printf("\n ");
#endif
					*data++ = tmp;;
					mlen--;
				}
			}
#if DEBUG & 4
			printf("\n");
#endif
		}
	}
}

static void
dp83902a_TxEvent(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	unsigned char tsr;
	unsigned long key;

	DEBUG_FUNCTION();

	DP_IN(base, DP_TSR, tsr);
	if (dp->tx_int == 1) {
		key = dp->tx1_key;
		dp->tx1 = 0;
	} else {
		key = dp->tx2_key;
		dp->tx2 = 0;
	}
	/* Start next packet if one is ready */
	dp->tx_started = false;
	if (dp->tx1) {
		dp83902a_start_xmit(dp->tx1, dp->tx1_len);
		dp->tx_int = 1;
	} else if (dp->tx2) {
		dp83902a_start_xmit(dp->tx2, dp->tx2_len);
		dp->tx_int = 2;
	} else {
		dp->tx_int = 0;
	}
	/* Tell higher level we sent this packet */
	uboot_push_tx_done(key, 0);
}

/* Read the tally counters to clear them.  Called in response to a CNT */
/* interrupt. */
static void
dp83902a_ClearCounters(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	cyg_uint8 cnt1, cnt2, cnt3;

	DP_IN(base, DP_FER, cnt1);
	DP_IN(base, DP_CER, cnt2);
	DP_IN(base, DP_MISSED, cnt3);
	DP_OUT(base, DP_ISR, DP_ISR_CNT);
}

/* Deal with an overflow condition.  This code follows the procedure set */
/* out in section 7.0 of the datasheet. */
static void
dp83902a_Overflow(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *)&nic;
	cyg_uint8 *base = dp->base;
	cyg_uint8 isr;

	/* Issue a stop command and wait 1.6ms for it to complete. */
	DP_OUT(base, DP_CR, DP_CR_STOP | DP_CR_NODMA);
	CYGACC_CALL_IF_DELAY_US(1600);

	/* Clear the remote byte counter registers. */
	DP_OUT(base, DP_RBCL, 0);
	DP_OUT(base, DP_RBCH, 0);

	/* Enter loopback mode while we clear the buffer. */
	DP_OUT(base, DP_TCR, DP_TCR_LOCAL);
	DP_OUT(base, DP_CR, DP_CR_START | DP_CR_NODMA);

	/* Read in as many packets as we can and acknowledge any and receive */
	/* interrupts.  Since the buffer has overflowed, a receive event of */
	/* some kind will have occured. */
	dp83902a_RxEvent();
	DP_OUT(base, DP_ISR, DP_ISR_RxP|DP_ISR_RxE);

	/* Clear the overflow condition and leave loopback mode. */
	DP_OUT(base, DP_ISR, DP_ISR_OFLW);
	DP_OUT(base, DP_TCR, DP_TCR_NORMAL);

	/* If a transmit command was issued, but no transmit event has occured, */
	/* restart it here. */
	DP_IN(base, DP_ISR, isr);
	if (dp->tx_started && !(isr & (DP_ISR_TxP|DP_ISR_TxE))) {
		DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_TXPKT | DP_CR_START);
	}
}

static void
dp83902a_poll(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	cyg_uint8 *base = dp->base;
	unsigned char isr;

	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0 | DP_CR_START);
	DP_IN(base, DP_ISR, isr);
	while (0 != isr) {
		/* The CNT interrupt triggers when the MSB of one of the error */
		/* counters is set.  We don't much care about these counters, but */
		/* we should read their values to reset them. */
		if (isr & DP_ISR_CNT) {
			dp83902a_ClearCounters();
		}
		/* Check for overflow.  It's a special case, since there's a */
		/* particular procedure that must be followed to get back into */
		/* a running state.a */
		if (isr & DP_ISR_OFLW) {
			dp83902a_Overflow();
		} else {
			/* Other kinds of interrupts can be acknowledged simply by */
			/* clearing the relevant bits of the ISR.  Do that now, then */
			/* handle the interrupts we care about. */
			DP_OUT(base, DP_ISR, isr);      /* Clear set bits */
			if (!dp->running) break;	/* Is this necessary? */
			/* Check for tx_started on TX event since these may happen */
			/* spuriously it seems. */
			if (isr & (DP_ISR_TxP|DP_ISR_TxE) && dp->tx_started) {
				dp83902a_TxEvent();
			}
			if (isr & (DP_ISR_RxP|DP_ISR_RxE)) {
				dp83902a_RxEvent();
			}
		}
		DP_IN(base, DP_ISR, isr);
	}
}

/* find prom (taken from pc_net_cs.c from Linux) */

#include "8390.h"

typedef struct hw_info_t {
	u_int	offset;
	u_char	a0, a1, a2;
	u_int	flags;
} hw_info_t;

#define DELAY_OUTPUT	0x01
#define HAS_MISC_REG	0x02
#define USE_BIG_BUF	0x04
#define HAS_IBM_MISC	0x08
#define IS_DL10019	0x10
#define IS_DL10022	0x20
#define HAS_MII		0x40
#define USE_SHMEM	0x80	/* autodetected */

#define AM79C9XX_HOME_PHY	0x00006B90  /* HomePNA PHY */
#define AM79C9XX_ETH_PHY	0x00006B70  /* 10baseT PHY */
#define MII_PHYID_REV_MASK	0xfffffff0
#define MII_PHYID_REG1		0x02
#define MII_PHYID_REG2		0x03

static hw_info_t hw_info[] = {
	{ /* Accton EN2212 */ 0x0ff0, 0x00, 0x00, 0xe8, DELAY_OUTPUT },
	{ /* Allied Telesis LA-PCM */ 0x0ff0, 0x00, 0x00, 0xf4, 0 },
	{ /* APEX MultiCard */ 0x03f4, 0x00, 0x20, 0xe5, 0 },
	{ /* ASANTE FriendlyNet */ 0x4910, 0x00, 0x00, 0x94,
	  DELAY_OUTPUT | HAS_IBM_MISC },
	{ /* Danpex EN-6200P2 */ 0x0110, 0x00, 0x40, 0xc7, 0 },
	{ /* DataTrek NetCard */ 0x0ff0, 0x00, 0x20, 0xe8, 0 },
	{ /* Dayna CommuniCard E */ 0x0110, 0x00, 0x80, 0x19, 0 },
	{ /* D-Link DE-650 */ 0x0040, 0x00, 0x80, 0xc8, 0 },
	{ /* EP-210 Ethernet */ 0x0110, 0x00, 0x40, 0x33, 0 },
	{ /* EP4000 Ethernet */ 0x01c0, 0x00, 0x00, 0xb4, 0 },
	{ /* Epson EEN10B */ 0x0ff0, 0x00, 0x00, 0x48,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* ELECOM Laneed LD-CDWA */ 0xb8, 0x08, 0x00, 0x42, 0 },
	{ /* Hypertec Ethernet */ 0x01c0, 0x00, 0x40, 0x4c, 0 },
	{ /* IBM CCAE */ 0x0ff0, 0x08, 0x00, 0x5a,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM CCAE */ 0x0ff0, 0x00, 0x04, 0xac,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM CCAE */ 0x0ff0, 0x00, 0x06, 0x29,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM FME */ 0x0374, 0x08, 0x00, 0x5a,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM FME */ 0x0374, 0x00, 0x04, 0xac,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kansai KLA-PCM/T */ 0x0ff0, 0x00, 0x60, 0x87,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x08, 0x00, 0x17,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x00, 0xc0, 0xa8,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x00, 0xa0, 0xb0,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0198, 0x00, 0x20, 0xe0,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* I-O DATA PCLA/T */ 0x0ff0, 0x00, 0xa0, 0xb0, 0 },
	{ /* Katron PE-520 */ 0x0110, 0x00, 0x40, 0xf6, 0 },
	{ /* Kingston KNE-PCM/x */ 0x0ff0, 0x00, 0xc0, 0xf0,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kingston KNE-PCM/x */ 0x0ff0, 0xe2, 0x0c, 0x0f,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kingston KNE-PC2 */ 0x0180, 0x00, 0xc0, 0xf0, 0 },
	{ /* Maxtech PCN2000 */ 0x5000, 0x00, 0x00, 0xe8, 0 },
	{ /* NDC Instant-Link */ 0x003a, 0x00, 0x80, 0xc6, 0 },
	{ /* NE2000 Compatible */ 0x0ff0, 0x00, 0xa0, 0x0c, 0 },
	{ /* Network General Sniffer */ 0x0ff0, 0x00, 0x00, 0x65,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Panasonic VEL211 */ 0x0ff0, 0x00, 0x80, 0x45,
	  HAS_MISC_REG | HAS_IBM_MISC },
	{ /* PreMax PE-200 */ 0x07f0, 0x00, 0x20, 0xe0, 0 },
	{ /* RPTI EP400 */ 0x0110, 0x00, 0x40, 0x95, 0 },
	{ /* SCM Ethernet */ 0x0ff0, 0x00, 0x20, 0xcb, 0 },
	{ /* Socket EA */ 0x4000, 0x00, 0xc0, 0x1b,
	  DELAY_OUTPUT | HAS_MISC_REG | USE_BIG_BUF },
	{ /* Socket LP-E CF+ */ 0x01c0, 0x00, 0xc0, 0x1b, 0 },
	{ /* SuperSocket RE450T */ 0x0110, 0x00, 0xe0, 0x98, 0 },
	{ /* Volktek NPL-402CT */ 0x0060, 0x00, 0x40, 0x05, 0 },
	{ /* NEC PC-9801N-J12 */ 0x0ff0, 0x00, 0x00, 0x4c, 0 },
	{ /* PCMCIA Technology OEM */ 0x01c8, 0x00, 0xa0, 0x0c, 0 }
};

#define NR_INFO		(sizeof(hw_info)/sizeof(hw_info_t))

static hw_info_t default_info = { 0, 0, 0, 0, 0 };

unsigned char dev_addr[6];

#define PCNET_CMD	0x00
#define PCNET_DATAPORT	0x10	/* NatSemi-defined port window offset. */
#define PCNET_RESET	0x1f	/* Issue a read to reset, a write to clear. */
#define PCNET_MISC	0x18	/* For IBM CCAE and Socket EA cards */

unsigned long nic_base;

static void pcnet_reset_8390(void)
{
	int i, r;

	PRINTK("nic base is %lx\n", nic_base);

	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE1+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);

	n2k_outb(n2k_inb(PCNET_RESET), PCNET_RESET);

	for (i = 0; i < 100; i++) {
		if ((r = (n2k_inb(EN0_ISR) & ENISR_RESET)) != 0)
			break;
		PRINTK("got %x in reset\n", r);
		my_udelay(100);
	}
	n2k_outb(ENISR_RESET, EN0_ISR); /* Ack intr. */

	if (i == 100)
		printf("pcnet_reset_8390() did not complete.\n");
} /* pcnet_reset_8390 */

static hw_info_t * get_prom(void ) {
	unsigned char prom[32];
	int i, j;
	struct {
		u_char value, offset;
	} program_seq[] = {
		{E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD}, /* Select page 0*/
		{0x48,	EN0_DCFG},	/* Set byte-wide (0x48) access. */
		{0x00,	EN0_RCNTLO},	/* Clear the count regs. */
		{0x00,	EN0_RCNTHI},
		{0x00,	EN0_IMR},	/* Mask completion irq. */
		{0xFF,	EN0_ISR},
		{E8390_RXOFF, EN0_RXCR},	/* 0x20  Set to monitor */
		{E8390_TXOFF, EN0_TXCR},	/* 0x02  and loopback mode. */
		{32,	EN0_RCNTLO},
		{0x00,	EN0_RCNTHI},
		{0x00,	EN0_RSARLO},	/* DMA starting at 0x0000. */
		{0x00,	EN0_RSARHI},
		{E8390_RREAD+E8390_START, E8390_CMD},
	};

	PRINTK("trying to get MAC via prom reading\n");

	pcnet_reset_8390();

	mdelay(10);

	for (i = 0; i < sizeof(program_seq)/sizeof(program_seq[0]); i++)
		n2k_outb(program_seq[i].value, program_seq[i].offset);

	PRINTK("PROM:");
	for (i = 0; i < 32; i++) {
		prom[i] = n2k_inb(PCNET_DATAPORT);
		PRINTK(" %02x", prom[i]);
	}
	PRINTK("\n");
	for (i = 0; i < NR_INFO; i++) {
		if ((prom[0] == hw_info[i].a0) &&
		    (prom[2] == hw_info[i].a1) &&
		    (prom[4] == hw_info[i].a2)) {
			PRINTK("matched board %d\n", i);
			break;
		}
	}
	if ((i < NR_INFO) || ((prom[28] == 0x57) && (prom[30] == 0x57))) {
		for (j = 0; j < 6; j++)
			dev_addr[j] = prom[j<<1];
		PRINTK("on exit i is %d/%ld\n", i, NR_INFO);
		PRINTK("MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n",
		       dev_addr[0],dev_addr[1],dev_addr[2],dev_addr[3],dev_addr[4],dev_addr[5]);
		return (i < NR_INFO) ? hw_info+i : &default_info;
	}
	return NULL;
}

/* U-boot specific routines */

#define NB 5

static unsigned char *pbuf = NULL;
static int plen[NB];
static int nrx = 0;

static int pkey = -1;
static int initialized=0;

void uboot_push_packet_len(int len) {
	PRINTK("pushed len = %d, nrx = %d\n", len, nrx);
	if (len>=2000) {
		printf("NE2000: packet too big\n");
		return;
	}
	if (nrx >= NB) {
		printf("losing packets in rx\n");
		return;
	}
	plen[nrx] = len;
	dp83902a_recv(&pbuf[nrx*2000], len);

	/*Just pass it to the upper layer*/
	NetReceive(&pbuf[nrx*2000], plen[nrx]);
}

void uboot_push_tx_done(int key, int val) {
	PRINTK("pushed key = %d\n", key);
	pkey = key;
}

int eth_init(bd_t *bd) {
	static hw_info_t * r;
	char ethaddr[20];

	PRINTK("### eth_init\n");

	if (!pbuf) {
		pbuf = malloc(NB*2000);
		if (!pbuf) {
			printf("Cannot allocate rx buffers\n");
			return -1;
		}
	}

#ifdef CONFIG_DRIVER_NE2000_CCR
	{
		volatile unsigned char *p =  (volatile unsigned char *) CONFIG_DRIVER_NE2000_CCR;

		PRINTK("CCR before is %x\n", *p);
		*p = CONFIG_DRIVER_NE2000_VAL;
		PRINTK("CCR after is %x\n", *p);
	}
#endif

	nic_base = CONFIG_DRIVER_NE2000_BASE;
	nic.base = (cyg_uint8 *) CONFIG_DRIVER_NE2000_BASE;

	r = get_prom();
	if (!r)
		return -1;

	sprintf (ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
		 dev_addr[0], dev_addr[1],
		 dev_addr[2], dev_addr[3],
		 dev_addr[4], dev_addr[5]) ;
	PRINTK("Set environment from HW MAC addr = \"%s\"\n", ethaddr);
	setenv ("ethaddr", ethaddr);


#define DP_DATA		0x10
	nic.data = nic.base + DP_DATA;
	nic.tx_buf1 = 0x40;
	nic.tx_buf2 = 0x48;
	nic.rx_buf_start = 0x50;
	nic.rx_buf_end = 0x80;

	if (dp83902a_init() == false)
		return -1;
	dp83902a_start(dev_addr);
	initialized=1;
	return 0;
}

void eth_halt() {

	PRINTK("### eth_halt\n");
	if(initialized)
		dp83902a_stop();
	initialized=0;
}

int eth_rx() {
dp83902a_poll();
return 1;
}

int eth_send(volatile void *packet, int length) {
	int tmo;

	PRINTK("### eth_send\n");

	pkey = -1;

	dp83902a_send((unsigned char *) packet, length, 666);
	tmo = get_timer (0) + TOUT * CFG_HZ;
	while(1) {
		dp83902a_poll();
		if (pkey != -1) {
			PRINTK("Packet sucesfully sent\n");
			return 0;
		}
		if (get_timer (0) >= tmo) {
			printf("transmission error (timoeut)\n");
			return 0;
		}

	}
	return 0;
}
#endif
