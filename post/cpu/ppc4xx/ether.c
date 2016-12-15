/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
 *   CONFIG_SYS_POST_ETH_LOOPS - Number of test loops. Each loop
 *     is tested with a different frame length. Starting with
 *     MAX_PACKET_LENGTH and going down to MIN_PACKET_LENGTH.
 *     Defaults to 10 and can be overridden in the board config header.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_ETHER

#include <asm/cache.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/ppc4xx-mal.h>
#include <asm/ppc4xx-emac.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Get count of EMAC devices (doesn't have to be the max. possible number
 * supported by the cpu)
 *
 * CONFIG_BOARD_EMAC_COUNT added so now a "dynamic" way to configure the
 * EMAC count is possible. As it is needed for the Kilauea/Haleakala
 * 405EX/405EXr eval board, using the same binary.
 */
#if defined(CONFIG_BOARD_EMAC_COUNT)
#define LAST_EMAC_NUM	board_emac_count()
#else /* CONFIG_BOARD_EMAC_COUNT */
#if defined(CONFIG_HAS_ETH3)
#define LAST_EMAC_NUM	4
#elif defined(CONFIG_HAS_ETH2)
#define LAST_EMAC_NUM	3
#elif defined(CONFIG_HAS_ETH1)
#define LAST_EMAC_NUM	2
#else
#define LAST_EMAC_NUM	1
#endif
#endif /* CONFIG_BOARD_EMAC_COUNT */

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_MFR_ETH_CLK_SEL_V(n)	((0x01<<27) / (n+1))
#endif

#define MIN_PACKET_LENGTH	64
#define MAX_PACKET_LENGTH	1514
#ifndef CONFIG_SYS_POST_ETH_LOOPS
#define CONFIG_SYS_POST_ETH_LOOPS	10
#endif
#define PACKET_INCR	((MAX_PACKET_LENGTH - MIN_PACKET_LENGTH) / \
			 CONFIG_SYS_POST_ETH_LOOPS)

static volatile mal_desc_t tx __cacheline_aligned;
static volatile mal_desc_t rx __cacheline_aligned;
static char *tx_buf;
static char *rx_buf;

int board_emac_count(void);

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
	mfsdr (SDR0_MFR, mfr);
	mfr |= SDR0_MFR_ETH_CLK_SEL_V(devnum);
	mtsdr (SDR0_MFR, mfr);
	sync ();
#endif
	/* reset emac */
	out_be32 ((void*)(EMAC0_MR0 + hw_addr), EMAC_MR0_SRST);
	sync ();

	for (i = 0;; i++) {
		if (!(in_be32 ((void*)(EMAC0_MR0 + hw_addr)) & EMAC_MR0_SRST))
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
		mode_reg |= EMAC_MR1_OBCI_66;
	else if (sysinfo.freqOPB <= 83333333)
		mode_reg |= EMAC_MR1_OBCI_83;
	else if (sysinfo.freqOPB <= 100000000)
		mode_reg |= EMAC_MR1_OBCI_100;
	else
		mode_reg |= EMAC_MR1_OBCI_GT100;

	out_be32 ((void*)(EMAC0_MR1 + hw_addr), mode_reg);

#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */

	/* set the Mal configuration reg */
#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	mtdcr (MAL0_CFG, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA |
	       MAL_CR_PLBLT_DEFAULT | 0x00330000);
#else
	mtdcr (MAL0_CFG, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);
	/* Errata 1.12: MAL_1 -- Disable MAL bursting */
	if (get_pvr() == PVR_440GP_RB) {
		mtdcr (MAL0_CFG, mfdcr(MAL0_CFG) & ~MAL_CR_PLBB);
	}
