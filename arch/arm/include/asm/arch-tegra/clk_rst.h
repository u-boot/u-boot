/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _CLK_RST_H_
#define _CLK_RST_H_

/* PLL registers - there are several PLLs in the clock controller */
struct clk_pll {
	uint pll_base;		/* the control register */
	uint pll_out;		/* output control */
	uint reserved;
	uint pll_misc;		/* other misc things */
};

/* PLL registers - there are several PLLs in the clock controller */
struct clk_pll_simple {
	uint pll_base;		/* the control register */
	uint pll_misc;		/* other misc things */
};

/*
 * Most PLLs use the clk_pll structure, but some have a simpler two-member
 * structure for which we use clk_pll_simple. The reason for this non-
 * othogonal setup is not stated.
 */
enum {
	TEGRA_CLK_PLLS		= 6,	/* Number of normal PLLs */
	TEGRA_CLK_SIMPLE_PLLS	= 3,	/* Number of simple PLLs */
	TEGRA_CLK_REGS		= 3,	/* Number of clock enable registers */
	TEGRA_CLK_SOURCES	= 64,	/* Number of peripheral clock sources */
};

/* Clock/Reset Controller (CLK_RST_CONTROLLER_) regs */
struct clk_rst_ctlr {
	uint crc_rst_src;			/* _RST_SOURCE_0,0x00 */
	uint crc_rst_dev[TEGRA_CLK_REGS];	/* _RST_DEVICES_L/H/U_0 */
	uint crc_clk_out_enb[TEGRA_CLK_REGS];	/* _CLK_OUT_ENB_L/H/U_0 */
	uint crc_reserved0;		/* reserved_0,		0x1C */
	uint crc_cclk_brst_pol;		/* _CCLK_BURST_POLICY_0,0x20 */
	uint crc_super_cclk_div;	/* _SUPER_CCLK_DIVIDER_0,0x24 */
	uint crc_sclk_brst_pol;		/* _SCLK_BURST_POLICY_0, 0x28 */
	uint crc_super_sclk_div;	/* _SUPER_SCLK_DIVIDER_0,0x2C */
	uint crc_clk_sys_rate;		/* _CLK_SYSTEM_RATE_0,	0x30 */
	uint crc_prog_dly_clk;		/* _PROG_DLY_CLK_0,	0x34 */
	uint crc_aud_sync_clk_rate;	/* _AUDIO_SYNC_CLK_RATE_0,0x38 */
	uint crc_reserved1;		/* reserved_1,		0x3C */
	uint crc_cop_clk_skip_plcy;	/* _COP_CLK_SKIP_POLICY_0,0x40 */
	uint crc_clk_mask_arm;		/* _CLK_MASK_ARM_0,	0x44 */
	uint crc_misc_clk_enb;		/* _MISC_CLK_ENB_0,	0x48 */
	uint crc_clk_cpu_cmplx;		/* _CLK_CPU_CMPLX_0,	0x4C */
	uint crc_osc_ctrl;		/* _OSC_CTRL_0,		0x50 */
	uint crc_pll_lfsr;		/* _PLL_LFSR_0,		0x54 */
	uint crc_osc_freq_det;		/* _OSC_FREQ_DET_0,	0x58 */
	uint crc_osc_freq_det_stat;	/* _OSC_FREQ_DET_STATUS_0,0x5C */
	uint crc_reserved2[8];		/* reserved_2[8],	0x60-7C */

	struct clk_pll crc_pll[TEGRA_CLK_PLLS];	/* PLLs from 0x80 to 0xdc */

	/* PLLs from 0xe0 to 0xf4    */
	struct clk_pll_simple crc_pll_simple[TEGRA_CLK_SIMPLE_PLLS];

	uint crc_reserved10;		/* _reserved_10,	0xF8 */
	uint crc_reserved11;		/* _reserved_11,	0xFC */

	uint crc_clk_src[TEGRA_CLK_SOURCES]; /*_I2S1_0...	0x100-1fc */
	uint crc_reserved20[80];	/*			0x200-33C */
	uint crc_cpu_cmplx_set;		/* _CPU_CMPLX_SET_0,	0x340	  */
	uint crc_cpu_cmplx_clr;		/* _CPU_CMPLX_CLR_0,	0x344     */
};

/* CLK_RST_CONTROLLER_CLK_CPU_CMPLX_0 */
#define CPU1_CLK_STP_SHIFT	9

#define CPU0_CLK_STP_SHIFT	8
#define CPU0_CLK_STP_MASK	(1U << CPU0_CLK_STP_SHIFT)

/* CLK_RST_CONTROLLER_PLLx_BASE_0 */
#define PLL_BYPASS_SHIFT	31
#define PLL_BYPASS_MASK		(1U << PLL_BYPASS_SHIFT)

#define PLL_ENABLE_SHIFT	30
#define PLL_ENABLE_MASK		(1U << PLL_ENABLE_SHIFT)

#define PLL_BASE_OVRRIDE_MASK	(1U << 28)

#define PLL_DIVP_SHIFT		20
#define PLL_DIVP_MASK		(7U << PLL_DIVP_SHIFT)

#define PLL_DIVN_SHIFT		8
#define PLL_DIVN_MASK		(0x3ffU << PLL_DIVN_SHIFT)

#define PLL_DIVM_SHIFT		0
#define PLL_DIVM_MASK		(0x1f << PLL_DIVM_SHIFT)

/* CLK_RST_CONTROLLER_PLLx_MISC_0 */
#define PLL_CPCON_SHIFT		8
#define PLL_CPCON_MASK		(15U << PLL_CPCON_SHIFT)

#define PLL_LFCON_SHIFT		4
#define PLL_LFCON_MASK		(15U << PLL_LFCON_SHIFT)

#define PLLU_VCO_FREQ_SHIFT	20
#define PLLU_VCO_FREQ_MASK	(1U << PLLU_VCO_FREQ_SHIFT)

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT		30
#define OSC_FREQ_MASK		(3U << OSC_FREQ_SHIFT)
#define OSC_XOBP_SHIFT		1
#define OSC_XOBP_MASK		(1U << OSC_XOBP_SHIFT)

/*
 * CLK_RST_CONTROLLER_CLK_SOURCE_x_OUT_0 - the mask here is normally 8 bits
 * but can be 16. We could use knowledge we have to restrict the mask in
 * the 8-bit cases (the divider_bits value returned by
 * get_periph_clock_source()) but it does not seem worth it since the code
 * already checks the ranges of values it is writing, in clk_get_divider().
 */
#define OUT_CLK_DIVISOR_SHIFT	0
#define OUT_CLK_DIVISOR_MASK	(0xffff << OUT_CLK_DIVISOR_SHIFT)

#define OUT_CLK_SOURCE_SHIFT	30
#define OUT_CLK_SOURCE_MASK	(3U << OUT_CLK_SOURCE_SHIFT)

#define OUT_CLK_SOURCE4_SHIFT	28
#define OUT_CLK_SOURCE4_MASK	(15U << OUT_CLK_SOURCE4_SHIFT)

#endif	/* CLK_RST_H */
