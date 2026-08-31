// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
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
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <dm/platdata.h>
#include <linux/bitfield.h>
#include <linux/string.h>
#include "../../../board/xilinx/common/board.h"

DECLARE_GLOBAL_DATA_PTR;

#define VERSAL_NET_MEM_MAP_USED	5

#define DRAM_BANKS CONFIG_NR_DRAM_BANKS

/* +1 is end of list which needs to be empty */
#define VERSAL_NET_MEM_MAP_MAX (VERSAL_NET_MEM_MAP_USED + DRAM_BANKS + 1)

static struct mm_region versal_mem_map[VERSAL_NET_MEM_MAP_MAX] = {
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
	}, {
		.virt = 0x600000000UL,
		.phys = 0x600000000UL,
		.size = 0x800000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe00000000UL,
		.phys = 0xe00000000UL,
		.size = 0xf200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

void mem_map_fill(void)
{
	int banks = VERSAL_NET_MEM_MAP_USED;

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* Zero size means no more DDR that's this is end */
		if (!gd->dram[i].size)
			break;

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

void versal_net_timer_setup(void)
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

u32 versal_net_bootmode_reg(void)
{
	return readl(&crp_base->boot_mode_usr);
}

u8 __weak versal_net_get_bootmode(void)
{
	u32 reg = versal_net_bootmode_reg();

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	return reg & BOOT_MODES_MASK;
}

static u32 platform_id, platform_version;

char *soc_name_decode(void)
{
	char *name, *platform_name;

	switch (platform_id) {
	case VERSAL_NET_SPP:
		platform_name = "ipp";
		break;
	case VERSAL_NET_EMU:
		platform_name = "emu";
		break;
	case VERSAL_NET_QEMU:
		platform_name = "qemu";
		break;
	default:
		return NULL;
	}

	/*
	 * --rev. are 6 chars
	 * max platform name is qemu which is 4 chars
	 * platform version number are 1+1
	 * Plus 1 char for \n
	 */
	name = calloc(1, strlen(CONFIG_SYS_BOARD) + 13);
	if (!name)
		return NULL;

	sprintf(name, "%s-%s-rev%d.%d", CONFIG_SYS_BOARD,
		platform_name, platform_version / 10,
		platform_version % 10);

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

	if (platform_id == VERSAL_NET_SPP ||
	    platform_id == VERSAL_NET_EMU) {
		if (ps_version == PS_VERSION_PRODUCTION) {
			/*
			 * ES1 version ends at 1.9 version where there was +9
			 * used because of IPP/SPP conversion. Production
			 * version have platform_version started from 0 again
			 * that's why adding +20 to continue with the same line.
			 * It means the last ES1 version ends at 1.9 version and
			 * new PRODUCTION line starts at 2.0.
			 */
			platform_version += 20;
		} else {
			/*
			 * 9 is diff for
			 * 0 means 0.9 version
			 * 1 means 1.0 version
			 * 2 means 1.1 version
			 * etc,
			 */
			platform_version += 9;
		}
	}

	debug("Platform id: %d version: %d.%d\n", platform_id,
	      platform_version / 10, platform_version % 10);

	return true;
}

U_BOOT_DRVINFO(soc_xilinx_versal_net) = {
	.name = "soc_xilinx_versal_net",
};
