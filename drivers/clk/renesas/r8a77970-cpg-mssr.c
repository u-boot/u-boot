/*
 * Renesas R8A77970 CPG MSSR driver
 *
 * Copyright (C) 2017-2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>

#include <dt-bindings/clock/r8a77970-cpg-mssr.h>

#include "renesas-cpg-mssr.h"

static const struct cpg_core_clk r8a77970_core_clks[] = {
	/* External Clock Inputs */
	DEF_INPUT("extal",  CLK_EXTAL),
	DEF_INPUT("extalr", CLK_EXTALR),

	/* Internal Core Clocks */
	DEF_BASE(".main",       CLK_MAIN, CLK_TYPE_GEN3_MAIN, CLK_EXTAL),
	DEF_BASE(".pll0",       CLK_PLL0, CLK_TYPE_GEN3_PLL0, CLK_MAIN),
	DEF_BASE(".pll1",       CLK_PLL1, CLK_TYPE_GEN3_PLL1, CLK_MAIN),
	DEF_BASE(".pll3",       CLK_PLL3, CLK_TYPE_GEN3_PLL3, CLK_MAIN),

	DEF_FIXED(".pll1_div2", CLK_PLL1_DIV2,     CLK_PLL1,       2, 1),
	DEF_FIXED(".pll1_div4", CLK_PLL1_DIV4,     CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED(".s1",        CLK_S1,            CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED(".s2",        CLK_S2,            CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED(".rpcsrc",    CLK_RPCSRC,        CLK_PLL1,       2, 1),

	/* Core Clock Outputs */
	DEF_BASE("z2",          R8A77970_CLK_Z2,    CLK_TYPE_GEN3_Z2, CLK_PLL1_DIV4),
	DEF_FIXED("ztr",        R8A77970_CLK_ZTR,   CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED("ztrd2",      R8A77970_CLK_ZTRD2, CLK_PLL1_DIV2, 12, 1),
	DEF_FIXED("zt",         R8A77970_CLK_ZT,    CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED("zx",         R8A77970_CLK_ZX,    CLK_PLL1_DIV2,  3, 1),
	DEF_FIXED("s1d1",       R8A77970_CLK_S1D1,  CLK_S1,         1, 1),
	DEF_FIXED("s1d2",       R8A77970_CLK_S1D2,  CLK_S1,         2, 1),
	DEF_FIXED("s1d4",       R8A77970_CLK_S1D4,  CLK_S1,         4, 1),
	DEF_FIXED("s2d1",       R8A77970_CLK_S2D1,  CLK_S2,         1, 1),
	DEF_FIXED("s2d2",       R8A77970_CLK_S2D2,  CLK_S2,         2, 1),
	DEF_FIXED("s2d4",       R8A77970_CLK_S2D4,  CLK_S2,         4, 1),

	DEF_GEN3_SD("sd0",      R8A77970_CLK_SD0,   CLK_PLL1_DIV4, 0x0074),

	DEF_GEN3_RPC("rpc",     R8A77970_CLK_RPC,   CLK_RPCSRC,    0x238),

	DEF_FIXED("cl",         R8A77970_CLK_CL,    CLK_PLL1_DIV2, 48, 1),
	DEF_FIXED("cp",         R8A77970_CLK_CP,    CLK_EXTAL,      2, 1),

	/* NOTE: HDMI, CSI, CAN etc. clock are missing */

	DEF_BASE("r",           R8A77970_CLK_R, CLK_TYPE_GEN3_R, CLK_RINT),
};

static const struct mssr_mod_clk r8a77970_mod_clks[] = {
	DEF_MOD("ivcp1e",		 127,	R8A77970_CLK_S2D1),
	DEF_MOD("scif4",		 203,	R8A77970_CLK_S2D4),	/* @@ H3=S3D4 */
	DEF_MOD("scif3",		 204,	R8A77970_CLK_S2D4),	/* @@ H3=S3D4 */
	DEF_MOD("scif1",		 206,	R8A77970_CLK_S2D4),	/* @@ H3=S3D4 */
	DEF_MOD("scif0",		 207,	R8A77970_CLK_S2D4),	/* @@ H3=S3D4 */
	DEF_MOD("msiof3",		 208,	R8A77970_CLK_MSO),
	DEF_MOD("msiof2",		 209,	R8A77970_CLK_MSO),
	DEF_MOD("msiof1",		 210,	R8A77970_CLK_MSO),
	DEF_MOD("msiof0",		 211,	R8A77970_CLK_MSO),
	DEF_MOD("mfis",			 213,	R8A77970_CLK_S2D2),	/* @@ H3=S3D2 */
	DEF_MOD("sys-dmac2",	 217,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("sys-dmac1",	 218,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("sdif",			 314,	R8A77970_CLK_SD0),
	DEF_MOD("rwdt0",		 402,	R8A77970_CLK_R),
	DEF_MOD("intc-ex",		 407,	R8A77970_CLK_CP),
	DEF_MOD("intc-ap",		 408,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("hscif3",		 517,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("hscif2",		 518,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("hscif1",		 519,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("hscif0",		 520,	R8A77970_CLK_S2D1),	/* @@ H3=S3D1 */
	DEF_MOD("thermal",		 522,	R8A77970_CLK_CP),
	DEF_MOD("pwm",			 523,	R8A77970_CLK_S2D4),
	DEF_MOD("fcpvd0",		 603,	R8A77970_CLK_S2D1),
	DEF_MOD("vspd0",		 623,	R8A77970_CLK_S2D1),
	DEF_MOD("csi40",		 716,	R8A77970_CLK_CSI0),
	DEF_MOD("du0",			 724,	R8A77970_CLK_S2D1),
	DEF_MOD("lvds",			 727,	R8A77970_CLK_S2D1),
	DEF_MOD("vin3",			 808,	R8A77970_CLK_S2D1),
	DEF_MOD("vin2",			 809,	R8A77970_CLK_S2D1),
	DEF_MOD("vin1",			 810,	R8A77970_CLK_S2D1),
	DEF_MOD("vin0",			 811,	R8A77970_CLK_S2D1),
	DEF_MOD("etheravb",		 812,	R8A77970_CLK_S2D2),
	DEF_MOD("isp",			 817,	R8A77970_CLK_S2D1),
	DEF_MOD("gpio5",		 907,	R8A77970_CLK_CP),
	DEF_MOD("gpio4",		 908,	R8A77970_CLK_CP),
	DEF_MOD("gpio3",		 909,	R8A77970_CLK_CP),
	DEF_MOD("gpio2",		 910,	R8A77970_CLK_CP),
	DEF_MOD("gpio1",		 911,	R8A77970_CLK_CP),
	DEF_MOD("gpio0",		 912,	R8A77970_CLK_CP),
	DEF_MOD("can-fd",		 914,	R8A77970_CLK_S2D2),
	DEF_MOD("rpc",			 917,	R8A77970_CLK_RPC),
	DEF_MOD("i2c4",			 927,	R8A77970_CLK_S2D2),
	DEF_MOD("i2c3",			 928,	R8A77970_CLK_S2D2),
	DEF_MOD("i2c2",			 929,	R8A77970_CLK_S2D2),
	DEF_MOD("i2c1",			 930,	R8A77970_CLK_S2D2),
	DEF_MOD("i2c0",			 931,	R8A77970_CLK_S2D2),
};

static const struct mstp_stop_table r8a77970_mstp_table[] = {
	{ 0x00230000, 0x0 },	{ 0xFFFFFFFF, 0x0 },
	{ 0x14062FD8, 0x2040 },	{ 0xFFFFFFDF, 0x400 },
	{ 0x80000184, 0x180 },	{ 0x83FFFFFF, 0x0 },
	{ 0xFFFFFFFF, 0x0 },	{ 0xFFFFFFFF, 0x0 },
	{ 0x7FF3FFF4, 0x0 },	{ 0xFBF7FF97, 0x0 },
	{ 0xFFFEFFE0, 0x0 },	{ 0x000000B7, 0x0 },
};

static const struct cpg_mssr_info r8a77970_cpg_mssr_info = {
	.core_clk		= r8a77970_core_clks,
	.core_clk_size		= ARRAY_SIZE(r8a77970_core_clks),
	.mod_clk		= r8a77970_mod_clks,
	.mod_clk_size		= ARRAY_SIZE(r8a77970_mod_clks),
	.mstp_table		= r8a77970_mstp_table,
	.mstp_table_size	= ARRAY_SIZE(r8a77970_mstp_table),
	.reset_node		= "renesas,r8a77970-rst",
	.extalr_node		= "extalr",
};

static const struct udevice_id r8a77970_clk_ids[] = {
	{
		.compatible	= "renesas,r8a77970-cpg-mssr",
		.data		= (ulong)&r8a77970_cpg_mssr_info
	},
	{ }
};

U_BOOT_DRIVER(clk_r8a77970) = {
	.name		= "clk_r8a77970",
	.id		= UCLASS_CLK,
	.of_match	= r8a77970_clk_ids,
	.priv_auto_alloc_size = sizeof(struct gen3_clk_priv),
	.ops		= &gen3_clk_ops,
	.probe		= gen3_clk_probe,
	.remove		= gen3_clk_remove,
};
