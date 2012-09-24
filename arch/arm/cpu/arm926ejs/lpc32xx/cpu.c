/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/wdt.h>
#include <asm/io.h>

static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;
static struct wdt_regs  *wdt = (struct wdt_regs *)WDT_BASE;

void reset_cpu(ulong addr)
{
	/* Enable watchdog clock */
	setbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);

	/* Reset pulse length is 13005 peripheral clock frames */
	writel(13000, &wdt->pulse);

	/* Force WDOG_RESET2 and RESOUT_N signal active */
	writel(WDTIM_MCTRL_RESFRC2 | WDTIM_MCTRL_RESFRC1 | WDTIM_MCTRL_M_RES2,
	       &wdt->mctrl);

	while (1)
		/* NOP */;
}

#if defined(CONFIG_ARCH_CPU_INIT)
int arch_cpu_init(void)
{
	/*
	 * It might be necessary to flush data cache, if U-boot is loaded
	 * from kickstart bootloader, e.g. from S1L loader
	 */
	flush_dcache_all();

	return 0;
}
#else
#error "You have to select CONFIG_ARCH_CPU_INIT"
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	printf("CPU:   NXP LPC32XX\n");
	printf("CPU clock:        %uMHz\n", get_hclk_pll_rate() / 1000000);
	printf("AHB bus clock:    %uMHz\n", get_hclk_clk_rate() / 1000000);
	printf("Peripheral clock: %uMHz\n", get_periph_clk_rate() / 1000000);

	return 0;
}
#endif
