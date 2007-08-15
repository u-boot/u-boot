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
#ifndef CONFIG_NAND_SPL
	tlbentry(CFG_BOOT_BASE_ADDR, SZ_256M, CFG_BOOT_BASE_ADDR, 0, AC_R|AC_W|AC_X|SA_G)
#else
	tlbentry(CFG_NAND_BOOT_SPL_SRC, SZ_4K, CFG_NAND_BOOT_SPL_SRC, 0, AC_R|AC_W|AC_X|SA_G)
	tlbentry(CFG_SDRAM_BASE, SZ_256M, CFG_SDRAM_BASE, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
#endif

	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry(CFG_INIT_RAM_ADDR, SZ_4K, CFG_INIT_RAM_ADDR, 0, AC_R|AC_W|AC_X|SA_G)

	/* PCI base & peripherals */
	tlbentry(CFG_PCI_BASE, SZ_256M, CFG_PCI_BASE, 0, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_NVRAM_BASE_ADDR, SZ_256M, CFG_NVRAM_BASE_ADDR, 0, AC_R|AC_W|AC_X|SA_W|SA_I)
	tlbentry(CFG_NAND_ADDR, SZ_4K, CFG_NAND_ADDR, 0, AC_R|AC_W|AC_X|SA_W|SA_I)

	/* PCI */
	tlbentry(CFG_PCI_MEMBASE, SZ_256M, CFG_PCI_MEMBASE, 0, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE1, SZ_256M, CFG_PCI_MEMBASE1, 0, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE2, SZ_256M, CFG_PCI_MEMBASE2, 0, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE3, SZ_256M, CFG_PCI_MEMBASE3, 0, AC_R|AC_W|SA_G|SA_I)

	/* USB 2.0 Device */
	tlbentry(CFG_USB_DEVICE, SZ_1K, CFG_USB_DEVICE, 0, AC_R|AC_W|SA_G|SA_I)

	tlbtab_end

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
	/*
	 * For NAND booting the first TLB has to be reconfigured to full size
	 * and with caching disabled after running from RAM!
	 */
#define TLB00	TLB0(CFG_BOOT_BASE_ADDR, SZ_256M)
#define TLB01	TLB1(CFG_BOOT_BASE_ADDR, 0)
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
