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
		if (!(pllctl_reg_read(data->pll, stat) & PLLSTAT_GOSTAT_MASK))
			break;
	}
}

static inline void bypass_main_pll(const struct pll_init_data *data)
{
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLENSRC_MASK |
			   PLLCTL_PLLEN_MASK);

	/* 4 cycles of reference clock CLKIN*/
	sdelay(340);
}

static void configure_mult_div(const struct pll_init_data *data)
{
	u32 pllm, plld, bwadj;

	pllm = data->pll_m - 1;
	plld = (data->pll_d - 1) & CFG_PLLCTL0_PLLD_MASK;

	/* Program Multiplier */
	if (data->pll == MAIN_PLL)
		pllctl_reg_write(data->pll, mult, pllm & PLLM_MULT_LO_MASK);

	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_PLLM_MASK,
			pllm << CFG_PLLCTL0_PLLM_SHIFT);

	/* Program BWADJ */
	bwadj = (data->pll_m - 1) >> 1; /* Divide pllm by 2 */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_BWADJ_MASK,
			(bwadj << CFG_PLLCTL0_BWADJ_SHIFT) &
			CFG_PLLCTL0_BWADJ_MASK);
	bwadj = bwadj >> CFG_PLLCTL0_BWADJ_BITS;
	clrsetbits_le32(keystone_pll_regs[data->pll].reg1,
			CFG_PLLCTL1_BWADJ_MASK, bwadj);

	/* Program Divider */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_PLLD_MASK, plld);
}

void configure_main_pll(const struct pll_init_data *data)
{
	u32 tmp, pllod, i, alnctl_val = 0;
	u32 *offset;

	pllod = data->pll_od - 1;

	/* 100 micro sec for stabilization */
	sdelay(210000);

	tmp = pllctl_reg_read(data->pll, secctl);

	/* Check for Bypass */
	if (tmp & SECCTL_BYPASS_MASK) {
		setbits_le32(keystone_pll_regs[data->pll].reg1,
			     CFG_PLLCTL1_ENSAT_MASK);

		bypass_main_pll(data);

		/* Powerdown and powerup Main Pll */
		pllctl_reg_setbits(data->pll, secctl, SECCTL_BYPASS_MASK);
		pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLPWRDN_MASK);
		/* 5 micro sec */
		sdelay(21000);

		pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLPWRDN_MASK);
	} else {
		bypass_main_pll(data);
	}

	configure_mult_div(data);

	/* Program Output Divider */
	pllctl_reg_rmw(data->pll, secctl, SECCTL_OP_DIV_MASK,
		       ((pllod << SECCTL_OP_DIV_SHIFT) & SECCTL_OP_DIV_MASK));

	/* Program PLLDIVn */
	wait_for_completion(data);
	for (i = 0; i < PLLDIV_MAX; i++) {
		if (i < 3)
			offset = pllctl_reg(data->pll, div1) + i;
		else
			offset = pllctl_reg(data->pll, div4) + (i - 3);

		if (divn_val[i] != -1) {
			__raw_writel(divn_val[i] | PLLDIV_ENABLE_MASK, offset);
			alnctl_val |= BIT(i);
		}
	}

	if (alnctl_val) {
		pllctl_reg_setbits(data->pll, alnctl, alnctl_val);
		/*
		 * Set GOSET bit in PLLCMD to initiate the GO operation
		 * to change the divide
		 */
		pllctl_reg_setbits(data->pll, cmd, PLLSTAT_GOSTAT_MASK);
		wait_for_completion(data);
	}

	/* Reset PLL */
	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLRST_MASK);
	sdelay(21000);	/* Wait for a minimum of 7 us*/
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLRST_MASK);
	sdelay(105000);	/* Wait for PLL Lock time (min 50 us) */

	/* Enable PLL */
	pllctl_reg_clrbits(data->pll, secctl, SECCTL_BYPASS_MASK);
	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLEN_MASK);
}

void configure_secondary_pll(const struct pll_init_data *data)
{
	int pllod = data->pll_od - 1;

	/* Enable Bypass mode */
	setbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_ENSAT_MASK);
	setbits_le32(keystone_pll_regs[data->pll].reg0,
		     CFG_PLLCTL0_BYPASS_MASK);

	/* Enable Glitch free bypass for ARM PLL */
	if (cpu_is_k2hk() && data->pll == TETRIS_PLL)
		clrbits_le32(KS2_MISC_CTRL, MISC_CTL1_ARM_PLL_EN);

	configure_mult_div(data);

	/* Program Output Divider */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_CLKOD_MASK,
			(pllod << CFG_PLLCTL0_CLKOD_SHIFT) &
			CFG_PLLCTL0_CLKOD_MASK);

	/* Reset PLL */
	setbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_RST_MASK);
	/* Wait for 5 micro seconds */
	sdelay(21000);

	/* Select the Output of PASS PLL as input to PASS */
	if (data->pll == PASS_PLL)
		setbits_le32(keystone_pll_regs[data->pll].reg1,
			     CFG_PLLCTL1_PAPLL_MASK);

	/* Select the Output of ARM PLL as input to ARM */
	if (data->pll == TETRIS_PLL)
		setbits_le32(KS2_MISC_CTRL, MISC_CTL1_ARM_PLL_EN);

	clrbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_RST_MASK);
	/* Wait for 500 * REFCLK cucles * (PLLD + 1) */
	sdelay(105000);

	/* Switch to PLL mode */
	clrbits_le32(keystone_pll_regs[data->pll].reg0,
		     CFG_PLLCTL0_BYPASS_MASK);
}

void init_pll(const struct pll_init_data *data)
{
	if (data->pll == MAIN_PLL)
		configure_main_pll(data);
	else
		configure_secondary_pll(data);

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

#ifndef CONFIG_SOC_K2E
inline int get_max_arm_speed(void)
{
	return get_max_speed(read_efuse_bootrom() & 0xffff, arm_speeds);
}
#endif

inline int get_max_dev_speed(void)
{
	return get_max_speed((read_efuse_bootrom() >> 16) & 0xffff, dev_speeds);
}
