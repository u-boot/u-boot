/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

/* CPU specific code */
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>

#include <faraday/ftwdt010_wdt.h>

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

#ifdef CONFIG_MMU
	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();

	/* flush I/D-cache */
	invalidate_icac();
	invalidate_dcac();
#endif

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();

	/*
	 * reset to the base addr of andesboot.
	 * currently no ROM loader at addr 0.
	 * do not use reset_cpu(0);
	 */
#ifdef CONFIG_FTWDT010_WATCHDOG
	/*
	 * workaround: if we use CONFIG_HW_WATCHDOG with ftwdt010, will lead
	 * automatic hardware reset when booting Linux.
	 * Please do not use CONFIG_HW_WATCHDOG and WATCHDOG_RESET() here.
	 */
	ftwdt010_wdt_reset();
	while (1)
		;
#endif /* CONFIG_FTWDT010_WATCHDOG */

	/*NOTREACHED*/
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache)
{
	if (cache == ICACHE)
		return 8 << (((GET_ICM_CFG() & ICM_CFG_MSK_ISZ) \
					>> ICM_CFG_OFF_ISZ) - 1);
	else
		return 8 << (((GET_DCM_CFG() & DCM_CFG_MSK_DSZ) \
					>> DCM_CFG_OFF_DSZ) - 1);
}

void dcache_flush_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start) {
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB" : : "r"(start));
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL" : : "r"(start));
		start += line_size;
	}
}

void icache_inval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(ICACHE);
	while (end > start) {
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL" : : "r"(start));
		start += line_size;
	}
}

void flush_cache(unsigned long addr, unsigned long size)
{
	dcache_flush_range(addr, addr + size);
	icache_inval_range(addr, addr + size);
}

void icache_enable(void)
{
	__asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"ori	$p0, $p0, 0x01\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

void icache_disable(void)
{
	__asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"li	$p1, ~0x01\n\t"
		"and	$p0, $p0, $p1\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

int icache_status(void)
{
	int ret;

	 __asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"andi	%0,  $p0, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);

	 return ret;
}

void dcache_enable(void)
{
	 __asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"ori	$p0, $p0, 0x02\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

void dcache_disable(void)
{
	 __asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"li	$p1, ~0x02\n\t"
		"and	$p0, $p0, $p1\n\t"
		"mtsr	$p0, $mr8\n\t"
		"isb\n\t"
	);
}

int dcache_status(void)
{
	int ret;

	__asm__ __volatile__ (
		"mfsr	$p0, $mr8\n\t"
		"andi	%0, $p0, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	 );

	 return ret;
}
