// SPDX-License-Identifier: GPL-2.0+
/*
 * SAMA7D65 PMC clock support.
 *
 * Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Ryan Wanner <Ryan.Wanner@microchip.com>
 *
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clock/at91.h>
#include <linux/clk-provider.h>

#include "pmc.h"

/**
 * Clock identifiers to be used in conjunction with macros like
 * AT91_TO_CLK_ID()
 *
 * @ID_MD_SLCK:			TD slow clock identifier
 * @ID_TD_SLCK:			MD slow clock identifier
 * @ID_MAIN_XTAL:		Main Xtal clock identifier
 * @ID_MAIN_RC:			Main RC clock identifier
 * @ID_MAIN_RC_OSC:		Main RC Oscillator clock identifier
 * @ID_MAIN_OSC:		Main Oscillator clock identifier
 * @ID_MAINCK:			MAINCK clock identifier
 * @ID_PLL_CPU_FRAC:		CPU PLL fractional clock identifier
 * @ID_PLL_CPU_DIV:		CPU PLL divider clock identifier
 * @ID_PLL_SYS_FRAC:		SYS PLL fractional clock identifier
 * @ID_PLL_SYS_DIV:		SYS PLL divider clock identifier
 * @ID_PLL_DDR_FRAC:		DDR PLL fractional clock identifier
 * @ID_PLL_DDR_DIV:		DDR PLL divider clock identifier
 * @ID_PLL_GPU_FRAC:		GPU PLL fractional clock identifier
 * @ID_PLL_GPU_DIV:		GPU PLL divider clock identifier
 * @ID_PLL_BAUD_FRAC:		Baud PLL fractional clock identifier
 * @ID_PLL_BAUD_DIV:		Baud PLL divider clock identifier
 * @ID_PLL_AUDIO_FRAC:		Audio PLL fractional clock identifier
 * @ID_PLL_AUDIO_DIVPMC:	Audio PLL PMC divider clock identifier
 * @ID_PLL_AUDIO_DIVIO:		Audio PLL IO divider clock identifier
 * @ID_PLL_ETH_FRAC:		Ethernet PLL fractional clock identifier
 * @ID_PLL_ETH_DIV:		Ethernet PLL divider clock identifier
 * @ID_PLL_LVDS_FRAC:		LVDS PLL fractional clock identifier
 * @ID_PLL_LVDS_DIV:		LVDS PLL divider clock identifier
 * @ID_PLL_USB_FRAC:		USB PLL fractional clock identifier
 * @ID_PLL_USB_DIV:		USB PLL divider clock identifier

 * @ID_MCK0_PRES:		MCK0 PRES clock identifier
 * @ID_MCK0_DIV:		MCK0 DIV clock identifier
 * @ID_MCK1:			MCK1 clock identifier
 * @ID_MCK2:			MCK2 clock identifier
 * @ID_MCK3:			MCK3 clock identifier
 * @ID_MCK4:			MCK4 clock identifier

 * @ID_UTMI:			UTMI clock identifier

 * @ID_PROG0:			Programmable 0 clock identifier
 * @ID_PROG1:			Programmable 1 clock identifier
 * @ID_PROG2:			Programmable 2 clock identifier
 * @ID_PROG3:			Programmable 3 clock identifier
 * @ID_PROG4:			Programmable 4 clock identifier
 * @ID_PROG5:			Programmable 5 clock identifier
 * @ID_PROG6:			Programmable 6 clock identifier
 * @ID_PROG7:			Programmable 7 clock identifier

 * @ID_PCK0:			System clock 0 clock identifier
 * @ID_PCK1:			System clock 1 clock identifier
 * @ID_PCK2:			System clock 2 clock identifier
 * @ID_PCK3:			System clock 3 clock identifier
 * @ID_PCK4:			System clock 4 clock identifier
 * @ID_PCK5:			System clock 5 clock identifier
 * @ID_PCK6:			System clock 6 clock identifier
 * @ID_PCK7:			System clock 7 clock identifier
 */
enum pmc_clk_ids {
	ID_MD_SLCK		= 0,
	ID_TD_SLCK		= 1,
	ID_MAIN_XTAL		= 2,
	ID_MAIN_RC		= 3,
	ID_MAIN_RC_OSC		= 4,
	ID_MAIN_OSC		= 5,
	ID_MAINCK		= 6,

	ID_PLL_CPU_FRAC		= 7,
	ID_PLL_CPU_DIV		= 8,
	ID_PLL_SYS_FRAC		= 9,
	ID_PLL_SYS_DIV		= 10,
	ID_PLL_DDR_FRAC		= 11,
	ID_PLL_DDR_DIV		= 12,
	ID_PLL_GPU_FRAC		= 13,
	ID_PLL_GPU_DIV		= 14,
	ID_PLL_BAUD_FRAC	= 15,
	ID_PLL_BAUD_DIV		= 16,
	ID_PLL_AUDIO_FRAC	= 17,
	ID_PLL_AUDIO_DIVPMC	= 18,
	ID_PLL_AUDIO_DIVIO	= 19,
	ID_PLL_ETH_FRAC		= 20,
	ID_PLL_ETH_DIV		= 21,
	ID_PLL_LVDS_FRAC	= 22,
	ID_PLL_LVDS_DIV		= 23,
	ID_PLL_USB_FRAC		= 24,
	ID_PLL_USB_DIV		= 25,

