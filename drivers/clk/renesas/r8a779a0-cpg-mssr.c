// SPDX-License-Identifier: GPL-2.0
/*
 * r8a779a0 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2020 Renesas Electronics Corp.
 *
 * Based on r8a7795-cpg-mssr.c
 *
 * Copyright (C) 2015 Glider bvba
 * Copyright (C) 2015 Renesas Electronics Corp.
 */

#include <clk-uclass.h>
#include <dm.h>

#include <dt-bindings/clock/r8a779a0-cpg-mssr.h>

#include "renesas-cpg-mssr.h"
#include "rcar-gen3-cpg.h"

enum clk_ids {
	/* Core Clock Outputs exported to DT */
	LAST_DT_CORE_CLK = R8A779A0_CLK_OSC,

	/* External Input Clocks */
	CLK_EXTAL,
	CLK_EXTALR,

	/* Internal Core Clocks */
	CLK_MAIN,
	CLK_PLL1,
	CLK_PLL20,
	CLK_PLL21,
	CLK_PLL30,
	CLK_PLL31,
	CLK_PLL5,
	CLK_PLL1_DIV2,
	CLK_PLL20_DIV2,
	CLK_PLL21_DIV2,
	CLK_PLL30_DIV2,
	CLK_PLL31_DIV2,
	CLK_PLL5_DIV2,
	CLK_PLL5_DIV4,
	CLK_S1,
	CLK_S3,
	CLK_SDSRC,
	CLK_RPCSRC,
	CLK_OCO,

	/* Module Clocks */
	MOD_CLK_BASE
};

#define DEF_PLL(_name, _id, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN4_PLL2X_3X, CLK_MAIN, \
		 .offset = _offset)

#define CPG_PLL20CR	0x0834	/* PLL20 Control Register */
#define CPG_PLL21CR	0x0838	/* PLL21 Control Register */
#define CPG_PLL30CR	0x083c	/* PLL30 Control Register */
#define CPG_PLL31CR	0x0840	/* PLL31 Control Register */

#define CPG_SD0CKCR	0x870	/* SD-IF0 Clock Frequency Control Register */
#define CPG_CANFDCKCR	0x878	/* CAN-FD Clock Frequency Control Register */
#define CPG_MSOCKCR	0x87c	/* MSIOF Clock Frequency Control Register */
#define CPG_CSICKCR	0x880	/* CSI Clock Frequency Control Register */
#define CPG_DSIEXTCKCR	0x884	/* DSI Clock Frequency Control Register */

