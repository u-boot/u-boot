// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM625 PocketBeagle 2
 * https://www.beagleboard.org/boards/pocketbeagle-2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2025 Robert Nelson, BeagleBoard.org Foundation
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>
#include <efi_loader.h>

#include "../../ti/common/board_detect.h"
#include "pocketbeagle2_ddr.h"

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
	.dfu_string = "mmc 0=tiboot3.bin raw 0 2000 mmcpart 1;"
		      "tispl.bin fat 0 1;u-boot.img fat 0 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

static int pocketbeagle2_get_ddr_size(void)
{
	// check config overrides first
	if (IS_ENABLED(CONFIG_POCKETBEAGLE2_AM62X_RAM_SIZE_FIX)) {
		if (IS_ENABLED(CONFIG_POCKETBEAGLE2_AM62X_RAM_SIZE_512MB))
			return EEPROM_RAM_SIZE_512MB;
		if (IS_ENABLED(CONFIG_POCKETBEAGLE2_AM62X_RAM_SIZE_1GB))
			return EEPROM_RAM_SIZE_1GB;
	}

#if IS_ENABLED(CONFIG_TI_I2C_BOARD_DETECT)
	// dynamically detect the config if we can
	if (!do_board_detect_am6()) {
		struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

		if (strlen(ep->name) > 11 && ep->name[0] == 'P') {
			/*
			 * POCKETBEAGL2A00 (am6232 512MB)
			 * POCKETBEAGL2A10 (am625 512MB)
			 * POCKETBEAGL2A1I (am625 1GB)
			 */
			if (!strncmp(&ep->name[11], "2A1I", 4))
				return EEPROM_RAM_SIZE_1GB;
		}
	}
#endif

	return EEPROM_RAM_SIZE_512MB;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

/**
 * board_setup_dest_addr - Adjust the relocation address for this device
 *
 * The u-boot stack can collide with some dt reservations in the 512MB
 * configuration. Because of this, we need to relocate u-boot to just below our
 * dt reservations. This is the lowest remoteproc related reservation in dt
 * currently.
 */
int board_setup_dest_addr(void)
{
	gd->relocaddr = 0x9c800000;
	return 0;
}

// logic after this block assumes that there is only 1 DRAM bank currently
#if CONFIG_NR_DRAM_BANKS != 1
#error Unsupported number of DRAM banks!
#endif

int dram_init_banksize(void)
{
	eeprom_ram_size ram_size;

	memset(gd->bd->bi_dram, 0, sizeof(gd->bd->bi_dram[0]));

	ram_size = pocketbeagle2_get_ddr_size();
	switch (ram_size) {
	case EEPROM_RAM_SIZE_1GB:
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = SZ_1G;
		gd->ram_size = SZ_1G;
		break;
	case EEPROM_RAM_SIZE_512MB:
		fallthrough;
	default:
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = SZ_512M;
		gd->ram_size = SZ_512M;
		break;
	}

	return 0;
}

#if IS_ENABLED(CONFIG_K3_DDRSS)
static int update_ddrss_timings(void)
{
	int ret = 0;
	eeprom_ram_size ram_size;
	struct ddrss *ddr_patch = NULL;
	void *fdt = (void *)gd->fdt_blob;

	ram_size = pocketbeagle2_get_ddr_size();
	ddr_patch = &pocketbeagle2_ddrss_data[ram_size];

	if (!ddr_patch)
		return ret;

	ret = fdt_apply_ddrss_timings_patch(fdt, ddr_patch);
	if (ret) {
		printf("Failed to apply ddrs timings patch: %d\n", ret);
		return ret;
	}

	return ret;
}

int do_board_detect(void)
{
	void *fdt = (void *)gd->fdt_blob;
	u64 start[] = { gd->bd->bi_dram[0].start };
	u64 size[] = { gd->bd->bi_dram[0].size };
	int ret;

	dram_init();
	dram_init_banksize();

	ret = fdt_fixup_memory_banks(fdt, start, size, 1);
	if (ret) {
		printf("Failed to fixup memory banks: %d\n", ret);
		return ret;
	}

	return update_ddrss_timings();
}
#endif /* CONFIG_K3_DDRSS */

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
