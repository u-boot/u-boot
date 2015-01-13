/*
 * Designware master SPI core controller driver
 *
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * Very loosely based on the Linux driver:
 * drivers/spi/spi-dw.c, which is:
 * Copyright (c) 2009, Intel Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <fdtdec.h>
#include <linux/compat.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/* Register offsets */
#define DW_SPI_CTRL0			0x00
#define DW_SPI_CTRL1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFLTR			0x18
#define DW_SPI_RXFLTR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET			0

#define SPI_FRF_OFFSET			4
#define SPI_FRF_SPI			0x0
#define SPI_FRF_SSP			0x1
#define SPI_FRF_MICROWIRE		0x2
#define SPI_FRF_RESV			0x3

#define SPI_MODE_OFFSET			6
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7

#define SPI_TMOD_OFFSET			8
#define SPI_TMOD_MASK			(0x3 << SPI_TMOD_OFFSET)
#define	SPI_TMOD_TR			0x0		/* xmit & recv */
#define SPI_TMOD_TO			0x1		/* xmit only */
#define SPI_TMOD_RO			0x2		/* recv only */
#define SPI_TMOD_EPROMREAD		0x3		/* eeprom read mode */

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12

/* Bit fields in SR, 7 bits */
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT			(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)

#define RX_TIMEOUT			1000		/* timeout in ms */

struct dw_spi_platdata {
	s32 frequency;		/* Default clock frequency, -1 for none */
	void __iomem *regs;
};

struct dw_spi_priv {
	void __iomem *regs;
	unsigned int freq;		/* Default frequency */
	unsigned int mode;

	int bits_per_word;
	u8 cs;			/* chip select pin */
	u8 tmode;		/* TR/TO/RO/EEPROM */
	u8 type;		/* SPI/SSP/MicroWire */
	int len;

	u32 fifo_len;		/* depth of the FIFO buffer */
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;
};

static inline u32 dw_readl(struct dw_spi_priv *priv, u32 offset)
{
	return __raw_readl(priv->regs + offset);
}

static inline void dw_writel(struct dw_spi_priv *priv, u32 offset, u32 val)
{
	__raw_writel(val, priv->regs + offset);
}

static inline u16 dw_readw(struct dw_spi_priv *priv, u32 offset)
{
	return __raw_readw(priv->regs + offset);
}

static inline void dw_writew(struct dw_spi_priv *priv, u32 offset, u16 val)
{
	__raw_writew(val, priv->regs + offset);
}

static int dw_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct dw_spi_platdata *plat = bus->platdata;
	const void *blob = gd->fdt_blob;
	int node = bus->of_offset;

	plat->regs = (struct dw_spi *)fdtdec_get_addr(blob, node, "reg");

	/* Use 500KHz as a suitable default */
	plat->frequency = fdtdec_get_int(blob, node, "spi-max-frequency",
					500000);
	debug("%s: regs=%p max-frequency=%d\n", __func__, plat->regs,
	      plat->frequency);

	return 0;
}

static inline void spi_enable_chip(struct dw_spi_priv *priv, int enable)
{
	dw_writel(priv, DW_SPI_SSIENR, (enable ? 1 : 0));
}

/* Restart the controller, disable all interrupts, clean rx fifo */
static void spi_hw_init(struct dw_spi_priv *priv)
{
	spi_enable_chip(priv, 0);
	dw_writel(priv, DW_SPI_IMR, 0xff);
	spi_enable_chip(priv, 1);

	/*
	 * Try to detect the FIFO depth if not set by interface driver,
	 * the depth could be from 2 to 256 from HW spec
	 */
	if (!priv->fifo_len) {
		u32 fifo;

		for (fifo = 2; fifo <= 256; fifo++) {
			dw_writew(priv, DW_SPI_TXFLTR, fifo);
			if (fifo != dw_readw(priv, DW_SPI_TXFLTR))
				break;
		}

		priv->fifo_len = (fifo == 2) ? 0 : fifo - 1;
		dw_writew(priv, DW_SPI_TXFLTR, 0);
	}
	debug("%s: fifo_len=%d\n", __func__, priv->fifo_len);
}

