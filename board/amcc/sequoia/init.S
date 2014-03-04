/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
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
 */
    .section .bootpg,"ax"
    .globl tlbtab

tlbtab:
	tlbtab_start

	/* vxWorks needs this as first entry for the Machine Check interrupt */
	tlbentry( 0x40000000, SZ_256M, 0, 0, AC_RWX | SA_IG )

	/*
	 * The RAM-boot version skips the SDRAM TLB (identified by EPN=0). This
	 * entry is already configured for SDRAM via the JTAG debugger and mustn't
	 * be re-initialized by this RAM-booting U-Boot version.
	 */
#ifndef CONFIG_SYS_RAMBOOT
	/* TLB-entry for DDR SDRAM (Up to 2GB) */
#ifdef CONFIG_4xx_DCACHE
	tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE, 0, AC_RWX | SA_G)
#else
	tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE, 0, AC_RWX | SA_IG )
#endif
#endif /* CONFIG_SYS_RAMBOOT */

	/* TLB-entry for EBC */
	tlbentry( CONFIG_SYS_BCSR_BASE, SZ_256M, CONFIG_SYS_BCSR_BASE, 1, AC_RWX | SA_IG )

	/* BOOT_CS (FLASH) must be forth. Before relocation SA_I can be off to use the
	 * speed up boot process. It is patched after relocation to enable SA_I
	 */
	tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M, CONFIG_SYS_BOOT_BASE_ADDR, 1, AC_RWX | SA_G )

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry( CONFIG_SYS_INIT_RAM_ADDR, SZ_64K, CONFIG_SYS_INIT_RAM_ADDR, 0, AC_RWX | SA_G )
#endif

	/* TLB-entry for PCI Memory */
	tlbentry( CONFIG_SYS_PCI_MEMBASE, SZ_256M, CONFIG_SYS_PCI_MEMBASE, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE1, SZ_256M, CONFIG_SYS_PCI_MEMBASE1, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE2, SZ_256M, CONFIG_SYS_PCI_MEMBASE2, 1, AC_RW | SA_IG )
	tlbentry( CONFIG_SYS_PCI_MEMBASE3, SZ_256M, CONFIG_SYS_PCI_MEMBASE3, 1, AC_RW | SA_IG )

	/* TLB-entry for NAND */
	tlbentry( CONFIG_SYS_NAND_ADDR, SZ_1K, CONFIG_SYS_NAND_ADDR, 1, AC_RWX | SA_IG )

	/* TLB-entry for Internal Registers & OCM */
	tlbentry( 0xe0000000, SZ_16M, 0xe0000000, 0,  AC_RWX | SA_I )

	/*TLB-entry PCI registers*/
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_RWX | SA_IG )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_RWX | SA_IG)

	/* TLB-entry PCI IO Space - from sr@denx.de */
	tlbentry(0xE8000000, SZ_64K, 0xE8000000, 1, AC_RWX | SA_IG)

	tlbtab_end
