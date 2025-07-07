// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_debug_support
 *
 * Copyright (c) 2025 Ying-Chun Liu, Linaro Ltd. <paul.liu@linaro.org>
 *
 * Test the EFI_DEBUG_SUPPORT
 */

#include <efi_loader.h>
#include <efi_selftest.h>

/**
 * efi_st_debug_support_execute() - execute test
 *
 * Test EFI_DEBUG_SUPPORT tables.
 *
 * Return:	status code
 */
static int efi_st_debug_support_execute(void)
{
	struct efi_debug_image_info_table_header *efi_st_debug_info_table_header = NULL;
	efi_guid_t efi_debug_image_info_table_guid = EFI_DEBUG_IMAGE_INFO_TABLE_GUID;

	/* get EFI_DEBUG_IMAGE_INFO_TABLE */
	efi_st_debug_info_table_header = efi_st_get_config_table(&efi_debug_image_info_table_guid);

	if (!efi_st_debug_info_table_header) {
		efi_st_error("Missing EFI_DEBUG_IMAGE_INFO_TABLE\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(debug_support) = {
	.name = "debug_support",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = efi_st_debug_support_execute,
};
