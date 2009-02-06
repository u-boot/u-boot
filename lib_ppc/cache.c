/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/cache.h>
#include <watchdog.h>

void flush_cache(ulong start_addr, ulong size)
{
#ifndef CONFIG_5xx
	ulong addr, start, end;

	start = start_addr & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	end = start_addr + size - 1;

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("dcbst 0,%0" : : "r" (addr) : "memory");
		WATCHDOG_RESET();
	}
	/* wait for all dcbst to complete on bus */
	asm volatile("sync" : : : "memory");

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("icbi 0,%0" : : "r" (addr) : "memory");
		WATCHDOG_RESET();
	}
	asm volatile("sync" : : : "memory");
	/* flush prefetch queue */
	asm volatile("isync" : : : "memory");
#endif
}
