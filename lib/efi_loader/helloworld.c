/*
 * EFI hello world
 *
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * This program demonstrates calling a boottime service.
 * It writes a greeting and the load options to the console.
 */

#include <common.h>
#include <efi_api.h>

static const efi_guid_t loaded_image_guid = LOADED_IMAGE_GUID;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;

static int hw_memcmp(const void *buf1, const void *buf2, size_t length)
{
	const u8 *pos1 = buf1;
	const u8 *pos2 = buf2;

	for (; length; --length) {
		if (*pos1 != *pos2)
			return *pos1 - *pos2;
		++pos1;
		++pos2;
	}
	return 0;
}

/*
 * Entry point of the EFI application.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 * @return	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;
	struct efi_boot_services *boottime = systable->boottime;
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;
	efi_uintn_t i;

	con_out->output_string(con_out, L"Hello, world!\n");

	/* Get the loaded image protocol */
	ret = boottime->handle_protocol(handle, &loaded_image_guid,
					(void **)&loaded_image);
	if (ret != EFI_SUCCESS) {
		con_out->output_string(con_out,
				       L"Cannot open loaded image protocol\n");
		goto out;
	}
	/* Find configuration tables */
	for (i = 0; i < systable->nr_tables; ++i) {
		if (!hw_memcmp(&systable->tables[i].guid, &fdt_guid,
			       sizeof(efi_guid_t)))
			con_out->output_string(con_out, L"Have device tree\n");
		if (!hw_memcmp(&systable->tables[i].guid, &smbios_guid,
			       sizeof(efi_guid_t)))
			con_out->output_string(con_out, L"Have SMBIOS table\n");
	}
	/* Output the load options */
	con_out->output_string(con_out, L"Load options: ");
	if (loaded_image->load_options_size && loaded_image->load_options)
		con_out->output_string(con_out,
				       (u16 *)loaded_image->load_options);
	else
		con_out->output_string(con_out, L"<none>");
	con_out->output_string(con_out, L"\n");

out:
	boottime->exit(handle, ret, 0, NULL);

	/* We should never arrive here */
	return ret;
}
