/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <ppc_asm.tmpl>
#include <asm/mmu.h>
#include <config.h>

/*
 * TLB TABLE
 *
 * This table is used by the cpu boot code to setup the initial tlb
 * entries. Rather than make broad assumptions in the cpu source tree,
 * this table lets each board set things up however they like.
 *
 *  Pointer to the table is returned in r1
 *
 */
    .section .bootpg,"ax"
    .globl tlbtab

tlbtab:
	tlbtab_start

	/*
	 * BOOT_CS (FLASH) must be first. Before relocation SA_I can be off to use the
	 * speed up boot process. It is patched after relocation to enable SA_I
	 */
	tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M, CONFIG_SYS_BOOT_BASE_ADDR, 1, AC_RWX | SA_G )

	/* TLB entries for DDR2 SDRAM are generated dynamically */

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry( CONFIG_SYS_INIT_RAM_ADDR, SZ_64K, CONFIG_SYS_INIT_RAM_ADDR, 0, AC_RWX | SA_G )
#endif

	/* TLB-entry for PCI Memory */
	tlbentry( CONFIG_SYS_PCI_MEMBASE, SZ_256M, CONFIG_SYS_PCI_MEMBASE, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE1, SZ_256M, CONFIG_SYS_PCI_MEMBASE1, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE2, SZ_256M, CONFIG_SYS_PCI_MEMBASE2, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE3, SZ_256M, CONFIG_SYS_PCI_MEMBASE3, 1, AC_RW | SA_IG )

	/* TLB-entries for EBC */
	/* PMC440 maps EBC to 0xef000000 which is handled by the peripheral
	 * tlb entry.
	 * This dummy entry is only for convinience in order not to modify the
	 * amount of entries. Currently OS/9 relies on this :-)
	 */
	tlbentry( 0xc0000000, SZ_256M, 0xc0000000, 1, AC_RWX | SA_IG )

	/* TLB-entry for NAND */
	tlbentry( CONFIG_SYS_NAND_ADDR, SZ_1K, CONFIG_SYS_NAND_ADDR, 1, AC_RWX | SA_IG )

	/* TLB-entry for Internal Registers & OCM */
	tlbentry( 0xe0000000, SZ_16M, 0xe0000000, 0,  AC_RWX | SA_I )

	/*TLB-entry PCI registers*/
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_RWX | SA_IG )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_RWX | SA_IG)

	/* TLB-entry PCI IO space */
	tlbentry(0xE8000000, SZ_64K, 0xE8000000, 1, AC_RWX | SA_IG)

	/* TODO:  what about high IO space */
	tlbtab_end
