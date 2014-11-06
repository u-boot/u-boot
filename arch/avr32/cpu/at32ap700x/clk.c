/*
 * Copyright (C) 2005-2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>

#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/portmux.h>

#include "sm.h"

void clk_init(void)
{
	uint32_t cksel;

	/* in case of soft resets, disable watchdog */
	sm_writel(WDT_CTRL, SM_BF(KEY, 0x55));
	sm_writel(WDT_CTRL, SM_BF(KEY, 0xaa));

#ifdef CONFIG_PLL
	/* Initialize the PLL */
	sm_writel(PM_PLL0, (SM_BF(PLLCOUNT, CONFIG_SYS_PLL0_SUPPRESS_CYCLES)
			    | SM_BF(PLLMUL, CONFIG_SYS_PLL0_MUL - 1)
			    | SM_BF(PLLDIV, CONFIG_SYS_PLL0_DIV - 1)
			    | SM_BF(PLLOPT, CONFIG_SYS_PLL0_OPT)
			    | SM_BF(PLLOSC, 0)
			    | SM_BIT(PLLEN)));

	/* Wait for lock */
	while (!(sm_readl(PM_ISR) & SM_BIT(LOCK0))) ;
#endif

	/* Set up clocks for the CPU and all peripheral buses */
	cksel = 0;
	if (CONFIG_SYS_CLKDIV_CPU)
		cksel |= SM_BIT(CPUDIV) | SM_BF(CPUSEL, CONFIG_SYS_CLKDIV_CPU - 1);
	if (CONFIG_SYS_CLKDIV_HSB)
		cksel |= SM_BIT(HSBDIV) | SM_BF(HSBSEL, CONFIG_SYS_CLKDIV_HSB - 1);
	if (CONFIG_SYS_CLKDIV_PBA)
		cksel |= SM_BIT(PBADIV) | SM_BF(PBASEL, CONFIG_SYS_CLKDIV_PBA - 1);
	if (CONFIG_SYS_CLKDIV_PBB)
		cksel |= SM_BIT(PBBDIV) | SM_BF(PBBSEL, CONFIG_SYS_CLKDIV_PBB - 1);
	sm_writel(PM_CKSEL, cksel);

#ifdef CONFIG_PLL
	/* Use PLL0 as main clock */
	sm_writel(PM_MCCTRL, SM_BIT(PLLSEL));

#ifdef CONFIG_LCD
	/* Set up pixel clock for the LCDC */
	sm_writel(PM_GCCTRL(7), SM_BIT(PLLSEL) | SM_BIT(CEN));
#endif
#endif
}

unsigned long __gclk_set_rate(unsigned int id, enum gclk_parent parent,
		unsigned long rate, unsigned long parent_rate)
{
	unsigned long divider;

	if (rate == 0 || parent_rate == 0) {
		sm_writel(PM_GCCTRL(id), 0);
		return 0;
	}

	divider = (parent_rate + rate / 2) / rate;
	if (divider <= 1) {
		sm_writel(PM_GCCTRL(id), parent | SM_BIT(CEN));
		rate = parent_rate;
	} else {
		divider = min(255UL, divider / 2 - 1);
		sm_writel(PM_GCCTRL(id), parent | SM_BIT(CEN) | SM_BIT(DIVEN)
				| SM_BF(DIV, divider));
		rate = parent_rate / (2 * (divider + 1));
	}

	return rate;
}
