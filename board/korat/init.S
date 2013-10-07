/*
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <ppc_asm.tmpl>
#include <asm/mmu.h>
#include <config.h>

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
	tlbentry( 0xF0000000, SZ_256M, 0xF0000000, 1, AC_RWX | SA_G )

	/*
	 * TLB entries for SDRAM are not needed on this platform.  They are
	 * generated dynamically in the SPD DDR2 detection routine.
	 */

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry( CONFIG_SYS_INIT_RAM_ADDR, SZ_64K, CONFIG_SYS_INIT_RAM_ADDR, 0,
		  AC_RWX | SA_G )
#endif

	/* TLB-entry for PCI Memory */
	tlbentry( CONFIG_SYS_PCI_MEMBASE + 0x00000000, SZ_256M,
		  CONFIG_SYS_PCI_MEMBASE + 0x00000000, 1, AC_RW | SA_IG )

	tlbentry( CONFIG_SYS_PCI_MEMBASE + 0x10000000, SZ_256M,
		  CONFIG_SYS_PCI_MEMBASE + 0x10000000, 1, AC_RW | SA_IG )

	tlbentry( CONFIG_SYS_PCI_MEMBASE + 0x20000000, SZ_256M,
		  CONFIG_SYS_PCI_MEMBASE + 0x20000000, 1, AC_RW | SA_IG )

	tlbentry( CONFIG_SYS_PCI_MEMBASE + 0x30000000, SZ_256M,
		  CONFIG_SYS_PCI_MEMBASE + 0x30000000, 1, AC_RW | SA_IG )

	/* TLB-entry for EBC */
	tlbentry( CONFIG_SYS_CPLD_BASE, SZ_1K, CONFIG_SYS_CPLD_BASE, 1, AC_RW | SA_IG )

	/* TLB-entry for Internal Registers & OCM */
	/* I wonder why this must be executable -- lrj@acm.org 2007-10-08 */
	tlbentry( 0xE0000000, SZ_16M, 0xE0000000, 0, AC_RWX | SA_I )

	/*TLB-entry PCI registers*/
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1, AC_RW | SA_IG )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_RW | SA_IG)

	/* TLB-entry PCI IO Space - from sr@denx.de */
	tlbentry(0xE8000000, SZ_64K, 0xE8000000, 1, AC_RW | SA_IG)

	tlbtab_end

#if defined(CONFIG_KORAT_PERMANENT)
	.globl	korat_branch_absolute
korat_branch_absolute:
	mtlr	r3
	blr
#endif
