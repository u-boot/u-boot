// SPDX-License-Identifier: GPL-2.0+
/*
 * Clock drivers for Qualcomm IPQ40xx
 *
 * Copyright (c) 2019 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>

struct msm_clk_priv {
	phys_addr_t base;
};

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	switch (clk->id) {
	case 26: /*UART1*/
		/* This clock is already initialized by SBL1 */
		return 0; 
		break;
	default:
		return 0;
	}
}

static int msm_clk_probe(struct udevice *dev)
{
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_get_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static ulong msm_clk_set_rate(struct clk *clk, ulong rate)
{
	return msm_set_rate(clk, rate);
}

static struct clk_ops msm_clk_ops = {
	.set_rate = msm_clk_set_rate,
};

static const struct udevice_id msm_clk_ids[] = {
	{ .compatible = "qcom,gcc-ipq4019" },
	{ }
};

U_BOOT_DRIVER(clk_msm) = {
	.name		= "clk_msm",
	.id		= UCLASS_CLK,
	.of_match	= msm_clk_ids,
	.ops		= &msm_clk_ops,
	.priv_auto_alloc_size = sizeof(struct msm_clk_priv),
	.probe		= msm_clk_probe,
};
