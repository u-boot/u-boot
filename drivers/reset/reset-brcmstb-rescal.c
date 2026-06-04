// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB generic reset controller
 *
 * Copyright (C) 2024 EPAM Systems
 * Moved from linux kernel:
 * Copyright (C) 2018-2020 Broadcom
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>

#define BRCM_RESCAL_START 0x0
#define BRCM_RESCAL_START_BIT BIT(0)
#define BRCM_RESCAL_CTRL 0x4
#define BRCM_RESCAL_STATUS 0x8
#define BRCM_RESCAL_STATUS_BIT BIT(0)

struct brcm_rescal_reset {
	void __iomem *base;
};

/* Also doubles a deassert */
static int brcm_rescal_reset_set(struct reset_ctl *rst)
{
	struct brcm_rescal_reset *data = dev_get_priv(rst->dev);
	void __iomem *base = data->base;
	u32 reg;
	int ret;

	reg = readl(base + BRCM_RESCAL_START);
	writel(reg | BRCM_RESCAL_START_BIT, base + BRCM_RESCAL_START);
	reg = readl(base + BRCM_RESCAL_START);
	if (!(reg & BRCM_RESCAL_START_BIT)) {
		dev_err(rst->dev, "failed to start SATA/PCIe rescal\n");
		return -EIO;
	}

	ret = readl_poll_timeout(base + BRCM_RESCAL_STATUS, reg,
				 (reg & BRCM_RESCAL_STATUS_BIT), 100);
	if (ret) {
		dev_err(rst->dev, "time out on SATA/PCIe rescal\n");
		return ret;
	}

	reg = readl(base + BRCM_RESCAL_START);
	writel(reg & ~BRCM_RESCAL_START_BIT, base + BRCM_RESCAL_START);

	dev_dbg(rst->dev, "SATA/PCIe rescal success\n");
	return 0;
}

/* A dummy function - deassert/reset does all the work */
static int brcm_rescal_reset_assert(struct reset_ctl *rst)
{
	return 0;
}

static int brcm_rescal_reset_xlate(struct reset_ctl *reset_ctl,
				   struct ofnode_phandle_args *args)
{
	/* This is needed if #reset-cells == 0. */
	return 0;
}

static const struct reset_ops brcm_rescal_reset_ops = {
	.rst_deassert = brcm_rescal_reset_set,
	.rst_assert = brcm_rescal_reset_assert,
	.of_xlate = brcm_rescal_reset_xlate,
};

static int brcm_rescal_reset_probe(struct udevice *dev)
{
	struct brcm_rescal_reset *data = dev_get_priv(dev);

	data->base = dev_remap_addr(dev);
	if (!data->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id brcm_rescal_reset_of_match[] = {
	{.compatible = "brcm,bcm7216-pcie-sata-rescal"},
	{},
};

U_BOOT_DRIVER(brcmstb_reset_rescal) = {
	.name = "brcmstb-reset-rescal",
	.id = UCLASS_RESET,
	.of_match = brcm_rescal_reset_of_match,
	.ops = &brcm_rescal_reset_ops,
	.probe = brcm_rescal_reset_probe,
	.priv_auto = sizeof(struct brcm_rescal_reset),
};
