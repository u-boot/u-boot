/*
* Copyright (C) 2002,2003, Motorola Inc.
* Xianghua Xiao <X.Xiao@motorola.com>
*
* (C) Copyright 2004 Wind River Systems Inc <www.windriver.com>.
* Added support for Wind River SBC8560 board
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

#define	entry_start \
	mflr	r1 	;	\
	bl	0f 	;

#define	entry_end \
0:	mflr	r0	;	\
	mtlr	r1	;	\
	blr		;


/* LAW(Local Access Window) configuration:
 * 0000_0000-0800_0000: DDR(512M) -or- larger
 * c000_0000-cfff_ffff: PCI(256M)
 * d000_0000-dfff_ffff: RapidIO(256M)
 * e000_0000-ffff_ffff: localbus(512M)
 *   e000_0000-e3ff_ffff: LBC 64M, 32-bit flash on CS6
 *   e400_0000-e7ff_ffff: LBC 64M, 32-bit flash on CS1
 *   e800_0000-efff_ffff: LBC 128M, nothing here
 *   f000_0000-f3ff_ffff: LBC 64M, SDRAM on CS3
 *   f400_0000-f7ff_ffff: LBC 64M, SDRAM on CS4
 *   f800_0000-fdff_ffff: LBC 64M, nothing here
 *   fc00_0000-fcff_ffff: LBC 16M, CSR,RTC,UART,etc on CS5
 *   fd00_0000-fdff_ffff: LBC 16M, nothing here
 *   fe00_0000-feff_ffff: LBC 16M, nothing here
 *   ff00_0000-ff6f_ffff: LBC 7M, nothing here
 *   ff70_0000-ff7f_ffff: CCSRBAR 1M
 *   ff80_0000-ffff_ffff: LBC 8M, 8-bit flash on CS0
 * Note: CCSRBAR and L2-as-SRAM don't need configure Local Access
 *       Window.
 * Note: If flash is 8M at default position(last 8M),no LAW needed.
 */

#if !defined(CONFIG_SPD_EEPROM)
  #define LAWBAR0 ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff)
  #define LAWAR0  (LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & LAWAR_SIZE_512M))
#else
  #define LAWBAR0 0
  #define LAWAR0  ((LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & LAWAR_SIZE_512M)) & ~LAWAR_EN)
#endif

#define LAWBAR1 ((CFG_PCI_MEM_BASE>>12) & 0xfffff)
#define LAWAR1  (LAWAR_EN | LAWAR_TRGT_IF_PCIX | (LAWAR_SIZE & LAWAR_SIZE_256M))

#define LAWBAR2 ((0xe0000000>>12) & 0xfffff)
#define LAWAR2  (LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_512M))

	.section .bootpg, "ax"
	.globl  law_entry
law_entry:
	entry_start
	.long 0x03
	.long LAWBAR0,LAWAR0,LAWBAR1,LAWAR1,LAWBAR2,LAWAR2
	entry_end

/* TLB1 entries configuration: */

	.section	.bootpg, "ax"
	.globl		tlb1_entry

tlb1_entry:
	entry_start

	.long 0x08	/* the following data table uses a few of 16 TLB entries */

/* TLB for CCSRBAR (IMMR) */

	.long TLB1_MAS0(1,1,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(((CFG_CCSRBAR>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_CCSRBAR>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

/* TLB for Local Bus stuff, just map the whole 512M */
/* note that the LBC SDRAM is cache-inhibit and guarded, like everything else */

	.long TLB1_MAS0(1,2,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2(((0xe0000000>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((0xe0000000>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,3,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2(((0xf0000000>>12)&0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((0xf0000000>>12)&0xfffff),0,0,0,0,0,1,0,1,0,1)

#if !defined(CONFIG_SPD_EEPROM)
	.long TLB1_MAS0(1,4,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,5,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2((((CFG_DDR_SDRAM_BASE+0x10000000)>>12) & 0xfffff),0,0,0,0,0,0,0,0)
	.long TLB1_MAS3((((CFG_DDR_SDRAM_BASE+0x10000000)>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)
#else
	.long TLB1_MAS0(1,4,0)
	.long TLB1_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(0,0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(0,0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,5,0)
	.long TLB1_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(0,0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(0,0,0,0,0,0,1,0,1,0,1)
#endif

	.long TLB1_MAS0(1,6,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_16K)
#ifdef CONFIG_L2_INIT_RAM
	.long TLB1_MAS2(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,1,0,0,0,0)
#else
	.long TLB1_MAS2(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,0,0,0,0,0)
#endif
	.long TLB1_MAS3(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,7,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2(((CFG_PCI_MEM_BASE>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_PCI_MEM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

#if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
	.long TLB1_MAS0(1,15,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(((CFG_CCSRBAR_DEFAULT>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_CCSRBAR_DEFAULT>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)
#else
	.long TLB1_MAS0(1,15,0)
	.long TLB1_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(0,0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(0,0,0,0,0,0,1,0,1,0,1)
#endif
	entry_end
