// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm sm6115 (and sm4250/qrb4210)
 *
 * Copyright (c) 2024 Linaro Ltd.
 *
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <dt-bindings/clock/qcom,gcc-sm6115.h>

#include "clock-qcom.h"

#define QUPV3_WRAP0_S4_CMD_RCGR 0x1f608
#define SDCC1_APPS_CLK_CMD_RCGR 0x38028
#define SDCC2_APPS_CLK_CMD_RCGR 0x1e00c

static const struct freq_tbl ftbl_gcc_qupv3_wrap0_s0_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_AUX2, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_AUX2, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_AUX2, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 16, 75),
	F(75000000, CFG_CLK_SRC_GPLL0_AUX2, 4, 0, 0),
	F(80000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0_AUX2, 3, 0, 0),
	F(102400000, CFG_CLK_SRC_GPLL0_AUX2, 1, 128, 375),
	F(112000000, CFG_CLK_SRC_GPLL0_AUX2, 1, 28, 75),
	F(117964800, CFG_CLK_SRC_GPLL0_AUX2, 1, 6144, 15625),
	F(120000000, CFG_CLK_SRC_GPLL0_AUX2, 2.5, 0, 0),
	F(128000000, CFG_CLK_SRC_GPLL6, 3, 0, 0),
	{}
};

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(25000000, CFG_CLK_SRC_GPLL0_AUX2, 12, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_AUX2, 6, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0_AUX2, 3, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	{}
};

static const struct pll_vote_clk gpll0_clk = {
	.status = 0,
	.status_bit = BIT(31),
	.ena_vote = 0x79000,
	.vote_bit = BIT(0),
};

static const struct gate_clk sm6115_clks[] = {
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0x1a084, 0x00000001),
	GATE_CLK(GCC_QUPV3_WRAP0_CORE_2X_CLK, 0x7900c, 0x00000200),
	GATE_CLK(GCC_QUPV3_WRAP0_CORE_CLK, 0x7900c, 0x00000100),
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK, 0x7900c, 0x00000400),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK, 0x7900c, 0x00000800),
	GATE_CLK(GCC_QUPV3_WRAP0_S2_CLK, 0x7900c, 0x00001000),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK, 0x7900c, 0x00002000),
	GATE_CLK(GCC_QUPV3_WRAP0_S4_CLK, 0x7900c, 0x00004000),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK, 0x7900c, 0x00008000),
	GATE_CLK(GCC_QUPV3_WRAP_0_M_AHB_CLK, 0x7900c, 0x00000040),
	GATE_CLK(GCC_QUPV3_WRAP_0_S_AHB_CLK, 0x7900c, 0x00000080),
	GATE_CLK(GCC_SDCC1_AHB_CLK, 0x38008, 0x00000001),
	GATE_CLK(GCC_SDCC1_APPS_CLK, 0x38004, 0x00000001),
	GATE_CLK(GCC_SDCC1_ICE_CORE_CLK, 0x3800c, 0x00000001),
	GATE_CLK(GCC_SDCC2_AHB_CLK, 0x1e008, 0x00000001),
	GATE_CLK(GCC_SDCC2_APPS_CLK, 0x1e004, 0x00000001),
	GATE_CLK(GCC_SYS_NOC_CPUSS_AHB_CLK, 0x79004, 0x00000001),
	GATE_CLK(GCC_SYS_NOC_UFS_PHY_AXI_CLK, 0x45098, 0x00000001),
	GATE_CLK(GCC_SYS_NOC_USB3_PRIM_AXI_CLK, 0x1a080, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK, 0x45014, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK, 0x45010, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_CLK, 0x45044, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK, 0x45078, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK, 0x4501c, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK, 0x45018, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK, 0x45040, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0x1a010, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0x1a018, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0x1a014, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_CLKREF_CLK, 0x9f000, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0x1a054, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK, 0x1a058, 0x00000001),
	GATE_CLK(GCC_AHB2PHY_USB_CLK, 0x1d008, 0x00000001),
	GATE_CLK(GCC_UFS_CLKREF_CLK, 0x8c000, 0x00000001),
};

static ulong sm6115_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	debug("%s: clk %s rate %lu\n", __func__, sm6115_clks[clk->id].name,
	      rate);

	switch (clk->id) {
	case GCC_QUPV3_WRAP0_S4_CLK: /*UART2*/
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S4_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src,
				     16);
		return 0;
	case GCC_SDCC2_APPS_CLK:
		/* Enable GPLL7 so we can point SDCC2_APPS_CLK_SRC RCG at it */
		clk_enable_gpll0(priv->base, &gpll0_clk);
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		WARN(freq->src != CFG_CLK_SRC_GPLL0,
		     "SDCC2_APPS_CLK_SRC not set to GPLL0, requested rate %lu\n",
		     rate);
		clk_rcg_set_rate_mnd(priv->base, SDCC2_APPS_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src,
				     8);
		return freq->freq;
	case GCC_SDCC1_APPS_CLK:
		/* The firmware turns this on for us and always sets it to this rate */
		return 384000000;
	default:
		return rate;
	}
}

static int sm6115_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, sm6115_clks[clk->id].name);

	switch (clk->id) {
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_CLKREF_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sm6115_gcc_resets[] = {
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x1c000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x1c004 },
	[GCC_SDCC1_BCR] = { 0x38000 },
	[GCC_SDCC2_BCR] = { 0x1e000 },
	[GCC_UFS_PHY_BCR] = { 0x45000 },
	[GCC_USB30_PRIM_BCR] = { 0x1a000 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x1d000 },
	[GCC_USB3PHY_PHY_PRIM_SP0_BCR] = { 0x1b008 },
	[GCC_USB3_PHY_PRIM_SP0_BCR] = { 0x1b000 },
	[GCC_VCODEC0_BCR] = { 0x58094 },
	[GCC_VENUS_BCR] = { 0x58078 },
	[GCC_VIDEO_INTERFACE_BCR] = { 0x6e000 },
};

static const struct qcom_power_map sm6115_gdscs[] = {
	[GCC_USB30_PRIM_GDSC] = { 0x1a004 },
};

static const phys_addr_t sm6115_gpll_addrs[] = {
	0x01400000, // GCC_GPLL0_MODE
	0x01401000, // GCC_GPLL1_MODE
	0x01402000, // GCC_GPLL2_MODE
	0x01403000, // GCC_GPLL3_MODE
	0x01404000, // GCC_GPLL4_MODE
	0x01405000, // GCC_GPLL5_MODE
	0x01406000, // GCC_GPLL6_MODE
	0x01407000, // GCC_GPLL7_MODE
	0x01408000, // GCC_GPLL8_MODE
	0x01409000, // GCC_GPLL9_MODE
	0x0140a000, // GCC_GPLL10_MODE
	0x0140b000, // GCC_GPLL11_MODE
};

static const phys_addr_t sm6115_rcg_addrs[] = {
	0x0141a01c, // GCC_USB30_PRIM_MASTER_CMD_RCGR
	0x0141a034, // GCC_USB30_PRIM_MOCK_UTMI_CMD_RCGR
	0x0141a060, // GCC_USB3_PRIM_PHY_AUX_CMD_RCGR
	0x01438028, // GCC_SDCC1_APPS_CMD_RCGR
	0x0141e00c, // GCC_SDCC2_APPS_CMD_RCGR
	0x0141f018, // GCC_QUPV3_WRAP0_CORE_2X_CMD_RCGR
	0x0141f148, // GCC_QUPV3_WRAP0_S0_CMD_RCGR
	0x0141f278, // GCC_QUPV3_WRAP0_S1_CMD_RCGR
	0x0141f3a8, // GCC_QUPV3_WRAP0_S2_CMD_RCGR
	0x0141f4d8, // GCC_QUPV3_WRAP0_S3_CMD_RCGR
	0x0141f608, // GCC_QUPV3_WRAP0_S4_CMD_RCGR
	0x0141f738, // GCC_QUPV3_WRAP0_S5_CMD_RCGR
	0x01428014, // GCC_SLEEP_CMD_RCGR
	0x0142802c, // GCC_XO_CMD_RCGR
	0x01445020, // GCC_UFS_PHY_AXI_CMD_RCGR
	0x01445048, // GCC_UFS_PHY_ICE_CORE_CMD_RCGR
	0x01445060, // GCC_UFS_PHY_UNIPRO_CORE_CMD_RCGR
	0x0144507c, // GCC_UFS_PHY_PHY_AUX_CMD_RCGR
};

static const char *const sm6115_rcg_names[] = {
	"GCC_USB30_PRIM_MASTER_CMD_RCGR",
	"GCC_USB30_PRIM_MOCK_UTMI_CMD_RCGR",
	"GCC_USB3_PRIM_PHY_AUX_CMD_RCGR",
	"GCC_SDCC1_APPS_CMD_RCGR",
	"GCC_SDCC2_APPS_CMD_RCGR",
	"GCC_QUPV3_WRAP0_CORE_2X_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S0_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S1_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S2_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S3_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S4_CMD_RCGR",
	"GCC_QUPV3_WRAP0_S5_CMD_RCGR",
	"GCC_SLEEP_CMD_RCGR",
	"GCC_XO_CMD_RCGR",
	"GCC_UFS_PHY_AXI_CMD_RCGR",
	"GCC_UFS_PHY_ICE_CORE_CMD_RCGR",
	"GCC_UFS_PHY_UNIPRO_CORE_CMD_RCGR",
	"GCC_UFS_PHY_PHY_AUX_CMD_RCGR",
};

static struct msm_clk_data sm6115_gcc_data = {
	.resets = sm6115_gcc_resets,
	.num_resets = ARRAY_SIZE(sm6115_gcc_resets),
	.clks = sm6115_clks,
	.num_clks = ARRAY_SIZE(sm6115_clks),
	.power_domains = sm6115_gdscs,
	.num_power_domains = ARRAY_SIZE(sm6115_gdscs),

	.enable = sm6115_enable,
	.set_rate = sm6115_set_rate,

	.dbg_pll_addrs = sm6115_gpll_addrs,
	.num_plls = ARRAY_SIZE(sm6115_gpll_addrs),
	.dbg_rcg_addrs = sm6115_rcg_addrs,
	.num_rcgs = ARRAY_SIZE(sm6115_rcg_addrs),
	.dbg_rcg_names = sm6115_rcg_names,
};

static const struct udevice_id gcc_sm6115_of_match[] = {
	{
		.compatible = "qcom,gcc-sm6115",
		.data = (ulong)&sm6115_gcc_data,
	},
	{}
};

U_BOOT_DRIVER(gcc_sm6115) = {
	.name = "gcc_sm6115",
	.id = UCLASS_NOP,
	.of_match = gcc_sm6115_of_match,
	.bind = qcom_cc_bind,
	.flags = DM_FLAG_PRE_RELOC,
};
