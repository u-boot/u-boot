/*
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
#include <asm/mmu.h>

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

	/* vxWorks needs this entry for the Machine Check interrupt,  */
	/* tlbentry( 0x40000000, SZ_256M, 0, 1, AC_R|AC_W|AC_X|SA_G|SA_I ) */

	/*
	 * BOOT_CS (FLASH) must be second. Before relocation SA_I can be off to use the
	 * speed up boot process. It is patched after relocation to enable SA_I
	 */
	tlbentry( CFG_BOOT_BASE_ADDR, SZ_1M, CFG_BOOT_BASE_ADDR, 1, AC_R|AC_W|AC_X|SA_G )

	/* TLB-entry for PCI Memory */
	tlbentry( CFG_PCI_MEMBASE, SZ_256M, CFG_PCI_MEMBASE, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CFG_PCI_MEMBASE1, SZ_256M, CFG_PCI_MEMBASE1, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CFG_PCI_MEMBASE2, SZ_256M, CFG_PCI_MEMBASE2, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CFG_PCI_MEMBASE3, SZ_256M, CFG_PCI_MEMBASE3, 1, AC_R|AC_W|SA_G|SA_I )

	/* TLB-entry for EBC (CFG_CPLD) */
	/* tlbentry( CFG_CPLD, SZ_1K, CFG_CPLD, 1, AC_R|AC_W|AC_X|SA_G|SA_I ) */
	/* 		CAN */
	tlbentry( CFG_CS_1, SZ_16M, CFG_CS_1, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
	 /* 		IMC + CPLD */
	tlbentry( CFG_CS_2, SZ_16M, CFG_CS_2, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
	tlbentry( CFG_CS_2 + 0x1000000, SZ_16M, CFG_CS_2 + 0x1000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
	 /* 		IMC-Fast */
	tlbentry( CFG_CS_3, SZ_16M, CFG_CS_3, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
	tlbentry( CFG_CS_3 + 0x1000000, SZ_16M, CFG_CS_3 + 0x1000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I )

	/* TLB-entry for Internal Registers & OCM */
	tlbentry( CFG_PCI_BASE, SZ_16M, 0xe0000000, 0,  AC_R|AC_W|AC_X|SA_I )

	/*TLB-entry PCI registers*/
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_R|AC_W|AC_X|SA_G|SA_I )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)

	/* TLB for SDRAM will be added by initdram (sdram.c) */

	tlbtab_end
