/*
 * spi driver for rockchip
 *
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2013 Rockchip Electronics
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <spi.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>
#include "rk_spi.h"

DECLARE_GLOBAL_DATA_PTR;

/* Change to 1 to output registers at the start of each transaction */
#define DEBUG_RK_SPI	0

struct rockchip_spi_platdata {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3288_spi of_plat;
#endif
	s32 frequency;		/* Default clock frequency, -1 for none */
	fdt_addr_t base;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
	uint activate_delay_us;		/* Delay to wait after activate */
};

struct rockchip_spi_priv {
	struct rockchip_spi *regs;
	struct clk clk;
	unsigned int max_freq;
	unsigned int mode;
	ulong last_transaction_us;	/* Time of last transaction end */
	u8 bits_per_word;		/* max 16 bits per word */
	u8 n_bytes;
	unsigned int speed_hz;
	unsigned int last_speed_hz;
	unsigned int tmode;
	uint input_rate;
};

#define SPI_FIFO_DEPTH		32

static void rkspi_dump_regs(struct rockchip_spi *regs)
{
	debug("ctrl0: \t\t0x%08x\n", readl(&regs->ctrlr0));
	debug("ctrl1: \t\t0x%08x\n", readl(&regs->ctrlr1));
	debug("ssienr: \t\t0x%08x\n", readl(&regs->enr));
	debug("ser: \t\t0x%08x\n", readl(&regs->ser));
	debug("baudr: \t\t0x%08x\n", readl(&regs->baudr));
	debug("txftlr: \t\t0x%08x\n", readl(&regs->txftlr));
	debug("rxftlr: \t\t0x%08x\n", readl(&regs->rxftlr));
	debug("txflr: \t\t0x%08x\n", readl(&regs->txflr));
	debug("rxflr: \t\t0x%08x\n", readl(&regs->rxflr));
	debug("sr: \t\t0x%08x\n", readl(&regs->sr));
	debug("imr: \t\t0x%08x\n", readl(&regs->imr));
	debug("isr: \t\t0x%08x\n", readl(&regs->isr));
	debug("dmacr: \t\t0x%08x\n", readl(&regs->dmacr));
	debug("dmatdlr: \t0x%08x\n", readl(&regs->dmatdlr));
	debug("dmardlr: \t0x%08x\n", readl(&regs->dmardlr));
}

static void rkspi_enable_chip(struct rockchip_spi *regs, bool enable)
{
	writel(enable ? 1 : 0, &regs->enr);
}

static void rkspi_set_clk(struct rockchip_spi_priv *priv, uint speed)
{
	uint clk_div;

	clk_div = clk_get_divisor(priv->input_rate, speed);
	debug("spi speed %u, div %u\n", speed, clk_div);

	writel(clk_div, &priv->regs->baudr);
	priv->last_speed_hz = speed;
}

