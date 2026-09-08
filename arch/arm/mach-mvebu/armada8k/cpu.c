// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <lmb.h>
#include <log.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <mach/fw_info.h>

#include "soc_info.h"

DECLARE_GLOBAL_DATA_PTR;

/* Armada 7k/8k */
#define MVEBU_RFU_BASE			(MVEBU_REGISTER(0x6f0000))
#define RFU_GLOBAL_SW_RST		(MVEBU_RFU_BASE + 0x84)
#define RFU_SW_RESET_OFFSET		0

#define SAR0_REG			(MVEBU_REGISTER(0x2400200))
#define BOOT_MODE_MASK			0x3f
#define BOOT_MODE_OFFSET		4

static struct mm_region mvebu_mem_map[] = {
	/* Armada 80x0 memory regions include the CP1 (slave) units */
	{
		/* RAM 0-64MB */
		.phys = 0x0UL,
		.virt = 0x0UL,
		.size = ATF_REGION_START,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	/* ATF and TEE region 0x4000000-0x5400000 not mapped */
	{
		/* RAM 66MB-2GB */
		.phys = ATF_REGION_END,
		.virt = ATF_REGION_END,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* MMIO regions */
		.phys = MMIO_REGS_PHY_BASE,
		.virt = MMIO_REGS_PHY_BASE,
		.size = SZ_1G,

		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

#if CONFIG_IS_ENABLED(LMB_ARCH_MEM_MAP)
/**
 * mvebu_lmb_reserve() - mark a region as present but not allocatable
 * @base:	start of the region
 * @size:	size of the region
 */
static void mvebu_lmb_reserve(phys_addr_t base, phys_size_t size)
{
	phys_addr_t addr = base;

	if (lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &addr, size, LMB_NOOVERWRITE))
		log_err("Failed to reserve 0x%llx bytes at 0x%llx\n",
			(unsigned long long)size, (unsigned long long)base);
}

/**
 * mvebu_lmb_add_bank() - add one memory range, honouring ram_top
 * @base:	start of the range
 * @size:	size of the range
 * @ram_top:	highest address U-Boot may allocate from
 *
 * The reservation above @ram_top mirrors the generic lmb_add_memory(), which
 * this hook replaces.
 */
static void mvebu_lmb_add_bank(phys_addr_t base, phys_size_t size, u64 ram_top)
{
	phys_addr_t bank_end = base + size;

	lmb_add(base, size);

	if (!IS_ENABLED(CONFIG_LMB_LIMIT_DMA_BELOW_RAM_TOP))
		return;

	if (base >= ram_top)
		mvebu_lmb_reserve(base, size);
	else if (bank_end > ram_top)
		mvebu_lmb_reserve(ram_top, bank_end - ram_top);
}

/**
 * lmb_arch_add_memory() - add DRAM to LMB, minus the ATF and TEE region
 *
 * mvebu_mem_map[] above deliberately has no entry for ATF_REGION_START to
 * ATF_REGION_END, so U-Boot has no translation for that range and any access
 * to it takes a translation fault. It must not be handed to LMB either:
 * everything LMB holds as available is published to EFI payloads as
 * EFI_CONVENTIONAL_MEMORY, and the first payload to use it aborts.
 */
void lmb_arch_add_memory(void)
{
	phys_addr_t bank_start, bank_end;
	u64 ram_top = gd->ram_top;
	int i;

	/* Assume a 4GB ram_top if not defined */
	if (!ram_top)
		ram_top = 0x100000000ULL;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (!gd->dram[i].size)
			continue;

		bank_start = gd->dram[i].start;
		bank_end = bank_start + gd->dram[i].size;

		if (bank_end <= ATF_REGION_START ||
		    bank_start >= ATF_REGION_END) {
			mvebu_lmb_add_bank(bank_start, gd->dram[i].size,
					   ram_top);
			continue;
		}

		if (bank_start < ATF_REGION_START)
			mvebu_lmb_add_bank(bank_start,
					   ATF_REGION_START - bank_start,
					   ram_top);
		if (bank_end > ATF_REGION_END)
			mvebu_lmb_add_bank(ATF_REGION_END,
					   bank_end - ATF_REGION_END,
					   ram_top);
	}
}
#endif

void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void reset_cpu(void)
{
	u32 reg;

	reg = readl(RFU_GLOBAL_SW_RST);
	reg &= ~(1 << RFU_SW_RESET_OFFSET);
	writel(reg, RFU_GLOBAL_SW_RST);
}

/*
 * TODO - implement this functionality using platform
 *        clock driver once it gets available
 * Return NAND clock in Hz
 */
u32 mvebu_get_nand_clock(void)
{
	unsigned long NAND_FLASH_CLK_CTRL = 0xF2440700UL;
	unsigned long NF_CLOCK_SEL_MASK = 0x1;
	u32 reg;

	reg = readl(NAND_FLASH_CLK_CTRL);
	if (reg & NF_CLOCK_SEL_MASK)
		return 400 * 1000000;
	else
		return 250 * 1000000;
}

int mmc_get_env_dev(void)
{
	u32 reg;
	unsigned int boot_mode;

	reg = readl(SAR0_REG);
	boot_mode = (reg >> BOOT_MODE_OFFSET) & BOOT_MODE_MASK;

	switch (boot_mode) {
	case 0x28:
	case 0x2a:
		return 0;
	case 0x29:
	case 0x2b:
		return 1;
	}

	return CONFIG_ENV_MMC_DEVICE_INDEX;
}

int print_cpuinfo(void)
{
	if (!IS_ENABLED(CONFIG_DISPLAY_CPUINFO))
		return 0;

	soc_print_clock_info();
	soc_print_soc_info();
	return 0;
}