#endif
	/* setup buffer descriptors */
	tx.ctrl = MAL_TX_CTRL_WRAP;
	tx.data_len = 0;
	tx.data_ptr = (char*)L1_CACHE_ALIGN((u32)tx_buf);

	rx.ctrl = MAL_TX_CTRL_WRAP | MAL_RX_CTRL_EMPTY;
	rx.data_len = 0;
	rx.data_ptr = (char*)L1_CACHE_ALIGN((u32)rx_buf);
	flush_dcache_range((u32)&rx, (u32)&rx + sizeof(mal_desc_t));
	flush_dcache_range((u32)&tx, (u32)&tx + sizeof(mal_desc_t));

	switch (devnum) {
	case 1:
		/* setup MAL tx & rx channel pointers */
#if defined (CONFIG_405EP) || defined (CONFIG_440EP) || defined (CONFIG_440GR)
		mtdcr (MAL0_TXCTP2R, &tx);
#else
		mtdcr (MAL0_TXCTP1R, &tx);
#endif
#if defined(CONFIG_440)
		mtdcr (MAL0_TXBADDR, 0x0);
		mtdcr (MAL0_RXBADDR, 0x0);
#endif
		mtdcr (MAL0_RXCTP1R, &rx);
		/* set RX buffer size */
		mtdcr (MAL0_RCBS1, PKTSIZE_ALIGN / 16);
		break;
	case 0:
	default:
		/* setup MAL tx & rx channel pointers */
#if defined(CONFIG_440)
		mtdcr (MAL0_TXBADDR, 0x0);
		mtdcr (MAL0_RXBADDR, 0x0);
#endif
		mtdcr (MAL0_TXCTP0R, &tx);
		mtdcr (MAL0_RXCTP0R, &rx);
		/* set RX buffer size */
		mtdcr (MAL0_RCBS0, PKTSIZE_ALIGN / 16);
		break;
	}

	/* Enable MAL transmit and receive channels */
#if defined(CONFIG_405EP) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	mtdcr (MAL0_TXCASR, (MAL_TXRX_CASR >> (devnum*2)));
#else
	mtdcr (MAL0_TXCASR, (MAL_TXRX_CASR >> devnum));
#endif
	mtdcr (MAL0_RXCASR, (MAL_TXRX_CASR >> devnum));

	/* set internal loopback mode */
#ifdef CONFIG_SYS_POST_ETHER_EXT_LOOPBACK
	out_be32 ((void*)(EMAC0_MR1 + hw_addr), EMAC_MR1_FDE | 0 |
		  EMAC_MR1_RFS_4K | EMAC_MR1_TX_FIFO_2K |
		  EMAC_MR1_MF_100MBPS | EMAC_MR1_IST |
		  in_be32 ((void*)(EMAC0_MR1 + hw_addr)));
#else
	out_be32 ((void*)(EMAC0_MR1 + hw_addr), EMAC_MR1_FDE | EMAC_MR1_ILE |
		  EMAC_MR1_RFS_4K | EMAC_MR1_TX_FIFO_2K |
		  EMAC_MR1_MF_100MBPS | EMAC_MR1_IST |
		  in_be32 ((void*)(EMAC0_MR1 + hw_addr)));
#endif

	/* set transmit enable & receive enable */
	out_be32 ((void*)(EMAC0_MR0 + hw_addr), EMAC_MR0_TXE | EMAC_MR0_RXE);

	/* enable broadcast address */
	out_be32 ((void*)(EMAC0_RXM + hw_addr), EMAC_RMR_BAE);

	/* set transmit request threshold register */
	out_be32 ((void*)(EMAC0_TRTR + hw_addr), 0x18000000);	/* 256 byte threshold */

	/* set receive	low/high water mark register */
#if defined(CONFIG_440)
	/* 440s has a 64 byte burst length */
	out_be32 ((void*)(EMAC0_RX_HI_LO_WMARK + hw_addr), 0x80009000);
#else
	/* 405s have a 16 byte burst length */
	out_be32 ((void*)(EMAC0_RX_HI_LO_WMARK + hw_addr), 0x0f002000);
