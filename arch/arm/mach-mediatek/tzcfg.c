// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <errno.h>
#include <stdio.h>
#include <asm/global_data.h>
#include <linux/kernel.h>
#include <linux/arm-smccc.h>
#include <linux/sizes.h>
#include <command.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <lmb.h>

DECLARE_GLOBAL_DATA_PTR;

#define MTK_SIP_GET_BL31_REGION		0x82000300
#define MTK_SIP_GET_BL32_REGION		0x82000301

#define BL31_DEFAULT_ADDR		0x43000000
#define BL31_DEFAULT_SIZE		0x80000

#define TZ_REGION_MAX_SIZE		SZ_16M
#define U_BOOT_MIN_SIZE			SZ_4M
#define U_BOOT_MIN_STACK_SIZE		SZ_1M
#define REGION_ALIGNMENT		SZ_64K

struct tz_reserved_region {
	phys_addr_t addr;
	phys_addr_t size;
};

static bool fix_tz_region(struct tz_reserved_region region[],
			  uint32_t used_regions)
{
	phys_addr_t size;

	if (region[0].addr + region[0].size > gd->ram_top) {
		if (region[0].addr >= gd->ram_top) {
			debug("Discarded region 0x%08llx, size 0x%llx\n",
			      region[0].addr, region[0].size);

			if (used_regions > 1)
				region[0] = region[1];

			return true;
		}

		size = gd->ram_top - region[0].addr;

		debug("Truncated region 0x%08llx, size 0x%llx -> 0x%llx\n",
		      region[0].addr, region[0].size, size);

		region[0].size = size;
	}

	return false;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	phys_addr_t uboot_ram_top, pstore_size, uboot_size = 0;
	struct tz_reserved_region region[2], tmp;
	phys_addr_t top_addr, low_addr;
	struct arm_smccc_res res;
	u32 used_regions = 1;

	/* BL31 region */
	arm_smccc_smc(MTK_SIP_GET_BL31_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		/* Assume PIE is not enabled for BL31 */
		region[0].addr = BL31_DEFAULT_ADDR;
		region[0].size = BL31_DEFAULT_SIZE;
	} else {
		region[0].addr = res.a1;
		region[0].size = res.a2;
	}

	debug("BL31 @ 0x%08llx, size 0x%llx\n", region[0].addr,
	      region[0].size);

	/* BL32 region is optional */
	arm_smccc_smc(MTK_SIP_GET_BL32_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
	if (!res.a0 && res.a1 && res.a2) {
		region[used_regions].addr = res.a1;
		region[used_regions].size = res.a2;

		debug("BL32 @ 0x%08llx, size 0x%llx\n",
		      region[used_regions].addr, region[used_regions].size);

		used_regions++;
	}

	if (used_regions == 2) {
		if (region[0].addr < region[1].addr) {
			/* Make sure region[0] is higher than region[1] */
			tmp = region[0];
			region[0] = region[1];
			region[1] = tmp;
		}

		top_addr = region[0].addr + region[0].size;
		low_addr = min_t(phys_addr_t, region[0].addr, region[1].addr);

		if (top_addr - low_addr <= TZ_REGION_MAX_SIZE) {
			/* Merge region if they're overlapped or close enough */
			region[0].size = top_addr - low_addr;
			region[0].addr = low_addr;

			debug("Merged region @ 0x%08llx, size 0x%llx\n",
			      region[0].addr, region[0].size);

			used_regions = 1;
		}
	}

	debug("Effective memory @ 0x%08zx, size 0x%llx\n", gd->ram_base,
	      gd->ram_top - gd->ram_base);

	/* Discard/fix region which is outside the effective memory */
	if (fix_tz_region(region, used_regions)) {
		used_regions--;

		if (used_regions) {
			if (fix_tz_region(region, used_regions))
				used_regions--;
		}
	}

	/* Size needed for u-boot & pstore */
#if IS_ENABLED(CONFIG_CMD_PSTORE)
	/* pstore will be placed under ram top */
	pstore_size = (CONFIG_CMD_PSTORE_MEM_SIZE + REGION_ALIGNMENT - 1) &
		      ~(REGION_ALIGNMENT - 1);
	/* u-boot will be placed under pstore */
	uboot_size += pstore_size;
#endif

	uboot_size += max_t(uintptr_t, U_BOOT_MIN_SIZE, total_size);
	uboot_size += U_BOOT_MIN_STACK_SIZE + REGION_ALIGNMENT - 1;
	uboot_size &= ~(REGION_ALIGNMENT - 1);

	uboot_ram_top = gd->ram_top & ~(REGION_ALIGNMENT - 1);

	if (!used_regions ||
	    (uboot_ram_top - region[0].addr - region[0].size >= uboot_size)) {
		/* No reserved region present,
		 * or gap between high region and ram top is large enough
		 */
		uboot_ram_top -= pstore_size;
		return uboot_ram_top;
	}

	uboot_ram_top = region[0].addr & ~(REGION_ALIGNMENT - 1);

	if (used_regions == 2 &&
	    (uboot_ram_top - region[1].addr - region[1].size >= uboot_size)) {
		/* Gap between high region and low region is large enough */
		uboot_ram_top -= pstore_size;
		return uboot_ram_top;
	}

	uboot_ram_top = region[used_regions - 1].addr & ~(REGION_ALIGNMENT - 1);

	/* Under low region */
	uboot_ram_top -= pstore_size;
	return uboot_ram_top;
}

int arch_misc_init(void)
{
	phys_addr_t addr;
	struct arm_smccc_res res;

	/*
	 * Since board_get_usable_ram_top is be called before arch_misc_init,
	 * there's no need to check the result
	 */
	arm_smccc_smc(MTK_SIP_GET_BL31_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
	addr = (phys_addr_t)res.a1;
	lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &addr, res.a2, LMB_NOMAP);

	arm_smccc_smc(MTK_SIP_GET_BL32_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
	addr = (phys_addr_t)res.a1;
	if (!res.a0 && res.a1 && res.a2)
		lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &addr, res.a2,
			      LMB_NOMAP);

#if IS_ENABLED(CONFIG_CMD_PSTORE)
	char cmd[64];

	/* Override default pstore address */
	snprintf(cmd, sizeof(cmd), "pstore set 0x%llx 0x%x", gd->ram_top,
		 CONFIG_CMD_PSTORE_MEM_SIZE);
	run_command(cmd, 0);
#endif

	return 0;
}

/* For board-level setup */
__weak int mtk_ft_system_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	struct arm_smccc_res res;
	struct fdt_memory mem;
	int ret;

	arm_smccc_smc(MTK_SIP_GET_BL31_REGION, 0, 0, 0, 0, 0, 0, 0, &res);

	mem.start = res.a1;
	mem.end = res.a1 + res.a2 - 1;

	ret = fdtdec_add_reserved_memory(blob, "secmon", &mem, NULL, 0, NULL,
					 FDTDEC_RESERVED_MEMORY_NO_MAP);
	if (ret < 0) {
		log_err("Failed to add reserved-memory for BL31: %s\n",
			fdt_strerror(ret));
		return ret;
	}

	arm_smccc_smc(MTK_SIP_GET_BL32_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
	if (!res.a0 && res.a1 && res.a2) {
		mem.start = res.a1;
		mem.end = res.a1 + res.a2 - 1;

		ret = fdtdec_add_reserved_memory(blob, "trustzone", &mem, NULL,
						 0, NULL,
						 FDTDEC_RESERVED_MEMORY_NO_MAP);
		if (ret < 0) {
			log_err("Failed to add reserved-memory for BL32: %s\n",
				fdt_strerror(ret));
			return ret;
		}
	}

	return mtk_ft_system_setup(blob, bd);
}
