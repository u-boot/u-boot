// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm sm6350
 *
 * (C) Copyright 2024 Linaro Ltd.
 * (C) Copyright 2025 Luca Weiss <luca.weiss@fairphone.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-sm6350.h>

#include "clock-qcom.h"

#undef CFG_CLK_SRC_GPLL0_ODD
#define CFG_CLK_SRC_GPLL0_ODD (2 << 8)
#define CFG_CLK_SRC_GPLL6_EVEN (2 << 8)

#define GCC_SE12_UART_RCG_REG 0x223a8
#define GCC_SDCC2_APPS_CLK_SRC_REG 0x2000c

#define APCS_GPLL7_STATUS 0x7000
#define APCS_GPLLX_ENA_REG 0x52010

static const struct freq_tbl ftbl_gcc_qupv3_wrap1_s3_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	F(102400000, CFG_CLK_SRC_GPLL0_EVEN, 1, 128, 375),
	F(112000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 28, 75),
	F(117964800, CFG_CLK_SRC_GPLL0_EVEN, 1, 6144, 15625),
	F(120000000, CFG_CLK_SRC_GPLL0_EVEN, 2.5, 0, 0),
	F(128000000, CFG_CLK_SRC_GPLL6_EVEN, 3, 0, 0),
	{}
};

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(9600000, CFG_CLK_SRC_CXO, 2, 0, 0),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(25000000, CFG_CLK_SRC_GPLL0_ODD, 8, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_ODD, 4, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0_ODD, 2, 0, 0),
	F(202000000, CFG_CLK_SRC_GPLL7, 4, 0, 0),
	{}
};

static struct pll_vote_clk gpll7_vote_clk = {
	.status = APCS_GPLL7_STATUS,
	.status_bit = BIT(31),
	.ena_vote = APCS_GPLLX_ENA_REG,
	.vote_bit = BIT(7),
};

static ulong sm6350_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n", __func__,
		      priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_QUPV3_WRAP1_S3_CLK: /*UART9*/
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap1_s3_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_SE12_UART_RCG_REG,
				     freq->pre_div, freq->m, freq->n, freq->src,
				     16);

		return freq->freq;
	case GCC_SDCC2_APPS_CLK:
		/* Enable GPLL7 so that we can point SDCC2_APPS_CLK_SRC at it */
		clk_enable_gpll0(priv->base, &gpll7_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		WARN(freq->src != CFG_CLK_SRC_GPLL7,
		     "SDCC2_APPS_CLK_SRC not set to GPLL7, requested rate %lu\n",
		     rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_SDCC2_APPS_CLK_SRC_REG,
				     freq->pre_div, freq->m, freq->n,
				     CFG_CLK_SRC_GPLL7, 8);

		return rate;
	default:
		return 0;
	}
}

static const struct gate_clk sm6350_clks[] = {
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK, 0x3e014, 0x00000001),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0x3e010, 0x00000001),
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0x1101c, 0x00000001),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK, 0x52000, 0x00800000),
	GATE_CLK(GCC_QUPV3_WRAP_1_M_AHB_CLK, 0x52000, 0x00040000),
	GATE_CLK(GCC_QUPV3_WRAP_1_S_AHB_CLK, 0x52000, 0x00080000),
	GATE_CLK(GCC_SDCC2_AHB_CLK, 0x20008, 0x00000001),
	GATE_CLK(GCC_SDCC2_APPS_CLK, 0x20004, 0x00000001),
	GATE_CLK(GCC_UFS_MEM_CLKREF_CLK, 0x8c000, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK, 0x3a00c, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK, 0x3a034, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_CLK, 0x3a0a4, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK, 0x3a0ac, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK, 0x3a014, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK, 0x3a018, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK, 0x3a010, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK, 0x3a09c, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0x1a00c, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0x1a018, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0x1a014, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_CLKREF_CLK, 0x8c010, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK, 0x1a050, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0x1a054, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK, 0x1a058, 0x00000001),
};

static int sm6350_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, sm6350_clks[clk->id].name);

	switch (clk->id) {
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sm6350_gcc_resets[] = {
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x1d000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x1e000 },
	[GCC_SDCC1_BCR] = { 0x4b000 },
	[GCC_SDCC2_BCR] = { 0x20000 },
	[GCC_UFS_PHY_BCR] = { 0x3a000 },
	[GCC_USB30_PRIM_BCR] = { 0x1a000 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x1c000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x1c008 },
};

static const struct qcom_power_map sm6350_gdscs[] = {
	[USB30_PRIM_GDSC] = { 0x1a004 },
	[UFS_PHY_GDSC] = { 0x3a004 },
	[HLOS1_VOTE_MMNOC_MMU_TBU_HF0_GDSC] = { 0xb7040 },
	[HLOS1_VOTE_MMNOC_MMU_TBU_HF1_GDSC] = { 0xb7044 },
};

static struct msm_clk_data sm6350_gcc_data = {
	.resets = sm6350_gcc_resets,
	.num_resets = ARRAY_SIZE(sm6350_gcc_resets),
	.clks = sm6350_clks,
	.num_clks = ARRAY_SIZE(sm6350_clks),
	.power_domains = sm6350_gdscs,
	.num_power_domains = ARRAY_SIZE(sm6350_gdscs),

	.enable = sm6350_enable,
	.set_rate = sm6350_set_rate,
};

static const struct udevice_id gcc_sm6350_of_match[] = {
	{
		.compatible = "qcom,gcc-sm6350",
		.data = (ulong)&sm6350_gcc_data,
	},
	{}
};

U_BOOT_DRIVER(gcc_sm6350) = {
	.name = "gcc_sm6350",
	.id = UCLASS_NOP,
	.of_match = gcc_sm6350_of_match,
	.bind = qcom_cc_bind,
	.flags = DM_FLAG_PRE_RELOC,
};
