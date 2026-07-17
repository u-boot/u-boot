// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <init.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <vsprintf.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <dm/platdata.h>
#include <linux/bitfield.h>
#include <linux/string.h>
#include "../../../board/xilinx/common/board.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(PCIE_DW_AMD)
#define VERSAL2_MEM_MAP_USED	4
#else
#define VERSAL2_MEM_MAP_USED	3
#endif

#define DRAM_BANKS CONFIG_NR_DRAM_BANKS

/* +1 is end of list which needs to be empty */
#define VERSAL2_MEM_MAP_MAX (VERSAL2_MEM_MAP_USED + DRAM_BANKS + 1)

static struct mm_region versal2_mem_map[VERSAL2_MEM_MAP_MAX] = {
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x0fe00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
#if CONFIG_IS_ENABLED(PCIE_DW_AMD)
	}, {
		/* PCIe DBI (1 MB) and config space (255 MB) are contiguous */
		.virt = 0x100000000000UL,
		.phys = 0x100000000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
#endif
	}
};

/**
 * mem_map_fill() - Populate global memory map with DRAM banks
 * @bank_info: Array of memory regions parsed from device tree
 * @num_banks: Number of valid DRAM banks in bank_info array
 *
 * Copies DRAM bank information into the global versal2_mem_map[] array
 * starting at index VERSAL2_MEM_MAP_USED, which is after the fixed
 * device mappings. This must be called early in boot before MMU
 * initialization so that get_page_table_size() can calculate the
 * required page table size based on actual memory configuration.
 */
void mem_map_fill(struct mm_region *bank_info, u32 num_banks)
{
	int banks = VERSAL2_MEM_MAP_USED;

	for (int i = 0; i < num_banks; i++) {
		if (banks > VERSAL2_MEM_MAP_MAX)
			return;

		versal2_mem_map[banks].virt = bank_info[i].phys;
		versal2_mem_map[banks].phys = bank_info[i].phys;
		versal2_mem_map[banks].size = bank_info[i].size;
		versal2_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
}

/**
 * fill_bd_mem_info() - Copy DRAM banks from mem_map to bd_info
 *
 * Transfers DRAM bank information from the global versal2_mem_map[]
 * array to gd->dram[] for passing memory configuration to the
 * Linux kernel via boot parameters (ATAGS/FDT). Each bank's physical
 * address and size are copied.
 *
 * This is called during dram_init_banksize() after the memory map
 * has been populated by mem_map_fill() in dram_init(). Called after
 * dram_init() but before kernel handoff.
 */
void fill_bd_mem_info(void)
{
	int banks = VERSAL2_MEM_MAP_USED;

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (!versal2_mem_map[banks].size)
			break;

		gd->dram[i].start = versal2_mem_map[banks].phys;
		gd->dram[i].size = versal2_mem_map[banks].size;
		banks++;
	}
}

struct mm_region *mem_map = versal2_mem_map;

#if CONFIG_IS_ENABLED(SYS_MEM_RSVD_FOR_MMU)
u64 get_page_table_size(void)
{
	return 0x14000;
}
#endif

u32 versal2_multi_boot_reg(void)
{
	return readl(PMC_MULTI_BOOT_REG) & PMC_MULTI_BOOT_MASK;
}

u32 __weak versal2_pmc_multi_boot(void)
{
	return versal2_multi_boot_reg();
}

u8 __weak versal2_get_bootmode(void)
{
	u8 bootmode;
	u32 reg;

	reg = readl(&crp_base->boot_mode_usr);

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	return bootmode;
}

void versal2_timer_setup(void)
{
	u32 val;

	debug("iou_switch ctrl div0 %x\n",
	      readl(&crlapb_base->iou_switch_ctrl));

	writel(IOU_SWITCH_CTRL_CLKACT_BIT |
	       (CONFIG_IOU_SWITCH_DIVISOR0 << IOU_SWITCH_CTRL_DIVISOR0_SHIFT),
	       &crlapb_base->iou_switch_ctrl);

	/* Global timer init - Program time stamp reference clk */
	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val |= CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	writel(val, &crlapb_base->timestamp_ref_ctrl);

	debug("ref ctrl 0x%x\n",
	      readl(&crlapb_base->timestamp_ref_ctrl));

	/* Clear reset of timestamp reg */
	writel(0, &crlapb_base->rst_timestamp);

	/*
	 * Program freq register in System counter and
	 * enable system counter.
	 */
	writel(CONFIG_COUNTER_FREQUENCY,
	       &iou_scntr_secure->base_frequency_id_register);

	debug("counter val 0x%x\n",
	      readl(&iou_scntr_secure->base_frequency_id_register));

	writel(IOU_SCNTRS_CONTROL_EN,
	       &iou_scntr_secure->counter_control_register);

	debug("scntrs control 0x%x\n",
	      readl(&iou_scntr_secure->counter_control_register));
	debug("timer 0x%llx\n", get_ticks());
	debug("timer 0x%llx\n", get_ticks());
}

static u32 platform_id, platform_version;

char *soc_name_decode(void)
{
	char *name, *platform_name;

	switch (platform_id) {
	case VERSAL2_SPP:
		platform_name = "spp";
		break;
	case VERSAL2_EMU:
		platform_name = "emu";
		break;
	case VERSAL2_SPP_MMD:
		platform_name = "spp-mmd";
		break;
	case VERSAL2_EMU_MMD:
		platform_name = "emu-mmd";
		break;
	case VERSAL2_QEMU:
		platform_name = "qemu";
		break;
	default:
		return NULL;
	}

	/*
	 * --rev.-el are 9 chars
	 * max platform name is emu-mmd which is 7 chars
	 * platform version number are 1+1
	 * el is 1 char
	 * Plus 1 char for NULL byte
	 */
	name = calloc(1, strlen(CONFIG_SYS_BOARD) + 20);
	if (!name)
		return NULL;

	sprintf(name, "%s-%s-rev%d.%d-el%d", CONFIG_SYS_BOARD,
		platform_name, platform_version / 10,
		platform_version % 10, current_el());

	return name;
}

bool soc_detection(void)
{
	u32 version, ps_version;

	version = readl(PMC_TAP_VERSION);
	platform_id = FIELD_GET(PLATFORM_MASK, version);
	ps_version = FIELD_GET(PS_VERSION_MASK, version);

	debug("idcode %x, version %x, usercode %x\n",
	      readl(PMC_TAP_IDCODE), version,
	      readl(PMC_TAP_USERCODE));

	debug("pmc_ver %lx, ps version %x, rtl version %lx\n",
	      FIELD_GET(PMC_VERSION_MASK, version),
	      ps_version,
	      FIELD_GET(RTL_VERSION_MASK, version));

	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, version);

	debug("Platform id: %d version: %d.%d\n", platform_id,
	      platform_version / 10, platform_version % 10);

	return true;
}

U_BOOT_DRVINFO(soc_amd_versal2) = {
	.name = "soc_amd_versal2",
};