	ID_MCK0_DIV		= 26,
	ID_MCK1			= 27,
	ID_MCK2			= 28,
	ID_MCK3			= 29,
	ID_MCK4			= 30,
	ID_MCK5			= 31,
	ID_MCK6			= 32,
	ID_MCK7			= 33,
	ID_MCK8			= 34,
	ID_MCK9			= 35,

	ID_UTMI			= 36,

	ID_PROG0		= 37,
	ID_PROG1		= 38,
	ID_PROG2		= 39,
	ID_PROG3		= 40,
	ID_PROG4		= 41,
	ID_PROG5		= 42,
	ID_PROG6		= 43,
	ID_PROG7		= 44,

	ID_PCK0			= 45,
	ID_PCK1			= 46,
	ID_PCK2			= 47,
	ID_PCK3			= 48,
	ID_PCK4			= 49,
	ID_PCK5			= 50,
	ID_PCK6			= 51,
	ID_PCK7			= 52,

	ID_MCK0_PRES		= 53,

	ID_MAX,
};

/**
 * PLL type identifiers
 * @PLL_TYPE_FRAC:	fractional PLL identifier
 * @PLL_TYPE_DIV:	divider PLL identifier
 */
enum pll_type {
	PLL_TYPE_FRAC,
	PLL_TYPE_DIV,
};

/* Clock names used as parents for multiple clocks. */
static const char *clk_names[] = {
	[ID_MAIN_RC]		= "main_rc",
	[ID_MAIN_RC_OSC]	= "main_rc_osc",
	[ID_MAIN_OSC]		= "main_osc",
	[ID_MAINCK]		= "mainck",
	[ID_PLL_CPU_DIV]	= "cpupll_divpmcck",
	[ID_PLL_SYS_DIV]	= "syspll_divpmcck",
	[ID_PLL_DDR_DIV]	= "ddrpll_divpmcck",
	[ID_PLL_GPU_DIV]	= "gpupll_divpmcck",
	[ID_PLL_BAUD_DIV]	= "baudpll_divpmcck",
	[ID_PLL_AUDIO_DIVPMC]	= "audiopll_divpmcck",
	[ID_PLL_AUDIO_DIVIO]	= "audiopll_diviock",
	[ID_PLL_ETH_DIV]	= "ethpll_divpmcck",
	[ID_PLL_LVDS_DIV]	= "lvdspll_divpmcck",
	[ID_PLL_USB_DIV]	= "usbpll_divpmcck",
	[ID_MCK0_DIV]		= "mck0_div",
	[ID_MCK0_PRES]		= "mck0_pres",
};

/* Fractional PLL output range. */
static const struct clk_range pll_outputs[] = {
	{ .min = 2343750, .max = 1200000000 },
};

/* Fractional PLL core output range. */
static const struct clk_range core_outputs[] = {
	{ .min = 600000000, .max = 1200000000 },
};

/* PLL characteristics. */
static const struct clk_pll_characteristics pll_characteristics = {
	.input = { .min = 12000000, .max = 50000000 },
	.num_output = ARRAY_SIZE(pll_outputs),
	.output = pll_outputs,
	.core_output = core_outputs,
};

/* Layout for fractional PLLs. */
static const struct clk_pll_layout pll_layout_frac = {
	.mul_mask	= GENMASK(31, 24),
	.frac_mask	= GENMASK(21, 0),
	.mul_shift	= 24,
	.frac_shift	= 0,
};

/* Layout for DIVPMC dividers. */
static const struct clk_pll_layout pll_layout_divpmc = {
	.div_mask	= GENMASK(7, 0),
	.endiv_mask	= BIT(29),
	.div_shift	= 0,
	.endiv_shift	= 29,
};

/* Layout for DIVIO dividers. */
static const struct clk_pll_layout pll_layout_divio = {
	.div_mask	= GENMASK(19, 12),
	.endiv_mask	= BIT(30),
	.div_shift	= 12,
	.endiv_shift	= 30,
};

/* MCK0 characteristics. */
static const struct clk_master_characteristics mck0_characteristics = {
	.output = { .min = 140000000, .max = 200000000 },
	.divisors = { 1, 2, 4, 3, 5 },
	.have_div3_pres = 1,
};

/* MCK0 layout. */
static const struct clk_master_layout mck0_layout = {
	.mask = 0x773,
	.pres_shift = 4,
	.offset = 0x28,
};

/* Programmable clock layout. */
static const struct clk_programmable_layout programmable_layout = {
	.pres_mask = 0xff,
	.pres_shift = 8,
	.css_mask = 0x1f,
	.have_slck_mck = 0,
	.is_pres_direct = 1,
};

/* Peripheral clock layout. */
static const struct clk_pcr_layout sama7d65_pcr_layout = {
	.offset = 0x88,
	.cmd = BIT(31),
	.gckcss_mask = GENMASK(12, 8),
	.pid_mask = GENMASK(6, 0),
};

