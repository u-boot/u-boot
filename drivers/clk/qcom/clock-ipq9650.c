// SPDX-License-Identifier: GPL-2.0-only
/*
 * Clock driver for Qualcomm IPQ9650
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <dt-bindings/clock/qcom,ipq9650-gcc.h>
#include <dt-bindings/reset/qcom,ipq9650-gcc.h>

#include <clk.h>
#include <log.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/read.h>
#include <linux/bitops.h>

#include "clock-qcom.h"

#define GCC_IM_SLEEP_CBCR	0x34020

#define P_XO			CFG_CLK_SRC_CXO
#define P_GPLL0			CFG_CLK_SRC_GPLL0
#define P_GPLL2			CFG_CLK_SRC_GPLL2
#define P_GPLL0_OUT_MAIN_DIV	(4 << 8)

static const struct freq_tbl ftbl_qupv3_wrap_se[] = {
	F(960000,    P_XO,                 10, 2, 5),
	F(3686636,   P_GPLL0_OUT_MAIN_DIV,  1, 2, 217),
	F(4800000,   P_XO,                  5, 0, 0),
	F(7373272,   P_GPLL0_OUT_MAIN_DIV,  1, 4, 217),
	F(9600000,   P_XO,                  2.5, 0, 0),
	F(14746544,  P_GPLL0_OUT_MAIN_DIV,  1, 8, 217),
	F(16000000,  P_GPLL0,              10, 1, 5),
	F(24000000,  P_XO,                  1, 0, 0),
	F(25000000,  P_GPLL0,              16, 1, 2),
	F(32000000,  P_GPLL0,               1, 1, 25),
	F(40000000,  P_GPLL0,               1, 1, 20),
	F(46400000,  P_GPLL0,               2, 29, 250),
	F(48000000,  P_GPLL0,               1, 3, 50),
	F(50000000,  P_GPLL0,              16, 0, 0),
	F(51200000,  P_GPLL0,               1, 8, 125),
	F(56000000,  P_GPLL0,               1, 7, 100),
	F(58986175,  P_GPLL0,               1, 16, 217),
	F(60000000,  P_GPLL0,               1, 3, 40),
	F(64000000,  P_GPLL0,              12.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_sdcc1_apps[] = {
	F(144000,    P_XO,    16,  12, 125),
	F(400000,    P_XO,    12,  1, 5),
	F(24000000,  P_GPLL2, 12,  1, 2),
	F(48000000,  P_GPLL2, 12,  0, 0),
	F(96000000,  P_GPLL2, 6,   0, 0),
	F(177777778, P_GPLL0, 4.5, 0, 0),
	F(192000000, P_GPLL2, 3,   0, 0),
	F(200000000, P_GPLL0, 4,   0, 0),
	{ }
};

static ulong ipq9650_rcg_set_rate_mnd(const struct msm_clk_priv *priv, u32 cmd_rcgr,
				      const struct freq_tbl *ftbl, ulong rate,
				      u8 mnd_width)
{
	const struct freq_tbl *freq = qcom_find_freq(ftbl, rate);

	clk_rcg_set_rate_mnd(priv->base, cmd_rcgr, freq->pre_div, freq->m,
			     freq->n, freq->src, mnd_width);

	return freq->freq;
}

static ulong ipq9650_set_rate(struct clk *clk, ulong rate)
{
	const struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_QUPV3_WRAP_SE6_CLK:
		return ipq9650_rcg_set_rate_mnd(priv, 0x04004, ftbl_qupv3_wrap_se,
					       rate, 8);
	case GCC_SDCC1_APPS_CLK:
		return ipq9650_rcg_set_rate_mnd(priv, 0x33004, ftbl_sdcc1_apps,
					       rate, 8);
	default:
		return -EINVAL;
	}
}

static int ipq_gate_clk_enable(struct clk *clk)
{
	const struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= priv->data->num_clks || !priv->data->clks[clk->id].reg) {
		log_debug("gcc: clock %lu not modelled, left as is\n", clk->id);
		return 0;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct gate_clk ipq9650_clks[] = {
	GATE_CLK_POLLED(GCC_SDCC1_AHB_CLK, 0x3303c, BIT(0), 0x3303c),
	GATE_CLK_POLLED(GCC_SDCC1_APPS_CLK, 0x3302c, BIT(0), 0x33004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_SE6_CLK, 0x04018, BIT(0), 0x04004),
};

static const struct qcom_reset_map ipq9650_gcc_resets[] = {
	[GCC_SDCC_BCR] = { 0x33000 },
};

static struct msm_clk_data ipq9650_gcc_data = {
	.resets		= ipq9650_gcc_resets,
	.num_resets	= ARRAY_SIZE(ipq9650_gcc_resets),
	.clks		= ipq9650_clks,
	.num_clks	= ARRAY_SIZE(ipq9650_clks),
	.enable		= ipq_gate_clk_enable,
	.set_rate	= ipq9650_set_rate,
};

static int ipq_gcc_enable_sleep_clk(struct udevice *dev, u32 cbcr)
{
	phys_addr_t base = dev_read_addr(dev);

	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	setbits_le32(base + cbcr, BIT(0));

	return 0;
}

static int ipq9650_gcc_probe(struct udevice *dev)
{
	return ipq_gcc_enable_sleep_clk(dev, GCC_IM_SLEEP_CBCR);
}

static const struct udevice_id gcc_ipq9650_of_match[] = {
	{ .compatible = "qcom,ipq9650-gcc", .data = (ulong)&ipq9650_gcc_data },
	{ }
};

U_BOOT_DRIVER(gcc_ipq9650) = {
	.name		= "gcc_ipq9650",
	.id		= UCLASS_NOP,
	.of_match	= gcc_ipq9650_of_match,
	.probe		= ipq9650_gcc_probe,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
