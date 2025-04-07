// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8096
 *
 * (C) Copyright 2017 Jorge Ramirez Ortiz <jorge.ramirez-ortiz@linaro.org>
 *
 * Based on Little Kernel driver, simplified
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-msm8996.h>

#include "clock-qcom.h"

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x0000)
#define APCS_GPLL_ENA_VOTE		(0x52000)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(0x52004)

#define SDCC2_BCR			(0x14000) /* block reset */
#define SDCC2_APPS_CBCR			(0x14004) /* branch control */
#define SDCC2_AHB_CBCR			(0x14008)
#define SDCC2_CMD_RCGR			(0x14010)

#define BLSP2_AHB_CBCR			(0x25004)
#define BLSP2_UART2_APPS_CBCR		(0x29004)
#define BLSP2_UART2_APPS_CMD_RCGR	(0x2900C)

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE		BIT(30)
#define APCS_GPLL_ENA_VOTE_GPLL0	BIT(0)

static const struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = APCS_GPLL_ENA_VOTE_GPLL0,
};

static struct vote_clk gcc_blsp2_ahb_clk = {
	.cbcr_reg = BLSP2_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(15),
};

static int clk_init_sdc(struct msm_clk_priv *priv, uint rate)
{
	int div = 5;

	clk_enable_cbc(priv->base + SDCC2_AHB_CBCR);
	clk_rcg_set_rate_mnd(priv->base, SDCC2_CMD_RCGR, div, 0, 0,
			     CFG_CLK_SRC_GPLL0, 8);
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);
	clk_enable_cbc(priv->base + SDCC2_APPS_CBCR);

	return rate;
}

static int clk_init_uart(struct msm_clk_priv *priv)
{
	/* Enable AHB clock */
	clk_enable_vote_clk(priv->base, &gcc_blsp2_ahb_clk);

	/* 7372800 uart block clock @ GPLL0 */
	clk_rcg_set_rate_mnd(priv->base, BLSP2_UART2_APPS_CMD_RCGR, 1, 192, 15625,
			     CFG_CLK_SRC_GPLL0, 16);

	/* Vote for gpll0 clock */
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);

	/* Enable core clk */
	clk_enable_cbc(priv->base + BLSP2_UART2_APPS_CBCR);

	return 0;
}

static ulong apq8096_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_SDCC2_APPS_CLK: /* SDC2 */
		return clk_init_sdc(priv, rate);
		break;
	case GCC_BLSP2_UART2_APPS_CLK: /*UART2*/
		clk_init_uart(priv);
		return 7372800;
	default:
		return 0;
	}
}

static struct msm_clk_data apq8096_clk_data = {
	.set_rate = apq8096_clk_set_rate,
};

static const struct udevice_id gcc_apq8096_of_match[] = {
	{
		.compatible = "qcom,gcc-msm8996",
		.data = (ulong)&apq8096_clk_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_apq8096) = {
	.name		= "gcc_apq8096",
	.id		= UCLASS_NOP,
	.of_match	= gcc_apq8096_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
