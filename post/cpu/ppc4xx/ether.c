/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
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

/*
 * Ethernet test
 *
 * The Ethernet Media Access Controllers (EMAC) are tested in the
 * internal loopback mode.
 * The controllers are configured accordingly and several packets
 * are transmitted. The configurable test parameters are:
 *   MIN_PACKET_LENGTH - minimum size of packet to transmit
 *   MAX_PACKET_LENGTH - maximum size of packet to transmit
 *   TEST_NUM - number of tests
 */

#ifdef CONFIG_POST

#include <post.h>

#if CONFIG_POST & CFG_POST_ETHER

#include <asm/cache.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <405_mal.h>
#include <ppc4xx_enet.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_MFR_ETH_CLK_SEL_V(n)	((0x01<<27) / (n+1))
#endif

#define MIN_PACKET_LENGTH	64
#define MAX_PACKET_LENGTH	256
#define TEST_NUM		1

static volatile mal_desc_t tx __cacheline_aligned;
static volatile mal_desc_t rx __cacheline_aligned;
static char *tx_buf;
static char *rx_buf;

static void ether_post_init (int devnum, int hw_addr)
{
	int i;
#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	unsigned mode_reg;
	sys_info_t sysinfo;
#endif
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || defined(CONFIG_440SPE)
	unsigned long mfr;
#endif

#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	/* Need to get the OPB frequency so we can access the PHY */
	get_sys_info (&sysinfo);
#endif

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	/* provide clocks for EMAC internal loopback  */
	mfsdr (sdr_mfr, mfr);
	mfr |= SDR0_MFR_ETH_CLK_SEL_V(devnum);
	mtsdr (sdr_mfr, mfr);
	sync ();
#endif
	/* reset emac */
	out32 (EMAC_M0 + hw_addr, EMAC_M0_SRST);
	sync ();

	for (i = 0;; i++) {
		if (!(in32 (EMAC_M0 + hw_addr) & EMAC_M0_SRST))
			break;
		if (i >= 1000) {
			printf ("Timeout resetting EMAC\n");
			break;
		}
		udelay (1000);
	}
#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	/* Whack the M1 register */
	mode_reg = 0x0;
	if (sysinfo.freqOPB <= 50000000);
	else if (sysinfo.freqOPB <= 66666667)
		mode_reg |= EMAC_M1_OBCI_66;
	else if (sysinfo.freqOPB <= 83333333)
		mode_reg |= EMAC_M1_OBCI_83;
	else if (sysinfo.freqOPB <= 100000000)
		mode_reg |= EMAC_M1_OBCI_100;
	else
		mode_reg |= EMAC_M1_OBCI_GT100;

	out32 (EMAC_M1 + hw_addr, mode_reg);

#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */

	/* set the Mal configuration reg */
#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	mtdcr (malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA |
	       MAL_CR_PLBLT_DEFAULT | 0x00330000);
#else
	mtdcr (malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);
	/* Errata 1.12: MAL_1 -- Disable MAL bursting */
	if (get_pvr() == PVR_440GP_RB) {
		mtdcr (malmcr, mfdcr(malmcr) & ~MAL_CR_PLBB);
	}
