// SPDX-License-Identifier: GPL-2.0+
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <config.h>
#include <mpc85xx.h>
#include <asm/mmu.h>
#include <linux/sizes.h>
#include <linux/build_bug.h>

/*
 * NOTE: e500v2 supports only following Book-E page sizes:
 *
 * TLB0:
 * BOOKE_PAGESZ_4K
 *
 * TLB1:
 * BOOKE_PAGESZ_4K
 * BOOKE_PAGESZ_16K
 * BOOKE_PAGESZ_64K
 * BOOKE_PAGESZ_256K
 * BOOKE_PAGESZ_1M
 * BOOKE_PAGESZ_4M
 * BOOKE_PAGESZ_16M
 * BOOKE_PAGESZ_64M
 * BOOKE_PAGESZ_256M
 * BOOKE_PAGESZ_1G
 * BOOKE_PAGESZ_4G
 */

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 */

	/* ***** - Initial stack in L1 cache 16K */
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 0 * SZ_4K,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 0 * SZ_4K,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 1 * SZ_4K,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 1 * SZ_4K,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 2 * SZ_4K,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 2 * SZ_4K,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 3 * SZ_4K,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 3 * SZ_4K,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 */

	/* *I*** - Boot page 4K */
	SET_TLB_ENTRY(1, BPTR_VIRT_ADDR,
		      0xfffff000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I,
		      0, 0, BOOKE_PAGESZ_4K, 1),

	/* *I*G* - CCSR 1M */
	SET_TLB_ENTRY(1, CFG_SYS_CCSRBAR,
		      CFG_SYS_CCSRBAR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 1, BOOKE_PAGESZ_1M, 1),

	/* W**G* - NOR 16M */
	/* This will be changed to *I*G* after relocation to RAM in board_early_init_r() */
	SET_TLB_ENTRY(1, CFG_SYS_FLASH_BASE,
		      CFG_SYS_FLASH_BASE_PHYS,
		      MAS3_SX | MAS3_SR, MAS2_W | MAS2_G,
		      0, 2, BOOKE_PAGESZ_16M, 1),

	/* *I*G* - CPLD 256K (effective 128K) */
	SET_TLB_ENTRY(1, CFG_SYS_CPLD_BASE,
		      CFG_SYS_CPLD_BASE_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 3, BOOKE_PAGESZ_256K, 1),

	/* *I*G* - NAND 256K */
	SET_TLB_ENTRY(1, CFG_SYS_NAND_BASE,
		      CFG_SYS_NAND_BASE_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 4, BOOKE_PAGESZ_256K, 1),

	/* *I*G* - PCIe MEM (bus 1 and 2) 1G */
	SET_TLB_ENTRY(1, CFG_SYS_PCIE1_MEM_VIRT,
		      CFG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 5, BOOKE_PAGESZ_1G, 1),

	/* *I*G* - PCIe MEM (bus 3) 4M (effective 2M) */
	SET_TLB_ENTRY(1, CFG_SYS_PCIE3_MEM_VIRT,
		      CFG_SYS_PCIE3_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 6, BOOKE_PAGESZ_4M, 1),

	/* *I*G* - PCIe I/O (all 3 buses) 256K (effective 192K) */
	SET_TLB_ENTRY(1, CFG_SYS_PCIE1_IO_VIRT,
		      CFG_SYS_PCIE1_IO_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 7, BOOKE_PAGESZ_256K, 1),

#ifdef CFG_SYS_INIT_L2_ADDR
	/* ***G* - Initial SRAM in L2 cache 512K */
	SET_TLB_ENTRY(1, CFG_SYS_INIT_L2_ADDR,
		      CFG_SYS_INIT_L2_ADDR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_G,
		      0, 8, BOOKE_PAGESZ_256K, 1),
	SET_TLB_ENTRY(1, CFG_SYS_INIT_L2_ADDR + SZ_256K,
		      CFG_SYS_INIT_L2_ADDR_PHYS + SZ_256K,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_G,
		      0, 9, BOOKE_PAGESZ_256K, 1),
#endif

#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	/* **M** - SDRAM 2G */
	SET_TLB_ENTRY(1, CFG_SYS_DDR_SDRAM_BASE,
		      CFG_SYS_DDR_SDRAM_BASE,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_M,
		      0, 10, BOOKE_PAGESZ_1G, 1),
	SET_TLB_ENTRY(1, CFG_SYS_DDR_SDRAM_BASE + SZ_1G,
		      CFG_SYS_DDR_SDRAM_BASE + SZ_1G,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_M,
		      0, 11, BOOKE_PAGESZ_1G, 1),
#endif
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);

/*
 * PCIe MEM TLB entry expects that second PCIe MEM window is mapped after the
 * first PCIe MEM window. Check for this requirement.
 */
static_assert(CFG_SYS_PCIE1_MEM_VIRT + SZ_512M == CFG_SYS_PCIE2_MEM_VIRT);
static_assert(CFG_SYS_PCIE1_MEM_PHYS + SZ_512M == CFG_SYS_PCIE2_MEM_PHYS);

/*
 * PCIe I/O TLB entry expects that all 3 PCIe I/O windows are mapped one after
 * another. Check for this requirement.
 */
static_assert(CFG_SYS_PCIE1_IO_VIRT + SZ_64K == CFG_SYS_PCIE2_IO_VIRT);
static_assert(CFG_SYS_PCIE1_IO_PHYS + SZ_64K == CFG_SYS_PCIE2_IO_PHYS);
static_assert(CFG_SYS_PCIE2_IO_VIRT + SZ_64K == CFG_SYS_PCIE3_IO_VIRT);
static_assert(CFG_SYS_PCIE2_IO_PHYS + SZ_64K == CFG_SYS_PCIE3_IO_PHYS);
