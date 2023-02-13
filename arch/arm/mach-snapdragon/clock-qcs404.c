// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm QCS404
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include "clock-snapdragon.h"

#include <dt-bindings/clock/qcom,gcc-qcs404.h>

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE BIT(31)

#define CFG_CLK_SRC_GPLL1	BIT(8)
#define GPLL1_STATUS_ACTIVE	BIT(31)

static struct vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = BLSP1_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(10) | BIT(5) | BIT(4),
};

static const struct bcr_regs uart2_regs = {
	.cfg_rcgr = BLSP1_UART2_APPS_CFG_RCGR,
	.cmd_rcgr = BLSP1_UART2_APPS_CMD_RCGR,
	.M = BLSP1_UART2_APPS_M,
	.N = BLSP1_UART2_APPS_N,
	.D = BLSP1_UART2_APPS_D,
};

static const struct bcr_regs sdc_regs = {
	.cfg_rcgr = SDCC_CFG_RCGR(1),
	.cmd_rcgr = SDCC_CMD_RCGR(1),
	.M = SDCC_M(1),
	.N = SDCC_N(1),
	.D = SDCC_D(1),
};

static struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(0),
};

static struct pll_vote_clk gpll1_vote_clk = {
	.status = GPLL1_STATUS,
	.status_bit = GPLL1_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(1),
};

static const struct bcr_regs usb30_master_regs = {
	.cfg_rcgr = USB30_MASTER_CFG_RCGR,
	.cmd_rcgr = USB30_MASTER_CMD_RCGR,
	.M = USB30_MASTER_M,
	.N = USB30_MASTER_N,
	.D = USB30_MASTER_D,
};

static const struct bcr_regs emac_regs = {
	.cfg_rcgr = EMAC_CFG_RCGR,
	.cmd_rcgr = EMAC_CMD_RCGR,
	.M = EMAC_M,
	.N = EMAC_N,
	.D = EMAC_D,
};

static const struct bcr_regs emac_ptp_regs = {
	.cfg_rcgr = EMAC_PTP_CFG_RCGR,
	.cmd_rcgr = EMAC_PTP_CMD_RCGR,
	.M = EMAC_M,
	.N = EMAC_N,
	.D = EMAC_D,
};

static const struct bcr_regs blsp1_qup0_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP0_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP0_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup1_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP1_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP1_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup2_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP2_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP2_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup3_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP3_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP3_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup4_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP4_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP4_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_BLSP1_UART2_APPS_CLK:
		/* UART: 115200 */
		clk_rcg_set_rate_mnd(priv->base, &uart2_regs, 0, 12, 125,
				     CFG_CLK_SRC_CXO);
		clk_enable_cbc(priv->base + BLSP1_UART2_APPS_CBCR);
		break;
	case GCC_BLSP1_AHB_CLK:
		clk_enable_vote_clk(priv->base, &gcc_blsp1_ahb_clk);
		break;
	case GCC_SDCC1_APPS_CLK:
		/* SDCC1: 200MHz */
		clk_rcg_set_rate_mnd(priv->base, &sdc_regs, 4, 0, 0,
				     CFG_CLK_SRC_GPLL0);
		clk_enable_gpll0(priv->base, &gpll0_vote_clk);
		clk_enable_cbc(priv->base + SDCC_APPS_CBCR(1));
		break;
	case GCC_SDCC1_AHB_CLK:
		clk_enable_cbc(priv->base + SDCC_AHB_CBCR(1));
		break;
	case GCC_ETH_RGMII_CLK:
		if (rate == 250000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 2, 0, 0,
					     CFG_CLK_SRC_GPLL1);
		else if (rate == 125000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 4, 0, 0,
					     CFG_CLK_SRC_GPLL1);
		else if (rate == 50000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 10, 0, 0,
					     CFG_CLK_SRC_GPLL1);
		else if (rate == 5000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 2, 1, 50,
					     CFG_CLK_SRC_GPLL1);
		break;
	default:
		return 0;
	}

	return 0;
}

int msm_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_USB30_MASTER_CLK:
		clk_enable_cbc(priv->base + USB30_MASTER_CBCR);
		clk_rcg_set_rate_mnd(priv->base, &usb30_master_regs, 4, 0, 0,
				     CFG_CLK_SRC_GPLL0);
		break;
	case GCC_SYS_NOC_USB3_CLK:
		clk_enable_cbc(priv->base + SYS_NOC_USB3_CBCR);
		break;
	case GCC_USB30_SLEEP_CLK:
		clk_enable_cbc(priv->base + USB30_SLEEP_CBCR);
		break;
	case GCC_USB30_MOCK_UTMI_CLK:
		clk_enable_cbc(priv->base + USB30_MOCK_UTMI_CBCR);
		break;
	case GCC_USB_HS_PHY_CFG_AHB_CLK:
		clk_enable_cbc(priv->base + USB_HS_PHY_CFG_AHB_CBCR);
		break;
	case GCC_USB2A_PHY_SLEEP_CLK:
		clk_enable_cbc(priv->base + USB_HS_PHY_CFG_AHB_CBCR);
		break;
	case GCC_ETH_PTP_CLK:
		/* SPEED_1000: freq -> 250MHz */
		clk_enable_cbc(priv->base + ETH_PTP_CBCR);
		clk_enable_gpll0(priv->base, &gpll1_vote_clk);
		clk_rcg_set_rate_mnd(priv->base, &emac_ptp_regs, 2, 0, 0,
				     CFG_CLK_SRC_GPLL1);
		break;
	case GCC_ETH_RGMII_CLK:
		/* SPEED_1000: freq -> 250MHz */
		clk_enable_cbc(priv->base + ETH_RGMII_CBCR);
		clk_enable_gpll0(priv->base, &gpll1_vote_clk);
		clk_rcg_set_rate_mnd(priv->base, &emac_regs, 2, 0, 0,
				     CFG_CLK_SRC_GPLL1);
		break;
	case GCC_ETH_SLAVE_AHB_CLK:
		clk_enable_cbc(priv->base + ETH_SLAVE_AHB_CBCR);
		break;
	case GCC_ETH_AXI_CLK:
		clk_enable_cbc(priv->base + ETH_AXI_CBCR);
		break;
	case GCC_BLSP1_AHB_CLK:
		clk_enable_vote_clk(priv->base, &gcc_blsp1_ahb_clk);
		break;
	case GCC_BLSP1_QUP0_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP0_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup0_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP1_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP1_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup1_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP2_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP2_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup2_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP3_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP3_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup3_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP4_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP4_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup4_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	default:
		return 0;
	}

	return 0;
}
