/*
 * (C) Copyright 2008 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/armv7.h>
#include <linux/compiler.h>

void __weak cpu_cache_initialization(void){}

int cleanup_before_linux(void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */
#ifndef CONFIG_SPL_BUILD
	disable_interrupts();
#endif

	/*
	 * Turn off I-cache and invalidate it
	 */
	icache_disable();
	invalidate_icache_all();

	/*
	 * turn off D-cache
	 * dcache_disable() in turn flushes the d-cache and disables MMU
	 */
	dcache_disable();
	v7_outer_cache_disable();

	/*
	 * After D-cache is flushed and before it is disabled there may
	 * be some new valid entries brought into the cache. We are sure
	 * that these lines are not dirty and will not affect our execution.
	 * (because unwinding the call-stack and setting a bit in CP15 SCTRL
	 * is all we did during this. We have not pushed anything on to the
	 * stack. Neither have we affected any static data)
	 * So just invalidate the entire d-cache again to avoid coherency
	 * problems for kernel
	 */
	invalidate_dcache_all();

	/*
	 * Some CPU need more cache attention before starting the kernel.
	 */
	cpu_cache_initialization();

	return 0;
}
