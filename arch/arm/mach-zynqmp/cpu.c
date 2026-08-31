// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <init.h>
#include <time.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <zynqmp_firmware.h>
#include <asm/cache.h>
#include <dm/platdata.h>

#define ZYNQ_SILICON_VER_MASK	0xF000
#define ZYNQ_SILICON_VER_SHIFT	12

DECLARE_GLOBAL_DATA_PTR;

/*
 * Number of filled static entries and also the first empty
 * slot in zynqmp_mem_map.
 */
#define ZYNQMP_MEM_MAP_USED	4

#if !defined(CONFIG_ZYNQMP_NO_DDR)
#define DRAM_BANKS CONFIG_NR_DRAM_BANKS
#else
#define DRAM_BANKS 0
#endif

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
#define TCM_MAP 1
#else
#define TCM_MAP 0
#endif

/* +1 is end of list which needs to be empty */
#define ZYNQMP_MEM_MAP_MAX (ZYNQMP_MEM_MAP_USED + DRAM_BANKS + TCM_MAP + 1)

static struct mm_region zynqmp_mem_map[ZYNQMP_MEM_MAP_MAX] = {
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf8000000UL,
		.phys = 0xf8000000UL,
		.size = 0x07e00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x400000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x1000000000UL,
		.phys = 0x1000000000UL,
		.size = 0xf000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

void mem_map_fill(void)
{
	int banks = ZYNQMP_MEM_MAP_USED;

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
	zynqmp_mem_map[banks].virt = 0xffe00000UL;
	zynqmp_mem_map[banks].phys = 0xffe00000UL;
	zynqmp_mem_map[banks].size = 0x00200000UL;
	zynqmp_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				      PTE_BLOCK_INNER_SHARE;
	banks = banks + 1;
#endif

#if !defined(CONFIG_ZYNQMP_NO_DDR)
	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* Zero size means no more DDR that's this is end */
		if (!gd->dram[i].size)
			break;

		zynqmp_mem_map[banks].virt = gd->dram[i].start;
		zynqmp_mem_map[banks].phys = gd->dram[i].start;
		zynqmp_mem_map[banks].size = gd->dram[i].size;
		zynqmp_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
#endif
}

struct mm_region *mem_map = zynqmp_mem_map;

u64 get_page_table_size(void)
{
	return 0x14000;
}

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
void tcm_init(enum tcm_mode mode)
{
	int ret;

	ret = check_tcm_mode(mode);
	if (!ret) {
		puts("WARNING: Initializing TCM overwrites TCM content\n");
		initialize_tcm(mode);
		memset((void *)ZYNQMP_TCM_BASE_ADDR, 0, ZYNQMP_TCM_SIZE);
	}

	if (ret == -EACCES)
		printf("ERROR: Split to lockstep mode required reset/disable cpu\n");

	/* Ignore if ret is -EAGAIN, trying to initialize same mode again */
}
#endif

#ifdef CONFIG_SYS_MEM_RSVD_FOR_MMU
int arm_reserve_mmu(void)
{
	puts("WARNING: Initializing TCM overwrites TCM content\n");
	initialize_tcm(TCM_LOCK);
	memset((void *)ZYNQMP_TCM_BASE_ADDR, 0, ZYNQMP_TCM_SIZE);

	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = ZYNQMP_TCM_BASE_ADDR;

	return 0;
}
#endif

static unsigned int zynqmp_get_silicon_version_secure(void)
{
	u32 ver;

	ver = readl(&csu_base->version);
	ver &= ZYNQMP_SILICON_VER_MASK;
	ver >>= ZYNQMP_SILICON_VER_SHIFT;

	return ver;
}

unsigned int zynqmp_get_silicon_version(void)
{
	if (current_el() == 3)
		return zynqmp_get_silicon_version_secure();

	gd->cpu_clk = get_tbclk();

	switch (gd->cpu_clk) {
	case 50000000:
		return ZYNQMP_CSU_VERSION_QEMU;
	}

	return ZYNQMP_CSU_VERSION_SILICON;
}

int zynqmp_mmio_rawread(const u32 address, u32 *value)
{
	*value = readl((ulong)address);
	return 0;
}

int zynqmp_mmio_rawwrite(const u32 address, const u32 mask, const u32 value)
{
	u32 data;
	u32 value_local = value;
	int ret;

	ret = zynqmp_mmio_rawread(address, &data);
	if (ret)
		return ret;

	data &= ~mask;
	value_local &= mask;
	value_local |= data;
	writel(value_local, (ulong)address);
	return 0;
}

int __weak zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value)
{
	if (IS_ENABLED(CONFIG_XPL_BUILD) || current_el() == 3)
		return zynqmp_mmio_rawwrite(address, mask, value);

	return -EINVAL;
}

int __weak zynqmp_mmio_read(const u32 address, u32 *value)
{
	if (!value)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_XPL_BUILD) || current_el() == 3)
		return zynqmp_mmio_rawread(address, value);

	return -EINVAL;
}

void zynqmp_timer_setup(void)
{
	u32 val;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val &= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;

	if (!val) {
		val = readl(&crlapb_base->timestamp_ref_ctrl);
		val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
		writel(val, &crlapb_base->timestamp_ref_ctrl);

		/* Program freq register in System counter */
		writel(zynqmp_get_system_timer_freq(),
		       &iou_scntr_secure->base_frequency_id_register);
		/* And enable system counter */
		writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
		       &iou_scntr_secure->counter_control_register);
	}
}

U_BOOT_DRVINFO(soc_xilinx_zynqmp) = {
	.name = "soc_xilinx_zynqmp",
};
