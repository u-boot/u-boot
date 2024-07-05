// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm APQ8016
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Based on Little Kernel driver, simplified
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-msm8916.h>

#include "clock-qcom.h"

#define USB_HS_SYSTEM_CLK_CMD_RCGR	0x41010

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x2101C)
#define APCS_GPLL_ENA_VOTE		(0x45000)
#define APCS_CLOCK_BRANCH_ENA_VOTE (0x45004)

#define SDCC_BCR(n)			((n * 0x1000) + 0x41000)
#define SDCC_CMD_RCGR(n)		(((n + 1) * 0x1000) + 0x41004)
#define SDCC_APPS_CBCR(n)		((n * 0x1000) + 0x41018)
#define SDCC_AHB_CBCR(n)		((n * 0x1000) + 0x4101C)

/* BLSP1 AHB clock (root clock for BLSP) */
#define BLSP1_AHB_CBCR			0x1008

/* Uart clock control registers */
#define BLSP1_UART1_APPS_CBCR		(0x203C)
#define BLSP1_UART1_APPS_CMD_RCGR	(0x2044)
#define BLSP1_UART2_APPS_CBCR		(0x302C)
#define BLSP1_UART2_APPS_CMD_RCGR	(0x3034)

/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE BIT(17)

static struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(0),
};

static struct vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = BLSP1_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(10),
};

static const struct gate_clk apq8016_clks[] = {
	GATE_CLK(GCC_USB_HS_AHB_CLK,    0x41008, 0x00000001),
	GATE_CLK(GCC_USB_HS_SYSTEM_CLK,	0x41004, 0x00000001),
};

/* SDHCI */
static int apq8016_clk_init_sdc(struct msm_clk_priv *priv, int slot, uint rate)
{
	int div = 15; /* 100MHz default */

	if (rate == 200000000)
		div = 4;

	clk_enable_cbc(priv->base + SDCC_AHB_CBCR(slot));
	/* 800Mhz/div, gpll0 */
	clk_rcg_set_rate_mnd(priv->base, SDCC_CMD_RCGR(slot), div, 0, 0,
			     CFG_CLK_SRC_GPLL0, 8);
	clk_enable_gpll0(priv->base, &gpll0_vote_clk);
	clk_enable_cbc(priv->base + SDCC_APPS_CBCR(slot));

	return rate;
}

/* UART: 115200 */
int apq8016_clk_init_uart(phys_addr_t base, unsigned long id)
{
	u32 cmd_rcgr, apps_cbcr;

	switch (id) {
	case GCC_BLSP1_UART1_APPS_CLK:
		cmd_rcgr = BLSP1_UART1_APPS_CMD_RCGR;
		apps_cbcr = BLSP1_UART1_APPS_CBCR;
		break;
	case GCC_BLSP1_UART2_APPS_CLK:
		cmd_rcgr = BLSP1_UART2_APPS_CMD_RCGR;
		apps_cbcr = BLSP1_UART2_APPS_CBCR;
		break;
	default:
		return 0;
	}

	/* Enable AHB clock */
	clk_enable_vote_clk(base, &gcc_blsp1_ahb_clk);

	/* 7372800 uart block clock @ GPLL0 */
	clk_rcg_set_rate_mnd(base, cmd_rcgr, 1, 144, 15625, CFG_CLK_SRC_GPLL0,
			     16);

	/* Vote for gpll0 clock */
	clk_enable_gpll0(base, &gpll0_vote_clk);

	/* Enable core clk */
	clk_enable_cbc(base + apps_cbcr);

	return 0;
}

static ulong apq8016_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_SDCC1_APPS_CLK: /* SDC1 */
		return apq8016_clk_init_sdc(priv, 0, rate);
	case GCC_SDCC2_APPS_CLK: /* SDC2 */
		return apq8016_clk_init_sdc(priv, 1, rate);
	case GCC_BLSP1_UART1_APPS_CLK: /* UART1 */
	case GCC_BLSP1_UART2_APPS_CLK: /* UART2 */
		apq8016_clk_init_uart(priv->base, clk->id);
		return 7372800;
	case GCC_USB_HS_SYSTEM_CLK:
		if (rate != 80000000)
			log_warning("Unexpected rate %ld requested for USB_HS_SYSTEM_CLK\n",
				    rate);
		clk_rcg_set_rate_mnd(priv->base, USB_HS_SYSTEM_CLK_CMD_RCGR,
				     10, 0, 0, CFG_CLK_SRC_GPLL0, 0);
		return rate;
	default:
		return 0;
	}
}

static int apq8016_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		log_warning("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, apq8016_clks[clk->id].name);
	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static struct msm_clk_data apq8016_clk_data = {
	.set_rate = apq8016_clk_set_rate,
	.clks = apq8016_clks,
	.num_clks = ARRAY_SIZE(apq8016_clks),
	.enable = apq8016_clk_enable,
};

static const struct udevice_id gcc_apq8016_of_match[] = {
	{
		.compatible = "qcom,gcc-msm8916",
		.data = (ulong)&apq8016_clk_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_apq8016) = {
	.name		= "gcc_apq8016",
	.id		= UCLASS_NOP,
	.of_match	= gcc_apq8016_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
