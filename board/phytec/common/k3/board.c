// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include <efi_loader.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <dm/ofnode.h>
#include <mtd.h>
#include <spl.h>
#include <malloc.h>
#include <asm/arch/hardware.h>

#include "../am6_som_detection.h"

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.fw_name = PHYCORE_AM6XX_FW_NAME_TIBOOT3,
		.image_index = 1,
	},
	{
		.fw_name = PHYCORE_AM6XX_FW_NAME_SPL,
		.image_index = 2,
	},
	{
		.fw_name = PHYCORE_AM6XX_FW_NAME_UBOOT,
		.image_index = 3,
	}
};

struct efi_capsule_update_info update_info = {
	.dfu_string = NULL,
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

/**
 * configure_capsule_updates() - Set up the DFU string for capsule updates
 *
 * Configures all three bootloader binaries for updates on the current
 * booted flash device, which may be eMMC, OSPI NOR, or a uSD card. If
 * booting from USB or Serial, capsule updates will be performed on the
 * eMMC device.
 *
 * Note: Currently, eMMC hardware partitions are not differentiated; Updates
 * are always applied to the first boot partition.
 */
static void configure_capsule_updates(void)
{
	static char dfu_string[128] = { 0 };
	const char *dfu_raw = "tiboot3.bin raw 0x0 0x400 mmcpart 1;"
			      "tispl.bin raw 0x400 0x1000 mmcpart 1;"
			      "u-boot.img.raw raw 0x1400 0x2000 mmcpart 1";
	const char *dfu_fat = "tiboot3.bin fat 1 1;"
			      "tispl.bin fat 1 1;"
			      "u-boot.img fat 1 1";
	const char *dfu_spi = "tiboot3.bin part 1;"
			     "tispl.bin part 2;"
			     "u-boot.img part 3";
	u32 boot_device = get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		snprintf(dfu_string, 128, "mmc 0=%s", dfu_raw);
		break;
	case BOOT_DEVICE_MMC2:
		snprintf(dfu_string, 128, "mmc 1=%s", dfu_fat);
		break;
	case BOOT_DEVICE_SPI:
		mtd_probe_devices();
		snprintf(dfu_string, 128, "mtd nor0=%s", dfu_spi);
		break;
	default:
		snprintf(dfu_string, 128, "mmc 0=%s", dfu_raw);
		break;
	};

	update_info.dfu_string = dfu_string;
}
#endif

#if IS_ENABLED(CONFIG_ENV_IS_IN_FAT) || IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
int mmc_get_env_dev(void)
{
	u32 boot_device = get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	};

	return CONFIG_ENV_MMC_DEVICE_INDEX;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 boot_device = get_boot_device();

	if (prio)
		return ENVL_UNKNOWN;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
	case BOOT_DEVICE_SPI:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
	default:
		return ENVL_NOWHERE;
	};
}

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
/**
 * Ensure the boot order favors the device we just booted from.
 * If boot_targets is still at its default value, move the current
 * boot device to the front of the list. Otherwise, leave any customized
 * order untouched.
 */
static void boot_targets_setup(void)
{
	u32 boot_device = get_boot_device();
	const char *boot_targets = NULL;
	char boot_targets_default[100];
	int ret;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		env_set_ulong("mmcdev", 0);
		env_set("boot", "mmc");
		boot_targets = "mmc0 mmc1 spi_flash dhcp";
		break;
	case BOOT_DEVICE_MMC2:
		env_set_ulong("mmcdev", 1);
		env_set("boot", "mmc");
		boot_targets = "mmc1 mmc0 spi_flash dhcp";
		break;
	case BOOT_DEVICE_SPI:
		env_set("boot", "spi");
		boot_targets = "spi_flash mmc0 mmc1 dhcp";
		break;
	case BOOT_DEVICE_ETHERNET:
		env_set("boot", "net");
		boot_targets = "dhcp mmc0 mmc1 spi_flash";
		break;
	case BOOT_DEVICE_UART:
		env_set("boot", "uart");
		break;
	case BOOT_DEVICE_DFU:
		env_set("boot", "usbdfu");
		break;
	};

	if (!boot_targets)
		return;

	ret = env_get_default_into("boot_targets", boot_targets_default, sizeof(boot_targets_default));
	if (ret < 0)
		boot_targets_default[0] = '\0';

	if (strcmp(boot_targets_default, env_get("boot_targets"))) {
		debug("boot_targets not default, don't change it\n");
		return;
	}

	env_set("boot_targets", boot_targets);
}

