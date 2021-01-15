// SPDX-License-Identifier: GPL-2.0+
/*
 * TI gate clock support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Loosely based on Linux kernel drivers/clk/ti/gate.c
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <linux/clk-provider.h>

struct clk_ti_gate_priv {
	fdt_addr_t reg;
	u8 enable_bit;
	u32 flags;
	bool invert_enable;
};

static int clk_ti_gate_disable(struct clk *clk)
{
	struct clk_ti_gate_priv *priv = dev_get_priv(clk->dev);
	u32 v;

	v = readl(priv->reg);
	if (priv->invert_enable)
		v |= (1 << priv->enable_bit);
	else
		v &= ~(1 << priv->enable_bit);

	writel(v, priv->reg);
	/* No OCP barrier needed here since it is a disable operation */
	return 0;
}

static int clk_ti_gate_enable(struct clk *clk)
{
	struct clk_ti_gate_priv *priv = dev_get_priv(clk->dev);
	u32 v;

	v = readl(priv->reg);
	if (priv->invert_enable)
		v &= ~(1 << priv->enable_bit);
	else
		v |= (1 << priv->enable_bit);

	writel(v, priv->reg);
	/* OCP barrier */
	v = readl(priv->reg);
	return 0;
}

static int clk_ti_gate_of_to_plat(struct udevice *dev)
{
	struct clk_ti_gate_priv *priv = dev_get_priv(dev);

	priv->reg = dev_read_addr(dev);
	if (priv->reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get control register\n");
		return -EINVAL;
	}

	dev_dbg(dev, "reg=0x%08lx\n", priv->reg);
	priv->enable_bit = dev_read_u32_default(dev, "ti,bit-shift", 0);
	if (dev_read_bool(dev, "ti,set-rate-parent"))
		priv->flags |= CLK_SET_RATE_PARENT;

	priv->invert_enable = dev_read_bool(dev, "ti,set-bit-to-disable");
	return 0;
}

static struct clk_ops clk_ti_gate_ops = {
	.enable = clk_ti_gate_enable,
	.disable = clk_ti_gate_disable,
};

static const struct udevice_id clk_ti_gate_of_match[] = {
	{ .compatible = "ti,gate-clock" },
	{ },
};

U_BOOT_DRIVER(clk_ti_gate) = {
	.name = "ti_gate_clock",
	.id = UCLASS_CLK,
	.of_match = clk_ti_gate_of_match,
	.of_to_plat = clk_ti_gate_of_to_plat,
	.priv_auto = sizeof(struct clk_ti_gate_priv),
	.ops = &clk_ti_gate_ops,
};
