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
#include <linux/sizes.h>

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
	struct efi_mem_desc *memory_map;
	efi_uintn_t map_size = 0;
	efi_uintn_t map_key;
	efi_uintn_t desc_size;
	u32 desc_version;
	efi_status_t ret;

	/* get EFI_DEBUG_IMAGE_INFO_TABLE */
	efi_st_debug_info_table_header = efi_st_get_config_table(&efi_debug_image_info_table_guid);

	if (!efi_st_debug_info_table_header) {
		efi_st_error("Missing EFI_DEBUG_IMAGE_INFO_TABLE\n");
		return EFI_ST_FAILURE;
	}

	/* Load memory map */
	ret = st_boottime->get_memory_map(&map_size, NULL, &map_key, &desc_size,
					  &desc_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error
			("GetMemoryMap did not return EFI_BUFFER_TOO_SMALL\n");
		return EFI_ST_FAILURE;
	}
	/* Allocate extra space for newly allocated memory */
	map_size += sizeof(struct efi_mem_desc);
	ret = st_boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, map_size,
					 (void **)&memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = st_boottime->get_memory_map(&map_size, memory_map, &map_key,
					  &desc_size, &desc_version);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetMemoryMap failed\n");
		return EFI_ST_FAILURE;
	}
	/* Find the system table pointer */
	for (efi_uintn_t i = 0; map_size; ++i, map_size -= desc_size) {
		struct efi_mem_desc *entry = &memory_map[i];
		u64 end;

		if (entry->type != EFI_RUNTIME_SERVICES_DATA)
			continue;

		end = entry->physical_start +
		      (entry->num_pages << EFI_PAGE_SHIFT);
		for (u64 pos = ALIGN(entry->physical_start, SZ_4M);
		     pos <= end; pos += SZ_4M) {
			struct efi_system_table_pointer *systab_pointer =
				(void *)(uintptr_t)pos;

			/* check for overflow */
			if (pos < entry->physical_start)
				break;
			if (systab_pointer->signature ==
			    EFI_SYSTEM_TABLE_SIGNATURE) {
				if (systab_pointer->efi_system_table_base !=
				    (uintptr_t)st_systable) {
					efi_st_error("Wrong system table address\n");
					ret = EFI_ST_FAILURE;
					goto out;
				}
				ret = EFI_ST_SUCCESS;
				goto out;
			}
		}
	}
	efi_st_error("System table pointer not found\n");
	ret = EFI_ST_FAILURE;

out:
	st_boottime->free_pool(memory_map);

	return ret;
}

EFI_UNIT_TEST(debug_support) = {
	.name = "debug_support",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = efi_st_debug_support_execute,
};