#endif
	/* setup buffer descriptors */
	tx.ctrl = MAL_TX_CTRL_WRAP;
	tx.data_len = 0;
	tx.data_ptr = (char*)L1_CACHE_ALIGN((u32)tx_buf);

	rx.ctrl = MAL_TX_CTRL_WRAP | MAL_RX_CTRL_EMPTY;
	rx.data_len = 0;
	rx.data_ptr = (char*)L1_CACHE_ALIGN((u32)rx_buf);

	switch (devnum) {
	case 1:
		/* setup MAL tx & rx channel pointers */
#if defined (CONFIG_405EP) || defined (CONFIG_440EP) || defined (CONFIG_440GR)
		mtdcr (maltxctp2r, &tx);
#else
		mtdcr (maltxctp1r, &tx);
#endif
#if defined(CONFIG_440)
		mtdcr (maltxbattr, 0x0);
		mtdcr (malrxbattr, 0x0);
#endif
		mtdcr (malrxctp1r, &rx);
		/* set RX buffer size */
		mtdcr (malrcbs1, PKTSIZE_ALIGN / 16);
		break;
	case 0:
	default:
		/* setup MAL tx & rx channel pointers */
#if defined(CONFIG_440)
		mtdcr (maltxbattr, 0x0);
		mtdcr (malrxbattr, 0x0);
#endif
		mtdcr (maltxctp0r, &tx);
		mtdcr (malrxctp0r, &rx);
		/* set RX buffer size */
		mtdcr (malrcbs0, PKTSIZE_ALIGN / 16);
		break;
	}

	/* Enable MAL transmit and receive channels */
#if defined(CONFIG_405EP) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mtdcr (maltxcasr, (MAL_TXRX_CASR >> (devnum*2)));
#else
	mtdcr (maltxcasr, (MAL_TXRX_CASR >> devnum));
#endif
	mtdcr (malrxcasr, (MAL_TXRX_CASR >> devnum));

	/* set internal loopback mode */
#ifdef CFG_POST_ETHER_EXT_LOOPBACK
	out32 (EMAC_M1 + hw_addr, EMAC_M1_FDE | 0 |
	       EMAC_M1_RFS_4K | EMAC_M1_TX_FIFO_2K |
	       EMAC_M1_MF_100MBPS | EMAC_M1_IST |
	       in32 (EMAC_M1));
#else
	out32 (EMAC_M1 + hw_addr, EMAC_M1_FDE | EMAC_M1_ILE |
	       EMAC_M1_RFS_4K | EMAC_M1_TX_FIFO_2K |
	       EMAC_M1_MF_100MBPS | EMAC_M1_IST |
	       in32 (EMAC_M1));
#endif

	/* set transmit enable & receive enable */
	out32 (EMAC_M0 + hw_addr, EMAC_M0_TXE | EMAC_M0_RXE);

	/* enable broadcast address */
	out32 (EMAC_RXM + hw_addr, EMAC_RMR_BAE);

	/* set transmit request threshold register */
	out32 (EMAC_TRTR + hw_addr, 0x18000000);	/* 256 byte threshold */

	/* set receive	low/high water mark register */
#if defined(CONFIG_440)
	/* 440s has a 64 byte burst length */
	out32 (EMAC_RX_HI_LO_WMARK + hw_addr, 0x80009000);
#else
	/* 405s have a 16 byte burst length */
	out32 (EMAC_RX_HI_LO_WMARK + hw_addr, 0x0f002000);
#endif /* defined(CONFIG_440) */
	out32 (EMAC_TXM1 + hw_addr, 0xf8640000);

	/* Set fifo limit entry in tx mode 0 */
	out32 (EMAC_TXM0 + hw_addr, 0x00000003);
	/* Frame gap set */
	out32 (EMAC_I_FRAME_GAP_REG + hw_addr, 0x00000008);
	sync ();
}

static void ether_post_halt (int devnum, int hw_addr)
{
	int i = 0;
#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	unsigned long mfr;
#endif

	/* 1st reset MAL channel */
	/* Note: writing a 0 to a channel has no effect */
#if defined(CONFIG_405EP) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mtdcr (maltxcarr, MAL_TXRX_CASR >> (devnum * 2));
#else
	mtdcr (maltxcarr, MAL_TXRX_CASR >> devnum);
#endif
	mtdcr (malrxcarr, MAL_TXRX_CASR >> devnum);

	/* wait for reset */
	while (mfdcr (malrxcasr) & (MAL_TXRX_CASR >> devnum)) {
		if (i++ >= 1000)
			break;
		udelay (1000);
	}
	/* emac reset */
	out32 (EMAC_M0 + hw_addr, EMAC_M0_SRST);

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	/* remove clocks for EMAC internal loopback  */
	mfsdr (sdr_mfr, mfr);
	mfr &= ~SDR0_MFR_ETH_CLK_SEL_V(devnum);
	mtsdr (sdr_mfr, mfr);
#endif
}

