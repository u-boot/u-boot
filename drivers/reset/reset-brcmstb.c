// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB generic reset controller
 *
 * Copyright (C) 2024 EPAM Systems
 *
 * Moved from linux kernel:
 * Author: Florian Fainelli <f.fainelli@gmail.com>
 * Copyright (C) 2018 Broadcom
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>

struct brcmstb_reset {
	void __iomem *base;
};

#define SW_INIT_SET 0x00
#define SW_INIT_CLEAR 0x04
#define SW_INIT_STATUS 0x08

#define SW_INIT_BIT(id) BIT((id) & 0x1f)
#define SW_INIT_BANK(id) ((id) >> 5)

#define usleep_range(a, b) udelay((b))

/* A full bank contains extra registers that we are not utilizing but still
 * qualify as a single bank.
 */
#define SW_INIT_BANK_SIZE 0x18

static int brcmstb_reset_assert(struct reset_ctl *rst)
{
	unsigned int off = SW_INIT_BANK(rst->id) * SW_INIT_BANK_SIZE;
	struct brcmstb_reset *priv = dev_get_priv(rst->dev);

	writel_relaxed(SW_INIT_BIT(rst->id), priv->base + off + SW_INIT_SET);
	return 0;
}

static int brcmstb_reset_deassert(struct reset_ctl *rst)
{
	unsigned int off = SW_INIT_BANK(rst->id) * SW_INIT_BANK_SIZE;
	struct brcmstb_reset *priv = dev_get_priv(rst->dev);

	writel_relaxed(SW_INIT_BIT(rst->id), priv->base + off + SW_INIT_CLEAR);
	/* Maximum reset delay after de-asserting a line and seeing block
	 * operation is typically 14us for the worst case, build some slack
	 * here.
	 */
	usleep_range(100, 200);
	return 0;
}

static int brcmstb_reset_status(struct reset_ctl *rst)
{
	unsigned int off = SW_INIT_BANK(rst->id) * SW_INIT_BANK_SIZE;
	struct brcmstb_reset *priv = dev_get_priv(rst->dev);

	return readl_relaxed(priv->base + off + SW_INIT_STATUS) &
			SW_INIT_BIT(rst->id);
}

struct reset_ops brcmstb_reset_reset_ops = {
	.rst_assert = brcmstb_reset_assert,
	.rst_deassert = brcmstb_reset_deassert,
	.rst_status = brcmstb_reset_status};

static int brcmstb_reset_probe(struct udevice *dev)
{
	struct brcmstb_reset *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id brcmstb_reset_ids[] = {
	{.compatible = "brcm,brcmstb-reset"}, {/* sentinel */}};

U_BOOT_DRIVER(brcmstb_reset) = {
	.name = "brcmstb-reset",
	.id = UCLASS_RESET,
	.of_match = brcmstb_reset_ids,
	.ops = &brcmstb_reset_reset_ops,
	.probe = brcmstb_reset_probe,
	.priv_auto = sizeof(struct brcmstb_reset),
};
