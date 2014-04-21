/*
 * Keystone2: pll initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm-generic/errno.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>

static void wait_for_completion(const struct pll_init_data *data)
{
	int i;
	for (i = 0; i < 100; i++) {
		sdelay(450);
		if ((pllctl_reg_read(data->pll, stat) & PLLSTAT_GO) == 0)
			break;
	}
}

struct pll_regs {
	u32	reg0, reg1;
};

static const struct pll_regs pll_regs[] = {
	[CORE_PLL]	= { K2HK_MAINPLLCTL0, K2HK_MAINPLLCTL1},
	[PASS_PLL]	= { K2HK_PASSPLLCTL0, K2HK_PASSPLLCTL1},
	[TETRIS_PLL]	= { K2HK_ARMPLLCTL0,  K2HK_ARMPLLCTL1},
	[DDR3A_PLL]	= { K2HK_DDR3APLLCTL0, K2HK_DDR3APLLCTL1},
	[DDR3B_PLL]	= { K2HK_DDR3BPLLCTL0, K2HK_DDR3BPLLCTL1},
};

/* Fout = Fref * NF(mult) / NR(prediv) / OD */
static unsigned long pll_freq_get(int pll)
{
	unsigned long mult = 1, prediv = 1, output_div = 2;
	unsigned long ret;
	u32 tmp, reg;

	if (pll == CORE_PLL) {
		ret = external_clk[sys_clk];
		if (pllctl_reg_read(pll, ctl) & PLLCTL_PLLEN) {
			/* PLL mode */
			tmp = __raw_readl(K2HK_MAINPLLCTL0);
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
			reg = K2HK_PASSPLLCTL0;
			break;
		case TETRIS_PLL:
			ret = external_clk[tetris_clk];
			reg = K2HK_ARMPLLCTL0;
			break;
		case DDR3A_PLL:
			ret = external_clk[ddr3a_clk];
			reg = K2HK_DDR3APLLCTL0;
			break;
		case DDR3B_PLL:
			ret = external_clk[ddr3b_clk];
			reg = K2HK_DDR3BPLLCTL0;
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

void init_pll(const struct pll_init_data *data)
{
	u32 tmp, tmp_ctl, pllm, plld, pllod, bwadj;

	pllm = data->pll_m - 1;
	plld = (data->pll_d - 1) & PLL_DIV_MASK;
	pllod = (data->pll_od - 1) & PLL_CLKOD_MASK;

	if (data->pll == MAIN_PLL) {
		/* The requered delay before main PLL configuration */
		sdelay(210000);

		tmp = pllctl_reg_read(data->pll, secctl);

		if (tmp & (PLLCTL_BYPASS)) {
			setbits_le32(pll_regs[data->pll].reg1,
				     BIT(MAIN_ENSAT_OFFSET));

			pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLEN |
					   PLLCTL_PLLENSRC);
			sdelay(340);

			pllctl_reg_setbits(data->pll, secctl, PLLCTL_BYPASS);
			pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLPWRDN);
			sdelay(21000);

			pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLPWRDN);
		} else {
			pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLEN |
					   PLLCTL_PLLENSRC);
			sdelay(340);
		}

		pllctl_reg_write(data->pll, mult, pllm & PLLM_MULT_LO_MASK);

		clrsetbits_le32(pll_regs[data->pll].reg0, PLLM_MULT_HI_SMASK,
				(pllm << 6));

		/* Set the BWADJ     (12 bit field)  */
		tmp_ctl = pllm >> 1; /* Divide the pllm by 2 */
		clrsetbits_le32(pll_regs[data->pll].reg0, PLL_BWADJ_LO_SMASK,
				(tmp_ctl << PLL_BWADJ_LO_SHIFT));
		clrsetbits_le32(pll_regs[data->pll].reg1, PLL_BWADJ_HI_MASK,
				(tmp_ctl >> 8));

		/*
		 * Set the pll divider (6 bit field) *
		 * PLLD[5:0] is located in MAINPLLCTL0
		 */
		clrsetbits_le32(pll_regs[data->pll].reg0, PLL_DIV_MASK, plld);

		/* Set the OUTPUT DIVIDE (4 bit field) in SECCTL */
		pllctl_reg_rmw(data->pll, secctl, PLL_CLKOD_SMASK,
			       (pllod << PLL_CLKOD_SHIFT));
		wait_for_completion(data);

		pllctl_reg_write(data->pll, div1, PLLM_RATIO_DIV1);
		pllctl_reg_write(data->pll, div2, PLLM_RATIO_DIV2);
		pllctl_reg_write(data->pll, div3, PLLM_RATIO_DIV3);
		pllctl_reg_write(data->pll, div4, PLLM_RATIO_DIV4);
		pllctl_reg_write(data->pll, div5, PLLM_RATIO_DIV5);

		pllctl_reg_setbits(data->pll, alnctl, 0x1f);

		/*
		 * Set GOSET bit in PLLCMD to initiate the GO operation
		 * to change the divide
		 */
		pllctl_reg_setbits(data->pll, cmd, PLLSTAT_GO);
		sdelay(1500); /* wait for the phase adj */
		wait_for_completion(data);

		/* Reset PLL */
		pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLRST);
		sdelay(21000);	/* Wait for a minimum of 7 us*/
		pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLRST);
		sdelay(105000);	/* Wait for PLL Lock time (min 50 us) */

		pllctl_reg_clrbits(data->pll, secctl, PLLCTL_BYPASS);

		tmp = pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLEN);

	} else if (data->pll == TETRIS_PLL) {
		bwadj = pllm >> 1;
		/* 1.5 Set PLLCTL0[BYPASS] =1 (enable bypass), */
		setbits_le32(pll_regs[data->pll].reg0,  PLLCTL_BYPASS);
		/*
		 * Set CHIPMISCCTL1[13] = 0 (enable glitchfree bypass)
		 * only applicable for Kepler
		 */
		clrbits_le32(K2HK_MISC_CTRL, ARM_PLL_EN);
		/* 2 In PLLCTL1, write PLLRST = 1 (PLL is reset) */
		setbits_le32(pll_regs[data->pll].reg1 ,
			     PLL_PLLRST | PLLCTL_ENSAT);

		/*
		 * 3 Program PLLM and PLLD in PLLCTL0 register
		 * 4 Program BWADJ[7:0] in PLLCTL0 and BWADJ[11:8] in
		 * PLLCTL1 register. BWADJ value must be set
		 * to ((PLLM + 1) >> 1) – 1)
		 */
		tmp = ((bwadj & PLL_BWADJ_LO_MASK) << PLL_BWADJ_LO_SHIFT) |
			(pllm << 6) |
			(plld & PLL_DIV_MASK) |
			(pllod << PLL_CLKOD_SHIFT) | PLLCTL_BYPASS;
		__raw_writel(tmp, pll_regs[data->pll].reg0);

		/* Set BWADJ[11:8] bits */
		tmp = __raw_readl(pll_regs[data->pll].reg1);
		tmp &= ~(PLL_BWADJ_HI_MASK);
		tmp |= ((bwadj>>8) & PLL_BWADJ_HI_MASK);
		__raw_writel(tmp, pll_regs[data->pll].reg1);
		/*
		 * 5 Wait for at least 5 us based on the reference
		 * clock (PLL reset time)
		 */
		sdelay(21000);	/* Wait for a minimum of 7 us*/

		/* 6 In PLLCTL1, write PLLRST = 0 (PLL reset is released) */
		clrbits_le32(pll_regs[data->pll].reg1, PLL_PLLRST);
		/*
		 * 7 Wait for at least 500 * REFCLK cycles * (PLLD + 1)
		 * (PLL lock time)
		 */
		sdelay(105000);
		/* 8 disable bypass */
		clrbits_le32(pll_regs[data->pll].reg0, PLLCTL_BYPASS);
		/*
		 * 9 Set CHIPMISCCTL1[13] = 1 (disable glitchfree bypass)
		 * only applicable for Kepler
		 */
		setbits_le32(K2HK_MISC_CTRL, ARM_PLL_EN);
	} else {
		setbits_le32(pll_regs[data->pll].reg1, PLLCTL_ENSAT);
		/*
		 * process keeps state of Bypass bit while programming
		 * all other DDR PLL settings
		 */
		tmp = __raw_readl(pll_regs[data->pll].reg0);
		tmp &= PLLCTL_BYPASS;	/* clear everything except Bypass */

		/*
		 * Set the BWADJ[7:0], PLLD[5:0] and PLLM to PLLCTL0,
		 * bypass disabled
		 */
		bwadj = pllm >> 1;
		tmp |= ((bwadj & PLL_BWADJ_LO_SHIFT) << PLL_BWADJ_LO_SHIFT) |
			(pllm << PLL_MULT_SHIFT) |
			(plld & PLL_DIV_MASK) |
			(pllod << PLL_CLKOD_SHIFT);
		__raw_writel(tmp, pll_regs[data->pll].reg0);

		/* Set BWADJ[11:8] bits */
		tmp = __raw_readl(pll_regs[data->pll].reg1);
		tmp &= ~(PLL_BWADJ_HI_MASK);
		tmp |= ((bwadj >> 8) & PLL_BWADJ_HI_MASK);

		/* set PLL Select (bit 13) for PASS PLL */
		if (data->pll == PASS_PLL)
			tmp |= PLLCTL_PAPLL;

		__raw_writel(tmp, pll_regs[data->pll].reg1);

		/* Reset bit: bit 14 for both DDR3 & PASS PLL */
		tmp = PLL_PLLRST;
		/* Set RESET bit = 1 */
		setbits_le32(pll_regs[data->pll].reg1, tmp);
		/* Wait for a minimum of 7 us*/
		sdelay(21000);
		/* Clear RESET bit */
		clrbits_le32(pll_regs[data->pll].reg1, tmp);
		sdelay(105000);

		/* clear BYPASS (Enable PLL Mode) */
		clrbits_le32(pll_regs[data->pll].reg0, PLLCTL_BYPASS);
		sdelay(21000);	/* Wait for a minimum of 7 us*/
	}

	/*
	 * This is required to provide a delay between multiple
	 * consequent PPL configurations
	 */
	sdelay(210000);
}

void init_plls(int num_pll, struct pll_init_data *config)
{
	int i;

	for (i = 0; i < num_pll; i++)
		init_pll(&config[i]);
}