static const struct cpg_core_clk r8a779a0_core_clks[] __initconst = {
	/* External Clock Inputs */
	DEF_INPUT("extal",  CLK_EXTAL),
	DEF_INPUT("extalr", CLK_EXTALR),

	/* Internal Core Clocks */
	DEF_BASE(".main", CLK_MAIN,	CLK_TYPE_GEN4_MAIN, CLK_EXTAL),
	DEF_BASE(".pll1", CLK_PLL1,	CLK_TYPE_GEN4_PLL1, CLK_MAIN),
	DEF_BASE(".pll5", CLK_PLL5,	CLK_TYPE_GEN4_PLL5, CLK_MAIN),
	DEF_PLL(".pll20", CLK_PLL20,	CPG_PLL20CR),
	DEF_PLL(".pll21", CLK_PLL21,	CPG_PLL21CR),
	DEF_PLL(".pll30", CLK_PLL30,	CPG_PLL30CR),
	DEF_PLL(".pll31", CLK_PLL31,	CPG_PLL31CR),

	DEF_FIXED(".pll1_div2",		CLK_PLL1_DIV2,	CLK_PLL1,	2, 1),
	DEF_FIXED(".pll20_div2",	CLK_PLL20_DIV2,	CLK_PLL20,	2, 1),
	DEF_FIXED(".pll21_div2",	CLK_PLL21_DIV2,	CLK_PLL21,	2, 1),
	DEF_FIXED(".pll30_div2",	CLK_PLL30_DIV2,	CLK_PLL30,	2, 1),
	DEF_FIXED(".pll31_div2",	CLK_PLL31_DIV2,	CLK_PLL31,	2, 1),
	DEF_FIXED(".pll5_div2",		CLK_PLL5_DIV2,	CLK_PLL5,	2, 1),
	DEF_FIXED(".pll5_div4",		CLK_PLL5_DIV4,	CLK_PLL5_DIV2,	2, 1),
	DEF_FIXED(".s1",		CLK_S1,		CLK_PLL1_DIV2,	2, 1),
	DEF_FIXED(".s3",		CLK_S3,		CLK_PLL1_DIV2,	4, 1),
	DEF_FIXED(".sdsrc",		CLK_SDSRC,	CLK_PLL5_DIV4,	1, 1),

	DEF_RATE(".oco",		CLK_OCO,	32768),

	DEF_BASE(".rpcsrc",		CLK_RPCSRC,	CLK_TYPE_GEN4_RPCSRC, CLK_PLL5),

	/* Core Clock Outputs */
	DEF_GEN4_Z("z0",	R8A779A0_CLK_Z0,	CLK_TYPE_GEN4_Z,	CLK_PLL20,	2, 0),
	DEF_GEN4_Z("z1",	R8A779A0_CLK_Z1,	CLK_TYPE_GEN4_Z,	CLK_PLL21,	2, 8),
	DEF_FIXED("zx",		R8A779A0_CLK_ZX,	CLK_PLL20_DIV2,	2, 1),
	DEF_FIXED("s1d1",	R8A779A0_CLK_S1D1,	CLK_S1,		1, 1),
	DEF_FIXED("s1d2",	R8A779A0_CLK_S1D2,	CLK_S1,		2, 1),
	DEF_FIXED("s1d4",	R8A779A0_CLK_S1D4,	CLK_S1,		4, 1),
	DEF_FIXED("s1d8",	R8A779A0_CLK_S1D8,	CLK_S1,		8, 1),
	DEF_FIXED("s1d12",	R8A779A0_CLK_S1D12,	CLK_S1,		12, 1),
	DEF_FIXED("s3d1",	R8A779A0_CLK_S3D1,	CLK_S3,		1, 1),
	DEF_FIXED("s3d2",	R8A779A0_CLK_S3D2,	CLK_S3,		2, 1),
	DEF_FIXED("s3d4",	R8A779A0_CLK_S3D4,	CLK_S3,		4, 1),
	DEF_FIXED("zs",		R8A779A0_CLK_ZS,	CLK_PLL1_DIV2,	4, 1),
	DEF_FIXED("zt",		R8A779A0_CLK_ZT,	CLK_PLL1_DIV2,	2, 1),
	DEF_FIXED("ztr",	R8A779A0_CLK_ZTR,	CLK_PLL1_DIV2,	2, 1),
	DEF_FIXED("zr",		R8A779A0_CLK_ZR,	CLK_PLL1_DIV2,	1, 1),
	DEF_FIXED("cnndsp",	R8A779A0_CLK_CNNDSP,	CLK_PLL5_DIV4,	1, 1),
	DEF_FIXED("vip",	R8A779A0_CLK_VIP,	CLK_PLL5,	5, 1),
	DEF_FIXED("adgh",	R8A779A0_CLK_ADGH,	CLK_PLL5_DIV4,	1, 1),
	DEF_FIXED("icu",	R8A779A0_CLK_ICU,	CLK_PLL5_DIV4,	2, 1),
	DEF_FIXED("icud2",	R8A779A0_CLK_ICUD2,	CLK_PLL5_DIV4,	4, 1),
	DEF_FIXED("vcbus",	R8A779A0_CLK_VCBUS,	CLK_PLL5_DIV4,	1, 1),
	DEF_FIXED("cbfusa",	R8A779A0_CLK_CBFUSA,	CLK_EXTAL,	2, 1),
	DEF_FIXED("cp",		R8A779A0_CLK_CP,	CLK_EXTAL,	2, 1),
	DEF_FIXED("cl16mck",	R8A779A0_CLK_CL16MCK,	CLK_PLL1_DIV2,	64, 1),

	DEF_GEN4_SDH("sd0h",	R8A779A0_CLK_SD0H,	CLK_SDSRC,	   CPG_SD0CKCR),
	DEF_GEN4_SD("sd0",	R8A779A0_CLK_SD0,	R8A779A0_CLK_SD0H, CPG_SD0CKCR),

	DEF_BASE("rpc",		R8A779A0_CLK_RPC, CLK_TYPE_GEN4_RPC, CLK_RPCSRC),
	DEF_BASE("rpcd2",	R8A779A0_CLK_RPCD2, CLK_TYPE_GEN4_RPCD2,
		 R8A779A0_CLK_RPC),

	DEF_DIV6P1("mso",	R8A779A0_CLK_MSO,	CLK_PLL5_DIV4,	CPG_MSOCKCR),
	DEF_DIV6P1("canfd",	R8A779A0_CLK_CANFD,	CLK_PLL5_DIV4,	CPG_CANFDCKCR),
	DEF_DIV6P1("csi0",	R8A779A0_CLK_CSI0,	CLK_PLL5_DIV4,	CPG_CSICKCR),
	DEF_DIV6P1("dsi",	R8A779A0_CLK_DSI,	CLK_PLL5_DIV4,	CPG_DSIEXTCKCR),

	DEF_GEN4_OSC("osc",	R8A779A0_CLK_OSC,	CLK_EXTAL,	8),
	DEF_GEN4_MDSEL("r",	R8A779A0_CLK_R, 29, CLK_EXTALR, 1, CLK_OCO, 1),
};

