/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	 * TLB 0:	64M	Non-cacheable, guarded
	 * 0xfc000000	6M4	FLASH
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 1:	256M	Non-cacheable, guarded
	 * 0x80000000	256M	PCI1 MEM First half
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI1_MEM_PHYS, CONFIG_SYS_PCI1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 * 0x90000000	256M	PCI1 MEM Second half
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI1_MEM_PHYS + 0x10000000, CONFIG_SYS_PCI1_MEM_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 * 0xa0000000	256M	PCI2 MEM First half
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI2_MEM_PHYS, CONFIG_SYS_PCI2_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 4:	256M	Non-cacheable, guarded
	 * 0xb0000000	256M	PCI2 MEM Second half
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI2_MEM_PHYS + 0x10000000, CONFIG_SYS_PCI2_MEM_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 5:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	16M	PCI1 IO
	 * 0xe300_0000	16M	PCI2 IO
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 6:	256M	Non-cacheable, guarded
	 * 0xf0000000		Local bus expansion option.
	 * 0xfb000000		Configuration Latch register (one word)
	 * 0xfc000000		Up to 64M flash
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_LBC_OPTION_BASE, CONFIG_SYS_LBC_OPTION_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 7, BOOKE_PAGESZ_256M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
