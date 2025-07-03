// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * EFI debug support
 *
 * Copyright (c) 2025 Ying-Chun Liu, Linaro Ltd. <paul.liu@linaro.org>
 */

#include <efi_loader.h>
#include <linux/sizes.h>
#include <u-boot/crc.h>

struct efi_system_table_pointer __efi_runtime_data * systab_pointer = NULL;

/**
 * efi_initialize_system_table_pointer() - Initialize system table pointer
 *
 * Return:	status code
 */
efi_status_t efi_initialize_system_table_pointer(void)
{
	/* Allocate efi_system_table_pointer structure with 4MB alignment. */
	systab_pointer = efi_alloc_aligned_pages(sizeof(struct efi_system_table_pointer),
						 EFI_RUNTIME_SERVICES_DATA,
						 SZ_4M);

	if (!systab_pointer) {
		log_err("Installing EFI system table pointer failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	systab_pointer->crc32 = 0;

	systab_pointer->signature = EFI_SYSTEM_TABLE_SIGNATURE;
	systab_pointer->efi_system_table_base = (uintptr_t)&systab;
	systab_pointer->crc32 = crc32(0,
				      (const unsigned char *)systab_pointer,
				      sizeof(struct efi_system_table_pointer));

	return EFI_SUCCESS;
}
