// SPDX-License-Identifier: GPL-2.0
/*
 * https://beagleboard.org/ai-64
 *
 * Copyright (C) 2022-2023 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2022-2023 Jason Kridner, BeagleBoard.org Foundation
 * Copyright (C) 2022-2023 Robert Nelson, BeagleBoard.org Foundation
 */

#include <efi_loader.h>
#include <cpu_func.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = BEAGLEBONEAI64_TIBOOT3_IMAGE_GUID,
		.fw_name = u"BEAGLEBONEAI64_TIBOOT3",
		.image_index = 1,
	},
	{
		.image_type_id = BEAGLEBONEAI64_SPL_IMAGE_GUID,
		.fw_name = u"BEAGLEBONEAI64_SPL",
		.image_index = 2,
	},
	{
		.image_type_id = BEAGLEBONEAI64_UBOOT_IMAGE_GUID,
		.fw_name = u"BEAGLEBONEAI64_UBOOT",
		.image_index = 3,
	},
	{
		.image_type_id = BEAGLEBONEAI64_SYSFW_IMAGE_GUID,
		.fw_name = u"BEAGLEBONEAI64_SYSFW",
		.image_index = 4,
	}
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=tiboot3.bin raw 0 2000 mmcpart 1;"
	"tispl.bin fat 0 1;u-boot.img fat 0 1; sysfw.itb fat 0 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	char fdtfile[50];

	snprintf(fdtfile, sizeof(fdtfile), "%s.dtb", CONFIG_DEFAULT_DEVICE_TREE);

	env_set("fdtfile", fdtfile);

	return 0;
}
#endif
