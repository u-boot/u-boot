/*
 * (C) Copyright 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_gpbr.h>
#include <asm/arch/clk.h>

#ifndef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK 0
#endif

int arch_cpu_init(void)
{
	return at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);
}

void arch_preboot_os(void)
{
	ulong cpiv;
	at91_pit_t *pit = (at91_pit_t *) ATMEL_BASE_PIT;

	cpiv = AT91_PIT_MR_PIV_MASK(readl(&pit->piir));

	/*
	 * Disable PITC
	 * Add 0x1000 to current counter to stop it faster
	 * without waiting for wrapping back to 0
	 */
	writel(cpiv + 0x1000, &pit->mr);
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU: %s\n", ATMEL_CPU_NAME);
	printf("Crystal frequency: %8s MHz\n",
					strmhz(buf, get_main_clk_rate()));
	printf("CPU clock        : %8s MHz\n",
					strmhz(buf, get_cpu_clk_rate()));
	printf("Master clock     : %8s MHz\n",
					strmhz(buf, get_mck_clk_rate()));

	return 0;
}
#endif