static void setup_mac_from_eeprom(void)
{
	struct phytec_api3_element *block_element;
	struct phytec_eeprom_data data;
	int ret;

	ret = phytec_eeprom_data_setup(&data, 0, EEPROM_ADDR);
	if (ret || !data.valid)
		return;

	PHYTEC_API3_FOREACH_BLOCK(block_element, &data) {
		switch (block_element->block_type) {
		case PHYTEC_API3_BLOCK_MAC:
			phytec_blocks_add_mac_to_env(block_element);
			break;
		default:
			debug("%s: Unknown block type %i\n", __func__,
			      block_element->block_type);
		}
	}
}

int board_late_init(void)
{
	boot_targets_setup();

	if (IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION_BLOCKS))
		setup_mac_from_eeprom();

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
	configure_capsule_updates();
#endif

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
static int fdt_apply_overlay_from_fit(const char *overlay_path, void *fdt)
{
	u64 loadaddr;
	ofnode node;
	int ret;

	node = ofnode_path(overlay_path);
	if (!ofnode_valid(node))
		return -FDT_ERR_NOTFOUND;

	ret = ofnode_read_u64(node, "load", &loadaddr);
	if (ret)
		return ret;

	return fdt_overlay_apply_verbose(fdt, (void *)loadaddr);
}

static void fdt_apply_som_overlays(void *blob)
{
	void *fdt_copy;
	u32 fdt_size;
	struct phytec_eeprom_data data;
	int err;

	fdt_size = fdt_totalsize(blob);
	fdt_copy = malloc(fdt_size);
	if (!fdt_copy)
		goto fixup_error;

	memcpy(fdt_copy, blob, fdt_size);

	err = phytec_eeprom_data_setup(&data, 0, EEPROM_ADDR);
	if (err)
		goto fixup_error;

	if (phytec_get_am6_rtc(&data) == 0) {
		err = fdt_apply_overlay_from_fit("/fit-images/som-no-rtc", fdt_copy);
		if (err)
			goto fixup_error;
	}

	if (phytec_get_am6_spi(&data) == PHYTEC_EEPROM_VALUE_X) {
		err = fdt_apply_overlay_from_fit("/fit-images/som-no-spi", fdt_copy);
		if (err)
			goto fixup_error;
	}

	if (phytec_get_am6_eth(&data) == 0) {
		err = fdt_apply_overlay_from_fit("/fit-images/som-no-eth", fdt_copy);
		if (err)
			goto fixup_error;
	}

	if (phytec_am6_is_qspi(&data)) {
		err = fdt_apply_overlay_from_fit("/fit-images/som-qspi-nor", fdt_copy);
		if (err)
			goto fixup_error;
	}

	memcpy(blob, fdt_copy, fdt_size);

cleanup:
	free(fdt_copy);
	return;

fixup_error:
	pr_err("Failed to apply SoM overlays\n");
	goto cleanup;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct phytec_eeprom_data data;
	int ret;

	fdt_apply_som_overlays(blob);
	fdt_copy_fixed_partitions(blob);

	ret = phytec_eeprom_data_setup(&data, 0, EEPROM_ADDR);
	if (ret || !data.valid)
		return 0;

	ret = phytec_ft_board_fixup(&data, blob);
	if (ret)
		pr_err("%s: Failed to add PHYTEC information to fdt.\n",
		       __func__);

	return 0;
}
#endif
