/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <asm/mmu.h>

struct fsl_e_tlb_entry tlb_table[] = {
	SET_TLB_ENTRY(1, CFG_CCSRBAR, CFG_CCSRBAR,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_1M, 1),

  #if defined(CFG_FLASH_PORT_WIDTH_16)
	SET_TLB_ENTRY(1, CFG_FLASH_BASE, CFG_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_4M, 1),
	SET_TLB_ENTRY(1, CFG_FLASH_BASE + 0x400000, CFG_FLASH_BASE + 0x400000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_4M, 1),
  #else
	SET_TLB_ENTRY(1, CFG_FLASH_BASE, CFG_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_16M, 1),
  #endif

  #if !defined(CONFIG_SPD_EEPROM)
	SET_TLB_ENTRY(1, CFG_DDR_SDRAM_BASE, CFG_DDR_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 4, BOOKE_PAGESZ_64M, 1),

	SET_TLB_ENTRY(1, CFG_DDR_SDRAM_BASE + 0x4000000, CFG_DDR_SDRAM_BASE + 0x4000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 5, BOOKE_PAGESZ_64M, 1),
  #endif

	SET_TLB_ENTRY(1, CFG_LBC_SDRAM_BASE, CFG_LBC_SDRAM_BASE,
  #if defined(CONFIG_RAM_AS_FLASH)
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
  #else
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
  #endif
		      0, 6, BOOKE_PAGESZ_64M, 1),

	SET_TLB_ENTRY(1, CFG_INIT_RAM_ADDR, CFG_INIT_RAM_ADDR,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 7, BOOKE_PAGESZ_16K, 1),

	SET_TLB_ENTRY(1, CFG_PCI_MEM_PHYS, CFG_PCI_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 8, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, CFG_BCSR, CFG_BCSR,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 9, BOOKE_PAGESZ_16K, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
