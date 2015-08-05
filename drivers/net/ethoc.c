/*
 * Opencore 10/100 ethernet mac driver
 *
 * Copyright (C) 2007-2008 Avionic Design Development GmbH
 * Copyright (C) 2008-2009 Avionic Design GmbH
 *   Thierry Reding <thierry.reding@avionic-design.de>
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/cache.h>

/* register offsets */
#define	MODER		0x00
#define	INT_SOURCE	0x04
#define	INT_MASK	0x08
#define	IPGT		0x0c
#define	IPGR1		0x10
#define	IPGR2		0x14
#define	PACKETLEN	0x18
#define	COLLCONF	0x1c
#define	TX_BD_NUM	0x20
#define	CTRLMODER	0x24
#define	MIIMODER	0x28
#define	MIICOMMAND	0x2c
#define	MIIADDRESS	0x30
#define	MIITX_DATA	0x34
#define	MIIRX_DATA	0x38
#define	MIISTATUS	0x3c
#define	MAC_ADDR0	0x40
#define	MAC_ADDR1	0x44
#define	ETH_HASH0	0x48
#define	ETH_HASH1	0x4c
#define	ETH_TXCTRL	0x50

/* mode register */
#define	MODER_RXEN	(1 <<  0)	/* receive enable */
#define	MODER_TXEN	(1 <<  1)	/* transmit enable */
#define	MODER_NOPRE	(1 <<  2)	/* no preamble */
#define	MODER_BRO	(1 <<  3)	/* broadcast address */
#define	MODER_IAM	(1 <<  4)	/* individual address mode */
#define	MODER_PRO	(1 <<  5)	/* promiscuous mode */
#define	MODER_IFG	(1 <<  6)	/* interframe gap for incoming frames */
#define	MODER_LOOP	(1 <<  7)	/* loopback */
#define	MODER_NBO	(1 <<  8)	/* no back-off */
#define	MODER_EDE	(1 <<  9)	/* excess defer enable */
#define	MODER_FULLD	(1 << 10)	/* full duplex */
#define	MODER_RESET	(1 << 11)	/* FIXME: reset (undocumented) */
#define	MODER_DCRC	(1 << 12)	/* delayed CRC enable */
#define	MODER_CRC	(1 << 13)	/* CRC enable */
#define	MODER_HUGE	(1 << 14)	/* huge packets enable */
#define	MODER_PAD	(1 << 15)	/* padding enabled */
#define	MODER_RSM	(1 << 16)	/* receive small packets */

/* interrupt source and mask registers */
#define	INT_MASK_TXF	(1 << 0)	/* transmit frame */
#define	INT_MASK_TXE	(1 << 1)	/* transmit error */
#define	INT_MASK_RXF	(1 << 2)	/* receive frame */
#define	INT_MASK_RXE	(1 << 3)	/* receive error */
#define	INT_MASK_BUSY	(1 << 4)
#define	INT_MASK_TXC	(1 << 5)	/* transmit control frame */
#define	INT_MASK_RXC	(1 << 6)	/* receive control frame */

#define	INT_MASK_TX	(INT_MASK_TXF | INT_MASK_TXE)
#define	INT_MASK_RX	(INT_MASK_RXF | INT_MASK_RXE)

#define	INT_MASK_ALL ( \
		INT_MASK_TXF | INT_MASK_TXE | \
		INT_MASK_RXF | INT_MASK_RXE | \
		INT_MASK_TXC | INT_MASK_RXC | \
		INT_MASK_BUSY \
	)

/* packet length register */
#define	PACKETLEN_MIN(min)		(((min) & 0xffff) << 16)
#define	PACKETLEN_MAX(max)		(((max) & 0xffff) <<  0)
#define	PACKETLEN_MIN_MAX(min, max)	(PACKETLEN_MIN(min) | \
					PACKETLEN_MAX(max))

/* transmit buffer number register */
#define	TX_BD_NUM_VAL(x)	(((x) <= 0x80) ? (x) : 0x80)

/* control module mode register */
#define	CTRLMODER_PASSALL	(1 << 0)	/* pass all receive frames */
#define	CTRLMODER_RXFLOW	(1 << 1)	/* receive control flow */
#define	CTRLMODER_TXFLOW	(1 << 2)	/* transmit control flow */

/* MII mode register */
#define	MIIMODER_CLKDIV(x)	((x) & 0xfe)	/* needs to be an even number */
#define	MIIMODER_NOPRE		(1 << 8)	/* no preamble */

/* MII command register */
#define	MIICOMMAND_SCAN		(1 << 0)	/* scan status */
#define	MIICOMMAND_READ		(1 << 1)	/* read status */
#define	MIICOMMAND_WRITE	(1 << 2)	/* write control data */

