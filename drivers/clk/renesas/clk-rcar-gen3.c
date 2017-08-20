/*
 * Renesas RCar Gen3 R8A7795/R8A7796 CPG MSSR driver
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
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
#include <errno.h>
#include <wait_bit.h>
#include <asm/io.h>

#include <dt-bindings/clock/r8a7795-cpg-mssr.h>
#include <dt-bindings/clock/r8a7796-cpg-mssr.h>

#define CPG_RST_MODEMR		0x0060

#define CPG_PLL0CR		0x00d8
#define CPG_PLL2CR		0x002c
#define CPG_PLL4CR		0x01f4

/*
 * Module Standby and Software Reset register offets.
 *
 * If the registers exist, these are valid for SH-Mobile, R-Mobile,
 * R-Car Gen2, R-Car Gen3, and RZ/G1.
 * These are NOT valid for R-Car Gen1 and RZ/A1!
 */

/*
 * Module Stop Status Register offsets
 */

static const u16 mstpsr[] = {
	0x030, 0x038, 0x040, 0x048, 0x04C, 0x03C, 0x1C0, 0x1C4,
	0x9A0, 0x9A4, 0x9A8, 0x9AC,
};

#define	MSTPSR(i)	mstpsr[i]


/*
 * System Module Stop Control Register offsets
 */

static const u16 smstpcr[] = {
	0x130, 0x134, 0x138, 0x13C, 0x140, 0x144, 0x148, 0x14C,
	0x990, 0x994, 0x998, 0x99C,
};

#define	SMSTPCR(i)	smstpcr[i]


/* Realtime Module Stop Control Register offsets */
#define RMSTPCR(i)	(smstpcr[i] - 0x20)

/* Modem Module Stop Control Register offsets (r8a73a4) */
#define MMSTPCR(i)	(smstpcr[i] + 0x20)

/* Software Reset Clearing Register offsets */
#define	SRSTCLR(i)	(0x940 + (i) * 4)

struct gen3_clk_priv {
	void __iomem	*base;
	struct clk	clk_extal;
	struct clk	clk_extalr;
	const struct rcar_gen3_cpg_pll_config *cpg_pll_config;
	const struct cpg_core_clk *core_clk;
	u32		core_clk_size;
	const struct mssr_mod_clk *mod_clk;
	u32		mod_clk_size;
};

/*
 * Definitions of CPG Core Clocks
 *
 * These include:
 *   - Clock outputs exported to DT
 *   - External input clocks
 *   - Internal CPG clocks
 */
struct cpg_core_clk {
	/* Common */
	const char *name;
	unsigned int id;
	unsigned int type;
	/* Depending on type */
	unsigned int parent;	/* Core Clocks only */
	unsigned int div;
	unsigned int mult;
	unsigned int offset;
};

enum clk_types {
	/* Generic */
	CLK_TYPE_IN,		/* External Clock Input */
	CLK_TYPE_FF,		/* Fixed Factor Clock */

	/* Custom definitions start here */
	CLK_TYPE_CUSTOM,
};

#define DEF_TYPE(_name, _id, _type...)	\
	{ .name = _name, .id = _id, .type = _type }
#define DEF_BASE(_name, _id, _type, _parent...)	\
	DEF_TYPE(_name, _id, _type, .parent = _parent)

#define DEF_INPUT(_name, _id) \
	DEF_TYPE(_name, _id, CLK_TYPE_IN)
#define DEF_FIXED(_name, _id, _parent, _div, _mult)	\
	DEF_BASE(_name, _id, CLK_TYPE_FF, _parent, .div = _div, .mult = _mult)
#define DEF_GEN3_SD(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_GEN3_SD, _parent, .offset = _offset)

/*
 * Definitions of Module Clocks
 */
struct mssr_mod_clk {
	const char *name;
	unsigned int id;
	unsigned int parent;	/* Add MOD_CLK_BASE for Module Clocks */
};

/* Convert from sparse base-100 to packed index space */
#define MOD_CLK_PACK(x)	((x) - ((x) / 100) * (100 - 32))

#define MOD_CLK_ID(x)	(MOD_CLK_BASE + MOD_CLK_PACK(x))

#define DEF_MOD(_name, _mod, _parent...)	\
	{ .name = _name, .id = MOD_CLK_ID(_mod), .parent = _parent }

enum rcar_gen3_clk_types {
	CLK_TYPE_GEN3_MAIN = CLK_TYPE_CUSTOM,
	CLK_TYPE_GEN3_PLL0,
	CLK_TYPE_GEN3_PLL1,
	CLK_TYPE_GEN3_PLL2,
	CLK_TYPE_GEN3_PLL3,
	CLK_TYPE_GEN3_PLL4,
	CLK_TYPE_GEN3_SD,
	CLK_TYPE_GEN3_R,
};

struct rcar_gen3_cpg_pll_config {
	unsigned int extal_div;
	unsigned int pll1_mult;
	unsigned int pll3_mult;
};

enum clk_ids {
	/* Core Clock Outputs exported to DT */
	LAST_DT_CORE_CLK = R8A7796_CLK_OSC,

	/* External Input Clocks */
	CLK_EXTAL,
	CLK_EXTALR,

	/* Internal Core Clocks */
	CLK_MAIN,
	CLK_PLL0,
	CLK_PLL1,
	CLK_PLL2,
	CLK_PLL3,
	CLK_PLL4,
	CLK_PLL1_DIV2,
	CLK_PLL1_DIV4,
	CLK_S0,
	CLK_S1,
	CLK_S2,
	CLK_S3,
	CLK_SDSRC,
	CLK_SSPSRC,
	CLK_RINT,

	/* Module Clocks */
	MOD_CLK_BASE
};

