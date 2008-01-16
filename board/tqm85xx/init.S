/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright (C) 2002,2003, Motorola Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
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
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <config.h>
#include <mpc85xx.h>


/*
 * TLB0 and TLB1 Entries
 *
 * Out of reset, TLB1's Entry 0 maps the highest 4K for CCSRBAR.
 * However, CCSRBAR is then relocated to CFG_CCSRBAR right after
 * these TLB entries are established.
 *
 * The TLB entries for DDR are dynamically setup in spd_sdram()
 * and use TLB1 Entries 8 through 15 as needed according to the
 * size of DDR memory.
 *
 * MAS0: tlbsel, esel, nv
 * MAS1: valid, iprot, tid, ts, tsize
 * MAS2: epn, x0, x1, w, i, m, g, e
 * MAS3: rpn, u0-u3, ux, sx, uw, sw, ur, sr
 */

#define entry_start \
	mflr	r1	;	\
	bl	0f	;

#define entry_end \
0:	mflr	r0	;	\
	mtlr	r1	;	\
	blr		;


	.section	.bootpg, "ax"
	.globl	tlb1_entry
tlb1_entry:
	entry_start

	/*
	 * Number of TLB0 and TLB1 entries in the following table
	 */
	.long 13

	/*
	 * TLB0		16K	Cacheable, non-guarded
	 * 0xd001_0000	16K	Temporary Global data for initialization
	 *
	 * Use four 4K TLB0 entries.  These entries must be cacheable
	 * as they provide the bootstrap memory before the memory
	 * controler and real memory have been configured.
	 *
	 * These entries end up at TLB0 Indicies 0x10, 0x14, 0x18 and 0x1c,
	 * and must not collide with other TLB0 entries.
	 */
	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR, 0)
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 4 * 1024, 0)
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 4 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 8 * 1024, 0)
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 8 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 12 * 1024, 0)
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 12 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))


	/*
	 * TLB 0, 1:	128M	Non-cacheable, guarded
	 * 0xf8000000	128M	FLASH
	 * Out of reset this entry is only 4K.
	 */
	.long FSL_BOOKE_MAS0(1, 1, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_FLASH_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_FLASH_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))
	.long FSL_BOOKE_MAS0(1, 0, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_FLASH_BASE+0x4000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_FLASH_BASE+0x4000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 * 0x80000000	256M	PCI1 MEM First half
	 */
	.long FSL_BOOKE_MAS0(1, 2, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_PCI1_MEM_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI1_MEM_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 * 0x90000000	256M	PCI1 MEM Second half
	 */
	.long FSL_BOOKE_MAS0(1, 3, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_PCI1_MEM_BASE + 0x10000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI1_MEM_BASE + 0x10000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 4:	256M	Non-cacheable, guarded
	 * 0xc0000000	256M	Rapid IO MEM First half
	 */
	.long FSL_BOOKE_MAS0(1, 4, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_RIO_MEM_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_RIO_MEM_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 5:	256M	Non-cacheable, guarded
	 * 0xd0000000	256M	Rapid IO MEM Second half
	 */
	.long FSL_BOOKE_MAS0(1, 5, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_RIO_MEM_BASE + 0x10000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_RIO_MEM_BASE + 0x10000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 6:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	16M	PCI1 IO
	 */
	.long FSL_BOOKE_MAS0(1, 6, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 7+8:	512M	DDR, cache disabled (needed for memory test)
	 * 0x00000000  512M	DDR System memory
	 * Without SPD EEPROM configured DDR, this must be setup manually.
	 * Make sure the TLB count at the top of this table is correct.
	 * Likely it needs to be increased by two for these entries.
	 */
	.long FSL_BOOKE_MAS0(1, 7, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_DDR_SDRAM_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_DDR_SDRAM_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))
	.long FSL_BOOKE_MAS0(1, 8, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_DDR_SDRAM_BASE+0x10000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_DDR_SDRAM_BASE+0x10000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	entry_end
