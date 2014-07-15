/*
 * clock_am43xx.c
 *
 * clocks for AM43XX based boards
 * Derived from AM33XX based boards
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

struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;

const struct dpll_regs dpll_mpu_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x560,
	.cm_idlest_dpll		= CM_WKUP + 0x564,
	.cm_clksel_dpll		= CM_WKUP + 0x56c,
	.cm_div_m2_dpll		= CM_WKUP + 0x570,
};

const struct dpll_regs dpll_core_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x520,
	.cm_idlest_dpll		= CM_WKUP + 0x524,
	.cm_clksel_dpll		= CM_WKUP + 0x52C,
	.cm_div_m4_dpll		= CM_WKUP + 0x538,
	.cm_div_m5_dpll		= CM_WKUP + 0x53C,
	.cm_div_m6_dpll		= CM_WKUP + 0x540,
};

const struct dpll_regs dpll_per_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x5E0,
	.cm_idlest_dpll		= CM_WKUP + 0x5E4,
	.cm_clksel_dpll		= CM_WKUP + 0x5EC,
	.cm_div_m2_dpll		= CM_WKUP + 0x5F0,
};

const struct dpll_regs dpll_ddr_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x5A0,
	.cm_idlest_dpll		= CM_WKUP + 0x5A4,
	.cm_clksel_dpll		= CM_WKUP + 0x5AC,
	.cm_div_m2_dpll		= CM_WKUP + 0x5B0,
	.cm_div_m4_dpll		= CM_WKUP + 0x5B8,
};

void setup_clocks_for_console(void)
{
	/* Do not add any spl_debug prints in this function */
	clrsetbits_le32(&cmwkup->wkclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	/* Enable UART0 */
	clrsetbits_le32(&cmwkup->wkup_uart0ctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}

void enable_basic_clocks(void)
{
	u32 *const clk_domains[] = {
		&cmper->l3clkstctrl,
		&cmper->l3sclkstctrl,
		&cmper->l4lsclkstctrl,
		&cmwkup->wkclkstctrl,
		&cmper->emifclkstctrl,
		0
	};

	u32 *const clk_modules_explicit_en[] = {
		&cmper->l3clkctrl,
		&cmper->l4lsclkctrl,
		&cmper->l4fwclkctrl,
		&cmwkup->wkl4wkclkctrl,
		&cmper->l3instrclkctrl,
		&cmper->l4hsclkctrl,
		&cmwkup->wkgpio0clkctrl,
		&cmwkup->wkctrlclkctrl,
		&cmper->timer2clkctrl,
		&cmper->gpmcclkctrl,
		&cmper->elmclkctrl,
		&cmper->mmc0clkctrl,
		&cmper->mmc1clkctrl,
		&cmwkup->wkup_i2c0ctrl,
		&cmper->gpio1clkctrl,
		&cmper->gpio2clkctrl,
		&cmper->gpio3clkctrl,
		&cmper->gpio4clkctrl,
		&cmper->gpio5clkctrl,
		&cmper->i2c1clkctrl,
		&cmper->cpgmac0clkctrl,
		&cmper->emiffwclkctrl,
		&cmper->emifclkctrl,
		&cmper->otfaemifclkctrl,
		&cmper->qspiclkctrl,
		0
	};

	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);

	/* Select the Master osc clk as Timer2 clock source */
	writel(0x1, &cmdpll->clktimer2clk);
}
