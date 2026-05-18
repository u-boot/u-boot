// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm Milos
 *
 * (C) Copyright 2024 Linaro Ltd.
 * (C) Copyright 2026 Luca Weiss <luca.weiss@fairphone.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,milos-gcc.h>
#include <dt-bindings/clock/qcom,rpmh.h>

#include "clock-qcom.h"

/* On-board TCXO, TOFIX get from DT */
#define TCXO_RATE	76800000

/* bi_tcxo_div4 divided after RPMh output */
#define TCXO_DIV4_RATE	(TCXO_RATE / 4)

static const struct freq_tbl ftbl_gcc_qupv3_wrap0_s3_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(51200000, CFG_CLK_SRC_GPLL0_EVEN, 1, 64, 375),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(37500000, CFG_CLK_SRC_GPLL0_EVEN, 8, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_EVEN, 6, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	/* TOFIX F(202000000, CFG_CLK_SRC_GPLL9, 4, 0, 0), */
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static ulong milos_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP0_S5_CLK: /* UART5 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s3_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x18500,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_SDCC2_APPS_CLK:
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x14018,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x3902c,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, 0x39044, 0, 0);
		return TCXO_DIV4_RATE;
	default:
		return 0;
	}
}

static const struct gate_clk milos_clks[] = {
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK,		0x39090, BIT(0)),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK,		0x52008, BIT(27)),
	GATE_CLK(GCC_QUPV3_WRAP_0_M_AHB_CLK,		0x52008, BIT(20)),
	GATE_CLK(GCC_QUPV3_WRAP_0_S_AHB_CLK,		0x52008, BIT(21)),
	GATE_CLK(GCC_SDCC2_AHB_CLK,			0x14010, BIT(0)),
	GATE_CLK(GCC_SDCC2_APPS_CLK,			0x14004, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,		0x39018, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK,		0x39028, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK,		0x39024, BIT(0)),
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,		0x3908c, BIT(0)),
};

static int milos_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map milos_gcc_resets[] = {
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_SDCC1_BCR] = { 0xa3000 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
};

static const struct qcom_power_map milos_gdscs[] = {
	[UFS_PHY_GDSC] = { 0x77004 },
	[UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[USB30_PRIM_GDSC] = { 0x39004 },
};

static struct msm_clk_data milos_gcc_data = {
	.resets = milos_gcc_resets,
	.num_resets = ARRAY_SIZE(milos_gcc_resets),
	.clks = milos_clks,
	.num_clks = ARRAY_SIZE(milos_clks),
	.power_domains = milos_gdscs,
	.num_power_domains = ARRAY_SIZE(milos_gdscs),

	.enable = milos_enable,
	.set_rate = milos_set_rate,
};

static const struct udevice_id gcc_milos_of_match[] = {
	{
		.compatible = "qcom,milos-gcc",
		.data = (ulong)&milos_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_milos) = {
	.name		= "gcc_milos",
	.id		= UCLASS_NOP,
	.of_match	= gcc_milos_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

static ulong milos_rpmh_clk_set_rate(struct clk *clk, ulong rate)
{
	return (clk->rate = rate);
}

static ulong milos_rpmh_clk_get_rate(struct clk *clk)
{
	switch (clk->id) {
	case RPMH_CXO_CLK:
		return TCXO_DIV4_RATE;
	default:
		return clk->rate;
	}
}

static int milos_rpmh_clk_nop(struct clk *clk)
{
	return 0;
}

static struct clk_ops milos_rpmh_clk_ops = {
	.set_rate = milos_rpmh_clk_set_rate,
	.get_rate = milos_rpmh_clk_get_rate,
	.enable = milos_rpmh_clk_nop,
	.disable = milos_rpmh_clk_nop,
};

static const struct udevice_id milos_rpmh_clk_ids[] = {
	{ .compatible = "qcom,milos-rpmh-clk" },
	{ }
};

U_BOOT_DRIVER(milos_rpmh_clk) = {
	.name		= "milos_rpmh_clk",
	.id		= UCLASS_CLK,
	.of_match	= milos_rpmh_clk_ids,
	.ops		= &milos_rpmh_clk_ops,
	.flags		= DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
