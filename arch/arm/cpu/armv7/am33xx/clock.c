/*
 * clock.c
 *
 * Clock initialization for AM33XX boards.
 * Derived from OMAP4 boards
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

static void setup_post_dividers(const struct dpll_regs *dpll_regs,
			 const struct dpll_params *params)
{
	/* Setup post-dividers */
	if (params->m2 >= 0)
		writel(params->m2, dpll_regs->cm_div_m2_dpll);
	if (params->m3 >= 0)
		writel(params->m3, dpll_regs->cm_div_m3_dpll);
	if (params->m4 >= 0)
		writel(params->m4, dpll_regs->cm_div_m4_dpll);
	if (params->m5 >= 0)
		writel(params->m5, dpll_regs->cm_div_m5_dpll);
	if (params->m6 >= 0)
		writel(params->m6, dpll_regs->cm_div_m6_dpll);
}

static inline void do_lock_dpll(const struct dpll_regs *dpll_regs)
{
	clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_LOCK << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_lock(const struct dpll_regs *dpll_regs)
{
	if (!wait_on_value(ST_DPLL_CLK_MASK, ST_DPLL_CLK_MASK,
			   (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("DPLL locking failed for 0x%x\n",
		       dpll_regs->cm_clkmode_dpll);
		hang();
	}
}

static inline void do_bypass_dpll(const struct dpll_regs *dpll_regs)
{
	clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_MN_BYPASS << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_bypass(const struct dpll_regs *dpll_regs)
{
	if (!wait_on_value(ST_DPLL_CLK_MASK, 0,
			   (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("Bypassing DPLL failed 0x%x\n",
		       dpll_regs->cm_clkmode_dpll);
	}
}

static void bypass_dpll(const struct dpll_regs *dpll_regs)
{
	do_bypass_dpll(dpll_regs);
	wait_for_bypass(dpll_regs);
}

void do_setup_dpll(const struct dpll_regs *dpll_regs,
		   const struct dpll_params *params)
{
	u32 temp;

	if (!params)
		return;

	temp = readl(dpll_regs->cm_clksel_dpll);

	bypass_dpll(dpll_regs);

	/* Set M & N */
	temp &= ~CM_CLKSEL_DPLL_M_MASK;
	temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

	temp &= ~CM_CLKSEL_DPLL_N_MASK;
	temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

	writel(temp, dpll_regs->cm_clksel_dpll);

	setup_post_dividers(dpll_regs, params);

	/* Wait till the DPLL locks */
	do_lock_dpll(dpll_regs);
	wait_for_lock(dpll_regs);
}

void setup_dplls(void)
{
	const struct dpll_params *params;
	do_setup_dpll(&dpll_core_regs, &dpll_core);
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu);
	do_setup_dpll(&dpll_per_regs, &dpll_per);
	writel(0x300, &cmwkup->clkdcoldodpllper);

	params = get_dpll_ddr_params();
	do_setup_dpll(&dpll_ddr_regs, params);
}
