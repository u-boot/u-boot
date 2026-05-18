// SPDX-License-Identifier: GPL-2.0-only
/*
 * Board specific initialization for Aquila AM69 SoM
 *
 * Copyright (C) 2025 Toradex - https://www.toradex.com/
 */

#include <asm/arch/k3-common-fdt.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <env.h>
#include <fdt_support.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>

#include "../common/tdx-common.h"
#include "aquila_ddrs.h"
#include "ddrs_patch.h"

#define CTRL_MMR_CFG0_MCU_ADC1_CTRL	0x40F040B4

#define HW_CFG_MEM_SZ_32GB		0x00
#define HW_CFG_MEM_SZ_16GB_RANK_2	0x01
#define HW_CFG_MEM_SZ_8GB		0x02
#define HW_CFG_MEM_SZ_16GB		0x03

#define HW_CFG_MEM_CFG_MASK		0x03

DECLARE_GLOBAL_DATA_PTR;

static u8 get_hw_cfg(void)
{
	struct gpio_desc gpio_hw_cfg;
	char gpio_name[20];
	u8 hw_cfg = 0;
	int i;

	for (i = 0; i < 5; i++) {
		sprintf(gpio_name, "gpio@42110000_%d", 82 + i);
		if (dm_gpio_lookup_name(gpio_name, &gpio_hw_cfg) < 0) {
			printf("Lookup named gpio error\n");
			return 0;
		}

		if (dm_gpio_request(&gpio_hw_cfg, "hw_cfg")) {
			printf("gpio request error\n");
			return 0;
		}

		if (dm_gpio_get_value(&gpio_hw_cfg) == 1)
			hw_cfg |= BIT(i);

		dm_gpio_free(NULL, &gpio_hw_cfg);
	}
	return hw_cfg;
}

static u64 aquila_am69_memory_size(void)
{
	u8 hw_cfg = get_hw_cfg();

	switch (hw_cfg & HW_CFG_MEM_CFG_MASK) {
	case HW_CFG_MEM_SZ_32GB:
		return SZ_32G;
	case HW_CFG_MEM_SZ_16GB_RANK_2:
	case HW_CFG_MEM_SZ_16GB:
		return SZ_16G;
	case HW_CFG_MEM_SZ_8GB:
		return SZ_8G;
	default:
		puts("Invalid memory size configuration\n");
		return -EINVAL;
	}
}

#if defined(CONFIG_TARGET_AQUILA_AM69_R5)
static void update_ddr_timings(u8 hw_cfg)
{
	int ret = 0;
	void *fdt = (void *)gd->fdt_blob;

	switch (hw_cfg & HW_CFG_MEM_CFG_MASK) {
	case HW_CFG_MEM_SZ_8GB:
		ret = aquila_am69_fdt_apply_ddr_patch(fdt, aquila_am69_ddrss_patch_8GB,
						      MULTI_DDR_CFG_INTRLV_SIZE_8GB);
		break;
	case HW_CFG_MEM_SZ_16GB_RANK_2:
		ret = aquila_am69_fdt_apply_ddr_patch(fdt, aquila_am69_ddrss_patch_16GB_rank_2,
						      MULTI_DDR_CFG_INTRLV_SIZE_16GB);
		break;
	case HW_CFG_MEM_SZ_16GB:
		ret = aquila_am69_fdt_apply_ddr_patch(fdt, aquila_am69_ddrss_patch_16GB,
						      MULTI_DDR_CFG_INTRLV_SIZE_16GB);
		break;
	}

	if (ret)
		printf("Applying DDR patch error: %d\n", ret);
}
#endif

static int aquila_am69_fdt_fixup_memory_size(u64 total_sz)
{
	void *blob = (void *)gd->fdt_blob;

	u64 s[CONFIG_NR_DRAM_BANKS] = {
		CFG_SYS_SDRAM_BASE,
		CFG_SYS_SDRAM_BASE1
	};

	u64 e[CONFIG_NR_DRAM_BANKS] = {
		SZ_2G,
		total_sz - SZ_2G
	};

	return fdt_fixup_memory_banks(blob, s, e, CONFIG_NR_DRAM_BANKS);
}

#if defined(CONFIG_TARGET_AQUILA_AM69_R5)
void do_board_detect(void)
{
	u8 hw_cfg;

	/* MCU_ADC1 pins used as General Purpose Inputs */
	writel(readl(CTRL_MMR_CFG0_MCU_ADC1_CTRL) | BIT(16),
	       CTRL_MMR_CFG0_MCU_ADC1_CTRL);

	hw_cfg = get_hw_cfg();
	printf("HW CFG: 0x%02x\n", hw_cfg);

	if (IS_ENABLED(CONFIG_K3_DDRSS))
		update_ddr_timings(hw_cfg);
}
#endif

#if defined(CONFIG_XPL_BUILD)
void spl_perform_board_fixups(struct spl_image_info *spl_image)
{
	fixup_memory_node(spl_image);
}
#endif

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base_lowest();
	if (ret)
		printf("Error setting up mem size and base. %d\n", ret);

	return ret;
}

int dram_init_banksize(void)
{
	int ret;

	if (IS_ENABLED(CONFIG_SPL_BUILD) &&
	    IS_ENABLED(CONFIG_TARGET_AQUILA_AM69_A72)) {
		u64 mem_sz = aquila_am69_memory_size();

		ret = aquila_am69_fdt_fixup_memory_size(mem_sz);
		if (ret)
			printf("Error setting memory size. %d\n", ret);
	} else {
		fdtdec_setup_mem_size_base();
	}

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		printf("Error setting up memory banksize. %d\n", ret);

	return ret;
}

phys_size_t board_get_usable_ram_top(phys_size_t total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif

	return gd->ram_top;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;

	ret = fdt_fixup_msmc_ram_k3(blob);
	if (ret)
		return ret;

	return ft_common_board_setup(blob, bd);
}
#endif

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_ESM_K3)) {
		const char * const esms[] = {"esm@700000", "esm@40800000", "esm@42080000"};

		for (int i = 0; i < ARRAY_SIZE(esms); ++i) {
			ret = uclass_get_device_by_name(UCLASS_MISC, esms[i],
							&dev);
			if (ret) {
				printf("MISC init for %s failed: %d\n", esms[i], ret);
				break;
			}
		}
	}

	if (IS_ENABLED(CONFIG_ESM_PMIC) && ret == 0) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(pmic_esm),
						  &dev);
		if (ret)
			printf("ESM PMIC init failed: %d\n", ret);
	}
}
