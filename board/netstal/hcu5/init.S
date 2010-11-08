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

	/* TLB#0: vxWorks needs this entry for the Machine Check interrupt, */
	tlbentry( 0x40000000, SZ_256M, 0, 0, AC_RWX | SA_IG )
	/* TLB#1: TLB-entry for DDR SDRAM (Up to 2GB) */
	tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE, 0,
		AC_RWX | SA_IG )

	/* TLB#2: TLB-entry for EBC */
	tlbentry( 0x80000000, SZ_256M, 0x80000000, 1, AC_RWX | SA_IG)

	/*
	 * TLB#3: BOOT_CS (FLASH) must be forth. Before relocation SA_I can be
	 * off to use the speed up boot process. It is patched after relocation
	 * to enable SA_I
	 */
	tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_1M, CONFIG_SYS_BOOT_BASE_ADDR, 1,
		AC_RWX | SA_G)

	/*
	 * TLB entries for SDRAM are not needed on this platform.
	 * They are dynamically generated in the SPD DDR(2) detection
	 * routine.
	 */

	/* TLB#4: */
	tlbentry( CONFIG_SYS_PCI_MEMBASE1, SZ_256M, CONFIG_SYS_PCI_MEMBASE1, 1,
		AC_RW | SA_IG )
	/* TLB#5: */
	tlbentry( CONFIG_SYS_PCI_MEMBASE2, SZ_256M, CONFIG_SYS_PCI_MEMBASE2, 1,
		AC_RW | SA_IG )
	/* TLB#6: */
	tlbentry( CONFIG_SYS_PCI_MEMBASE3, SZ_256M, CONFIG_SYS_PCI_MEMBASE3, 1,
		AC_RW | SA_IG )

	/* TLB-entry for Internal Registers & OCM */
	/* TLB#7: */
	tlbentry( 0xe0000000, SZ_16M, 0xe0000000, 0,
		AC_RWX | SA_IG )

	/*TLB-entry PCI registers*/
	/* TLB#8: */
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_RWX | SA_IG )

	/* TLB-entry for peripherals */
	/* TLB#9: */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_RWX | SA_IG)

	/*		CAN */
	/* TLB#10: */
	tlbentry( CONFIG_SYS_CS_1, SZ_1K, CONFIG_SYS_CS_1, 1, AC_RWX | SA_IG )

	/* TLB#11:  CPLD and IMC-Standard 32 MB */
	tlbentry( CONFIG_SYS_CS_2, SZ_16M, CONFIG_SYS_CS_2, 1, AC_RWX | SA_IG )

	/* TLB#12: */
	tlbentry( CONFIG_SYS_CS_2 + 0x1000000, SZ_16M, CONFIG_SYS_CS_2 + 0x1000000, 1,
		AC_RWX | SA_IG )

	 /*		IMC-Fast 32 MB */
	/* TLB#13: */
	tlbentry( CONFIG_SYS_CS_3, SZ_16M, CONFIG_SYS_CS_3, 1, AC_RWX | SA_IG )
	/* TLB#14: */
	tlbentry( CONFIG_SYS_CS_3 + 0x1000000, SZ_16M, CONFIG_SYS_CS_3, 1,
		AC_RWX | SA_IG )

	tlbtab_end
