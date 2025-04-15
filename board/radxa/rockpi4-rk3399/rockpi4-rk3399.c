// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <efi_loader.h>

#define ROCKPI4_UPDATABLE_IMAGES	2

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
static struct efi_fw_image fw_images[ROCKPI4_UPDATABLE_IMAGES] = {0};

struct efi_capsule_update_info update_info = {
	.num_images = ROCKPI4_UPDATABLE_IMAGES,
	.images = fw_images,
};

#endif

#ifndef CONFIG_XPL_BUILD
#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) && IS_ENABLED(CONFIG_EFI_PARTITION)
static bool board_is_rockpi_4b(void)
{
	return of_machine_is_compatible("radxa,rockpi4b");
}

static bool board_is_rockpi_4c(void)
{
	return of_machine_is_compatible("radxa,rockpi4c");
}

void rockchip_capsule_update_board_setup(void)
{
	if (board_is_rockpi_4b()) {
		efi_guid_t idbldr_image_type_guid =
			ROCKPI_4B_IDBLOADER_IMAGE_GUID;
		efi_guid_t uboot_image_type_guid = ROCKPI_4B_UBOOT_IMAGE_GUID;

		guidcpy(&fw_images[0].image_type_id, &idbldr_image_type_guid);
		guidcpy(&fw_images[1].image_type_id, &uboot_image_type_guid);

		fw_images[0].fw_name = u"ROCKPI4B-IDBLOADER";
		fw_images[1].fw_name = u"ROCKPI4B-UBOOT";
	} else if (board_is_rockpi_4c()) {
		efi_guid_t idbldr_image_type_guid =
			ROCKPI_4C_IDBLOADER_IMAGE_GUID;
		efi_guid_t uboot_image_type_guid = ROCKPI_4C_UBOOT_IMAGE_GUID;

		guidcpy(&fw_images[0].image_type_id, &idbldr_image_type_guid);
		guidcpy(&fw_images[1].image_type_id, &uboot_image_type_guid);

		fw_images[0].fw_name = u"ROCKPI4C-IDBLOADER";
		fw_images[1].fw_name = u"ROCKPI4C-UBOOT";
	} else {
		update_info.num_images = 0;
	}
}
#endif /* CONFIG_EFI_HAVE_CAPSULE_SUPPORT && CONFIG_EFI_PARTITION */
#endif /* !CONFIG_XPL_BUILD */
