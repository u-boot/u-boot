/*
 * Copyright 2004-2007 Freescale Semiconductor.
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

#define LAWAR_TRGT_PCI1         0x00000000
#define LAWAR_TRGT_PCIE1	0x00200000
#define LAWAR_TRGT_RIO          0x00c00000
#define LAWAR_TRGT_LBC          0x00400000
#define LAWAR_TRGT_DDR          0x00f00000

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
 * MAS2: epn, sharen, x0, x1, w, i, m, g, e
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
	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_CCSRBAR_DEFAULT), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_CCSRBAR_DEFAULT), 0,0,0,0,0,1,0,1,0,1)
#else
#error("Update the number of table entries in tlb1_entry")
#endif

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

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR), 0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR), 0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 4 * 1024),
			0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR + 4 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 8 * 1024),
			0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 8 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 12 * 1024),
			0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR + 12 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	/* TLB 1 Initializations */
	/*
	 * TLBe 0:	16M	Non-cacheable, guarded
	 * 0xff000000	16M	FLASH (upper half)
	 * Out of reset this entry is only 4K.
	 */
	.long TLB1_MAS0(1, 0, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_16M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_FLASH_BASE + 0x1000000),
			0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_FLASH_BASE + 0x1000000),
			0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLBe 1:	16M	Non-cacheable, guarded
	 * 0xfe000000	16M	FLASH (lower half)
	 */
	.long TLB1_MAS0(1, 1, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_16M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_FLASH_BASE), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_FLASH_BASE), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLBe 2:	1G	Non-cacheable, guarded
	 * 0x80000000	512M	PCI1 MEM
	 * 0xa0000000 	512M	PCIe MEM
	 */
	.long TLB1_MAS0(1, 2, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1G)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_PCI1_MEM_BASE), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_PCI1_MEM_BASE), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLBe 3:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	8M	PCI1 IO
	 * 0xe280_0000	8M	PCIe IO
	 */
	.long TLB1_MAS0(1, 3, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_CCSRBAR), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_CCSRBAR), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLBe 4:	64M	Cacheable, non-guarded
	 * 0xf000_0000	64M	LBC SDRAM
	 */
	.long TLB1_MAS0(1, 4, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_LBC_SDRAM_BASE), 0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_LBC_SDRAM_BASE), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLBe 5:	256K	Non-cacheable, guarded
	 * 0xf8000000	32K BCSR
	 * 0xf8008000	32K PIB (CS4)
	 * 0xf8010000	32K PIB (CS5)
	 */
	.long TLB1_MAS0(1, 5, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_256K)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_BCSR_BASE), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_BCSR_BASE), 0,0,0,0,0,1,0,1,0,1)

2:
	entry_end

/*
 * LAW(Local Access Window) configuration:
 *
 *0)   0x0000_0000   0x7fff_ffff     DDR                     2G
 *1)   0x8000_0000   0x9fff_ffff     PCI1 MEM                512MB
 *2)   0xa000_0000   0xbfff_ffff     PCIe MEM                512MB
 *-)   0xe000_0000   0xe00f_ffff     CCSR                    1M
 *3)   0xe200_0000   0xe27f_ffff     PCI1 I/O                8M
 *4)   0xe280_0000   0xe2ff_ffff     PCIe I/O                8M
 *5)   0xc000_0000   0xdfff_ffff     SRIO                    512MB
 *6.a) 0xf000_0000   0xf3ff_ffff     SDRAM                   64MB
 *6.b) 0xf800_0000   0xf800_7fff     BCSR                    32KB
 *6.c) 0xf800_8000   0xf800_ffff     PIB (CS4)		     32KB
 *6.d) 0xf801_0000   0xf801_7fff     PIB (CS5)		     32KB
 *6.e) 0xfe00_0000   0xffff_ffff     Flash                   32MB
 *
 *Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 *
 * The defines below are 1-off of the actual LAWAR0 usage.
 * So LAWAR3 define uses the LAWAR4 register in the ECM.
 */

#define LAWBAR0 0
#define LAWAR0  ((LAWAR_TRGT_DDR | (LAWAR_SIZE & LAWAR_SIZE_128M)) & ~LAWAR_EN)

#define LAWBAR1 ((CFG_PCI1_MEM_BASE>>12) & 0xfffff)
#define LAWAR1	(LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_512M))

#define LAWBAR2 ((CFG_PCIE1_MEM_BASE>>12) & 0xfffff)
#define LAWAR2	(LAWAR_EN | LAWAR_TRGT_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_512M))

#define LAWBAR3 ((CFG_PCI1_IO_PHYS>>12) & 0xfffff)
#define LAWAR3	(LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_8M))

#define LAWBAR4 ((CFG_PCIE1_IO_PHYS>>12) & 0xfffff)
#define LAWAR4  (LAWAR_EN | LAWAR_TRGT_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_8M))


#define LAWBAR5 ((CFG_SRIO_MEM_BASE>>12) & 0xfffff)
#define LAWAR5	(LAWAR_EN | LAWAR_TRGT_RIO | (LAWAR_SIZE & LAWAR_SIZE_512M))

/* LBC window - maps 256M.  That's SDRAM, BCSR, PIBs, and Flash */
#define LAWBAR6 ((CFG_LBC_SDRAM_BASE>>12) & 0xfffff)
#define LAWAR6	(LAWAR_EN | LAWAR_TRGT_LBC | (LAWAR_SIZE & LAWAR_SIZE_256M))

	.section .bootpg, "ax"
	.globl	law_entry

law_entry:
	entry_start
	.long (4f-3f)/8
3:
	.long LAWBAR0,LAWAR0,LAWBAR1,LAWAR1,LAWBAR2,LAWAR2,LAWBAR3,LAWAR3
	.long LAWBAR4,LAWAR4,LAWBAR5,LAWAR5,LAWBAR6,LAWAR6
4:
	entry_end
