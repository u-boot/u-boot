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


void flush_cache (ulong start_addr, ulong size)
{
#ifndef CONFIG_5xx
	ulong addr, end_addr = start_addr + size;

	if (CFG_CACHELINE_SIZE) {
		addr = start_addr & (CFG_CACHELINE_SIZE - 1);
		for (addr = start_addr;
		     addr < end_addr;
		     addr += CFG_CACHELINE_SIZE) {
			asm ("dcbst 0,%0": :"r" (addr));
		}
		asm ("sync");	/* Wait for all dcbst to complete on bus */

		for (addr = start_addr;
		     addr < end_addr;
		     addr += CFG_CACHELINE_SIZE) {
			asm ("icbi 0,%0": :"r" (addr));
		}
	}
	asm ("sync");		/* Always flush prefetch queue in any case */
	asm ("isync");
#endif
}
