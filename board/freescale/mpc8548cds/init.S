/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Copyright 2002,2003, Motorola Inc.
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
#if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
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
#else
#error("Update the number of table entries in tlb1_entry")
#endif

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
	 * TLB 0:	16M	Non-cacheable, guarded
	 * 0xff000000	16M	FLASH
	 * Out of reset this entry is only 4K.
	 */
	.long FSL_BOOKE_MAS0(1, 0, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_16M)
	.long FSL_BOOKE_MAS2(CFG_BOOT_BLOCK, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_BOOT_BLOCK, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 1:	1G	Non-cacheable, guarded
	 * 0x80000000	1G	PCI1/PCIE  8,9,a,b
	 */
	.long FSL_BOOKE_MAS0(1, 1, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1G)
	.long FSL_BOOKE_MAS2(CFG_PCI_PHYS, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI_PHYS, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

#ifdef CFG_RIO_MEM_PHYS
	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 */
	.long FSL_BOOKE_MAS0(1, 2, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_RIO_MEM_PHYS, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_RIO_MEM_PHYS,	0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 */
	.long FSL_BOOKE_MAS0(1, 3, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_RIO_MEM_PHYS + 0x10000000, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_RIO_MEM_PHYS + 0x10000000, 0, (MAS3_SX|MAS3_SW|MAS3_SR))
#endif
	/*
	 * TLB 5:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	1M	PCI1 IO
	 * 0xe210_0000	1M	PCI2 IO
	 * 0xe300_0000	1M	PCIe IO
	 */
	.long FSL_BOOKE_MAS0(1, 5, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 6:	64M	Cacheable, non-guarded
	 * 0xf000_0000	64M	LBC SDRAM
	 */
	.long FSL_BOOKE_MAS0(1, 6, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_LBC_CACHE_BASE, 0)
	.long FSL_BOOKE_MAS3(CFG_LBC_CACHE_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

	/*
	 * TLB 7:	64M	Non-cacheable, guarded
	 * 0xf8000000	64M	CADMUS registers, relocated L2SRAM
	 */
	.long FSL_BOOKE_MAS0(1, 7, 0)
	.long FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_LBC_NONCACHE_BASE, (MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_LBC_NONCACHE_BASE, 0, (MAS3_SX|MAS3_SW|MAS3_SR))

2:
	entry_end

/*
 * LAW(Local Access Window) configuration:
 *
 * 0x0000_0000     0x7fff_ffff     DDR                     2G
 * 0x8000_0000     0x9fff_ffff     PCI1 MEM                512M
 * 0xa000_0000     0xbfff_ffff     PCIe MEM                512M
 * 0xc000_0000     0xdfff_ffff     RapidIO                 512M
 * 0xe000_0000     0xe000_ffff     CCSR                    1M
 * 0xe200_0000     0xe10f_ffff     PCI1 IO                 1M
 * 0xe280_0000     0xe20f_ffff     PCI2 IO                 1M
 * 0xe300_0000     0xe30f_ffff     PCIe IO                 1M
 * 0xf000_0000     0xf3ff_ffff     SDRAM                   64M
 * 0xf800_0000     0xf80f_ffff     NVRAM/CADMUS (*)        1M
 * 0xff00_0000     0xff7f_ffff     FLASH (2nd bank)        8M
 * 0xff80_0000     0xffff_ffff     FLASH (boot bank)       8M
 *
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 *
 * LAW 0 is reserved for boot mapping
 */

	.section .bootpg, "ax"
	.globl	law_entry
law_entry:
	entry_start

	.long (4f-3f)/8
3:
	.long  0
	.long  (LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & LAWAR_SIZE_128M)) & ~LAWAR_EN

#ifdef CFG_PCI1_MEM_PHYS
	.long	(CFG_PCI1_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_512M)

	.long	(CFG_PCI1_IO_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_1M)
#endif

#ifdef CFG_PCI2_MEM_PHYS
	.long	(CFG_PCI2_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_512M)

	.long	(CFG_PCI2_IO_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_1M)
#endif

#ifdef CFG_PCIE1_MEM_PHYS
	.long	(CFG_PCIE1_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_512M)

	.long	(CFG_PCIE1_IO_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_1M)
#endif

	/* LBC window - maps 256M 0xf0000000 -> 0xffffffff */
	.long	(CFG_LBC_CACHE_BASE>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_256M)

#ifdef CFG_RIO_MEM_PHYS
	.long	(CFG_RIO_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_IF_RIO | (LAWAR_SIZE & LAWAR_SIZE_512M)
#endif
4:
	entry_end