/**
 * PLL clocks description
 * @n:		clock name
 * @p:		clock parent
 * @l:		clock layout
 * @t:		clock type
 * @c:		true if clock is critical and cannot be disabled
 * @id:		clock id corresponding to PLL driver
 * @cid:	clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	const struct clk_pll_layout *l;
	u8 t;
	u8 c;
	u8 id;
	u8 cid;
} sama7d65_plls[] = {
	{
		.n = "cpupll_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.c = 1,
		.id = 0,
		.cid = ID_PLL_CPU_FRAC,
	},

	{
		.n = "cpupll_divpmcck",
		.p = "cpupll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.c = 1,
		.id = 0,
		.cid = ID_PLL_CPU_DIV,
	},

	{
		.n = "syspll_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.c = 1,
		.id = 1,
		.cid = ID_PLL_SYS_FRAC,
	},

	{
		.n = "syspll_divpmcck",
		.p = "syspll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.c = 1,
		.id = 1,
		.cid = ID_PLL_SYS_DIV,
	},

	{
		.n = "ddrpll_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.c = 1,
		.id = 2,
		.cid = ID_PLL_DDR_FRAC,
	},

	{
		.n = "ddrpll_divpmcck",
		.p = "ddrpll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.c = 1,
		.id = 2,
		.cid = ID_PLL_DDR_DIV,
	},

	{
		.n = "gpupll_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 3,
		.cid = ID_PLL_GPU_FRAC,
	},

	{
		.n = "gpupll_divpmcck",
		.p = "gpupll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 3,
		.cid = ID_PLL_GPU_DIV
	},

	{
		.n = "baudpll_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 4,
		.cid = ID_PLL_BAUD_FRAC,
	},

	{
		.n = "baudpll_divpmcck",
		.p = "baudpll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 4,
		.cid = ID_PLL_BAUD_DIV,
	},

	{
		.n = "audiopll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 5,
		.cid = ID_PLL_AUDIO_FRAC,
	},

	{
		.n = "audiopll_divpmcck",
		.p = "audiopll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 5,
		.cid = ID_PLL_AUDIO_DIVPMC,
	},

	{
		.n = "audiopll_diviock",
		.p = "audiopll_fracck",
		.l = &pll_layout_divio,
		.t = PLL_TYPE_DIV,
		.id = 5,
		.cid = ID_PLL_AUDIO_DIVIO,
	},

	{
		.n = "ethpll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 6,
		.cid = ID_PLL_ETH_FRAC,
	},

	{
		.n = "ethpll_divpmcck",
		.p = "ethpll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 6,
		.cid = ID_PLL_ETH_DIV,
	},
	{
		.n = "lvdspll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 7,
		.cid = ID_PLL_LVDS_FRAC,
	},
	{
		.n = "lvdspll_divpmcck",
		.p = "lvdspll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 7,
		.cid = ID_PLL_LVDS_DIV,
	},
	{
		.n = "usbpll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.t = PLL_TYPE_FRAC,
		.id = 8,
		.cid = ID_PLL_USB_FRAC,
	},
	{
		.n = "usbpll_divmpcck",
		.p = "usbpll_fracck",
		.l = &pll_layout_divpmc,
		.t = PLL_TYPE_DIV,
		.id = 8,
		.cid = ID_PLL_USB_DIV,
	},
};

/**
 * Master clock (MCK[1..9]) description
 * @n:			clock name
 * @ep:			extra parents names array
 * @ep_chg_chg_id:	index in parents array that specifies the changeable
 *			parent
 * @ep_count:		extra parents count
 * @ep_mux_table:	mux table for extra parents
 * @ep_clk_mux_table:	mux table to deal with subsystem clock ids
 * @id:			clock id corresponding to MCK driver
 * @cid:		clock id corresponding to clock subsystem
 * @c:			true if clock is critical and cannot be disabled
 */
static const struct {
	const char *n;
	const char *ep[4];
	u8 ep_count;
	u8 ep_mux_table[4];
	u8 ep_clk_mux_table[4];
	u8 id;
	u8 cid;
	u8 c;
} sama7d65_mckx[] = {
	{
		.n = "mck1",
		.id = 1,
		.cid = ID_MCK1,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
		.c = 1,
	},

	{
		.n = "mck2",
		.id = 2,
		.cid = ID_MCK2,
		.ep = { "syspll_divpmcck", "ddrpll_divpmcck", },
		.ep_mux_table = { 5, 6, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_DDR_DIV, },
		.ep_count = 2,
		.c = 1,
	},

	{
		.n = "mck3",
		.id = 3,
		.cid = ID_MCK3,
		.ep = { "syspll_divpmcck", "ddrpll_divpmcck", },
		.ep_mux_table = { 5, 6, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_DDR_DIV, },
		.ep_count = 2,
	},

	{
		.n = "mck4",
		.id = 4,
		.cid = ID_MCK4,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
		.c = 1,
	},

	{
		.n = "mck5",
		.id = 5,
		.cid = ID_MCK5,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
	},

	{
		.n = "mck6",
		.id = 6,
		.cid = ID_MCK6,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
		.c = 1,
	},

	{
		.n = "mck7",
		.id = 7,
		.cid = ID_MCK7,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
		.c = 1,
	},

	{
		.n = "mck8",
		.id = 8,
		.cid = ID_MCK8,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
		.c = 1,
	},

	{
		.n = "mck9",
		.id = 9,
		.cid = ID_MCK9,
		.ep = { "syspll_divpmcck", },
		.ep_mux_table = { 5, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, },
		.ep_count = 1,
	},

};

