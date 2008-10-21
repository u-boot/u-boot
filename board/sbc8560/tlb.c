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
/* TLB for CCSRBAR (IMMR) */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_1M, 1),

/* TLB for Local Bus stuff, just map the whole 512M */
/* note that the LBC SDRAM is cache-inhibit and guarded, like everything else */

	SET_TLB_ENTRY(1, 0xe0000000, 0xe0000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, 0xf0000000, 0xf0000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

#if !defined(CONFIG_SPD_EEPROM)
	SET_TLB_ENTRY(1, CONFIG_SYS_DDR_SDRAM_BASE, CONFIG_SYS_DDR_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 4, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000, CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 5, BOOKE_PAGESZ_256M, 1),
#endif

	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_RAM_ADDR, CONFIG_SYS_INIT_RAM_ADDR,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 6, BOOKE_PAGESZ_16K, 1),

	SET_TLB_ENTRY(1, CONFIG_SYS_PCI_MEM_PHYS, CONFIG_SYS_PCI_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 7, BOOKE_PAGESZ_256M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
