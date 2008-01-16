/*
 * Copyright 2007 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#define	entry_start \
	mflr	r1	;	\
	bl	0f	;

#define	entry_end \
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
	.long (2f-1f)/16
1:
	/*
	 * TLB0		4K	Non-cacheable, guarded
	 * 0xff700000	4K	Initial CCSRBAR mapping
	 *
	 * This ends up at a TLB0 Index==0 entry, and must not collide
	 * with other TLB0 Entries.
	 */
	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR_DEFAULT, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR_DEFAULT, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB0		16K	Cacheable, guarded
	 * Temporary Global data for initialization
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
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR, (MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 4 * 1024, (MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 4 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 8 * 1024, (MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 8 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(0, 0, 0)
	.long FSL_BOOKE_MAS1(1, 0, 0, 0, 0)
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR + 12 * 1024, (MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR + 12 * 1024, 0, (MAS3_SX|MAS3_SW|MAS3_SR))


	/*
	 * TLB 0:	64M	Non-cacheable, guarded
	 * 0xfc000000	64M	Covers FLASH at 0xFE800000 and 0xFF800000
	 * Out of reset this entry is only 4K.
	 */
	.long FSL_BOOKE_MAS0(1, 0, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_BOOT_BLOCK, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_BOOT_BLOCK, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 1:	1G	Non-cacheable, guarded
	 * 0x80000000	1G	PCIE  8,9,a,b
	 */
	.long FSL_BOOKE_MAS0(1, 1, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1G)
	.long FSL_BOOKE_MAS2(CFG_PCIE_PHYS, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCIE_PHYS, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 */
	.long FSL_BOOKE_MAS0(1, 2, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_PCI_PHYS, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI_PHYS, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 */
	.long FSL_BOOKE_MAS0(1, 3, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_PCI_PHYS + 0x10000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI_PHYS + 0x10000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 4:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe100_0000	255M	PCI IO range
	 */
	.long FSL_BOOKE_MAS0(1, 4, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

#ifdef CFG_LBC_CACHE_BASE
	/*
	 * TLB 5:	64M	Cacheable, non-guarded
	 */
	.long FSL_BOOKE_MAS0(1, 5, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_LBC_CACHE_BASE, 0)
	.long FSL_BOOKE_MAS3(CFG_LBC_CACHE_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))
#endif
	/*
	 * TLB 6:	64M	Non-cacheable, guarded
	 * 0xf8000000	64M	PIXIS 0xF8000000 - 0xFBFFFFFF
	 */
	.long FSL_BOOKE_MAS0(1, 6, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_LBC_NONCACHE_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_LBC_NONCACHE_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))
2:
	entry_end
