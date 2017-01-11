/*
 * EFI hello world
 *
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <part_efi.h>
#include <efi_api.h>

efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;
	struct efi_boot_services *boottime = systable->boottime;

	con_out->output_string(con_out, L"Hello, world!\n");
	boottime->exit(handle, 0, 0, NULL);

	return EFI_SUCCESS;
}