static int rkspi_wait_till_not_busy(struct rockchip_spi *regs)
{
	unsigned long start;

	start = get_timer(0);
	while (readl(&regs->sr) & SR_BUSY) {
		if (get_timer(start) > ROCKCHIP_SPI_TIMEOUT_MS) {
			debug("RK SPI: Status keeps busy for 1000us after a read/write!\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void spi_cs_activate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_platdata *plat = bus->platdata;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;

	/* If it's too soon to do another transaction, wait */
	if (plat->deactivate_delay_us && priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < plat->deactivate_delay_us)
			udelay(plat->deactivate_delay_us - delay_us);
	}

	debug("activate cs%u\n", cs);
	writel(1 << cs, &regs->ser);
	if (plat->activate_delay_us)
		udelay(plat->activate_delay_us);
}

static void spi_cs_deactivate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_platdata *plat = bus->platdata;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;

	debug("deactivate cs%u\n", cs);
	writel(0, &regs->ser);

	/* Remember time of this transaction so we can honour the bus delay */
	if (plat->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int conv_of_platdata(struct udevice *dev)
{
	struct rockchip_spi_platdata *plat = dev->platdata;
	struct dtd_rockchip_rk3288_spi *dtplat = &plat->of_plat;
	struct rockchip_spi_priv *priv = dev_get_priv(dev);
	int ret;

	plat->base = dtplat->reg[0];
	plat->frequency = 20000000;
	ret = clk_get_by_index_platdata(dev, 0, dtplat->clocks, &priv->clk);
	if (ret < 0)
		return ret;
	dev->req_seq = 0;

	return 0;
}
#endif

static int rockchip_spi_ofdata_to_platdata(struct udevice *bus)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_spi_platdata *plat = dev_get_platdata(bus);
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	const void *blob = gd->fdt_blob;
	int node = bus->of_offset;
	int ret;

	plat->base = dev_get_addr(bus);

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret < 0) {
		debug("%s: Could not get clock for %s: %d\n", __func__,
		      bus->name, ret);
		return ret;
	}

	plat->frequency = fdtdec_get_int(blob, node, "spi-max-frequency",
					 50000000);
	plat->deactivate_delay_us = fdtdec_get_int(blob, node,
					"spi-deactivate-delay", 0);
	plat->activate_delay_us = fdtdec_get_int(blob, node,
						 "spi-activate-delay", 0);
	debug("%s: base=%x, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, (uint)plat->base, plat->frequency,
	      plat->deactivate_delay_us);
#endif

	return 0;
}

static int rockchip_spi_probe(struct udevice *bus)
{
	struct rockchip_spi_platdata *plat = dev_get_platdata(bus);
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	int ret;

	debug("%s: probe\n", __func__);
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = conv_of_platdata(bus);
	if (ret)
		return ret;
#endif
	priv->regs = (struct rockchip_spi *)plat->base;

	priv->last_transaction_us = timer_get_us();
	priv->max_freq = plat->frequency;

	/*
	 * Use 99 MHz as our clock since it divides nicely into 594 MHz which
	 * is the assumed speed for CLK_GENERAL.
	 */
	ret = clk_set_rate(&priv->clk, 99000000);
	if (ret < 0) {
		debug("%s: Failed to set clock: %d\n", __func__, ret);
		return ret;
	}
	priv->input_rate = ret;
	debug("%s: rate = %u\n", __func__, priv->input_rate);
	priv->bits_per_word = 8;
	priv->tmode = TMOD_TR; /* Tx & Rx */

	return 0;
}

static int rockchip_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;
	u8 spi_dfs, spi_tf;
	uint ctrlr0;

	/* Disable the SPI hardware */
	rkspi_enable_chip(regs, 0);

	switch (priv->bits_per_word) {
	case 8:
		priv->n_bytes = 1;
		spi_dfs = DFS_8BIT;
		spi_tf = HALF_WORD_OFF;
		break;
	case 16:
		priv->n_bytes = 2;
		spi_dfs = DFS_16BIT;
		spi_tf = HALF_WORD_ON;
		break;
	default:
		debug("%s: unsupported bits: %dbits\n", __func__,
		      priv->bits_per_word);
		return -EPROTONOSUPPORT;
	}

	if (priv->speed_hz != priv->last_speed_hz)
		rkspi_set_clk(priv, priv->speed_hz);

	/* Operation Mode */
	ctrlr0 = OMOD_MASTER << OMOD_SHIFT;

	/* Data Frame Size */
	ctrlr0 |= spi_dfs << DFS_SHIFT;

	/* set SPI mode 0..3 */
	if (priv->mode & SPI_CPOL)
		ctrlr0 |= SCOL_HIGH << SCOL_SHIFT;
	if (priv->mode & SPI_CPHA)
		ctrlr0 |= SCPH_TOGSTA << SCPH_SHIFT;

	/* Chip Select Mode */
	ctrlr0 |= CSM_KEEP << CSM_SHIFT;

	/* SSN to Sclk_out delay */
	ctrlr0 |= SSN_DELAY_ONE << SSN_DELAY_SHIFT;

	/* Serial Endian Mode */
	ctrlr0 |= SEM_LITTLE << SEM_SHIFT;

	/* First Bit Mode */
	ctrlr0 |= FBM_MSB << FBM_SHIFT;

	/* Byte and Halfword Transform */
	ctrlr0 |= spi_tf << HALF_WORD_TX_SHIFT;

	/* Rxd Sample Delay */
	ctrlr0 |= 0 << RXDSD_SHIFT;

	/* Frame Format */
	ctrlr0 |= FRF_SPI << FRF_SHIFT;

	/* Tx and Rx mode */
	ctrlr0 |= (priv->tmode & TMOD_MASK) << TMOD_SHIFT;

	writel(ctrlr0, &regs->ctrlr0);

	return 0;
}

static int rockchip_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	rkspi_enable_chip(priv->regs, false);

	return 0;
}

static int rockchip_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	int len = bitlen >> 3;
	const u8 *out = dout;
	u8 *in = din;
	int toread, towrite;
	int ret;

	debug("%s: dout=%p, din=%p, len=%x, flags=%lx\n", __func__, dout, din,
	      len, flags);
	if (DEBUG_RK_SPI)
		rkspi_dump_regs(regs);

	/* Assert CS before transfer */
	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev, slave_plat->cs);

	while (len > 0) {
		int todo = min(len, 0xffff);

		rkspi_enable_chip(regs, false);
		writel(todo - 1, &regs->ctrlr1);
		rkspi_enable_chip(regs, true);

		toread = todo;
		towrite = todo;
		while (toread || towrite) {
			u32 status = readl(&regs->sr);

			if (towrite && !(status & SR_TF_FULL)) {
				writel(out ? *out++ : 0, regs->txdr);
				towrite--;
			}
			if (toread && !(status & SR_RF_EMPT)) {
				u32 byte = readl(regs->rxdr);

				if (in)
					*in++ = byte;
				toread--;
			}
		}
		ret = rkspi_wait_till_not_busy(regs);
		if (ret)
			break;
		len -= todo;
	}

	/* Deassert CS after transfer */
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev, slave_plat->cs);

	rkspi_enable_chip(regs, false);

	return ret;
}

static int rockchip_spi_set_speed(struct udevice *bus, uint speed)
{
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	if (speed > ROCKCHIP_SPI_MAX_RATE)
		return -EINVAL;
	if (speed > priv->max_freq)
		speed = priv->max_freq;
	priv->speed_hz = speed;

	return 0;
}

static int rockchip_spi_set_mode(struct udevice *bus, uint mode)
{
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops rockchip_spi_ops = {
	.claim_bus	= rockchip_spi_claim_bus,
	.release_bus	= rockchip_spi_release_bus,
	.xfer		= rockchip_spi_xfer,
	.set_speed	= rockchip_spi_set_speed,
	.set_mode	= rockchip_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id rockchip_spi_ids[] = {
	{ .compatible = "rockchip,rk3288-spi" },
	{ }
};

U_BOOT_DRIVER(rockchip_spi) = {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	.name	= "rockchip_rk3288_spi",
#else
	.name	= "rockchip_spi",
#endif
	.id	= UCLASS_SPI,
	.of_match = rockchip_spi_ids,
	.ops	= &rockchip_spi_ops,
	.ofdata_to_platdata = rockchip_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct rockchip_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct rockchip_spi_priv),
	.probe	= rockchip_spi_probe,
};
