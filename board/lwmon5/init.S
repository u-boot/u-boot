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
	tlbentry(CFG_BOOT_BASE_ADDR, SZ_256M, CFG_BOOT_BASE_ADDR, 1, AC_R|AC_W|AC_X|SA_G)

	/*
	 * TLB entries for SDRAM are not needed on this platform.
	 * They are dynamically generated in the SPD DDR(2) detection
	 * routine.
	 */

#ifdef CFG_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry(CFG_INIT_RAM_ADDR, SZ_64K, CFG_INIT_RAM_ADDR, 0, AC_R|AC_W|AC_X|SA_G)
#endif

	/* TLB-entry for PCI Memory */
	tlbentry(CFG_PCI_MEMBASE, SZ_256M, CFG_PCI_MEMBASE, 1, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE1, SZ_256M, CFG_PCI_MEMBASE1, 1, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE2, SZ_256M, CFG_PCI_MEMBASE2, 1, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE3, SZ_256M, CFG_PCI_MEMBASE3, 1, AC_R|AC_W|SA_G|SA_I)

	/* TLB-entry for the FPGA Chip select 2 */
	tlbentry(CFG_FPGA_BASE_0, SZ_1M, CFG_FPGA_BASE_0, 1, AC_R|AC_W|AC_X|SA_I|SA_G)

	/* TLB-entry for the FPGA Chip select 3 */
	tlbentry(CFG_FPGA_BASE_1, SZ_1M, CFG_FPGA_BASE_1, 1,AC_R|AC_W|AC_X|SA_I|SA_G)

	/* TLB-entry for the LIME Controller */
	tlbentry(CFG_LIME_BASE_0, SZ_16M, CFG_LIME_BASE_0, 1, AC_R|AC_W|AC_X|SA_I|SA_G)
	tlbentry(CFG_LIME_BASE_1, SZ_16M, CFG_LIME_BASE_1, 1, AC_R|AC_W|AC_X|SA_I|SA_G)
	tlbentry(CFG_LIME_BASE_2, SZ_16M, CFG_LIME_BASE_2, 1, AC_R|AC_W|AC_X|SA_I|SA_G)
	tlbentry(CFG_LIME_BASE_3, SZ_16M, CFG_LIME_BASE_3, 1, AC_R|AC_W|AC_X|SA_I|SA_G)

	/* TLB-entry for Internal Registers & OCM */
	tlbentry(0xe0000000, SZ_16M, 0xe0000000, 0,  AC_R|AC_W|AC_X|SA_I)

	/*TLB-entry PCI registers*/
	tlbentry(0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_R|AC_W|AC_X|SA_G|SA_I)

	/* TLB-entry for peripherals */
	tlbentry(0xEF000000, SZ_16M, 0xEF000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)

	tlbtab_end
