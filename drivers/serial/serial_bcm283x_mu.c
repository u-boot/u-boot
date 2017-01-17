/*
 * (C) Copyright 2016 Stephen Warren <swarren@wwwdotorg.org>
 *
 * Derived from pl01x code:
 *
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Simple U-Boot driver for the BCM283x mini UART */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <dm/platform_data/serial_bcm283x_mu.h>
#include <linux/compiler.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct bcm283x_mu_regs {
	u32 io;
	u32 iir;
	u32 ier;
	u32 lcr;
	u32 mcr;
	u32 lsr;
	u32 msr;
	u32 scratch;
	u32 cntl;
	u32 stat;
	u32 baud;
};

#define BCM283X_MU_LCR_DATA_SIZE_8	3

#define BCM283X_MU_LSR_TX_IDLE		BIT(6)
/* This actually means not full, but is named not empty in the docs */
#define BCM283X_MU_LSR_TX_EMPTY		BIT(5)
#define BCM283X_MU_LSR_RX_READY		BIT(0)

struct bcm283x_mu_priv {
	struct bcm283x_mu_regs *regs;
};

static int bcm283x_mu_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	u32 divider;

	if (plat->disabled || plat->skip_init)
		return 0;

	divider = plat->clock / (baudrate * 8);

	writel(BCM283X_MU_LCR_DATA_SIZE_8, &regs->lcr);
	writel(divider - 1, &regs->baud);

	return 0;
}

static int bcm283x_mu_serial_probe(struct udevice *dev)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);

	if (plat->disabled)
		return -ENODEV;

	priv->regs = (struct bcm283x_mu_regs *)plat->base;

	return 0;
}

static int bcm283x_mu_serial_getc(struct udevice *dev)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	u32 data;

	if (plat->disabled)
		return -EAGAIN;

	/* Wait until there is data in the FIFO */
	if (!(readl(&regs->lsr) & BCM283X_MU_LSR_RX_READY))
		return -EAGAIN;

	data = readl(&regs->io);

	return (int)data;
}

static int bcm283x_mu_serial_putc(struct udevice *dev, const char data)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;

	if (plat->disabled)
		return 0;

	/* Wait until there is space in the FIFO */
	if (!(readl(&regs->lsr) & BCM283X_MU_LSR_TX_EMPTY))
		return -EAGAIN;

	/* Send the character */
	writel(data, &regs->io);

	return 0;
}

static int bcm283x_mu_serial_pending(struct udevice *dev, bool input)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	unsigned int lsr;

	if (plat->disabled)
		return 0;

	lsr = readl(&regs->lsr);

	if (input) {
		WATCHDOG_RESET();
		return (lsr & BCM283X_MU_LSR_RX_READY) ? 1 : 0;
	} else {
		return (lsr & BCM283X_MU_LSR_TX_IDLE) ? 0 : 1;
	}
}

static const struct dm_serial_ops bcm283x_mu_serial_ops = {
	.putc = bcm283x_mu_serial_putc,
	.pending = bcm283x_mu_serial_pending,
	.getc = bcm283x_mu_serial_getc,
	.setbrg = bcm283x_mu_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id bcm283x_mu_serial_id[] = {
	{.compatible = "brcm,bcm2835-aux-uart"},
	{}
};

static int bcm283x_mu_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	plat->clock = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock",
				     1);
	plat->skip_init = fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
	                                  "skip-init");
	plat->disabled = false;
	return 0;
}
#endif

U_BOOT_DRIVER(serial_bcm283x_mu) = {
	.name = "serial_bcm283x_mu",
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(bcm283x_mu_serial_id),
	.ofdata_to_platdata = of_match_ptr(bcm283x_mu_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct bcm283x_mu_serial_platdata),
	.probe = bcm283x_mu_serial_probe,
	.ops = &bcm283x_mu_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct bcm283x_mu_priv),
};