/* MII address register */
#define	MIIADDRESS_FIAD(x)		(((x) & 0x1f) << 0)
#define	MIIADDRESS_RGAD(x)		(((x) & 0x1f) << 8)
#define	MIIADDRESS_ADDR(phy, reg)	(MIIADDRESS_FIAD(phy) | \
					MIIADDRESS_RGAD(reg))

/* MII transmit data register */
#define	MIITX_DATA_VAL(x)	((x) & 0xffff)

/* MII receive data register */
#define	MIIRX_DATA_VAL(x)	((x) & 0xffff)

/* MII status register */
#define	MIISTATUS_LINKFAIL	(1 << 0)
#define	MIISTATUS_BUSY		(1 << 1)
#define	MIISTATUS_INVALID	(1 << 2)

/* TX buffer descriptor */
#define	TX_BD_CS		(1 <<  0)	/* carrier sense lost */
#define	TX_BD_DF		(1 <<  1)	/* defer indication */
#define	TX_BD_LC		(1 <<  2)	/* late collision */
#define	TX_BD_RL		(1 <<  3)	/* retransmission limit */
#define	TX_BD_RETRY_MASK	(0x00f0)
#define	TX_BD_RETRY(x)		(((x) & 0x00f0) >>  4)
#define	TX_BD_UR		(1 <<  8)	/* transmitter underrun */
#define	TX_BD_CRC		(1 << 11)	/* TX CRC enable */
#define	TX_BD_PAD		(1 << 12)	/* pad enable */
#define	TX_BD_WRAP		(1 << 13)
#define	TX_BD_IRQ		(1 << 14)	/* interrupt request enable */
#define	TX_BD_READY		(1 << 15)	/* TX buffer ready */
#define	TX_BD_LEN(x)		(((x) & 0xffff) << 16)
#define	TX_BD_LEN_MASK		(0xffff << 16)

#define	TX_BD_STATS		(TX_BD_CS | TX_BD_DF | TX_BD_LC | \
				TX_BD_RL | TX_BD_RETRY_MASK | TX_BD_UR)

/* RX buffer descriptor */
#define	RX_BD_LC	(1 <<  0)	/* late collision */
#define	RX_BD_CRC	(1 <<  1)	/* RX CRC error */
#define	RX_BD_SF	(1 <<  2)	/* short frame */
#define	RX_BD_TL	(1 <<  3)	/* too long */
#define	RX_BD_DN	(1 <<  4)	/* dribble nibble */
#define	RX_BD_IS	(1 <<  5)	/* invalid symbol */
#define	RX_BD_OR	(1 <<  6)	/* receiver overrun */
#define	RX_BD_MISS	(1 <<  7)
#define	RX_BD_CF	(1 <<  8)	/* control frame */
#define	RX_BD_WRAP	(1 << 13)
#define	RX_BD_IRQ	(1 << 14)	/* interrupt request enable */
#define	RX_BD_EMPTY	(1 << 15)
#define	RX_BD_LEN(x)	(((x) & 0xffff) << 16)

#define	RX_BD_STATS	(RX_BD_LC | RX_BD_CRC | RX_BD_SF | RX_BD_TL | \
			RX_BD_DN | RX_BD_IS | RX_BD_OR | RX_BD_MISS)

#define	ETHOC_BUFSIZ		1536
#define	ETHOC_ZLEN		64
#define	ETHOC_BD_BASE		0x400
#define	ETHOC_TIMEOUT		(HZ / 2)
#define	ETHOC_MII_TIMEOUT	(1 + (HZ / 5))

/**
 * struct ethoc - driver-private device structure
 * @num_tx:	number of send buffers
 * @cur_tx:	last send buffer written
 * @dty_tx:	last buffer actually sent
 * @num_rx:	number of receive buffers
 * @cur_rx:	current receive buffer
 */
struct ethoc {
	u32 num_tx;
	u32 cur_tx;
	u32 dty_tx;
	u32 num_rx;
	u32 cur_rx;
};

/**
 * struct ethoc_bd - buffer descriptor
 * @stat:	buffer statistics
 * @addr:	physical memory address
 */
struct ethoc_bd {
	u32 stat;
	u32 addr;
};

static inline u32 ethoc_read(struct eth_device *dev, size_t offset)
{
	return readl(dev->iobase + offset);
}

static inline void ethoc_write(struct eth_device *dev, size_t offset, u32 data)
{
	writel(data, dev->iobase + offset);
}

