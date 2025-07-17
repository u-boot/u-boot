// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM62Px platforms
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#include <efi_loader.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>
#include "../common/fdt_ops.h"

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = AM62PX_SK_TIBOOT3_IMAGE_GUID,
		.fw_name = u"AM62PX_SK_TIBOOT3",
		.image_index = 1,
	},
	{
		.image_type_id = AM62PX_SK_SPL_IMAGE_GUID,
		.fw_name = u"AM62PX_SK_SPL",
		.image_index = 2,
	},
	{
		.image_type_id = AM62PX_SK_UBOOT_IMAGE_GUID,
		.fw_name = u"AM62PX_SK_UBOOT",
		.image_index = 3,
	}
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=tiboot3.bin raw 0 80000;"
	"tispl.bin raw 80000 200000;u-boot.img raw 280000 400000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#if defined(CONFIG_XPL_BUILD)
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS)) {
		if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
			fixup_ddr_driver_for_ecc(spl_image);
	} else {
		fixup_memory_node(spl_image);
	}
}
#endif

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	ti_set_fdt_env(NULL, NULL);
	return 0;
}
#endif
