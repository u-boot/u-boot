// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <mach/clock.h>
#include <mach/soc.h>
#include <mach/fw_info.h>

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
		//WT202003032 .size = SZ_2G,
		  .size = SZ_4G,
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
#if defined(CONFIG_PCIE_DW_MVEBU_EP)
	{
		/* PCIe OCP regions */
		.phys = MV_PCIE_EP_REGION_BASE,
		.virt = MV_PCIE_EP_REGION_BASE,
		.size = MV_PCIE_EP_REGION_SIZE,

		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
#endif
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void reset_cpu(ulong ignored)
{
	u32 reg;

	reg = readl(RFU_GLOBAL_SW_RST);
	reg &= ~(1 << RFU_SW_RESET_OFFSET);
	writel(reg, RFU_GLOBAL_SW_RST);
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

	return CONFIG_SYS_MMC_ENV_DEV;
}

#if defined(CONFIG_DISPLAY_BOARDINFO)
int print_cpuinfo(void)
{
	soc_print_device_info();
	soc_print_clock_info();
	soc_print_system_cache_info();
	return 0;
}
#endif
