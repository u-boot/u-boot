/*
 * Keystone2: pll initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>

#define MAX_SPEEDS		13

static void wait_for_completion(const struct pll_init_data *data)
{
	int i;
	for (i = 0; i < 100; i++) {
		sdelay(450);
		if ((pllctl_reg_read(data->pll, stat) & PLLSTAT_GO) == 0)
			break;
	}
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
			setbits_le32(keystone_pll_regs[data->pll].reg1,
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

		clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
				PLLM_MULT_HI_SMASK, (pllm << 6));

		/* Set the BWADJ     (12 bit field)  */
		tmp_ctl = pllm >> 1; /* Divide the pllm by 2 */
		clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
				PLL_BWADJ_LO_SMASK,
				(tmp_ctl << PLL_BWADJ_LO_SHIFT));
		clrsetbits_le32(keystone_pll_regs[data->pll].reg1,
				PLL_BWADJ_HI_MASK,
				(tmp_ctl >> 8));

		/*
		 * Set the pll divider (6 bit field) *
		 * PLLD[5:0] is located in MAINPLLCTL0
		 */
		clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
				PLL_DIV_MASK, plld);

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

#ifndef CONFIG_SOC_K2E
	} else if (data->pll == TETRIS_PLL) {
		bwadj = pllm >> 1;
		/* 1.5 Set PLLCTL0[BYPASS] =1 (enable bypass), */
		setbits_le32(keystone_pll_regs[data->pll].reg0,  PLLCTL_BYPASS);
		/*
		 * Set CHIPMISCCTL1[13] = 0 (enable glitchfree bypass)
		 * only applicable for Kepler
		 */
		clrbits_le32(KS2_MISC_CTRL, KS2_ARM_PLL_EN);
		/* 2 In PLLCTL1, write PLLRST = 1 (PLL is reset) */
		setbits_le32(keystone_pll_regs[data->pll].reg1 ,
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
		__raw_writel(tmp, keystone_pll_regs[data->pll].reg0);

		/* Set BWADJ[11:8] bits */
		tmp = __raw_readl(keystone_pll_regs[data->pll].reg1);
		tmp &= ~(PLL_BWADJ_HI_MASK);
		tmp |= ((bwadj>>8) & PLL_BWADJ_HI_MASK);
		__raw_writel(tmp, keystone_pll_regs[data->pll].reg1);
		/*
		 * 5 Wait for at least 5 us based on the reference
		 * clock (PLL reset time)
		 */
		sdelay(21000);	/* Wait for a minimum of 7 us*/

		/* 6 In PLLCTL1, write PLLRST = 0 (PLL reset is released) */
		clrbits_le32(keystone_pll_regs[data->pll].reg1, PLL_PLLRST);
		/*
		 * 7 Wait for at least 500 * REFCLK cycles * (PLLD + 1)
		 * (PLL lock time)
		 */
		sdelay(105000);
		/* 8 disable bypass */
		clrbits_le32(keystone_pll_regs[data->pll].reg0, PLLCTL_BYPASS);
		/*
		 * 9 Set CHIPMISCCTL1[13] = 1 (disable glitchfree bypass)
		 * only applicable for Kepler
		 */
		setbits_le32(KS2_MISC_CTRL, KS2_ARM_PLL_EN);
#endif
	} else {
		setbits_le32(keystone_pll_regs[data->pll].reg1, PLLCTL_ENSAT);
		/*
		 * process keeps state of Bypass bit while programming
		 * all other DDR PLL settings
		 */
		tmp = __raw_readl(keystone_pll_regs[data->pll].reg0);
		tmp &= PLLCTL_BYPASS;	/* clear everything except Bypass */

		/*
		 * Set the BWADJ[7:0], PLLD[5:0] and PLLM to PLLCTL0,
		 * bypass disabled
		 */
		bwadj = pllm >> 1;
		tmp |= ((bwadj & PLL_BWADJ_LO_MASK) << PLL_BWADJ_LO_SHIFT) |
			(pllm << PLL_MULT_SHIFT) |
			(plld & PLL_DIV_MASK) |
			(pllod << PLL_CLKOD_SHIFT);
		__raw_writel(tmp, keystone_pll_regs[data->pll].reg0);

		/* Set BWADJ[11:8] bits */
		tmp = __raw_readl(keystone_pll_regs[data->pll].reg1);
		tmp &= ~(PLL_BWADJ_HI_MASK);
		tmp |= ((bwadj >> 8) & PLL_BWADJ_HI_MASK);

		/* set PLL Select (bit 13) for PASS PLL */
		if (data->pll == PASS_PLL)
			tmp |= PLLCTL_PAPLL;

		__raw_writel(tmp, keystone_pll_regs[data->pll].reg1);

		/* Reset bit: bit 14 for both DDR3 & PASS PLL */
		tmp = PLL_PLLRST;
		/* Set RESET bit = 1 */
		setbits_le32(keystone_pll_regs[data->pll].reg1, tmp);
		/* Wait for a minimum of 7 us*/
		sdelay(21000);
		/* Clear RESET bit */
		clrbits_le32(keystone_pll_regs[data->pll].reg1, tmp);
		sdelay(105000);

		/* clear BYPASS (Enable PLL Mode) */
		clrbits_le32(keystone_pll_regs[data->pll].reg0, PLLCTL_BYPASS);
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

static int get_max_speed(u32 val, int *speeds)
{
	int j;

	if (!val)
		return speeds[0];

	for (j = 1; j < MAX_SPEEDS; j++) {
		if (val == 1)
			return speeds[j];
		val >>= 1;
	}

	return SPD800;
}

#ifdef CONFIG_SOC_K2HK
static u32 read_efuse_bootrom(void)
{
	return (cpu_revision() > 1) ? __raw_readl(KS2_EFUSE_BOOTROM) :
		__raw_readl(KS2_REV1_DEVSPEED);
}
#else
static inline u32 read_efuse_bootrom(void)
{
	return __raw_readl(KS2_EFUSE_BOOTROM);
}
#endif

inline int get_max_dev_speed(void)
{
	return get_max_speed(read_efuse_bootrom() & 0xffff, dev_speeds);
}

#ifndef CONFIG_SOC_K2E
inline int get_max_arm_speed(void)
{
	return get_max_speed((read_efuse_bootrom() >> 16) & 0xffff, arm_speeds);
}
#endif
