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

	.long FSL_BOOKE_MAS0(1,1,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR,0,(MAS3_SX|MAS3_SW|MAS3_SR))

  #if defined(CFG_FLASH_PORT_WIDTH_16)
	.long FSL_BOOKE_MAS0(1,2,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_4M)
	.long FSL_BOOKE_MAS2(CFG_FLASH_BASE,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_FLASH_BASE,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,3,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_4M)
	.long FSL_BOOKE_MAS2(CFG_FLASH_BASE+0x400000,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_FLASH_BASE+0x400000,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #else
	.long FSL_BOOKE_MAS0(1,2,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_16M)
	.long FSL_BOOKE_MAS2(CFG_FLASH_BASE,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_FLASH_BASE,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,3,0)
	.long FSL_BOOKE_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(0,0)
	.long FSL_BOOKE_MAS3(0,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #endif

  #if !defined(CONFIG_SPD_EEPROM)
	.long FSL_BOOKE_MAS0(1,4,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_DDR_SDRAM_BASE,0)
	.long FSL_BOOKE_MAS3(CFG_DDR_SDRAM_BASE,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,5,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
	.long FSL_BOOKE_MAS2(CFG_DDR_SDRAM_BASE+0x4000000,0)
	.long FSL_BOOKE_MAS3(CFG_DDR_SDRAM_BASE+0x4000000,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #else
	.long FSL_BOOKE_MAS0(1,4,0)
	.long FSL_BOOKE_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(0,0)
	.long FSL_BOOKE_MAS3(0,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,5,0)
	.long FSL_BOOKE_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(0,0)
	.long FSL_BOOKE_MAS3(0,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #endif

	.long FSL_BOOKE_MAS0(1,6,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_64M)
  #if defined(CONFIG_RAM_AS_FLASH)
	.long FSL_BOOKE_MAS2(CFG_LBC_SDRAM_BASE,(MAS2_I|MAS2_G))
  #else
	.long FSL_BOOKE_MAS2(CFG_LBC_SDRAM_BASE,0)
  #endif
	.long FSL_BOOKE_MAS3(CFG_LBC_SDRAM_BASE,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,7,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_16K)
  #ifdef CONFIG_L2_INIT_RAM
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR,0,0,0,1,0,0,0,0)
  #else
	.long FSL_BOOKE_MAS2(CFG_INIT_RAM_ADDR,0)
  #endif
	.long FSL_BOOKE_MAS3(CFG_INIT_RAM_ADDR,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,8,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_256M)
	.long FSL_BOOKE_MAS2(CFG_PCI_MEM_PHYS,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_PCI_MEM_PHYS,0,(MAS3_SX|MAS3_SW|MAS3_SR))

	.long FSL_BOOKE_MAS0(1,9,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_16K)
	.long FSL_BOOKE_MAS2(CFG_BCSR,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_BCSR,0,(MAS3_SX|MAS3_SW|MAS3_SR))

  #if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
	.long FSL_BOOKE_MAS0(1,15,0)
	.long FSL_BOOKE_MAS1(1,1,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(CFG_CCSRBAR_DEFAULT,(MAS2_I|MAS2_G))
	.long FSL_BOOKE_MAS3(CFG_CCSRBAR_DEFAULT,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #else
	.long FSL_BOOKE_MAS0(1,15,0)
	.long FSL_BOOKE_MAS1(0,0,0,0,BOOKE_PAGESZ_1M)
	.long FSL_BOOKE_MAS2(0,0)
	.long FSL_BOOKE_MAS3(0,0,(MAS3_SX|MAS3_SW|MAS3_SR))
  #endif
	entry_end
