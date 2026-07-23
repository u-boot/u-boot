// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 - 2018 Xilinx, Inc.
 * (C) Copyright 2026, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <init.h>
#include <log.h>
#include <time.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <dm/platdata.h>

DECLARE_GLOBAL_DATA_PTR;

#define VERSAL_MEM_MAP_USED	3

#define DRAM_BANKS CONFIG_NR_DRAM_BANKS

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
#define TCM_MAP 1
#else
#define TCM_MAP 0
#endif

/* +1 is end of list which needs to be empty */
#define VERSAL_MEM_MAP_MAX (VERSAL_MEM_MAP_USED + DRAM_BANKS + TCM_MAP + 1)

static struct mm_region versal_mem_map[VERSAL_MEM_MAP_MAX] = {
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
	}, { /* FPD_AXI_PL_high */
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

void mem_map_fill(void)
{
	int banks = VERSAL_MEM_MAP_USED;

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
	versal_mem_map[banks].virt = 0xffe00000UL;
	versal_mem_map[banks].phys = 0xffe00000UL;
	versal_mem_map[banks].size = 0x00200000UL;
	versal_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				      PTE_BLOCK_INNER_SHARE;
	banks = banks + 1;
#endif

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* Zero size means no more DDR that's this is end */
		if (!gd->dram[i].size)
			break;

#if defined(CONFIG_VERSAL_NO_DDR)
		if (gd->dram[i].start < 0x80000000UL ||
		    gd->dram[i].start > 0x100000000UL) {
			printf("Ignore caches over %llx/%llx\n",
			       gd->dram[i].start,
			       gd->dram[i].size);
			continue;
		}
#endif
		versal_mem_map[banks].virt = gd->dram[i].start;
		versal_mem_map[banks].phys = gd->dram[i].start;
		versal_mem_map[banks].size = gd->dram[i].size;
		versal_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
}

struct mm_region *mem_map = versal_mem_map;

u64 get_page_table_size(void)
{
	return 0x14000;
}

#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU)
int arm_reserve_mmu(void)
{
	tcm_init(TCM_LOCK);
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = VERSAL_TCM_BASE_ADDR;

	return 0;
}
#endif

u32 versal_multi_boot_reg(void)
{
	return readl(PMC_MULTI_BOOT_REG) & PMC_MULTI_BOOT_MASK;
}

u32 __weak versal_pmc_multi_boot(void)
{
	return versal_multi_boot_reg();
}

void versal_timer_setup(void)
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

u32 versal_bootmode_reg(void)
{
	return readl(&crp_base->boot_mode_usr);
}

u8 __weak versal_get_bootmode(void)
{
	u32 reg = versal_bootmode_reg();

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	return reg & BOOT_MODES_MASK;
}

U_BOOT_DRVINFO(soc_xilinx_versal) = {
	.name = "soc_xilinx_versal",
};
