/*
 * Faraday FTMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>

#include "ftmac100.h"

#define ETH_ZLEN	60

struct ftmac100_data {
	struct ftmac100_txdes txdes[1];
	struct ftmac100_rxdes rxdes[PKTBUFSRX];
	int rx_index;
};

/*
 * Reset MAC
 */
static void ftmac100_reset (struct eth_device *dev)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)dev->iobase;

	debug ("%s()\n", __func__);

	writel (FTMAC100_MACCR_SW_RST, &ftmac100->maccr);

	while (readl (&ftmac100->maccr) & FTMAC100_MACCR_SW_RST)
		;
}

/*
 * Set MAC address
 */
static void ftmac100_set_mac (struct eth_device *dev, const unsigned char *mac)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)dev->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug ("%s(%x %x)\n", __func__, maddr, laddr);

	writel (maddr, &ftmac100->mac_madr);
	writel (laddr, &ftmac100->mac_ladr);
}

static void ftmac100_set_mac_from_env (struct eth_device *dev)
{
	eth_getenv_enetaddr ("ethaddr", dev->enetaddr);

	ftmac100_set_mac (dev, dev->enetaddr);
}

/*
 * disable transmitter, receiver
 */
static void ftmac100_halt (struct eth_device *dev)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)dev->iobase;

	debug ("%s()\n", __func__);

	writel (0, &ftmac100->maccr);
}

static int ftmac100_init (struct eth_device *dev, bd_t *bd)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)dev->iobase;
	struct ftmac100_data *priv = dev->priv;
	struct ftmac100_txdes *txdes = priv->txdes;
	struct ftmac100_rxdes *rxdes = priv->rxdes;
	unsigned int maccr;
	int i;

	debug ("%s()\n", __func__);

	ftmac100_reset (dev);

	/* set the ethernet address */

	ftmac100_set_mac_from_env (dev);

	/* disable all interrupts */

	writel (0, &ftmac100->imr);

	/* initialize descriptors */

	priv->rx_index = 0;

	txdes[0].txdes1			= FTMAC100_TXDES1_EDOTR;
	rxdes[PKTBUFSRX - 1].rxdes1	= FTMAC100_RXDES1_EDORR;

	for (i = 0; i < PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		rxdes[i].rxdes2 = (unsigned int)NetRxPackets[i];
		rxdes[i].rxdes1 |= FTMAC100_RXDES1_RXBUF_SIZE (PKTSIZE_ALIGN);
		rxdes[i].rxdes0 = FTMAC100_RXDES0_RXDMA_OWN;
	}

	/* transmit ring */

	writel ((unsigned int)txdes, &ftmac100->txr_badr);

	/* receive ring */

	writel ((unsigned int)rxdes, &ftmac100->rxr_badr);

	/* poll receive descriptor automatically */

	writel (FTMAC100_APTC_RXPOLL_CNT (1), &ftmac100->aptc);

	/* enable transmitter, receiver */

	maccr = FTMAC100_MACCR_XMT_EN |
		FTMAC100_MACCR_RCV_EN |
		FTMAC100_MACCR_XDMA_EN |
		FTMAC100_MACCR_RDMA_EN |
		FTMAC100_MACCR_CRC_APD |
		FTMAC100_MACCR_ENRX_IN_HALFTX |
		FTMAC100_MACCR_RX_RUNT |
		FTMAC100_MACCR_RX_BROADPKT;

	writel (maccr, &ftmac100->maccr);

	return 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftmac100_recv (struct eth_device *dev)
{
	struct ftmac100_data *priv = dev->priv;
	struct ftmac100_rxdes *curr_des;
	unsigned short rxlen;

	curr_des = &priv->rxdes[priv->rx_index];

	if (curr_des->rxdes0 & FTMAC100_RXDES0_RXDMA_OWN)
		return -1;

	if (curr_des->rxdes0 & (FTMAC100_RXDES0_RX_ERR |
				FTMAC100_RXDES0_CRC_ERR |
				FTMAC100_RXDES0_FTL |
				FTMAC100_RXDES0_RUNT |
				FTMAC100_RXDES0_RX_ODD_NB)) {
		return -1;
	}

	rxlen = FTMAC100_RXDES0_RFL (curr_des->rxdes0);

	debug ("%s(): RX buffer %d, %x received\n",
	       __func__, priv->rx_index, rxlen);

	/* pass the packet up to the protocol layers. */

	NetReceive ((void *)curr_des->rxdes2, rxlen);

	/* release buffer to DMA */

	curr_des->rxdes0 |= FTMAC100_RXDES0_RXDMA_OWN;

	priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;

	return 0;
}

/*
 * Send a data block via Ethernet
 */
static int ftmac100_send(struct eth_device *dev, void *packet, int length)
{
	struct ftmac100 *ftmac100 = (struct ftmac100 *)dev->iobase;
	struct ftmac100_data *priv = dev->priv;
	struct ftmac100_txdes *curr_des = priv->txdes;
	ulong start;

	if (curr_des->txdes0 & FTMAC100_TXDES0_TXDMA_OWN) {
		debug ("%s(): no TX descriptor available\n", __func__);
		return -1;
	}

	debug ("%s(%x, %x)\n", __func__, (int)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	/* initiate a transmit sequence */

	curr_des->txdes2 = (unsigned int)packet;	/* TXBUF_BADR */

	curr_des->txdes1 &= FTMAC100_TXDES1_EDOTR;
	curr_des->txdes1 |= FTMAC100_TXDES1_FTS |
			    FTMAC100_TXDES1_LTS |
			    FTMAC100_TXDES1_TXBUF_SIZE (length);

	curr_des->txdes0 = FTMAC100_TXDES0_TXDMA_OWN;

	/* start transmit */

	writel (1, &ftmac100->txpd);

	/* wait for transfer to succeed */

	start = get_timer(0);
	while (curr_des->txdes0 & FTMAC100_TXDES0_TXDMA_OWN) {
		if (get_timer(start) >= 5) {
			debug ("%s(): timed out\n", __func__);
			return -1;
		}
	}

	debug ("%s(): packet sent\n", __func__);

	return 0;
}

int ftmac100_initialize (bd_t *bd)
{
	struct eth_device *dev;
	struct ftmac100_data *priv;

	dev = malloc (sizeof *dev);
	if (!dev) {
		printf ("%s(): failed to allocate dev\n", __func__);
		goto out;
	}

	/* Transmit and receive descriptors should align to 16 bytes */

	priv = memalign (16, sizeof (struct ftmac100_data));
	if (!priv) {
		printf ("%s(): failed to allocate priv\n", __func__);
		goto free_dev;
	}

	memset (dev, 0, sizeof (*dev));
	memset (priv, 0, sizeof (*priv));

	sprintf (dev->name, "FTMAC100");
	dev->iobase	= CONFIG_FTMAC100_BASE;
	dev->init	= ftmac100_init;
	dev->halt	= ftmac100_halt;
	dev->send	= ftmac100_send;
	dev->recv	= ftmac100_recv;
	dev->priv	= priv;

	eth_register (dev);

	return 1;

free_dev:
	free (dev);
out:
	return 0;
}
