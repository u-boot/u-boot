/*
 * Keystone2: get clk rate for K2HK
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>

const struct keystone_pll_regs keystone_pll_regs[] = {
	[CORE_PLL]	= {KS2_MAINPLLCTL0, KS2_MAINPLLCTL1},
	[PASS_PLL]	= {KS2_PASSPLLCTL0, KS2_PASSPLLCTL1},
	[TETRIS_PLL]	= {KS2_ARMPLLCTL0, KS2_ARMPLLCTL1},
	[DDR3A_PLL]	= {KS2_DDR3APLLCTL0, KS2_DDR3APLLCTL1},
	[DDR3B_PLL]	= {KS2_DDR3BPLLCTL0, KS2_DDR3BPLLCTL1},
};

int dev_speeds[] = {
	SPD800,
	SPD1000,
	SPD1200,
	SPD800,
	SPD800,
	SPD800,
	SPD800,
	SPD800,
	SPD1200,
	SPD1000,
	SPD800,
	SPD800,
	SPD800,
};

int arm_speeds[] = {
	SPD800,
	SPD1000,
	SPD1200,
	SPD1350,
	SPD1400,
	SPD800,
	SPD1400,
	SPD1350,
	SPD1200,
	SPD1000,
	SPD800,
	SPD800,
	SPD800,
};

/**
 * pll_freq_get - get pll frequency
 * Fout = Fref * NF(mult) / NR(prediv) / OD
 * @pll:	pll identifier
 */
static unsigned long pll_freq_get(int pll)
{
	unsigned long mult = 1, prediv = 1, output_div = 2;
	unsigned long ret;
	u32 tmp, reg;

	if (pll == CORE_PLL) {
		ret = external_clk[sys_clk];
		if (pllctl_reg_read(pll, ctl) & PLLCTL_PLLEN) {
			/* PLL mode */
			tmp = __raw_readl(KS2_MAINPLLCTL0);
			prediv = (tmp & PLL_DIV_MASK) + 1;
			mult = (((tmp & PLLM_MULT_HI_SMASK) >> 6) |
				(pllctl_reg_read(pll, mult) &
				 PLLM_MULT_LO_MASK)) + 1;
			output_div = ((pllctl_reg_read(pll, secctl) >>
				       PLL_CLKOD_SHIFT) & PLL_CLKOD_MASK) + 1;

			ret = ret / prediv / output_div * mult;
		}
	} else {
		switch (pll) {
		case PASS_PLL:
			ret = external_clk[pa_clk];
			reg = KS2_PASSPLLCTL0;
			break;
		case TETRIS_PLL:
			ret = external_clk[tetris_clk];
			reg = KS2_ARMPLLCTL0;
			break;
		case DDR3A_PLL:
			ret = external_clk[ddr3a_clk];
			reg = KS2_DDR3APLLCTL0;
			break;
		case DDR3B_PLL:
			ret = external_clk[ddr3b_clk];
			reg = KS2_DDR3BPLLCTL0;
			break;
		default:
			return 0;
		}

		tmp = __raw_readl(reg);

		if (!(tmp & PLLCTL_BYPASS)) {
			/* Bypass disabled */
			prediv = (tmp & PLL_DIV_MASK) + 1;
			mult = ((tmp >> PLL_MULT_SHIFT) & PLL_MULT_MASK) + 1;
			output_div = ((tmp >> PLL_CLKOD_SHIFT) &
				      PLL_CLKOD_MASK) + 1;
			ret = ((ret / prediv) * mult) / output_div;
		}
	}

	return ret;
}

unsigned long clk_get_rate(unsigned int clk)
{
	switch (clk) {
	case core_pll_clk:	return pll_freq_get(CORE_PLL);
	case pass_pll_clk:	return pll_freq_get(PASS_PLL);
	case tetris_pll_clk:	return pll_freq_get(TETRIS_PLL);
	case ddr3a_pll_clk:	return pll_freq_get(DDR3A_PLL);
	case ddr3b_pll_clk:	return pll_freq_get(DDR3B_PLL);
	case sys_clk0_1_clk:
	case sys_clk0_clk:	return pll_freq_get(CORE_PLL) / pll0div_read(1);
	case sys_clk1_clk:	return pll_freq_get(CORE_PLL) / pll0div_read(2);
	case sys_clk2_clk:	return pll_freq_get(CORE_PLL) / pll0div_read(3);
	case sys_clk3_clk:	return pll_freq_get(CORE_PLL) / pll0div_read(4);
	case sys_clk0_2_clk:	return clk_get_rate(sys_clk0_clk) / 2;
	case sys_clk0_3_clk:	return clk_get_rate(sys_clk0_clk) / 3;
	case sys_clk0_4_clk:	return clk_get_rate(sys_clk0_clk) / 4;
	case sys_clk0_6_clk:	return clk_get_rate(sys_clk0_clk) / 6;
	case sys_clk0_8_clk:	return clk_get_rate(sys_clk0_clk) / 8;
	case sys_clk0_12_clk:	return clk_get_rate(sys_clk0_clk) / 12;
	case sys_clk0_24_clk:	return clk_get_rate(sys_clk0_clk) / 24;
	case sys_clk1_3_clk:	return clk_get_rate(sys_clk1_clk) / 3;
	case sys_clk1_4_clk:	return clk_get_rate(sys_clk1_clk) / 4;
	case sys_clk1_6_clk:	return clk_get_rate(sys_clk1_clk) / 6;
	case sys_clk1_12_clk:	return clk_get_rate(sys_clk1_clk) / 12;
	default:
		break;
	}

	return 0;
}
