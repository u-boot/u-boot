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

#define PRCM_MOD_EN		0x2
#define PRCM_FORCE_WAKEUP	0x2
#define PRCM_FUNCTL		0x0

#define CPGMAC0_IDLE		0x30000
#define OSC	(V_OSCK/1000000)

const struct cm_perpll *cmper = (struct cm_perpll *)CM_PER;
const struct cm_wkuppll *cmwkup = (struct cm_wkuppll *)CM_WKUP;
const struct cm_dpll *cmdpll = (struct cm_dpll *)CM_DPLL;
const struct cm_rtc *cmrtc = (struct cm_rtc *)CM_RTC;

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

const struct dpll_params dpll_mpu = {
		CONFIG_SYS_MPUCLK, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_core = {
		1000, OSC-1, -1, -1, 10, 8, 4};
const struct dpll_params dpll_per = {
		960, OSC-1, 5, -1, -1, -1, -1};

static void enable_interface_clocks(void)
{
	/* Enable all the Interconnect Modules */
	writel(PRCM_MOD_EN, &cmper->l3clkctrl);
	while (readl(&cmper->l3clkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmper->l4lsclkctrl);
	while (readl(&cmper->l4lsclkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmper->l4fwclkctrl);
	while (readl(&cmper->l4fwclkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmwkup->wkl4wkclkctrl);
	while (readl(&cmwkup->wkl4wkclkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmper->l3instrclkctrl);
	while (readl(&cmper->l3instrclkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmper->l4hsclkctrl);
	while (readl(&cmper->l4hsclkctrl) != PRCM_MOD_EN)
		;

	writel(PRCM_MOD_EN, &cmwkup->wkgpio0clkctrl);
	while (readl(&cmwkup->wkgpio0clkctrl) != PRCM_MOD_EN)
		;
}

/*
 * Force power domain wake up transition
 * Ensure that the corresponding interface clock is active before
 * using the peripheral
 */
static void power_domain_wkup_transition(void)
{
	writel(PRCM_FORCE_WAKEUP, &cmper->l3clkstctrl);
	writel(PRCM_FORCE_WAKEUP, &cmper->l4lsclkstctrl);
	writel(PRCM_FORCE_WAKEUP, &cmwkup->wkclkstctrl);
	writel(PRCM_FORCE_WAKEUP, &cmper->l4fwclkstctrl);
	writel(PRCM_FORCE_WAKEUP, &cmper->l3sclkstctrl);
}

/*
 * Enable the peripheral clock for required peripherals
 */
static void enable_per_clocks(void)
{
	/* Enable the control module though RBL would have done it*/
	writel(PRCM_MOD_EN, &cmwkup->wkctrlclkctrl);
	while (readl(&cmwkup->wkctrlclkctrl) != PRCM_MOD_EN)
		;

	/* Enable the module clock */
	writel(PRCM_MOD_EN, &cmper->timer2clkctrl);
	while (readl(&cmper->timer2clkctrl) != PRCM_MOD_EN)
		;

	/* Select the Master osc 24 MHZ as Timer2 clock source */
	writel(0x1, &cmdpll->clktimer2clk);

	/* UART0 */
	writel(PRCM_MOD_EN, &cmwkup->wkup_uart0ctrl);
	while (readl(&cmwkup->wkup_uart0ctrl) != PRCM_MOD_EN)
		;

	/* UART1 */
#ifdef CONFIG_SERIAL2
	writel(PRCM_MOD_EN, &cmper->uart1clkctrl);
	while (readl(&cmper->uart1clkctrl) != PRCM_MOD_EN)
		;
#endif /* CONFIG_SERIAL2 */

	/* UART2 */
#ifdef CONFIG_SERIAL3
	writel(PRCM_MOD_EN, &cmper->uart2clkctrl);
	while (readl(&cmper->uart2clkctrl) != PRCM_MOD_EN)
		;
#endif /* CONFIG_SERIAL3 */

	/* UART3 */
#ifdef CONFIG_SERIAL4
	writel(PRCM_MOD_EN, &cmper->uart3clkctrl);
	while (readl(&cmper->uart3clkctrl) != PRCM_MOD_EN)
		;
#endif /* CONFIG_SERIAL4 */

	/* UART4 */
#ifdef CONFIG_SERIAL5
	writel(PRCM_MOD_EN, &cmper->uart4clkctrl);
	while (readl(&cmper->uart4clkctrl) != PRCM_MOD_EN)
		;
#endif /* CONFIG_SERIAL5 */

	/* UART5 */
#ifdef CONFIG_SERIAL6
	writel(PRCM_MOD_EN, &cmper->uart5clkctrl);
	while (readl(&cmper->uart5clkctrl) != PRCM_MOD_EN)
		;
#endif /* CONFIG_SERIAL6 */

	/* GPMC */
	writel(PRCM_MOD_EN, &cmper->gpmcclkctrl);
	while (readl(&cmper->gpmcclkctrl) != PRCM_MOD_EN)
		;

	/* ELM */
	writel(PRCM_MOD_EN, &cmper->elmclkctrl);
	while (readl(&cmper->elmclkctrl) != PRCM_MOD_EN)
		;

	/* MMC0*/
	writel(PRCM_MOD_EN, &cmper->mmc0clkctrl);
	while (readl(&cmper->mmc0clkctrl) != PRCM_MOD_EN)
		;

	/* MMC1 */
	writel(PRCM_MOD_EN, &cmper->mmc1clkctrl);
	while (readl(&cmper->mmc1clkctrl) != PRCM_MOD_EN)
		;

	/* i2c0 */
	writel(PRCM_MOD_EN, &cmwkup->wkup_i2c0ctrl);
	while (readl(&cmwkup->wkup_i2c0ctrl) != PRCM_MOD_EN)
		;

	/* gpio1 module */
	writel(PRCM_MOD_EN, &cmper->gpio1clkctrl);
	while (readl(&cmper->gpio1clkctrl) != PRCM_MOD_EN)
		;

	/* gpio2 module */
	writel(PRCM_MOD_EN, &cmper->gpio2clkctrl);
	while (readl(&cmper->gpio2clkctrl) != PRCM_MOD_EN)
		;

	/* gpio3 module */
	writel(PRCM_MOD_EN, &cmper->gpio3clkctrl);
	while (readl(&cmper->gpio3clkctrl) != PRCM_MOD_EN)
		;

	/* i2c1 */
	writel(PRCM_MOD_EN, &cmper->i2c1clkctrl);
	while (readl(&cmper->i2c1clkctrl) != PRCM_MOD_EN)
		;

	/* Ethernet */
	writel(PRCM_MOD_EN, &cmper->cpgmac0clkctrl);
	while ((readl(&cmper->cpgmac0clkctrl) & CPGMAC0_IDLE) != PRCM_FUNCTL)
		;

	/* spi0 */
	writel(PRCM_MOD_EN, &cmper->spi0clkctrl);
	while (readl(&cmper->spi0clkctrl) != PRCM_MOD_EN)
		;

	/* RTC */
	writel(PRCM_MOD_EN, &cmrtc->rtcclkctrl);
	while (readl(&cmrtc->rtcclkctrl) != PRCM_MOD_EN)
		;

	/* MUSB */
	writel(PRCM_MOD_EN, &cmper->usb0clkctrl);
	while (readl(&cmper->usb0clkctrl) != PRCM_MOD_EN)
		;
}

void enable_emif_clocks(void)
{
	/* Enable the  EMIF_FW Functional clock */
	writel(PRCM_MOD_EN, &cmper->emiffwclkctrl);
	/* Enable EMIF0 Clock */
	writel(PRCM_MOD_EN, &cmper->emifclkctrl);
	/* Poll if module is functional */
	while ((readl(&cmper->emifclkctrl)) != PRCM_MOD_EN)
		;
}

/*
 * Configure the PLL/PRCM for necessary peripherals
 */
void pll_init()
{
	setup_dplls();
	/* Enable the required interconnect clocks */
	enable_interface_clocks();

	/* Power domain wake up transition */
	power_domain_wkup_transition();

	/* Enable the required peripherals */
	enable_per_clocks();
}
