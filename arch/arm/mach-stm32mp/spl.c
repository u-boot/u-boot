/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */

#include <common.h>
#include <dm.h>
#include <spl.h>

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
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