static const struct mssr_mod_clk r8a779a0_mod_clks[] __initconst = {
	DEF_MOD("avb0",		211,	R8A779A0_CLK_S3D2),
	DEF_MOD("avb1",		212,	R8A779A0_CLK_S3D2),
	DEF_MOD("avb2",		213,	R8A779A0_CLK_S3D2),
	DEF_MOD("avb3",		214,	R8A779A0_CLK_S3D2),
	DEF_MOD("avb4",		215,	R8A779A0_CLK_S3D2),
	DEF_MOD("avb5",		216,	R8A779A0_CLK_S3D2),
	DEF_MOD("canfd0",	328,	R8A779A0_CLK_S3D2),
	DEF_MOD("csi40",	331,	R8A779A0_CLK_CSI0),
	DEF_MOD("csi41",	400,	R8A779A0_CLK_CSI0),
	DEF_MOD("csi42",	401,	R8A779A0_CLK_CSI0),
	DEF_MOD("csi43",	402,	R8A779A0_CLK_CSI0),
	DEF_MOD("du",		411,	R8A779A0_CLK_S3D1),
	DEF_MOD("dsi0",		415,	R8A779A0_CLK_DSI),
	DEF_MOD("dsi1",		416,	R8A779A0_CLK_DSI),
	DEF_MOD("fcpvd0",	508,	R8A779A0_CLK_S3D1),
	DEF_MOD("fcpvd1",	509,	R8A779A0_CLK_S3D1),
	DEF_MOD("hscif0",	514,	R8A779A0_CLK_S1D2),
	DEF_MOD("hscif1",	515,	R8A779A0_CLK_S1D2),
	DEF_MOD("hscif2",	516,	R8A779A0_CLK_S1D2),
	DEF_MOD("hscif3",	517,	R8A779A0_CLK_S1D2),
	DEF_MOD("i2c0",		518,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c1",		519,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c2",		520,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c3",		521,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c4",		522,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c5",		523,	R8A779A0_CLK_S1D4),
	DEF_MOD("i2c6",		524,	R8A779A0_CLK_S1D4),
	DEF_MOD("ispcs0",	612,	R8A779A0_CLK_S1D1),
	DEF_MOD("ispcs1",	613,	R8A779A0_CLK_S1D1),
	DEF_MOD("ispcs2",	614,	R8A779A0_CLK_S1D1),
	DEF_MOD("ispcs3",	615,	R8A779A0_CLK_S1D1),
	DEF_MOD("msi0",		618,	R8A779A0_CLK_MSO),
	DEF_MOD("msi1",		619,	R8A779A0_CLK_MSO),
	DEF_MOD("msi2",		620,	R8A779A0_CLK_MSO),
	DEF_MOD("msi3",		621,	R8A779A0_CLK_MSO),
	DEF_MOD("msi4",		622,	R8A779A0_CLK_MSO),
	DEF_MOD("msi5",		623,	R8A779A0_CLK_MSO),
	DEF_MOD("pwm0",		628,	R8A779A0_CLK_S1D8),
	DEF_MOD("rpc-if",	629,	R8A779A0_CLK_RPCD2),
	DEF_MOD("scif0",	702,	R8A779A0_CLK_S1D8),
	DEF_MOD("scif1",	703,	R8A779A0_CLK_S1D8),
	DEF_MOD("scif3",	704,	R8A779A0_CLK_S1D8),
	DEF_MOD("scif4",	705,	R8A779A0_CLK_S1D8),
	DEF_MOD("sdhi0",	706,	R8A779A0_CLK_SD0),
	DEF_MOD("sys-dmac1",	709,	R8A779A0_CLK_S1D2),
	DEF_MOD("sys-dmac2",	710,	R8A779A0_CLK_S1D2),
	DEF_MOD("tmu0",		713,	R8A779A0_CLK_CL16MCK),
	DEF_MOD("tmu1",		714,	R8A779A0_CLK_S1D4),
	DEF_MOD("tmu2",		715,	R8A779A0_CLK_S1D4),
	DEF_MOD("tmu3",		716,	R8A779A0_CLK_S1D4),
	DEF_MOD("tmu4",		717,	R8A779A0_CLK_S1D4),
	DEF_MOD("tpu0",		718,	R8A779A0_CLK_S1D8),
	DEF_MOD("vin00",	730,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin01",	731,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin02",	800,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin03",	801,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin04",	802,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin05",	803,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin06",	804,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin07",	805,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin10",	806,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin11",	807,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin12",	808,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin13",	809,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin14",	810,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin15",	811,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin16",	812,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin17",	813,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin20",	814,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin21",	815,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin22",	816,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin23",	817,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin24",	818,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin25",	819,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin26",	820,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin27",	821,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin30",	822,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin31",	823,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin32",	824,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin33",	825,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin34",	826,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin35",	827,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin36",	828,	R8A779A0_CLK_S1D1),
	DEF_MOD("vin37",	829,	R8A779A0_CLK_S1D1),
	DEF_MOD("vspd0",	830,	R8A779A0_CLK_S3D1),
	DEF_MOD("vspd1",	831,	R8A779A0_CLK_S3D1),
	DEF_MOD("rwdt",		907,	R8A779A0_CLK_R),
	DEF_MOD("cmt0",		910,	R8A779A0_CLK_R),
	DEF_MOD("cmt1",		911,	R8A779A0_CLK_R),
	DEF_MOD("cmt2",		912,	R8A779A0_CLK_R),
	DEF_MOD("cmt3",		913,	R8A779A0_CLK_R),
	DEF_MOD("pfc0",		915,	R8A779A0_CLK_CP),
	DEF_MOD("pfc1",		916,	R8A779A0_CLK_CP),
	DEF_MOD("pfc2",		917,	R8A779A0_CLK_CP),
	DEF_MOD("pfc3",		918,	R8A779A0_CLK_CP),
	DEF_MOD("tsc",		919,	R8A779A0_CLK_CL16MCK),
	DEF_MOD("vspx0",	1028,	R8A779A0_CLK_S1D1),
	DEF_MOD("vspx1",	1029,	R8A779A0_CLK_S1D1),
	DEF_MOD("vspx2",	1030,	R8A779A0_CLK_S1D1),
	DEF_MOD("vspx3",	1031,	R8A779A0_CLK_S1D1),
};

/*
 * CPG Clock Data
 */
/*
 *   MD	 EXTAL		PLL1	PLL20	PLL30	PLL4	PLL5	OSC
 * 14 13 (MHz)			   21	   31
 * ----------------------------------------------------------------
 * 0  0	 16.66 x 1	x128	x216	x128	x144	x192	/16
 * 0  1	 20    x 1	x106	x180	x106	x120	x160	/19
 * 1  0	 Prohibited setting
 * 1  1	 33.33 / 2	x128	x216	x128	x144	x192	/32
 */
#define CPG_PLL_CONFIG_INDEX(md)	((((md) & BIT(14)) >> 13) | \
					 (((md) & BIT(13)) >> 13))
