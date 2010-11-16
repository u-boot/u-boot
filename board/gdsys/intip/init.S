/*
 * (C) Copyright 2009
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on board/amcc/canyonlands/init.S
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

#include <asm-offsets.h>
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

	/*
	 * BOOT_CS (FLASH) must be first. Before relocation SA_I can be off to
	 * use the speed up boot process. It is patched after relocation to
	 * enable SA_I
	 */
	tlbentry(CONFIG_SYS_BOOT_BASE_ADDR, SZ_16M, CONFIG_SYS_BOOT_BASE_ADDR,
		4, AC_RWX | SA_G) /* TLB 0 */

	/*
	 * TLB entries for SDRAM are not needed on this platform.
	 * They are dynamically generated in the SPD DDR(2) detection
	 * routine.
	 */

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry(CONFIG_SYS_INIT_RAM_ADDR, SZ_4K, CONFIG_SYS_INIT_RAM_ADDR,
		0, AC_RWX | SA_G)
#endif

	tlbentry(CONFIG_SYS_PCI_BASE, SZ_256M, 0x00000000, 0xC,
		AC_RW | SA_IG)
	tlbentry(CONFIG_SYS_PCI_MEMBASE, SZ_256M, 0x20000000, 0xC,
		AC_RW | SA_IG)

	/* TLB-entry for NVRAM */
	tlbentry(CONFIG_SYS_NVRAM_BASE, SZ_1M, CONFIG_SYS_NVRAM_BASE, 4,
		AC_RW | SA_IG)

	/* TLB-entry for UART */
	tlbentry(CONFIG_SYS_UART_BASE, SZ_16K, CONFIG_SYS_UART_BASE, 4,
		AC_RW | SA_IG)

	/* TLB-entry for IO */
	tlbentry(CONFIG_SYS_IO_BASE, SZ_16K, CONFIG_SYS_IO_BASE, 4,
		AC_RW | SA_IG)

	/* TLB-entry for OCM */
	tlbentry(CONFIG_SYS_OCM_BASE, SZ_1M, 0x00000000, 4,
		AC_RWX | SA_I)

	/* TLB-entry for Local Configuration registers => peripherals */
	tlbentry(CONFIG_SYS_LOCAL_CONF_REGS, SZ_16M, CONFIG_SYS_LOCAL_CONF_REGS,
		4, AC_RWX | SA_IG)

	/* AHB: Internal USB Peripherals (USB, SATA) */
	tlbentry(CONFIG_SYS_AHB_BASE, SZ_1M, 0xbff00000, 4,
		AC_RWX | SA_IG)

	tlbtab_end
