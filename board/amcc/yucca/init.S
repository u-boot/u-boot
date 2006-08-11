/*
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
/* port to AMCC 440SPE evaluatioon board - SG April 12,2005  */

#include <ppc_asm.tmpl>
#include <config.h>

/* General */
#define TLB_VALID   0x00000200

/* Supported page sizes */

#define SZ_1K	0x00000000
#define SZ_4K	0x00000010
#define SZ_16K	0x00000020
#define SZ_64K	0x00000030
#define SZ_256K	0x00000040
#define SZ_1M	0x00000050
#define SZ_16M	0x00000070
#define SZ_256M	0x00000090

/* Storage attributes */
#define SA_W	0x00000800	/* Write-through */
#define SA_I	0x00000400	/* Caching inhibited */
#define SA_M	0x00000200	/* Memory coherence */
#define SA_G	0x00000100	/* Guarded */
#define SA_E	0x00000080	/* Endian */

/* Access control */
#define AC_X	0x00000024	/* Execute */
#define AC_W	0x00000012	/* Write */
#define AC_R	0x00000009	/* Read */

/* Some handy macros */

#define EPN(e)		((e) & 0xfffffc00)
#define TLB0(epn,sz)	((EPN((epn)) | (sz) | TLB_VALID ))
#define TLB1(rpn,erpn)	(((rpn) & 0xfffffc00) | (erpn))
#define TLB2(a)		((a) & 0x00000fbf)

#define tlbtab_start\
	mflr	r1	;\
	bl	0f	;

#define tlbtab_end\
	.long 0, 0, 0	;\
0:	mflr	r0	;\
	mtlr	r1	;\
	blr		;

#define tlbentry(epn,sz,rpn,erpn,attr)\
	.long TLB0(epn,sz),TLB1(rpn,erpn),TLB2(attr)

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

/**************************************************************************
 * TLB table for revA
 *************************************************************************/
	.globl tlbtabA
tlbtabA:
	tlbtab_start
	tlbentry(0xfff00000, SZ_16M, 0xfff00000, 4, AC_R|AC_W|AC_X|SA_G)

	tlbentry(CFG_SDRAM_BASE, SZ_256M, 0x00000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x10000000, SZ_256M, 0x10000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x20000000, SZ_256M, 0x20000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x30000000, SZ_256M, 0x30000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)

	tlbentry(CFG_ISRAM_BASE, SZ_256K, 0x00000000, 4, AC_R|AC_W|AC_X|SA_I)
	tlbentry(CFG_FPGA_BASE, SZ_1K, 0xE2000000, 4,AC_R|AC_W|SA_I)

	tlbentry(CFG_OPER_FLASH, SZ_16M, 0xE7000000, 4,AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_PERIPHERAL_BASE, SZ_4K, 0xF0000000, 4, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_PCI_BASE, SZ_256M, 0x00000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE, SZ_256M, 0x10000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE_MEMBASE, SZ_256M, 0xB0000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE_BASE, SZ_16K, 0x20000000, 0xC, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_PCIE0_CFGBASE, SZ_1K, 0x40000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_CFGBASE, SZ_1K, 0x80000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE2_CFGBASE, SZ_1K, 0xC0000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE0_XCFGBASE, SZ_1K, 0x50000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_XCFGBASE, SZ_1K, 0x90000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE2_XCFGBASE, SZ_1K, 0xD0000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbtab_end

/**************************************************************************
 * TLB table for revB
 *
 * Notice: revB of the 440SPe chip is very strict about PLB real addresses
 * and ranges to be mapped for config space: it seems to only work with
 * d_nnnn_nnnn range (hangs the core upon config transaction attempts when
 * set otherwise) while revA uses c_nnnn_nnnn.
 *************************************************************************/
	.globl tlbtabB
tlbtabB:
	tlbtab_start
	tlbentry(0xfff00000, SZ_16M, 0xfff00000, 4, AC_R|AC_W|AC_X|SA_G)

	tlbentry(CFG_SDRAM_BASE, SZ_256M, 0x00000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x10000000, SZ_256M, 0x10000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x20000000, SZ_256M, 0x20000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_SDRAM_BASE + 0x30000000, SZ_256M, 0x30000000, 0, AC_R|AC_W|AC_X|SA_G|SA_I)

	tlbentry(CFG_ISRAM_BASE, SZ_256K, 0x00000000, 4, AC_R|AC_W|AC_X|SA_I)
	tlbentry(CFG_FPGA_BASE, SZ_1K, 0xE2000000, 4,AC_R|AC_W|SA_I)

	tlbentry(CFG_OPER_FLASH, SZ_16M, 0xE7000000, 4,AC_R|AC_W|AC_X|SA_G|SA_I)
	tlbentry(CFG_PERIPHERAL_BASE, SZ_4K, 0xF0000000, 4, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_PCI_BASE, SZ_256M, 0x00000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCI_MEMBASE, SZ_256M, 0x10000000, 0xC, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE_MEMBASE, SZ_256M, 0xB0000000, 0xD, AC_R|AC_W|SA_G|SA_I)

	tlbentry(CFG_PCIE0_CFGBASE, SZ_1K, 0x00100000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_CFGBASE, SZ_1K, 0x20100000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE2_CFGBASE, SZ_1K, 0x40100000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE0_XCFGBASE, SZ_1K, 0x10000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE1_XCFGBASE, SZ_1K, 0x30000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbentry(CFG_PCIE2_XCFGBASE, SZ_1K, 0x50000000, 0xD, AC_R|AC_W|SA_G|SA_I)
	tlbtab_end
