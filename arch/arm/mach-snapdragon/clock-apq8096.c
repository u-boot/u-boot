// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8096
 *
 * (C) Copyright 2017 Jorge Ramirez Ortiz <jorge.ramirez-ortiz@linaro.org>
 *
 * Based on Little Kernel driver, simplified
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include "clock-snapdragon.h"

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE		BIT(30)
#define APCS_GPLL_ENA_VOTE_GPLL0	BIT(0)

static const struct bcr_regs sdc_regs = {
	.cfg_rcgr = SDCC2_CFG_RCGR,
	.cmd_rcgr = SDCC2_CMD_RCGR,
	.M = SDCC2_M,
	.N = SDCC2_N,
	.D = SDCC2_D,
};

static const struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = APCS_GPLL_ENA_VOTE_GPLL0,
};

static int clk_init_sdc(struct msm_clk_priv *priv, uint rate)
{
	int div = 3;

	clk_enable_cbc(priv->base + SDCC2_AHB_CBCR);
	clk_rcg_set_rate_mnd(priv->base, &sdc_regs, div, 0, 0,
			     CFG_CLK_SRC_GPLL0);
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);
	clk_enable_cbc(priv->base + SDCC2_APPS_CBCR);

	return rate;
}

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0: /* SDC1 */
		return clk_init_sdc(priv, rate);
		break;
	default:
		return 0;
	}
}
