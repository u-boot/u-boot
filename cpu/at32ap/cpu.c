/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
#include <command.h>

#include <asm/io.h>
#include <asm/sections.h>
#include <asm/sysreg.h>

#include <asm/arch/memory-map.h>
#include <asm/arch/platform.h>

#include "hsmc3.h"

DECLARE_GLOBAL_DATA_PTR;

int cpu_init(void)
{
	const struct device *hebi;
	extern void _evba(void);
	char *p;

	gd->cpu_hz = CFG_OSC0_HZ;

	/* fff03400: 00010001 04030402 00050005 10011103 */
	hebi = get_device(DEVICE_HEBI);
	hsmc3_writel(hebi, MODE0, 0x00031103);
	hsmc3_writel(hebi, CYCLE0, 0x000c000d);
	hsmc3_writel(hebi, PULSE0, 0x0b0a0906);
	hsmc3_writel(hebi, SETUP0, 0x00010002);

	pm_init();

	sysreg_write(EVBA, (unsigned long)&_evba);
	asm volatile("csrf	%0" : : "i"(SYSREG_EM_OFFSET));
	gd->console_uart = get_device(CFG_CONSOLE_UART_DEV);

	/* Lock everything that mess with the flash in the icache */
	for (p = __flashprog_start; p <= (__flashprog_end + CFG_ICACHE_LINESZ);
	     p += CFG_ICACHE_LINESZ)
		asm volatile("cache %0, 0x02" : "=m"(*p) :: "memory");

	return 0;
}

void prepare_to_boot(void)
{
	/* Flush both caches and the write buffer */
	asm volatile("cache  %0[4], 010\n\t"
		     "cache  %0[0], 000\n\t"
		     "sync   0" : : "r"(0) : "memory");
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	/* This will reset the CPU core, caches, MMU and all internal busses */
	__builtin_mtdr(8, 1 << 13);	/* set DC:DBE */
	__builtin_mtdr(8, 1 << 30);	/* set DC:RES */

	/* Flush the pipeline before we declare it a failure */
	asm volatile("sub   pc, pc, -4");

	return -1;
}
