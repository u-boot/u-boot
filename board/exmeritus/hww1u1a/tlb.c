/*
 * Copyright 2009-2010 eXMeritus, A Boeing Company
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
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
	SET_TLB_ENTRY(0,	CONFIG_SYS_INIT_RAM_ADDR      +  0 * 1024,
				CONFIG_SYS_INIT_RAM_ADDR_PHYS +  0 * 1024,
				MAS3_SX|MAS3_SW|MAS3_SR, 0,
				0, 0, BOOKE_PAGESZ_4K, 0),

	SET_TLB_ENTRY(0,	CONFIG_SYS_INIT_RAM_ADDR      +  4 * 1024,
				CONFIG_SYS_INIT_RAM_ADDR_PHYS +  4 * 1024,
				MAS3_SX|MAS3_SW|MAS3_SR, 0,
				0, 0, BOOKE_PAGESZ_4K, 0),

	SET_TLB_ENTRY(0,	CONFIG_SYS_INIT_RAM_ADDR      +  8 * 1024,
				CONFIG_SYS_INIT_RAM_ADDR_PHYS +  8 * 1024,
				MAS3_SX|MAS3_SW|MAS3_SR, 0,
				0, 0, BOOKE_PAGESZ_4K, 0),

	SET_TLB_ENTRY(0,	CONFIG_SYS_INIT_RAM_ADDR      + 12 * 1024,
				CONFIG_SYS_INIT_RAM_ADDR_PHYS + 12 * 1024,
				MAS3_SX|MAS3_SW|MAS3_SR, 0,
				0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 */
	/* *I*** - Boot page */
	SET_TLB_ENTRY(1,	CONFIG_BPTR_VIRT_ADDR,
				CONFIG_BPTR_VIRT_ADDR,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 0, BOOKE_PAGESZ_4K, 1),

	/* *I*G* - CCSRBAR */
	SET_TLB_ENTRY(1,	CONFIG_SYS_CCSRBAR,
				CONFIG_SYS_CCSRBAR_PHYS,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 1, BOOKE_PAGESZ_1M, 1),

	/*
	 * W**G* - FLASH (Will be *I*G* after relocation to RAM)
	 *
	 * This maps both SPI FLASH chips (128MByte per chip)
	 */
	SET_TLB_ENTRY(1,	CONFIG_SYS_FLASH_BASE,
				CONFIG_SYS_FLASH_BASE_PHYS,
				MAS3_SX|MAS3_SR, MAS2_W|MAS2_G,
				0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * *I*G* - PCI memory
	 *
	 * We have 1.5GB total PCI-E memory space to map and we want to use
	 * the minimum possible number of TLB entries.  Since Book-E TLB
	 * entries are sized in powers of 4, we use 1GB + 256MB + 256MB.
	 */
	SET_TLB_ENTRY(1,	CONFIG_SYS_PCIE3_MEM_VIRT,
				CONFIG_SYS_PCIE3_MEM_PHYS,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 3, BOOKE_PAGESZ_1G, 1),
	SET_TLB_ENTRY(1,	CONFIG_SYS_PCIE3_MEM_VIRT + 0x40000000,
				CONFIG_SYS_PCIE3_MEM_PHYS + 0x40000000,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 4, BOOKE_PAGESZ_256M, 1),
	SET_TLB_ENTRY(1,	CONFIG_SYS_PCIE3_MEM_VIRT + 0x50000000,
				CONFIG_SYS_PCIE3_MEM_PHYS + 0x50000000,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 5, BOOKE_PAGESZ_256M, 1),

	/*
	 * *I*G* - PCI I/O
	 *
	 * This one entry covers all 3 64k PCI-E I/O windows
	 */
	SET_TLB_ENTRY(1,	CONFIG_SYS_PCIE3_IO_VIRT,
				CONFIG_SYS_PCIE3_IO_PHYS,
				MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
				0, 6, BOOKE_PAGESZ_256K, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