/**
 * Programmable clock description
 * @n:			clock name
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	u8 cid;
} sama7d65_prog[] = {
	{ .n = "prog0", .cid = ID_PROG0, },
	{ .n = "prog1", .cid = ID_PROG1, },
	{ .n = "prog2", .cid = ID_PROG2, },
	{ .n = "prog3", .cid = ID_PROG3, },
	{ .n = "prog4", .cid = ID_PROG4, },
	{ .n = "prog5", .cid = ID_PROG5, },
	{ .n = "prog6", .cid = ID_PROG6, },
	{ .n = "prog7", .cid = ID_PROG7, },
};

/* Mux table for programmable clocks. */
static u32 sama7d65_prog_mux_table[] = { 0, 1, 2, 3, 5, 7, 8, 9, 10, 12, };

/**
 * System clock description
 * @n:			clock name
 * @p:			parent clock name
 * @id:			clock id corresponding to system clock driver
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	u8 id;
	u8 cid;
} sama7d65_systemck[] = {
	{ .n = "pck0", .p = "prog0", .id = 8, .cid = ID_PCK0, },
	{ .n = "pck1", .p = "prog1", .id = 9, .cid = ID_PCK1, },
	{ .n = "pck2", .p = "prog2", .id = 10, .cid = ID_PCK2, },
	{ .n = "pck3", .p = "prog3", .id = 11, .cid = ID_PCK3, },
	{ .n = "pck4", .p = "prog4", .id = 12, .cid = ID_PCK4, },
	{ .n = "pck5", .p = "prog5", .id = 13, .cid = ID_PCK5, },
	{ .n = "pck6", .p = "prog6", .id = 14, .cid = ID_PCK6, },
	{ .n = "pck7", .p = "prog7", .id = 15, .cid = ID_PCK7, },
};

/**
 * Peripheral clock description
 * @n:		clock name
 * @p:		clock parent name
 * @r:		clock range values
 * @id:		clock id
 */
static const struct {
	const char *n;
	const char *p;
	struct clk_range r;
	u8 id;
} sama7d65_periphck[] = {
	{ .n = "pioA_clk",	.p = "mck0_div", .id = 10, },
	{ .n = "sfr_clk",	.p = "mck7", .id = 18, },
	{ .n = "hsmc_clk",	.p = "mck5", .id = 20, },
	{ .n = "xdmac0_clk",	.p = "mck6", .id = 21, },
	{ .n = "xdmac1_clk",	.p = "mck6", .id = 22, },
	{ .n = "xdmac2_clk",	.p = "mck1", .id = 23, },
	{ .n = "acc_clk",	.p = "mck7", .id = 24, },
	{ .n = "aes_clk",	.p = "mck6", .id = 26, },
	{ .n = "tzaesbasc_clk",	.p = "mck8", .id = 27, },
	{ .n = "asrc_clk",	.p = "mck9", .id = 29, .r = { .max = 200000000, }, },
	{ .n = "cpkcc_clk",	.p = "mck0_div", .id = 30, },
	{ .n = "eic_clk",	.p = "mck7", .id = 33, },
	{ .n = "flex0_clk",	.p = "mck7", .id = 34, },
	{ .n = "flex1_clk",	.p = "mck7", .id = 35, },
	{ .n = "flex2_clk",	.p = "mck7", .id = 36, },
	{ .n = "flex3_clk",	.p = "mck7", .id = 37, },
	{ .n = "flex4_clk",	.p = "mck8", .id = 38, },
	{ .n = "flex5_clk",	.p = "mck8", .id = 39, },
	{ .n = "flex6_clk",	.p = "mck8", .id = 40, },
	{ .n = "flex7_clk",	.p = "mck8", .id = 41, },
	{ .n = "flex8_clk",	.p = "mck9", .id = 42, },
	{ .n = "flex9_clk",	.p = "mck9", .id = 43, },
	{ .n = "flex10_clk",	.p = "mck9", .id = 44, },
	{ .n = "gmac0_clk",	.p = "mck6", .id = 46, },
	{ .n = "gmac1_clk",	.p = "mck6", .id = 47, },
	{ .n = "gmac0_tsu_clk",	.p = "mck1", .id = 49, },
	{ .n = "gmac1_tsu_clk",	.p = "mck1", .id = 50, },
	{ .n = "icm_clk",	.p = "mck5", .id = 53, },
	{ .n = "i2smcc0_clk",	.p = "mck9", .id = 54, .r = { .max = 200000000, }, },
	{ .n = "i2smcc1_clk",	.p = "mck9", .id = 55, .r = { .max = 200000000, }, },
	{ .n = "matrix_clk",	.p = "mck5", .id = 57, },
	{ .n = "mcan0_clk",	.p = "mck5", .id = 58, .r = { .max = 200000000, }, },
	{ .n = "mcan1_clk",	.p = "mck5", .id = 59, .r = { .max = 200000000, }, },
	{ .n = "mcan2_clk",	.p = "mck5", .id = 60, .r = { .max = 200000000, }, },
	{ .n = "mcan3_clk",	.p = "mck5", .id = 61, .r = { .max = 200000000, }, },
	{ .n = "mcan4_clk",	.p = "mck5", .id = 62, .r = { .max = 200000000, }, },
	{ .n = "pdmc0_clk",	.p = "mck9", .id = 64, .r = { .max = 200000000, }, },
	{ .n = "pdmc1_clk",	.p = "mck9", .id = 65, .r = { .max = 200000000, }, },
	{ .n = "pit64b0_clk",	.p = "mck7", .id = 66, },
	{ .n = "pit64b1_clk",	.p = "mck7", .id = 67, },
	{ .n = "pit64b2_clk",	.p = "mck7", .id = 68, },
	{ .n = "pit64b3_clk",	.p = "mck8", .id = 69, },
	{ .n = "pit64b4_clk",	.p = "mck8", .id = 70, },
	{ .n = "pit64b5_clk",	.p = "mck8", .id = 71, },
	{ .n = "pwm_clk",	.p = "mck7", .id = 72, },
	{ .n = "qspi0_clk",	.p = "mck5", .id = 73, },
	{ .n = "qspi1_clk",	.p = "mck5", .id = 74, },
	{ .n = "sdmmc0_clk",	.p = "mck1", .id = 75, },
	{ .n = "sdmmc1_clk",	.p = "mck1", .id = 76, },
	{ .n = "sdmmc2_clk",	.p = "mck1", .id = 77, },
	{ .n = "sha_clk",	.p = "mck6", .id = 78, },
	{ .n = "spdifrx_clk",	.p = "mck9", .id = 79, .r = { .max = 200000000, }, },
	{ .n = "spdiftx_clk",	.p = "mck9", .id = 80, .r = { .max = 200000000, }, },
	{ .n = "ssc0_clk",	.p = "mck7", .id = 81, .r = { .max = 200000000, }, },
	{ .n = "ssc1_clk",	.p = "mck8", .id = 82, .r = { .max = 200000000, }, },
	{ .n = "tcb0_ch0_clk",	.p = "mck8", .id = 83, .r = { .max = 200000000, }, },
	{ .n = "tcb0_ch1_clk",	.p = "mck8", .id = 84, .r = { .max = 200000000, }, },
	{ .n = "tcb0_ch2_clk",	.p = "mck8", .id = 85, .r = { .max = 200000000, }, },
	{ .n = "tcb1_ch0_clk",	.p = "mck5", .id = 86, .r = { .max = 200000000, }, },
	{ .n = "tcb1_ch1_clk",	.p = "mck5", .id = 87, .r = { .max = 200000000, }, },
	{ .n = "tcb1_ch2_clk",	.p = "mck5", .id = 88, .r = { .max = 200000000, }, },
	{ .n = "tcpca_clk",	.p = "mck5", .id = 89, },
	{ .n = "tcpcb_clk",	.p = "mck5", .id = 90, },
	{ .n = "tdes_clk",	.p = "mck6", .id = 91, },
	{ .n = "trng_clk",	.p = "mck6", .id = 92, },
	{ .n = "udphsa_clk",	.p = "mck5", .id = 99, },
	{ .n = "udphsb_clk",	.p = "mck5", .id = 100, },
	{ .n = "uhphs_clk",	.p = "mck5", .id = 101, },
};

