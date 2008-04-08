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
	 * BOOT_CS (FLASH) must be first. Before relocation SA_I can be off to
	 * use the speed up boot process. It is patched after relocation to
	 * enable SA_I
	 */
#ifndef CONFIG_NAND_SPL
	tlbentry(CFG_BOOT_BASE_ADDR, SZ_16M, CFG_BOOT_BASE_ADDR, 4, AC_R|AC_W|AC_X|SA_G) /* TLB 0 */
#else
	tlbentry(CFG_NAND_BOOT_SPL_SRC, SZ_4K, CFG_NAND_BOOT_SPL_SRC, 4, AC_R|AC_W|AC_X|SA_G)
	tlbentry(CFG_SDRAM_BASE, SZ_256M, CFG_SDRAM_BASE, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(256 << 20, SZ_256M, 256 << 20, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
#endif

	/*
	 * TLB entries for SDRAM are not needed on this platform.
	 * They are dynamically generated in the SPD DDR(2) detection
	 * routine.
	 */

#ifdef CFG_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry(CFG_INIT_RAM_ADDR, SZ_4K, CFG_INIT_RAM_ADDR, 0, AC_R|AC_W|AC_X|SA_G)
#endif

	tlbentry(CFG_PCI_BASE, SZ_256M, 0x00000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE, SZ_256M, 0x20000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE_MEMBASE, SZ_256M, 0xB0000000, 0xD, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_PCIE0_CFGBASE, SZ_16M, 0x00000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_CFGBASE, SZ_16M, 0x20000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE0_XCFGBASE, SZ_1K, 0x10000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_XCFGBASE, SZ_1K, 0x30000000, 0xD, AC_R|AC_W|SA_G|SA_I)

	/* PCIe UTL register */
	tlbentry(CFG_PCIE_BASE, SZ_16K, 0x08010000, 0xC, AC_R|AC_W|SA_G|SA_I)

	/* TLB-entry for NAND */
	tlbentry(CFG_NAND_ADDR, SZ_16M, CFG_NAND_ADDR, 4, AC_R|AC_W|AC_X|SA_G|SA_I)

	/* TLB-entry for CPLD */
	tlbentry(CFG_BCSR_BASE, SZ_1K, CFG_BCSR_BASE, 4, AC_R|AC_W|SA_G|SA_I)

	/* TLB-entry for OCM */
	tlbentry(CFG_OCM_BASE, SZ_16K, 0x00040000, 4, AC_R|AC_W|AC_X|SA_I)

	/* TLB-entry for Local Configuration registers => peripherals */
	tlbentry(CFG_LOCAL_CONF_REGS, SZ_16M, CFG_LOCAL_CONF_REGS, 4, AC_R|AC_W|AC_X|SA_G|SA_I)

	/* AHB: Internal USB Peripherals (USB, SATA) */
	tlbentry(CFG_AHB_BASE, SZ_1M, 0xbff00000, 4, AC_R|AC_W|AC_X|SA_G|SA_I)

	tlbtab_end

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
	/*
	 * For NAND booting the first TLB has to be reconfigured to full size
	 * and with caching disabled after running from RAM!
	 */
#define TLB00	TLB0(CFG_BOOT_BASE_ADDR, SZ_256M)
#define TLB01	TLB1(CFG_BOOT_BASE_ADDR, 1)
#define TLB02	TLB2(AC_R|AC_W|AC_X|SA_G|SA_I)

	.globl	reconfig_tlb0
reconfig_tlb0:
	sync
	isync
	addi	r4,r0,0x0000		/* TLB entry #0 */
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
