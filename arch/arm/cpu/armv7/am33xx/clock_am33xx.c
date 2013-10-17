/*
 * clock_am33xx.c
 *
 * clocks for AM33XX based boards
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

#define OSC	(V_OSCK/1000000)

struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;
struct cm_rtc *const cmrtc = (struct cm_rtc *)CM_RTC;

const struct dpll_regs dpll_mpu_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x88,
	.cm_idlest_dpll		= CM_WKUP + 0x20,
	.cm_clksel_dpll		= CM_WKUP + 0x2C,
	.cm_div_m2_dpll		= CM_WKUP + 0xA8,
};

const struct dpll_regs dpll_core_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x90,
	.cm_idlest_dpll		= CM_WKUP + 0x5C,
	.cm_clksel_dpll		= CM_WKUP + 0x68,
	.cm_div_m4_dpll		= CM_WKUP + 0x80,
	.cm_div_m5_dpll		= CM_WKUP + 0x84,
	.cm_div_m6_dpll		= CM_WKUP + 0xD8,
};

const struct dpll_regs dpll_per_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x8C,
	.cm_idlest_dpll		= CM_WKUP + 0x70,
	.cm_clksel_dpll		= CM_WKUP + 0x9C,
	.cm_div_m2_dpll		= CM_WKUP + 0xAC,
};

const struct dpll_regs dpll_ddr_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x94,
	.cm_idlest_dpll		= CM_WKUP + 0x34,
	.cm_clksel_dpll		= CM_WKUP + 0x40,
	.cm_div_m2_dpll		= CM_WKUP + 0xA0,
};

struct dpll_params dpll_mpu_opp100 = {
		CONFIG_SYS_MPUCLK, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_core_opp100 = {
		1000, OSC-1, -1, -1, 10, 8, 4};
const struct dpll_params dpll_mpu = {
		MPUPLL_M_300, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_core = {
		50, OSC-1, -1, -1, 1, 1, 1};
const struct dpll_params dpll_per = {
		960, OSC-1, 5, -1, -1, -1, -1};

void setup_clocks_for_console(void)
{
	clrsetbits_le32(&cmwkup->wkclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	clrsetbits_le32(&cmper->l4hsclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	clrsetbits_le32(&cmwkup->wkup_uart0ctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart1clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart2clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart3clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart4clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart5clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}

void enable_basic_clocks(void)
{
	u32 *const clk_domains[] = {
		&cmper->l3clkstctrl,
		&cmper->l4fwclkstctrl,
		&cmper->l3sclkstctrl,
		&cmper->l4lsclkstctrl,
		&cmwkup->wkclkstctrl,
		&cmper->emiffwclkctrl,
		&cmrtc->clkstctrl,
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
		&cmper->i2c1clkctrl,
		&cmper->cpgmac0clkctrl,
		&cmper->spi0clkctrl,
		&cmrtc->rtcclkctrl,
		&cmper->usb0clkctrl,
		&cmper->emiffwclkctrl,
		&cmper->emifclkctrl,
		0
	};

	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);

	/* Select the Master osc 24 MHZ as Timer2 clock source */
	writel(0x1, &cmdpll->clktimer2clk);
}