static int dw_spi_probe(struct udevice *bus)
{
	struct dw_spi_platdata *plat = dev_get_platdata(bus);
	struct dw_spi_priv *priv = dev_get_priv(bus);

	priv->regs = plat->regs;
	priv->freq = plat->frequency;

	/* Currently only bits_per_word == 8 supported */
	priv->bits_per_word = 8;

	priv->tmode = 0; /* Tx & Rx */

	/* Basic HW init */
	spi_hw_init(priv);

	return 0;
}

/* Return the max entries we can fill into tx fifo */
static inline u32 tx_max(struct dw_spi_priv *priv)
{
	u32 tx_left, tx_room, rxtx_gap;

	tx_left = (priv->tx_end - priv->tx) / (priv->bits_per_word >> 3);
	tx_room = priv->fifo_len - dw_readw(priv, DW_SPI_TXFLR);

	/*
	 * Another concern is about the tx/rx mismatch, we
	 * thought about using (priv->fifo_len - rxflr - txflr) as
	 * one maximum value for tx, but it doesn't cover the
	 * data which is out of tx/rx fifo and inside the
	 * shift registers. So a control from sw point of
	 * view is taken.
	 */
	rxtx_gap = ((priv->rx_end - priv->rx) - (priv->tx_end - priv->tx)) /
		(priv->bits_per_word >> 3);

	return min3(tx_left, tx_room, (u32)(priv->fifo_len - rxtx_gap));
}

/* Return the max entries we should read out of rx fifo */
static inline u32 rx_max(struct dw_spi_priv *priv)
{
	u32 rx_left = (priv->rx_end - priv->rx) / (priv->bits_per_word >> 3);

	return min_t(u32, rx_left, dw_readw(priv, DW_SPI_RXFLR));
}

static void dw_writer(struct dw_spi_priv *priv)
{
	u32 max = tx_max(priv);
	u16 txw = 0;

	while (max--) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if (priv->tx_end - priv->len) {
			if (priv->bits_per_word == 8)
				txw = *(u8 *)(priv->tx);
			else
				txw = *(u16 *)(priv->tx);
		}
		dw_writew(priv, DW_SPI_DR, txw);
		debug("%s: tx=0x%02x\n", __func__, txw);
		priv->tx += priv->bits_per_word >> 3;
	}
}

static int dw_reader(struct dw_spi_priv *priv)
{
	unsigned start = get_timer(0);
	u32 max;
	u16 rxw;

	/* Wait for rx data to be ready */
	while (rx_max(priv) == 0) {
		if (get_timer(start) > RX_TIMEOUT)
			return -ETIMEDOUT;
	}

	max = rx_max(priv);

	while (max--) {
		rxw = dw_readw(priv, DW_SPI_DR);
		debug("%s: rx=0x%02x\n", __func__, rxw);

		/*
		 * Care about rx only if the transfer's original "rx" is
		 * not null
		 */
		if (priv->rx_end - priv->len) {
			if (priv->bits_per_word == 8)
				*(u8 *)(priv->rx) = rxw;
			else
				*(u16 *)(priv->rx) = rxw;
		}
		priv->rx += priv->bits_per_word >> 3;
	}

	return 0;
}

static int poll_transfer(struct dw_spi_priv *priv)
{
	int ret;

	do {
		dw_writer(priv);
		ret = dw_reader(priv);
		if (ret < 0)
			return ret;
	} while (priv->rx_end > priv->rx);

	return 0;
}

