// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM625 PocketBeagle 2
 * https://www.beagleboard.org/boards/pocketbeagle-2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2025 Robert Nelson, BeagleBoard.org Foundation
 */

#include <efi_loader.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>

#include <asm/arch/hardware.h>
#include <asm/arch/k3-ddr.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = POCKETBEAGLE2_TIBOOT3_IMAGE_GUID,
		.fw_name = u"POCKETBEAGLE2_TIBOOT3",
		.image_index = 1,
	},
	{
		.image_type_id = POCKETBEAGLE2_SPL_IMAGE_GUID,
		.fw_name = u"POCKETBEAGLE2_SPL",
		.image_index = 2,
	},
	{
		.image_type_id = POCKETBEAGLE2_UBOOT_IMAGE_GUID,
		.fw_name = u"POCKETBEAGLE2_UBOOT",
		.image_index = 3,
	}
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 1=tiboot3.bin fat 1 1;"
		      "tispl.bin fat 1 1;u-boot.img fat 1 1",
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

/*
 * The U-Boot stack can collide with some dt reservations in the 512MB
 * configuration. Because of this, we need to relocate to just below our dt
 * reservations. The magic number below corresponds to the lowest value from
 * the block of conflicting reservations in the current device tree.
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	if (gd->ram_size == SZ_512M)
		return 0x9c800000;

	return gd->ram_top;
}

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	char fdtfile[50];

	snprintf(fdtfile, sizeof(fdtfile), "%s.dtb",
		 CONFIG_DEFAULT_DEVICE_TREE);

	env_set("fdtfile", fdtfile);

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */
