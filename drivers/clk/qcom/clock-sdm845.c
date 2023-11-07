// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm SDM845
 *
 * (C) Copyright 2017 Jorge Ramirez Ortiz <jorge.ramirez-ortiz@linaro.org>
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 *
 * Based on Little Kernel driver, simplified
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-sdm845.h>

#include "clock-qcom.h"

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

struct freq_tbl {
	uint freq;
	uint src;
	u8 pre_div;
	u16 m;
	u16 n;
};

static const struct freq_tbl ftbl_gcc_qupv3_wrap0_s0_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	F(102400000, CFG_CLK_SRC_GPLL0_EVEN, 1, 128, 375),
	F(112000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 28, 75),
	F(117964800, CFG_CLK_SRC_GPLL0_EVEN, 1, 6144, 15625),
	F(120000000, CFG_CLK_SRC_GPLL0_EVEN, 2.5, 0, 0),
	F(128000000, CFG_CLK_SRC_GPLL0, 1, 16, 75),
	{ }
};

static const struct bcr_regs uart2_regs = {
	.cfg_rcgr = SE9_UART_APPS_CFG_RCGR,
	.cmd_rcgr = SE9_UART_APPS_CMD_RCGR,
	.M = SE9_UART_APPS_M,
	.N = SE9_UART_APPS_N,
	.D = SE9_UART_APPS_D,
};

const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, uint rate)
{
	if (!f)
		return NULL;

	if (!f->freq)
		return f;

	for (; f->freq; f++)
		if (rate <= f->freq)
			return f;

	/* Default to our fastest rate */
	return f - 1;
}

static int clk_init_uart(struct msm_clk_priv *priv, uint rate)
{
	const struct freq_tbl *freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s0_clk_src, rate);

	clk_rcg_set_rate_mnd(priv->base, &uart2_regs,
						freq->pre_div, freq->m, freq->n, freq->src);

	return 0;
}

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_QUPV3_WRAP1_S1_CLK: /*UART2*/
		return clk_init_uart(priv, rate);
	default:
		return 0;
	}
}

int msm_enable(struct clk *clk)
{
	return 0;
}

static const struct qcom_reset_map sdm845_gcc_resets[] = {
	[GCC_QUPV3_WRAPPER_0_BCR] = { 0x17000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x18000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_UFS_CARD_BCR] = { 0x75000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0xf000 },
	[GCC_USB30_SEC_BCR] = { 0x10000 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x5000c },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x50010 },
	[GCC_USB3_DP_PHY_SEC_BCR] = { 0x50014 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x6a000 },
};

static const struct msm_clk_data qcs404_gcc_data = {
	.resets = sdm845_gcc_resets,
	.num_resets = ARRAY_SIZE(sdm845_gcc_resets),
};

static const struct udevice_id gcc_sdm845_of_match[] = {
	{
		.compatible = "qcom,gcc-sdm845",
		.data = (ulong)&qcs404_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_sdm845) = {
	.name		= "gcc_sdm845",
	.id		= UCLASS_NOP,
	.of_match	= gcc_sdm845_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
