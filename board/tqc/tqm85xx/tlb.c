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
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CFG_INIT_RAM_ADDR, CFG_INIT_RAM_ADDR,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_INIT_RAM_ADDR + 4 * 1024 , CFG_INIT_RAM_ADDR + 4 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_INIT_RAM_ADDR + 8 * 1024 , CFG_INIT_RAM_ADDR + 8 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_INIT_RAM_ADDR + 12 * 1024 , CFG_INIT_RAM_ADDR + 12 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),


	/*
	 * TLB 0, 1:	128M	Non-cacheable, guarded
	 * 0xf8000000	128M	FLASH
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CFG_FLASH_BASE, CFG_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_64M, 1),
	SET_TLB_ENTRY(1, CFG_FLASH_BASE + 0x4000000, CFG_FLASH_BASE + 0x4000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 * 0x80000000	256M	PCI1 MEM First half
	 */
	SET_TLB_ENTRY(1, CFG_PCI1_MEM_PHYS, CFG_PCI1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 * 0x90000000	256M	PCI1 MEM Second half
	 */
	SET_TLB_ENTRY(1, CFG_PCI1_MEM_PHYS + 0x10000000, CFG_PCI1_MEM_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 4:	256M	Non-cacheable, guarded
	 * 0xc0000000	256M	Rapid IO MEM First half
	 */
	SET_TLB_ENTRY(1, CFG_RIO_MEM_BASE, CFG_RIO_MEM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 5:	256M	Non-cacheable, guarded
	 * 0xd0000000	256M	Rapid IO MEM Second half
	 */
	SET_TLB_ENTRY(1, CFG_RIO_MEM_BASE + 0x10000000, CFG_RIO_MEM_BASE + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 6:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	16M	PCI1 IO
	 */
	SET_TLB_ENTRY(1, CFG_CCSRBAR, CFG_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 6, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 7+8:	512M	DDR, cache disabled (needed for memory test)
	 * 0x00000000  512M	DDR System memory
	 * Without SPD EEPROM configured DDR, this must be setup manually.
	 * Make sure the TLB count at the top of this table is correct.
	 * Likely it needs to be increased by two for these entries.
	 */
	SET_TLB_ENTRY(1, CFG_DDR_SDRAM_BASE, CFG_DDR_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 7, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, CFG_DDR_SDRAM_BASE + 0x10000000, CFG_DDR_SDRAM_BASE + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 8, BOOKE_PAGESZ_256M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