static const struct rcar_gen4_cpg_pll_config cpg_pll_configs[4] = {
	/* EXTAL div	PLL1 mult/div	PLL2 mult/div	PLL3 mult/div	PLL4 mult/div	PLL5 mult/div	PLL6 mult/div	OSC prediv */
	{ 1,		128,	1,	0,	0,	0,	0,	144,	1,	192,	1,	0,	0,	16,	},
	{ 1,		106,	1,	0,	0,	0,	0,	120,	1,	160,	1,	0,	0,	19,	},
	{ 0,		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	},
	{ 2,		128,	1,	0,	0,	0,	0,	144,	1,	192,	1,	0,	0,	32,	},
};

/*
 * Note that the only clock left running before booting Linux are now
 * MFIS, INTC-AP, INTC-EX and SCIF0 on V3U
 */
#define MSTPCR7_SCIF0	BIT(2)
#define MSTPCR6_MFIS	BIT(17)
#define MSTPCR6_INTC	BIT(11) /* No information: INTC-AP, INTC-EX */

static const struct mstp_stop_table r8a779a0_mstp_table[] = {
	{ 0x003f7ffe, 0x0, 0x0, 0x0 },
	{ 0x00cb0000, 0x0, 0x0, 0x0 },
	{ 0x0001f800, 0x0, 0x0, 0x0 },
	{ 0x90000000, 0x0, 0x0, 0x0 },
	{ 0x0001c807, 0x0, 0x0, 0x0 },
	{ 0x7e03c380, 0x0, 0x0, 0x0 },
	{ 0x1f01f001, MSTPCR6_MFIS, 0x0, 0x0 },
	{ 0xffffe040, MSTPCR7_SCIF0, 0x0, 0x0 },
	{ 0xffffffff, 0x0, 0x0, 0x0 },
	{ 0x00003c78, 0x0, 0x0, 0x0 },
	{ 0xf0000000, 0x0, 0x0, 0x0 },
	{ 0x0000000f, 0x0, 0x0, 0x0 },
	{ 0xbe800000, 0x0, 0x0, 0x0 },
	{ 0x00000037, 0x0, 0x0, 0x0 },
	{ 0x00000000, 0x0, 0x0, 0x0 },
};