/**
 * Generic clock description
 * @n:			clock name
 * @ep:			extra parents names
 * @ep_mux_table:	extra parents mux table
 * @ep_clk_mux_table:	extra parents clock mux table (for CCF)
 * @r:			clock output range
 * @ep_count:		extra parents count
 * @id:			clock id
 */
static const struct {
	const char *n;
	const char *ep[8];
	const char ep_mux_table[8];
	const char ep_clk_mux_table[8];
	struct clk_range r;
	u8 ep_count;
	u8 id;
} sama7d65_gck[] = {
	{
		.n  = "adc_gclk",
		.id = 25,
		.r = { .max = 100000000, },
		.ep = { "baudpll_divpmcck", "audiopll_divpmcck", },
		.ep_mux_table = { 8, 9, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 2,
	},

	{
		.n  = "asrc_gclk",
		.id = 29,
		.r = { .max = 200000000 },
		.ep = { "audiopll_divpmcck", },
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "flex0_gclk",
		.id = 34,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex1_gclk",
		.id = 35,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex2_gclk",
		.id = 36,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8,},
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex3_gclk",
		.id = 37,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", },
		.ep_mux_table = { 8,},
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex4_gclk",
		.id = 38,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex5_gclk",
		.id = 39,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex6_gclk",
		.id = 40,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex7_gclk",
		.id = 41,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex8_gclk",
		.id = 42,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8,},
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex9_gclk",
		.id = 43,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8,},
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "flex10_gclk",
		.id = 44,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", },
		.ep_mux_table = { 8,},
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "gmac0_gclk",
		.id = 46,
		.r = { .max = 125000000},
		.ep = { "ethpll_divpmcck", },
		.ep_clk_mux_table = { ID_PLL_ETH_DIV, },
		.ep_mux_table = { 10, },
		.ep_count = 1,
	},

	{
		.n  = "gmac1_gclk",
		.id = 47,
		.r = { .max = 125000000},
		.ep = { "ethpll_divpmcck", },
		.ep_mux_table = { 10, },
		.ep_clk_mux_table = { ID_PLL_ETH_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "gmac0_tsu_gclk",
		.id = 49,
		.r = { .max = 400000000 },
		.ep = { "syspll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = { 5, 9, 10, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_AUDIO_DIVPMC, ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "gmac1_tsu_gclk",
		.id = 50,
		.r = { .max = 400000000 },
		.ep = { "syspll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = { 5, 9, 10, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_AUDIO_DIVPMC, ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "i2smcc0_gclk",
		.id = 54,
		.r = { .max = 100000000 },
		.ep = { "audiopll_divpmcck", },
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = {ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "i2smcc1_gclk",
		.id = 55,
		.r = { .max = 100000000 },
		.ep = {"audiopll_divpmcck", },
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = {ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "mcan0_gclk",
		.id = 58,
		.r = { .max = 80000000 },
		.ep = { "usbpll_divpmcck", },
		.ep_mux_table = { 12, },
		.ep_clk_mux_table = { ID_PLL_USB_DIV },
		.ep_count = 1,
	},

	{
		.n  = "mcan1_gclk",
		.id = 59,
		.r = { .max = 80000000 },
		.ep = { "usbpll_divpmcck", },
		.ep_mux_table = { 12, },
		.ep_clk_mux_table = { ID_PLL_USB_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "mcan2_gclk",
		.id = 60,
		.r = { .max = 80000000 },
		.ep = { "usbpll_divpmcck", },
		.ep_mux_table = { 12, },
		.ep_clk_mux_table = { ID_PLL_USB_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "mcan3_gclk",
		.id = 61,
		.r = { .max = 80000000 },
		.ep = { "usbpll_divpmcck", },
		.ep_mux_table = { 12, },
		.ep_clk_mux_table = { ID_PLL_USB_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "mcan4_gclk",
		.id = 62,
		.r = { .max = 80000000 },
		.ep = { "usbpll_divpmcck", },
		.ep_mux_table = { 12, },
		.ep_clk_mux_table = { ID_PLL_USB_DIV, },
		.ep_count = 1,
	},

	{
		.n  = "pdmc0_gclk",
		.id = 64,
		.r = { .max = 80000000 },
		.ep = { "audiopll_divpmcck", },
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "pdmc1_gclk",
		.id = 65,
		.r = { .max = 80000000, },
		.ep = { "audiopll_divpmcck",},
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = {ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "pit64b0_gclk",
		.id = 66,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "pit64b1_gclk",
		.id = 67,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = { ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "pit64b2_gclk",
		.id = 68,
		.r = { .max = 34000000 },
		.ep = { "baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = { 8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "pit64b3_gclk",
		.id = 69,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "pit64b4_gclk",
		.id = 70,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "pit64b5_gclk",
		.id = 71,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "qspi0_gclk",
		.id = 73,
		.r = { .max = 400000000 },
		.ep = { "syspll_divpmcck", "baudpll_divpmcck", },
		.ep_mux_table = { 5, 8, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_BAUD_DIV, },
		.ep_count = 2,
	},

	{
		.n  = "qspi1_gclk",
		.id = 74,
		.r = { .max = 266000000 },
		.ep = { "syspll_divpmcck", "baudpll_divpmcck", },
		.ep_mux_table = { 5, 8, },
		.ep_clk_mux_table = { ID_PLL_SYS_DIV, ID_PLL_BAUD_DIV, },
		.ep_count = 2,
	},

	{
		.n  = "sdmmc0_gclk",
		.id = 75,
		.r = { .max = 208000000 },
		.ep = {"baudpll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_ETH_DIV,},
		.ep_count = 2,
	},

	{
		.n  = "sdmmc1_gclk",
		.id = 76,
		.r = { .max = 208000000 },
		.ep = { "baudpll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 10,},
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_ETH_DIV, },
		.ep_count = 2,
	},

	{
		.n  = "sdmmc2_gclk",
		.id = 77,
		.r = { .max = 208000000 },
		.ep = {"baudpll_divpmcck", "ethpll_divpmcck",},
		.ep_mux_table = {8, 10,},
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_ETH_DIV,},
		.ep_count = 2,
	},

	{
		.n  = "spdifrx_gclk",
		.id = 79,
		.r = { .max = 150000000 },
		.ep = {"audiopll_divpmcck", },
		.ep_mux_table = { 9, },
		.ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n = "spdiftx_gclk",
		.id = 80,
		.r = { .max = 25000000  },
		.ep = { "audiopll_divpmcck", },
		.ep_mux_table = {9, },
		.ep_clk_mux_table = {ID_PLL_AUDIO_DIVPMC, },
		.ep_count = 1,
	},

	{
		.n  = "tcb0_ch0_gclk",
		.id = 83,
		.r = { .max = 34000000 },
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = { 8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n  = "tcb1_ch0_gclk",
		.id = 86,
		.r = { .max = 67000000 },
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},

	{
		.n = "DSI_gclk",
		.id = 103,
		.r = {.max = 27000000},
		.ep = {"syspll_divpmcck"},
		.ep_mux_table = {5},
		.ep_clk_mux_table = {ID_PLL_SYS_DIV},
		.ep_count = 1,
	},

	{
		.n = "I3CC_gclk",
		.id = 105,
		.r = {.max = 125000000},
		.ep = {"baudpll_divpmcck", "audiopll_divpmcck", "ethpll_divpmcck", },
		.ep_mux_table = {8, 9, 10, },
		.ep_clk_mux_table = {ID_PLL_BAUD_DIV, ID_PLL_AUDIO_DIVPMC,
				      ID_PLL_ETH_DIV, },
		.ep_count = 3,
	},
};

/* Clock setup description */
static const struct pmc_clk_setup sama7d65_clk_setup[] = {
	{
		.cid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_ETH_FRAC),
		.rate = 625000000,
	},

	{
		.cid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_ETH_DIV),
		.rate = 625000000,
	},
};

#define SAMA7D65_MAX_MUX_ALLOCS		(64)

#define prepare_mux_table(_allocs, _index, _dst, _src, _num, _label)	\
	do {								\
		int _i;							\
		if ((_index) >= SAMA7D65_MAX_MUX_ALLOCS) {		\
			debug("%s(): AT91: MUX: insufficient space\n",	\
			      __func__);				\
			goto _label;					\
		}							\
		(_dst) = kzalloc(sizeof(*(_dst)) * (_num), GFP_KERNEL);	\
		if (!(_dst))						\
			goto _label;					\
		(_allocs)[(_index)++] = (_dst);				\
		for (_i = 0; _i < (_num); _i++)				\
			(_dst)[_i] = (_src)[_i];			\
	} while (0)

static int sama7d65_clk_probe(struct udevice *dev)
{
	void __iomem *base = (void *)devfdt_get_addr(dev);
	unsigned int *clkmuxallocs[SAMA7D65_MAX_MUX_ALLOCS];
	unsigned int *muxallocs[SAMA7D65_MAX_MUX_ALLOCS];
	const char *p[12];
	unsigned int cm[12], m[12], *tmpclkmux, *tmpmux;
	struct clk clk, *c;
	bool main_osc_bypass;
	int ret, muxallocindex = 0, clkmuxallocindex = 0, i, j;

	if (IS_ERR(base))
		return PTR_ERR(base);

	memset(muxallocs,    0, ARRAY_SIZE(muxallocs));
	memset(clkmuxallocs, 0, ARRAY_SIZE(clkmuxallocs));

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;
	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;
	clk_names[ID_TD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_TD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;
	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;
	clk_names[ID_MD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_MD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 2, &clk);
	if (ret)
		return ret;
	clk_names[ID_MAIN_XTAL] = kmemdup(clk_hw_get_name(&clk),
					  strlen(clk_hw_get_name(&clk)) + 1,
					  GFP_KERNEL);
	if (!clk_names[ID_MAIN_XTAL])
		return -ENOMEM;

	main_osc_bypass = dev_read_bool(dev, "atmel,main-osc-bypass");

	/* Register main rc oscillator. */
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC),
	       at91_clk_main_rc(base, clk_names[ID_MAIN_RC_OSC],
				NULL));

	/* Register main oscillator. */
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC),
	       at91_clk_main_osc(base, clk_names[ID_MAIN_OSC],
				 clk_names[ID_MAIN_XTAL], main_osc_bypass));

	/* Register mainck. */
	p[0] = clk_names[ID_MAIN_RC_OSC];
	p[1] = clk_names[ID_MAIN_OSC];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 2,
			  fail);
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK),
	       at91_clk_sam9x5_main(base, clk_names[ID_MAINCK], p, 2,
				    tmpclkmux, PMC_TYPE_CORE));

	/* Register PLL fracs clocks. */
	for (i = 0; i < ARRAY_SIZE(sama7d65_plls); i++) {
		if (sama7d65_plls[i].t != PLL_TYPE_FRAC)
			continue;

		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sama7d65_plls[i].cid),
		       sam9x60_clk_register_frac_pll(base, sama7d65_plls[i].n,
						     sama7d65_plls[i].p,
						     sama7d65_plls[i].id,
						     &pll_characteristics,
						     sama7d65_plls[i].l,
						     sama7d65_plls[i].c));
	}

	/* Register PLL div clocks. */
	for (i = 0; i < ARRAY_SIZE(sama7d65_plls); i++) {
		if (sama7d65_plls[i].t != PLL_TYPE_DIV)
			continue;

		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sama7d65_plls[i].cid),
		       sam9x60_clk_register_div_pll(base, sama7d65_plls[i].n,
						    sama7d65_plls[i].p,
						    sama7d65_plls[i].id,
						    &pll_characteristics,
						    sama7d65_plls[i].l,
						    sama7d65_plls[i].c));
	}

	/* Register MCK0_PRES clock. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_MAINCK];
	p[2] = clk_names[ID_PLL_CPU_DIV];
	p[3] = clk_names[ID_PLL_SYS_DIV];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_CPU_DIV);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_SYS_DIV);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 2,
			  fail);
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK0_PRES),
	       at91_clk_register_master_pres(base, clk_names[ID_MCK0_PRES],
					     p, 4, &mck0_layout,
					     &mck0_characteristics,
					     tmpclkmux));

	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK0_DIV),
	       at91_clk_register_master_div(base, clk_names[ID_MCK0_DIV],
					    clk_names[ID_MCK0_PRES],
					    &mck0_layout,
					    &mck0_characteristics));

	/* Register MCK1-9 clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK0_DIV];
	m[0] = 0;
	m[1] = 1;
	m[2] = 2;
	m[3] = 3;
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK0_DIV);
	for (i = 0; i < ARRAY_SIZE(sama7d65_mckx); i++) {
		for (j = 0; j < sama7d65_mckx[i].ep_count; j++) {
			p[4 + j] = sama7d65_mckx[i].ep[j];
			m[4 + j] = sama7d65_mckx[i].ep_mux_table[j];
			cm[4 + j] = AT91_TO_CLK_ID(PMC_TYPE_CORE,
						   sama7d65_mckx[i].ep_clk_mux_table[j]);
		}

		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  4 + sama7d65_mckx[i].ep_count, fail);
		prepare_mux_table(muxallocs, muxallocindex, tmpmux, m,
				  4 + sama7d65_mckx[i].ep_count, fail);

		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sama7d65_mckx[i].cid),
		       at91_clk_sama7g5_register_master(base, sama7d65_mckx[i].n, p,
							4 + sama7d65_mckx[i].ep_count,
							tmpmux, tmpclkmux,
							sama7d65_mckx[i].c,
							sama7d65_mckx[i].id));
	}

	/* Register programmable clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK0_DIV];
	p[4] = clk_names[ID_PLL_SYS_DIV];
	p[5] = clk_names[ID_PLL_GPU_DIV];
	p[6] = clk_names[ID_PLL_BAUD_DIV];
	p[7] = clk_names[ID_PLL_AUDIO_DIVPMC];
	p[8] = clk_names[ID_PLL_ETH_DIV];
	p[9] = clk_names[ID_PLL_USB_DIV];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK0_DIV);
	cm[4] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_SYS_DIV);
	cm[5] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_GPU_DIV);
	cm[6] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_BAUD_DIV);
	cm[7] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_AUDIO_DIVPMC);
	cm[8] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_ETH_DIV);
	cm[9] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_USB_DIV);

	for (i = 0; i < ARRAY_SIZE(sama7d65_prog); i++) {
		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  10, fail);

		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sama7d65_prog[i].cid),
		       at91_clk_register_programmable(base, sama7d65_prog[i].n,
						      p, 10, i,
						      &programmable_layout,
						      tmpclkmux,
						      sama7d65_prog_mux_table));
	}

	/* System clocks. */
	for (i = 0; i < ARRAY_SIZE(sama7d65_systemck); i++) {
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_SYSTEM, sama7d65_systemck[i].cid),
		       at91_clk_register_system(base, sama7d65_systemck[i].n,
						sama7d65_systemck[i].p,
						sama7d65_systemck[i].id));
	}

	/* Peripheral clocks. */
	for (i = 0; i < ARRAY_SIZE(sama7d65_periphck); i++) {
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_PERIPHERAL, sama7d65_periphck[i].id),
		       at91_clk_register_sam9x5_peripheral(base,
							   &sama7d65_pcr_layout,
							   sama7d65_periphck[i].n,
							   sama7d65_periphck[i].p,
							   sama7d65_periphck[i].id,
							   &sama7d65_periphck[i].r));
	}

	/* Generic clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK1];
	m[0] = 0;
	m[1] = 1;
	m[2] = 2;
	m[3] = 3;
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK1);
	for (i = 0; i < ARRAY_SIZE(sama7d65_gck); i++) {
		for (j = 0; j < sama7d65_gck[i].ep_count; j++) {
			p[4 + j] = sama7d65_gck[i].ep[j];
			m[4 + j] = sama7d65_gck[i].ep_mux_table[j];
			cm[4 + j] = AT91_TO_CLK_ID(PMC_TYPE_CORE,
						   sama7d65_gck[i].ep_clk_mux_table[j]);
		}

		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  4 + sama7d65_gck[i].ep_count, fail);
		prepare_mux_table(muxallocs, muxallocindex, tmpmux, m,
				  4 + sama7d65_gck[i].ep_count, fail);

		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_GCK, sama7d65_gck[i].id),
		       at91_clk_register_generic(base, &sama7d65_pcr_layout,
						 sama7d65_gck[i].n, p,
						 tmpclkmux, tmpmux,
						 4 + sama7d65_gck[i].ep_count,
						 sama7d65_gck[i].id,
						 &sama7d65_gck[i].r));
	}

	/* Setup clocks. */
	ret = at91_clk_setup(sama7d65_clk_setup, ARRAY_SIZE(sama7d65_clk_setup));
	if (ret)
		goto fail;

	return 0;

fail:
	for (i = 0; i < ARRAY_SIZE(muxallocs); i++)
		kfree(muxallocs[i]);

	for (i = 0; i < ARRAY_SIZE(clkmuxallocs); i++)
		kfree(clkmuxallocs[i]);

	return -ENOMEM;
}

static const struct udevice_id sama7d65_clk_ids[] = {
	{ .compatible = "microchip,sama7d65-pmc" },
	{ /* Sentinel. */ },
};

U_BOOT_DRIVER(at91_sama7d65_pmc) = {
	.name = "at91-sama7d65-pmc",
	.id = UCLASS_CLK,
	.of_match = sama7d65_clk_ids,
	.ops = &at91_clk_ops,
	.probe = sama7d65_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