#endif /* defined(CONFIG_440) */
	out_be32 ((void*)(EMAC0_TMR1 + hw_addr), 0xf8640000);

	/* Set fifo limit entry in tx mode 0 */
	out_be32 ((void*)(EMAC0_TMR0 + hw_addr), 0x00000003);
	/* Frame gap set */
	out_be32 ((void*)(EMAC0_I_FRAME_GAP_REG + hw_addr), 0x00000008);
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
	mtdcr (MAL0_TXCARR, MAL_TXRX_CASR >> (devnum * 2));
#else
	mtdcr (MAL0_TXCARR, MAL_TXRX_CASR >> devnum);
#endif
	mtdcr (MAL0_RXCARR, MAL_TXRX_CASR >> devnum);

	/* wait for reset */
	while (mfdcr (MAL0_RXCASR) & (MAL_TXRX_CASR >> devnum)) {
		if (i++ >= 1000)
			break;
		udelay (1000);
	}
	/* emac reset */
	out_be32 ((void*)(EMAC0_MR0 + hw_addr), EMAC_MR0_SRST);

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	/* remove clocks for EMAC internal loopback  */
	mfsdr (SDR0_MFR, mfr);
	mfr &= ~SDR0_MFR_ETH_CLK_SEL_V(devnum);
	mtsdr (SDR0_MFR, mfr);
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
		invalidate_dcache_range((u32)&tx, (u32)&tx + sizeof(mal_desc_t));
	}
	tx.ctrl = MAL_TX_CTRL_READY | MAL_TX_CTRL_WRAP | MAL_TX_CTRL_LAST |
		EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP;
	tx.data_len = length;
	memcpy (tx.data_ptr, packet, length);
	flush_dcache_range((u32)&tx, (u32)&tx + sizeof(mal_desc_t));
	flush_dcache_range((u32)tx.data_ptr, (u32)tx.data_ptr + length);
	sync ();

	out_be32 ((void*)(EMAC0_TMR0 + hw_addr), in_be32 ((void*)(EMAC0_TMR0 + hw_addr)) | EMAC_TMR0_GNP0);
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
		invalidate_dcache_range((u32)&rx, (u32)&rx + sizeof(mal_desc_t));
	}
	length = rx.data_len - 4;
	if (length <= max_length) {
		invalidate_dcache_range((u32)rx.data_ptr, (u32)rx.data_ptr + length);
		memcpy(packet, rx.data_ptr, length);
	}
	sync ();

	rx.ctrl |= MAL_RX_CTRL_EMPTY;
	flush_dcache_range((u32)&rx, (u32)&rx + sizeof(mal_desc_t));
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

	char packet_send[MAX_PACKET_LENGTH];
	char packet_recv[MAX_PACKET_LENGTH];
static int test_ctlr (int devnum, int hw_addr)
{
	int res = -1;
	int length;
	int l;

	ether_post_init (devnum, hw_addr);

	for (l = MAX_PACKET_LENGTH; l >= MIN_PACKET_LENGTH;
	     l -= PACKET_INCR) {
		packet_fill (packet_send, l);

		ether_post_send (devnum, hw_addr, packet_send, l);

		length = ether_post_recv (devnum, hw_addr, packet_recv,
					  sizeof (packet_recv));

		if (length != l || packet_check (packet_recv, length) < 0) {
			goto Done;
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
	int i;

	/* Allocate tx & rx packet buffers */
	tx_buf = malloc (PKTSIZE_ALIGN + CONFIG_SYS_CACHELINE_SIZE);
	rx_buf = malloc (PKTSIZE_ALIGN + CONFIG_SYS_CACHELINE_SIZE);

	if (!tx_buf || !rx_buf) {
		printf ("Failed to allocate packet buffers\n");
		res = -1;
		goto out_free;
	}

	for (i = 0; i < LAST_EMAC_NUM; i++) {
		if (test_ctlr (i, i*0x100))
			res = -1;
	}

out_free:
	free (tx_buf);
	free (rx_buf);

	return res;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_ETHER */