static const struct cpg_core_clk r8a7795_core_clks[] = {
	/* External Clock Inputs */
	DEF_INPUT("extal",      CLK_EXTAL),
	DEF_INPUT("extalr",     CLK_EXTALR),

	/* Internal Core Clocks */
	DEF_BASE(".main",       CLK_MAIN, CLK_TYPE_GEN3_MAIN, CLK_EXTAL),
	DEF_BASE(".pll0",       CLK_PLL0, CLK_TYPE_GEN3_PLL0, CLK_MAIN),
	DEF_BASE(".pll1",       CLK_PLL1, CLK_TYPE_GEN3_PLL1, CLK_MAIN),
	DEF_BASE(".pll2",       CLK_PLL2, CLK_TYPE_GEN3_PLL2, CLK_MAIN),
	DEF_BASE(".pll3",       CLK_PLL3, CLK_TYPE_GEN3_PLL3, CLK_MAIN),
	DEF_BASE(".pll4",       CLK_PLL4, CLK_TYPE_GEN3_PLL4, CLK_MAIN),

	DEF_FIXED(".pll1_div2", CLK_PLL1_DIV2,     CLK_PLL1,       2, 1),
	DEF_FIXED(".pll1_div4", CLK_PLL1_DIV4,     CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED(".s0",        CLK_S0,            CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED(".s1",        CLK_S1,            CLK_PLL1_DIV2,  3, 1),
	DEF_FIXED(".s2",        CLK_S2,            CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED(".s3",        CLK_S3,            CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED(".sdsrc",     CLK_SDSRC,         CLK_PLL1_DIV2,  2, 1),

	/* Core Clock Outputs */
	DEF_FIXED("ztr",        R8A7795_CLK_ZTR,   CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED("ztrd2",      R8A7795_CLK_ZTRD2, CLK_PLL1_DIV2, 12, 1),
	DEF_FIXED("zt",         R8A7795_CLK_ZT,    CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED("zx",         R8A7795_CLK_ZX,    CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED("s0d1",       R8A7795_CLK_S0D1,  CLK_S0,         1, 1),
	DEF_FIXED("s0d2",       R8A7795_CLK_S0D2,  CLK_S0,         2, 1),
	DEF_FIXED("s0d3",       R8A7795_CLK_S0D3,  CLK_S0,         3, 1),
	DEF_FIXED("s0d4",       R8A7795_CLK_S0D4,  CLK_S0,         4, 1),
	DEF_FIXED("s0d6",       R8A7795_CLK_S0D6,  CLK_S0,         6, 1),
	DEF_FIXED("s0d8",       R8A7795_CLK_S0D8,  CLK_S0,         8, 1),
	DEF_FIXED("s0d12",      R8A7795_CLK_S0D12, CLK_S0,        12, 1),
	DEF_FIXED("s1d1",       R8A7795_CLK_S1D1,  CLK_S1,         1, 1),
	DEF_FIXED("s1d2",       R8A7795_CLK_S1D2,  CLK_S1,         2, 1),
	DEF_FIXED("s1d4",       R8A7795_CLK_S1D4,  CLK_S1,         4, 1),
	DEF_FIXED("s2d1",       R8A7795_CLK_S2D1,  CLK_S2,         1, 1),
	DEF_FIXED("s2d2",       R8A7795_CLK_S2D2,  CLK_S2,         2, 1),
	DEF_FIXED("s2d4",       R8A7795_CLK_S2D4,  CLK_S2,         4, 1),
	DEF_FIXED("s3d1",       R8A7795_CLK_S3D1,  CLK_S3,         1, 1),
	DEF_FIXED("s3d2",       R8A7795_CLK_S3D2,  CLK_S3,         2, 1),
	DEF_FIXED("s3d4",       R8A7795_CLK_S3D4,  CLK_S3,         4, 1),

	DEF_GEN3_SD("sd0",      R8A7795_CLK_SD0,   CLK_SDSRC,     0x074),
	DEF_GEN3_SD("sd1",      R8A7795_CLK_SD1,   CLK_SDSRC,     0x078),
	DEF_GEN3_SD("sd2",      R8A7795_CLK_SD2,   CLK_SDSRC,     0x268),
	DEF_GEN3_SD("sd3",      R8A7795_CLK_SD3,   CLK_SDSRC,     0x26c),

	DEF_FIXED("cl",         R8A7795_CLK_CL,    CLK_PLL1_DIV2, 48, 1),
	DEF_FIXED("cp",         R8A7795_CLK_CP,    CLK_EXTAL,      2, 1),

	/* NOTE: HDMI, CSI, CAN etc. clock are missing */

	DEF_BASE("r",           R8A7795_CLK_R,     CLK_TYPE_GEN3_R, CLK_RINT),
};

static const struct mssr_mod_clk r8a7795_mod_clks[] = {
	DEF_MOD("fdp1-2",		 117,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fdp1-1",		 118,	R8A7795_CLK_S0D1),
	DEF_MOD("fdp1-0",		 119,	R8A7795_CLK_S0D1),
	DEF_MOD("scif5",		 202,	R8A7795_CLK_S3D4),
	DEF_MOD("scif4",		 203,	R8A7795_CLK_S3D4),
	DEF_MOD("scif3",		 204,	R8A7795_CLK_S3D4),
	DEF_MOD("scif1",		 206,	R8A7795_CLK_S3D4),
	DEF_MOD("scif0",		 207,	R8A7795_CLK_S3D4),
	DEF_MOD("msiof3",		 208,	R8A7795_CLK_MSO),
	DEF_MOD("msiof2",		 209,	R8A7795_CLK_MSO),
	DEF_MOD("msiof1",		 210,	R8A7795_CLK_MSO),
	DEF_MOD("msiof0",		 211,	R8A7795_CLK_MSO),
	DEF_MOD("sys-dmac2",		 217,	R8A7795_CLK_S0D3),
	DEF_MOD("sys-dmac1",		 218,	R8A7795_CLK_S0D3),
	DEF_MOD("sys-dmac0",		 219,	R8A7795_CLK_S0D3),
	DEF_MOD("cmt3",			 300,	R8A7795_CLK_R),
	DEF_MOD("cmt2",			 301,	R8A7795_CLK_R),
	DEF_MOD("cmt1",			 302,	R8A7795_CLK_R),
	DEF_MOD("cmt0",			 303,	R8A7795_CLK_R),
	DEF_MOD("scif2",		 310,	R8A7795_CLK_S3D4),
	DEF_MOD("sdif3",		 311,	R8A7795_CLK_SD3),
	DEF_MOD("sdif2",		 312,	R8A7795_CLK_SD2),
	DEF_MOD("sdif1",		 313,	R8A7795_CLK_SD1),
	DEF_MOD("sdif0",		 314,	R8A7795_CLK_SD0),
	DEF_MOD("pcie1",		 318,	R8A7795_CLK_S3D1),
	DEF_MOD("pcie0",		 319,	R8A7795_CLK_S3D1),
	DEF_MOD("usb-dmac30",		 326,	R8A7795_CLK_S3D1),
	DEF_MOD("usb3-if1",		 327,	R8A7795_CLK_S3D1), /* ES1.x */
	DEF_MOD("usb3-if0",		 328,	R8A7795_CLK_S3D1),
	DEF_MOD("usb-dmac31",		 329,	R8A7795_CLK_S3D1),
	DEF_MOD("usb-dmac0",		 330,	R8A7795_CLK_S3D1),
	DEF_MOD("usb-dmac1",		 331,	R8A7795_CLK_S3D1),
	DEF_MOD("rwdt",			 402,	R8A7795_CLK_R),
	DEF_MOD("intc-ex",		 407,	R8A7795_CLK_CP),
	DEF_MOD("intc-ap",		 408,	R8A7795_CLK_S3D1),
	DEF_MOD("audmac1",		 501,	R8A7795_CLK_S0D3),
	DEF_MOD("audmac0",		 502,	R8A7795_CLK_S0D3),
	DEF_MOD("drif7",		 508,	R8A7795_CLK_S3D2),
	DEF_MOD("drif6",		 509,	R8A7795_CLK_S3D2),
	DEF_MOD("drif5",		 510,	R8A7795_CLK_S3D2),
	DEF_MOD("drif4",		 511,	R8A7795_CLK_S3D2),
	DEF_MOD("drif3",		 512,	R8A7795_CLK_S3D2),
	DEF_MOD("drif2",		 513,	R8A7795_CLK_S3D2),
	DEF_MOD("drif1",		 514,	R8A7795_CLK_S3D2),
	DEF_MOD("drif0",		 515,	R8A7795_CLK_S3D2),
	DEF_MOD("hscif4",		 516,	R8A7795_CLK_S3D1),
	DEF_MOD("hscif3",		 517,	R8A7795_CLK_S3D1),
	DEF_MOD("hscif2",		 518,	R8A7795_CLK_S3D1),
	DEF_MOD("hscif1",		 519,	R8A7795_CLK_S3D1),
	DEF_MOD("hscif0",		 520,	R8A7795_CLK_S3D1),
	DEF_MOD("thermal",		 522,	R8A7795_CLK_CP),
	DEF_MOD("pwm",			 523,	R8A7795_CLK_S0D12),
	DEF_MOD("fcpvd3",		 600,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fcpvd2",		 601,	R8A7795_CLK_S0D2),
	DEF_MOD("fcpvd1",		 602,	R8A7795_CLK_S0D2),
	DEF_MOD("fcpvd0",		 603,	R8A7795_CLK_S0D2),
	DEF_MOD("fcpvb1",		 606,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpvb0",		 607,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpvi2",		 609,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fcpvi1",		 610,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpvi0",		 611,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpf2",		 613,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fcpf1",		 614,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpf0",		 615,	R8A7795_CLK_S0D1),
	DEF_MOD("fcpci1",		 616,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fcpci0",		 617,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("fcpcs",		 619,	R8A7795_CLK_S0D1),
	DEF_MOD("vspd3",		 620,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("vspd2",		 621,	R8A7795_CLK_S0D2),
	DEF_MOD("vspd1",		 622,	R8A7795_CLK_S0D2),
	DEF_MOD("vspd0",		 623,	R8A7795_CLK_S0D2),
	DEF_MOD("vspbc",		 624,	R8A7795_CLK_S0D1),
	DEF_MOD("vspbd",		 626,	R8A7795_CLK_S0D1),
	DEF_MOD("vspi2",		 629,	R8A7795_CLK_S2D1), /* ES1.x */
	DEF_MOD("vspi1",		 630,	R8A7795_CLK_S0D1),
	DEF_MOD("vspi0",		 631,	R8A7795_CLK_S0D1),
	DEF_MOD("ehci3",		 700,	R8A7795_CLK_S3D4),
	DEF_MOD("ehci2",		 701,	R8A7795_CLK_S3D4),
	DEF_MOD("ehci1",		 702,	R8A7795_CLK_S3D4),
	DEF_MOD("ehci0",		 703,	R8A7795_CLK_S3D4),
	DEF_MOD("hsusb",		 704,	R8A7795_CLK_S3D4),
	DEF_MOD("hsusb3",		 705,	R8A7795_CLK_S3D4),
	DEF_MOD("csi21",		 713,	R8A7795_CLK_CSI0), /* ES1.x */
	DEF_MOD("csi20",		 714,	R8A7795_CLK_CSI0),
	DEF_MOD("csi41",		 715,	R8A7795_CLK_CSI0),
	DEF_MOD("csi40",		 716,	R8A7795_CLK_CSI0),
	DEF_MOD("du3",			 721,	R8A7795_CLK_S2D1),
	DEF_MOD("du2",			 722,	R8A7795_CLK_S2D1),
	DEF_MOD("du1",			 723,	R8A7795_CLK_S2D1),
	DEF_MOD("du0",			 724,	R8A7795_CLK_S2D1),
	DEF_MOD("lvds",			 727,	R8A7795_CLK_S0D4),
	DEF_MOD("hdmi1",		 728,	R8A7795_CLK_HDMI),
	DEF_MOD("hdmi0",		 729,	R8A7795_CLK_HDMI),
	DEF_MOD("vin7",			 804,	R8A7795_CLK_S0D2),
	DEF_MOD("vin6",			 805,	R8A7795_CLK_S0D2),
	DEF_MOD("vin5",			 806,	R8A7795_CLK_S0D2),
	DEF_MOD("vin4",			 807,	R8A7795_CLK_S0D2),
	DEF_MOD("vin3",			 808,	R8A7795_CLK_S0D2),
	DEF_MOD("vin2",			 809,	R8A7795_CLK_S0D2),
	DEF_MOD("vin1",			 810,	R8A7795_CLK_S0D2),
	DEF_MOD("vin0",			 811,	R8A7795_CLK_S0D2),
	DEF_MOD("etheravb",		 812,	R8A7795_CLK_S0D6),
	DEF_MOD("sata0",		 815,	R8A7795_CLK_S3D2),
	DEF_MOD("imr3",			 820,	R8A7795_CLK_S0D2),
	DEF_MOD("imr2",			 821,	R8A7795_CLK_S0D2),
	DEF_MOD("imr1",			 822,	R8A7795_CLK_S0D2),
	DEF_MOD("imr0",			 823,	R8A7795_CLK_S0D2),
	DEF_MOD("gpio7",		 905,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio6",		 906,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio5",		 907,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio4",		 908,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio3",		 909,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio2",		 910,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio1",		 911,	R8A7795_CLK_S3D4),
	DEF_MOD("gpio0",		 912,	R8A7795_CLK_S3D4),
	DEF_MOD("can-fd",		 914,	R8A7795_CLK_S3D2),
	DEF_MOD("can-if1",		 915,	R8A7795_CLK_S3D4),
	DEF_MOD("can-if0",		 916,	R8A7795_CLK_S3D4),
	DEF_MOD("i2c6",			 918,	R8A7795_CLK_S0D6),
	DEF_MOD("i2c5",			 919,	R8A7795_CLK_S0D6),
	DEF_MOD("i2c-dvfs",		 926,	R8A7795_CLK_CP),
	DEF_MOD("i2c4",			 927,	R8A7795_CLK_S0D6),
	DEF_MOD("i2c3",			 928,	R8A7795_CLK_S0D6),
	DEF_MOD("i2c2",			 929,	R8A7795_CLK_S3D2),
	DEF_MOD("i2c1",			 930,	R8A7795_CLK_S3D2),
	DEF_MOD("i2c0",			 931,	R8A7795_CLK_S3D2),
	DEF_MOD("ssi-all",		1005,	R8A7795_CLK_S3D4),
	DEF_MOD("ssi9",			1006,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi8",			1007,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi7",			1008,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi6",			1009,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi5",			1010,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi4",			1011,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi3",			1012,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi2",			1013,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi1",			1014,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi0",			1015,	MOD_CLK_ID(1005)),
	DEF_MOD("scu-all",		1017,	R8A7795_CLK_S3D4),
	DEF_MOD("scu-dvc1",		1018,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-dvc0",		1019,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-ctu1-mix1",	1020,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-ctu0-mix0",	1021,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src9",		1022,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src8",		1023,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src7",		1024,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src6",		1025,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src5",		1026,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src4",		1027,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src3",		1028,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src2",		1029,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src1",		1030,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src0",		1031,	MOD_CLK_ID(1017)),
};

static const struct cpg_core_clk r8a7796_core_clks[] = {
	/* External Clock Inputs */
	DEF_INPUT("extal",      CLK_EXTAL),
	DEF_INPUT("extalr",     CLK_EXTALR),

	/* Internal Core Clocks */
	DEF_BASE(".main",       CLK_MAIN, CLK_TYPE_GEN3_MAIN, CLK_EXTAL),
	DEF_BASE(".pll0",       CLK_PLL0, CLK_TYPE_GEN3_PLL0, CLK_MAIN),
	DEF_BASE(".pll1",       CLK_PLL1, CLK_TYPE_GEN3_PLL1, CLK_MAIN),
	DEF_BASE(".pll2",       CLK_PLL2, CLK_TYPE_GEN3_PLL2, CLK_MAIN),
	DEF_BASE(".pll3",       CLK_PLL3, CLK_TYPE_GEN3_PLL3, CLK_MAIN),
	DEF_BASE(".pll4",       CLK_PLL4, CLK_TYPE_GEN3_PLL4, CLK_MAIN),

	DEF_FIXED(".pll1_div2", CLK_PLL1_DIV2,     CLK_PLL1,       2, 1),
	DEF_FIXED(".pll1_div4", CLK_PLL1_DIV4,     CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED(".s0",        CLK_S0,            CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED(".s1",        CLK_S1,            CLK_PLL1_DIV2,  3, 1),
	DEF_FIXED(".s2",        CLK_S2,            CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED(".s3",        CLK_S3,            CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED(".sdsrc",     CLK_SDSRC,         CLK_PLL1_DIV2,  2, 1),

	/* Core Clock Outputs */
	DEF_FIXED("ztr",        R8A7796_CLK_ZTR,   CLK_PLL1_DIV2,  6, 1),
	DEF_FIXED("ztrd2",      R8A7796_CLK_ZTRD2, CLK_PLL1_DIV2, 12, 1),
	DEF_FIXED("zt",         R8A7796_CLK_ZT,    CLK_PLL1_DIV2,  4, 1),
	DEF_FIXED("zx",         R8A7796_CLK_ZX,    CLK_PLL1_DIV2,  2, 1),
	DEF_FIXED("s0d1",       R8A7796_CLK_S0D1,  CLK_S0,         1, 1),
	DEF_FIXED("s0d2",       R8A7796_CLK_S0D2,  CLK_S0,         2, 1),
	DEF_FIXED("s0d3",       R8A7796_CLK_S0D3,  CLK_S0,         3, 1),
	DEF_FIXED("s0d4",       R8A7796_CLK_S0D4,  CLK_S0,         4, 1),
	DEF_FIXED("s0d6",       R8A7796_CLK_S0D6,  CLK_S0,         6, 1),
	DEF_FIXED("s0d8",       R8A7796_CLK_S0D8,  CLK_S0,         8, 1),
	DEF_FIXED("s0d12",      R8A7796_CLK_S0D12, CLK_S0,        12, 1),
	DEF_FIXED("s1d1",       R8A7796_CLK_S1D1,  CLK_S1,         1, 1),
	DEF_FIXED("s1d2",       R8A7796_CLK_S1D2,  CLK_S1,         2, 1),
	DEF_FIXED("s1d4",       R8A7796_CLK_S1D4,  CLK_S1,         4, 1),
	DEF_FIXED("s2d1",       R8A7796_CLK_S2D1,  CLK_S2,         1, 1),
	DEF_FIXED("s2d2",       R8A7796_CLK_S2D2,  CLK_S2,         2, 1),
	DEF_FIXED("s2d4",       R8A7796_CLK_S2D4,  CLK_S2,         4, 1),
	DEF_FIXED("s3d1",       R8A7796_CLK_S3D1,  CLK_S3,         1, 1),
	DEF_FIXED("s3d2",       R8A7796_CLK_S3D2,  CLK_S3,         2, 1),
	DEF_FIXED("s3d4",       R8A7796_CLK_S3D4,  CLK_S3,         4, 1),

	DEF_GEN3_SD("sd0",      R8A7796_CLK_SD0,   CLK_SDSRC,     0x074),
	DEF_GEN3_SD("sd1",      R8A7796_CLK_SD1,   CLK_SDSRC,     0x078),
	DEF_GEN3_SD("sd2",      R8A7796_CLK_SD2,   CLK_SDSRC,     0x268),
	DEF_GEN3_SD("sd3",      R8A7796_CLK_SD3,   CLK_SDSRC,     0x26c),

	DEF_FIXED("cl",         R8A7796_CLK_CL,    CLK_PLL1_DIV2, 48, 1),
	DEF_FIXED("cp",         R8A7796_CLK_CP,    CLK_EXTAL,      2, 1),

	/* NOTE: HDMI, CSI, CAN etc. clock are missing */

	DEF_BASE("r",           R8A7796_CLK_R,     CLK_TYPE_GEN3_R, CLK_RINT),
};

static const struct mssr_mod_clk r8a7796_mod_clks[] = {
	DEF_MOD("scif5",		 202,	R8A7796_CLK_S3D4),
	DEF_MOD("scif4",		 203,	R8A7796_CLK_S3D4),
	DEF_MOD("scif3",		 204,	R8A7796_CLK_S3D4),
	DEF_MOD("scif1",		 206,	R8A7796_CLK_S3D4),
	DEF_MOD("scif0",		 207,	R8A7796_CLK_S3D4),
	DEF_MOD("msiof3",		 208,	R8A7796_CLK_MSO),
	DEF_MOD("msiof2",		 209,	R8A7796_CLK_MSO),
	DEF_MOD("msiof1",		 210,	R8A7796_CLK_MSO),
	DEF_MOD("msiof0",		 211,	R8A7796_CLK_MSO),
	DEF_MOD("sys-dmac2",		 217,	R8A7796_CLK_S0D3),
	DEF_MOD("sys-dmac1",		 218,	R8A7796_CLK_S0D3),
	DEF_MOD("sys-dmac0",		 219,	R8A7796_CLK_S0D3),
	DEF_MOD("cmt3",			 300,	R8A7796_CLK_R),
	DEF_MOD("cmt2",			 301,	R8A7796_CLK_R),
	DEF_MOD("cmt1",			 302,	R8A7796_CLK_R),
	DEF_MOD("cmt0",			 303,	R8A7796_CLK_R),
	DEF_MOD("scif2",		 310,	R8A7796_CLK_S3D4),
	DEF_MOD("sdif3",		 311,	R8A7796_CLK_SD3),
	DEF_MOD("sdif2",		 312,	R8A7796_CLK_SD2),
	DEF_MOD("sdif1",		 313,	R8A7796_CLK_SD1),
	DEF_MOD("sdif0",		 314,	R8A7796_CLK_SD0),
	DEF_MOD("pcie1",		 318,	R8A7796_CLK_S3D1),
	DEF_MOD("pcie0",		 319,	R8A7796_CLK_S3D1),
	DEF_MOD("usb-dmac0",		 330,	R8A7796_CLK_S3D1),
	DEF_MOD("usb-dmac1",		 331,	R8A7796_CLK_S3D1),
	DEF_MOD("rwdt",			 402,	R8A7796_CLK_R),
	DEF_MOD("intc-ex",		 407,	R8A7796_CLK_CP),
	DEF_MOD("intc-ap",		 408,	R8A7796_CLK_S3D1),
	DEF_MOD("audmac1",		 501,	R8A7796_CLK_S0D3),
	DEF_MOD("audmac0",		 502,	R8A7796_CLK_S0D3),
	DEF_MOD("drif7",		 508,	R8A7796_CLK_S3D2),
	DEF_MOD("drif6",		 509,	R8A7796_CLK_S3D2),
	DEF_MOD("drif5",		 510,	R8A7796_CLK_S3D2),
	DEF_MOD("drif4",		 511,	R8A7796_CLK_S3D2),
	DEF_MOD("drif3",		 512,	R8A7796_CLK_S3D2),
	DEF_MOD("drif2",		 513,	R8A7796_CLK_S3D2),
	DEF_MOD("drif1",		 514,	R8A7796_CLK_S3D2),
	DEF_MOD("drif0",		 515,	R8A7796_CLK_S3D2),
	DEF_MOD("hscif4",		 516,	R8A7796_CLK_S3D1),
	DEF_MOD("hscif3",		 517,	R8A7796_CLK_S3D1),
	DEF_MOD("hscif2",		 518,	R8A7796_CLK_S3D1),
	DEF_MOD("hscif1",		 519,	R8A7796_CLK_S3D1),
	DEF_MOD("hscif0",		 520,	R8A7796_CLK_S3D1),
	DEF_MOD("thermal",		 522,	R8A7796_CLK_CP),
	DEF_MOD("pwm",			 523,	R8A7796_CLK_S0D12),
	DEF_MOD("fcpvd2",		 601,	R8A7796_CLK_S0D2),
	DEF_MOD("fcpvd1",		 602,	R8A7796_CLK_S0D2),
	DEF_MOD("fcpvd0",		 603,	R8A7796_CLK_S0D2),
	DEF_MOD("fcpvb0",		 607,	R8A7796_CLK_S0D1),
	DEF_MOD("fcpvi0",		 611,	R8A7796_CLK_S0D1),
	DEF_MOD("fcpf0",		 615,	R8A7796_CLK_S0D1),
	DEF_MOD("fcpci0",		 617,	R8A7796_CLK_S0D2),
	DEF_MOD("fcpcs",		 619,	R8A7796_CLK_S0D2),
	DEF_MOD("vspd2",		 621,	R8A7796_CLK_S0D2),
	DEF_MOD("vspd1",		 622,	R8A7796_CLK_S0D2),
	DEF_MOD("vspd0",		 623,	R8A7796_CLK_S0D2),
	DEF_MOD("vspb",			 626,	R8A7796_CLK_S0D1),
	DEF_MOD("vspi0",		 631,	R8A7796_CLK_S0D1),
	DEF_MOD("ehci1",		 702,	R8A7796_CLK_S3D4),
	DEF_MOD("ehci0",		 703,	R8A7796_CLK_S3D4),
	DEF_MOD("hsusb",		 704,	R8A7796_CLK_S3D4),
	DEF_MOD("csi20",		 714,	R8A7796_CLK_CSI0),
	DEF_MOD("csi40",		 716,	R8A7796_CLK_CSI0),
	DEF_MOD("du2",			 722,	R8A7796_CLK_S2D1),
	DEF_MOD("du1",			 723,	R8A7796_CLK_S2D1),
	DEF_MOD("du0",			 724,	R8A7796_CLK_S2D1),
	DEF_MOD("lvds",			 727,	R8A7796_CLK_S2D1),
	DEF_MOD("hdmi0",		 729,	R8A7796_CLK_HDMI),
	DEF_MOD("vin7",			 804,	R8A7796_CLK_S0D2),
	DEF_MOD("vin6",			 805,	R8A7796_CLK_S0D2),
	DEF_MOD("vin5",			 806,	R8A7796_CLK_S0D2),
	DEF_MOD("vin4",			 807,	R8A7796_CLK_S0D2),
	DEF_MOD("vin3",			 808,	R8A7796_CLK_S0D2),
	DEF_MOD("vin2",			 809,	R8A7796_CLK_S0D2),
	DEF_MOD("vin1",			 810,	R8A7796_CLK_S0D2),
	DEF_MOD("vin0",			 811,	R8A7796_CLK_S0D2),
	DEF_MOD("etheravb",		 812,	R8A7796_CLK_S0D6),
	DEF_MOD("imr1",			 822,	R8A7796_CLK_S0D2),
	DEF_MOD("imr0",			 823,	R8A7796_CLK_S0D2),
	DEF_MOD("gpio7",		 905,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio6",		 906,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio5",		 907,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio4",		 908,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio3",		 909,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio2",		 910,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio1",		 911,	R8A7796_CLK_S3D4),
	DEF_MOD("gpio0",		 912,	R8A7796_CLK_S3D4),
	DEF_MOD("can-fd",		 914,	R8A7796_CLK_S3D2),
	DEF_MOD("can-if1",		 915,	R8A7796_CLK_S3D4),
	DEF_MOD("can-if0",		 916,	R8A7796_CLK_S3D4),
	DEF_MOD("i2c6",			 918,	R8A7796_CLK_S0D6),
	DEF_MOD("i2c5",			 919,	R8A7796_CLK_S0D6),
	DEF_MOD("i2c-dvfs",		 926,	R8A7796_CLK_CP),
	DEF_MOD("i2c4",			 927,	R8A7796_CLK_S0D6),
	DEF_MOD("i2c3",			 928,	R8A7796_CLK_S0D6),
	DEF_MOD("i2c2",			 929,	R8A7796_CLK_S3D2),
	DEF_MOD("i2c1",			 930,	R8A7796_CLK_S3D2),
	DEF_MOD("i2c0",			 931,	R8A7796_CLK_S3D2),
	DEF_MOD("ssi-all",		1005,	R8A7796_CLK_S3D4),
	DEF_MOD("ssi9",			1006,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi8",			1007,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi7",			1008,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi6",			1009,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi5",			1010,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi4",			1011,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi3",			1012,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi2",			1013,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi1",			1014,	MOD_CLK_ID(1005)),
	DEF_MOD("ssi0",			1015,	MOD_CLK_ID(1005)),
	DEF_MOD("scu-all",		1017,	R8A7796_CLK_S3D4),
	DEF_MOD("scu-dvc1",		1018,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-dvc0",		1019,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-ctu1-mix1",	1020,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-ctu0-mix0",	1021,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src9",		1022,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src8",		1023,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src7",		1024,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src6",		1025,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src5",		1026,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src4",		1027,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src3",		1028,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src2",		1029,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src1",		1030,	MOD_CLK_ID(1017)),
	DEF_MOD("scu-src0",		1031,	MOD_CLK_ID(1017)),
};

/*
 * CPG Clock Data
 */

/*
 *   MD		EXTAL		PLL0	PLL1	PLL2	PLL3	PLL4
 * 14 13 19 17	(MHz)
 *-------------------------------------------------------------------
 * 0  0  0  0	16.66 x 1	x180	x192	x144	x192	x144
 * 0  0  0  1	16.66 x 1	x180	x192	x144	x128	x144
 * 0  0  1  0	Prohibited setting
 * 0  0  1  1	16.66 x 1	x180	x192	x144	x192	x144
 * 0  1  0  0	20    x 1	x150	x160	x120	x160	x120
 * 0  1  0  1	20    x 1	x150	x160	x120	x106	x120
 * 0  1  1  0	Prohibited setting
 * 0  1  1  1	20    x 1	x150	x160	x120	x160	x120
 * 1  0  0  0	25    x 1	x120	x128	x96	x128	x96
 * 1  0  0  1	25    x 1	x120	x128	x96	x84	x96
 * 1  0  1  0	Prohibited setting
 * 1  0  1  1	25    x 1	x120	x128	x96	x128	x96
 * 1  1  0  0	33.33 / 2	x180	x192	x144	x192	x144
 * 1  1  0  1	33.33 / 2	x180	x192	x144	x128	x144
 * 1  1  1  0	Prohibited setting
 * 1  1  1  1	33.33 / 2	x180	x192	x144	x192	x144
 */
#define CPG_PLL_CONFIG_INDEX(md)	((((md) & BIT(14)) >> 11) | \
					 (((md) & BIT(13)) >> 11) | \
					 (((md) & BIT(19)) >> 18) | \
					 (((md) & BIT(17)) >> 17))

static const struct rcar_gen3_cpg_pll_config cpg_pll_configs[16] = {
	/* EXTAL div	PLL1 mult	PLL3 mult */
	{ 1,		192,		192,	},
	{ 1,		192,		128,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		192,		192,	},
	{ 1,		160,		160,	},
	{ 1,		160,		106,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		160,		160,	},
	{ 1,		128,		128,	},
	{ 1,		128,		84,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		128,		128,	},
	{ 2,		192,		192,	},
	{ 2,		192,		128,	},
	{ 0, /* Prohibited setting */		},
	{ 2,		192,		192,	},
};

/*
 * SDn Clock
 */
#define CPG_SD_STP_HCK		BIT(9)
#define CPG_SD_STP_CK		BIT(8)

#define CPG_SD_STP_MASK		(CPG_SD_STP_HCK | CPG_SD_STP_CK)
#define CPG_SD_FC_MASK		(0x7 << 2 | 0x3 << 0)

#define CPG_SD_DIV_TABLE_DATA(stp_hck, stp_ck, sd_srcfc, sd_fc, sd_div) \
{ \
	.val = ((stp_hck) ? CPG_SD_STP_HCK : 0) | \
	       ((stp_ck) ? CPG_SD_STP_CK : 0) | \
	       ((sd_srcfc) << 2) | \
	       ((sd_fc) << 0), \
	.div = (sd_div), \
}

struct sd_div_table {
	u32 val;
	unsigned int div;
};

/* SDn divider
 *                     sd_srcfc   sd_fc   div
 * stp_hck   stp_ck    (div)      (div)     = sd_srcfc x sd_fc
 *-------------------------------------------------------------------
 *  0         0         0 (1)      1 (4)      4
 *  0         0         1 (2)      1 (4)      8
 *  1         0         2 (4)      1 (4)     16
 *  1         0         3 (8)      1 (4)     32
 *  1         0         4 (16)     1 (4)     64
 *  0         0         0 (1)      0 (2)      2
 *  0         0         1 (2)      0 (2)      4
 *  1         0         2 (4)      0 (2)      8
 *  1         0         3 (8)      0 (2)     16
 *  1         0         4 (16)     0 (2)     32
 */
static const struct sd_div_table cpg_sd_div_table[] = {
/*	CPG_SD_DIV_TABLE_DATA(stp_hck,  stp_ck,   sd_srcfc,   sd_fc,  sd_div) */
	CPG_SD_DIV_TABLE_DATA(0,        0,        0,          1,        4),
	CPG_SD_DIV_TABLE_DATA(0,        0,        1,          1,        8),
	CPG_SD_DIV_TABLE_DATA(1,        0,        2,          1,       16),
	CPG_SD_DIV_TABLE_DATA(1,        0,        3,          1,       32),
	CPG_SD_DIV_TABLE_DATA(1,        0,        4,          1,       64),
	CPG_SD_DIV_TABLE_DATA(0,        0,        0,          0,        2),
	CPG_SD_DIV_TABLE_DATA(0,        0,        1,          0,        4),
	CPG_SD_DIV_TABLE_DATA(1,        0,        2,          0,        8),
	CPG_SD_DIV_TABLE_DATA(1,        0,        3,          0,       16),
	CPG_SD_DIV_TABLE_DATA(1,        0,        4,          0,       32),
};

static bool gen3_clk_is_mod(struct clk *clk)
{
	return (clk->id >> 16) == CPG_MOD;
}

static int gen3_clk_get_mod(struct clk *clk, const struct mssr_mod_clk **mssr)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	if (!gen3_clk_is_mod(clk))
		return -EINVAL;

	for (i = 0; i < priv->mod_clk_size; i++) {
		if (priv->mod_clk[i].id != MOD_CLK_ID(clkid))
			continue;

		*mssr = &priv->mod_clk[i];
		return 0;
	}

	return -ENODEV;
}

static int gen3_clk_get_core(struct clk *clk, const struct cpg_core_clk **core)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	if (gen3_clk_is_mod(clk))
		return -EINVAL;

	for (i = 0; i < priv->core_clk_size; i++) {
		if (priv->core_clk[i].id != clkid)
			continue;

		*core = &priv->core_clk[i];
		return 0;
	}

	return -ENODEV;
}

static int gen3_clk_get_parent(struct clk *clk, struct clk *parent)
{
	const struct cpg_core_clk *core;
	const struct mssr_mod_clk *mssr;
	int ret;

	if (gen3_clk_is_mod(clk)) {
		ret = gen3_clk_get_mod(clk, &mssr);
		if (ret)
			return ret;

		parent->id = mssr->parent;
	} else {
		ret = gen3_clk_get_core(clk, &core);
		if (ret)
			return ret;

		if (core->type == CLK_TYPE_IN)
			parent->id = ~0;	/* Top-level clock */
		else
			parent->id = core->parent;
	}

	parent->dev = clk->dev;

	return 0;
}

static int gen3_clk_endisable(struct clk *clk, bool enable)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	const unsigned long clkid = clk->id & 0xffff;
	const unsigned int reg = clkid / 100;
	const unsigned int bit = clkid % 100;
	const u32 bitmask = BIT(bit);

	if (!gen3_clk_is_mod(clk))
		return -EINVAL;

	debug("%s[%i] MSTP %lu=%02u/%02u %s\n", __func__, __LINE__,
	      clkid, reg, bit, enable ? "ON" : "OFF");

	if (enable) {
		clrbits_le32(priv->base + SMSTPCR(reg), bitmask);
		return wait_for_bit("MSTP", priv->base + MSTPSR(reg),
				    bitmask, 0, 100, 0);
	} else {
		setbits_le32(priv->base + SMSTPCR(reg), bitmask);
		return 0;
	}
}

static int gen3_clk_enable(struct clk *clk)
{
	return gen3_clk_endisable(clk, true);
}

static int gen3_clk_disable(struct clk *clk)
{
	return gen3_clk_endisable(clk, false);
}

static ulong gen3_clk_get_rate(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk parent;
	const struct cpg_core_clk *core;
	const struct rcar_gen3_cpg_pll_config *pll_config =
					priv->cpg_pll_config;
	u32 value, mult, rate = 0;
	int i, ret;

	debug("%s[%i] Clock: id=%lu\n", __func__, __LINE__, clk->id);

	ret = gen3_clk_get_parent(clk, &parent);
	if (ret) {
		printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (gen3_clk_is_mod(clk)) {
		rate = gen3_clk_get_rate(&parent);
		debug("%s[%i] MOD clk: parent=%lu => rate=%u\n",
		      __func__, __LINE__, parent.id, rate);
		return rate;
	}

	ret = gen3_clk_get_core(clk, &core);
	if (ret)
		return ret;

	switch (core->type) {
	case CLK_TYPE_IN:
		if (core->id == CLK_EXTAL) {
			rate = clk_get_rate(&priv->clk_extal);
			debug("%s[%i] EXTAL clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		if (core->id == CLK_EXTALR) {
			rate = clk_get_rate(&priv->clk_extalr);
			debug("%s[%i] EXTALR clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		return -EINVAL;

	case CLK_TYPE_GEN3_MAIN:
		rate = gen3_clk_get_rate(&parent) / pll_config->extal_div;
		debug("%s[%i] MAIN clk: parent=%i extal_div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->extal_div, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL0:
		value = readl(priv->base + CPG_PLL0CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL0 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL1:
		rate = gen3_clk_get_rate(&parent) * pll_config->pll1_mult;
		debug("%s[%i] PLL1 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll1_mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL2:
		value = readl(priv->base + CPG_PLL2CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL2 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL3:
		rate = gen3_clk_get_rate(&parent) * pll_config->pll3_mult;
		debug("%s[%i] PLL3 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll3_mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL4:
		value = readl(priv->base + CPG_PLL4CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL4 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_FF:
		rate = (gen3_clk_get_rate(&parent) * core->mult) / core->div;
		debug("%s[%i] FIXED clk: parent=%i div=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, core->mult, core->div, rate);
		return rate;

	case CLK_TYPE_GEN3_SD:		/* FIXME */
		value = readl(priv->base + core->offset);
		value &= CPG_SD_STP_MASK | CPG_SD_FC_MASK;

		for (i = 0; i < ARRAY_SIZE(cpg_sd_div_table); i++) {
			if (cpg_sd_div_table[i].val != value)
				continue;

			rate = gen3_clk_get_rate(&parent) /
			       cpg_sd_div_table[i].div;
			debug("%s[%i] SD clk: parent=%i div=%i => rate=%u\n",
			      __func__, __LINE__,
			      core->parent, cpg_sd_div_table[i].div, rate);

			return rate;
		}

		return -EINVAL;
	}

	printf("%s[%i] unknown fail\n", __func__, __LINE__);

	return -ENOENT;
}

static ulong gen3_clk_set_rate(struct clk *clk, ulong rate)
{
	return gen3_clk_get_rate(clk);
}

static int gen3_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = (args->args[0] << 16) | args->args[1];

	return 0;
}

static const struct clk_ops gen3_clk_ops = {
	.enable		= gen3_clk_enable,
	.disable	= gen3_clk_disable,
	.get_rate	= gen3_clk_get_rate,
	.set_rate	= gen3_clk_set_rate,
	.of_xlate	= gen3_clk_of_xlate,
};

enum gen3_clk_model {
	CLK_R8A7795,
	CLK_R8A7796,
};

static int gen3_clk_probe(struct udevice *dev)
{
	struct gen3_clk_priv *priv = dev_get_priv(dev);
	enum gen3_clk_model model = dev_get_driver_data(dev);
	fdt_addr_t rst_base;
	u32 cpg_mode;
	int ret;

	priv->base = (struct gen3_base *)devfdt_get_addr(dev);
	if (!priv->base)
		return -EINVAL;

	switch (model) {
	case CLK_R8A7795:
		priv->core_clk = r8a7795_core_clks;
		priv->core_clk_size = ARRAY_SIZE(r8a7795_core_clks);
		priv->mod_clk = r8a7795_mod_clks;
		priv->mod_clk_size = ARRAY_SIZE(r8a7795_mod_clks);
		ret = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						    "renesas,r8a7795-rst");
		if (ret < 0)
			return ret;
		break;
	case CLK_R8A7796:
		priv->core_clk = r8a7796_core_clks;
		priv->core_clk_size = ARRAY_SIZE(r8a7796_core_clks);
		priv->mod_clk = r8a7796_mod_clks;
		priv->mod_clk_size = ARRAY_SIZE(r8a7796_mod_clks);
		ret = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						    "renesas,r8a7796-rst");
		if (ret < 0)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	rst_base = fdtdec_get_addr(gd->fdt_blob, ret, "reg");
	if (rst_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	cpg_mode = readl(rst_base + CPG_RST_MODEMR);

	priv->cpg_pll_config = &cpg_pll_configs[CPG_PLL_CONFIG_INDEX(cpg_mode)];
	if (!priv->cpg_pll_config->extal_div)
		return -EINVAL;

	ret = clk_get_by_name(dev, "extal", &priv->clk_extal);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "extalr", &priv->clk_extalr);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct udevice_id gen3_clk_ids[] = {
	{ .compatible = "renesas,r8a7795-cpg-mssr", .data = CLK_R8A7795 },
	{ .compatible = "renesas,r8a7796-cpg-mssr", .data = CLK_R8A7796 },
	{ }
};

U_BOOT_DRIVER(clk_gen3) = {
	.name		= "clk_gen3",
	.id		= UCLASS_CLK,
	.of_match	= gen3_clk_ids,
	.priv_auto_alloc_size = sizeof(struct gen3_clk_priv),
	.ops		= &gen3_clk_ops,
	.probe		= gen3_clk_probe,
};
