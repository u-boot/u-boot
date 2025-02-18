// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2020 Intel Corporation. All rights reserved
 *  Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <hang.h>
#include <spl.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

#if IS_ENABLED(CONFIG_SPL_MMC)
u32 spl_boot_mode(const u32 boot_device)
{
	if (IS_ENABLED(CONFIG_SPL_FS_FAT) || IS_ENABLED(CONFIG_SPL_FS_EXT4))
		return MMCSD_MODE_FS;
	else
		return MMCSD_MODE_RAW;
}
#endif

/* board specific function prior loading SSBL / U-Boot */
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-smmu-secure-config", &dev);
	if (ret) {
		printf("HPS SMMU secure settings init failed: %d\n", ret);
		hang();
	}
}