static inline void ethoc_read_bd(struct eth_device *dev, int index,
				 struct ethoc_bd *bd)
{
	size_t offset = ETHOC_BD_BASE + (index * sizeof(struct ethoc_bd));
	bd->stat = ethoc_read(dev, offset + 0);
	bd->addr = ethoc_read(dev, offset + 4);
}

static inline void ethoc_write_bd(struct eth_device *dev, int index,
				  const struct ethoc_bd *bd)
{
	size_t offset = ETHOC_BD_BASE + (index * sizeof(struct ethoc_bd));
	ethoc_write(dev, offset + 0, bd->stat);
	ethoc_write(dev, offset + 4, bd->addr);
}

static int ethoc_set_mac_address(struct eth_device *dev)
{
	u8 *mac = dev->enetaddr;

	ethoc_write(dev, MAC_ADDR0, (mac[2] << 24) | (mac[3] << 16) |
		    (mac[4] << 8) | (mac[5] << 0));
	ethoc_write(dev, MAC_ADDR1, (mac[0] << 8) | (mac[1] << 0));
	return 0;
}

static inline void ethoc_ack_irq(struct eth_device *dev, u32 mask)
{
	ethoc_write(dev, INT_SOURCE, mask);
}

static inline void ethoc_enable_rx_and_tx(struct eth_device *dev)
{
	u32 mode = ethoc_read(dev, MODER);
	mode |= MODER_RXEN | MODER_TXEN;
	ethoc_write(dev, MODER, mode);
}

static inline void ethoc_disable_rx_and_tx(struct eth_device *dev)
{
	u32 mode = ethoc_read(dev, MODER);
	mode &= ~(MODER_RXEN | MODER_TXEN);
	ethoc_write(dev, MODER, mode);
}

static int ethoc_init_ring(struct eth_device *dev)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	struct ethoc_bd bd;
	int i;

	priv->cur_tx = 0;
	priv->dty_tx = 0;
	priv->cur_rx = 0;

	/* setup transmission buffers */
	bd.stat = TX_BD_IRQ | TX_BD_CRC;

	for (i = 0; i < priv->num_tx; i++) {
		if (i == priv->num_tx - 1)
			bd.stat |= TX_BD_WRAP;

		ethoc_write_bd(dev, i, &bd);
	}

	bd.stat = RX_BD_EMPTY | RX_BD_IRQ;

	for (i = 0; i < priv->num_rx; i++) {
		bd.addr = (u32)net_rx_packets[i];
		if (i == priv->num_rx - 1)
			bd.stat |= RX_BD_WRAP;

		flush_dcache_range(bd.addr, bd.addr + PKTSIZE_ALIGN);
		ethoc_write_bd(dev, priv->num_tx + i, &bd);
	}

	return 0;
}

static int ethoc_reset(struct eth_device *dev)
{
	u32 mode;

	/* TODO: reset controller? */

	ethoc_disable_rx_and_tx(dev);

	/* TODO: setup registers */

	/* enable FCS generation and automatic padding */
	mode = ethoc_read(dev, MODER);
	mode |= MODER_CRC | MODER_PAD;
	ethoc_write(dev, MODER, mode);

	/* set full-duplex mode */
	mode = ethoc_read(dev, MODER);
	mode |= MODER_FULLD;
	ethoc_write(dev, MODER, mode);
	ethoc_write(dev, IPGT, 0x15);

	ethoc_ack_irq(dev, INT_MASK_ALL);
	ethoc_enable_rx_and_tx(dev);
	return 0;
}

static int ethoc_init(struct eth_device *dev, bd_t * bd)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	printf("ethoc\n");

	priv->num_tx = 1;
	priv->num_rx = PKTBUFSRX;
	ethoc_write(dev, TX_BD_NUM, priv->num_tx);
	ethoc_init_ring(dev);
	ethoc_reset(dev);

	return 0;
}

static int ethoc_update_rx_stats(struct ethoc_bd *bd)
{
	int ret = 0;

	if (bd->stat & RX_BD_TL) {
		debug("ETHOC: " "RX: frame too long\n");
		ret++;
	}

	if (bd->stat & RX_BD_SF) {
		debug("ETHOC: " "RX: frame too short\n");
		ret++;
	}

	if (bd->stat & RX_BD_DN)
		debug("ETHOC: " "RX: dribble nibble\n");

	if (bd->stat & RX_BD_CRC) {
		debug("ETHOC: " "RX: wrong CRC\n");
		ret++;
	}

	if (bd->stat & RX_BD_OR) {
		debug("ETHOC: " "RX: overrun\n");
		ret++;
	}

	if (bd->stat & RX_BD_LC) {
		debug("ETHOC: " "RX: late collision\n");
		ret++;
	}

	return ret;
}

