/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
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
#include <asm/asm.h>

#if defined(CONFIG_CMD_CACHE)

int dcache_status (void)
{
	int i = 0;
	int mask = 0x80;
	__asm__ __volatile__ ("mfs %0,rmsr"::"r" (i):"memory");
	/* i&=0x80 */
	__asm__ __volatile__ ("and %0,%0,%1"::"r" (i), "r" (mask):"memory");
	return i;
}

int icache_status (void)
{
	int i = 0;
	int mask = 0x20;
	__asm__ __volatile__ ("mfs %0,rmsr"::"r" (i):"memory");
	/* i&=0x20 */
	__asm__ __volatile__ ("and %0,%0,%1"::"r" (i), "r" (mask):"memory");
	return i;
}

void	icache_enable (void) {
	MSRSET(0x20);
}

void	icache_disable(void) {
	MSRCLR(0x20);
}

void	dcache_enable (void) {
	MSRSET(0x80);
}

void	dcache_disable(void) {
	MSRCLR(0x80);
}
#endif