static int dw_spi_xfer(struct udevice *dev, unsigned int bitlen,
		       const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct dw_spi_priv *priv = dev_get_priv(bus);
	const u8 *tx = dout;
	u8 *rx = din;
	int ret = 0;
	u32 cr0 = 0;
	u32 cs;

	/* spi core configured to do 8 bit transfers */
	if (bitlen % 8) {
		debug("Non byte aligned SPI transfer.\n");
		return -1;
	}

	cr0 = (priv->bits_per_word - 1) | (priv->type << SPI_FRF_OFFSET) |
		(priv->mode << SPI_MODE_OFFSET) |
		(priv->tmode << SPI_TMOD_OFFSET);

	if (rx && tx)
		priv->tmode = SPI_TMOD_TR;
	else if (rx)
		priv->tmode = SPI_TMOD_RO;
	else
		priv->tmode = SPI_TMOD_TO;

	cr0 &= ~SPI_TMOD_MASK;
	cr0 |= (priv->tmode << SPI_TMOD_OFFSET);

	priv->len = bitlen >> 3;
	debug("%s: rx=%p tx=%p len=%d [bytes]\n", __func__, rx, tx, priv->len);

	priv->tx = (void *)tx;
	priv->tx_end = priv->tx + priv->len;
	priv->rx = rx;
	priv->rx_end = priv->rx + priv->len;

	/* Disable controller before writing control registers */
	spi_enable_chip(priv, 0);

	debug("%s: cr0=%08x\n", __func__, cr0);
	/* Reprogram cr0 only if changed */
	if (dw_readw(priv, DW_SPI_CTRL0) != cr0)
		dw_writew(priv, DW_SPI_CTRL0, cr0);

	/*
	 * Configure the desired SS (slave select 0...3) in the controller
	 * The DW SPI controller will activate and deactivate this CS
	 * automatically. So no cs_activate() etc is needed in this driver.
	 */
	cs = spi_chip_select(dev);
	dw_writel(priv, DW_SPI_SER, 1 << cs);

	/* Enable controller after writing control registers */
	spi_enable_chip(priv, 1);

	/* Start transfer in a polling loop */
	ret = poll_transfer(priv);

	return ret;
}

static int dw_spi_set_speed(struct udevice *bus, uint speed)
{
	struct dw_spi_platdata *plat = bus->platdata;
	struct dw_spi_priv *priv = dev_get_priv(bus);
	u16 clk_div;

	if (speed > plat->frequency)
		speed = plat->frequency;

	/* Disable controller before writing control registers */
	spi_enable_chip(priv, 0);

	/* clk_div doesn't support odd number */
	clk_div = cm_get_spi_controller_clk_hz() / speed;
	clk_div = (clk_div + 1) & 0xfffe;
	dw_writel(priv, DW_SPI_BAUDR, clk_div);

	/* Enable controller after writing control registers */
	spi_enable_chip(priv, 1);

	priv->freq = speed;
	debug("%s: regs=%p speed=%d clk_div=%d\n", __func__, priv->regs,
	      priv->freq, clk_div);

	return 0;
}

static int dw_spi_set_mode(struct udevice *bus, uint mode)
{
	struct dw_spi_priv *priv = dev_get_priv(bus);

	/*
	 * Can't set mode yet. Since this depends on if rx, tx, or
	 * rx & tx is requested. So we have to defer this to the
	 * real transfer function.
	 */
	priv->mode = mode;
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops dw_spi_ops = {
	.xfer		= dw_spi_xfer,
	.set_speed	= dw_spi_set_speed,
	.set_mode	= dw_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id dw_spi_ids[] = {
	{ .compatible = "snps,dw-apb-ssi" },
	{ }
};

U_BOOT_DRIVER(dw_spi) = {
	.name = "dw_spi",
	.id = UCLASS_SPI,
	.of_match = dw_spi_ids,
	.ops = &dw_spi_ops,
	.ofdata_to_platdata = dw_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct dw_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct dw_spi_priv),
	.per_child_auto_alloc_size = sizeof(struct spi_slave),
	.probe = dw_spi_probe,
};
