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
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR, CONFIG_SYS_INIT_RAM_ADDR,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/*
	 * TLB 0:	16M	Non-cacheable, guarded
	 * 0xff000000	16M	FLASH
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_BOOT_BLOCK, CONFIG_SYS_BOOT_BLOCK,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_16M, 1),

	/*
	 * TLB 1:	1G	Non-cacheable, guarded
	 * 0x80000000	1G	PCI1/PCIE  8,9,a,b
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI_VIRT, CONFIG_SYS_PCI_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_1G, 1),

#ifdef CONFIG_SYS_RIO_MEM_PHYS
	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_RIO_MEM_VIRT, CONFIG_SYS_RIO_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_RIO_MEM_VIRT + 0x10000000, CONFIG_SYS_RIO_MEM_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),
#endif
	/*
	 * TLB 5:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	1M	PCI1 IO
	 * 0xe210_0000	1M	PCI2 IO
	 * 0xe300_0000	1M	PCIe IO
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 6:	64M	Cacheable, non-guarded
	 * 0xf000_0000	64M	LBC SDRAM
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_LBC_CACHE_BASE, CONFIG_SYS_LBC_CACHE_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 6, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 7:	64M	Non-cacheable, guarded
	 * 0xf8000000	64M	CADMUS registers, relocated L2SRAM
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_LBC_NONCACHE_BASE, CONFIG_SYS_LBC_NONCACHE_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 7, BOOKE_PAGESZ_64M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
