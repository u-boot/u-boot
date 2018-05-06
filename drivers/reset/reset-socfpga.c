// SPDX-License-Identifier: GPL-2.0+
/*
 * Socfpga Reset Controller Driver
 *
 * Copyright 2014 Steffen Trumtrar <s.trumtrar@pengutronix.de>
 *
 * based on
 * Allwinner SoCs Reset Controller driver
 *
 * Copyright 2013 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/of_access.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#define BANK_INCREMENT		4
#define NR_BANKS		8

struct socfpga_reset_data {
	void __iomem *membase;
};

static int socfpga_reset_assert(struct reset_ctl *reset_ctl)
{
	struct socfpga_reset_data *data = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;
	int reg_width = sizeof(u32);
	int bank = id / (reg_width * BITS_PER_BYTE);
	int offset = id % (reg_width * BITS_PER_BYTE);

	setbits_le32(data->membase + (bank * BANK_INCREMENT), BIT(offset));
	return 0;
}

static int socfpga_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct socfpga_reset_data *data = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;
	int reg_width = sizeof(u32);
	int bank = id / (reg_width * BITS_PER_BYTE);
	int offset = id % (reg_width * BITS_PER_BYTE);

	clrbits_le32(data->membase + (bank * BANK_INCREMENT), BIT(offset));
	return 0;
}

static int socfpga_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__,
	      reset_ctl, reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int socfpga_reset_free(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static const struct reset_ops socfpga_reset_ops = {
	.request = socfpga_reset_request,
	.free = socfpga_reset_free,
	.rst_assert = socfpga_reset_assert,
	.rst_deassert = socfpga_reset_deassert,
};

static int socfpga_reset_probe(struct udevice *dev)
{
	struct socfpga_reset_data *data = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	u32 modrst_offset;

	data->membase = devfdt_get_addr_ptr(dev);

	modrst_offset = fdtdec_get_int(blob, node, "altr,modrst-offset", 0x10);
	data->membase += modrst_offset;

	return 0;
}

static const struct udevice_id socfpga_reset_match[] = {
	{ .compatible = "altr,rst-mgr" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(socfpga_reset) = {
	.name = "socfpga-reset",
	.id = UCLASS_RESET,
	.of_match = socfpga_reset_match,
	.probe = socfpga_reset_probe,
	.priv_auto_alloc_size = sizeof(struct socfpga_reset_data),
	.ops = &socfpga_reset_ops,
};
