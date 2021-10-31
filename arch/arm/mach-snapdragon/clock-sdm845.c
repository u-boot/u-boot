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
#include "clock-snapdragon.h"

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
	case 0x58: /*UART2*/
		return clk_init_uart(priv, rate);
	default:
		return 0;
	}
}
