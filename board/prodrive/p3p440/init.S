/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <ppc_asm.tmpl>
#include <asm/mmu.h>
#include <config.h>
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
    tlbentry( 0xf0000000, SZ_256M, 0xf0000000, 1, AC_RWX | SA_IG)
    tlbentry( CONFIG_SYS_PERIPHERAL_BASE, SZ_256M, 0x40000000, 1, AC_RW | SA_IG)
    tlbentry( CONFIG_SYS_ISRAM_BASE, SZ_4K, 0x80000000, 0, AC_RWX )
    tlbentry( CONFIG_SYS_ISRAM_BASE + 0x1000, SZ_4K, 0x80001000, 0, AC_RWX )
    tlbentry( CONFIG_SYS_SDRAM_BASE, SZ_256M, 0x00000000, 0, AC_RWX | SA_IG )
    tlbentry( CONFIG_SYS_PCI_BASE, SZ_256M, 0x00000000, 2, AC_RW | SA_IG )
    tlbentry( CONFIG_SYS_PCI_MEMBASE, SZ_256M, 0x00000000, 3, AC_RW | SA_IG )
    tlbtab_end
