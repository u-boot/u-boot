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

	con_out->output_string(con_out, L"Hello, world!\n");

	/* Get the loaded image protocol */
	ret = boottime->handle_protocol(handle, &loaded_image_guid,
					(void **)&loaded_image);
	if (ret != EFI_SUCCESS) {
		con_out->output_string(con_out,
				       L"Cannot open loaded image protocol\n");
		goto out;
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
