/*
 * clock.c
 *
 * clocks for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

#define PRCM_MOD_EN		0x2
#define PRCM_FORCE_WAKEUP	0x2
#define PRCM_FUNCTL		0x0

#define PRCM_EMIF_CLK_ACTIVITY	BIT(2)
#define PRCM_L3_GCLK_ACTIVITY	BIT(4)

#define PLL_BYPASS_MODE		0x4
#define ST_MN_BYPASS		0x00000100
#define ST_DPLL_CLK		0x00000001
#define CLK_SEL_MASK		0x7ffff
#define CLK_DIV_MASK		0x1f
#define CLK_DIV2_MASK		0x7f
#define CLK_SEL_SHIFT		0x8
#define CLK_MODE_SEL		0x7
#define CLK_MODE_MASK		0xfffffff8
#define CLK_DIV_SEL		0xFFFFFFE0
#define CPGMAC0_IDLE		0x30000
#define DPLL_CLKDCOLDO_GATE_CTRL        0x300

const struct cm_perpll *cmper = (struct cm_perpll *)CM_PER;
const struct cm_wkuppll *cmwkup = (struct cm_wkuppll *)CM_WKUP;
const struct cm_dpll *cmdpll = (struct cm_dpll *)CM_DPLL;
const struct cm_rtc *cmrtc = (struct cm_rtc *)CM_RTC;

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

	/* MMC0*/
	writel(PRCM_MOD_EN, &cmper->mmc0clkctrl);
	while (readl(&cmper->mmc0clkctrl) != PRCM_MOD_EN)
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

static void mpu_pll_config(void)
{
	u32 clkmode, clksel, div_m2;

	clkmode = readl(&cmwkup->clkmoddpllmpu);
	clksel = readl(&cmwkup->clkseldpllmpu);
	div_m2 = readl(&cmwkup->divm2dpllmpu);

	/* Set the PLL to bypass Mode */
	writel(PLL_BYPASS_MODE, &cmwkup->clkmoddpllmpu);
	while (readl(&cmwkup->idlestdpllmpu) != ST_MN_BYPASS)
		;

	clksel = clksel & (~CLK_SEL_MASK);
	clksel = clksel | ((MPUPLL_M << CLK_SEL_SHIFT) | MPUPLL_N);
	writel(clksel, &cmwkup->clkseldpllmpu);

	div_m2 = div_m2 & ~CLK_DIV_MASK;
	div_m2 = div_m2 | MPUPLL_M2;
	writel(div_m2, &cmwkup->divm2dpllmpu);

	clkmode = clkmode | CLK_MODE_SEL;
	writel(clkmode, &cmwkup->clkmoddpllmpu);

	while (readl(&cmwkup->idlestdpllmpu) != ST_DPLL_CLK)
		;
}

static void core_pll_config(void)
{
	u32 clkmode, clksel, div_m4, div_m5, div_m6;

	clkmode = readl(&cmwkup->clkmoddpllcore);
	clksel = readl(&cmwkup->clkseldpllcore);
	div_m4 = readl(&cmwkup->divm4dpllcore);
	div_m5 = readl(&cmwkup->divm5dpllcore);
	div_m6 = readl(&cmwkup->divm6dpllcore);

	/* Set the PLL to bypass Mode */
	writel(PLL_BYPASS_MODE, &cmwkup->clkmoddpllcore);

	while (readl(&cmwkup->idlestdpllcore) != ST_MN_BYPASS)
		;

	clksel = clksel & (~CLK_SEL_MASK);
	clksel = clksel | ((COREPLL_M << CLK_SEL_SHIFT) | COREPLL_N);
	writel(clksel, &cmwkup->clkseldpllcore);

	div_m4 = div_m4 & ~CLK_DIV_MASK;
	div_m4 = div_m4 | COREPLL_M4;
	writel(div_m4, &cmwkup->divm4dpllcore);

	div_m5 = div_m5 & ~CLK_DIV_MASK;
	div_m5 = div_m5 | COREPLL_M5;
	writel(div_m5, &cmwkup->divm5dpllcore);

	div_m6 = div_m6 & ~CLK_DIV_MASK;
	div_m6 = div_m6 | COREPLL_M6;
	writel(div_m6, &cmwkup->divm6dpllcore);

	clkmode = clkmode | CLK_MODE_SEL;
	writel(clkmode, &cmwkup->clkmoddpllcore);

	while (readl(&cmwkup->idlestdpllcore) != ST_DPLL_CLK)
		;
}

static void per_pll_config(void)
{
	u32 clkmode, clksel, div_m2;

	clkmode = readl(&cmwkup->clkmoddpllper);
	clksel = readl(&cmwkup->clkseldpllper);
	div_m2 = readl(&cmwkup->divm2dpllper);

	/* Set the PLL to bypass Mode */
	writel(PLL_BYPASS_MODE, &cmwkup->clkmoddpllper);

	while (readl(&cmwkup->idlestdpllper) != ST_MN_BYPASS)
		;

	clksel = clksel & (~CLK_SEL_MASK);
	clksel = clksel | ((PERPLL_M << CLK_SEL_SHIFT) | PERPLL_N);
	writel(clksel, &cmwkup->clkseldpllper);

	div_m2 = div_m2 & ~CLK_DIV2_MASK;
	div_m2 = div_m2 | PERPLL_M2;
	writel(div_m2, &cmwkup->divm2dpllper);

	clkmode = clkmode | CLK_MODE_SEL;
	writel(clkmode, &cmwkup->clkmoddpllper);

	while (readl(&cmwkup->idlestdpllper) != ST_DPLL_CLK)
		;

	writel(DPLL_CLKDCOLDO_GATE_CTRL, &cmwkup->clkdcoldodpllper);
}

void ddr_pll_config(unsigned int ddrpll_m)
{
	u32 clkmode, clksel, div_m2;

	clkmode = readl(&cmwkup->clkmoddpllddr);
	clksel = readl(&cmwkup->clkseldpllddr);
	div_m2 = readl(&cmwkup->divm2dpllddr);

	/* Set the PLL to bypass Mode */
	clkmode = (clkmode & CLK_MODE_MASK) | PLL_BYPASS_MODE;
	writel(clkmode, &cmwkup->clkmoddpllddr);

	/* Wait till bypass mode is enabled */
	while ((readl(&cmwkup->idlestdpllddr) & ST_MN_BYPASS)
				!= ST_MN_BYPASS)
		;

	clksel = clksel & (~CLK_SEL_MASK);
	clksel = clksel | ((ddrpll_m << CLK_SEL_SHIFT) | DDRPLL_N);
	writel(clksel, &cmwkup->clkseldpllddr);

	div_m2 = div_m2 & CLK_DIV_SEL;
	div_m2 = div_m2 | DDRPLL_M2;
	writel(div_m2, &cmwkup->divm2dpllddr);

	clkmode = (clkmode & CLK_MODE_MASK) | CLK_MODE_SEL;
	writel(clkmode, &cmwkup->clkmoddpllddr);

	/* Wait till dpll is locked */
	while ((readl(&cmwkup->idlestdpllddr) & ST_DPLL_CLK) != ST_DPLL_CLK)
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
	mpu_pll_config();
	core_pll_config();
	per_pll_config();

	/* Enable the required interconnect clocks */
	enable_interface_clocks();

	/* Power domain wake up transition */
	power_domain_wkup_transition();

	/* Enable the required peripherals */
	enable_per_clocks();
}
