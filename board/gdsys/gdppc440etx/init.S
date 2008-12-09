/*
* (C) Copyright 2008
* Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
*
* based on board/amcc/yosemite/init.S
* original Copyright not specified there
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

    /*
     * BOOT_CS (FLASH) must be first. Before relocation SA_I can be off to use
     * the speed up boot process. It is patched after relocation to enable SA_I
     */
    tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M, CONFIG_SYS_BOOT_BASE_ADDR,
	0, AC_R|AC_W|AC_X|SA_G/*|SA_I*/)

    /* TLB-entry for init-ram in dcache (SA_I must be turned off!) */
    tlbentry( CONFIG_SYS_INIT_RAM_ADDR, SZ_64K, CONFIG_SYS_INIT_RAM_ADDR,
	0, AC_R|AC_W|AC_X|SA_G )

    tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, CONFIG_SYS_SDRAM_BASE,
	0, AC_R|AC_W|AC_X|SA_G|SA_I )
    tlbentry( CONFIG_SYS_PCI_BASE, SZ_256M, CONFIG_SYS_PCI_BASE,
	0, AC_R|AC_W|SA_G|SA_I )

    /* PCI */
    tlbentry( CONFIG_SYS_PCI_MEMBASE, SZ_256M, CONFIG_SYS_PCI_MEMBASE,
	0, AC_R|AC_W|SA_G|SA_I )
    tlbentry( CONFIG_SYS_PCI_MEMBASE1, SZ_256M, CONFIG_SYS_PCI_MEMBASE1,
	0, AC_R|AC_W|SA_G|SA_I )
    tlbentry( CONFIG_SYS_PCI_MEMBASE2, SZ_256M, CONFIG_SYS_PCI_MEMBASE2,
	0, AC_R|AC_W|SA_G|SA_I )
    tlbentry( CONFIG_SYS_PCI_MEMBASE3, SZ_256M, CONFIG_SYS_PCI_MEMBASE3,
	0, AC_R|AC_W|SA_G|SA_I )

    tlbtab_end