static void ether_post_send (int devnum, int hw_addr, void *packet, int length)
{
	int i = 0;

	while (tx.ctrl & MAL_TX_CTRL_READY) {
		if (i++ > 100) {
			printf ("TX timeout\n");
			return;
		}
		udelay (1000);
	}
	tx.ctrl = MAL_TX_CTRL_READY | MAL_TX_CTRL_WRAP | MAL_TX_CTRL_LAST |
		EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP;
	tx.data_len = length;
	memcpy (tx.data_ptr, packet, length);
	sync ();

	out32 (EMAC_TXM0 + hw_addr, in32 (EMAC_TXM0 + hw_addr) | EMAC_TXM0_GNP0);
	sync ();
}

static int ether_post_recv (int devnum, int hw_addr, void *packet, int max_length)
{
	int length;
	int i = 0;

	while (rx.ctrl & MAL_RX_CTRL_EMPTY) {
		if (i++ > 100) {
			printf ("RX timeout\n");
			return 0;
		}
		udelay (1000);
	}
	length = rx.data_len - 4;
	if (length <= max_length)
		memcpy(packet, rx.data_ptr, length);
	sync ();

	rx.ctrl |= MAL_RX_CTRL_EMPTY;
	sync ();

	return length;
}

  /*
   * Test routines
   */

static void packet_fill (char *packet, int length)
{
	char c = (char) length;
	int i;

	/* set up ethernet header */
	memset (packet, 0xff, 14);

	for (i = 14; i < length; i++) {
		packet[i] = c++;
	}
}

static int packet_check (char *packet, int length)
{
	char c = (char) length;
	int i;

	for (i = 14; i < length; i++) {
		if (packet[i] != c++)
			return -1;
	}

	return 0;
}

static int test_ctlr (int devnum, int hw_addr)
{
	int res = -1;
	char packet_send[MAX_PACKET_LENGTH];
	char packet_recv[MAX_PACKET_LENGTH];
	int length;
	int i;
	int l;

	ether_post_init (devnum, hw_addr);

	for (i = 0; i < TEST_NUM; i++) {
		for (l = MIN_PACKET_LENGTH; l <= MAX_PACKET_LENGTH; l++) {
			packet_fill (packet_send, l);

			ether_post_send (devnum, hw_addr, packet_send, l);

			length = ether_post_recv (devnum, hw_addr, packet_recv,
						  sizeof (packet_recv));

			if (length != l || packet_check (packet_recv, length) < 0) {
				goto Done;
			}
		}
	}

	res = 0;

Done:

	ether_post_halt (devnum, hw_addr);

	if (res != 0) {
		post_log ("EMAC%d test failed\n", devnum);
	}

	return res;
}

int ether_post_test (int flags)
{
	int res = 0;

	/* Allocate tx & rx packet buffers */
	tx_buf = malloc (PKTSIZE_ALIGN + CFG_CACHELINE_SIZE);
	rx_buf = malloc (PKTSIZE_ALIGN + CFG_CACHELINE_SIZE);

	if (!tx_buf || !rx_buf) {
		printf ("Failed to allocate packet buffers\n");
		res = -1;
		goto out_free;
	}

	/* EMAC0 */
	if (test_ctlr (0, 0))
		res = -1;

	/* EMAC1 */
	if (test_ctlr (1, 0x100))
		res = -1;

out_free:
	free (tx_buf);
	free (rx_buf);

	return res;
}

#endif /* CONFIG_POST & CFG_POST_ETHER */
#endif /* CONFIG_POST */
