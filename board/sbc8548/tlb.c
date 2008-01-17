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
	 * TLB 0:	16M	Non-cacheable, guarded
	 * 0xff800000	16M	TLB for 8MB FLASH
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CFG_FLASH_BASE, CFG_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_16M, 1),

	/*
	 * TLB 1:	256M	Non-cacheable, guarded
	 * 0x80000000	256M	PCI1 MEM First half
	 */
	SET_TLB_ENTRY(1, CFG_PCI1_MEM_PHYS, CFG_PCI1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 * 0x90000000	256M	PCI1 MEM Second half
	 */
	SET_TLB_ENTRY(1, CFG_PCI1_MEM_PHYS + 0x10000000, CFG_PCI1_MEM_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 3:	256M Cacheable, non-guarded
	 * 0x0		256M DDR SDRAM
	 */
	#if !defined(CONFIG_SPD_EEPROM)
	SET_TLB_ENTRY(1, CFG_DDR_SDRAM_BASE, CFG_DDR_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 3, BOOKE_PAGESZ_256M, 1),
	#endif

	/*
	 * TLB 4:	64M	Non-cacheable, guarded
	 * 0xe0000000	1M	CCSRBAR
	 * 0xe2000000	16M	PCI1 IO
	 */
	SET_TLB_ENTRY(1, CFG_CCSRBAR, CFG_CCSRBAR,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 5:	64M	Cacheable, non-guarded
	 * 0xf0000000	64M	LBC SDRAM
	 */
	SET_TLB_ENTRY(1, CFG_LBC_SDRAM_BASE, CFG_LBC_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 5, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 6:	16M	Cacheable, non-guarded
	 * 0xf8000000	1M	7-segment LED display
	 * 0xf8100000	1M	User switches
	 * 0xf8300000	1M	Board revision
	 * 0xf8b00000	1M	EEPROM
	 */
	SET_TLB_ENTRY(1, CFG_EPLD_BASE, CFG_EPLD_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 6, BOOKE_PAGESZ_16M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
