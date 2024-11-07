// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * EFI test application
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * This test program is used to test the invocation of an EFI application.
 * It writes a few messages to the console and then exits boot services
 */

#include <efi_api.h>

static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

static struct efi_system_table *systable;
static struct efi_boot_services *boottime;
static struct efi_simple_text_output_protocol *con_out;

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systab:	system table
 * Return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systab)
{
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;

	systable = systab;
	boottime = systable->boottime;
	con_out = systable->con_out;

	/* Get the loaded image protocol */
	ret = boottime->open_protocol(handle, &loaded_image_guid,
				      (void **)&loaded_image, NULL, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		con_out->output_string
			(con_out, u"Cannot open loaded image protocol\r\n");
		goto out;
	}

	/* UEFI requires CR LF */
	con_out->output_string(con_out, u"U-Boot test app for EFI_LOADER\r\n");

out:
	con_out->output_string(con_out, u"Exiting test app\n");
	ret = boottime->exit(handle, ret, 0, NULL);

	/* We should never arrive here */
	return ret;
}