static const void *r8a779a0_get_pll_config(const u32 cpg_mode)
{
	return &cpg_pll_configs[CPG_PLL_CONFIG_INDEX(cpg_mode)];
}

static const struct cpg_mssr_info r8a779a0_cpg_mssr_info = {
	.core_clk		= r8a779a0_core_clks,
	.core_clk_size		= ARRAY_SIZE(r8a779a0_core_clks),
	.mod_clk		= r8a779a0_mod_clks,
	.mod_clk_size		= ARRAY_SIZE(r8a779a0_mod_clks),
	.mstp_table		= r8a779a0_mstp_table,
	.mstp_table_size	= ARRAY_SIZE(r8a779a0_mstp_table),
	.reset_node		= "renesas,r8a779a0-rst",
	.reset_modemr_offset	= CPG_RST_MODEMR0,
	.extalr_node		= "extalr",
	.mod_clk_base		= MOD_CLK_BASE,
	.clk_extal_id		= CLK_EXTAL,
	.clk_extalr_id		= CLK_EXTALR,
	.get_pll_config		= r8a779a0_get_pll_config,
	.reg_layout		= CLK_REG_LAYOUT_RCAR_GEN4,
};

static const struct udevice_id r8a779a0_cpg_ids[] = {
	{
		.compatible	= "renesas,r8a779a0-cpg-mssr",
		.data		= (ulong)&r8a779a0_cpg_mssr_info
	},
	{ }
};

U_BOOT_DRIVER(cpg_r8a779a0) = {
	.name		= "cpg_r8a779a0",
	.id		= UCLASS_NOP,
	.of_match	= r8a779a0_cpg_ids,
	.bind		= gen3_cpg_bind,
};
