/*
 * Copyright (c) 2010, Applied Micro Circuits Corporation
 * Author: Tirumala R Marri <tmarri@apm.com>
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
#include <asm/ppc4xx.h>

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

	/* TLB 0 */
	tlbentry(CONFIG_SYS_BOOT_BASE_ADDR, SZ_16M, CONFIG_SYS_BOOT_BASE_ADDR,
	4, AC_RWX | SA_G)

	/* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
	tlbentry(CONFIG_SYS_INIT_RAM_ADDR, SZ_4K, CONFIG_SYS_INIT_RAM_ADDR,
			0, AC_RWX | SA_G)

	/* TLB-entry for OCM */
	tlbentry(CONFIG_SYS_OCM_BASE, SZ_64K, 0x00040000, 4,
			AC_RWX | SA_I)

	/* TLB-entry for Local Configuration registers => peripherals */
	tlbentry(CONFIG_SYS_PERIPHERAL_BASE, SZ_16K,
			CONFIG_SYS_PERIPHERAL_BASE, 4, AC_RWX | SA_IG)
	tlbtab_end