static int ethoc_rx(struct eth_device *dev, int limit)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	int count;

	for (count = 0; count < limit; ++count) {
		u32 entry;
		struct ethoc_bd bd;

		entry = priv->num_tx + (priv->cur_rx % priv->num_rx);
		ethoc_read_bd(dev, entry, &bd);
		if (bd.stat & RX_BD_EMPTY)
			break;

		debug("%s(): RX buffer %d, %x received\n",
		      __func__, priv->cur_rx, bd.stat);
		if (ethoc_update_rx_stats(&bd) == 0) {
			int size = bd.stat >> 16;
			size -= 4;	/* strip the CRC */
			net_process_received_packet((void *)bd.addr, size);
		}

		/* clear the buffer descriptor so it can be reused */
		flush_dcache_range(bd.addr, bd.addr + PKTSIZE_ALIGN);
		bd.stat &= ~RX_BD_STATS;
		bd.stat |= RX_BD_EMPTY;
		ethoc_write_bd(dev, entry, &bd);
		priv->cur_rx++;
	}

	return count;
}

static int ethoc_update_tx_stats(struct ethoc_bd *bd)
{
	if (bd->stat & TX_BD_LC)
		debug("ETHOC: " "TX: late collision\n");

	if (bd->stat & TX_BD_RL)
		debug("ETHOC: " "TX: retransmit limit\n");

	if (bd->stat & TX_BD_UR)
		debug("ETHOC: " "TX: underrun\n");

	if (bd->stat & TX_BD_CS)
		debug("ETHOC: " "TX: carrier sense lost\n");

	return 0;
}

static void ethoc_tx(struct eth_device *dev)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	u32 entry = priv->dty_tx % priv->num_tx;
	struct ethoc_bd bd;

	ethoc_read_bd(dev, entry, &bd);
	if ((bd.stat & TX_BD_READY) == 0)
		(void)ethoc_update_tx_stats(&bd);
}

static int ethoc_send(struct eth_device *dev, void *packet, int length)
{
	struct ethoc *priv = (struct ethoc *)dev->priv;
	struct ethoc_bd bd;
	u32 entry;
	u32 pending;
	int tmo;

	entry = priv->cur_tx % priv->num_tx;
	ethoc_read_bd(dev, entry, &bd);
	if (unlikely(length < ETHOC_ZLEN))
		bd.stat |= TX_BD_PAD;
	else
		bd.stat &= ~TX_BD_PAD;
	bd.addr = (u32)packet;

	flush_dcache_range(bd.addr, bd.addr + length);
	bd.stat &= ~(TX_BD_STATS | TX_BD_LEN_MASK);
	bd.stat |= TX_BD_LEN(length);
	ethoc_write_bd(dev, entry, &bd);

	/* start transmit */
	bd.stat |= TX_BD_READY;
	ethoc_write_bd(dev, entry, &bd);

	/* wait for transfer to succeed */
	tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
	while (1) {
		pending = ethoc_read(dev, INT_SOURCE);
		ethoc_ack_irq(dev, pending & ~INT_MASK_RX);
		if (pending & INT_MASK_BUSY)
			debug("%s(): packet dropped\n", __func__);

		if (pending & INT_MASK_TX) {
			ethoc_tx(dev);
			break;
		}
		if (get_timer(0) >= tmo) {
			debug("%s(): timed out\n", __func__);
			return -1;
		}
	}

	debug("%s(): packet sent\n", __func__);
	return 0;
}

static void ethoc_halt(struct eth_device *dev)
{
	ethoc_disable_rx_and_tx(dev);
}

static int ethoc_recv(struct eth_device *dev)
{
	u32 pending;

	pending = ethoc_read(dev, INT_SOURCE);
	ethoc_ack_irq(dev, pending);
	if (pending & INT_MASK_BUSY)
		debug("%s(): packet dropped\n", __func__);
	if (pending & INT_MASK_RX) {
		debug("%s(): rx irq\n", __func__);
		ethoc_rx(dev, PKTBUFSRX);
	}

	return 0;
}

int ethoc_initialize(u8 dev_num, int base_addr)
{
	struct ethoc *priv;
	struct eth_device *dev;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return 0;
	dev = malloc(sizeof(*dev));
	if (!dev) {
		free(priv);
		return 0;
	}

	memset(dev, 0, sizeof(*dev));
	dev->priv = priv;
	dev->iobase = base_addr;
	dev->init = ethoc_init;
	dev->halt = ethoc_halt;
	dev->send = ethoc_send;
	dev->recv = ethoc_recv;
	dev->write_hwaddr = ethoc_set_mac_address;
	sprintf(dev->name, "%s-%hu", "ETHOC", dev_num);

	eth_register(dev);
	return 1;
}
