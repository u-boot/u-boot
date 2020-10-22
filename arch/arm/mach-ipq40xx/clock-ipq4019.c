// SPDX-License-Identifier: GPL-2.0+
/*
 * Clock drivers for Qualcomm IPQ40xx
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>

#include <dt-bindings/clock/qcom,ipq4019-gcc.h>

struct msm_clk_priv {
	phys_addr_t base;
};

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	switch (clk->id) {
	case GCC_BLSP1_UART1_APPS_CLK: /*UART1*/
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

static int msm_enable(struct clk *clk)
{
	switch (clk->id) {
	case GCC_BLSP1_QUP1_SPI_APPS_CLK: /*SPI1*/
		/* This clock is already initialized by SBL1 */
		return 0;
		break;
	case GCC_PRNG_AHB_CLK: /*PRNG*/
		/* This clock is already initialized by SBL1 */
		return 0;
		break;
	default:
		return 0;
	}
}

static struct clk_ops msm_clk_ops = {
	.set_rate = msm_clk_set_rate,
	.enable = msm_enable,
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
