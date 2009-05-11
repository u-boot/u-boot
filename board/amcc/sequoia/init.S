/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm-ppc/mmu.h>
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
	tlbentry( 0x40000000, SZ_256M, 0, 0, AC_R|AC_W|AC_X|SA_G|SA_I )

	/*
	 * The RAM-boot version skips the SDRAM TLB (identified by EPN=0). This
	 * entry is already configured for SDRAM via the JTAG debugger and mustn't
	 * be re-initialized by this RAM-booting U-Boot version.
	 */
#ifndef CONFIG_SYS_RAMBOOT
	/* TLB-entry for DDR SDRAM (Up to 2GB) */
#ifdef CONFIG_4xx_DCACHE
	tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE, 0, AC_R|AC_W|AC_X|SA_G)
#else
	tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE, 0, AC_R|AC_W|AC_X|SA_G|SA_I )
#endif
#endif /* CONFIG_SYS_RAMBOOT */

	/* TLB-entry for EBC */
	tlbentry( CONFIG_SYS_BCSR_BASE, SZ_256M, CONFIG_SYS_BCSR_BASE, 1, AC_R|AC_W|AC_X|SA_G|SA_I )

	/* BOOT_CS (FLASH) must be forth. Before relocation SA_I can be off to use the
	 * speed up boot process. It is patched after relocation to enable SA_I
	 */
#ifndef CONFIG_NAND_SPL
	tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M, CONFIG_SYS_BOOT_BASE_ADDR, 1, AC_R|AC_W|AC_X|SA_G )
#else
	tlbentry( CONFIG_SYS_NAND_BOOT_SPL_SRC, SZ_4K, CONFIG_SYS_NAND_BOOT_SPL_SRC, 1, AC_R|AC_W|AC_X|SA_G )
#endif

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry( CONFIG_SYS_INIT_RAM_ADDR, SZ_64K, CONFIG_SYS_INIT_RAM_ADDR, 0, AC_R|AC_W|AC_X|SA_G )
#endif

	/* TLB-entry for PCI Memory */
	tlbentry( CONFIG_SYS_PCI_MEMBASE, SZ_256M, CONFIG_SYS_PCI_MEMBASE, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CONFIG_SYS_PCI_MEMBASE1, SZ_256M, CONFIG_SYS_PCI_MEMBASE1, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CONFIG_SYS_PCI_MEMBASE2, SZ_256M, CONFIG_SYS_PCI_MEMBASE2, 1, AC_R|AC_W|SA_G|SA_I )
	tlbentry( CONFIG_SYS_PCI_MEMBASE3, SZ_256M, CONFIG_SYS_PCI_MEMBASE3, 1, AC_R|AC_W|SA_G|SA_I )

	/* TLB-entry for NAND */
	tlbentry( CONFIG_SYS_NAND_ADDR, SZ_1K, CONFIG_SYS_NAND_ADDR, 1, AC_R|AC_W|AC_X|SA_G|SA_I )

	/* TLB-entry for Internal Registers & OCM */
	tlbentry( 0xe0000000, SZ_16M, 0xe0000000, 0,  AC_R|AC_W|AC_X|SA_I )

	/*TLB-entry PCI registers*/
	tlbentry( 0xEEC00000, SZ_1K, 0xEEC00000, 1,  AC_R|AC_W|AC_X|SA_G|SA_I )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)

	/* TLB-entry PCI IO Space - from sr@denx.de */
	tlbentry(0xE8000000, SZ_64K, 0xE8000000, 1, AC_R|AC_W|AC_X|SA_G|SA_I)

	tlbtab_end

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
	/*
	 * For NAND booting the first TLB has to be reconfigured to full size
	 * and with caching disabled after running from RAM!
	 */
#define TLB00	TLB0(CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M)
#define TLB01	TLB1(CONFIG_SYS_BOOT_BASE_ADDR, 1)
#define TLB02	TLB2(AC_R|AC_W|AC_X|SA_G|SA_I)

	.globl	reconfig_tlb0
reconfig_tlb0:
	sync
	isync
	addi	r4,r0,CONFIG_SYS_TLB_FOR_BOOT_FLASH /* TLB entry # */
	lis	r5,TLB00@h
	ori	r5,r5,TLB00@l
	tlbwe	r5,r4,0x0000		/* Save it out */
	lis	r5,TLB01@h
	ori	r5,r5,TLB01@l
	tlbwe	r5,r4,0x0001		/* Save it out */
	lis	r5,TLB02@h
	ori	r5,r5,TLB02@l
	tlbwe	r5,r4,0x0002		/* Save it out */
	sync
	isync
	blr
#endif
