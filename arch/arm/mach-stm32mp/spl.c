// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/io.h>

u32 spl_boot_device(void)
{
	u32 boot_mode;

	boot_mode = (readl(TAMP_BOOT_CONTEXT) & TAMP_BOOT_MODE_MASK) >>
		    TAMP_BOOT_MODE_SHIFT;

	switch (boot_mode) {
	case BOOT_FLASH_SD_1:
	case BOOT_FLASH_EMMC_1:
		return BOOT_DEVICE_MMC1;
	case BOOT_FLASH_SD_2:
	case BOOT_FLASH_EMMC_2:
		return BOOT_DEVICE_MMC2;
	}

	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

int spl_boot_partition(const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
	case BOOT_DEVICE_MMC2:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_MMC2;
	default:
		return -EINVAL;
	}
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		debug("Clock init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RESET, 0, &dev);
	if (ret) {
		debug("Reset init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &dev);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		return;
	}

	/* enable console uart printing */
	preloader_console_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
}
