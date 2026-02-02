// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 * Goldfish TTY driver for U-Boot
 */

#include <dm.h>
#include <goldfish_tty.h>
#include <mapmem.h>
#include <serial.h>
#include <asm/io.h>
#include <linux/types.h>

/* Goldfish TTY Register Offsets */
#define GOLDFISH_TTY_PUT_CHAR       0x00
#define GOLDFISH_TTY_BYTES_READY    0x04
#define GOLDFISH_TTY_CMD            0x08
#define GOLDFISH_TTY_DATA_PTR       0x10
#define GOLDFISH_TTY_DATA_LEN       0x14
#define GOLDFISH_TTY_DATA_PTR_HIGH  0x18
#define GOLDFISH_TTY_VERSION        0x20

/* Commands */
#define CMD_WRITE_BUFFER   2
#define CMD_READ_BUFFER    3

struct goldfish_tty_priv {
	void __iomem *base;
	u8 rx_buf;
};

static int goldfish_serial_getc(struct udevice *dev)
{
	struct goldfish_tty_priv *priv = dev_get_priv(dev);
	unsigned long paddr;
	u32 count;

	count = __raw_readl(priv->base + GOLDFISH_TTY_BYTES_READY);
	if (count == 0)
		return -EAGAIN;

	paddr = virt_to_phys((void *)&priv->rx_buf);

	__raw_writel(0, priv->base + GOLDFISH_TTY_DATA_PTR_HIGH);
	__raw_writel(paddr, priv->base + GOLDFISH_TTY_DATA_PTR);
	__raw_writel(1, priv->base + GOLDFISH_TTY_DATA_LEN);
	__raw_writel(CMD_READ_BUFFER, priv->base + GOLDFISH_TTY_CMD);

	return priv->rx_buf;
}

static int goldfish_serial_putc(struct udevice *dev, const char ch)
{
	struct goldfish_tty_priv *priv = dev_get_priv(dev);

	__raw_writel(ch, priv->base + GOLDFISH_TTY_PUT_CHAR);

	return 0;
}

static int goldfish_serial_pending(struct udevice *dev, bool input)
{
	struct goldfish_tty_priv *priv = dev_get_priv(dev);

	if (input)
		return __raw_readl(priv->base + GOLDFISH_TTY_BYTES_READY);

	return 0;
}

static int goldfish_serial_of_to_plat(struct udevice *dev)
{
	struct goldfish_tty_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr != FDT_ADDR_T_NONE)
		plat->reg = addr;

	return 0;
}

static int goldfish_serial_probe(struct udevice *dev)
{
	struct goldfish_tty_plat *plat = dev_get_plat(dev);
	struct goldfish_tty_priv *priv = dev_get_priv(dev);

	if (!plat->reg)
		return -EINVAL;

	priv->base = map_sysmem(plat->reg, 0x1000);

	return 0;
}

static const struct dm_serial_ops goldfish_serial_ops = {
	.putc = goldfish_serial_putc,
	.pending = goldfish_serial_pending,
	.getc = goldfish_serial_getc,
};

static const struct udevice_id goldfish_serial_ids[] = {
	{ .compatible = "google,goldfish-tty" },
	{ }
};

U_BOOT_DRIVER(serial_goldfish) = {
	.name	= "serial_goldfish",
	.id	= UCLASS_SERIAL,
	.of_match = goldfish_serial_ids,
	.of_to_plat = goldfish_serial_of_to_plat,
	.plat_auto = sizeof(struct goldfish_tty_plat),
	.probe	= goldfish_serial_probe,
	.ops	= &goldfish_serial_ops,
	.priv_auto = sizeof(struct goldfish_tty_priv),
	.flags	= DM_FLAG_PRE_RELOC,
};
