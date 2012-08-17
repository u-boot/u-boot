/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include "ap20.h"
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/pmc.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* UARTs which we can enable */
	UARTA	= 1 << 0,
	UARTB	= 1 << 1,
	UARTD	= 1 << 3,
	UART_COUNT = 4,
};

/*
 * Boot ROM initializes the odmdata in APBDEV_PMC_SCRATCH20_0,
 * so we are using this value to identify memory size.
 */

unsigned int query_sdram_size(void)
{
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;
	u32 reg;

	reg = readl(&pmc->pmc_scratch20);
	debug("pmc->pmc_scratch20 (ODMData) = 0x%08x\n", reg);

	/* bits 31:28 in OdmData are used for RAM size  */
	switch ((reg) >> 28) {
	case 1:
		return 0x10000000;	/* 256 MB */
	case 2:
	default:
		return 0x20000000;	/* 512 MB */
	case 3:
		return 0x40000000;	/* 1GB */
	}
}

int dram_init(void)
{
	/* We do not initialise DRAM here. We just query the size */
	gd->ram_size = query_sdram_size();
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: %s\n", sysinfo.board_string);
	return 0;
}
#endif	/* CONFIG_DISPLAY_BOARDINFO */

#ifdef CONFIG_ARCH_CPU_INIT
/*
 * Note this function is executed by the ARM7TDMI AVP. It does not return
 * in this case. It is also called once the A9 starts up, but does nothing in
 * that case.
 */
int arch_cpu_init(void)
{
	/* Fire up the Cortex A9 */
	tegra2_start();

	/* We didn't do this init in start.S, so do it now */
	cpu_init_cp15();

	/* Initialize essential common plls */
	clock_early_init();

	return 0;
}
#endif

/**
 * Set up the specified uarts
 *
 * @param uarts_ids	Mask containing UARTs to init (UARTx)
 */
static void setup_uarts(int uart_ids)
{
	static enum periph_id id_for_uart[] = {
		PERIPH_ID_UART1,
		PERIPH_ID_UART2,
		PERIPH_ID_UART3,
		PERIPH_ID_UART4,
	};
	size_t i;

	for (i = 0; i < UART_COUNT; i++) {
		if (uart_ids & (1 << i)) {
			enum periph_id id = id_for_uart[i];

			funcmux_select(id, FUNCMUX_DEFAULT);
			clock_ll_start_uart(id);
		}
	}
}

void board_init_uart_f(void)
{
	int uart_ids = 0;	/* bit mask of which UART ids to enable */

#ifdef CONFIG_TEGRA2_ENABLE_UARTA
	uart_ids |= UARTA;
#endif
#ifdef CONFIG_TEGRA2_ENABLE_UARTB
	uart_ids |= UARTB;
#endif
#ifdef CONFIG_TEGRA2_ENABLE_UARTD
	uart_ids |= UARTD;
#endif
	setup_uarts(uart_ids);
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
