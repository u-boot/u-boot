// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025, Ferass El Hafidi <funderscore@postmarketos.org>
 */

#include <asm/arch/boot.h>
#include <dm.h>
#include <efi_loader.h>
#include <mmc.h>

/*
 * To be able to support multiple devices and flash to the correct one we need
 * to automatically generate the dfu_string and fw_name to match the device we
 * are booted from. This is done by meson_setup_capsule() which is then called
 * in board_late_init(). Right now we support EFI capsule updates on SPI flash,
 * eMMC and SD cards.
 */
struct efi_fw_image fw_images[] = {
	{
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = NULL, /* to be set in meson_capsule_setup */
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

/*
 * TODO: Support usecase e.g. FIT image on eMMC + SPL on SD.
 */
void meson_setup_capsule(void)
{
	static char dfu_string[32] = { 0 };
	int mmc_devnum = 0; /* mmc0 => SD card */
	u32 max_size = 0x2000; /* 4 MB (MMC sectors are 512 bytes each) */
	u32 offset = 0x1; /* offset for flashing to eMMC/SD */
	int boot_device = meson_get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_EMMC:
		mmc_devnum = 1; /* mmc1 is always eMMC */
		fallthrough;
	case BOOT_DEVICE_SD:
		snprintf(dfu_string, 32, "mmc %d=u-boot.bin raw %d %d", mmc_devnum, offset, max_size);
		fw_images[0].fw_name = u"U_BOOT_MESON_MMC";
		break;
	case BOOT_DEVICE_SPI:
		/* We assume there's only one SPI flash */
		fw_images[0].fw_name = u"U_BOOT_MESON_SPI";
		snprintf(dfu_string, 32, "sf 0:0=u-boot.bin raw 0 %d", max_size);
		break;
	default:
		debug("setup_capsule: Boot device %d unsupported\n", boot_device);
		return;
	}
	debug("EFI Capsule DFU string: %s", dfu_string);

	update_info.dfu_string = dfu_string;
}
