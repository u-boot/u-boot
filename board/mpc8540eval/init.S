/*
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

#define	entry_start \
	mflr	r1 	;	\
	bl	0f 	;

#define	entry_end \
0:	mflr	r0	;	\
	mtlr	r1	;	\
	blr		;

/* TLB1 entries configuration: */

	.section	.bootpg, "ax"
	.globl	tlb1_entry
tlb1_entry:
	entry_start

	.long 0x0a	/* the following data table uses a few of 16 TLB entries */

	.long TLB1_MAS0(1,1,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(((CFG_CCSRBAR>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_CCSRBAR>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

  #if defined(CFG_FLASH_PORT_WIDTH_16)
	.long TLB1_MAS0(1,2,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_4M)
	.long TLB1_MAS2(((CFG_FLASH_BASE>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_FLASH_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,3,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_4M)
	.long TLB1_MAS2((((CFG_FLASH_BASE+0x400000)>>12)&0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3((((CFG_FLASH_BASE+0x400000)>>12)&0xfffff),0,0,0,0,0,1,0,1,0,1)
  #else
	.long TLB1_MAS0(1,2,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_16M)
	.long TLB1_MAS2(((CFG_FLASH_BASE>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_FLASH_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,3,0)
	.long TLB1_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long TLB1_MAS2(0,0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(0,0,0,0,0,0,1,0,1,0,1)
  #endif

  #if !defined(CONFIG_SPD_EEPROM)
	.long TLB1_MAS0(1,4,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,0,0,0)
	.long TLB1_MAS3(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,5,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
	.long TLB1_MAS2((((CFG_DDR_SDRAM_BASE+0x4000000)>>12) & 0xfffff),0,0,0,0,0,0,0,0)
	.long TLB1_MAS3((((CFG_DDR_SDRAM_BASE+0x4000000)>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)
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
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
  #if defined(CONFIG_RAM_AS_FLASH)
	.long TLB1_MAS2(((CFG_LBC_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,1,0,1,0)
  #else
	.long TLB1_MAS2(((CFG_LBC_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,0,0,0)
  #endif
	.long TLB1_MAS3(((CFG_LBC_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,7,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_16K)
  #ifdef CONFIG_L2_INIT_RAM
	.long TLB1_MAS2(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,1,0,0,0,0)
  #else
	.long TLB1_MAS2(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,0,0,0,0,0)
  #endif
	.long TLB1_MAS3(((CFG_INIT_RAM_ADDR>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,8,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long TLB1_MAS2(((CFG_PCI_MEM_BASE>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_PCI_MEM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(1,9,0)
	.long TLB1_MAS1(1,1,0,0,BOOKE_PAGESZ_16K)
	.long TLB1_MAS2(((CFG_BCSR>>12) & 0xfffff),0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(((CFG_BCSR>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1)

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

/* LAW(Local Access Window) configuration:
 * 0000_0000-0800_0000: DDR(128M) -or- larger
 * f000_0000-f3ff_ffff: PCI(256M)
 * f400_0000-f7ff_ffff: RapidIO(128M)
 * f800_0000-ffff_ffff: localbus(128M)
 *   f800_0000-fbff_ffff: LBC SDRAM(64M)
 *   fc00_0000-fdef_ffff: LBC BCSR,RTC,etc(31M)
 *   fdf0_0000-fdff_ffff: CCSRBAR(1M)
 *   fe00_0000-ffff_ffff: Flash(32M)
 * Note: CCSRBAR and L2-as-SRAM don't need configure Local Access
 *       Window.
 * Note: If flash is 8M at default position(last 8M),no LAW needed.
 */

#if !defined(CONFIG_SPD_EEPROM)
#define LAWBAR0 ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff)
#define LAWAR0 	(LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & LAWAR_SIZE_128M))
#else
#define LAWBAR0 0
#define LAWAR0  ((LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & LAWAR_SIZE_128M)) & ~LAWAR_EN)
#endif

#define LAWBAR1 ((CFG_PCI_MEM_BASE>>12) & 0xfffff)
#define LAWAR1 	(LAWAR_EN | LAWAR_TRGT_IF_PCIX | (LAWAR_SIZE & LAWAR_SIZE_256M))

#if !defined(CONFIG_RAM_AS_FLASH)
#define LAWBAR2 ((CFG_LBC_SDRAM_BASE>>12) & 0xfffff)
#define LAWAR2 	(LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_128M))
#else
#define LAWBAR2 0
#define LAWAR2 ((LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_128M)) & ~LAWAR_EN)
#endif

	.section .bootpg, "ax"
	.globl	law_entry
law_entry:
	entry_start
	.long 0x03
	.long LAWBAR0,LAWAR0,LAWBAR1,LAWAR1,LAWBAR2,LAWAR2
	entry_end
