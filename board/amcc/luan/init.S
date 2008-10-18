/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 *  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <ppc_asm.tmpl>
#include <config.h>
#include <asm-ppc/mmu.h>

/**************************************************************************
 * TLB TABLE
 *
 * This table is used by the cpu boot code to setup the initial tlb
 * entries. Rather than make broad assumptions in the cpu source tree,
 * this table lets each board set things up however they like.
 *
 *  Pointer to the table is returned in r1
 *
 *************************************************************************/

	.section .bootpg,"ax"
	.globl tlbtab

tlbtab:
	tlbtab_start

	/*
	 * BOOT_CS (FLASH) must be first. Before relocation SA_I can be off to use the
	 * speed up boot process. It is patched after relocation to enable SA_I
	 */
	tlbentry(0xfff00000, SZ_1M, 0xfff00000, 1, AC_R|AC_W|AC_X|SA_G)

	tlbentry(0xffc00000, SZ_1M, 0xffc00000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(0xffd00000, SZ_1M, 0xffd00000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(0xffe00000, SZ_1M, 0xffe00000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(0xff900000, SZ_1M, 0xff900000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CONFIG_SYS_EPLD_BASE, SZ_256K, 0xff000000, 1, AC_R|AC_W|SA_G|SA_I)

	/*
	 * TLB entries for SDRAM are not needed on this platform.
	 * They are dynamically generated in the SPD DDR(2) detection
	 * routine.
	 */

	/* internal ram (l2 cache) */
	tlbentry(CONFIG_SYS_ISRAM_BASE, SZ_256K, 0x80000000, 0, AC_R|AC_W|AC_X|SA_I)

	/* peripherals at f0000000 */
	tlbentry(CONFIG_SYS_PERIPHERAL_BASE, SZ_4K, CONFIG_SYS_PERIPHERAL_BASE, 1, AC_R|AC_W|SA_G|SA_I)

	/* PCI */
	tlbentry(CONFIG_SYS_PCI_BASE, SZ_256M, 0x00000000, 9, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CONFIG_SYS_PCI_MEMBASE, SZ_256M, 0x10000000, 9, AC_R|AC_W|SA_G|SA_I)
	tlbtab_end
