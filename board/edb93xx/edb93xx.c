/*
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * (C) Copyright 2002 2003
 * Network Audio Technologies, Inc. <www.netaudiotech.com>
 * Adam Bezanson <bezanson@netaudiotech.com>
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
#include <netdev.h>
#include <asm/arch/ep93xx.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_BANK_SIZE	0x04000000 /* 64 MB */

static ulong const bank_addr[CONFIG_NR_DRAM_BANKS] = {
	PHYS_SDRAM_1,
#ifdef PHYS_SDRAM_2
	PHYS_SDRAM_2,
#endif
#ifdef PHYS_SDRAM_3
	PHYS_SDRAM_3,
#endif
#ifdef PHYS_SDRAM_4
	PHYS_SDRAM_4
#endif
};

int board_init(void)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;

	icache_enable();

#ifdef USE_920T_MMU
	dcache_enable();
#endif

	/*
	 * set UARTBAUD bit to drive UARTs with 14.7456MHz instead of
	 * 14.7456/2 MHz
	 */
	uint32_t value = readl(&syscon->pwrcnt);
	value |= SYSCON_PWRCNT_UART_BAUD;
	writel(value, &syscon->pwrcnt);

	/* Enable the uart in devicecfg */
	value = readl(&syscon->devicecfg);
	value |= 1<<18 /* U1EN */;
	writel(0xAA, &syscon->sysswlock);
	writel(value, &syscon->devicecfg);

	/* Machine number, as defined in linux/arch/arm/tools/mach-types */
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* We have a console */
	gd->have_console = 1;

	return 0;
}

int board_eth_init(bd_t *bd)
{
	return ep93xx_eth_initialize(0, MAC_BASE);
}

int dram_init(void)
{
	unsigned int *src, *dst;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		const ulong bank_size = get_ram_size((long *)bank_addr[i],
						MAX_BANK_SIZE);
		if (bank_size) {
			gd->bd->bi_dram[i].start = bank_addr[i];
			gd->bd->bi_dram[i].size = bank_size;
		}
	}

	/* copy exception vectors */
	src = (unsigned int *)_armboot_start;
	dst = (unsigned int *)PHYS_SDRAM_1;
	memcpy(dst, src, 16 * sizeof(unsigned int));

	return 0;
}
