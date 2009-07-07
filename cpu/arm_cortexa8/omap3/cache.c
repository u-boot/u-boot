/*
 * (C) Copyright 2008 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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
 * omap3 L2 cache code
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>

void l2_cache_enable(void)
{
	unsigned long i;
	volatile unsigned int j;

	/* ES2 onwards we can disable/enable L2 ourselves */
	if (get_cpu_rev() >= CPU_3XX_ES20) {
		__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
		__asm__ __volatile__("orr %0, %0, #0x2":"=r"(i));
		__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
	} else {
		/* Save r0, r12 and restore them after usage */
		__asm__ __volatile__("mov %0, r12":"=r"(j));
		__asm__ __volatile__("mov %0, r0":"=r"(i));

		/*
		 * GP Device ROM code API usage here
		 * r12 = AUXCR Write function and r0 value
		 */
		__asm__ __volatile__("mov r12, #0x3");
		__asm__ __volatile__("mrc p15, 0, r0, c1, c0, 1");
		__asm__ __volatile__("orr r0, r0, #0x2");
		/* SMI instruction to call ROM Code API */
		__asm__ __volatile__(".word 0xE1600070");
		__asm__ __volatile__("mov r0, %0":"=r"(i));
		__asm__ __volatile__("mov r12, %0":"=r"(j));
	}

}

void l2_cache_disable(void)
{
	unsigned long i;
	volatile unsigned int j;

	/* ES2 onwards we can disable/enable L2 ourselves */
	if (get_cpu_rev() >= CPU_3XX_ES20) {
		__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
		__asm__ __volatile__("bic %0, %0, #0x2":"=r"(i));
		__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
	} else {
		/* Save r0, r12 and restore them after usage */
		__asm__ __volatile__("mov %0, r12":"=r"(j));
		__asm__ __volatile__("mov %0, r0":"=r"(i));

		/*
		 * GP Device ROM code API usage here
		 * r12 = AUXCR Write function and r0 value
		 */
		__asm__ __volatile__("mov r12, #0x3");
		__asm__ __volatile__("mrc p15, 0, r0, c1, c0, 1");
		__asm__ __volatile__("bic r0, r0, #0x2");
		/* SMI instruction to call ROM Code API */
		__asm__ __volatile__(".word 0xE1600070");
		__asm__ __volatile__("mov r0, %0":"=r"(i));
		__asm__ __volatile__("mov r12, %0":"=r"(j));
	}
}
