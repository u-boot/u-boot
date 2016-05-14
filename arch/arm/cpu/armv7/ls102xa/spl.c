/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
	return BOOT_DEVICE_NAND;
}

u32 spl_boot_mode(const u32 boot_device)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
#ifdef CONFIG_SPL_FAT_SUPPORT
		return MMCSD_MODE_FS;
#else
		return MMCSD_MODE_RAW;
#endif
	case BOOT_DEVICE_NAND:
		return 0;
	default:
		puts("spl: error: unsupported device\n");
		hang();
	}
}
